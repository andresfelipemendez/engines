#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "build.h"

int build_forge_lib() {
    char *cmd = "clang -std=c99 -g -O0 -fno-omit-frame-pointer -Wall -Wextra -fPIC -dynamiclib .forge/forge.c -lcurl -o libforge.dylib";
    return system(cmd);
}


int build_host(void) {
    char *cmd = "clang -std=c99 -O2 -Wall -Wextra main.c -o host";
    return system(cmd);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s generate <name>, or watch\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *builder_src = ".forge/forge.c";
    const char *builder_lib = "libforge.dylib";

    if (build_forge_lib() != 0) {
        fprintf(stderr, "failed to build %s\n", builder_lib);
        return 1;
    }
    
    void *handle = dlopen(builder_lib, RTLD_NOW);
    if(!handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror()); 
        return 1; 
    }

    void (*builder_step)(void) = NULL;
    void (*generate_project)(const char *project_name) = NULL;

    *(void **)(&builder_step) = dlsym(handle, "builder_step");
    *(void **)(&generate_project) = dlsym(handle, "generate_project");

    if(strcmp(mode, "generate") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Missing project name for generate mode.\n");
            dlclose(handle);
            return 1;
        }
    }

reload_lib:
    if (handle) { dlclose(handle); handle = NULL; builder_step = NULL; }
    
    
    
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