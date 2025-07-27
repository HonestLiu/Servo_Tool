idf_component_register(
    SRCS
        "screens/ui_Screen1.c"
        "ui.c"
        "components/ui_comp_hook.c"
        "ui_helpers.c"
        "ui_events.c"
    INCLUDE_DIRS
        .
    REQUIRES lvgl  ui_interface
)