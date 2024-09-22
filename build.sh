#!/bin/bash

set -e

[ -d obj ] && rm -r obj
[ ! -d obj ] && mkdir obj

OPTS="-Wno-sign-compare -Wno-deprecated-declarations -Wno-pointer-arith"
if [ "$1" == "debug" ]; then
	OPTS="$OPTS -g -DDEBUG_BUILD"
fi

if [ -e /proc/cpuinfo ]; then
	if egrep -q 'BCM2708|BCM2711' /proc/cpuinfo; then
		echo "=== ENABLE RASPBERRY PI ==="
		OPTS="-D__rpi__ $OPTS"
	fi
fi

./bin/gcc-wrap  $OPTS --outdir=obj -c -I . \
	figlet_font/*.cpp \
	text_ui/*.c \
	graphics/*.c \
	windows/*.cpp windows/*.c \
	platform/*.cpp platform/*.c \
	main.cpp

g++ -o backboard obj/*.o
