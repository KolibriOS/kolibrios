//это вставить после загрузки основных библиотек

dword libtruetype = #att;

char att[23] = "/sys/lib/truetype.obj\0"; //"truetype.obj\0";


dword truetype   = #att_truetype;
dword get_length = #att_get_length;
dword get_width  = #att_get_width;
dword text_out   = #att_text_out;

dword am3__ = 0x0; 
dword bm3__ = 0x0;


char att_truetype[10]   = "truetype\0";
char att_get_length[12] = "get_length\0";
char att_get_width[11]  = "get_width\0";
char att_text_out[10]   = "text_out\0";