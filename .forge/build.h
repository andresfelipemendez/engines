#ifndef BUILD_H
#define BUILD_H

#define LIB_EXT ".dylib"
#define LIB_LINK "-dynamiclib"
#define EXTRA_EXE ""

time_t mtime_of(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 ? st.st_mtime : 0;
}

#endif