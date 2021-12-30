;-------------------------------------------------------------------------------
ini_data:
 .file_name      db    "/sys/settings/Docky.ini", 0
 .path_name      db    "path", 0
 .param_name     db    "param", 0
 .icon_name      db    "icon", 0
 .separator_name db    "$", 0

 .settings_name  db    "@", 0
 .location_name  db    "location", 0
 .fsize_name     db    "fsize", 0
 .ashow_name     db    "ashow", 0
;-------------------------------------------------------------------------------
load_lib_start:
  lib2 l_libs ini.name, file_name, ini.dir, ini
load_lib_end:
;-------------------------------------------------------------------------------
ini:
 .init           dd    .init_T
 .sections       dd    .sections_T
 .get_int        dd    .get_int_T
 .get_str        dd    .get_str_T

    dd 0,0

 .init_T         db    "lib_init", 0
 .sections_T     db    "ini_enum_sections", 0
 .get_int_T      db    "ini_get_int", 0
 .get_str_T      db    "ini_get_str", 0

 .dir            db    "/sys/lib/"
 .name           db    "libini.obj", 0
;-------------------------------------------------------------------------------
ICON_SIZE       equ   32 * 32
BUTTON_SIZE     equ   44
;-------------------------------------------------------------------------------
__dataend: