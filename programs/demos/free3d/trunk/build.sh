#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	fasm -m 16384 free3d.asm free3d
	kpack free3d
	exit 0




