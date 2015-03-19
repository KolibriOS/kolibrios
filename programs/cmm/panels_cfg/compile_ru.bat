@del lang.h--
@echo #define LANG_RUS 1 >lang.h--

@del panels_cfg
cls
c-- panels_cfg.c
@rename panels_cfg.com panels_cfg
@kpack panels_cfg
@del warning.txt
@pause