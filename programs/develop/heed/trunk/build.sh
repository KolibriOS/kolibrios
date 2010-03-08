#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix en"
	echo "lang fix en" > lang.inc
	mkdir bin
	fasm -m 16384 heed.asm ./bin/heed
	rm -f lang.inc
	exit 0




