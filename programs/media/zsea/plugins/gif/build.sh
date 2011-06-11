#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	fasm -m 16384 cnv_gif.asm cnv_gif.obj
	kpack cnv_gif.obj
	exit 0



