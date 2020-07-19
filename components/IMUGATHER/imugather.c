
#include "imugather.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>




#define sSDefaultAddr 0x68 //the defaultAddr i2c will talk to. Should be changed syncronously with flagActive
#define flagActive 0 //this is flag related with ssDefaultAddr, it defines being pins active low or high.



IMU* imuStack;
IMUGATHER* gather;
IMU* previousSelectedImu;



//#define BOARD


int initIMUGATHERSensors(IMUGATHER* _gather);
void initGPIOs();
void selectImu(IMU* imu);
void configureImus();

#ifdef BOARD

int assignedPinsResp[5] = {16}; //for development

#else

int assignedPinsResp[4] = {14,27,26,25}; // left to right fingers

#endif






int initIMUGATHERSensors(IMUGATHER* _gather){

    if(imuStack) free(imuStack); //make sure previous init is deleted
    previousSelectedImu = NULL;

    gather = _gather;

    imuStack = (IMU*) malloc(gather->numberOfImu * sizeof(IMU));

    if(!imuStack)  printf("could not allocate imuStack \n");
    printf("gather->numberOfImu = %d\n", _gather->numberOfImu);

    for(int i = 0; i < gather->numberOfImu; i++)
        imuStack[i].assignedPin = assignedPinsResp[i];

    initGPIOs(gather->numberOfImu);

    //after init gpio, set configurations
    
    configureImus();

    return 1;

}

void initGPIOs(){
    //will init gpios to output levels and set them !flagActive
    for(int i = 0; i < gather->numberOfImu; i++){
        int targetAssignedPin = (imuStack+i)->assignedPin;

        gpio_pad_select_gpio(targetAssignedPin);

        gpio_set_direction(targetAssignedPin, GPIO_MODE_OUTPUT);

        gpio_set_level(targetAssignedPin,!flagActive);
    }

}

void selectImu(IMU* imu){

    if(previousSelectedImu) {
        gpio_pad_select_gpio(previousSelectedImu->assignedPin);

        gpio_set_level(previousSelectedImu->assignedPin, !flagActive);
    }


    gpio_pad_select_gpio(imu->assignedPin);

    gpio_set_level(imu->assignedPin, flagActive);

    previousSelectedImu = imu;

}

//should be called after gpios are known to be initialised
void configureImus(){
    for(int i = 0; i < gather->numberOfImu; i++){
        selectImu(imuStack + i); //select the sensor to talk

        setConfigurations();
    }
}

void getGatherAccelerations(){

    if(!imuStack) printf("no Imu stack, hence can't get accelerations\n");

    for(int i = 0; i < gather->numberOfImu; i++){
        selectImu(imuStack + i);
        imuStack[i].imu.acc = getAccelerations();
    }    

}




void printAccelerationDatas(IMUGATHER* gather){
    
    if(!imuStack) printf("no Imu stack, hence can't get accelerations\n");

    for(int i = 0; i < gather->numberOfImu; i++)
        printAccelerationData(imuStack[i].imu);
    

}

void printAccelerationRawDatas(IMUGATHER* gather){
    
    if(!imuStack) printf("no Imu stack, hence can't get accelerations\n");

    for(int i = 0; i < gather->numberOfImu; i++)
        printAccelerationRawData(imuStack[i].imu);
    
}

int selfTestSensors(IMUGATHER* gather){
    if(!imuStack) printf("no Imu stack, hence can't apply self test\n");
  
    physical_standby_start();

    for(int i = 0; i < gather->numberOfImu; i++){
        selectImu(imuStack + i);
        selfTest();
    }    

    vTaskDelay(200 / portTICK_PERIOD_MS );

    physical_standby_stop();


    return 1;
}


int16_t* getGatherAccelerationsAsArrayInOrder(){

    int16_t* accelerationGatherData = (int16_t*) malloc(sizeof(int16_t) * 6 * gather->numberOfImu);

    if(accelerationGatherData == NULL){
        printf("\nErr:Could not allocated memory for acceleration data\n");
        return NULL;
    }

    if(imuStack == NULL){
        printf("\n Err: Imu stack is null, could not get accelerations raw.\n");
        return NULL;
    }
    
    for(int i = 0; i < gather->numberOfImu; i++){

        int offset = i * 6;

        accelerationGatherData[offset + 0] = imuStack[i].imu.acc.rawLinAccX;
        accelerationGatherData[offset + 1] = imuStack[i].imu.acc.rawLinAccY;
        accelerationGatherData[offset + 2] = imuStack[i].imu.acc.rawLinAccZ;
        accelerationGatherData[offset + 3] = imuStack[i].imu.acc.rawRadAccX;
        accelerationGatherData[offset + 4] = imuStack[i].imu.acc.rawRadAccY;
        accelerationGatherData[offset + 5] = imuStack[i].imu.acc.rawRadAccZ;

    }
    return accelerationGatherData;
}

float * getGatherAccelerationsAsArrayInOrderProcessed(){

    float* accelerationGatherDataProcessed = (float*) malloc(sizeof(float) * 6 * gather->numberOfImu);

    if(accelerationGatherDataProcessed == NULL){
        printf("\n Err: Could not allocated memory for acceleration data processed\n");
        return NULL;
    }

    if(imuStack == NULL){
        printf("\n Err: Imu stack is null, could not get accelerations processed.\n");
        return NULL;
    }

    for(int i = 0; i < gather->numberOfImu; i++){

        int offset = i * 6;

        accelerationGatherDataProcessed[offset + 0] = imuStack[i].imu.acc.linAccX;
        accelerationGatherDataProcessed[offset + 1] = imuStack[i].imu.acc.linAccY;
        accelerationGatherDataProcessed[offset + 2] = imuStack[i].imu.acc.linAccZ;
        accelerationGatherDataProcessed[offset + 3] = imuStack[i].imu.acc.radAccX;
        accelerationGatherDataProcessed[offset + 4] = imuStack[i].imu.acc.radAccY;
        accelerationGatherDataProcessed[offset + 5] = imuStack[i].imu.acc.radAccZ;

    }

    return accelerationGatherDataProcessed;    

}






