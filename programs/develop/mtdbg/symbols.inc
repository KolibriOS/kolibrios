;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;; Working with program symbols ;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include 'sort.inc'

; compare proc for sorter
compare:
        cmpsd
        jnz     @f
        cmp     esi, edi

  @@:
        ret

; compare proc for sorter 2
compare2:
        cmpsd

  @@:
        cmpsb
        jnz     @f
        cmp     byte [esi-1], 0
        jnz     @b
        cmp     esi, edi

  @@:
        ret

free_symbols:
        mov     ecx, [symbols]
        jecxz   @f
        mcall   68, 13
        and     [symbols], 0
        and     [num_symbols], 0

  @@:
        ret


;-----------------------------------------------------------------------------
;                        Load symbols event

OnLoadSymbols.fileerr:
        test    ebp, ebp
        jz      @f
        mcall   68, 13, edi
        ret

  @@:
        push    eax
        mcall   68, 13, edi
        mov     esi, aCannotLoadFile
        call    put_message_nodraw
        pop     eax
        cmp     eax, 0x20
        jae     .unk
        mov     esi, [load_err_msgs + eax*4]
        test    esi, esi
        jnz     put_message

  .unk:
        mov     esi, unk_err_msg2
        jmp     put_message


OnLoadSymbols:
        xor     ebp, ebp
; load input file
        mov     esi, [curarg]
        call    free_symbols

  .silent:
; esi = ptr to symbols filename
        xor     edi, edi
        cmp     [num_symbols], edi              ; Any previously loaded symbols?
        je      .loadfile
        call    free_symbols                    ; Yep, free them

  .loadfile:
        mov     ebx, fn70_attr_block            ; Get file attributes
        mov     [ebx+21], esi
        mcall   70
        test    eax, eax
        jnz     .fileerr
        cmp     dword [fileattr+36], edi        ; Is upper dword of filesize larger then 0?
        jnz     .memerr
        mov     ecx, dword [fileattr+32]        ; Lower dword of filesize
        mcall   68, 12                          ; allocate the memory
        test    eax, eax
        jz      .memerr
        mov     edi, eax
        mov     ebx, fn70_read_block
        mov     [ebx+12], ecx
        mov     [ebx+16], edi
        mov     [ebx+21], esi
        mcall   70                              ; Read the file into the allocated buffer
        test    eax, eax
        jnz     .fileerr


; calculate memory requirements to load debug symbols

        lea     edx, [ecx+edi-1]                ; edx = EOF-1
        mov     esi, edi
        xor     ecx, ecx

        mov     [symbol_section], 1    ;;;;;

  .calcloop:
        cmp     esi, edx
        jae     .calcdone
        cmp     byte[esi], ' '          ; skip spaces at the beginning of a line
        jne     .not_space
        inc     esi
        jmp     .calcloop
  .not_space:
        cmp     byte[esi], '.'
        jne     .not_section
        inc     esi
        mov     [symbol_section], 0
        cmp     dword[esi], 'text'
        je      .section_ok
        cmp     dword[esi], 'data'
        je      .section_ok
        cmp     dword[esi], 'bss '
        jne     .skipline
  .section_ok:
        inc     [symbol_section]
        jmp     .skipline
  .not_section:
        cmp     [symbol_section], 0
        je      .skipline

        cmp     word[esi], '0x'
        jne     .skipline
        inc     esi
        inc     esi

  @@:
        cmp     esi, edx
        jae     .calcdone
        lodsb
        or      al, 20h
        sub     al, '0'
        cmp     al, 9
        jbe     @b
        sub     al, 'a'-'0'-10
        cmp     al, 15
        jbe     @b
        dec     esi

  @@:
        cmp     esi, edx
        ja      .calcdone
        lodsb
        cmp     al, 20h
        je      @b
        jb      .calcloop
        cmp     al, 9
        jz      @b
        add     ecx, 12+1
        inc     [num_symbols]

  @@:
        inc     ecx
        cmp     esi, edx
        ja      .calcdone
        lodsb
        cmp     al, 0xD
        jz      .calcloop
        cmp     al, 0xA
        jz      .calcloop
        jmp     @b

  .skipline:
        cmp     esi, edx
        jae     .calcdone
        lodsb
        cmp     al, 0xD
        jz      .calcloop
        cmp     al, 0xA
        jz      .calcloop
        jmp     .skipline

  .calcdone:

; Allocate memory to place the debug symbols in

        mcall   68, 12
        test    eax, eax
        jnz     .memok
        inc     ebx
        mov     ecx, edi
        mov     al, 68
        mcall

  .memerr:
        mov     esi, aNoMemory
        jmp     put_message

  .memok:
        mov     [symbols], eax
        mov     ebx, eax
        push    edi
        mov     esi, edi
        mov     edi, [num_symbols]
        lea     ebp, [eax+edi*4]
        lea     edi, [eax+edi*8]

