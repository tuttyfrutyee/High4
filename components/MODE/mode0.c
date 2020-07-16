#include "mode0.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>


char * fileNameToWriteGlobal;
TaskHandle_t xRecorder = NULL;




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
        dataElement->array = (int8_t*)getGatherAccelerationsAsArrayInOrder();
        dataElement->arrayLength = gather->numberOfImu * 6 * 2;
        pushToQueue(dataElement);
        //totalBytes += dataElement->arrayLength;
        vTaskDelay(5 / portTICK_PERIOD_MS);

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