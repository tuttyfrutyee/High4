#ifndef SDCARD

#define SDCARD


#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include "acceleration.h"



void initSdCard();

void writeToSensorDataArray(int16_t* dataArray);
void writeToSensorDataBytes(int8_t* bytes, int count);

void writeToErrorLog(char* error);

void clearSensorData();
void clearErrorLog();

void unMountSdCard();

#endif