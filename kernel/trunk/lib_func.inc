$Revision$
;============================================================================
;
;         Загрузка ссылки на внешние функции
;
;
;
;
;
;
;============================================================================

align 4
@IMPORT:

library \
	libini,'libini.obj'

import	libini, \
	lib_init,'lib_init',\
	ini.get_str,'ini.get_str',\
	ini.enum_keys,'ini.enum_keys',\
	ini.get_int,'ini.get_int'