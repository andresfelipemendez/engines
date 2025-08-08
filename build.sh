#!/usr/bin/env bash
set -euo pipefail

# --- Config ---
CFLAGS="-std=c2x -Wall -Wextra -Werror -O2 -fPIC"
MACFW="-framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL"
OUTDIR="build"
HOST_OUT="$OUTDIR/host"
ENGINE_OUT="$OUTDIR/engine.dylib"
ENGINE_STAGE="$OUTDIR/engine_live.dylib"   # file the host actually dlopens

mkdir -p "$OUTDIR"

# Try pkg-config first; fallback to Homebrew path if needed
if pkg-config --exists glfw3; then
  GLFW_CFLAGS="$(pkg-config --cflags glfw3)"
  GLFW_LIBS="$(pkg-config --libs glfw3)"
else
  # Common Homebrew include/lib paths (adjust if your Homebrew lives elsewhere)
  GLFW_CFLAGS="-I/usr/local/include -I/opt/homebrew/include"
  GLFW_LIBS="-L/usr/local/lib -L/opt/homebrew/lib -lglfw"
fi

# --- Build engine.dylib ---
clang $CFLAGS -Isrc -Ithirdparty $GLFW_CFLAGS \
  -dynamiclib -o "$ENGINE_OUT" \
  src/engine/engine.c \
  $GLFW_LIBS $MACFW

# Stage a copy with a different name so the host can overwrite engine.dylib while running
cp -f "$ENGINE_OUT" "$ENGINE_STAGE"

# --- Build host executable (unity build) ---
clang $CFLAGS -Isrc -Ithirdparty $GLFW_CFLAGS \
  build.c -o "$HOST_OUT" \
  $GLFW_LIBS $MACFW

echo "\n✅ Built: $HOST_OUT"
echo "✅ Built: $ENGINE_OUT (and staged $ENGINE_STAGE)"
echo "\nRun: ./build/host"