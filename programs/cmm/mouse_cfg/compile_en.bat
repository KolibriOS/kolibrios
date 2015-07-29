@del lang.h--
@echo #define LANG_ENG 1 >lang.h--

@del mouse_cfg
cls
c-- mouse_cfg.c
@rename mouse_cfg.com mouse_cfg
@kpack mouse_cfg
@del warning.txt
@del lang.h--
@pause