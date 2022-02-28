#!/bin/sh

mkdir -p bin
mkdir -p dist

cat \
    src/pishtov.h \
    > dist/pishtov.h

cp src/game.c dist/game.c
cp src/game.c dist/game.cpp

echo 'Compiling with gcc'
gcc --std=c11 dist/game.c -lX11 -lGL -lm -O2 -o bin/game

echo 'Compiling with g++'
g++ --std=c++11 dist/game.cpp -lX11 -lGL -lm -O2 -o bin/gamepp
