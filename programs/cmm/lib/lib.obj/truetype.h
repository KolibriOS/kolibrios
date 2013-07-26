//это вставить после загрузки основных библиотек
 
char fontinfo[28]; 
 
dword libtruetype = #att;
 
char att[] = "/sys/lib/truetype.obj"; //"truetype.obj\0";
 
dword truetype = #att_truetype;
dword get_length = #att_get_length;
dword get_width  = #att_get_width;
dword text_out = #att_text_out;
dword init_font = #att_init_font; 
$DD 2 dup 0
 
//import  libimg                     , \ 
 
char att_truetype[] = "truetype";
char att_get_length[] = "get_length";
char att_get_width[] = "get_width";
char att_text_out[] = "text_out";
char att_init_font[] = "init_font";
 
//load_dll2(libtruetype, #truetype,0);
 
//а это - в момент отрисовки окна
//text_out stdcall (#text, -1, 40, 0xFF0000, 100, 100);
