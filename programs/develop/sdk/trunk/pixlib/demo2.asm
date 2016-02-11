
include '../../../../proc32.inc'

struc system_colors
{
  .frame            dd ?
  .grab             dd ?
  .work_3d_dark      dd ?
  .work_3d_light dd ?
  .grab_text        dd ?
  .work             dd ?
  .work_button      dd ?
  .work_button_text dd ?
  .work_text        dd ?
  .work_graph       dd ?
}

use32

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd 0
dd 0

include 'pixlib.inc'

align 4
start:
           call load_pxlib
           test eax, eax
           jz .fail

           sub esp, 1024

           mov eax, 9
           mov ebx, esp
           mov ecx, -1
           int 0x40

           movzx ecx, word [esp+0x1E]
           mov eax, 18
           mov ebx, 21
           int 0x40

           mov [slot], eax

           add esp, 1024

           mov  eax,48                      ; get system colors
           mov  ebx,3
           mov  ecx,sc
           mov  edx,10*4
           int 0x40

           CreateHatch HS_CROSS, 0xFFFFFFFF, 0xFF000000
           mov [br_cross], eax

           CreateHatch HS_DIAGCROSS, 0xFFFFFFFF, 0xFF000000
           mov [br_dcross], eax

           CreateHatch HS_HORIZONTAL, 0xFFFFFFFF, 0xFF000000
           mov [br_horz], eax

           CreateHatch HS_VERTICAL, 0xFFFFFFFF, 0xFF000000
           mov [br_vert], eax

           CreateHatch HS_FDIAGONAL, 0xFFFFFFFF, 0xFF000000
           mov [br_fd], eax

           CreateHatch HS_BDIAGONAL, 0xFFFFFFFF, 0xFF000000
           mov [br_bd], eax

.redraw:
           call draw_window

.wait_event:

           mov eax, 18
           mov ebx, 7
           int 0x40
           cmp eax, [slot]
           jne .skip_draw

           sub esp, 1024

           mov eax, 9
           mov ebx, esp
           mov ecx, -1
           int 0x40

           mov edx, [esp+0x22]; xwin
           mov ecx, [esp+0x26]; ywin
           add edx, [esp+0x36]
           add ecx, [esp+0x3A]

           mov eax, [esp+0x3E]  ;width
           mov ebx, [esp+0x42]  ;height

           add esp, 1024

           cmp eax, 50
           jle .skip_draw

           cmp ebx, 40
           jle .skip_draw

           push ebx
           push eax
           push ecx
           push edx
           call _Draw
           add esp, 16

.skip_draw:

           mov ebx, 1000
           mov eax, 23
           int 0x40

        ;   mov eax, 11
        ;   int 0x40

           dec eax                     ;   if event = 1
        ;  js .wait_event

           jz  .redraw                 ;   jump to redraw handler
           dec eax                     ;   else if event = 2
           jz  .key                    ;   jump to key handler
           dec eax
           jz  .button

           jmp .wait_event

.button:                               ; button event handler
           mov al, 17                  ;   get button identifier
           int 0x40

           cmp ah, 1
           jne .wait_event             ;   return if button id != 1
.exit:
                                       ; restore old screen and cleanup
.fail:
           or eax, -1                  ;   exit application
           int 0x40
.key:                                  ; key event handler
           mov al, 2                   ;   get key code
           int 0x40

           jmp .wait_event

draw_window:
           mov eax, 12                 ; start drawing
           mov ebx, 1
           int 0x40

           xor  eax, eax               ; create and draw the window
           mov  ebx, 100*65536+320     ; (window_cx)*65536+(window_sx)
           mov  ecx, 100*65536+240     ; (window_cy)*65536+(window_sy)
           mov  edx, [sc.work]         ; work area color
           or   edx, 0x33000000        ; & window type 3
           mov  edi, title             ; window title
           int  0x40

           mov  eax, 12                ; finish drawing
           mov  ebx, 2
           int  0x40

           ret

DWORD equ dword
PTR   equ

_Draw:
        push    ebp
	push	edi
	push	esi
	push	ebx
	mov	ebx, 1431655766
	sub	esp, 44
	mov	esi, DWORD PTR [esp+72]
	mov	ecx, DWORD PTR [esp+76]
	mov	DWORD PTR [esp+24], -16777216
	mov	DWORD PTR [esp], -1
	lea	edi, [esi-40]
	mov	eax, edi
	imul	ebx
	sar	edi, 31
	lea	eax, [ecx-30]
	sub	ecx, 10
	mov	ebp, edx
	sub	ebp, edi
	mov	edi, eax
	shr	edi, 31
	add	edi, eax
	lea	edx, [esi-20]
	lea	eax, [ebp+ebp*2]
	sub	edx, eax
	mov	esi, edx
	shr	esi, 31
	add	esi, edx
        sar     esi, 1
	add	esi, DWORD PTR [esp+64]
        sar     edi, 1
	lea	eax, [ebp+10+esi]
	mov	DWORD PTR [esp+32], eax
	lea	eax, [edi+edi]
	sub	ecx, eax
	mov	ebx, ecx
	shr	ebx, 31
	add	ebx, ecx
        sar     ebx, 1
	add	ebx, DWORD PTR [esp+68]
	lea	edx, [esi+20+ebp*2]
	mov	DWORD PTR [esp+36], edx
	lea	eax, [ebx+10+edi]
	mov	DWORD PTR [esp+40], eax

        FillRect -1, esi, ebx,ebp, edi, [br_fd], 0xFF000000

        mov     edx, DWORD PTR [esp+32]
        FillRect -1, edx, ebx, ebp, edi, [br_cross], 0xFF000000

	mov	eax, DWORD PTR [esp+36]
        FillRect -1, eax, ebx, ebp, edi, [br_horz],0xFF000000

	mov	edx, DWORD PTR [esp+40]
        FillRect -1, esi, edx, ebp, edi, [br_bd], 0xFF000000

	mov	edx, DWORD PTR [esp+32]
	mov	eax, DWORD PTR [esp+40]
        FillRect -1, edx, eax, ebp, edi, [br_dcross], 0xFF000000

	mov	edx, DWORD PTR [esp+36]
	mov	eax, DWORD PTR [esp+40]
        FillRect -1, edx, eax, ebp, edi, [br_vert], 0xFF000000

        call    [imp_FillRect]


	add	esp, 44
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
        ret

align 4


count       dd  0

title       db  'Hatches',0

i_end:

align 4

slot         rd 1

br_fd        rd 1
br_bd        rd 1
br_cross     rd 1
br_dcross    rd 1
br_horz      rd 1
br_vert      rd 1

sc   system_colors

align 16


rb 2048  ;stack
mem:

