#!/bin/bash
# This script does for Linux the same as build.bat for DOS,
# it compiles the current KolibriOS applications
OutDir=bin
FileName=box_lib.obj
OutFile=$OutDir/$FileName

	if [ -d "$OutDir" ]; then
	    if [ -e "$OutFile" ]; then
		echo " >>>> rm -f $OutFile delete old version"
		rm -f $OutFile
	    fi
	else
	    echo "mkdir $OutDir"
	    mkdir $OutDir
	fi
	fasm -m 16384 box_lib.asm $OutFile
	exit 0



