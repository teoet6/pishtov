#!/bin/sh

gcc -ldl -lX11 -lGL -lm -O2 -o dist/game src/game.c
zip -j dist/pishtov.zip src/game.c src/pishtov.h
