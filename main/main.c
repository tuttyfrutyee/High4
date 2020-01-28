#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "i2c.h"
#include "esp_timer.h"
#include "stdio.h"

#include "acceleration.h"
#include "imugather.h"
#include "sdcard.h"

#include "mqtt.h"



static const char *TAG = "mpu6050";


//vTaskDelay(300 / portTICK_PERIOD_MS);
void app_main(void)
{

/*     ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
 

     ESP_ERROR_CHECK(example_connect());

    mqtt_app_start(); */


    int ret;

    IMUGATHER gather = {1,3,0,300};

    ret = i2c_master_init(300000);

    initSdCard();

    startRecordingData();

    int16_t* dataArray = NULL;

    clearSensorData();
    clearErrorLog();


    if(ret == ESP_OK){
        printf("i2c is init well\n");

        initIMUGATHERSensors(&gather);

        selfTestSensors(&gather);

/*         for(int i=0; i < 3; i++){

            

            getGatherAccelerations();

            //printAccelerationDatas(&gather);

            printAccelerationRawDatas(&gather);

            dataArray = getGatherAccelerationsAsArrayInOrder(&gather);
            writeToSensorDataBytes(dataArray,12);

            vTaskDelay(50 / portTICK_PERIOD_MS);
        } */

        goCollectCurrentModeData(&gather);

    }
    else{
        printf("something went wrong with i2c init\n");
    }


    printf("all done\n");


}
