
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

           mov esi,[pAnd]
           mov ebx, [pBits]
.l1:
           mov eax, [esi]
           bswap eax
           mov [counter], 16
@@:
           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           mov ecx, [ebx]
           and ecx, 0xF0
           shr ecx, 2
           add ecx, [pQuad]
           mov ecx, [ecx]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi], edx

           xor edx, edx
           shl eax,1
           setc dl
           dec edx

           mov ecx, [ebx]
           and ecx, 0x0F
           shl ecx, 2
           add ecx, [pQuad]
           mov ecx, [ecx]
           and ecx, edx
           and edx, 0xFF000000
           or edx, ecx
           mov [edi+4], edx

           inc ebx
           add edi, 8
           dec [counter]
           jnz @B

           add esi, 4
           mov edi,[rBase]
           sub edi,128
           mov [rBase],edi
           sub [height],1
           jnz .l1
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
proc alloc_cursor

           pushfd
           cli
           mov ebx, [cursor_start]
           mov ecx, [cursor_end]
.l1:
           bsf eax,[ebx];
           jnz .found
           add ebx,4
           cmp ebx, ecx
           jb .l1
           popfd
           xor eax,eax
           ret
.found:
           btr [ebx], eax
           mov [cursor_start],ebx
           sub ebx, cursor_map
           shl ebx, 3
           add eax,ebx
           shl eax,3
           lea eax,[cursors+eax+eax*2]
           popfd
           ret
endp

align 4
proc create_cursor
           locals
             h_cur  dd ?
           endl

           call alloc_cursor
           test eax, eax
           jz .fail

           mov [h_cur], eax
           mov edi, eax

           xor ebx, ebx

           mov [edi+CURSOR.magic], 'CURS'
           mov [edi+CURSOR.size],  CURSOR_SIZE
           mov [edi+CURSOR.pid], ebx
           mov [edi+CURSOR.hot_x], ebx
           mov [edi+CURSOR.hot_y], ebx

           stdcall kernel_alloc, dword 0x2000
           test eax, eax
           jz .fail

           mov ebx, eax
           mov eax, [h_cur]
           mov [eax+CURSOR.base], ebx
           ret
.fail:
           ret
endp

align 4
proc set_cursor stdcall, hcursor:dword
           mov eax, [hcursor]
           mov ebx, [CURRENT_TASK]
           shl ebx, 8
           xchg eax, [ebx+PROC_BASE+APPDATA.cursor]
           ret
endp

align 4
proc load_cursor stdcall, src:dword, flags:dword
           locals
             handle  dd ?
           endl

           movzx eax, word [flags]
           cmp eax, LOAD_FROM_FILE
           jne .from_mem

           stdcall load_file, [src]
           test eax, eax
           jz .exit
           mov [src], eax

           call create_cursor
           test eax, eax
           jz .fail

           mov [handle], eax
           mov esi, [src]
           movzx ebx, word [esi+10]
           movzx ecx, word [esi+12]
           mov [eax+CURSOR.hot_x], ebx
           mov [eax+CURSOR.hot_y], ecx

           stdcall vesa_init_cursor, [eax+CURSOR.base], esi
           stdcall kernel_free, [src]
           mov eax, [handle]
           ret

.from_mem:
           cmp eax, LOAD_FROM_MEM
           jne .indirect

           call create_cursor
           test eax, eax
           jz .exit

           mov [handle], eax
           mov esi, [src]
           movzx ebx, word [esi+10]
           movzx ecx, word [esi+12]
           mov [eax+CURSOR.hot_x], ebx
           mov [eax+CURSOR.hot_y], ecx

           stdcall vesa_init_cursor, [eax+CURSOR.base], [src]
           mov eax, [handle]
           ret

.indirect:
           cmp eax, LOAD_INDIRECT
           jne .fail

           call create_cursor
           test eax, eax
           jz .exit

           movzx edx, byte [flags+2]
           movzx ebx, byte [flags+3]
           mov [eax+CURSOR.hot_x], ebx
           mov [eax+CURSOR.hot_y], edx

           mov edi, [eax+CURSOR.base]
           mov esi, [src]
           mov ecx, 1024
           cld
           rep movsd
           ret
.fail:
           mov ebx, [src]
           stdcall kernel_free, ebx
.exit:
           xor eax, eax
           ret
endp

align 4
proc init_cursors
           movzx eax, byte [ScreenBPP]
           mov ebx, [SCR_BYTES_PER_LINE]
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

           xor eax, eax
           mov edi, cursors
           mov ecx, CURSOR_SIZE*16
           cld
           rep stosd

           not eax
           mov [cursor_map], eax
           mov [cursor_map+4], eax
           mov edx, cursor_map
           mov [cursor_start], edx
           add edx, 4
           mov [cursor_end], edx

           stdcall load_cursor, def_arrow, dword LOAD_FROM_MEM
           mov [def_cursor], eax

           mov ecx, [SCR_X_SIZE]
           mov edx, [SCR_Y_SIZE]
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
           ;cmovg edi, eax
           jng @F
           mov edi, eax
@@:
           sub edi, [_dx]

           sub edx, [y]
           cmp edx, eax
           ;cmovg edx, eax
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
;           cmovg edi, eax
           jng @F
           mov edi, eax
@@:
           sub edi, [_dx]

           sub edx, [y]
           cmp edx, eax
;           cmovg edx, eax
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

