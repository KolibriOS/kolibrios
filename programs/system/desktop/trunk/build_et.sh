#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix et"
	echo "lang fix et" > lang.inc
	fasm -m 16384 desktop.asm desktop
	rm -f lang.inc
	exit 0




