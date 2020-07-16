#ifndef IMUGATHER_

#define IMUGATHER_


#include "esp_system.h"

#include "acceleration.h"
#include "imugather.h"
#include "mpu6050.h"
#include "helper.h"



typedef struct IMUGATHER {

    int numberOfImu;
    int currentModeIndicator;
    int criticalTime; //in milliseconds
    int dataCollectDuration; //in milliseconds

} IMUGATHER;

typedef struct IMU{

    int assignedPin;
    MPU6050 imu;

} IMU;

extern IMU* imuStack;
extern IMUGATHER* gather;
extern IMU* previousSelectedImu;




//These functions will effect the selected sensorImu

void printAccelerationDatas(IMUGATHER* gather);
void printAccelerationRawDatas(IMUGATHER* gather);

//Get Acc Datas by sweeping selected sensorImu and communicating
void getGatherAccelerations();

//Will use the given writeFunction to make the configurations
int initIMUGATHERSensors(IMUGATHER* gather);

//Will get all accelerations in order : [gather1accData(consistsof6int16_t array), gather2accData(...) ...]
int16_t* getGatherAccelerationsAsArrayInOrder();

int selfTestSensors();

void goCollectCurrentModeData(IMUGATHER* gather, char* fileNameToWrite);

void startRecordingData();

void stopRecordingData();

void startStreamer(int* keepStreaming);

#endif