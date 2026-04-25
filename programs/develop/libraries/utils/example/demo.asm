;-----------------------------------------------------------------------------;
;                           =[        INIT        ]=                          ;
;-----------------------------------------------------------------------------;
;; START:
    call    utils_init

;-----------------------------------------------------------------------------;
;                           =[        USE        ]=                           ;
;-----------------------------------------------------------------------------;
    push    fvalue                        ; value dt (XXXXX.XXXXX)
    push    fstring                       ; ASCIIZ string (rb 64)
    call    [_ftoa]                       ; convert

    push    fstring                       ; ASCIIZ string ('XXXX.XXXXXX',0)
    push    fvalue                        ; value dt (?)
    call    [_atof]                       ; EAX: 0 - error, 1 - convert

    call    [_random]                     ; EAX: random digit [0...99999]

;-----------------------------------------------------------------------------;
;                           =[        LOAD        ]=                          ;
;-----------------------------------------------------------------------------;
utils_init:
    mov     eax, 68                       ; load DLL
    mov     ebx, 19                       ;
    mov     ecx, utils_lib
    int     0x40
    test    eax, eax
    jz      utils_exit

    mov     edx, eax                      ; initialize import
    mov     esi, utils_import             ; import list
utils_loop:
    lodsd
    test    eax, eax
    jz      utils_done
    push    edx
utils_find:
    mov     ebx, [edx]
    test    ebx, ebx
    jz      utils_exit                     ;import_not_found
    push    eax
@@:
    mov     cl, [eax]
    cmp     cl, [ebx]
    jnz     utils_next
    test    cl, cl
    jz      utils_found
    inc     eax
    inc     ebx
    jmp     @b
utils_next:
    pop     eax
    add     edx, 8
    jmp     utils_find
utils_found:
    pop     eax
    mov     eax, [edx+4]
    mov     [esi-4], eax
    pop     edx
    jmp     utils_loop
utils_done:
    ret
utils_exit:
    mov    eax, -1
    int    0x40

;-----------------------------------------------------------------------------;
;                           =[        DATA        ]=                          ;
;-----------------------------------------------------------------------------;
    fvalue      dt -502556.267e600 ; dt ?
    fstring     db rb 100          ; '-15.246789',0

    utils_lib   db  '/sys/lib/utils.obj',0    ; path

  align 4
  utils_import:
    _ftoa       dd  ftoa
    _atof       dd  atof
    _random     dd  random
                dd  0

    ftoa        db  'ftoa',0
    atof        db  'atof',0
    random      db  'random',0
;-----------------------------------------------------------------------------;


