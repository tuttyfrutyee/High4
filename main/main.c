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
/* 
    nvs_flash_init();
    wifi_init();
    mqtt_app_start();
 */

    int ret;

    IMUGATHER gather = {4,3,0,3000};

    ret = i2c_master_init(300000);

    initSdCard();

    printf("hello world\n");

    startRecordingData();

    int16_t* dataArray = NULL;

    clearSensorData();
    clearErrorLog();


    if(ret == ESP_OK){
        printf("i2c is init well\n");

        initIMUGATHERSensors(&gather);

        selfTestSensors(&gather);


        goCollectCurrentModeData(&gather);

    }
    else{
        printf("something went wrong with i2c init\n");
    }


    printf("all done\n");


}
