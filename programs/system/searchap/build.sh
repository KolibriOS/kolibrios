#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KolibriOS application

	fasm -m 16384 searchap.asm ./searchap
	rm -f lang.inc
	exit 0
