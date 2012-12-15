del lang.h
echo #define LANG_RUS 1 >lang.h--

..\C--\C-- tmpdisk.c /lst
del tmpdisk
rename tmpdisk.com tmpdisk
rem ..\C--\kpack tmpdisk
pause
del warning.txt
