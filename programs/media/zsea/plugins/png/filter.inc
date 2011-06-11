;*****************************************************************************
; PNG to RAW convert plugin - for zSea image viewer
; Copyright (c) 2008, 2009, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;---------------------------------------------------------------------
filtering:
   push eax ecx edx
   mov  ebp,[line_filter_type]
   cmp  ebp,0
   je   .end
   cmp  ebp,1
   je   .Sub
   cmp  ebp,2
   je   .Up
   cmp  ebp,3
   je   .Averag
   cmp  ebp,4
   je   .Paeth
   jmp  .end
;---------------------------------------------------------------------
.Sub:
   cmp  [IHDR_data.Bit_depth],byte 16
   jne  @f
   mov  eax,ebx
   shr  eax,16
   add  bl,al
   add  bh,ah
   jmp  .end
@@:
   add  bl,bh
   jmp  .end
;---------------------------------------------------------------------
.Up:
   cmp  [first_line],byte 0
   jne  .end
   mov  eax,[IHDR_data.Width]
   cmp [IHDR_data.Color_type],byte 0
   je  .Up_Color_type_0_and_3
   cmp [IHDR_data.Color_type],byte 3
   je  .Up_Color_type_0_and_3
   cmp [IHDR_data.Color_type],byte 4
   je  @f
   lea  eax,[eax*3]
@@:
   cmp [IHDR_data.Color_type],byte 4
   je  .Up_Alpfa
   cmp [IHDR_data.Color_type],byte 6
   jne @f
.Up_Alpfa:
   add  eax,[IHDR_data.Width]
   
@@:
.Up_8:
   cmp  [IHDR_data.Bit_depth],byte 16
   je   .Up_16
   mov  ebp,edi
   sub  ebp,eax
   mov  bh,[ebp]
   add  bl,bh
   jmp  .end

.Up_Color_type_0_and_3:
   cmp  [IHDR_data.Bit_depth],byte 16
   je   .Up_16
   call  .Bit_depth_less_8
   jmp  .Up_8

.Bit_depth_less_8:   
;   imul eax,[resolution]
   cmp [resolution],8
   jne @f
   ret
@@:
   cmp [resolution],4
   jne @f 
   shl eax,2
   jmp  .continue_Bit_depth_less_8
@@:
   cmp [resolution],2
   jne .continue_Bit_depth_less_8 
   shl eax,1
.continue_Bit_depth_less_8:
   push ebx
   mov  ebx,eax
   shr  eax,3
   test ebx,7
   jz  @f
   inc  eax
@@:
   pop  ebx
   ret

.Up_16:
   shl  eax,1
   mov  ebp,edi
   sub  ebp,eax
   mov  ax,[ebp]
   add  bl,al
   add  bh,ah
   jmp  .end
;---------------------------------------------------------------------
.Averag:
   cmp [first_line],byte 0
   je  @f
   xor eax,eax
   cmp  [IHDR_data.Bit_depth],byte 16
   je  .Averag_2
   jmp .Averag_1
@@:
   mov eax,[IHDR_data.Width]
   cmp [IHDR_data.Color_type],byte 0
   je  .Averag_Color_type_0_and_3  ;@f
   cmp [IHDR_data.Color_type],byte 3
   je  .Averag_Color_type_0_and_3  ;@f
   cmp [IHDR_data.Color_type],byte 4
   je  @f
   lea eax,[eax*3]
@@:
   cmp [IHDR_data.Color_type],byte 4
   je  .Averag_Alpfa
   cmp [IHDR_data.Color_type],byte 6
   jne @f
.Averag_Alpfa:
   add eax,[IHDR_data.Width]

@@:
.Averag_8:
   cmp  [IHDR_data.Bit_depth],byte 16
   je  .Averag_16
   mov ebp,edi
   sub ebp,eax
   xor eax,eax
   mov al,[ebp] 
.Averag_1:
   movzx ebp,bh
   add eax,ebp
   shr eax,1
   add bl,al
   jmp  .end

.Averag_Color_type_0_and_3:
   cmp  [IHDR_data.Bit_depth],byte 16
   je   .Averag_16
   call  .Bit_depth_less_8
   jmp  .Averag_8
      
