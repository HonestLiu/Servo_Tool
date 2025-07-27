#ifndef SERVO_TOOL_H
#define SERVO_TOOL_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

/* ========== 舵机参数配置 ========== */
#define SERVO_MIN_PULSEWIDTH_US (500)      // 舵机最小脉宽：0.5ms (对应 0°)
#define SERVO_MAX_PULSEWIDTH_US (2500)     // 舵机最大脉宽：2.5ms (对应 180°)
#define SERVO_MAX_DEGREE        (180)      // 舵机最大旋转角度

/* ========== LEDC PWM 配置 ========== */
#define SERVO_LEDC_TIMER        LEDC_TIMER_0        // 使用 LEDC 定时器 0
#define SERVO_LEDC_MODE         LEDC_LOW_SPEED_MODE // 低速模式，适合舵机控制
#define SERVO_LEDC_OUTPUT_IO    (11)                // PWM 输出引脚 GPIO10
#define SERVO_LEDC_CHANNEL      LEDC_CHANNEL_1      // 使用 LEDC 通道 0
#define SERVO_LEDC_FREQUENCY    (50)                // PWM 频率 50Hz (标准舵机频率)
#define SERVO_LEDC_RESOLUTION   LEDC_TIMER_13_BIT   // 13位分辨率 (8192个等级)


typedef struct {
    bool init_state;
    int init_angle;
    int servo_pin;
} servo_init_result_t;


/* ========== 公共接口函数 ========== */
servo_init_result_t servo_tool_init(void);
bool servo_tool_set_angle(int angle);
bool servo_tool_deinit(void);
int servo_tool_get_current_angle(void);
bool servo_tool_sweep(int start_angle, int end_angle, int step, int delay_ms);

#endif // SERVO_TOOL_H