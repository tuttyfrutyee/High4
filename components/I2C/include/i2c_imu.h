#ifndef I2CIMU

#define I2CIMU


#include "inttypes.h"


#define sSDefaultAddr 0x68 //the defaultAddr i2c will talk to. Should be changed syncronously with flagActive

int writeFunctionIMU(uint8_t registerAddr, uint8_t* valueArray, uint8_t length);

int readFunctionIMU(uint8_t registerAddr, uint8_t* storeArray, uint8_t length);

#endif