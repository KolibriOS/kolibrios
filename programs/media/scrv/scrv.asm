
;=========================================
;
; Simple program to view ZX Spectrum
; screen files.
; 6912 bytes (*.scr, *.s) 
; and
; 6929 bytes (*.$c) files are supported now.

; Простая программа для просмотра
; файлов экранов ZX Spectrum.
; Поддерживаются файлы размером 
; 6912 байт (*.scr, *.s)
; и
; 6929 байт (*.$c).
;
; author: Oleksandr Bogomaz
; e-mail: albom85@yandex.ru
; site: http://albom06.boom.ru/
;
;=========================================

use32
org	0
db	'MENUET01'
dd	1
dd	_start
dd	_end
dd	_memory
dd	_stack
dd	_param
dd	0

;=========================================

_start:

call	mem_init
call	file_getsize
call	file_read
call	file_convert
call	window_draw

_event_wait:

mov	eax, 10
int	0x40

cmp	eax, 1
je	__repaint_wnd

cmp	eax, 2
je    __key

cmp	eax, 3
je    __button

jmp  _event_wait

__repaint_wnd:
call	window_draw
jmp	_event_wait


__key:
mov	eax, 2
int	0x40

cmp	ah, 27
jne	_event_wait

jmp	__end

jmp	_event_wait

__button:

mov	eax, 17
int	0x40

cmp	ah, 1
jne	_event_wait

__end:
call	mem_free

mov	eax, -1
int	0x40



;=========================================

window_draw:
mov	eax, 12
mov	ebx, 1
int	0x40

xor	eax, eax
mov	ebx, 10*65536+290
mov	ecx, 10*65536+230
mov	edx, 0x34ffffff
mov	edi, _app_title
int	0x40

mov	eax, 65
mov	ebx, [dst]
mov	ecx, 256*65536 + 192
mov	edx, 5*65536 + 5
mov	edi, _palette
mov	esi, 8
mov	ebp, 0
int    0x40

mov	eax, 12
mov	ebx, 2
int	0x40

ret

;=========================================

mem_init:

mov	eax, 68
mov	ebx, 11
int	0x40

mov	eax, 68
mov	ebx, 12
mov	ecx, 6144
int	0x40
mov	dword [src], eax

mov	eax, 68
mov	ebx, 12
mov	ecx, 768
int	0x40
mov	dword [atr], eax

mov	eax, 68
mov	ebx, 12
mov	ecx, 256*192
int	0x40
mov	dword [dst], eax

ret

;=========================================

mem_free:

mov	eax, 68
mov	ebx, 13
mov	ecx, [src]
int	0x40

mov	eax, 68
mov	ebx, 13
mov	ecx, [atr]
int	0x40

mov	eax, 68
mov	ebx, 13
mov	ecx, [dst]
int	0x40

ret

;=========================================

file_read:

mov	eax, 70
mov	ebx, _in_f1
mov	ecx, [src]
mov	[_in_f1+16], ecx
int	0x40

mov	eax, 70
mov	ebx, _in_f2
mov	ecx, [atr]
mov	[_in_f2+16], ecx
int	0x40

ret

;=========================================

file_getsize:

mov	eax, 68
mov	ebx, 12
mov	ecx, 0x5000
int	0x40
mov	dword [tmp], eax

mov	eax, 70
mov	ebx, _in_f0
mov	ecx, [tmp]
mov	[_in_f0+16], ecx
int	0x40

mov	[fsize], ebx

mov	eax, 68
mov	ebx, 13
mov	ecx, [tmp]
int	0x40

cmp	[fsize], 6929
jne	_ok_size

mov	dword [_in_f1+4], 17
mov	dword [_in_f2+4], 6144+17

_ok_size:

ret

;=========================================

file_convert:

xor	eax, eax
mov	[I], eax
mov	[J], eax
mov	[K], eax
mov	[L], eax
mov	[M], eax
mov	[N], eax

__J:
__I:
__K:
__L:
	xor	ebx, ebx
	mov	eax, [J]
	shl	eax, 0x0b ; eax * 2048
	add	ebx, eax
	mov	eax, [K]
	shl	eax, 0x08 ; eax * 256
	add	ebx, eax
	mov	eax, [I]
	shl	eax, 0x05 ; eax * 32
	add	ebx, eax
	add	ebx, [L]
	add	ebx, [src]

	xor	eax, eax
	mov	ah, [ebx]
	mov	[C], ah

	__M:
		mov	ah, [C]
		and	ah, 128
		shr	ah, 7
		mov	[S], ah

		xor	ebx, ebx
		mov	eax, [J]
		shl	eax, 0x08 ; eax * 256
		add	ebx, eax
		mov	eax, [I]
		shl	eax, 0x05 ; eax * 32
		add	ebx, eax
		add	ebx, [L]
		add	ebx, [atr]

		xor	eax, eax
		mov	ah, [ebx]
		mov	[A], ah

		and	ah, 64
		cmp	ah, 64
		jne	__b0
		mov	[B], 8
		jmp	__OK_b
		__b0:
		mov	[B], 0
		__OK_b:

		mov	ah, [S]
		cmp	ah, 0
		jne	__1

		mov	ah, [A]
		shr	ah, 3
		and	ah, 7
		add	ah, [B]
		jmp	__OK_col

		__1:
		mov	ah, [A]
		and	ah, 7
		add	ah, [B]

		__OK_col:
		mov	ebx, [dst]
		add	ebx, [N]
		mov	[ebx], ah
		inc	[N]

		shl	[C], 1

		inc	[M]
		cmp	[M], 8
		jne	__M

		mov	[M], 0
		inc	[L]
		cmp	[L], 32
		jne	__L

		mov	[L], 0
		inc	[K]
		cmp	[K], 8
		jne	__K

		mov	[K], 0
		inc	[I]
		cmp	[I], 8
		jne	__I

		mov	[I], 0
		inc	[J]
		cmp	[J], 3
		jne	__J


ret

;=========================================

_app_title:
db 'ScrV 0.2 by O.Bogomaz', 0

_in_f0:
dd	0
dq	0
dd	0x5000
dd	0
db	0
dd	_param

_in_f1:
dd	0
dq	0
dd	6144
dd	0
db	0
dd	_param

_in_f2:
dd	0
dq	6144
dd	768
dd	0
db	0
dd	_param

_palette:
dd	0		; black
dd	0x000000b0	; blue
dd	0x00b00000	; red
dd	0x00b000b0	; magenta
dd	0x0000b000	; green
dd	0x0000b0b0	; cyan
dd	0x00b0b000	; yellow
dd	0x00b0b0b0	; gray
dd	0		; black
dd	0x000000ff	; light blue
dd	0x00ff0000	; light red
dd	0x00ff00ff	; light magenta
dd	0x0000ff00	; light green
dd	0x0000ffff	; light cyan
dd	0x00ffff00	; light yellow
dd	0x00ffffff	; white

src	dd	0
dst	dd	0
atr	dd	0
tmp	dd	0

fsize	dd	0

I	dd	0
J	dd	0
K	dd	0
L	dd	0
M	dd	0
N	dd	0

C	db	0
A	db	0
B	db	0
S	db	0

_param:
rb 256

_end:

align 32
rb 2048
_stack:
_memory:

;=========================================