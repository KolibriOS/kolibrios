#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

CLANG=$1;

usage()
{
	echo "Usage: make.sh [en|ru|ge|et]"
	exit 1
}

compile()
{
	fasm -m 65536 kernel.asm bin/kernel.mnt
	rm -f lang.inc
	exit 0
}


if [ ! $CLANG ] ; then
	usage
fi

for i in "en" "ru" "ge" "et"; do
	if [ $i == $CLANG ] ; then
		echo "lang fix $i" > lang.inc
		compile
	fi
done
usage
	
	
