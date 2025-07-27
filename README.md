# ESP32-S3 舵机控制工具

![version v1.2.0](https://img.shields.io/badge/version-v1.2.0-brightgreen)
![license MIT](https://img.shields.io/badge/license-MIT-green)
![ESP32-S3](https://img.shields.io/badge/platform-ESP32--S3-blue)

## 项目简介

这是一个基于ESP32-S3微控制器的智能舵机控制工具，使用LVGL图形库创建现代化的触摸用户界面。项目支持ST7789驱动的TFT LCD显示屏和FT5x06触摸控制器，提供了直观的舵机角度控制界面，并采用了模块化的软件架构设计。

## 功能特性

### 🎯 舵机控制
- **精确角度控制**: 支持0°-180°精确角度设置
- **多种输入方式**: 滑块拖拽、预设按钮(0°, 45°, 90°, 180°)
- **实时反馈**: UI界面实时显示当前舵机角度
- **PWM输出**: 基于LEDC的高精度PWM信号生成

### 🎨 用户界面
- **现代化UI**: 基于LVGL 8.3的响应式界面设计
- **触摸操作**: 支持滑块拖拽和按钮点击
- **实时显示**: 显示当前角度值和GPIO引脚信息
- **流畅动画**: 滑块值变化带有平滑动画效果

### 🏗️ 软件架构
- **模块化设计**: 业务逻辑与UI层分离
- **任务调度**: FreeRTOS多任务并发处理
- **消息队列**: 任务间通过队列进行通信
- **组件化**: 可复用的舵机控制组件

### 🔧 硬件支持
- **微控制器**: ESP32-S3
- **显示屏**: 320x240 TFT LCD (ST7789驱动)
- **触摸屏**: FT5x06 电容触摸控制器
- **IO扩展**: PCA9557 I2C IO扩展芯片
- **舵机控制**: GPIO10 PWM输出(可配置)

## 项目结构

```
├── main/
│   ├── main.c              # 主程序入口
│   ├── main_update.c/h     # 任务更新逻辑(GUI和业务逻辑)
│   ├── lcd.c/lcd.h         # LCD驱动层
│   ├── lvgl-components.c/h # LVGL组件配置
│   └── CMakeLists.txt      # 主模块构建配置
├── components/
│   ├── servo_tool/         # 舵机控制组件
│   │   ├── include/
│   │   │   └── servo_tool.h # 舵机控制API
│   │   ├── servo_tool.c    # 舵机控制实现
│   │   └── CMakeLists.txt  # 舵机组件构建配置
│   ├── ui_interface/       # UI接口组件
│   │   ├── include/
│   │   │   └── ui_interface.h
│   │   ├── ui_interface.c  # UI接口实现
│   │   ├── ui_command.c    # 命令处理
│   │   └── CMakeLists.txt
│   └── ui_app/             # UI应用组件
│       ├── screens/        # UI屏幕文件
│       ├── ui.c/ui.h       # UI主文件
│       ├── ui_events.c/h   # UI事件处理
│       └── CMakeLists.txt
└── managed_components/     # ESP-IDF托管组件
    ├── espressif__esp_lcd_touch/
    ├── espressif__esp_lcd_touch_ft5x06/
    ├── espressif__esp_lvgl_port/
    └── lvgl__lvgl/
```

## 软件架构

### 🔄 任务架构
项目采用多任务架构，各任务职责明确：

```c
// 任务优先级设计
hardware_init_task (优先级5) → 初始化后自删除
├── gui_task (优先级6)       → 处理UI更新和用户交互
└── main_logic_task (优先级4) → 处理业务逻辑和硬件控制
```

### 📨 消息通信
任务间通过FreeRTOS队列进行通信：

```c
// UI → 业务逻辑
ui_to_logic_queue: UI_MSG_SERVO_SET_ANGLE

// 业务逻辑 → UI  
logic_to_ui_queue: LOGIC_MSG_SERVO_ANGLE_SET, SERVO_INITIALIZED
```

### 🎛️ 舵机控制API

```c
/**
 * @brief 初始化舵机工具
 * @return true 成功, false 失败
 */
bool servo_tool_init(void);

/**
 * @brief 设置舵机角度
 * @param angle 目标角度 (0-180°)
 * @return true 成功, false 失败
 */
bool servo_tool_set_angle(int angle);

/**
 * @brief 获取当前舵机角度
 * @return 当前角度，-1表示未初始化
 */
int servo_tool_get_current_angle(void);

/**
 * @brief 舵机扫描功能
 * @param start_angle 起始角度
 * @param end_angle 结束角度
 * @param step 步进角度
 * @param delay_ms 每步延时(毫秒)
 * @return true 成功, false 失败
 */
bool servo_tool_sweep(int start_angle, int end_angle, int step, int delay_ms);
```

## 硬件连接

### 舵机连接
| ESP32-S3引脚 | 舵机信号 | 描述 |
|-------------|---------|------|
| GPIO10      | PWM     | 舵机控制信号线(可配置) |
| 5V/VCC      | 红线    | 舵机电源正极 |
| GND         | 黑/棕线 | 舵机电源负极 |

### LCD显示屏 (SPI接口)
| ESP32-S3引脚 | LCD信号 | 描述 |
|-------------|---------|------|
| GPIO40      | MOSI    | SPI数据输出 |
| GPIO41      | SCLK    | SPI时钟 |
| GPIO39      | DC      | 数据/命令选择 |
| GPIO42      | BL      | 背光控制(PWM) |
| CS信号      | 通过PCA9557控制 | 片选信号 |

### 触摸屏 (I2C接口)
| ESP32-S3引脚 | 触摸屏信号 | 描述 |
|-------------|-----------|------|
| GPIO1       | SDA       | I2C数据线 |
| GPIO2       | SCL       | I2C时钟线 |

### PCA9557 IO扩展芯片
| PCA9557引脚 | 功能      | 描述 |
|------------|----------|------|
| IO0        | LCD_CS   | LCD片选控制 |
| IO1        | PA_EN    | 功放使能 |
| IO2        | DVP_PWDN | 摄像头电源控制 |

## 配置说明

### 舵机参数配置
在 `components/servo_tool/include/servo_tool.h` 中可以配置舵机参数：

```c
/* ========== 舵机参数配置 ========== */
#define SERVO_MIN_PULSEWIDTH_US (500)      // 舵机最小脉宽：0.5ms (对应 0°)
#define SERVO_MAX_PULSEWIDTH_US (2500)     // 舵机最大脉宽：2.5ms (对应 180°)
#define SERVO_MAX_DEGREE        (180)      // 舵机最大旋转角度

/* ========== LEDC PWM 配置 ========== */
#define SERVO_LEDC_OUTPUT_IO    (10)                // PWM 输出引脚 GPIO10
#define SERVO_LEDC_FREQUENCY    (50)                // PWM 频率 50Hz (标准舵机频率)
#define SERVO_LEDC_RESOLUTION   LEDC_TIMER_13_BIT   // 13位分辨率 (8192个等级)
```

### 性能优化特性
- **角度缓存**: 避免重复设置相同角度
- **错误处理**: 完整的ESP-IDF错误检查
- **预计算常量**: 提高PWM占空比计算效率
- **调试支持**: 详细的日志输出用于问题诊断

## 使用示例

### 基本使用
```c
#include "servo_tool.h"

void app_main() {
    // 初始化舵机
    if (servo_tool_init()) {
        ESP_LOGI("APP", "Servo initialized successfully");
        
        // 设置舵机到45度
        servo_tool_set_angle(45);
        
        // 获取当前角度
        int current = servo_tool_get_current_angle();
        ESP_LOGI("APP", "Current angle: %d", current);
    }
}
```

### 扫描功能示例
```c
// 从0度扫描到180度，每次步进10度，间隔100ms
servo_tool_sweep(0, 180, 10, 100);

// 反向扫描
servo_tool_sweep(180, 0, -10, 50);
```

## 快速开始

### 1. 环境准备
```bash
# 安装ESP-IDF v5.4+
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh
```

### 2. 克隆项目
```bash
git clone https://github.com/HonestLiu/Servo_Tool.git
cd Servo_Tool
```

### 3. 配置和编译
```bash
# 设置目标芯片
idf.py set-target esp32s3

# 配置项目
idf.py menuconfig

# 编译项目
idf.py build
```

### 4. 烧录和监控
```bash
# 烧录到设备
idf.py flash

# 监控串口输出
idf.py monitor
```

## 调试和故障排除

### 常见问题

#### 1. 舵机不动
- 检查舵机电源是否正确连接(需要独立5V电源)
- 确认信号线连接到正确的GPIO引脚
- 检查接地是否共通
- 查看串口日志是否有PWM设置错误

#### 2. 编译错误
```bash
# 清理构建
idf.py fullclean

# 重新编译
idf.py build
```

#### 3. 触摸不响应
- 检查I2C连接是否正确
- 确认FT5x06驱动是否正常加载

### 调试日志
项目提供了详细的调试日志，通过串口监控可以看到：
```
I (1234) Servo Tool: Initializing servo tool on GPIO10
I (1235) Servo Tool: servo_set_angle called with angle: 90, current_angle: -1
I (1236) Servo Tool: Calculated duty_us: 1500
I (1237) Servo Tool: Calculated duty: 614 (max: 8191)
I (1238) Servo Tool: Successfully set servo to 90 degrees (duty: 614)
```

## 版本历史

- **v1.2.0** (2025-07-27): 进一步解耦合，添加模块化架构
- **v1.1.0** (2025-07-26): 增加UI界面支持
- **v1.0.0** (2025-07-26): 初始版本，基础舵机控制功能

## 贡献指南

欢迎提交Issue和Pull Request来改进这个项目！

### 开发规范
- 使用有意义的提交信息
- 遵循ESP-IDF编码规范
- 添加适当的注释和文档
- 测试新功能的稳定性

## 许可证

本项目采用 MIT 许可证，详情请参阅 [LICENSE](LICENSE) 文件。

## 致谢

- [ESP-IDF](https://github.com/espressif/esp-idf) - 强大的ESP32开发框架
- [LVGL](https://github.com/lvgl/lvgl) - 优秀的嵌入式图形库
- [SquareLine Studio](https://squareline.io/) - UI设计工具

---

📧 如有问题，请联系: [honestliu@outlook.com](mailto:honestliu@outlook.com)

⭐ 如果这个项目对你有帮助，请给个Star支持一下！
