#!/bin/sh

mkdir -p bin
mkdir -p dist

cat \
    src/pishtov.h \
    > dist/pishtov.h

cp src/game.c dist/game.c

time gcc --std=c11 dist/game.c -ldl -lX11 -lGL -lm -O2 -o bin/game
