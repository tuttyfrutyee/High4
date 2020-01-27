#include "acceleration.h"
#include "imugather.h"
#include "mpu6050.h"
#include "sdcard.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>




#define sSDefaultAddr 0x68 //the defaultAddr i2c will talk to. Should be changed syncronously with flagActive
#define flagActive 0 //this is flag related with ssDefaultAddr, it defines being pins active low or high.

typedef struct IMU{

    int assignedPin;
    MPU6050 imu;

} IMU;

typedef struct QueueElement{

    int8_t* array;
    int arrayLength

} QueueElement;

QueueElement* queue;
int maxCapacity = 30; //actually max capacity is maxCapacity - 1
int front = 0, rear = 0;


//queue functions

int isQueueFull(){
    return ((rear+1) % maxCapacity) == front;
}

int isQueueEmpty(){
    return (rear == front);
}

int pushToQueue(QueueElement * element){
    
    if(isQueueFull){
        printf("Error : Queue is full, can not push anymore\n");
        return 0;
    }

    rear = (rear + 1) % maxCapacity;

    queue[rear] = element;
}

QueueElement* popFromQueue(){

    if(isQueueEmpty()){
        printf("Error . Queue is empty, can not pop anymore\n");
        return 0;
    }
    front = (front + 1) % maxCapacity;

}

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

IMU* imuStack;
IMUGATHER* gather;
IMU* previousSelectedImu;






int initIMUGATHERSensors(IMUGATHER* _gather){

    if(imuStack) free(imuStack); //make sure previous init is deleted
    previousSelectedImu = NULL;

    gather = _gather;

    imuStack = (IMU*) malloc(gather->numberOfImu * sizeof(IMU));

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
  
    for(int i = 0; i < gather->numberOfImu; i++){
        selectImu(imuStack + i);
        selfTest();
    }    

    vTaskDelay(200 / portTICK_PERIOD_MS);


    return 1;
}


int16_t* getGatherAccelerationsAsArrayInOrder(IMUGATHER* gather){

    int16_t* accelerationGatherData = (int16_t*) malloc(sizeof(int16_t) * 6 * gather->numberOfImu);
    
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

int64_t getTime(){
    return (int64_t) (clock() * 1000 / CLOCKS_PER_SEC); //returns time in milliseconds
}


void goCollectCurrentModeData(IMUGATHER* gather){

    int8_t startSequence[3] = {0,0,0}

    gather->currentModeIndicator = (gather->currentModeIndicator+1) % gather->numberOfModes;

    //put start sequence
    writeToSensorDataBytes(startSequence, 3);

    //put currentMode
    writeToSensorDataBytes(&(gather->currentModeIndicator), 1);

    //start to fetch data continously with duration gather->dataCollectDuration

    int64_t startTime = getTime();

    int64_t previousTime, currentTime;

    previousTime = currentTime = startTime;

    while((currentTime - startTime) < gather->dataCollectDuration){

        previousTime = currentTime;

        getGatherAccelerations();

        currentTime = getTime();

        if(!(previousTime == startTime)){
            QueueElement* timeDifferenceElement = (QueueElement*)malloc(sizeof(QueueElement));
            timeDifferenceElement->array = (int8_t*)malloc(sizeof(int8_t));
            timeDifferenceElement->array[0] = currentTime - previousTime; // hoping that time difference will fit into 8bit signed integer
            if((currentTime-previousTime) > 63) printf("Error : Time dif between consequent data capture is bigger than 63ms, it is : %d ms", currentTime-previousTime);
            timeDifferenceElement->arrayLength = 1;
            pushToQueue(timeDifferenceElement);
        }


        QueueElement* dataElement = (QueueElement*)malloc(sizeof(QueueElement));
        dataElement->array = (int8_t*)getGatherAccelerationsAsArrayInOrder(gather);
        dataElement->arrayLength = gather->numberOfImu * 6 * 2;
        pushToQueue(dataElement);

    }


}
