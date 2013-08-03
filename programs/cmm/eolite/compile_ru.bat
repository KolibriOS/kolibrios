@del lang.h
@echo #define LANG_RUS 1 >lang.h--

@del Eolite
..\C--\c-- Eolite.c
@rename Eolite.com Eolite
@del warning.txt
@pause