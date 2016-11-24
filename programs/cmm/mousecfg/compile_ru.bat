@del lang.h--
@echo #define LANG_RUS 1 >lang.h--

@del mousecfg
cls
@c-- mousecfg.c
@rename mousecfg.com mousecfg
@kpack mousecfg
@del warning.txt
@del lang.h--
@pause