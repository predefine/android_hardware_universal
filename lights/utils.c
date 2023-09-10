#define _GNU_SOURCE
#include "platforms.h"
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

char* get_hal_platform_name(void) {
    Dl_info info;
    if (! dladdr((void *) get_hal_platform_name, & info))
        return NULL;
    char* fullpath = realpath(info.dli_fname, NULL);
    char* name = basename(fullpath);

    char* tokens[strlen(name)];
    int curr_token = 0;
    tokens[curr_token] = strtok(name, ".");
    while((tokens[++curr_token] = strtok(NULL, ".")) != NULL);
    int platform_token = curr_token-2;
    return tokens[platform_token];
}

int get_hal_platform_num(void){
    char* platform_name = get_hal_platform_name();
    for(size_t i = 0; i < (sizeof(platforms)/sizeof(struct platform)); i++)
        if(!strcmp(platforms[i].platform_name, platform_name))
            return i;

    return -1;
}
