#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	fasm -m 16384 rotate.asm rotate.obj
	kpack rotate.obj
	exit 0



