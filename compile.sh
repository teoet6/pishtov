#!/bin/sh

rm -rf bin
mkdir -p bin

cat \
    src/pishtov.h \
    > pishtov.h

rm -rf build/
cp -r --no-target-directory examples/ build/
cp pishtov.h build/

for src in build/*.c; do
	out=$(basename "$src")
	out="bin/${out%.*}"
	echo "gcc $src -> $out"
	gcc --std=c11 -Wall -Werror "$src" -lX11 -lGL -lm -O2 -o "$out"
done

for src in build/*.cpp; do
	out=$(basename "$src")
	out="bin/${out%.*}"
	echo "g++ $src -> $out"
	g++ --std=c++11 -Wall -Werror "$src" -lX11 -lGL -lm -O2 -o "$out"
done
