
typedef Dword dword;

dword  am__ = 0x0;
dword  bm__ = 0x0;

char aEdit_box_draw[9]  = "edit_box";
char aEdit_box_key[13] = "edit_box_key";
char aEdit_box_mouse[15] = "edit_box_mouse";
char aVersion_ed[11] = "version_ed";

char aCheck_box_draw  [15] = "check_box_draw";
char aCheck_box_mouse [16] = "check_box_mouse";
char aVersion_ch      [11] = "version_ch";

char aOption_box_draw [16] = "option_box_draw";
char aOption_box_mouse[17] = "option_box_mouse";
char aVersion_op      [11] = "version_op" ;

//BOX_LIB

typedef dword __stdcall dword_func(dword);
//typedef dword __stdcall dword3_func(dword,dword,dword);

dword_func *edit_box_draw =(dword_func*) &aEdit_box_draw;
dword_func *edit_box_key =(dword_func*) &aEdit_box_key;
dword_func *edit_box_mouse =(dword_func*) &aEdit_box_mouse;

//char lib_path[] = "/sys/lib/box_lib.obj";
char lib_path[] = "/sys/lib/box_lib.obj";
dword lib_path_addr = (dword)lib_path;
dword dummy = 0;


struct edit_box{
dword width, left, top, color, shift_color, focus_border_color, blur_border_color,
text_color, max, text, mouse_variable, flags, size, pos, offset, cl_curs_x, cl_curs_y, shift, shift_old;
};

void load_edit_box()
{
	kol_struct_import *k = kol_cofflib_load(lib_path);

	if (k == NULL)
	{
		sprintf(debuf, "cannot load library %S", lib_path);
		rtlDebugOutString(debuf);
		return;
	}

	edit_box_draw = (dword_func*)kol_cofflib_procload(k, "edit_box");
	edit_box_key = (dword_func*)kol_cofflib_procload(k,"edit_box_key");
	edit_box_mouse = (dword_func*)kol_cofflib_procload(k,"edit_box_mouse");
	kos_SetMaskForEvents(0x27);

	if (edit_box_draw == NULL || edit_box_key == NULL || edit_box_mouse == NULL)
		rtlDebugOutString("some of functions cannot be loaded!");
}