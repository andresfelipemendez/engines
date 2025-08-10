#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "build.h"

typedef struct {
    char *path;
    time_t modified_time;
} Entry;

typedef struct {
    char* buffer;
    size_t capacity;
    size_t used;

    Entry *files;
    size_t count;
    size_t capacity_files; 
} Dict;

const size_t arena_size = 1 << 20;
const size_t max_files = 8192;

int file_exists(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode);
}

time_t mtime_of_path(const char *p) {
    struct stat st;
    return stat(p, &st) == 0 ? st.st_mtime : 0;
}

char* arena_push(Dict *d, const char *s) {
    size_t n = strlen(s) + 1;
    if(!d->buffer || d->capacity == 0) {
        fprintf(stderr, "arena not initialized\n");
        abort();
    }

    if(d->used + n > d->capacity) {
        fprintf(stderr, "arena overflow\n");
        abort();
    }
    char *dst = d->buffer + d->used;
    memcpy(dst, s, n);
    d->used += n;
    return dst;
}

void dict_init(Dict *d) {
    d->buffer = (char*)malloc(arena_size);
    if (!d->buffer) {perror("malloc"); exit(1);}
    d->capacity = arena_size;
    d->used = 0;

    d->files = (Entry*)calloc(max_files, sizeof(Entry));
    if (!d->files) {perror("calloc"); exit(1);}
    d->capacity_files = max_files;
    d->count = 0;
}

long dict_find(const Dict *d, const char *path){
    for(size_t i = 0; i < d->count; ++i)
        if(strcmp(d->files[i].path, path) == 0) return (long)i;
    return -1;
}

void dict_add(Dict *d, const char *path, time_t mt) {
    if(d->count == d->capacity_files) {
        fprintf(stderr, "dict full\n");
        abort();
    }
    d->files[d->count].path = arena_push(d,path);
    d->files[d->count].modified_time = mt;
    d->count++;
}

void dict_remove_swap(Dict *d, size_t idx) {
    d->files[idx] = d->files[d->count - 1];
    d->count--;
}

static int scan_dir_recursive(const char *root, Dict *d, int *changes) {
    DIR *dir = opendir(root);
    if (!dir) return 0;

    struct dirent *ent;
    char path[4096];

    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.' && (ent->d_name[1] == 0 || (ent->d_name[1]=='.' && ent->d_name[2]==0)))
            continue;

        int n = snprintf(path, sizeof(path), "%s/%s", root, ent->d_name);
        if (n <= 0 || (size_t)n >= sizeof(path)) continue;

        struct stat st;
        if (lstat(path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_dir_recursive(path, d, changes);
        } else if (S_ISREG(st.st_mode)) {
            time_t mt = st.st_mtime;
            long idx = dict_find(d, path);
            if (idx < 0) {
                dict_add(d, path, mt);
                (*changes)++;
            } else if (d->files[idx].modified_time != mt) {
                d->files[idx].modified_time = mt;
                (*changes)++;
            }
        }
    }
    closedir(dir);
    return 0;
}

int prune_deleted(Dict *d) {
    int del = 0;
    for(size_t i = 0; i < d->count;) {
        if(!file_exists(d->files[i].path)) {
            dict_remove_swap(d,i);
            del++;
            continue;
        }
        i++;
    }
    return del;
}

int poll_src_changes(Dict *d) {
    if(!d->buffer) dict_init(d);
    int changes = 0;
    scan_dir_recursive("src", d, &changes);
    changes += prune_deleted(d);
    return changes;
}

int build_engine_lib(void) {
    char *cmd = "clang -std=c99 -g -O0 -DGL_SILENCE_DEPRECATION -Wall -Wextra -Isrc -Ithirdparty -Ithirdparty/glfw/include -Ithirdparty/glad -Ithirdparty/nuklear -dynamiclib src/engine.c thirdparty/glfw/lib-arm64/libglfw3.a -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL -o enginelib.dylib";
    return system(cmd);
}

int download_deps(void) {
    return 0;
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