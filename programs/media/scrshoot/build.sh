#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix ru_RU"
	echo "lang fix ru_RU" > lang.inc
	fasm -m 16384 scrshoot.asm scrshoot
	rm -f lang.inc
	exit 0




