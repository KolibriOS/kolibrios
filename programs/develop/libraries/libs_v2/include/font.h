
:typedef struct
{
	char (*load)(char *path);
} _FONT_;
_FONT_ font;

static inline font_lib_init(void)
{
	if(font.load)return;
	library.load("/sys/lib/font.obj");
	font.load = library.get("font.load");
}