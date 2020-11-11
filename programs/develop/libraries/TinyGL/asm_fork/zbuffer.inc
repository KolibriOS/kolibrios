;
; Z buffer
;

include 'zfeatures.inc'

ZB_Z_BITS equ 16

ZB_POINT_Z_FRAC_BITS equ 14

ZB_POINT_TEXEL_SIZE equ 12 ;точность множителя для вычисления координат текселя
	;влияет на максимальный размер текстуры

ZB_POINT_RED_MIN equ (1 shl 8)
ZB_POINT_RED_MAX equ ( (1 shl 16)-1 )
ZB_POINT_GREEN_MIN equ (1 shl 8)
ZB_POINT_GREEN_MAX equ ( (1 shl 16)-1 )
ZB_POINT_BLUE_MIN equ (1 shl 8)
ZB_POINT_BLUE_MAX equ ( (1 shl 16)-1 )

; display modes
ZB_MODE_5R6G5B equ 1  ; true color 16 bits
ZB_MODE_INDEX  equ 2  ; color index 8 bits
ZB_MODE_RGBA   equ 3  ; 32 bit rgba mode
ZB_MODE_RGB24  equ 4  ; 24 bit rgb mode
ZB_NB_COLORS   equ 225 ; number of colors for 8 bit display

if TGL_FEATURE_RENDER_BITS eq 24

macro RGB_TO_PIXEL r,g,b
{
	mov eax,b
	shr eax,8
	push eax
		mov eax,g
		and eax,0xff00
		or dword[esp],eax
		mov eax,r
		shl eax,8
		or dword[esp],eax
	pop eax
}

;typedef unsigned char PIXEL;
PSZB equ 3
PSZSH equ 5

else if TGL_FEATURE_RENDER_BITS eq 32

;#define RGB_TO_PIXEL(r,g,b) \
;  ((((r) << 8) & 0xff0000) | ((g) & 0xff00) | ((b) >> 8))
;typedef unsigned int PIXEL;
;PSZB equ 4
;PSZSH equ 5

else

;#error Incorrect number of bits per pixel

end if

struct ZBuffer
	xsize dd ? ;int
	ysize dd ? ;int
	linesize dd ? ;int ;line size, in bytes
	mode dd ? ;int
    
	zbuf dd ? ;*unsigned short
	pbuf dd ? ;*PIXEL
	frame_buffer_allocated dd ? ;int
    
	nb_colors dd ? ;int
	dctable dd ? ;*unsigned char
	ctable dd ? ;*int
	current_texture dd ? ;*PIXEL
	s_log2 dd ? ;unsigned int
	s_bound dd ? ;unsigned int
	t_bound dd ? ;unsigned int
ends

struct ZBufferPoint
	x dd ? ;int ;integer coordinates in the zbuffer
	y dd ? ;int
	z dd ? ;int
	s dd ? ;int ;coordinates for the mapping
	t dd ? ;int
	r dd ? ;int ;color indexes
	g dd ? ;int
	b dd ? ;int

	fsz dd ? ;float ;temporary coordinates for mapping
	tz dd ? ;float
ends

; ztriangle.c

;
; Memory allocator for TinyGL
;

; modify these functions so that they suit your needs

align 4
proc gl_free uses eax ebx ecx, mptr:dword
	mov ecx,[mptr]
	or ecx,ecx
	jz @f
		mcall 68, 13
	@@:
	ret
endp

;description:
; выделение памяти
align 4
proc gl_malloc uses ebx ecx, size:dword
	mcall 68, 12, [size]
	ret
endp

;description:
; выделение очищеной памяти
align 4
proc gl_zalloc uses ebx ecx edi, size:dword
	mov ecx,[size]
	stdcall gl_malloc,ecx
	or eax,eax
	jz @f
		mov ebx,eax
		mov edi,eax
		xor eax,eax
		shr ecx,2
		rep stosd ;очистка памяти (пишем везде 0)
		mov eax,ebx
	@@:
	ret
endp
