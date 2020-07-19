#include "mode2.h"


#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>

TaskHandle_t xController = NULL;

float *** queues;
int * fronts;
int * rears;

float * runningMeans;

int MAXCAPACITY;
int QUEUECOUNT;

void initQueues(int maxCapacity, int queueCount){

    MAXCAPACITY = maxCapacity;
    QUEUECOUNT = queueCount;
    
    queues = (float***) calloc(queueCount, sizeof(float**));
    
    for(int i = 0; i < QUEUECOUNT; i++)
        queues[i] = (float**) calloc(MAXCAPACITY, sizeof(float*));

    fronts = (float*) calloc(queueCount, sizeof(float));  
    rears = (float*) calloc(queueCount, sizeof(float));    

}

void initRunningMeans(int queueCount){
    runningMeans = (float*) calloc(queueCount, sizeof(float));
}

void flashQueuesAndRunningMeans(){

    for(int i = 0; i < QUEUECOUNT; i++){
        rears[i] = 0;
        fronts[i] = 0;
        runningMeans[i] = 0;
    }

}

//queue functions

int isQueueFull(int queueIndex){
    return ((rears[queueIndex]+1) % MAXCAPACITY) == fronts[queueIndex];
}

int isQueueEmpty(int queueIndex){
    return (rears[queueIndex] == fronts[queueIndex]);
}

int getQueueSize(int queueIndex){
    int size = (rears[queueIndex] - fronts[queueIndex]);
    if(size < 0)
        size += MAXCAPACITY;
    return size;
}

int pushToQueue(float* element, int queueIndex){
    
    if(isQueueFull(queueIndex)){
        printf("Error : Queue is full, can not push anymore\n");
        return 0;
    }
    queues[queueIndex][rears[queueIndex]] = element;    
    rears[queueIndex] = (rears[queueIndex] + 1) % MAXCAPACITY;

    return 1;
}

float** popFromQueue(int queueIndex){

    if(isQueueEmpty(queueIndex)){
        printf("Error . Queue is empty, can not pop anymore\n");
        return 0;
    }

    float** willReturn = queues[queueIndex] + fronts[queueIndex];

    fronts[queueIndex] = (fronts[queueIndex] + 1) % MAXCAPACITY;

    return willReturn;

}



void updateWithNewOutcomes(float * newOutcomes){

    float**lastElementBeginning = NULL;

    for(int i = 0; i < QUEUECOUNT; i++){
        if(isQueueEmpty(i)){
            pushToQueue(newOutcomes+i, i);
            runningMeans[i] = newOutcomes[i];

        }
        else if(!isQueueFull(i)){
            const int N = getQueueSize(i);
            pushToQueue(newOutcomes+i, i);
            runningMeans[i] = ( runningMeans[i] * N + newOutcomes[i] ) / (N+1);            
        }
        else{
            const int N = getQueueSize(i);
            float** lastElement = popFromQueue(i);
            if(i == 0)
                lastElementBeginning = lastElement;
            pushToQueue(newOutcomes+i, i);
            runningMeans[i] = runningMeans[i] + (newOutcomes[i] - (**lastElement)) / N;
        }
    }

    if(lastElementBeginning != NULL)
        free(*lastElementBeginning);
}

int8_t getTheWinnerAndUpdate(float threshold){
    
    if(!isQueueFull(0))
        return 0;
    
    int theWinner = 0;

    for(int i = 0; i < QUEUECOUNT; i++)
        if(runningMeans[i] > threshold)
            theWinner = i;
    
    if(theWinner != 0)
        flashQueuesAndRunningMeans();
    
    return theWinner;

}



void startController(int * continueProcessing){
    xTaskCreate(processAndControl, "processAndControl", 4096 * 5, continueProcessing, 10, &xController);    
}



void processAndControl(int * continueProcessing){

    int numberOfPasses = 0;
    bool abortFlag = false;

    float threshold = 0.9;

    dl_matrix3d_t ** workArea =  createForwardPassWorkArea();

    //input preperation

    const int inputSize = refinementLayerweight.w;
    const int hiddenSize = lstmweight_hh_l0.w;
    const int outputSize = layerFinalweight.h;

    dl_matrix3d_t * inputX = (dl_matrix3d_t *)dl_matrix3d_alloc(1, inputSize,  1,  1);   
    dl_matrix3d_t * hidden = (dl_matrix3d_t *)dl_matrix3d_alloc(1, hiddenSize,  1,  1);
    dl_matrix3d_t * cellState = (dl_matrix3d_t *) dl_matrix3d_alloc(1, hiddenSize, 1, 1); 

    fillZerosIntoMatrix3d(hidden);
    fillZerosIntoMatrix3d(cellState);   

    initQueues(20, outputSize);
    initRunningMeans(outputSize);

    while(true){

        if(*continueProcessing && !abortFlag){

            //fetch the sensor data


            getGatherAccelerations();


            float* accelerations = getGatherAccelerationsAsArrayInOrderProcessed();

            fillNumbersIntoMatrix3d(inputX, accelerations);

            

            //feed forward in the network -> output

            forwardPass(inputX, hidden, &cellState, workArea);

            //stack the outputs through running average filter



            float* newOutComes = (float*) calloc(outputSize, sizeof(float));



            for(int i = 0; i < outputSize; i++)
                newOutComes[i] = (float)((workArea[10])->item[i]);            
           

            updateWithNewOutcomes(newOutComes);

            //now pass it through a desicion
            int8_t theWinner = getTheWinnerAndUpdate(threshold);

            printf("theWinner : %" PRIi8 "\n", theWinner);
            if(theWinner != 0)
                printf("\n########################\n");

            //based on the desicion take action

            numberOfPasses++;



        }else{

            if((!abortFlag) && numberOfPasses){
                abortFlag = true;
                (*continueProcessing) = numberOfPasses;
                printMatrix3d(workArea[10]);
            }

            vTaskDelay(10 / portTICK_PERIOD_MS);

        }

    }


}