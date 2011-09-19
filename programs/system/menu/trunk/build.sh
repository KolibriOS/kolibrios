#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix en"
	echo "lang fix en" > lang.inc
	fasm -m 16384 menu.asm @menu
	kpack @menu
	rm -f lang.inc
	exit 0




