#include "buzzer.h"

#define BUZZERPIN 15

void initBuzzGpio(){

    //init buzzer gpio
    gpio_config_t io_conf;        
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL << BUZZERPIN;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf); 

}


void buzz(int64_t duration){ //duration in ms
    gpio_set_level(BUZZERPIN, 1);
    vTaskDelay(duration / portTICK_RATE_MS);
    gpio_set_level(BUZZERPIN, 0);
}

