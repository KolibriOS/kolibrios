;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


LOAD_FROM_FILE  equ 0
LOAD_FROM_MEM   equ 1
LOAD_INDIRECT   equ 2
LOAD_SYSTEM     equ 3

struc BITMAPINFOHEADER {
  .biSize          dd ? ; DWORD
  .biWidth         dd ? ; LONG
  .biHeight        dd ? ; LONG
  .biPlanes        dw ? ; WORD
  .biBitCount      dw ? ; WORD
  .biCompression   dd ? ; DWORD
  .biSizeImage     dd ? ; DWORD
  .biXPelsPerMeter dd ? ; LONG
  .biYPelsPerMeter dd ? ; LONG
  .biClrUsed       dd ? ; DWORD
  .biClrImportant  dd ? ; DWORD
}

virtual at 0
  BI BITMAPINFOHEADER
end virtual

align 4
proc vesa_init_cursor stdcall, dst:dword, src:dword
           locals
             rBase    dd ?
             pQuad    dd ?
             pBits    dd ?
             pAnd     dd ?
             width    dd ?
             height   dd ?
             counter  dd ?
           endl

           mov esi, [src]
           add esi,[esi+18]
           mov eax,esi

           cmp [esi+BI.biBitCount], 24
           je .img_24
           cmp [esi+BI.biBitCount], 8
           je .img_8
           cmp [esi+BI.biBitCount], 4
           je .img_4

.img_2:
           add eax, [esi]
           mov [pQuad],eax
           add eax,8
           mov [pBits],eax
           add eax, 128
           mov [pAnd],eax
           mov eax,[esi+4]
           mov [width],eax
           mov ebx,[esi+8]
           shr ebx,1
           mov [height],ebx

           mov edi, [dst]
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pQuad]
.l21:
           mov ebx, [pBits]
           mov ebx, [ebx]
           bswap ebx
           mov eax, [pAnd]
           mov eax, [eax]
           bswap eax
           mov [counter], 32
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           xor ecx, ecx
           shl ebx,1
           setc cl
           mov ecx, [esi+ecx*4]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           add edi, 4
           dec [counter]
           jnz @B

           add [pBits], 4
           add [pAnd], 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l21
           ret

.img_4:
           add eax, [esi]
           mov [pQuad],eax
           add eax,64
           mov [pBits],eax
           add eax, 0x200
           mov [pAnd],eax
           mov eax,[esi+4]
           mov [width],eax
           mov ebx,[esi+8]
           shr ebx,1
           mov [height],ebx

           mov edi, [dst]
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pQuad]
           mov ebx, [pBits]
.l4:
           mov eax, [pAnd]
           mov eax, [eax]
           bswap eax
           mov [counter], 16
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           movzx ecx, byte [ebx]
           and cl, 0xF0
           shr ecx, 2
           mov ecx, [esi+ecx]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           movzx ecx, byte [ebx]
           and cl, 0x0F
           mov ecx, [esi+ecx*4]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi+4], edx

           inc ebx
           add edi, 8
           dec [counter]
           jnz @B

           add [pAnd], 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l4
           ret
.img_8:
           add eax, [esi]
           mov [pQuad],eax
           add eax,1024
           mov [pBits],eax
           add eax, 1024
           mov [pAnd],eax
           mov eax,[esi+4]
           mov [width],eax
           mov ebx,[esi+8]
           shr ebx,1
           mov [height],ebx

           mov edi, [dst]
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pQuad]
           mov ebx, [pBits]
.l81:
           mov eax, [pAnd]
           mov eax, [eax]
           bswap eax
           mov [counter], 32
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           movzx ecx,  byte [ebx]
           mov ecx, [esi+ecx*4]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           inc ebx
           add edi, 4
           dec [counter]
           jnz @B

           add [pAnd], 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l81
           ret
