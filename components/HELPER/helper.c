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