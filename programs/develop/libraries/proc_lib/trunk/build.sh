#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications

	mkdir bin
	fasm -m 16384 proc_lib.asm ./bin/proc_lib.obj
	exit 0



