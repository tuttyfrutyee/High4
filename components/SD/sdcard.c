#include "sdcard.h"

#define PIN_NUM_MISO 17
#define PIN_NUM_MOSI 16
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   18

static const char *TAG = "sdcard";


void initSdCard(){

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();


    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;

    host.max_freq_khz = 1000;


    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };


    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    sdmmc_card_print_info(stdout, card);

}

void writeToSensorDataBytes(int8_t* bytes, int count){
    FILE* f = fopen("/sdcard/high4/sData.bin","a+b");
    if(f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;        
    }
    int successCount = fwrite(bytes, sizeof(int8_t), count, f);
    
    if(successCount != count)
        printf("Error occured while writing sensor data bytes\n");

    fclose(f);
}

void writeToErrorLog(char* error){

    struct stat st;

    if (stat("/sdcard/high4", &st) == -1) {
        mkdir("/sdcard/high4", 0777);
    }

    FILE* f = fopen("/sdcard/high4/errorLog.txt", "a+");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, error);

    fclose(f);
    ESP_LOGI(TAG, "File written");

}

void clearSensorData(){
    remove("/sdcard/high4/sData.bin");
};

void clearErrorLog(){
    remove("/sdcard/high4/errorLog.txt");
};

void unMountSdCard(){
    esp_vfs_fat_sdmmc_unmount();
}

