#include "math.h"
#include "acceleration.h"
#include "mpu6050.h"
#include "i2c.h"
#include "i2c_imu.h"

//Helper functions
Acceleration getMeanAccelerations(int numOfSamples);
int setTestConfigurationOnSensor_withoutActivation();
int setTestConfigurationOnSensor_withActivation();
Acceleration getFactorTrimValues();
void printAccelerations(Acceleration accelerations);
void printRawAccelerations(Acceleration accelerations);

//Constants
int linRange = 8; // range is +-8g
int radRange = 1; // range is 1 * 250 degrees/second

uint16_t linRangeBox[17] = {0,0,0x00,0,0x08,0,0,0,0x10,0,0,0,0,0,0,0,0x18};
uint16_t radRangeBox[9] = {0,0,0x08,0,0x10,0,0,0,0x18};


float g = 9.807; // m/s^2
float pi = 3.14159265359;

int16_t signedByteConcatenater(uint8_t byteH, uint8_t byteL){
    return ( ((uint16_t)byteH) << 8  ) | ( (uint16_t)byteL );
}

void printAccelerationData(MPU6050 device){
    printAccelerations(device.acc);
}

void printAccelerationRawData(MPU6050 device){
    printRawAccelerations(device.acc);
}


int setConfigurations(){    

        uint8_t control = 0;
        uint8_t linearRangeSettings = linRangeBox[linRange];//0x18; // set range to +-16g
        uint8_t radialRangeSettings = radRangeBox[radRange];
        uint8_t filterSetting = 4; // set filter to 6
        uint8_t sampleDivider = 3; // divide 1khz / (3 + 1) = 250 hz

        int ret = 1;

        ret = ret && checkI2cError(writeFunctionIMU(0x6b, &control, 1), "writeControlRegister");

        ret = ret && checkI2cError(writeFunctionIMU(0x1a, &filterSetting, 1), "writeFilterSetting"); //set filter to 6

        ret = ret && checkI2cError(writeFunctionIMU(0x1c, &linearRangeSettings, 1), "writeLinearRangeSettings"); //set linear acc range to 16g

        ret = ret && checkI2cError(writeFunctionIMU(0x1b, &radialRangeSettings, 1), "writeRadialRangeSettings");

        ret = ret && checkI2cError(writeFunctionIMU(0x19, &sampleDivider, 1), "writeSampleDivider");

        return ret;
}

Acceleration getAccelerations(){
    uint8_t sensorDatas[14] = {0};
    Acceleration accelerations = {};

    checkI2cError(readFunctionIMU(0x3b, sensorDatas, 14),"Read Acceleration");

    accelerations.rawLinAccX = signedByteConcatenater(sensorDatas[0], sensorDatas[1]);
    accelerations.rawLinAccY = signedByteConcatenater(sensorDatas[2], sensorDatas[3]);
    accelerations.rawLinAccZ = signedByteConcatenater(sensorDatas[4], sensorDatas[5]);
    // sensorDatas[6 and 7] are temperature measurement
    accelerations.rawRadAccX = signedByteConcatenater(sensorDatas[8], sensorDatas[9]);
    accelerations.rawRadAccY = signedByteConcatenater(sensorDatas[10], sensorDatas[11]);
    accelerations.rawRadAccZ = signedByteConcatenater(sensorDatas[12], sensorDatas[13]);

    //convert raws
    accelerations.linAccX = accelerations.rawLinAccX / (65536.0 / 2.) * linRange * g;
    accelerations.linAccY = accelerations.rawLinAccY / (65536.0 / 2.) * linRange * g;
    accelerations.linAccZ = accelerations.rawLinAccZ / (65536.0 / 2.) * linRange * g;

    accelerations.radAccX = accelerations.rawRadAccX / (65536.0 / 2.) * radRange * 250 ;
    accelerations.radAccY = accelerations.rawRadAccY / (65536.0 / 2.) * radRange * 250 ;
    accelerations.radAccZ = accelerations.rawRadAccZ / (65536.0 / 2.) * radRange * 250 ;

    return accelerations;

}

void printAccelerations(Acceleration accelerations){
    printf("\n****************\n");
    printf("linAccX : %f m/s^2\n", accelerations.linAccX);
    printf("linAccY : %f m/s^2\n", accelerations.linAccY);
    printf("linAccZ : %f m/s^2\n", accelerations.linAccZ);

    printf("radAccX : %f rad/s\n", accelerations.radAccX);
    printf("radAccY : %f rad/s\n", accelerations.radAccY);
    printf("radAccZ : %f rad/s\n", accelerations.radAccZ); 
}

