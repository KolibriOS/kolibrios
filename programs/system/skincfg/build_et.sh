#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	echo "lang fix et_EE"
	echo "lang fix et_EE" > lang.inc
	fasm -m 16384 skincfg.asm skincfg
	rm -f lang.inc
	exit 0




