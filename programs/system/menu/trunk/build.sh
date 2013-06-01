#!/bin/sh

	echo "lang fix ru"
	echo "lang fix en"
	echo "lang fix de"
	echo "lang fix it" > lang.inc
	#/opt/fasm/fasm menu.asm @menu
	fasm menu.asm @menu
	kpack @menu
	#rm -f lang.inc
	exit 0




