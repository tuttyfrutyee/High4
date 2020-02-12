#include "buzzer.h"
#include "button.h"

//Structs

typedef struct QueueButtonEl {

    int64_t occTime;
    char type; // 0 : low, 1 : high

} QueueButtonEl;

//functions

void physical_alarmError();

void physical_standby_start();

void physical_standby_stop();

void physical_alarmShort();

void physical_alarmMedium();

void flashBlueLight();

bool checkButtonPressHoldMedium();

bool checkButtonPressHoldLong();

//detect button press types

bool detectDoubleTap(QueueButtonEl* stack, int stackSize);
bool detectSingleTap(QueueButtonEl* stack, int stackSize);
bool detectHoldMedium(QueueButtonEl* stack, int stackSize);
bool detectHoldLong(QueueButtonEl* stack, int stackSize);

