#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	fasm -m 16384 convert.asm convert.obj
	kpack convert.obj
	exit 0



