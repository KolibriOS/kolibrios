@del lang.h--
@echo #define LANG_ENG 1 >lang.h--

@c-- dl.c
@del dl
@rename dl.com dl
@del warning.txt
@del lang.h--

if not exist dl ( @pause )