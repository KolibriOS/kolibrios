#define printf
class TKlbrGraphDraw : public TBaseGraphDraw<TKlbrGraphDraw>
{
	unsigned long cur_color;
	int quit;
	typedef TBaseGraphDraw<TKlbrGraphDraw> TGraphDraw;
public:
	TKlbrGraphDraw(const char *s = 0) : TGraphDraw(s) {}
	int GetStatus() {return 1;}
	int Run(int evmask = 0, int w = INT_MIN, int h = INT_MIN);
	int SetColor(unsigned long c) {cur_color=c;return 1;}
	int DrawLine(int x0, int y0, int x1, int y1);
	int DrawText(int x0, int y0, char* text);
	int IsDraw(void) {return 1;}
	int DrawClear();
	unsigned long CreateColor(unsigned short red,
			unsigned short green, unsigned short blue);
	void GetSize(int &w, int &h);
	int GetTextH(const char *s) {return 10;}
	int GetTextW(const char *s) {return 6 * strlen(s);}
	void Quit(int q = 1) {quit = q;}
};
int TKlbrGraphDraw::DrawLine(int x0, int y0, int x1, int y1)
{
	__asm__ __volatile__("int $0x40" :: "a"(38), "b"((x0 << 16) + x1), "c"((y0 << 16) + y1), "d"(cur_color));
	return 1;
}
int TKlbrGraphDraw::DrawText(int x0, int y0, char* text)
{
	__asm__ __volatile__("int $0x40" :: "a"(4), "b"((x0 << 16) + y0), "c"(cur_color | 0xC0000000), "d"(text), "D"(0xFFFFFF));
	return 1;
}
int TKlbrGraphDraw::DrawClear(void)
{
	int w,h;
	GetSize(w,h);
	__asm__ __volatile__("int $0x40" :: "a"(13), "b"(w), "c"(h), "d"(0xFFFFFF));
	return 1;
}
unsigned long TKlbrGraphDraw::CreateColor(unsigned short red,
                          unsigned short green, unsigned short blue)
{
  return (unsigned long)(blue >> 8) + ((unsigned long)(green >> 8) << 8) +
         ((unsigned long)(red >> 8) << 16);
}
void TKlbrGraphDraw::GetSize(int &w, int &h)
{
	char buffer[1024];
	__asm__ __volatile__("int $0x40" :: "a"(9), "b"(buffer), "c"(-1));
	w = *(int*)(buffer + 62);
	h = *(int*)(buffer + 66);
}
#define XK_Left		0xB0
#define XK_Right	0xB3
#define XK_Up		0xB2
#define XK_Down		0xB1
#define XK_Return	0x0D
#define XK_space	0x20
#define XK_Escape	0x1B
#define XK_less		'<'
#define XK_comma	','
#define XK_period	'.'
#define XK_greater	'>'
#define XK_minus	'-'
#define XK_equal	'='
#define XK_underscore	'_'
#define XK_plus		'+'
#define XK_Delete	0xB6
#define XK_F8		0x39
#define XK_l		'l'
#define XK_L		'L'
#define XK_F2		0x33
#define XK_s		's'
#define XK_S		'S'
#define XK_slash	'/'
#define XK_question	'?'
#define XK_n		'n'
#define XK_N		'N'
#define XK_t		't'
#define XK_T		'T'
#define XK_r		'r'
#define XK_R		'R'
#define XK_b		'b'
#define XK_B		'B'
#define XK_f		'f'
#define XK_F		'F'
int TKlbrGraphDraw::Run(int evmask, int w, int h)
{
	__asm__ __volatile__("int $0x40" :: "a"(40), "b"(0x27)); // enable mouse events
	quit = 0;
	char buffer[1024];
	int event = 1;
	int skinHeight;
	int prev_mouse_btn = 0;
	for (;;) {
		switch (event) {
		case 1:
			__asm__ __volatile__("int $0x40" :: "a"(12), "b"(1));
			__asm__ __volatile__("int $0x40" : "=a"(skinHeight) : "a"(48), "b"(4));
			__asm__ __volatile__("int $0x40" :: "a"(0), "b"(w+9), "c"(h+skinHeight+4), "d"(0x33FFFFFF), "S"(0), "D"(title));
			{
				TGraphDraw::event ev;
				ev.type = TGraphDraw::event::draw;
				ev.any.drw = this;
				evfunc(ev);
			}
			__asm__ __volatile__("int $0x40" :: "a"(12), "b"(2));
			break;
		case 2:
			{
				int keyCode;
				__asm__ __volatile__("int $0x40" : "=a"(keyCode) : "a"(2));
				if (!(keyCode & 0xFF)) {
					TGraphDraw::event ev;
					ev.type = TGraphDraw::event::key_down;
					ev.any.drw = this;
					ev.key.k = (keyCode >> 8) & 0xFF;
					evfunc(ev);
				}
			}
			break;
		case 3:
			return 0;
		case 6:
			{
				int mouse_btn;
				int mouse_pos;
				__asm__ __volatile__("int $0x40" : "=a"(mouse_btn) : "a"(37), "b"(2));
				mouse_btn &= 1;
				if (prev_mouse_btn == mouse_btn)
					break;
				prev_mouse_btn = mouse_btn;
				__asm__ __volatile__("int $0x40" : "=a"(mouse_pos) : "a"(37), "b"(1));
				TGraphDraw::event ev;
				ev.type = mouse_btn ? TGraphDraw::event::button_down : TGraphDraw::event::button_up;
				ev.any.drw = this;
				ev.button.n = 1;
				ev.button.x = mouse_pos >> 16;
				ev.button.y = (short)mouse_pos;
				evfunc(ev);
			}
			break;
		}
		if (quit)
			return quit;
		__asm__ __volatile__("int $0x40" : "=a"(event) : "a"(10));
	}
}
