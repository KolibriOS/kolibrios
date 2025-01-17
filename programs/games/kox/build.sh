#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

	fasm -m 16384 mgb.asm mgb
	kpack mgb
	exit 0




