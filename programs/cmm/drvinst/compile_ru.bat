@del lang.h--
@echo #define LANG_RUS 1 >lang.h--

@del *.kex
@c-- drvinst.c
@rename *.com *.kex
@del warning.txt
@del lang.h--

@pause