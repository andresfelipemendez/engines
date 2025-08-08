#!/usr/bin/env bash
set -euo pipefail

CFLAGS="-std=c23 -Wall -Wextra -Werror -O2 -fPIC -DGL_SILENCE_DEPRECATION -DGLFW_INCLUDE_NONE"
MACFW="-framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL"
INCLUDES="-Isrc -Ithirdparty -Ithirdparty/glfw/include -Ithirdparty/glad -Ithirdparty/nuklear"
OUTDIR=build
GLFW_STATIC="thirdparty/glfw/lib-arm64/libglfw3.a"

mkdir -p "$OUTDIR"

clang $CFLAGS $INCLUDES -dynamiclib engine.c "$GLFW_STATIC" $MACFW -o "$OUTDIR/engine.dylib"
cp -f "$OUTDIR/engine.dylib" "$OUTDIR/engine_live.dylib"

clang $CFLAGS $INCLUDES build.c "$GLFW_STATIC" $MACFW -o "$OUTDIR/host"