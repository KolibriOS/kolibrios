set_img:
    db	    0, 0, 1, 0, 0, 0
    db	    0, 0, 0, 1, 0, 0
    db	    1, 0, 0, 1, 0, 0
    db	    0, 1, 1, 1, 0, 0
    db	    0, 0, 0, 0, 1, 0
    db	    0, 0, 0, 0, 0, 1
;-------------------------------------------------------------------------------
cfg_app:
    db	    "dockycfg", 0
;-------------------------------------------------------------------------------
img_data:
 .file_name:
    db	    "/sys/iconstrp.png", 0
;-------------------------------------------------------------------------------
ini_data:
 .file_name:
    db	    "/sys/settings/Docky.ini", 0
 .path_name:
    db	    "path", 0
 .param_name:
    db	    "param", 0
 .icon_name:
    db	    "icon", 0
 .separator_name:
    db	    "$", 0

 .settings_name:
    db	    "@", 0
 .location_name:
    db	    "location", 0
;-------------------------------------------------------------------------------
load_lib_start:
    lib1    l_libs img.name,	     \
		   sys_path,	     \
		   file_name,	     \
		   img.dir,	     \
		   error,	     \
		   error,	     \
		   img, 	     \
		   error,	     \
		   error

    lib2    l_libs ini.name,	     \
		   sys_path,	     \
		   file_name,	     \
		   ini.dir,	     \
		   error,	     \
		   error,	     \
		   ini, 	     \
		   error,	     \
		   error
load_lib_end:
;-------------------------------------------------------------------------------
img:
 .init	   \
    dd	    .init_T
 .toRGB    \
    dd	    .toRGB_T
 .decode   \
    dd	    .decode_T
 .destroy  \
    dd	    .destroy_T

    dd	    0, 0

 .init_T:
    db	    "lib_init", 0
 .toRGB_T:
    db	    "img_to_rgb2", 0
 .decode_T:
    db	    "img_decode", 0
 .destroy_T:
    db	    "img_destroy", 0

 .dir:
    db	    "/sys/lib/"
 .name:
    db	    "libimg.obj", 0
;-------------------------------------------------------------------------------
ini:
 .init	   \
    dd	    .init_T
 .sections \
    dd	    .sections_T
 .get_int  \
    dd	    .get_int_T
 .get_str  \
    dd	    .get_str_T

    dd	    0, 0

 .init_T:
    db	    "lib_init", 0
 .sections_T:
    db	    "ini_enum_sections", 0
 .get_int_T:
    db	    "ini_get_int", 0
 .get_str_T:
    db	    "ini_get_str", 0

 .dir:
    db	    "/sys/lib/"
 .name:
    db	    "libini.obj", 0
;-------------------------------------------------------------------------------
ICON_SIZE	 equ  32 * 32
ICON_SIZE_BGR	 equ  ICON_SIZE * 3
ICON_SIZE_RGB	 equ  ICON_SIZE * 4

BUTTON_SIZE	 equ  44
;-------------------------------------------------------------------------------
__dataend: