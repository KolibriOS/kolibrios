#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS program, hopefully ;-)

	fasm -m 16384 madmouse.asm madmouse
	kpack madmouse
	exit 0




