gcc -c -O2 -nostdinc -nostdlib -march=pentium -fomit-frame-pointer -fno-builtin -o libGUI.obj libGUI.c
strip -X --strip-unneeded libGUI.obj
@pause
