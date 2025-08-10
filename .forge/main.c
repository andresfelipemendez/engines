#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "build.h"

int build_forge_lib() {
    char *cmd = "clang -std=c99 -g -O0 -fno-omit-frame-pointer -Wall -Wextra -fPIC -dynamiclib .forge/forge.c -o libforge.dylib";
    return system(cmd);
}


int build_host(void) {
    char *cmd = "clang -std=c99 -O2 -Wall -Wextra main.c -o host";
    return system(cmd);
}

int main() {
    const char *builder_src = ".forge/forge.c";
    const char *builder_lib = "libforge.dylib";

    if(access(builder_lib, F_OK) != 0) {
        if (build_forge_lib() != 0) {
            fprintf(stderr, "failed to build %s\n", builder_lib);
            return 1;
        }
    }

    void *handle = NULL;
    void (*builder_step)(void) = NULL;

reload_lib:
    if (handle) { dlclose(handle); handle = NULL; builder_step = NULL; }
    handle = dlopen(builder_lib, RTLD_NOW);
    if (!handle) {fprintf(stderr, "dlopen: %s\n", dlerror()); return 1; }
    *(void **)(&builder_step) = dlsym(handle, "builder_step");
    if (!builder_step) {fprintf(stderr, "dlsym(builder_step): %s\n", dlerror()); return 1; }

    time_t last_src_mtime = mtime_of(builder_src);
    
    for(;;) {
        builder_step();
        time_t now = mtime_of(builder_src);
        if(now && now != last_src_mtime) {
            printf("[host] Rebuilding builder\n");
            if(build_forge_lib() == 0) {
                goto reload_lib;
            } else {
                fprintf(stderr, "[host] Rebuil failed\n");
            }
        }
        usleep(200 * 1000);
    }
}