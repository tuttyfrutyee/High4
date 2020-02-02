#ifndef MPU6050_

#define MPU6050_

//Global Constants
extern int linRange; 
extern int radRange; 

//hardcoded register values
extern uint16_t linRangeBox[17];
extern uint16_t radRangeBox[9];

typedef struct MPU6050 {
    Acceleration acc;
} MPU6050;

//These functions will effect the selected sensorImu

void printAccelerationData(MPU6050 device);

void printAccelerationRawData(MPU6050 device);


//Will use the given writeFunction to make the configurations
int setConfigurations();

//Will get all accelerations
Acceleration getAccelerations();

//Will make self test on sensors
int selfTest();

#endif