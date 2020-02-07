#include "helper.h"

void physical_alarmError(){
    buzz(2000);
    lightOnBlueLed();
}

void physical_standby_start(int64_t duration){ //in ms
    int64_t startTime = getTime();

    int64_t  currentTime;

   currentTime = startTime;    

    int toggle = 0;

    while((currentTime - startTime) < duration){

        if(toggle) lightOnBlueLed();
        else lightOffBlueLed();

        vTaskDelay(100 / portTICK_RATE_MS);

        toggle = !toggle;
        currentTime = getTime();
    }
}

void physical_clear(){
    lightOffBlueLed();
}

void physical_alarmShort();

void physical_alarmMedium();