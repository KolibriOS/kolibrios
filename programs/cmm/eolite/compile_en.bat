@del lang.h--
@echo #define LANG_ENG 1 >lang.h--

@del Eolite
cls
..\C--\c-- Eolite.c
@rename Eolite.com Eolite
@del warning.txt
@del lang.h--
@pause