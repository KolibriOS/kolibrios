#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	fasm -m 16384 cnv_png.asm cnv_png.obj
	kpack cnv_png.obj
	exit 0



