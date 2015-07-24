@del lang.h--
@echo #define LANG_RUS 1 >lang.h--

@del Calypte
cls
c-- Calypte.c
@kpack Calypte.com
@rename Calypte.com Calypte
@del warning.txt
@del lang.h--
@pause