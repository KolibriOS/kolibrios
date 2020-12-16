#!/bin/sh
ok=`tput setaf 2`
error=`tput setaf 1`
reset=`tput sgr0`

clear
rm build/kos_main.o
rm build/mupdf
make
if [ ! -f build/mupdf ]; then
    echo "${error} Compilation error ${reset}"
    $SHELL
fi
echo "${ok} OK ${reset}"
objcopy -O binary build/mupdf
rm updf
cp build/mupdf updf
ncftpput -u xxxx -p xxxx kolibri-n.org /public_ftp ~/Desktop/updf/updf
if [ $? -ne 0 ]; then echo \"Upload failed\"; fi 




