
#include "i2c_imu.h"
#include "i2c.h"


int writeFunctionIMU(uint8_t registerAddr, uint8_t* valueArray, uint8_t length){
    return user_i2c_write(sSDefaultAddr, registerAddr, valueArray, length);
}

int readFunctionIMU(uint8_t registerAddr, uint8_t* storeArray, uint8_t length){
    return user_i2c_read(sSDefaultAddr, registerAddr, storeArray, length);
}