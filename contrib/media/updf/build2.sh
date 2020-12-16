#!/bin/bash
rm mupdf
rm -rf build
make -f Makefile_2
mv build/mupdf ./
objcopy mupdf -O binary 
sleep 10
