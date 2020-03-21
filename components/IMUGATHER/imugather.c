#include "acceleration.h"
#include "imugather.h"
#include "mpu6050.h"
#include "sdcard.h"
#include "helper.h"

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>




#define sSDefaultAddr 0x68 //the defaultAddr i2c will talk to. Should be changed syncronously with flagActive
#define flagActive 0 //this is flag related with ssDefaultAddr, it defines being pins active low or high.

TaskHandle_t xRecorder = NULL;

char * fileNameToWriteGlobal;

typedef struct IMU{

    int assignedPin;
    MPU6050 imu;

} IMU;


typedef struct QueueElement{

    int8_t* array;
    int arrayLength;

} QueueElement;


#define MAXCAPACITY 1000 //actually max capacity is maxCapacity - 1
int front = 0, rear = 0;
QueueElement** queue[MAXCAPACITY];

//queue functions

int isQueueFull(){
    return ((rear+1) % MAXCAPACITY) == front;
}

int isQueueEmpty(){
    return (rear == front);
}

int getQueueSize(){
    int size = (rear - front);
    if(size < 0)
        size += MAXCAPACITY;
    return size;
}

int pushToQueue(QueueElement* element){
    
    if(isQueueFull()){
        printf("Error : Queue is full, can not push anymore\n");
        return 0;
    }

    queue[rear] = element;    

    rear = (rear + 1) % MAXCAPACITY;

    return 1;
}