.img_24:
           add eax, [esi]
           mov [pQuad],eax
           add eax, 0xC00
           mov [pAnd],eax
           mov eax,[esi+BI.biWidth]
           mov [width],eax
           mov ebx,[esi+BI.biHeight]
           shr ebx,1
           mov [height],ebx

           mov edi, [dst]
           add edi, 32*31*4
           mov [rBase],edi

           mov esi,[pAnd]
           mov ebx, [pQuad]
.row_24:
           mov eax, [esi]
           bswap eax
           mov [counter], 32
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           mov ecx, [ebx]
           and ecx, 0x00FFFFFF
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx
           add ebx, 3
           add edi, 4
           dec [counter]
           jnz @B

           add esi, 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .row_24
           ret
endp

align 4
proc set_cursor stdcall, hcursor:dword
           mov eax, [hcursor]
           cmp [eax+CURSOR.magic], 'CURS'
           jne .fail
;           cmp [eax+CURSOR.size], CURSOR_SIZE
;           jne .fail
           mov ebx, [current_slot]
           xchg eax, [ebx+APPDATA.cursor]
           ret
.fail:
           mov eax, [def_cursor]
           mov ebx, [current_slot]
           xchg eax, [ebx+APPDATA.cursor]
           ret
endp

; param
;  eax= pid
;  ebx= src
;  ecx= flags

vesa_cursor:
.src     equ esp
.flags   equ esp+4
.hcursor equ esp+8

           sub esp, 4          ;space for .hcursor
           push ecx
           push ebx

           mov ebx, eax
           mov eax, CURSOR_SIZE
           call create_kernel_object
           test eax, eax
           jz .fail

           mov [.hcursor],eax

           xor ebx, ebx
           mov [eax+CURSOR.magic], 'CURS'
           mov [eax+CURSOR.destroy], destroy_cursor
           mov [eax+CURSOR.hot_x], ebx
           mov [eax+CURSOR.hot_y], ebx

           stdcall kernel_alloc, 0x1000
           test eax, eax
           jz .fail

           mov edi, [.hcursor]
           mov [edi+CURSOR.base], eax

           mov esi, [.src]
           mov ebx, [.flags]
           cmp bx, LOAD_INDIRECT
           je .indirect

           movzx ecx, word [esi+10]
           movzx edx, word [esi+12]
           mov [edi+CURSOR.hot_x], ecx
           mov [edi+CURSOR.hot_y], edx

           stdcall vesa_init_cursor, eax, esi
           mov eax, [.hcursor]
.fail:
           add esp, 12
           ret
.indirect:
           shr ebx, 16
           movzx ecx, bh
           movzx edx, bl
           mov [eax+CURSOR.hot_x], ecx
           mov [eax+CURSOR.hot_y], edx

           xchg edi, eax
           mov ecx, 1024
           cld
           rep movsd
           add esp, 12
           ret

align 4
proc load_cursor stdcall, src:dword, flags:dword
           locals
             handle  dd ?
           endl

           xor eax, eax
           mov [handle], eax
           cmp word [flags], LOAD_FROM_FILE
           jne @F

           stdcall load_file, [src]
           test eax, eax
           jz .exit
           mov [src], eax
@@:
           mov eax, [CURRENT_TASK]
           shl eax, 5
           mov eax, [CURRENT_TASK+eax+4]
           mov ebx, [src]
           mov ecx, [flags]
           call [create_cursor]    ;eax, ebx, ecx
           mov [handle], eax
.fail:
           cmp word [flags], LOAD_FROM_FILE
           jne .exit
           stdcall kernel_free, [src]
.exit:
           mov eax, [handle]
           ret
endp

align 4
proc delete_cursor stdcall, hcursor:dword
           locals
             hsrv       dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
           endl

           mov esi, [hcursor]
           cmp [esi+CURSOR.magic], 'CURS'
           jne .fail
