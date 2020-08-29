#include "mode2.h"


#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>

TaskHandle_t xController = NULL;

#define MONITORQUEUE 1
#define TRANSMIT_MONITOR_QUEUE 1

float *** queues;
int * fronts;
int * rears;
float * runningMeans;
int MAXCAPACITY;
int QUEUECOUNT;

float *** monitorQueues;
int * fronts_monitor;
int * rears_monitor;
int MAXCAPACITY_MONITOR;

int64_t lastDetection = 0;



void initQueues(int maxCapacity, int queueCount){

    MAXCAPACITY = maxCapacity;
    QUEUECOUNT = queueCount;
    
    queues = (float***) calloc(queueCount, sizeof(float**));
    
    for(int i = 0; i < QUEUECOUNT; i++)
        queues[i] = (float**) calloc(MAXCAPACITY, sizeof(float*));

    fronts = (float*) calloc(queueCount, sizeof(float));  
    rears = (float*) calloc(queueCount, sizeof(float));    

}

void initMonitorQueues(int maxCapacity_monitor){
    MAXCAPACITY_MONITOR = maxCapacity_monitor;
    
    monitorQueues = (float***) calloc(QUEUECOUNT, sizeof(float**));
    
    for(int i = 0; i < QUEUECOUNT; i++)
        monitorQueues[i] = (float**) calloc(MAXCAPACITY_MONITOR, sizeof(float*));

    fronts_monitor = (float*) calloc(QUEUECOUNT, sizeof(float));  
    rears_monitor = (float*) calloc(QUEUECOUNT, sizeof(float));        
}

void initRunningMeans(int queueCount){
    runningMeans = (float*) calloc(queueCount, sizeof(float));
}