void printRawAccelerations(Acceleration accelerations){
    printf("\n****************\n");
    printf("linAccX : %" PRId16 "m/s^2\n", accelerations.rawLinAccX);
    printf("linAccY : %" PRId16 "m/s^2\n", accelerations.rawLinAccY);
    printf("linAccZ : %" PRId16 "m/s^2\n", accelerations.rawLinAccZ);

    printf("radAccX : %" PRId16 "rad/s\n", accelerations.rawRadAccX);
    printf("radAccY : %" PRId16 "rad/s\n", accelerations.rawRadAccY);
    printf("radAccZ : %" PRId16 "rad/s\n", accelerations.rawRadAccZ); 
}

int selfTest(){

    float changeFromFactorTrimExtreme = 14.0; //  | trim | <= changeFromFactorTrimExtreme to be healthy
    int numOfSamples = 1000;

    float errLinAccX, errLinAccY, errLinAccZ, errRadAccX, errRadAccY, errRadAccZ;

    Acceleration meanAccelerationsInActive, meanAccelerationsActive, meanAccelerationsSelfTest;

    //first set the test configurations of the sensor as given in the documentation without activating selftest

    setTestConfigurationOnSensor_withoutActivation();

    //get average values from sensor by sampling without any movement for given numSamples
    meanAccelerationsInActive = getMeanAccelerations(numOfSamples);


    //secondly set the test configurations of the sensor as given in the documentation and activate selftest

    setTestConfigurationOnSensor_withActivation();

    //get average values from sensor by sampling without any movement for given numSamples
    meanAccelerationsActive = getMeanAccelerations(numOfSamples);

    //get the difference between active and inactive mean to calculate meanAccSelfTest
    meanAccelerationsSelfTest.linAccX = meanAccelerationsActive.linAccX - meanAccelerationsInActive.linAccX;
    meanAccelerationsSelfTest.linAccY = meanAccelerationsActive.linAccY - meanAccelerationsInActive.linAccY;
    meanAccelerationsSelfTest.linAccZ = meanAccelerationsActive.linAccZ - meanAccelerationsInActive.linAccZ;
    
    meanAccelerationsSelfTest.radAccX = meanAccelerationsActive.radAccX - meanAccelerationsInActive.radAccX;
    meanAccelerationsSelfTest.radAccY = meanAccelerationsActive.radAccY - meanAccelerationsInActive.radAccY;
    meanAccelerationsSelfTest.radAccZ = meanAccelerationsActive.radAccZ - meanAccelerationsInActive.radAccZ;


    //now get factor trim values to compare self test

    Acceleration factorTrimValues = getFactorTrimValues();

//  printAccelerations(factorTrimValues);   

    //get the relative percentage difference between factorTrimValues and selfTest
    errLinAccX = (meanAccelerationsSelfTest.linAccX - factorTrimValues.linAccX) / factorTrimValues.linAccX * 100;
    errLinAccY = (meanAccelerationsSelfTest.linAccY - factorTrimValues.linAccY) / factorTrimValues.linAccY * 100;
    errLinAccZ = (meanAccelerationsSelfTest.linAccZ - factorTrimValues.linAccZ) / factorTrimValues.linAccZ * 100;

    errRadAccX = (meanAccelerationsSelfTest.radAccX - factorTrimValues.radAccX) / factorTrimValues.radAccX * 100;
    errRadAccY = (meanAccelerationsSelfTest.radAccY - factorTrimValues.radAccY) / factorTrimValues.radAccY * 100;
    errRadAccZ = (meanAccelerationsSelfTest.radAccZ - factorTrimValues.radAccZ) / factorTrimValues.radAccZ * 100;

    printf("\n\nLogging selftest errors\n");

    printf("errLinAccX : %0.2f %%\n",errLinAccX);
    printf("errLinAccY : %0.2f %%\n",errLinAccY);
    printf("errLinAccZ : %0.2f %%\n",errLinAccZ);

    printf("errRadAccX : %0.2f %%\n",errRadAccX);
    printf("errRadAccY : %0.2f %%\n",errRadAccY);
    printf("errRadAccZ : %0.2f %%\n",errRadAccZ);

    //make sure inactivated selftest
    setConfigurations();

    return 0;

}