.Averag_16:
   shl eax,1
   mov ebp,edi
   sub ebp,eax
   xor eax,eax
   mov ax,[ebp] 
.Averag_2:
   mov ebp,ebx
   shr ebp,16
   mov ecx,ebp
   and ecx,0xff
   mov edx,eax
   and edx,0xff   
   add ecx,edx
   shr ecx,1
   add bl,cl
   shr ebp,8
   shr eax,8
   add eax,ebp
   shr eax,1
   add bh,al   
   jmp  .end
;---------------------------------------------------------------------
.Paeth:
   cmp [first_line],byte 0
   je  @f
   xor eax,eax
   cmp  [IHDR_data.Bit_depth],byte 16
   je  .Paeth_16_1
   jmp .Paeth_1
@@:
   mov eax,[IHDR_data.Width]
   cmp [IHDR_data.Color_type],byte 0
   je  .Paeth_Color_type_0_and_3  ;@f
   cmp [IHDR_data.Color_type],byte 3
   je  .Paeth_Color_type_0_and_3  ;@f
   cmp [IHDR_data.Color_type],byte 4
   je  @f
   lea eax,[eax*3]
@@:
   cmp [IHDR_data.Color_type],byte 4
   je  .Paeth_Alpfa
   cmp [IHDR_data.Color_type],byte 6
   jne @f
.Paeth_Alpfa:
   add eax,[IHDR_data.Width]

@@:
   cmp  [IHDR_data.Bit_depth],byte 16
   je  .Paeth_16
.Paeth_8:
   mov ebp,edi
   sub ebp,eax
   xor eax,eax
   mov al,[ebp]
.Paeth_1:
   mov [Paeth_filter.b],al
   
   cmp [first_line],byte 0
   jne  @f 
   cmp [first_pixel],0
   jne  @f
   mov ah,[ebp-1]
   cmp [IHDR_data.Color_type],byte 0
   je  @f
   cmp [IHDR_data.Color_type],byte 3
   je  @f
   mov ah,[ebp-2]
   cmp [IHDR_data.Color_type],byte 4
   je  @f
   mov ah,[ebp-3]
   cmp [IHDR_data.Color_type],byte 2
   je @f
   mov ah,[ebp-4]
@@:
   mov [Paeth_filter.c],ah
   mov [Paeth_filter.a],bh
   
   xor eax,eax
   mov al,[Paeth_filter.a]
   mov ebp,eax
   mov al,[Paeth_filter.b]
   add ebp,eax
   mov al,[Paeth_filter.c]
   sub ebp,eax
   mov [Paeth_filter.p],ebp
;----------------------------------------
   mov al,[Paeth_filter.a]
   sub ebp,eax
   test ebp,0x80000000
   jz  @f
   neg ebp
@@:
   mov [Paeth_filter.pa],ebp
   
   mov ebp,[Paeth_filter.p]
   mov al,[Paeth_filter.b]
   sub ebp,eax
   test ebp,0x80000000
   jz  @f
   neg ebp
@@:
   mov [Paeth_filter.pb],ebp   
   
   mov ebp,[Paeth_filter.p]
   mov al,[Paeth_filter.c]
   sub ebp,eax
   test ebp,0x80000000
   jz  @f
   neg ebp
@@:
   mov [Paeth_filter.pc],ebp 
   
   mov eax,[Paeth_filter.pa]
   mov ebp,[Paeth_filter.pb]
   cmp eax,ebp
   ja .no_return_a
   cmp eax,[Paeth_filter.pc]
   ja .no_return_a
   add bl,[Paeth_filter.a]
   jmp .end
.no_return_a:
   cmp ebp,[Paeth_filter.pc]
   ja .no_return_b
   add bl,[Paeth_filter.b]
   jmp .end
.no_return_b:
   add bl,[Paeth_filter.c]
.end:
   pop  edx ecx eax
   ret
;----------------------------------------   
.Paeth_Color_type_0_and_3:
   cmp  [IHDR_data.Bit_depth],byte 16
   je   .Paeth_16
   call  .Bit_depth_less_8
   jmp  .Paeth_8
;----------------------------------------
.Paeth_16:
   shl eax,1
   mov ebp,edi
   sub ebp,eax
   xor eax,eax
   mov ax,[ebp]
