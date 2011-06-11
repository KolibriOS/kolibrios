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
deflate_callback:
   cmp  [deflate_start_offset],1
   je   @f
   mov  [deflate_start_offset],1
   jmp  .1
@@:
   mov  eax,[IDAT.pointer]
   add  eax,[IDAT.size]
   mov  ebx,[image_file]
   add  ebx,[file_size]
   sub  ebx,32
   cmp  ebx,eax
   jbe   @f
   mov  [next_Chunk],eax
   call search_IDAT
   jmp  .1
@@:
   xor  eax,eax
   mov  [IDAT.size],eax
   mov  [IDAT.pointer],eax
.1:
	mov	ecx, [esp+8]
	mov eax,[IDAT.size]
	mov	[ecx], eax   ;length
	mov	eax,[IDAT.pointer]   ; buffer
	ret	8
;---------------------------------------------------------------------
search_IHDR:
   mov  [Chunk_pointer],IHDR_name
   call get_Chunk_data_pointer
   mov  eax,[Chunk_pointer]
   cmp  eax,0
   je   .no_png_file
   mov  [IHDR.pointer],eax
   mov  ecx,[eax-8]
   call convert_NBO_to_PC
   mov  [IHDR.size],ecx
   ret
.no_png_file:
   add  esp,4
   jmp  no_png_file
;---------------------------------------------------------------------
search_PLTE:
   push eax ecx
   mov  [Chunk_pointer],PLTE_name 
   call get_Chunk_data_pointer
   mov  eax,[Chunk_pointer]
   cmp  eax,0
   je   .no_png_file
   mov  [PLTE.pointer],eax
   mov  ecx,[eax-8]
   call convert_NBO_to_PC
   mov  [PLTE.size],ecx
   pop  ecx eax
   ret
.no_png_file:
   add  esp,12
   jmp  no_png_file
;---------------------------------------------------------------------
search_IDAT:
   mov  [Chunk_pointer],IDAT_name  
   call get_Chunk_data_pointer
   mov  eax,[Chunk_pointer]
   cmp  eax,0
   je   .no_IDAT_next
   mov  [IDAT.pointer],eax
   mov  ecx,[eax-8]
   call convert_NBO_to_PC
   mov  [IDAT.size],ecx
   xor  eax,eax
   ret
.no_IDAT_next:
;   add  esp,4
;   jmp  no_png_file
   mov  eax,1
   ret
;---------------------------------------------------------------------
search_IEND:
   mov  [Chunk_pointer],IEND_name
   call get_Chunk_data_pointer
   mov  eax,[Chunk_pointer]
   cmp  eax,0
   je   .no_png_file
   mov  [IEND.pointer],eax
   mov  ecx,[eax-8]
   call convert_NBO_to_PC
   mov  [IEND.size],ecx
   ret
.no_png_file:
   add  esp,4
   jmp  no_png_file
;--------------------------------------------------------------------- 
convert_NBO_to_PC:   ;network byte order value to PC value
   push eax
   mov  al,ch
   mov  ah,cl
   shl  eax,16
   shr  ecx,16
   mov  al,ch
   mov  ah,cl
   mov  ecx,eax
   pop  eax
   ret
;---------------------------------------------------------------------
get_Chunk_data_pointer:
   pushad
   mov   ebp,4
;   mov   edi,[image_file]
   mov   edi,[next_Chunk]   
   mov   edx,[image_file]  ;edi
   add   edx,[file_size]
   dec   edi
.search_Chunk:
   cmp   edx,edi
   jbe   .end
   mov   esi,[Chunk_pointer]
   mov   ecx,ebp
   inc   edi
   mov   ebx,edi
   cld
   rep   cmpsb
   mov   edi,ebx
   jne   .search_Chunk
   add   edi,ebp
   mov   [Chunk_pointer],edi
   popad
   ret
.end:
   mov   [Chunk_pointer],0
   popad
   ret
;---------------------------------------------------------------------