;           cmp [esi+CURSOR.size], CURSOR_SIZE
;           jne .fail

           mov ebx, [CURRENT_TASK]
           shl ebx, 5
           mov ebx, [CURRENT_TASK+ebx+4]
           cmp ebx, [esi+CURSOR.pid]
           jne .fail

           mov ebx, [current_slot]
           cmp esi, [ebx+APPDATA.cursor]
           jne @F
           mov eax, [def_cursor]
           mov [ebx+APPDATA.cursor], eax
@@:
           mov eax, [hcursor]
           call [eax+APPOBJ.destroy]
.fail:
           ret
endp

; param
;  eax= cursor

align 4
destroy_cursor:

           push eax
           stdcall kernel_free, [eax+CURSOR.base]
           pop eax

           call destroy_kernel_object
           ret

align 4
proc init_cursors
           cmp [SCR_MODE],word 0x13
           jbe .fail

           movzx eax, byte [ScreenBPP]
           mov ebx, [BytesPerScanLine]
           cmp eax, 32
           jne @F
           sub ebx, 128
           jmp .init
@@:
           cmp eax, 24
           jne .fail
           sub ebx, 96
.init:
           mov [cur_def_interl], ebx

           stdcall load_driver, szHwMouse
           mov [hw_cursor], eax
           test eax, eax
           jz .sw_mouse

           stdcall load_cursor, def_arrow, dword LOAD_FROM_MEM
           mov [def_cursor], eax
           ret
.sw_mouse:
           mov [create_cursor], vesa_cursor

           stdcall load_cursor, def_arrow, dword LOAD_FROM_MEM
           mov [def_cursor], eax

           mov ecx, [ScreenWidth]
           mov edx, [ScreenHeight]
           inc ecx
           inc edx
           mov [scr_width], ecx
           mov [scr_height], edx

           movzx ebx, byte [ScreenBPP]
           cmp ebx, 32
           jne @F

           mov dword [set_hw_cursor], cursor_32
           mov dword [hw_restore], restore_32
           ret
@@:
           mov dword [set_hw_cursor], cursor_24
           mov dword [hw_restore], restore_24
           ret
.fail:
           xor eax, eax
           mov dword [set_hw_cursor], eax
           mov dword [hw_restore], eax
           ret
endp

align 4
proc restore_24 stdcall, x:dword, y:dword
           locals
             w  dd ?
           endl

           mov edi, [cur_saved_base]
           mov edx, [cur_saved_h]
           mov ebx, [cur_saved_interl]

           mov esi, cur_saved_data
@@:
           mov ecx, [cur_saved_w]
           lea ecx, [ecx+ecx*2]
           rep movsb
           add edi, ebx
           dec edx
           jnz @B
           ret
endp

align 4
proc restore_32 stdcall, x:dword, y:dword
           locals
             w  dd ?
           endl

           mov edi, [cur_saved_base]
           mov edx, [cur_saved_h]
           mov ebx, [cur_saved_interl]

           mov esi, cur_saved_data
@@:
           mov ecx, [cur_saved_w]
           rep movsd
           add edi, ebx
           dec edx
           jnz @B
           ret
endp

align 4
proc cursor_24 stdcall, hcursor:dword, x:dword, y:dword
           locals
             w      dd ?
             h      dd ?
             st     dd ?
             _dx     dd ?
             _dy     dd ?
           endl

           mov esi, [hcursor]
           mov ecx, [x]
           mov eax, [y]
           mov ebx, [BytesPerScanLine]

           xor edx, edx
           sub ecx, [esi+CURSOR.hot_x]
           mov [x], ecx
           sets dl
           dec edx
           and ecx, edx       ;clip x to 0<=x
           mov edi, ecx
           sub edi, [x]
           mov [_dx], edi

           xor edx, edx
           sub eax, [esi+CURSOR.hot_y]
           mov [y], eax
           sets dl
           dec edx
           and eax, edx       ;clip y to 0<=y
           mov edi, eax
           sub edi, [y]
           mov [_dy], edi

           mul ebx
           lea esi, [ecx+ecx*2]
           add esi, [LFBAddress]
           add esi, eax
           mov [cur_saved_base],esi

           mov edi, [scr_width]
           mov edx, [scr_height]
           mov eax, 32

           sub edi, ecx
           cmp edi, eax
           jng @F
           mov edi, eax
