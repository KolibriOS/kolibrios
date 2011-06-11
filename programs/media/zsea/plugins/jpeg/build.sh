#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	fasm -m 16384 cnv_jpeg.asm cnv_jpeg.obj
	kpack cnv_jpeg.obj
	exit 0



