#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

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
    int (*generate_project)(const char *project_name) = NULL;

    *(void **)(&builder_step) = dlsym(handle, "builder_step");
    *(void **)(&generate_project) = dlsym(handle, "generate_project");

    if(strcmp(mode, "generate") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Missing project name for generate mode.\n");
            dlclose(handle);
            return 1;
        }
        if(!generate_project) {
            fprintf(stderr, "forge_init() not found in %s\n", builder_lib);
            dlclose(handle);
            return 1;
        }
        return generate_project(argv[2]);
    }

    if(strcmp(mode, "watch") == 0) {
        if (!builder_step) {
            fprintf(stderr, "builder_step() not found in %s\n", builder_lib);
            dlclose(handle);
            return 1;
        }

        time_t last_src_mtime = mtime_of(builder_src);

        for (;;) {
            builder_step();
            time_t now = mtime_of(builder_src);
            if (now && now != last_src_mtime) {
                printf("[host] Rebuilding builder\n");
                if(build_forge_lib() == 0) {
                    dlclose(handle);
                    handle = dlopen(builder_lib, RTLD_NOW);
                    *(void **)(&builder_step) = dlsym(handle, "builder_step");
                    *(void **)(&generate_project) = dlsym(handle, "generate_project");
                    last_src_mtime = now;
                } else {
                    fprintf(stderr, "[host] Rebuild failed\n");
                }
            }
            usleep(200 * 1000);
        }
    }

    fprintf(stderr, "Unknown mode: %s\n", mode);
    dlclose(handle);
    return 1;
}