@@:
           sub edi, [_dx]

           sub edx, [y]
           cmp edx, eax
           jng @F
           mov edx, eax
@@:
           sub edx, [_dy]

           mov [w], edi
           mov [h], edx
           mov [cur_saved_w], edi
           mov [cur_saved_h], edx

           sub eax, edi
           shl eax, 2       ;lea eax, [eax+eax*2]
           lea edi, [edi+edi*2]
           sub ebx, edi
           mov [cur_saved_interl], ebx

           mov edi, cur_saved_data
@@:
           mov ecx, [w]
           lea ecx, [ecx+ecx*2]
           rep movsb
           add esi, ebx
           dec edx
           jnz @B

;draw cursor
           mov edx, eax
           mov edi, [cur_saved_base]
           mov eax, [_dy]
           shl eax, 5
           add eax, [_dx]
           shl eax, 2

           mov esi, [hcursor]
           mov esi, [esi+CURSOR.base]
           add esi, eax
.row:
           mov ecx, [w]
.pix:
           lodsd
           test eax, 0xFF000000
           jz @F

           mov word [edi], ax
           shr eax, 16
           mov [edi+2],al
@@:
           add edi, 3
           dec ecx
           jnz .pix

           add esi, edx
           add edi, ebx
           dec [h]
           jnz .row
           ret
endp

align 4
proc cursor_32 stdcall, hcursor:dword, x:dword, y:dword
           locals
             w      dd ?
             h      dd ?
             st     dd ?
             _dx     dd ?
             _dy     dd ?
           endl

           mov esi, [hcursor]
           mov ecx, [x]
           mov eax, [y]
           mov ebx, [BytesPerScanLine]

           xor edx, edx
           sub ecx, [esi+CURSOR.hot_x]
           mov [x], ecx
           sets dl
           dec edx
           and ecx, edx       ;clip x to 0<=x
           mov edi, ecx
           sub edi, [x]
           mov [_dx], edi

           xor edx, edx
           sub eax, [esi+CURSOR.hot_y]
           mov [y], eax
           sets dl
           dec edx
           and eax, edx       ;clip y to 0<=y
           mov edi, eax
           sub edi, [y]
           mov [_dy], edi

           mul ebx
           lea esi, [eax+ecx*4]
           add esi, [LFBAddress]
           mov [cur_saved_base],esi

           mov edi, [scr_width]
           mov edx, [scr_height]
           mov eax, 32

           sub edi, ecx
           cmp edi, eax
           jng @F
           mov edi, eax
@@:
           sub edi, [_dx]

           sub edx, [y]
           cmp edx, eax
           jng @F
           mov edx, eax
@@:
           sub edx, [_dy]

           mov [w], edi
           mov [h], edx
           mov [cur_saved_w], edi
           mov [cur_saved_h], edx

           sub eax, edi
           shl eax, 2
           shl edi, 2
           sub ebx, edi
           mov [cur_saved_interl], ebx

           mov edi, cur_saved_data
@@:
           mov ecx, [w]
           rep movsd
           add esi, ebx
           dec edx
           jnz @B

;draw cursor
           mov edx, eax
           mov edi, [cur_saved_base]
           mov eax, [_dy]
           shl eax, 5
           add eax, [_dx]
           shl eax, 2

           mov esi, [hcursor]
           mov esi, [esi+CURSOR.base]
           add esi, eax
.row:
           mov ecx, [w]
.pix:
           lodsd
           test eax, 0xFF000000
           jz @F
           mov [edi], eax
@@:
           add edi, 4
           dec ecx
           jnz .pix
           add esi, edx
           add edi, ebx
           dec [h]
           jnz .row
           ret
endp

align 4
def_arrow:
  file 'arrow.cur'

