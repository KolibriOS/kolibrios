#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the program, hopefully ;-)

	echo "lang fix en"
	echo "lang fix en" > lang.inc
	fasm -m 16384 tinypad.asm tinypad
	kpack tinypad
	rm -f lang.inc
	exit 0




