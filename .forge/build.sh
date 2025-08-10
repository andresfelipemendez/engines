#!/usr/bin/env bash
(
cd .forge
clang -std=c99 -g -Wall -Wextra -O0 -fno-omit-frame-pointer main.c -o ../forge
)
# ./forge