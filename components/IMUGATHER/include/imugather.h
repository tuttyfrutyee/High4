#ifndef IMUGATHER_

#define IMUGATHER_



typedef struct IMUGATHER {

    int numberOfImu;
    int currentModeIndicator;
    int criticalTime; //in milliseconds
    int dataCollectDuration; //in milliseconds

} IMUGATHER;


//These functions will effect the selected sensorImu

void printAccelerationDatas(IMUGATHER* gather);
void printAccelerationRawDatas(IMUGATHER* gather);

//Get Acc Datas by sweeping selected sensorImu and communicating
void getGatherAccelerations();

//Will use the given writeFunction to make the configurations
int initIMUGATHERSensors(IMUGATHER* gather);

//Will get all accelerations in order : [gather1accData(consistsof6int16_t array), gather2accData(...) ...]
int16_t* getGatherAccelerationsAsArrayInOrder(IMUGATHER* gather);

int selfTestSensors();

void goCollectCurrentModeData(IMUGATHER* gather, char* fileNameToWrite);

void startRecordingData();

void stopRecordingData();


#endif