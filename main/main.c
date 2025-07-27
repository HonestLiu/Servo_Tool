/**
 * @file main.c
 * @brief Servo Tool Main Application
 * @author honestliu
 * @date 2025-07-26
 * @version 1.0
 * 
 * @date 2025-07-27
 * @version 1.1 增加UI
 * @version 1.2 进一步解耦合
 */

#include <stdio.h>
#include "demos/lv_demos.h"
#include "esp_log.h"
#include "esp_err.h"

#include "main_update.h"

static const char *TAG = "main";



void app_main(void) {
   ESP_LOGI(TAG, "Starting main application...");

    // 创建硬件初始化任务
    xTaskCreate(
        hardware_init_task,   ///< 任务函数
        "hardware_init",      ///< 任务名称
        4096,                 ///< 堆栈大小
        NULL,                 ///< 任务参数
        5,                    ///< 任务优先级
        NULL                  ///< 任务句柄
    );

}