void flashQueuesAndRunningMeans(){

    for(int i = 0; i < QUEUECOUNT; i++){

        runningMeans[i] = 0;

        if(i == 0 && !MONITORQUEUE){
            for(int j = 0; j < MAXCAPACITY; j++){
                if(rears[i] != j && queues[i][j] != NULL){
                    printf("i : %d, j : %d, value : %f \n", i, j, *queues[i][j]);
                    free( queues[i][j]);               
                    queues[i][j] = NULL;
                }
            }
        }

        rears[i] = 0;
        fronts[i] = 0;
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

//monitor queue functions

int isMonitorQueueFull(int queueIndex){
    return ((rears_monitor[queueIndex]+1) % MAXCAPACITY_MONITOR) == fronts_monitor[queueIndex];
}

int isMonitorQueueEmpty(int queueIndex){
    return (rears_monitor[queueIndex] == fronts_monitor[queueIndex]);
}

int getMonitorQueueSize(int queueIndex){
    int size = (rears_monitor[queueIndex] - fronts_monitor[queueIndex]);
    if(size < 0)
        size += MAXCAPACITY_MONITOR;
    return size;
}

int pushToMonitorQueue(float* element, int queueIndex){
    
    if(isMonitorQueueFull(queueIndex)){
        printf("Error : Queue is full, can not push anymore\n");
        return 0;
    }
    monitorQueues[queueIndex][rears_monitor[queueIndex]] = element;    
    rears_monitor[queueIndex] = (rears_monitor[queueIndex] + 1) % MAXCAPACITY_MONITOR;

    return 1;
}

float** popFromMonitorQueue(int queueIndex){

    if(isMonitorQueueEmpty(queueIndex)){
        printf("Error . Queue is empty, can not pop anymore\n");
        return 0;
    }

    float** willReturn = monitorQueues[queueIndex] + fronts_monitor[queueIndex];

    fronts_monitor[queueIndex] = (fronts_monitor[queueIndex] + 1) % MAXCAPACITY_MONITOR;

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

    if(!MONITORQUEUE)
        if(lastElementBeginning != NULL)
            free(*lastElementBeginning);

}

void pushNewOutComes_toMonitorQueues(float * newOutcomes){

    float**lastElementBeginning = NULL;

    for(int i = 0; i < QUEUECOUNT; i++){

        if(isMonitorQueueEmpty(i)){
            pushToMonitorQueue(newOutcomes+i, i);

        }
        else if(!isMonitorQueueFull(i)){
            pushToMonitorQueue(newOutcomes+i, i);
        }
        else{
            float** lastElement = popFromMonitorQueue(i);
            if(i == 0)
                lastElementBeginning = lastElement;
            pushToMonitorQueue(newOutcomes+i, i);
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

void transmitMonitorQueues(){

    printf("Start to trasmit queues\n");
    
    int16_t totalTransmissionInOneQueue = (rears_monitor[0] - fronts_monitor[0]);
    if(totalTransmissionInOneQueue < 0)
        totalTransmissionInOneQueue += MAXCAPACITY_MONITOR;    



    //first transmit the number of packages to transmit
    pushDataToMonitor(&totalTransmissionInOneQueue, 2);

    //secondly transmit the number of the elements in one package
    char queueCount = QUEUECOUNT;
    pushDataToMonitor(&queueCount, 1);
    //now send all of it
    for(int i = 0; i < totalTransmissionInOneQueue; i++){
        float * willSend = (float*) calloc(QUEUECOUNT, sizeof(float));
        for(int j = 0; j < QUEUECOUNT; j++)
            willSend[j] = *monitorQueues[j][((fronts_monitor[j] + i) % MAXCAPACITY_MONITOR)];

        pushDataToMonitor(willSend, 4 * QUEUECOUNT);
        free(willSend);
    }
}



void startController(int * continueProcessing){
    xTaskCreate(processAndControl, "processAndControl", 4096 * 5, continueProcessing, 10, &xController);    
}



void processAndControl(int * continueProcessing){

    int numberOfPasses = 0;
    bool sessionStarted = false;

    int64_t timerStart = 0;    

    float threshold = 0.7;

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

    initQueues(30, outputSize);
    if(MONITORQUEUE)
        initMonitorQueues(300);

    initRunningMeans(outputSize);

    while(true){

        

        if(*continueProcessing){


            if(!sessionStarted){
                sessionStarted = true;
                numberOfPasses = 0;
                timerStart = esp_timer_get_time();                
            }


            //fetch the sensor data


            getGatherAccelerations();

            int imuIndexes[] = {0,1,2};
            float* accelerations = getGatherAccelerationsAsArrayInOrderProcessed(imuIndexes,3);

            fillNumbersIntoMatrix3d(inputX, accelerations);

            

            //feed forward in the network -> output

            forwardPass(inputX, hidden, &cellState, workArea);

            //stack the outputs through running average filter

            free(accelerations);

            float* newOutComes = (float*) calloc(outputSize, sizeof(float));



            for(int i = 0; i < outputSize; i++)
                newOutComes[i] = (float)((workArea[10])->item[i]);            
           

            updateWithNewOutcomes(newOutComes);
            if(MONITORQUEUE)
                pushNewOutComes_toMonitorQueues(newOutComes);


            //now pass it through a desicion
            int8_t theWinner = getTheWinnerAndUpdate(threshold);

            if(theWinner != 0){

                if((getTime() - lastDetection) > 1000){

                    printf("theWinner : %" PRIi8 "\n", theWinner);
                    printf("\n########################\n");
                    pushGestureToNotification(theWinner);

                    fillZerosIntoMatrix3d(hidden);
                    fillZerosIntoMatrix3d(cellState); 

                }

                lastDetection = getTime();

                  

            }

            //based on the desicion take action

            numberOfPasses++;



        }else{

            if(sessionStarted){
                sessionStarted = false;
                printMatrix3d(workArea[10]);
                printf("Fps is : %f\n", numberOfPasses / ((esp_timer_get_time() - timerStart) / 1000000.0));

                if(MONITORQUEUE && TRANSMIT_MONITOR_QUEUE)
                    transmitMonitorQueues();                

            }

            vTaskDelay(10 / portTICK_PERIOD_MS);

        }

    }


}