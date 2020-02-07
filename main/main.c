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

static const char *TAG = "MAIN";

static xQueueHandle gpio_evt_queue;


static IMUGATHER gather = {4,3,0,3000,7000}; // numberOfImu, numberOfMode, currentMode, criticalTime, dataCollectionDuration


//local function definitions
static void buttonHandler();
static void lifeCycleStart();
static void initPeripherals();

static int fatalError = 0;
static int flagGo = 0;

void app_main(void)
{


    initPeripherals();

    selfTestSensors(&gather);    

    lifeCycleStart();

}



static void buttonHandler(void* arg)
{
    uint32_t io_num;

    int counter = 0;

    int64_t startTime = getTime();

    int64_t previousTime, currentTime;

    previousTime = currentTime = startTime;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

            previousTime = currentTime;
            currentTime = getTime();

            if((currentTime - previousTime) > 250){
                printf("GPIO[%d] intr, val: %d, time between : %lld\n", io_num, gpio_get_level(io_num), currentTime - previousTime);
                if(counter % 2 == 0)
                    lightOnBlueLed();
                else
                    lightOffBlueLed();
                counter ++;
            }

        }
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

    //note that, imuInit has to be after i2c init
    initIMUGATHERSensors(&gather);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    setUpButtonIsr(&gpio_evt_queue);
    xTaskCreate(buttonHandler, "buttonHandler", 4096 * 2, NULL, 10, NULL);



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

        //go collect data here
        goCollectCurrentModeData(&gather);

        flagGo = 0;
        lifeCycle++;
    }
}

