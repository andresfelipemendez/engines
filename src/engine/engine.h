#ifndef ENGINE_H
#define ENGINE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct EngineAPI {
    void (*init)(void* glfw_window, int width, int height);
    void (*resize)(int width, int height);
    void (*update)(float dt);
    void (*shutdown)(void);
};

#if defined(__APPLE__)
  #define ENGINE_EXPORT __attribute__((visibility("default")))
#else
  #define ENGINE_EXPORT
#endif

ENGINE_EXPORT void get_engine_api(struct EngineAPI* out_api);

#ifdef __cplusplus
}
#endif
#endif // ENGINE_H