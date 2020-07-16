
#include "mode1.h"


#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include <time.h>


TaskHandle_t xStreamer = NULL;



void startStreamer(int* keepStreaming){
    xTaskCreate(streamData, "streamData", 4096 * 3, keepStreaming, 10, &xStreamer);
}


void streamData(int* continueS){

    const int fps = 60;
    float delayAmount = 1000. / fps;

    while(true){

        if(*continueS){
            getGatherAccelerations();

            char* data = (char*) getGatherAccelerationsAsArrayInOrder();

            pushDataToStream(data, gather->numberOfImu * 6 * 2);

            vTaskDelay(delayAmount / portTICK_PERIOD_MS);
        }else{
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }

    }


}