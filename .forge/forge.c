#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>

#include "download.c"
#include "dict.c"
#include "filewatch.c"

int build_engine_lib(void) {
    char *cmd = "clang -std=c99 -g -O0 -DGL_SILENCE_DEPRECATION -Wall -Wextra -Isrc -Ithirdparty -Ithirdparty/glfw/include -Ithirdparty/glad -Ithirdparty/nuklear -dynamiclib src/engine.c thirdparty/glfw/lib-arm64/libglfw3.a -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL -o enginelib.dylib";
    return system(cmd);
}

int download_deps(void) {
    return 0;
}

__attribute__((visibility("default")))
void generate_project(const char *project_name) {
    // generate_project

}

__attribute__((visibility("default")))
void builder_step(void) {
    static Dict dict = {0};
    static int first = 1;
    
    int changed = poll_src_changes(&dict);
    if(first) { first = 0; changed = 1;}
    
    if(changed) {
        if(build_engine_lib() == 0) {
            puts("[forge] libengine rebuilt");
        } else {
            fprintf(stderr, "[forge] build failed.\n");
        }
    }
}