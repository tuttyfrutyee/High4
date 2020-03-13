#ifndef SDCARD

#define SDCARD


#include <stdio.h>
#include <string.h>
#include "stdlib.h"
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


void writeToFile(int8_t* bytes, int count, char* fileName);
void writeToBinFile(int8_t* bytes, int count, char* fileName);
char* readFile(int count, const char* fileName, FILE** f_);

void getAndUpdateLookup(int* recordCounter_, int* currentModeCounter_);

void deleteLastSensorFile();
void clearSensorData();
void clearErrorLog();

void unMountSdCard();

#endif