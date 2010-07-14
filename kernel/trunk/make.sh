#!/bin/bash
# This script does for linux the same as build.bat for DOS,
# it compiles the KoOS kernel, hopefully ;-)

CLANG=$1;

# set debug=true to print executed bash commands
debug=true

outDir=bin
outFileName=kernel.mnt
outFile=$outDir/$outFileName

usage()
{
   echo "Usage: make.sh [en|ru|ge|et]"
   exit 1
}

compile()
{
   if [ -d "$outDir" ]; then
      $debug && echo "rm -f $outFile"
            rm -f $outFile
   else
      $debug && echo "mkdir $outDir"
            mkdir $outDir
   fi

   $debug && echo "fasm -m 65536 kernel.asm $outFile"
         fasm -m 65536 kernel.asm $outFile

   $debug && echo "rm -f lang.inc"
         rm -f lang.inc

   $debug && echo "exit 0"
         exit 0
}

if [ ! $CLANG ] ; then
   usage
fi

for i in "en" "ru" "ge" "et"; do
   if [ $i == $CLANG ] ; then
      $debug && echo "echo \"lang fix \$i\" > lang.inc"
            echo "lang fix $i" > lang.inc
      compile
   fi
done
usage
