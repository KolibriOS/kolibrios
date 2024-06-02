#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	echo "lang fix en_US"
	echo "lang fix en_US" > lang.inc
	mkdir bin
	fasm -m 16384 ctrldemo.asm ./bin/ctrldemo.kex
	fasm -m 16384 editbox_ex.asm ./bin/editbox_ex.kex
	rm -f lang.inc
	cp reload_16x16_8b.png ./bin/reload_16x16_8b.png
	exit 0



