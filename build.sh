#!/bin/sh
rm -rf ./build
mkdir build
gcc -o build/zulu.out src/main.c `sdl2-config --cflags --libs` -lenet -lm
cp ./res/* ./build