QueueElement* popFromQueue(){

    if(isQueueEmpty()){
        printf("Error . Queue is empty, can not pop anymore\n");
        return 0;
    }

    QueueElement* willReturn = queue[front];

    front = (front + 1) % MAXCAPACITY;

    return willReturn;

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


void goCollectCurrentModeData(IMUGATHER* gather, char* fileNameToWrite){

    if(!(fileNameToWriteGlobal && (strcmp(fileNameToWrite, fileNameToWriteGlobal) == 0))){
        free(fileNameToWriteGlobal);
        int fileLength = strlen(fileNameToWrite);
        fileNameToWriteGlobal = (char*) malloc(sizeof(char) * (fileLength + 1));
        memset(fileNameToWriteGlobal, '\0', fileLength);
        strcpy(fileNameToWriteGlobal, fileNameToWrite);
    }



    const int64_t fireTimeElapsed = gather->dataCollectDuration / 2;

    bool fired = false;
// 8,3,7,1,6,5
    int8_t fireSequence[6] = {1,2,0,3,4,0};
    const int fireSequenceLength = 6;

    
    //put number of sensors used

    writeToBinFile(&(gather->numberOfImu), 1, fileNameToWrite);

    //put currentMode

    writeToBinFile(&(gather->currentModeIndicator), 1, fileNameToWrite);

    //put configSettings -> linRange(1byte) - radRange(1byte)

    writeToBinFile(&linRange, 1, fileNameToWrite);

    writeToBinFile(&radRange, 1, fileNameToWrite);

/*     int16_t * deneme = (int16_t *) malloc(sizeof(int16_t) * 1);
    deneme[0] = 513;

    writeToBinFile(deneme, 2, fileNameToWrite); */


    //start to fetch data continously with duration gather->dataCollectDuration

    int64_t startTime = getTime();

    int64_t previousTime, currentTime;

    previousTime = currentTime = startTime;

    int totalCapture = 0;

    int totalBytes = 0;

    while((currentTime - startTime) < gather->dataCollectDuration){

        if(((currentTime - startTime) > fireTimeElapsed) && !fired){
            fired = true;
            alarmFire();
            QueueElement* fireElement = (QueueElement*)malloc(sizeof(QueueElement));
            fireElement->array = (int8_t*)malloc(sizeof(int8_t) * fireSequenceLength);
            printf("\n\n");
            for(int i = 0; i < fireSequenceLength; i++){
                fireElement->array[i] = fireSequence[i];
                printf("%d, ",fireSequence[i]);
            }
            printf("\n\n");
            fireElement->arrayLength = fireSequenceLength;
            pushToQueue(fireElement);            
        }

        totalCapture++;

        previousTime = currentTime;

        getGatherAccelerations();

        //printAccelerationDatas(gather);

        currentTime = getTime();

        if(!(previousTime == startTime)){
            QueueElement* timeDifferenceElement = (QueueElement*)malloc(sizeof(QueueElement));
            timeDifferenceElement->array = (int8_t*)malloc(sizeof(int8_t));
            timeDifferenceElement->array[0] = currentTime - previousTime; // hoping that time difference will fit into 8bit signed integer
            if((currentTime-previousTime) > 63) printf("Error : Time dif between consequent data capture is bigger than 63ms, it is : %lld ms", currentTime-previousTime);
            timeDifferenceElement->arrayLength = 1;
            //totalBytes++;
            pushToQueue(timeDifferenceElement);
        }


        QueueElement* dataElement = (QueueElement*)malloc(sizeof(QueueElement));
        dataElement->array = (int8_t*)getGatherAccelerationsAsArrayInOrder(gather);
        dataElement->arrayLength = gather->numberOfImu * 6 * 2;
        pushToQueue(dataElement);
        //totalBytes += dataElement->arrayLength;
        vTaskDelay(5 / portTICK_PERIOD_MS);

/*         if(totalCapture == 100){
            int8_t * deneme = (int8_t*) malloc(sizeof(int8_t) * 6);
            deneme[0] = 8;
            deneme[1] = 3;
            deneme[2] = 7;
            deneme[3] = 1;
            deneme[4] = 6;
            deneme[5] = 5;            
            QueueElement* dataElement = (QueueElement*)malloc(sizeof(QueueElement));
            dataElement->array = deneme;
            dataElement->arrayLength = 6;
            pushToQueue(dataElement);
        } */

    }

    printf("capturePerSecond = %f\n", ((float)totalCapture) / (currentTime - startTime) * 1000.0 );
    //printf("totalBytes : %d\n", totalBytes);


}

#define CLUSTER_MAX_LENGTH 100

void recordData(){
    printf("started to recording data peopleeee \n");
    
    int iteration = 0;

    QueueElement* elementCluster[CLUSTER_MAX_LENGTH];

    //int totalTransmitedByte = 0;
    
    
    while(1){

        if(isQueueEmpty()){
            vTaskDelay(100 / portTICK_PERIOD_MS);
            //printf("queue was empty\n");
            continue;
        }

        iteration++;

        printf("Iteration : %d, queue size %d\n", iteration, getQueueSize());


        int elementClusterLength = 0;
        int totalTransmitByteLength = 0;

        for(; elementClusterLength < CLUSTER_MAX_LENGTH; elementClusterLength++){
            if(isQueueEmpty())
                break;
            elementCluster[elementClusterLength] = popFromQueue();
            totalTransmitByteLength += elementCluster[elementClusterLength]->arrayLength;
        }

        int8_t* byteCluster = (int8_t*) malloc(sizeof(int8_t) * totalTransmitByteLength);

        int byteIndex = 0;

        for(int j = 0; j < elementClusterLength; j++){
            for(int k = 0; k < elementCluster[j]->arrayLength; k++){
                byteCluster[byteIndex] = elementCluster[j]->array[k];
                byteIndex++;
            }
            free(elementCluster[j]->array);
            free(elementCluster[j]);
        }

        writeToBinFile(byteCluster, totalTransmitByteLength, fileNameToWriteGlobal);

        free(byteCluster);

    }
}

void startRecordingData(){
    xTaskCreate( recordData, "dataRecorder",  CLUSTER_MAX_LENGTH * 100 , NULL, 3 | portPRIVILEGE_BIT, &xRecorder );
}

void stopRecordingData(){
    vTaskDelete(xRecorder);
}