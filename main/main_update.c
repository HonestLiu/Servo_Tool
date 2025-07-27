#include "main_update.h"
#include "esp_log.h"
#include "servo_tool.h"
#include "ui_command.h"
#include "ui_interface.h"
#include <stdbool.h>
#include "ui.h"
#include "lvgl.h"
#include "lcd.h"
#include "lvgl-components.h"
#include "esp_err.h"


static const char *TAG = "Main Update";

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

/**
 * @brief 更新舵机初始化状态到UI
 * @param init_angle 初始化角度
 * @param servo_pin 舵机引脚号
 */
static void update_servo_initialization_ui(int init_angle, int servo_pin) {
    // 更新默认角度到UI
    lv_label_set_text_fmt(ui_angleValue, "%d °", init_angle);
    lv_slider_set_value(ui_angleSlider, init_angle, LV_ANIM_ON);
    // 更新舵机引脚到UI
    lv_label_set_text_fmt(ui_ServoPin, "%d", servo_pin);
    ESP_LOGI(TAG, "Servo initialized: angle=%d °, pin=%d", init_angle, servo_pin);
}

/**
 * @brief 更新舵机角度到UI
 * @param angle 新的角度值
 */
static void update_servo_angle_ui(int angle) {
    // 更新UI显示当前舵机角度
    lv_label_set_text_fmt(ui_angleValue, "%d °", angle);
    ESP_LOGI(TAG, "Servo angle updated in UI: %d °", angle);
}

/**
 * @brief 处理舵机角度设置请求
 * @param angle 目标角度
 * @return true 设置成功, false 设置失败
 */
static bool handle_servo_angle_request(int angle) {
    bool ret = servo_tool_set_angle(angle);
    
    if (ret) {
        ESP_LOGI(TAG, "Servo angle set successfully: %d °", angle);

        // 发送成功消息到UI
        logic_to_ui_msg_t send_msg;
        send_msg.type = LOGIC_MSG_SERVO_ANGLE_SET;
        send_msg.angle = angle;
        send_logic_message(&send_msg);
    } else {
        ESP_LOGE(TAG, "Failed to set servo angle: %d °", angle);
    }
    
    return ret;
}

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
    if (servo_res.init_state) {
        send_msg.type = SERVO_INITIALIZED;
        send_msg.init_angle = servo_res.init_angle;
        send_msg.servo_pin = servo_res.servo_pin;
        send_logic_message(&send_msg);
    }

    vTaskDelete(NULL); // 删除当前任务
}

/**
 * @brief GUI任务，用于处理UI事件和更新
 * @note 在logic_task中发送信号量
 * @param pvParameter 任务参数
 */
void gui_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting GUI task");
    logic_to_ui_msg_t rec_msg;
    
    while (1) {
        if (xQueueReceive(logic_to_ui_queue, &rec_msg, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (rec_msg.type) {
                case SERVO_INITIALIZED:
                    update_servo_initialization_ui(rec_msg.init_angle, rec_msg.servo_pin);
                    ESP_LOGI(TAG, "Servo Initialized Success");
                    break;
                    
                case LOGIC_MSG_SERVO_ANGLE_SET:
                    update_servo_angle_ui(rec_msg.angle);
                    break;
                    
                default:
                    ESP_LOGW(TAG, "Unknown message type: %d", rec_msg.type);
                    break;
            }
        }
    }
}

void main_logic_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting main logic task");
    ui_to_logic_msg_t rec_msg; // 接收来自UI任务的消息

    while (1) {
        if (xQueueReceive(ui_to_logic_queue, &rec_msg, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (rec_msg.type) {
                case UI_MSG_SERVO_SET_ANGLE:
                    // 处理来自UI的舵机角度设置消息
                    handle_servo_angle_request(rec_msg.angle);
                    break;
                    
                default:
                    ESP_LOGW(TAG, "Unknown message type: %d", rec_msg.type);
                    break;
            }
        }
    }
}
