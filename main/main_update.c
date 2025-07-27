#include "main_update.h"
#include "esp_log.h"
#include "servo_tool.h"
#include "ui_command.h"
#include "ui_interface.h"
#include <stdbool.h>
#include "ui.h"
#include "lvgl.h"

static const char *TAG = "Main Update";

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
                    // 更新默认角度到UI
                    lv_label_set_text_fmt(ui_angleValue, "%d", rec_msg.init_angle);
                    lv_slider_set_value(ui_angleSlider,rec_msg.init_angle,LV_ANIM_ON);
                    // 更新舵机引脚到UI
                    lv_label_set_text_fmt(ui_ServoPin, "%d", rec_msg.servo_pin);
                    ESP_LOGI(TAG,"Serveo Initialized Success");
                    break;
                case LOGIC_MSG_SERVO_ANGLE_SET:
                    // 更新UI显示当前舵机角度
                    lv_label_set_text_fmt(ui_angleValue, "%d", rec_msg.angle);
                    ESP_LOGI(TAG, "Servo angle set to: %d", rec_msg.angle);
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
    logic_to_ui_msg_t send_msg; // 发送到UI层的数据
    bool ret;

    while (1) {
       if (xQueueReceive(ui_to_logic_queue,&rec_msg,pdMS_TO_TICKS(10)) == pdTRUE)
       {
           switch (rec_msg.type) {
               case UI_MSG_SERVO_SET_ANGLE:
                   // 处理来自UI的舵机角度设置消息
                   ret =  servo_tool_set_angle(rec_msg.angle);
                   if (ret == true)
                   {
                    ESP_LOGI(TAG, "Received servo angle set message: %d", rec_msg.angle);
                    send_msg.type = LOGIC_MSG_SERVO_ANGLE_SET;
                    send_msg.angle = rec_msg.angle;
                    send_logic_message(&send_msg);
                   }
                   break;
               default:
                   ESP_LOGW(TAG, "Unknown message type: %d", rec_msg.type);
                   break;
           }
       }
       
    }
}
