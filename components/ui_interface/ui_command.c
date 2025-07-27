#include "ui_command.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "UI Command";

QueueHandle_t ui_to_logic_queue;  ///< UI任务到主逻辑任务的消息队列句柄

QueueHandle_t logic_to_ui_queue;  ///< 主逻辑任务到UI任务的消息队列句柄

/**
 * @brief 初始化任务间通信模块
 * 创建UI任务到主逻辑任务和主逻辑任务到UI任务的消息队列。
 */
void task_command_init(void){
    // 创建消息队列
    ui_to_logic_queue = xQueueCreate(10, sizeof(ui_to_logic_msg_t));
    logic_to_ui_queue = xQueueCreate(10, sizeof(logic_to_ui_msg_t));

    if (ui_to_logic_queue == NULL || logic_to_ui_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create message queues");
        return;
    }
    ESP_LOGI(TAG, "Message queues created successfully");
}

/**
 * @brief 发送UI消息到逻辑任务
 * @param msg 指向UI消息结构体的指针
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_ui_message(ui_to_logic_msg_t *msg){

    if(ui_to_logic_queue == NULL || msg == NULL) {
        ESP_LOGE(TAG, "UI to logic queue or message is NULL");
        return false;
    }
    if (xQueueSend(ui_to_logic_queue, msg, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send message to UI to logic queue");
        return false;
    }
    ESP_LOGI(TAG, "Message sent to UI to logic queue: type=%d, angle=%d", msg->type, msg->angle);
    return true;
}

/**
 * @brief 发送逻辑消息到UI任务
 * @param msg 指向逻辑消息结构体的指针
 * @return true 发送成功
 * @return false 发送失败
 */
bool send_logic_message(logic_to_ui_msg_t *msg){
    if(logic_to_ui_queue == NULL || msg == NULL) {
        ESP_LOGE(TAG, "Logic to UI queue or message is NULL");
        return false;
    }
    if (xQueueSend(logic_to_ui_queue, msg, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(TAG, "Failed to send message to Logic to UI queue");
        return false;
    }
    ESP_LOGI(TAG, "Message sent to Logic to UI queue: type=%d, angle=%d", msg->type, msg->angle);
    return true;
}