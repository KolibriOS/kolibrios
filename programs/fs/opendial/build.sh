#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	echo "lang fix en"
	echo "lang fix en" > lang.inc
	fasm -m 16384 opendial.asm opendial
	rm -f lang.inc
	exit 0



