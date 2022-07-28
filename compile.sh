#!/bin/sh

rm -rf bin
mkdir -p bin

buildc () {
	out=$(basename "$1")
	out="bin/${out%.*}"
	echo "gcc $1 -> $out"
	gcc --std=c11 -Wall -Werror "$1" -lX11 -lGL -lm -O2 -o "$out"
}

buildcpp () {
	out=$(basename "$1")
	out="bin/${out%.*}"
	echo "g++ $1 -> $out"
	g++ --std=c++11 -Wall -Werror "$1" -lX11 -lGL -lm -O2 -o "$out"
}

if [ -n "$1" ]; then
	case ${1##*.} in
		c)   buildc   "$1" ;;
		cpp) buildcpp "$1" ;;
		*)   echo "Don't know how to build $1." > /dev/stderr ; exit 1 ;;
	esac
	exit 0
fi

for src in examples/*.c; do
	buildc "$src"
done

for src in examples/*.cpp; do
	buildcpp "$src"
done
