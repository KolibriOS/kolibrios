unsigned long cur_color;
class TKlbrGraphDraw : public TGraphDraw
{
public:
	virtual int SetColor(unsigned long c) {cur_color=c;return 1;}
	virtual int DrawLine(int x0, int y0, int x1, int y1);
	virtual int DrawText(int x0, int y0, char* text);
	virtual int IsDraw(void) {return 1;}
	virtual int DrawClear();
	virtual unsigned long CreateColor(unsigned short red,
			unsigned short green, unsigned short blue);
	virtual void GetSize(int &w, int &h);
	virtual int GetTextH(const char *s) {return 10;}
	virtual int GetTextW(const char *s) {return 6 * strlen(s);}
	virtual void Quit(int q = 1) {CloseWindow();}
};
int TKlbrGraphDraw::DrawLine(int x0, int y0, int x1, int y1)
{
	asm	mov	ebx, x0
	asm	shl	ebx, 16
	asm	add	ebx, x1
	asm	mov	ecx, y0
	asm	shl	ecx, 16
	asm	add	ecx, y1
	asm	mov	edx, [cur_color]
	asm	push	38
	asm	pop	eax
	asm	int	40h
	return 1;
}
int TKlbrGraphDraw::DrawText(int x0, int y0, char* text)
{
	asm	mov	ebx, x0
	asm	shl	ebx, 16
	asm	add	ebx, y0
	asm	mov	ecx, [cur_color]
	asm	or	ecx, 0xC0000000
	asm	mov	edx, text
	asm	mov	edi, 0xFFFFFF
	asm	push	4
	asm	pop	eax
	asm	int	40h
	return 1;
}
int TKlbrGraphDraw::DrawClear(void)
{
	int w,h;
	GetSize(w,h);
	asm	mov	ebx, w
	asm	mov	ecx, h
	asm	mov	edx, 0xFFFFFF
	asm	push	13
	asm	pop	eax
	asm	int	40h
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
	int width, height;
	asm	sub	esp, 1024
	asm	mov	ebx, esp
	asm	or	ecx, -1
	asm	push	9
	asm	pop	eax
	asm	int	40h
	asm	mov	eax, [esp+62]
	asm	mov	width, eax
	asm	mov	eax, [esp+66]
	asm	mov	height, eax
	asm	add	esp, 1024
	w = width;
	h = height;
}
