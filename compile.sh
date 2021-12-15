#!/bin/sh

g++ -ldl -lX11 -lGL -lm -O2 -o dist/game src/game.cpp
zip -j dist/pishtov.zip src/game.cpp src/pishtov.h
