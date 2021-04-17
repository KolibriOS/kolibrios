;
;  KolibriOS Dll load support
;
;  (C) 2020-2021 Coldy	
;  Thank's you for use this code and software based on it!
;  I will glad if it's will be helpful. 
;
;  Distributed under terms of GPL
;
format MS COFF
public @EXPORT as 'EXPORTS'

include '../../../proc32.inc'
include '../../../macros.inc'

section '.flat' code readable align 16

app_version	  equ word[8]
i_table_min_size  =   1

APP_START_THUNK:
    ; First make shure that app 
    ; have header version 2.0 or more
    cmp app_version,2
    jl .denied
    
    ; Then make shure that we first
    mov eax, @EXPORT
    cmp dword[eax-4],0
    je .denied
    
    ; Don't allow second time
    mov dword[eax-4],0	
    
   ; Early app initialization
   
   ; Test import table	
   mov	eax, [0x24] ; i_table_ptr
   test eax, eax
   jz  @f
   mov esi, [0x10] 
   cmp esi, eax
   jbe @f	    ; i_table_ptr >= img_size ?
   mov ebx, eax
   add ebx, i_table_min_size 
   cmp esi, ebx
   jbe @f	    ; i_table_ptr + i_table_min_size >= img_size ?
   
   ; Link app import table with DLL's exoport table	  
   stdcall dll.Load,eax 
   test eax, eax
   jnz	@f   
   ; Start of app code
   mov	eax, [0x0C]
   ; TODO: test start_ptr + min_code_size < img_size	
   call eax
@@:
   mov eax, -1
   int 0x40
.denied:
   ret
; } APP_START_THUNK


; WARNING! This code must be after app initialization thunk!
include '../../../dll.inc'
align 4
;dd 0xdeadbeef
dd APP_START_THUNK
@EXPORT:
export				    \
    dll.Load,		'dll_load',  \
    dll.Link,		'dll_link',  \
    dll.GetProcAddress, 'dll_sym' ;