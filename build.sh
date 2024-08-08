#!/bin/bash

set -e

[ -d obj ] && rm -r obj
[ ! -d obj ] && mkdir obj

OPTS="-Wno-sign-compare"

./bin/gcc-wrap $OPTS --outdir=obj -c -I include figlet_font/*.cpp text_ui/*.c graphics/*.c
