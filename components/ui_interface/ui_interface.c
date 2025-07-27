#include "ui_interface.h"
#include "ui_command.h"
#include "esp_log.h"

static const char *TAG = "UI Interface";

bool ui_servo_set_angle(int angle){
    ui_to_logic_msg_t msg;
    msg.type = UI_MSG_SERVO_SET_ANGLE;
    msg.angle = angle;

    bool result = send_ui_message(&msg);

    ESP_LOGI(TAG, "Set servo angle: %d, result: %s", angle, result ? "success" : "failure");

    return result;
}