; Actual loading of the debug symbols
; esi->input, edx->EOF, ebx->ptrs, edi->names

        mov     [symbol_section], 1    ;;;;;

  .readloop:
        cmp     esi, edx
        jae     .readdone
        cmp     byte[esi], ' '
        jne     .not_space2
        inc     esi
        jmp     .readloop
  .not_space2:
        cmp     byte[esi], '.'
        jne     .not_section2
        inc     esi
        mov     [symbol_section], 0
        cmp     dword[esi], 'text'
        je      .section_ok2
        cmp     dword[esi], 'data'
        je      .section_ok2
        cmp     dword[esi], 'bss '
        jne     .readline
  .section_ok2:
        inc     [symbol_section]
        jmp     .readline
  .not_section2:
        cmp     [symbol_section], 0
        je      .readline

        cmp     word[esi], '0x'
        jnz     .readline
        inc     esi
        inc     esi
        xor     eax, eax
        xor     ecx, ecx

  @@:
        shl     ecx, 4
        add     ecx, eax
        cmp     esi, edx
        jae     .readdone
        lodsb
        or      al, 20h
        sub     al, '0'
        cmp     al, 9
        jbe     @b
        sub     al, 'a'-'0'-10
        cmp     al, 15
        jbe     @b
        dec     esi

  @@:
        cmp     esi, edx
        ja      .readdone
        lodsb
        cmp     al, 20h
        jz      @b
        jb      .readloop
        cmp     al, 9
        jz      @b
        mov     dword [ebx], edi
        add     ebx, 4
        mov     dword [ebp], edi
        add     ebp, 4
        mov     dword [edi], ecx
        add     edi, 4
        stosb

  @@:
        xor     eax, eax
        stosb
        cmp     esi, edx
        ja      .readdone
        lodsb
        cmp     al, 0xD
        jz      .readloop
        cmp     al, 0xA
        jz      .readloop
        mov     byte [edi-1], al
        jmp     @b

  .readline:
        cmp     esi, edx
        jae     .readdone
        lodsb
        cmp     al, 0xD
        jz      .readloop
        cmp     al, 0xA
        jz      .readloop
        jmp     .readline

  .readdone:
        pop     ecx
        mcall   68, 13
        mov     ecx, [num_symbols]
        mov     edx, [symbols]
        mov     ebx, compare
        call    sort
        mov     ecx, [num_symbols]
        lea     edx, [edx+ecx*4]
        mov     ebx, compare2
        call    sort
        mov     esi, aSymbolsLoaded
        call    put_message
        jmp     draw_disasm.redraw

;-----------------------------------------------------------------------------
;
; in: EAX = address
; out: ESI, CF

find_symbol:
        cmp     [num_symbols], 0
        jnz     @f

  .ret0:
        xor     esi, esi
        stc
        ret

  @@:
        push    ebx ecx edx
        xor     edx, edx
        mov     esi, [symbols]
        mov     ecx, [num_symbols]
        mov     ebx, [esi]
        cmp     [ebx], eax
        jz      .donez
        jb      @f
        pop     edx ecx ebx
        jmp     .ret0

  @@:
    ; invariant: symbols_addr[edx] < eax < symbols_addr[ecx]
    ; TODO: add meaningful label names
  .0:
        push    edx

  .1:
        add     edx, ecx
        sar     edx, 1
        cmp     edx, [esp]
        jz      .done2
        mov     ebx, [esi+edx*4]
        cmp     [ebx], eax
        jz      .done
        ja      .2
        mov     [esp], edx
        jmp     .1

  .2:
        mov     ecx, edx
        pop     edx
        jmp     .0

  .donecont:
        dec     edx

  .done:
        test    edx, edx
        jz      @f
        mov     ebx, [esi+edx*4-4]
        cmp     [ebx], eax
        jz      .donecont

  @@:
        pop     ecx

  .donez:
        mov     esi, [esi+edx*4]
        add     esi, 4
        pop     edx ecx ebx
        clc
        ret

  .done2:
        lea     esi, [esi+edx*4]
        pop     ecx edx ecx ebx
        stc
        ret

;-----------------------------------------------------------------------------
;
; in: esi->name
; out: if found: CF = 0, EAX = value
;      otherwise CF = 1
find_symbol_name:
        cmp     [num_symbols], 0
        jnz     @f

  .stc_ret:
        stc
        ret

  @@:
        push    ebx ecx edx edi
        push    -1
        pop     edx
        mov     ebx, [symbols]
        mov     ecx, [num_symbols]
        lea     ebx, [ebx+ecx*4]
    
    ; invariant: symbols_name[edx] < name < symbols_name[ecx]
  .0:
        push    edx

  .1:
        add     edx, ecx
        sar     edx, 1
        cmp     edx, [esp]
        jz      .done2
        call    .cmp
        jz      .done
        jb      .2
        mov     [esp], edx
        jmp     .1

  .2:
        mov     ecx, edx
        pop     edx
        jmp     .0

  .done:
        pop     ecx

  .donez:
        mov     eax, [ebx+edx*4]
        mov     eax, [eax]
        pop     edi edx ecx ebx
        clc
        ret

  .done2:
        pop     edx edi edx ecx ebx
        stc
        ret

  .cmp:
        mov     edi, [ebx+edx*4]
        push    esi
        add     edi, 4

  @@:
        cmpsb
        jnz     @f
        cmp     byte [esi-1], 0
        jnz     @b

  @@:
        pop     esi
        ret