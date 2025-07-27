#ifndef UI_COMMAND_H
#define UI_COMMAND_H
// 舵机控制UI中间层

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "stdbool.h"

/**
 * @brief UI任务到主逻辑任务的消息队列句柄
 */
extern QueueHandle_t ui_to_logic_queue;

/**
 * @brief 主逻辑任务到UI任务的消息队列句柄
 */
extern QueueHandle_t logic_to_ui_queue;

// UI任务到主逻辑任务的消息类型
typedef enum {
    UI_MSG_SERVO_SET_ANGLE,  ///< 设置舵机角度
}ui_message_type_t;

// 主逻辑任务到UI任务的消息结构体
typedef enum {
    // Initialized
    SERVO_INITIALIZED,          ///< 舵机完成初始化
    LOGIC_MSG_SERVO_ANGLE_SET,  ///< 舵机角度已设置
}logic_message_type_t;

// UI任务到主逻辑任务的消息结构体
typedef struct {
    ui_message_type_t type;      ///< 消息类型
    int angle;                   ///< 目标角度
} ui_to_logic_msg_t;

// 主逻辑任务到UI任务的消息结构体
typedef struct {
    logic_message_type_t type;   ///< 消息类型
    int angle;                   ///< 当前角度
    int servo_pin;
    int init_angle;
} logic_to_ui_msg_t;

// 初始化任务间通信模块
void task_command_init(void);

// 发送UI消息到逻辑任务
bool send_ui_message(ui_to_logic_msg_t *msg);

// 发送逻辑消息到UI任务
bool send_logic_message(logic_to_ui_msg_t *msg);

#endif // UI_COMMAND_H