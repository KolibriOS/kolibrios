;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; A firework demo 
; Programmed by Yaniv LEVIATHAN 
; http://yaniv.leviathanonline.com 
; Converted to DexOS, By Dex 
; Converted to KolibriOS, By Asper
; Optimized for KolibriOS, By Diamond
; Assemble with
; c:fasm firework.asm firework.kex
; NOTE: Needs MMX & SSE 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
use32 
	org	0x0

	db	'MENUET00'	; 8 byte id
	dd	38		; required os
	dd	STARTAPP	; program start
	dd	I_END		; program image size
	dd	0x100000	; required amount of memory
	dd	0x00000000	; reserved=no extended header

include '../../../macros.inc'
include "aspapi.inc"
SCREEN_WIDTH   equ    320
SCREEN_HEIGHT  equ    200
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Global defines 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

   NUM_PARTS   = 150
   X_OFFSET   = 0
   Y_OFFSET   = 4
   X_SPEED_OFFSET   = 8
   Y_SPEED_OFFSET   = 12
   COLOR_OFFSET   = 16
   PART_SIZE   = 20

macro draw_window
{
local x, xsize, y, ysize, areacolor, caption
x = 100
y = 70
xsize = SCREEN_WIDTH+9
ysize = SCREEN_HEIGHT+4
areacolor = 0x14224466
caption = labelt
	mov	eax, 12 		; function 12:tell os about windowdraw
	mov	ebx, 1			; 1, start of draw
	int	0x40
	; DRAW WINDOW
	mov	eax, 48
	mov	ebx, 4
	int	0x40
	lea	ecx, [y*65536+ysize+eax]
	xor	eax, eax		; function 0 : define and draw window
	mov	ebx, x*65536+xsize	; [x start] *65536 + [x size]
	mov	edx, areacolor		 ; color of work area RRGGBB
	mov	edi, caption
	int	0x40
;  start_draw_window 100,70,SCREEN_WIDTH+9,SCREEN_HEIGHT+29,0x04224466,labelt;, 14;labellen-labelt
  end_draw_window
}

macro mmx_shade
{
      mov ecx, SCREEN_WIDTH*SCREEN_HEIGHT/8
      mov  edi,buffer 
      movq mm1, [sub_mask] 
.lop: 
      movq mm0, [edi] 
      psubusb mm0, mm1 
      movq [edi], mm0 
      add edi, 8 
      loop .lop 
}

macro mmx_blur_prepare
{
      mov ecx, (SCREEN_WIDTH*SCREEN_HEIGHT-330*2)/8
      mov edi,buffer + 328
}

