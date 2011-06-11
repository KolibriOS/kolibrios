#!/bin/bash
# This script does for linux the same as build.bat for DOS
	echo "lang fix en"
	echo "lang fix en" > lang.inc
	fasm -m 65536 zSea.asm zSea
	@kpack zSea
	rm -f lang.inc
	read
	exit 0
