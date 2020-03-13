#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "i2c.h"
#include "esp_timer.h"
#include "stdio.h"
#include <time.h>


#include "acceleration.h"
#include "imugather.h"
#include "sdcard.h"
#include "mqtt.h"
#include "button.h"
#include "buzzer.h"
#include "helper.h"

static const char *TAG = "MAIN";

static xQueueHandle gpio_evt_queue;



static IMUGATHER gather = {4,3,0,6000,15000}; // numberOfImu, numberOfMode, currentMode, criticalTime, dataCollectionDuration


//local function definitions
static void buttonHandler();
static void lifeCycleStart();
static void initPeripherals();

static int fatalError = 0;
static int flagGo = 0;

void app_main(void)
{


    initPeripherals();    

    //clearSensorData();

    //selfTestSensors(&gather);    

    lifeCycleStart();

}

//stack imp for button handler

#define BUTTON_STACK_CAPACITY 5

QueueButtonEl buttonStack[BUTTON_STACK_CAPACITY];

static int buttonStackSize = 0;

void pushElementToButtonStack(QueueButtonEl el){
    if(buttonStackSize == (BUTTON_STACK_CAPACITY)){
        for(int i = BUTTON_STACK_CAPACITY - 1; i > 0; i --){
            buttonStack[i - 1] = buttonStack[i];
        }
        buttonStack[BUTTON_STACK_CAPACITY - 1] = el;
    }else{
        buttonStack[buttonStackSize] = el;
        buttonStackSize++;
    }
}

void flashButtonStack(){
    buttonStackSize = 0;
}

QueueButtonEl getLastStackButtonEl(){
    QueueButtonEl willReturn = {0,0};
    if(buttonStackSize > 0)
        willReturn = buttonStack[buttonStackSize - 1];
    return willReturn;
}

static void buttonHandler(void* arg)
{
    uint32_t io_num;
    int64_t thresholdTime = 90; //in ms

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {


            QueueButtonEl lastElement = getLastStackButtonEl();

            if(!lastElement.occTime){
                QueueButtonEl newEl = {getTime(), gpio_get_level(io_num)};
                pushElementToButtonStack(newEl);
                printf("\n           ------------------ \n");
                printf("initiated queue\n");
            }else{
                //printf("something... \n");
                if((getTime() - lastElement.occTime) > thresholdTime){
                    
                    printf("%d\n", gpio_get_level(io_num));                    
                    printf("put it as queuebuttonel, %lld \n", (getTime() - lastElement.occTime));
                    QueueButtonEl newEl = {getTime(), gpio_get_level(io_num)};
                    pushElementToButtonStack(newEl);
                }

            }

        }
    }
}

static void tapRecogniser(){

    bool flagPrevDetectedSingleTap = 0;
    int64_t diffSADTap = 400; //in ms

    bool singleTapDetected = 0;
    bool doubleTapDetected = 0;
    bool holdMediumDetected = 0;
    bool holdLongDetected = 0;

    printf("tapRecogniser started \n");

    while(true){

        singleTapDetected = detectSingleTap(buttonStack, buttonStackSize);
        holdMediumDetected = detectHoldMedium(buttonStack, buttonStackSize);
        holdLongDetected = detectHoldLong(buttonStack, buttonStackSize);

        if(singleTapDetected){

            vTaskDelay(diffSADTap / portTICK_RATE_MS);

            doubleTapDetected = detectDoubleTap(buttonStack, buttonStackSize);

            if(doubleTapDetected){
                //handle double tap
                printf("double tap occured\n");
                flashButtonStack();
            }
            else{
                //handle single tap
                printf("single tap occured\n"); 
                flagGo = 1;              


                flashButtonStack();
            }
        }
        else if(holdMediumDetected){
            //handle hold medium
            printf("hold medium occured\n");
            unMountSdCard();

            flashButtonStack();
        }
        else if(holdLongDetected){
            //hanlde hold long
            printf("hold long occured\n");

            flashButtonStack();
        }

        vTaskDelay(250 / portTICK_RATE_MS);

    }



}

static void initPeripherals(){

/*  //init internal storeage system, not sure if need it, todo: check it
    nvs_flash_init();
    
    //init wifi for mqtt, at this point esp32 draws heavy current
    wifi_init();

    //init mqtt
    mqtt_app_start(); */
    
    //init i2c
    int ret;
    ret = i2c_master_init(300000);
    if(ret == ESP_OK)
        printf("i2c is init well\n");
    else
        printf("something went wrong with i2c init\n");

    //init gpio relateds
    //note : it init button gpio and also the internal blue led gpio
    initButtonGpio();

    initBuzzGpio();

    //note that, imuInit has to be after i2c init
    initIMUGATHERSensors(&gather);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    setUpButtonIsr(&gpio_evt_queue);
    xTaskCreate(buttonHandler, "buttonHandler", 4096, NULL, 10, NULL);

    xTaskCreate(tapRecogniser, "tapRecogniser", 4096 * 2, NULL, 10, NULL);



    //init spi, sdcard
    initSdCard();


}

static void lifeCycleStart(){

    int lifeCycle = 0;

    startRecordingData();

    while(true){

        while((!flagGo) | fatalError){ //if there is fatal error stay in hold on mode
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        printf("button pressed, collecting... \n");
        //get recordCounter and increment by 1
        //get currentModeCounter and update it with the next one
        int recordCounter, currentModeCounter;

        getAndUpdateLookUpTable(&recordCounter, &currentModeCounter);

        char fileNameToWrite[20];
        sprintf(fileNameToWrite, "D_%d_%d", recordCounter, currentModeCounter);

        printf("\n filename : %s \n", fileNameToWrite);

        //go collect data here
        goCollectCurrentModeData(&gather, fileNameToWrite);

        flagGo = 0;
        lifeCycle++;
    }
}

