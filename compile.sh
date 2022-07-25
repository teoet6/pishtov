#!/bin/sh

rm -rf bin
mkdir -p bin

for src in examples/*.c; do
	out=$(basename "$src")
	out="bin/${out%.*}"
	echo "gcc $src -> $out"
	gcc --std=c11 -Wall -Werror "$src" -lX11 -lGL -lm -O2 -o "$out"
done

for src in examples/*.cpp; do
	out=$(basename "$src")
	out="bin/${out%.*}"
	echo "g++ $src -> $out"
	g++ --std=c++11 -Wall -Werror "$src" -lX11 -lGL -lm -O2 -o "$out"
done
