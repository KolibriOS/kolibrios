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

include '../../proc32.inc'
include '../../macros.inc'

section '.flat' code readable align 16

app_version       equ word[8]
i_table_min_size  =   1

sizeof.kx_header  =   8

APP_STARTUP_THUNK:
    ; First make shure that app 
    ; have header version 2.0 or more
    cmp app_version,2
    jl .denied            ; App with app_version < 2 shouldn't be here 
    
    ; Then make shure that we first
    mov eax, @EXPORT
    cmp dword[eax-4],0
    je .denied
    
    ; Don't allow second time
    mov dword[eax-4],0  
    
   ; Early app initialization

;{ Test KX header
   ;xor  eax, eax
   mov	esi,0x24
   lodsw
   cmp ax, 'KX'
   jne @f ; Not KX
   lodsw
   cmp ax, 0
   jne @f ; Bad magic
   lodsw
   
   bt ax, 6 ; Have import?
   jnc .app_start 
;}
   
   ; Test import table (use legacy style)  
   mov  eax, [sizeof.kx_header + 0x24] ; i_table_ptr
   test eax, eax
   jz  .app_start           ; i_table_ptr = 0 ?
   ;js      .error
   mov esi, [0x10] 
   cmp esi, eax
   jbe @f           ; i_table_ptr >= img_size ?
   mov ebx, eax
   add ebx, i_table_min_size 
   cmp esi, ebx
   jb @f           ; i_table_ptr + i_table_min_size > img_size ?
   
   ; Link app/dependent libs import tables with libs export table
   ; TODO: need revision of the exists lib format and dll.Load (for libs import binds)
           
   stdcall dll.Load,eax 
   test	eax, eax
   jnz	.import_error
.app_start:   
   ; Start of app code
   mov  eax, [0x0C]
   ; TODO: test start_ptr + min_code_size < img_size    
   call eax
@@:
   mov eax, -1
   int 0x40
.import_error:
  ; Run @NOTIFY and tell user then error occured
  ; BOARD will contaits details
  jmp @b
.denied:
    ; Kolibri has no ability kill app if this enter from no from main thread
    ; So just alert and return
    ;DEBUGF	1, 'APP_STARTUP_THUNK@dll.obj: App twice/with app_version < 2 has entered!\n'
    ret  
; } APP_STARTUP_THUNK


; WARNING! This code must be after app initialization thunk!
include '../../dll.inc'
align 4
;dd 0xdeadbeef
dd APP_STARTUP_THUNK
@EXPORT:
export                              \
    dll.Load,           'dll_load',  \
    dll.Link,           'dll_link',  \
    dll.GetProcAddress, 'dll_sym' ;