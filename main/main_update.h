#ifndef MAIN_UPDATE_H
#define MAIN_UPDATE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void gui_task(void *pvParameter);

void main_logic_task(void *pvParameter);

void hardware_init_task(void *pvParameters);

#endif // MAIN_UPDATE_H