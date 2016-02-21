@del lang.h--
@echo #define LANG_ENG 1 >lang.h--

@C-- pixie.c
@del pixie
@kpack pixie.com
@rename pixie.com pixie
@del warning.txt
@del lang.h--
@pause