#ifndef ACCELERATION

#define ACCELERATION


#include "inttypes.h"


typedef struct Acceleration{
    
    int16_t rawLinAccX;
    int16_t rawLinAccY;
    int16_t rawLinAccZ;

    int16_t rawRadAccX;
    int16_t rawRadAccY;
    int16_t rawRadAccZ;

    float linAccX; // unit: m/s^2
    float linAccY;
    float linAccZ;

    float radAccX; // unit: rad/s^2
    float radAccY;
    float radAccZ;

} Acceleration;

#endif