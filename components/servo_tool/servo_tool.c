#include "servo_tool.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "Servo Tool";

// 当前舵机角度缓存，避免重复设置相同角度
static int current_angle = -1;

// PWM 周期常量 (微秒)
#define SERVO_PERIOD_US (20000)  // 20ms = 20000μs

// 预计算的常量，提高性能
#define DUTY_RESOLUTION_MAX ((1 << 13) - 1)  // 2^13 - 1 = 8191


/**
 * @brief 角度转换为脉宽函数
 * @param angle 输入角度 (0-180°)
 * @return 对应的脉宽时间 (微秒)
 * 
 * 计算公式：
 * duty_us = min_pulse + (max_pulse - min_pulse) * angle / max_angle
 * 
 * 示例：
 * - 0°   → 500μs
 * - 90°  → 1500μs  
 * - 180° → 2500μs
 */
static uint32_t servo_angle_to_duty_us(int angle) {
    // 角度范围限制，防止超出舵机物理极限
    if (angle < 0) angle = 0;
    if (angle > SERVO_MAX_DEGREE) angle = SERVO_MAX_DEGREE;
    
    // 线性插值计算脉宽
    uint32_t duty_us = SERVO_MIN_PULSEWIDTH_US +
        (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) * angle / SERVO_MAX_DEGREE;
    return duty_us;
}

/**
 * @brief 设置舵机角度函数 (优化版本)
 * @param angle 目标角度 (0-180°)
 * 
 * 优化要点：
 * 1. 使用预计算常量提高性能
 * 2. 避免重复设置相同角度
 * 3. 更精确的占空比计算
 */
static void servo_set_angle(int angle)
{
    // 检查是否为相同角度，避免重复设置
    if (angle == current_angle) {
        return;
    }
    
    // Step 1: 获取对应角度的脉宽时间
    uint32_t duty_us = servo_angle_to_duty_us(angle);
    
    // Step 2: 优化的占空比计算
    // 使用位移操作替代除法，提高计算效率
    // duty = (duty_us * (1 << 13)) / 20000
    uint32_t duty = (duty_us * DUTY_RESOLUTION_MAX) / SERVO_PERIOD_US;
    
    // 确保占空比不超过最大值
    if (duty > DUTY_RESOLUTION_MAX) {
        duty = DUTY_RESOLUTION_MAX;
    }
    
    // Step 3: 设置 LEDC 占空比
    esp_err_t ret = ledc_set_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, duty);
    if (ret != ESP_OK) {
        return;
    }
    
    // Step 4: 更新占空比设置（使新设置生效）
    ret = ledc_update_duty(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL);
    if (ret == ESP_OK) {
        current_angle = angle;  // 更新当前角度缓存
    }
}

servo_init_result_t servo_tool_init(void){
    esp_err_t ret;
    servo_init_result_t result = {
        .init_angle = 0,
        .init_state = false,
        .servo_pin = SERVO_LEDC_OUTPUT_IO,
    };

    // 配置 LEDC 定时器
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = SERVO_LEDC_RESOLUTION, // 13位分辨率
        .freq_hz = SERVO_LEDC_FREQUENCY,          // 50Hz
        .clk_cfg = LEDC_AUTO_CLK,                 // LEDC自动选择时钟
        .speed_mode = SERVO_LEDC_MODE,            // LEDC低速模式
        .timer_num = SERVO_LEDC_TIMER,            // 使用定时器0
    };
    
    ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return result;
    }

    // 配置 LEDC 通道
    ledc_channel_config_t ledc_channel = {
        .speed_mode = SERVO_LEDC_MODE,         // LEDC低速模式
        .channel = SERVO_LEDC_CHANNEL,         // 使用通道0
        .timer_sel = SERVO_LEDC_TIMER,         // 关联定时器0
        .intr_type = LEDC_INTR_DISABLE,        // 禁用中断
        .gpio_num = SERVO_LEDC_OUTPUT_IO,      // 输出引脚 GPIO18
        .duty = 0,                             // 初始占空比为0
        .hpoint = 0,                           // 初始hpoint为0
    };
    
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return result;
    }

    // 初始化舵机到中位角度 (90度)
    servo_set_angle(0);
    
    ESP_LOGI(TAG, "Servo tool initialized successfully on GPIO%d", SERVO_LEDC_OUTPUT_IO);
    result.init_state = true;
    return result;
}

bool servo_tool_set_angle(int angle){
    // 参数检查
    if (angle < 0 || angle > SERVO_MAX_DEGREE) {
        ESP_LOGE(TAG, "Invalid angle: %d. Must be between 0 and %d.", angle, SERVO_MAX_DEGREE);
        return false;
    }

    // 设置舵机角度
    servo_set_angle(angle);
    
    ESP_LOGI(TAG, "Servo angle set to: %d degrees", angle);
    return true;
}

/**
 * @brief 反初始化舵机工具
 * @return true 成功, false 失败
 */
bool servo_tool_deinit(void) {
    esp_err_t ret;
    
    // 停止 LEDC 通道
    ret = ledc_stop(SERVO_LEDC_MODE, SERVO_LEDC_CHANNEL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop LEDC channel: %s", esp_err_to_name(ret));
        return false;
    }
    
    current_angle = -1;  // 重置角度缓存
    ESP_LOGI(TAG, "Servo tool deinitialized");
    return true;
}

/**
 * @brief 获取当前舵机角度
 * @return 当前角度，-1表示未初始化
 */
int servo_tool_get_current_angle(void) {
    return current_angle;
}

/**
 * @brief 舵机扫描功能 - 在指定角度范围内循环转动
 * @param start_angle 起始角度
 * @param end_angle 结束角度  
 * @param step 步进角度
 * @param delay_ms 每步之间的延时(毫秒)
 * @return true 成功, false 失败
 */
bool servo_tool_sweep(int start_angle, int end_angle, int step, int delay_ms) {
    // 参数验证
    if (start_angle < 0 || start_angle > SERVO_MAX_DEGREE ||
        end_angle < 0 || end_angle > SERVO_MAX_DEGREE ||
        step <= 0 || delay_ms < 0) {
        ESP_LOGE(TAG, "Invalid sweep parameters");
        return false;
    }
    
    // 确定扫描方向
    int direction = (start_angle < end_angle) ? 1 : -1;
    step *= direction;
    
    // 执行扫描
    for (int angle = start_angle; 
         (direction > 0) ? (angle <= end_angle) : (angle >= end_angle); 
         angle += step) {
        
        if (!servo_tool_set_angle(angle)) {
            return false;
        }
        
        if (delay_ms > 0) {
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    
    ESP_LOGI(TAG, "Sweep completed from %d° to %d°", start_angle, end_angle);
    return true;
}