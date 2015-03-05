#!/bin/sh
red=`tput setaf 2`
error=`tput setaf 1`
reset=`tput sgr0`
DROPBOX_UPDF="/home/leency/Dropbox/updf"

echo "${red}Removing kos_main.o...${reset}"
rm build/kos_main.o
rm build/mupdf

echo "${red}Building updf...${reset}"
make

echo "${red}Converting to KolibriOS binnary...${reset}"
cd build
objcopy -O binary mupdf

if [ -f /home/leency/Dropbox/updf ]; then
    echo "${red}Removing mypdf from Dropbox...${reset}"
    rm $DROPBOX_UPDF
fi

echo "${red}Copying new file to Dropbox...${reset}"
cp mupdf $DROPBOX_UPDF

if [ ! -f $DROPBOX_UPDF ]; then
    echo "${error}Compilation error${reset}"
    $SHELL
fi
