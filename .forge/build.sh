#!/usr/bin/env bash
(
cd .forge
clang -std=c99 -Wall -Wextra -O2 main.c -o ../forge
clang -std=c99 -Wall -Wextra -O2 -fPIC -shared builder.c -o libbuilder.so
)
./forge