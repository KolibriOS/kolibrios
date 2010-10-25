#!/bin/bash
# This is a simplified script to compile Kolibri-A kernel 
# please place *fasm program to the topmost KOS directory
# and create an empty /bin folder there before run this script  

	../../../../../fasm -m 65536 kernel.asm ../../../../../bin/kernel.mnt
	exit 0
	
	
