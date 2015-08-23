@del lang.h--
@echo #define LANG_ENG 1 >lang.h--

@del ttf_viewer
cls
c-- ttf_viewer.c
@rename ttf_viewer.com ttf_viewer
@del warning.txt
@del lang.h--
@pause
kpack ttf_viewer
