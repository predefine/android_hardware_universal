#include "utils.h"
#include "platforms.h"
#include <hardware/hardware.h>
#include <hardware/lights.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct platform curr_platform;
static int curr_platform_unknown = 1;

void init_globals(void)
{
    int platform_id = get_hal_platform_num();
    if(platform_id == -1) return;
    curr_platform = platforms[platform_id];
    curr_platform_unknown = 0;

    pthread_mutex_init(&g_lock, NULL);
}

static int write_int(const char* path, int value){
    int fd = open(path, O_WRONLY);
    if (fd < 0) goto error;
    char buffer[12];
    sprintf(buffer, "%d\n", value);
    if(write(fd, buffer, sizeof(buffer)) < 1) goto error;
    errno = 0;

error:
    if(fd > 0) close(fd);
    return -errno;
}

static int rgb_to_whiteblack(int color){
    return (
        (77*((color>>16)&0xff)) +
        (150*((color>>8)&0xff)) + 
        (29*(color&0xff))
    ) >> 8;
}

static int set_light_backlight(struct light_device_t* device, const struct light_state_t* state){
    (void)device;
    pthread_mutex_lock(&g_lock);
    int err = write_int(curr_platform.brightness_file_path,
        rgb_to_whiteblack(state->color)
    );
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int close_lights(struct light_device_t *device){
    if (device)
        free(device);
    return 0;
}

static int open_lights(const struct hw_module_t* module, const char* type,
                       struct hw_device_t** device){
    
    pthread_once(&g_init, init_globals);

    //Unknown platform
    if(curr_platform_unknown) return -1;

    if(!curr_platform.use_default_open_lights)
        return curr_platform.open_lights(module, type, device);

    //default open_lights
    if(strcmp(LIGHT_ID_BACKLIGHT, type))
        //Not implemented
        return -EINVAL;


    struct light_device_t* dev = malloc(sizeof(struct light_device_t));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int(*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light_backlight;
    
    *device = (struct hw_device_t*)dev;
    return 0;
}

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "OpenSource Universal Lights hardware",
    .author = "predefine",
    .methods = &(struct hw_module_methods_t){
        .open = open_lights
    },
};