Acceleration getMeanAccelerations(int numOfSamples){
    Acceleration meanAccelerations = {};
    Acceleration temp = {};

    for(int i = 0; i < numOfSamples; i++){

        temp = getAccelerations();

        meanAccelerations.linAccX += temp.linAccX;
        meanAccelerations.linAccY += temp.linAccY;
        meanAccelerations.linAccZ += temp.linAccZ;

        meanAccelerations.radAccX += temp.radAccX;
        meanAccelerations.radAccY += temp.radAccY;
        meanAccelerations.radAccZ += temp.radAccZ;

    }

    meanAccelerations.linAccX /= numOfSamples;
    meanAccelerations.linAccY /= numOfSamples;
    meanAccelerations.linAccZ /= numOfSamples;

    meanAccelerations.radAccX /= numOfSamples;
    meanAccelerations.radAccY /= numOfSamples;
    meanAccelerations.radAccZ /= numOfSamples;

    return meanAccelerations;
}

Acceleration getFactorTrimValues(){

    uint8_t factoryValues[4] = {0};
    Acceleration accelerations = {};
    int xa_test, ya_test, za_test, xg_test, yg_test, zg_test;
    int ftxa, ftya, ftza, ftxg, ftyg, ftzg;


    checkI2cError(readFunctionIMU(0x0D, factoryValues, 4),"Read Factor Values For Self Test");

    xg_test = factoryValues[0] & 0x1f;
    yg_test = factoryValues[1] & 0x1f;
    zg_test = factoryValues[2] & 0x1f;

    xa_test = ((factoryValues[0] & 0xe0) >> 3) | ((factoryValues[3] & 0x30) >> 4);
    ya_test = ((factoryValues[1] & 0xe0) >> 3) | ((factoryValues[3] & 0xc) >> 2);
    za_test = ((factoryValues[2] & 0xe0) >> 3) | ((factoryValues[3] & 0x3));

    ftxg = 25 * 131 * pow(1.046, xg_test - 1);
    ftyg = -25 * 131 * pow(1.046, yg_test - 1);
    ftzg = 25 * 131 * pow(1.046, zg_test - 1);

    ftxa = 4096 * 0.34 * pow(0.92/0.34, (xa_test - 1) / 30.0);
    ftya = 4096 * 0.34 * pow(0.92/0.34, (ya_test - 1) / 30.0);
    ftza = 4096 * 0.34 * pow(0.92/0.34, (za_test - 1) / 30.0);

    accelerations.linAccX = ftxa / (65536.0 / 2.) * 8 * g;
    accelerations.linAccY = ftya / (65536.0 / 2.) * 8 * g;
    accelerations.linAccZ = ftza / (65536.0 / 2.) * 8 * g;

    accelerations.radAccX = ftxg / (65536.0 / 2.) * 1 * 250 / 360.0 * 2 * pi;
    accelerations.radAccY = ftyg / (65536.0 / 2.) * 1 * 250 / 360.0 * 2 * pi;
    accelerations.radAccZ = ftzg / (65536.0 / 2.) * 1 * 250 / 360.0 * 2 * pi;

    return accelerations;

}


int setTestConfigurationOnSensor_withoutActivation(){

        uint8_t linearRangeSettings = linRangeBox[8];//set range to +-8g as specified in datasheet
        uint8_t radialRangeSettings = radRangeBox[1]; //set range to 250dps as specified in datasheet

        int ret = 1;

        ret = ret && checkI2cError(writeFunctionIMU(0x1c, &linearRangeSettings, 1), "writeLinearRangeSettings"); //set linear acc range to 16g

        ret = ret && checkI2cError(writeFunctionIMU(0x1b, &radialRangeSettings, 1), "writeRadialRangeSettings");

        return ret;    
}

int setTestConfigurationOnSensor_withActivation(){

        uint8_t linearRangeSettings = linRangeBox[8] | (0xe0) ;//set range to +-8g as specified in datasheet
        uint8_t radialRangeSettings = radRangeBox[1] | (0xe0) ; //set range to 250dps as specified in datasheet

        int ret = 1;

        ret = ret && checkI2cError(writeFunctionIMU(0x1c, &linearRangeSettings, 1), "writeLinearRangeSettings"); //set linear acc range to 16g

        ret = ret && checkI2cError(writeFunctionIMU(0x1b, &radialRangeSettings, 1), "writeRadialRangeSettings");

        return ret;   
}