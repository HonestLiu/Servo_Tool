idf_component_register(SRCS "main.c" "lcd.c" "lvgl-components.c" "main_update.c"
                    INCLUDE_DIRS "."
                    REQUIRES ui_interface servo_tool ui_app
                    )