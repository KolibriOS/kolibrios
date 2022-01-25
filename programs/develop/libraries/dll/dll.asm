;
;  Support for Dll auto load & linking
;
;  (C) 2020-2022 Coldy	
;  Thank's you for use this code and software based on it!
;  I will glad if it's will be helpful. 
;
;  Distributed under terms of GPL
;

format MS COFF
public @EXPORT as 'EXPORTS'
section '.flat' code readable align 16

include 'external.inc'
include 'dll.inc'

; This need for @notyfy pre/postformat
STR_BUILD_OFFSET  = 1
STR_BUILD_EXTRA   = 5

mem_alloc          = mem.Alloc 

include 'strhlp.inc'

app_version       equ word[8]
i_table_min_size  =   1

sizeof.kx_header  =   8

ERROR_BAD_IMAGE   =   0x010

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
   jne .image_error ; Not KX
   lodsw
   cmp ax, 0
   jne .image_error ; Bad magic
   lodsw
   
   bt ax, 6 ; Have import?
   jnc .app_start 
;}
   
   ; Test import table (use legacy style)  
   mov  eax, [sizeof.kx_header + 0x24] ; i_table_ptr
   test eax, eax
   jz  .image_error;.import_error;.app_start           ; i_table_ptr = 0 ? => Bad image
   ;js      .error
   mov esi, [0x10] 
   cmp esi, eax
   jbe .image_error;@f           ; i_table_ptr >= img_size ?
   mov ebx, eax
   add ebx, i_table_min_size 
   cmp esi, ebx
   jb .image_error;@f           ; i_table_ptr + i_table_min_size > img_size ?
   
   ; Link app/dependent libs import tables with libs export table
   ; TODO: need revision of the exists lib format and dll.Load (for libs import binds)
           
   stdcall dll.Load,eax 
   test	eax, eax
   jnz	.link_error;.import_error
.app_start:   
   ; Start of app code
   mov  eax, [0x0C]
   ; TODO: test start_ptr + min_code_size < img_size    
   call eax
@@:
   mov eax, -1
   int 0x40
.image_error:
   mov eax, ERROR_BAD_IMAGE 
.link_error:
  ; Run @NOTIFY and tell user then error occurred 
  call show_error
  jmp @b
.denied:
    ; Kolibri has no ability kill app if this enter from no from main thread
    ; So just alert and return
    ;DEBUGF	1, 'APP_STARTUP_THUNK@dll.obj: App twice/with app_version < 2 has entered!\n'
    ret  
; } APP_STARTUP_THUNK


; eax = error code  (see ERROR_xxx above in this and dll.inc files)

show_error:  

  ; Store error code
  mov    edx, eax 

   ; Get app name
   sub  esp,1024
   mov  eax, 9
   mov  ebx, esp
   mov  ecx, -1
   int    0x40
   
   ;
   
   mov esi, esp
   add esi, 10       ; esi = app name

   
   cmp    edx, ERROR_ENTRY_NOT_FOUND
   je     .entry_not_found 
   
   ; Init heap not needed 
   ; (kernel already initialized heap implicitly when load dll.obj)
   
   cmp    edx, ERROR_LIBRARY_NOT_LOAD
   je     .library_not_loaded  
   
   ccall  str_build, szWrongFormat, esi, szBanner
   jmp @f

.library_not_loaded:
  ccall  str_build, szLibraryNotLoaded, esi, szBanner, s_libdir.fname
  jmp @f
  
.entry_not_found:
  mov eax, [szEntryName]
  ccall  str_build, szEntryNotFound, esi, szBanner, eax, s_libdir.fname 
   
@@:
  add esp, 1024

   mov    byte[eax],'"'
   mov    byte[edi],'"'
   mov    dword[edi+1],"-tdE" ; with title, disable autoclose, error icon
   
   mov    esi, eax    
   
   ; Display error
    mov   [pNotify.params], eax  
    mov   ebx, pNotify
    mov   eax, 70
    int   0x40
    
    stdcall mem.Free, esi
    
.exit:    
    ret


align 4
;dd 0xdeadbeef
dd APP_STARTUP_THUNK
@EXPORT:
export                              \
    dll.Load,           'dll_load',  \        
    dll.Link,           'dll_link',  \         
    dll.GetProcAddress, 'dll_sym'    
                         
                         
pNotify:
          dd	    7, 0
  .params dd      0      
          dd      0, 0
          db	    "/sys/@notify", 0
          
; { Strings
if defined LANG_RUS
include 'strings.rus'
;elseif defined LANG_xxx
; TODO:   Add another supported languges here
;       - Copy 'elseif defined LANG_xxx', change xxx here and below to the
;         corresponded constat of you languge
;       - Create strings.xxx in the root of this project and do translate,
;         follow the format message below.
;       - add include 'strings.xxx'  
else ; Default languge (English)
szBanner                db "Error!\n",0
szWrongFormat           db "$ - $Application has wrong format!",0
szLibraryNotLoaded      db "$ - $Can't load library $", 0
szEntryNotFound         db "$ - $Entry $\nin library $\nnot found!",0

end if
; } Strings