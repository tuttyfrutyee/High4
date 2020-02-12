#include "button.h"
#include <time.h>
#include "esp_timer.h"


#define BUTTONPIN 21
#define BLUELED 2

static xQueueHandle * button_evt_queue;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t pin = 21;

    xQueueSendFromISR(*button_evt_queue, &(pin), NULL);
}

void initButtonGpio(){

    //init button gpio

    gpio_config_t io_conf;    
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL << BUTTONPIN;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);


    //init blue led gpio
    
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL << BLUELED;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf); 

}

void setUpButtonIsr(xQueueHandle* button_evt_queue_){

    button_evt_queue = button_evt_queue_;

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTONPIN, gpio_isr_handler, 0);

}

void lightOnBlueLed(){
    gpio_set_level(BLUELED, 1);
}

void lightOffBlueLed(){
    gpio_set_level(BLUELED, 0);
}

int64_t getTime(){
    return (int64_t) (clock() * 1000 / CLOCKS_PER_SEC ); //returns time in milliseconds
}
