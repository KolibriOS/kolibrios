#!/bin/bash
# This script does for linux the same as build.bat for DOS
	echo "lang fix en_US"
	echo "lang fix en_US" > lang.inc
	fasm -m 65536 zSea.asm zSea
	kpack zSea
	rm -f lang.inc
	read
	exit 0
