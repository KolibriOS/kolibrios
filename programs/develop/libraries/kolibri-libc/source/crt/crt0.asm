format ELF
section '.text' executable
public start
public start as '_start'
;extrn mf_init
extrn main
;include 'debug2.inc'
include 'proc32.inc'
include 'macros.inc'
include 'dll.inc'
__DEBUG__=0

;start_:
virtual at 0
	db 'MENUET01' ; 1. Magic number (8 bytes)
	dd 0x01       ; 2. Version of executable file
	dd start       ; 3. Start address
imgsz	dd 0x0	      ; 4. Size of image
	dd 0x100000   ; 5. Size of needed memory
	dd 0x100000   ; 6. Pointer to stack
hparams dd 0x0	      ; 7. Pointer to program arguments
hpath	dd 0x0	      ; 8. Pointer to program path
end virtual

start:
;DEBUGF 'Start programm\n'
    ;init heap of memory
    mov eax,68
    mov ebx,11
    int 0x40

    mov  [argc], 0
    mov  eax, [hparams]
    test eax, eax
    jz	 .without_path
    mov  eax, path
    cmp	 word ptr eax, 32fh  ; '/#3'  UTF8 
    jne  .without_path
    mov  word ptr eax, 12fh  ; '/#1'  fix to CP866
.without_path:
    mov  esi, eax
    call push_param
; retrieving parameters
    mov  esi, params
    xor  edx, edx  ; dl - идёт параметр(1) или разделители(0)
		   ; dh - символ с которого начался параметр (1 кавычки, 0 остальное)
    mov  ecx, 1    ; cl = 1
		   ; ch = 0  просто ноль
.parse: 
    lodsb
    test al, al
    jz	 .run
    test dl, dl
    jnz  .findendparam
		     ;{если был разделитель
    cmp  al, ' '
    jz	 .parse  ;загружен пробел, грузим следующий символ
    mov  dl, cl  ;начинается параметр
    cmp  al, '"'
    jz	 @f	 ;загружены кавычки
    mov  dh, ch     ;параметр без кавычек
    dec  esi
    call push_param
    inc  esi
    jmp  .parse

  @@:  
    mov  dh, cl     ;параметр в кавычеках
    call push_param ;если не пробел значит начинается какой то параметр
    jmp  .parse     ;если был разделитель}

.findendparam:
    test dh, dh
    jz	 @f ; без кавычек
    cmp  al, '"'
    jz	 .clear
    jmp  .parse
  @@:  
    cmp  al, ' '
    jnz  .parse

.clear: 
    lea  ebx, [esi - 1]
    mov  [ebx], ch
    mov  dl, ch
    jmp  .parse

.run:
    call load_imports
    push argv
    push [argc]
    call main
.exit:
    xor  eax,eax
    dec  eax
    int  0x40
    dd	 -1
.crash:
    jmp  .exit
;============================
push_param:
;============================
;parameters
;  esi - pointer
;description
;  procedure increase argc
;  and add pointer to array argv
;  procedure changes ebx
    mov  ebx, [argc]
    cmp  ebx, max_parameters
    jae  .dont_add
    mov  [argv+4*ebx], esi
    inc  [argc]
.dont_add:    
    ret
;==============================

;==============================
load_imports:
;==============================
;parameters
;  none
;description
;  imports must be located at end of image (but before BSS sections)
;  the address of end of imports (next byte after imports) is located in imgsz
;  look at each import from that address up to illegal import
;  legal import is such that:
;    first pointer points to procedure name
;      and is smaller than imgsz
;    second pointer points lo library name, starting with 0x55, 0xAA
;      and is smaller than imgsz
;  each library should be initialized as appropriate, once
;  so as library is initialized, its name will be replaced 0x00
    mov ebx, [imgsz]                ; byte after imports
.handle_next_import:
    sub ebx, 4                      ; ebx = pointer to pointer to library name
    mov esi, dword[ebx]             ; esi = pointer to library name
    push ebx
    push esi
    call load_library               ; eax = pointer to library exports
    pop esi
    pop ebx
    test eax, eax
    jz .done
    sub ebx, 4                      ; ebx = pointer to pointer to symbol name
    push ebx
    stdcall dll.GetProcAddress, eax, dword[ebx]
    pop ebx
    test eax, eax
    jz .fail
    mov dword[ebx], eax
    jmp .handle_next_import
.done:
    ret
.fail:
    ret
;==============================

;==============================
load_library:
;==============================
;parameters
;  ebx: library name address
;description
;  each library should be initialized as appropriate, once
;  so as library is initialized, its name will be replaced 0x00
;  and 4 next bytes will be set to address of library
    ; first two bytes of library name must be 0x55, 0xAA (is like a magic)
    cld                ; move esi further, not back
    cmp esi, [imgsz]
    ja .fail
    lodsb              ; al = first byte of library name
    cmp al, 0x55
    jne .fail
    lodsb              ; al = second byte of library name
    cmp al, 0xAA
    jne .fail
    lodsb              ; al = third byte of library name (0x00 if the library is already loaded)
    test al, al
    jnz .load
    lodsd              ; if we here, then third byte is 0x00 => address of library is in next 4 bytes
    ; now eax contains address of library
    ret
.load:
    dec esi ; we checked on 0 before, let's go back
    mov eax, 68
    mov ebx, 19
    mov ecx, esi
    int 0x40           ; eax = address of exports
    mov byte[esi], 0   ; library is loaded, let's place 0 in first byte of name
    mov [esi + 1], eax ; now next 4 bytes of library name are replaced by address of library
    ; call lib_init
    stdcall dll.GetProcAddress, eax, lib_init_str ; eax = address of lib_init
    test eax, eax
    jz .ret
    stdcall dll.Init, eax
.ret:
    mov eax, [esi + 1] ; put address of library into eax
    ret
.fail:
    mov eax, 0
    ret
    
; ==== memmove for tcc ======

proc memmove c, to:dword,from:dword,count:dword

    push esi
    push edi
	mov ecx,[count]
	test ecx,ecx
	jz no_copy_block_
		mov esi,[from]
		mov edi,[to]
		cmp esi, edi
		je no_copy_block_
		jg copy_
            add	esi, ecx
            add	edi, ecx
            dec	esi
            dec	edi
            std
copy_:
		rep movsb
        cld
no_copy_block_:

    pop edi
    pop esi
    mov eax,[to]
	ret
endp
    
    
;==============================

lib_init_str db 'lib_init', 0

public argc as '__argc'
public params as '__argv'
public path as '__path'
public memmove

section '.bss' 
buf_len = 0x400
max_parameters=0x20
argc	 rd 1
argv	 rd max_parameters
path	 rb buf_len 
params	 rb buf_len 

;section '.data'
;include_debug_strings ; ALWAYS present in data section