.Paeth_16_1:
   mov [Paeth_filter.b],ax
   xor eax,eax
   cmp [first_line],byte 0
   jne  @f 
   cmp [first_pixel],0
   jne  @f
   mov ax,[ebp-2]
   cmp [IHDR_data.Color_type],byte 0
   je  @f
   mov ax,[ebp-4]
   cmp [IHDR_data.Color_type],byte 4
   je  @f
   mov ax,[ebp-6]
   cmp [IHDR_data.Color_type],byte 2
   je  @f
   mov ax,[ebp-8]
@@:
   mov [Paeth_filter.c],ax
   mov eax,ebx
   shr eax,16
   mov [Paeth_filter.a],ax
;----------------------------------------
   xor eax,eax
   mov ax,[Paeth_filter.a]
   shr ax,8
   mov bp,ax
   mov ax,[Paeth_filter.b]
   shr ax,8
   add bp,ax
   mov ax,[Paeth_filter.c]
   shr ax,8
   sub bp,ax
   shl ebp,16
   
   xor eax,eax
   mov al,[Paeth_filter.a]
   mov bp,ax
   mov al,[Paeth_filter.b]
   add bp,ax
   mov al,[Paeth_filter.c]
   sub bp,ax   
   
   mov [Paeth_filter.p],ebp
;----------------------------------------
   xor eax,eax
   mov ax,[Paeth_filter.a]
   mov ecx,ebp
   mov dx,ax
   shr ecx,16
   shr dx,8
   sub cx,dx
   test cx,0x8000
   jz  @f
   neg cx
@@:
   shl ecx,16

   and ebp,0xffff
   and eax,0xff
   sub bp,ax
   test bp,0x8000
   jz  @f
   neg bp
@@:
   add ebp,ecx
   mov [Paeth_filter.pa],ebp
;----------------------------------------
   xor eax,eax
   mov ebp,[Paeth_filter.p]
   mov ax,[Paeth_filter.b]
   mov ecx,ebp
   mov dx,ax
   shr ecx,16
   shr dx,8
   sub cx,dx
   test cx,0x8000
   jz  @f
   neg cx
@@:
   shl ecx,16

   and ebp,0xffff
   and eax,0xff
   sub bp,ax
   test bp,0x8000
   jz  @f
   neg bp
@@:
   add ebp,ecx
   mov [Paeth_filter.pb],ebp
;----------------------------------------
   xor eax,eax
   mov ebp,[Paeth_filter.p]
   mov ax,[Paeth_filter.c]
   mov ecx,ebp
   mov dx,ax
   shr ecx,16
   shr dx,8
   sub cx,dx
   test cx,0x8000
   jz  @f
   neg cx
@@:
   shl ecx,16

   and ebp,0xffff
   and eax,0xff
   sub bp,ax
   test bp,0x8000
   jz  @f
   neg bp
@@:
   add ebp,ecx
   mov [Paeth_filter.pc],ebp
;----------------------------------------
   mov eax,[Paeth_filter.pa]
   mov ebp,[Paeth_filter.pb]
   mov ecx,[Paeth_filter.pc]
   shr eax,16
   shr ebp,16
   shr ecx,16
   cmp ax,bp
   ja .no_return_a_1
   cmp ax,cx
   ja .no_return_a_1
   mov ax,[Paeth_filter.a]
   add bh,ah
   jmp .end_1
.no_return_a_1:
   cmp bp,cx
   ja .no_return_b_1
   mov ax,[Paeth_filter.b]
   add bh,ah
   jmp .end_1
.no_return_b_1:
   mov ax,[Paeth_filter.c]
   add bh,ah
.end_1:
;----------------------------------------
   mov eax,[Paeth_filter.pa]
   mov ebp,[Paeth_filter.pb]
   mov ecx,[Paeth_filter.pc]
   cmp ax,bp
   ja .no_return_a_2
   cmp ax,cx
   ja .no_return_a_2
   add bl,[Paeth_filter.a]
   jmp .end_2
.no_return_a_2:
   cmp bp,cx
   ja .no_return_b_2
   add bl,[Paeth_filter.b]
   jmp .end_2
.no_return_b_2:
   add bl,[Paeth_filter.c]
.end_2:
   pop  edx ecx eax
   ret
;---------------------------------------------------------------------