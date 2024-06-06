#!/bin/sh

	echo "lang fix it_IT"
	echo "lang fix it_IT" > lang.inc
	#/opt/fasm/fasm menu.asm @menu
	fasm menu.asm @menu
	kpack @menu
	#rm -f lang.inc
	exit 0




