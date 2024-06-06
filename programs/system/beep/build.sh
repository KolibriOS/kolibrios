#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix en_US"
	echo "lang fix en_US" > lang.inc
	mkdir bin
	fasm -m 65536 beep.asm ./bin/beep
	rm -f lang.inc
	exit 0
