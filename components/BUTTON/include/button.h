#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


void initButtonGpio();
void setUpButtonIsr(xQueueHandle* button_evt_queue_);
void lightOnBlueLed();
void lightOffBlueLed();
int64_t getTime();