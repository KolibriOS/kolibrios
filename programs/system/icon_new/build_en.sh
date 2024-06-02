#!/bin/bash
# This script does for linux the same as build.bat for DOS,

	echo "lang fix en_US"
	echo "lang fix en_US" > lang.inc
	fasm -m 16384 icon.asm @icon
	kpack @icon
	rm -f lang.inc
	exit 0




