
USE32

format MS COFF

include 'proc32.inc'

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

public ___create_cursor
public ___destroy_cursor
public _copy_cursor@8

extrn _create_cursor    :dword
extrn _destroy_cursor   :dword
extrn _tmp_cursor       :dword

section 'AUTO' code readable executable align 16

align 16
___create_cursor:
             push ecx
             push ebx
             push eax
             call _create_cursor
             add esp, 12
             ret
align 16
___destroy_cursor:
             push eax
             call _destroy_cursor
             add esp, 4
             ret
align 16
proc _copy_cursor@8 stdcall, dst:dword, src:dword
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

           mov edi, _tmp_cursor
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
           jmp .copy
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

           mov edi, _tmp_cursor
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
           jmp .copy
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

           mov edi, _tmp_cursor
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
           jmp .copy
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

           mov edi, _tmp_cursor
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
.copy:
           mov edi, [dst]
           mov ecx, 64*64
           xor eax,eax
           rep stosd

           mov esi, _tmp_cursor
           mov edi, [dst]
           mov ebx, 32
           cld
@@:
           mov ecx, 32
           rep movsd
           add edi, 128
           dec ebx
           jnz @B
           ret
endp

macro rdr op1, op2
{
     mov op1, [edi+op2]
}

macro wrr dest, src
{
     mov dword [edi+dest], src
}

struc CURSOR
{;common object header
   .magic       dd ?   ;'CURS'
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id

 ;cursor data
   .base        dd ?   ;allocated memory
   .hot_x       dd ?   ;hotspot coords
   .hot_y       dd ?
}
virtual at 0
  CURSOR CURSOR
end virtual

;public _r500_SelectCursor@4
align 4
proc _r500_SelectCursor@4 stdcall,hcursor:dword

           mov ecx, [hcursor]
           mov edi, [_rhd]

           mov edx, [ecx+CURSOR.base]
           sub edx, [_rhd+3*4]
           add edx, [_rhd+5*4]
           mov [edi+0x6408], edx

           mov eax, [ecx+CURSOR.hot_x]
           shl eax, 16
           mov ax, word [ecx+CURSOR.hot_y]
           mov [edi+0x6418], eax
           ret
endp

;public _r500_SetCursor@12
align 4
proc _r500_SetCursor@12 stdcall, hcursor:dword, x:dword, y:dword
           pushfd
           cli

           mov edx, [_rhd]

           mov eax, [x]
           shl eax, 16
           mov ax, word [y]

           mov [edx+0x6414], eax
           or dword [edx+0x6400], 1

           popfd
           ret
endp

