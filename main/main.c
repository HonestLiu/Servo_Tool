/**
 * @file main.c
 * @brief Servo Tool Main Application
 * @author honestliu
 * @date 2025-07-26
 * @version 1.0
 * 
 * @date 2025-07-26
 * @version 1.1 增加UI
 */

#include <stdio.h>
#include "demos/lv_demos.h"
#include "lcd.h"
#include "lvgl-components.h"
#include "ui.h"

#include "servo_tool.h"

#include "main_update.h"
#include "ui_interface.h"

#include "ui_command.h"

static const char *TAG = "main";

uint8_t result;

/**
 * @defgroup HARDWARE_HANDLES 硬件句柄定义
 * @brief 定义LCD相关的硬件句柄
 * @{
 */
static esp_lcd_panel_io_handle_t io_handle = NULL;    ///< LCD IO句柄，用于LCD通信
static esp_lcd_panel_handle_t panel_handle = NULL;    ///< LCD面板句柄，用于LCD控制
/** @} */

static TaskHandle_t gui_task_handle = NULL;           ///< GUI任务句柄
static TaskHandle_t main_logic_task_handle = NULL;    ///< 主逻辑任务句柄


logic_to_ui_msg_t send_msg; // 发送到UI层的数据

void hardware_init_task(void *pvParameters) {
     // 硬件初始化
    bsp_i2c_init();                                    ///< 初始化I2C接口
    pca9557_init();                                    ///< 初始化PCA9557 IO扩展芯片
    servo_init_result_t servo_res = servo_tool_init();                                  
    bsp_lvgl_start(&io_handle, &panel_handle);         ///< 启动LVGL显示系统

    ui_init();

    task_command_init(); // 初始化任务间通信模块

            // 创建GUI任务（高优先级，保证界面响应性）
    xTaskCreate(
        gui_task,           // 任务函数
        "GUI_Task",         // 任务名称
        8192,               // 栈大小（GUI可能需要更大的栈）
        NULL,               // 参数
        6,                  // 高优先级
        &gui_task_handle    // 任务句柄
    );
    
    // 创建主逻辑任务（中等优先级）
    xTaskCreate(
        main_logic_task,         // 任务函数
        "Main_Logic_Task",       // 任务名称
        4096,                    // 栈大小
        NULL,                    // 参数
        4,                       // 中等优先级
        &main_logic_task_handle  // 任务句柄
    );

    ESP_LOGI(TAG,"所有任务创建完毕");

    // 舵机初始化成功，更新初始化数据到UI
    if (servo_res.init_state)
    {
        send_msg.type = SERVO_INITIALIZED;
                    send_msg.init_angle = servo_res.init_angle;
                    send_msg.servo_pin = servo_res.servo_pin;
        send_logic_message(&send_msg);
    }

    vTaskDelete(NULL); // 删除当前任务
}


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