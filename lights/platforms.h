#pragma once
#include <hardware/hardware.h>

struct platform {
    char *platform_name;
    int use_default_open_lights;
    
    // set only when use_default_open_lights = 1, or value be ignored
    char *brightness_file_path;

    // set only when use_default_open_lights = 0, or value be ignored
    int (*open_lights)(const struct hw_module_t* module, const char* type,
                       struct hw_device_t** device);
};

const static struct platform platforms[] = {
    {"mt6580", 1, "/sys/class/leds/lcd-backlight/brightness", NULL},
    {"universal7570", 1, "/sys/class/backlight/panel/brightness", NULL}
};