macro mmx_blur
{
local .lop
.lop: 
      movq mm0, [edi] 
      movq mm1, [edi+1] 
      movq mm2, [edi-1] 
      movq mm3, mm0 
      movq mm4, [edi-SCREEN_WIDTH]
      movq mm5, [edi+SCREEN_WIDTH]

      pavgb mm0, mm1 ; mm0 = avg(cur,cur+1) 
      pavgb mm3, mm2 ; mm3 = avg(cur,cur-1) 
      pavgb mm4, mm5 ; mm4 = avg(cur+320,cur-320) 
      pavgb mm3, mm4 ; mm3 = avg(avg(cur,cur-1),avg(cur+320,cur-320)) 
      pavgb mm0, mm3 ; mm0 = avg(avg(cur,cur+1), 

      movq [edi], mm0 
      add edi, 8 
      loop .lop 
}


macro mmx_blur_right
{
local .lop
.lop: 
      movq mm0, [edi] 
      movq mm1, [edi+1]
      movq mm2, [edi+SCREEN_WIDTH]
      movq mm3, [edi+SCREEN_WIDTH+1]
      pavgb mm0, mm1 
      pavgb mm3, mm2 
      pavgb mm0, mm3 
      movq [edi], mm0 
      add edi, 8 
      loop .lop 
}

STARTAPP:

init_palette:
   mov edi, pal
	xor	eax, eax
red_loop:
	stosd
	stosd
	add	eax, 0x040000
	and	eax, 0xFFFFFF
	jnz	red_loop

	mov	eax, 63*4*65536
@@:
	stosd
	stosd
	add	ax, 0x0404
	jnc	@b
    
;zero_buffer: 
   mov ecx, SCREEN_WIDTH*SCREEN_HEIGHT/4
;   mov edi,buffer 
   xor eax, eax 
   rep stosd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
; Main Functions 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
virtual at esp
global_x	dd	?
global_y	dd	?
seed		dd	?
end virtual

   db 0x0f, 0x31 
	push	eax	; seed
	push	100*64	; global_y
	push	160*64	; global_x

   jmp MAIN 


red:
	mcall	9,proc_info,-1
   draw_window
MAIN:
	test	[proc_info.wnd_state], 0x04
	jnz	still
   mov ecx, NUM_PARTS 
   mov ebp, particles 
   .advance_particles: 
      mov eax, [ebp+X_OFFSET] 
      mov ebx, [ebp+Y_OFFSET] 

      sar eax, 6 
      sar ebx, 6 

      cmp eax, 5 
      jb .new_particle 
      cmp eax, SCREEN_WIDTH-5;315
      jge .new_particle 
      cmp ebx, 5 
      jb .new_particle 
      cmp ebx, SCREEN_HEIGHT-5;195
      jl .part_ok

   .new_particle: 
      call init_particle 
      jmp .advance_particles 

   .part_ok: 
;      mov edi, eax 
 ;     add edi,buffer 
;      mov eax, SCREEN_WIDTH
;      mul ebx 
	imul	edi, ebx, SCREEN_WIDTH
      mov dl, [ebp+COLOR_OFFSET] 
      mov [buffer+eax+edi], dl

;      mov eax, [ebp+X_OFFSET] 
;      mov ebx, [ebp+Y_OFFSET] 
;      add eax, [ebp+X_SPEED_OFFSET] 
;      add ebx, [ebp+Y_SPEED_OFFSET] 
;      mov [ebp+X_OFFSET], eax 
;      mov [ebp+Y_OFFSET], ebx 
	mov	eax, [ebp+X_SPEED_OFFSET]
	add	[ebp+X_OFFSET], eax
	mov	eax, [ebp+Y_SPEED_OFFSET]
	add	[ebp+Y_OFFSET], eax

      db 0x0f, 0x31 
      and al, 0x7F 
      jnz .dont_inc_y_speed 
      inc dword [ebp+Y_SPEED_OFFSET] 
      .dont_inc_y_speed: 
    
      add ebp, PART_SIZE 
   loop .advance_particles 

   mmx_shade 
; jmp .copy_buffer_to_video
   mmx_blur_prepare
   test dword [blur_right_flag] , 0x800000
   jnz .do_blur_right 
   mmx_blur 
   db 0x0f, 0x31 
   and al, 1 
   jz .blur_ok 
   jmp .dont_blur 
   .do_blur_right: 
   mmx_blur_right 
   .blur_ok: 
   add dword [blur_right_flag], 0x1000
   .dont_blur: 

   .copy_buffer_to_video: 
    ;    mov     eax, 18 ;@WAITVSYNC();
    ;    mov     ebx, 14
    ;    int     0x40

	mov	eax, 48
	mov	ebx, 4
	int	0x40
	lea	edx, [5*65536+eax]

	mov	eax, 65 ;copyfard(0xA000,0,screen,0,16000);
	mov	ebx, buffer;dword [screen]
	mov	ecx, SCREEN_WIDTH*65536+SCREEN_HEIGHT ;ecx = w*65536+h
;	mov	edx, 5*65536+25 ;edx = x*65536+y
	push	8
	pop	esi
	;mov	esi, 8
	mov	edi, pal
	xor	ebp, ebp
	int	0x40


still:
	mov	eax, 11 	    ; Test if there is an event in the queue.
	int	0x40

	dec	eax		      ; redraw request ?
	jz	red
	dec	eax		      ; key in buffer ?
	jz	key
	dec	eax		      ; button in buffer ?
	jz	button

	jmp	MAIN


key:
	mov	eax, 2
	int	0x40
;	cmp	ah, 1		    ; Test Esc in Scan
;	je	close_app
	cmp	ah, 27		    ; Test Esc in ASCII
	je	close_app
	jmp	MAIN

button:
; we have only one button, close
;	mov	eax, 17 	    ; Get pressed button code
;	int	0x40
;	cmp	ah, 1		    ; Test x button
;	je	close_app
;	jmp	MAIN
; fall through to close_app

fail:
	; Type something here.
close_app:
    mov  eax,-1 		 ; close this program
    int  0x40

init_particle: 
   db 0x0f, 0x31 
   and al, 0x1F 
jnz .dont_re_init_globals 
   ; init x 
   call rand 
   cdq
   ;xor dx, dx 
   mov ebx, SCREEN_WIDTH
   div ebx 
   shl edx, 6 
   mov [4+global_x], edx 
   ; init y 
   call rand 
   cdq
   ;xor dx, dx 
   mov ebx, SCREEN_HEIGHT
   div ebx
   shl edx, 6
   mov [4+global_y], edx 
.dont_re_init_globals: 
   ; init x 
   mov eax, [4+global_x] 
   mov [ebp+X_OFFSET], eax 
   ; init y 
   mov eax, [4+global_y] 
   mov [ebp+Y_OFFSET], eax 
   ; init x speed 
   call rand 
   and eax, 31 
   sub eax, 15 
   ;shl ax, 6 
   mov [ebp+X_SPEED_OFFSET], eax 
   ; init y speed 
   call rand 
   and eax, 31 
   sub eax, 15 
   ;shl ax, 6
   mov [ebp+Y_SPEED_OFFSET], eax 
   ; init color 
;   mov ax, 255 
   ;call rand
   ;and ax, 0xFF
   mov [ebp+COLOR_OFFSET], dword 255;ax 
   ret 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Misc. Functions 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 


rand: 
      mov eax, [8+seed] 
      imul eax, 214013 
      add eax, 2531011 
      mov [8+seed], eax 
      shr eax, 16 
      ret 

; DATA AREA

; Application Title
labelt          db      'Firework demo',0
;labelt		db	'Matrix demo',0

;seed:	     dd 0
;global_x:    dd 160*64 
;global_y:    dd 100*64 
sub_mask:    dd 0x01010101, 0x01010101 
;                             x, y, x_speed, y_speed, color 
particles: times NUM_PARTS dd 0, 0, 0,	      0,       0 
blur_right_flag: dd 0 
;include 'Dex.inc'
I_END:
proc_info	process_information
pal	     rb 256*4	;dup(0)
;pal             dd      256   dup(0)
;buffer       rb 1024*64
buffer	     rb SCREEN_WIDTH*SCREEN_HEIGHT

