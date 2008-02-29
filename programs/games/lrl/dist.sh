#!/bin/sh

BIN="`pwd`/bin"
DIST="dist"

if ! [ -d $DIST ]; then
	mkdir $DIST; fi
	
cd $DIST

rm -rf *
mkdir lrl

for name in $BIN/*; do
	cp "$name" "lrl/`echo \`basename \"$name\"\` | tr [A-Z] [a-z]`"; done

tar cf - lrl | bzip2 -9f > lrl.tar.bz2

cd ..
