#include "helper.h"

void physical_alarmError(){
    buzz(2000);
    lightOnBlueLed();
}

static TaskHandle_t standbyTaskHandle;
int flagStandby = 0;

void standby(){

    int toggle = 0;

    while(true){

        if(toggle) lightOnBlueLed();
        else lightOffBlueLed();

        vTaskDelay(75 / portTICK_RATE_MS);

        toggle = !toggle;
    }
}

void physical_standby_start(){ //in ms
    
    if(flagStandby) return;

    flagStandby = 1;

    xTaskCreate(standby, "standby", 4096, NULL, 10, &standbyTaskHandle);

}

void physical_standby_stop(){
    if(!(flagStandby)) return;

    flagStandby = 0;

    vTaskDelete( standbyTaskHandle );

    lightOffBlueLed();
}

void physical_clear(){
    lightOffBlueLed();
}

void flashBlueLight(){
    lightOnBlueLed();
    vTaskDelay(200 / portTICK_RATE_MS);    
    lightOffBlueLed();
}

void physical_alarmShort();

void physical_alarmMedium();

// tap detection

#define SingleTapDifLow 200 //in ms
#define SingleTapDifHigh 300 //in ms

#define DoubleTapDifLow 200 //in ms
#define DoubleTapDifHigh 400 //in ms

#define HoldMediumDurLow 1500 //in ms
#define HoldMediumDurHigh 2000 //in ms

#define HoldLongDurLow 2500 //in ms
#define HoldLongDurHigh 3500 //in ms



bool detectSingleTap(QueueButtonEl* stack, int stackSize){
    
    if(stackSize < 2) return false;
    
    QueueButtonEl second = stack[stackSize - 1];
    QueueButtonEl first = stack[stackSize - 2];

    if(second.type == 0 && second.type == 1){
        if(((second.occTime - first.occTime) > SingleTapDifLow) && ((second.occTime - first.occTime) < SingleTapDifHigh))
            return true;
    }

    return false;

}

bool detectDoubleTap(QueueButtonEl* stack, int stackSize){

    if(stackSize < 4) return false;

    bool firstTap = detectSingleTap(stack - 2, stackSize - 2);
    bool secondTap = detectSingleTap(stack, stackSize);

    if(firstTap && secondTap){
        QueueButtonEl firstEnd = stack[stackSize - 3];
        QueueButtonEl secondStart = stack[stackSize - 2];

        int timeBetween = secondStart.occTime - firstEnd.occTime;
        if((timeBetween > DoubleTapDifLow) && (timeBetween < DoubleTapDifHigh))
            return true;
    }

    return false;
}

bool detectHoldMedium(QueueButtonEl* stack, int stackSize){
    if(stackSize < 2) return false;
    
    QueueButtonEl second = stack[stackSize - 1];
    QueueButtonEl first = stack[stackSize - 2];

    if(second.type == 0 && second.type == 1){
        if(((second.occTime - first.occTime) > HoldMediumDurLow) && ((second.occTime - first.occTime) < HoldMediumDurHigh))
            return true;
    }

    return false;
}

bool detectHoldLong(QueueButtonEl* stack, int stackSize){
    if(stackSize < 2) return false;
    
    QueueButtonEl second = stack[stackSize - 1];
    QueueButtonEl first = stack[stackSize - 2];

    if(second.type == 0 && second.type == 1){
        if(((second.occTime - first.occTime) > HoldLongDurLow) && ((second.occTime - first.occTime) < HoldLongDurHigh))
            return true;
    }

    return false;
}
