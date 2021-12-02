
include 'sort.inc'

; compare proc for sorter
Compare:
        cmpsd
        jnz     @F
        cmp     esi,edi
  @@:
        ret

; compare proc for sorter 2
Compare2:
        cmpsd
  @@:
        cmpsb
        jnz     @F
        cmp     byte [esi-1],0
        jnz     @B
        cmp     esi,edi
  @@:
        ret

FreeSymbols:
        mov     ecx,[Symbols]
        jecxz   @F
        mcall   68,13
        and     [Symbols],0
        and     [NumSymbols],0
  @@:
        ret

;-----------------------------------------------------------------------------
;                        Load symbols event

OnLoadSymbols.FileErr:
        test    ebp,ebp
        jz      @f
        mcall   68,13,edi
        ret
  @@:
        push    eax
        mcall   68,13,edi
        mov     esi,aCannotLoadFile
        call    PutMessageNoDraw
        pop     eax
        cmp     eax,0x20
        jae     .Unk
        mov     esi,[LoadErrMsgs+eax*4]
        test    esi,esi
        jnz     PutMessage
  .Unk:
        mov     esi,UnkErrMsg2
        jmp     PutMessage

OnLoadSymbols:
        xor     ebp,ebp
; load input file
        mov     esi,[CurArg]
        call    FreeSymbols
  .Silent:
; esi = ptr to symbols filename
        xor     edi,edi
        cmp     [NumSymbols],edi        ; Any previously loaded symbols?
        je      .LoadFile
        call    FreeSymbols             ; Yep, free them
  .LoadFile:
        mov     ebx,FN70AttrBlock       ; Get file attributes
        mov     [ebx+21],esi
        mcall   70
        test    eax,eax
        jnz     .FileErr
        cmp     dword [FileAttr+36],edi ; Is upper dword of filesize larger then 0?
        jnz     .MemErr
        mov     ecx,dword [FileAttr+32] ; Lower dword of filesize
        mcall   68,12                   ; allocate the memory
        test    eax,eax
        jz      .MemErr
        mov     edi,eax
        mov     ebx,FN70ReadBlock
        mov     [ebx+12],ecx
        mov     [ebx+16],edi
        mov     [ebx+21],esi
        mcall   70                      ; Read the file into the allocated buffer
        test    eax,eax
        jnz     .FileErr

; calculate memory requirements to load debug symbols

        lea     edx,[ecx+edi-1]         ; edx = EOF-1
        mov     esi,edi
        xor     ecx,ecx

        mov     [SymbolSection],1       ;;;;;
  .CalcLoop:
        cmp     esi,edx
        jae     .CalcDone
        cmp     byte [esi],' '          ; skip spaces at the beginning of a line
        jne     .NotSpace
        inc     esi
        jmp     .CalcLoop
  .NotSpace:
        cmp     byte [esi],'.'
        jne     .NotSection
        inc     esi
        mov     [SymbolSection],0
        cmp     dword [esi],'text'
        je      .SectionOK
        cmp     dword [esi],'data'
        je      .SectionOK
        cmp     dword [esi],'bss '
        jne     .SkipLine
  .SectionOK:
        inc     [SymbolSection]
        jmp     .SkipLine
  .NotSection:
        cmp     [SymbolSection],0
        je      .SkipLine

        cmp     word [esi],'0x'
        jne     .SkipLine
        inc     esi
        inc     esi
  @@:
        cmp     esi,edx
        jae     .CalcDone
        lodsb
        or      al,20h
        sub     al,'0'
        cmp     al,9
        jbe     @B
        sub     al,'a'-'0'-10
        cmp     al,15
        jbe     @B
        dec     esi
  @@:
        cmp     esi,edx
        ja      .CalcDone
        lodsb
        cmp     al,20h
        je      @B
        jb      .CalcLoop
        cmp     al,9
        jz      @B
        add     ecx,12+1
        inc     [NumSymbols]
  @@:
        inc     ecx
        cmp     esi,edx
        ja      .CalcDone
        lodsb
        cmp     al,0xD
        jz      .CalcLoop
        cmp     al,0xA
        jz      .CalcLoop
        jmp     @B

  .SkipLine:
        cmp     esi,edx
        jae     .CalcDone
        lodsb
        cmp     al,0xD
        jz      .CalcLoop
        cmp     al,0xA
        jz      .CalcLoop
        jmp     .SkipLine

  .CalcDone:

; Allocate memory to place the debug symbols in
        mcall   68,12
        test    eax,eax
        jnz     .MemOK
        inc     ebx
        mov     ecx,edi
        mov     al,68
        mcall
  .MemErr:
        mov     esi,aNoMemory
        jmp     PutMessage
  .MemOK:
        mov     [Symbols],eax
        mov     ebx,eax
        push    edi
        mov     esi,edi
        mov     edi,[NumSymbols]
        lea     ebp,[eax+edi*4]
        lea     edi,[eax+edi*8]

; Actual loading of the debug symbols
; esi->input, edx->EOF, ebx->ptrs, edi->names

        mov     [SymbolSection],1       ;;;;;
  .ReadLoop:
        cmp     esi,edx
        jae     .ReadDone
        cmp     byte [esi],' '
        jne     .NotSpace2
        inc     esi
        jmp     .ReadLoop
  .NotSpace2:
        cmp     byte [esi],'.'
        jne     .NotSection2
        inc     esi
        mov     [SymbolSection],0
        cmp     dword [esi],'text'
        je      .SectionOK2
        cmp     dword [esi],'data'
        je      .SectionOK2
        cmp     dword [esi],'bss '
        jne     .ReadLine
  .SectionOK2:
        inc     [SymbolSection]
        jmp     .ReadLine
  .NotSection2:
        cmp     [SymbolSection],0
        je      .ReadLine

        cmp     word [esi],'0x'
        jnz     .ReadLine
        inc     esi
        inc     esi
        xor     eax,eax
        xor     ecx,ecx
  @@:
        shl     ecx,4
        add     ecx,eax
        cmp     esi,edx
        jae     .ReadDone
        lodsb
        or      al,20h
        sub     al,'0'
        cmp     al,9
        jbe     @B
        sub     al,'a'-'0'-10
        cmp     al,15
        jbe     @B
        dec     esi
  @@:
        cmp     esi,edx
        ja      .ReadDone
        lodsb
        cmp     al,20h
        jz      @B
        jb      .ReadLoop
        cmp     al,9
        jz      @B
        mov     dword [ebx],edi
        add     ebx,4
        mov     dword [ebp],edi
        add     ebp,4
        mov     dword [edi],ecx
        add     edi,4
        stosb
  @@:
        xor     eax,eax
        stosb
        cmp     esi,edx
        ja      .ReadDone
        lodsb
        cmp     al,0xD
        jz      .ReadLoop
        cmp     al,0xA
        jz      .ReadLoop
        mov     byte [edi-1],al
        jmp     @B

  .ReadLine:
        cmp     esi,edx
        jae     .ReadDone
        lodsb
        cmp     al,0xD
        jz      .ReadLoop
        cmp     al,0xA
        jz      .ReadLoop
        jmp     .ReadLine

  .ReadDone:
        pop     ecx
        mcall   68,13
        mov     ecx,[NumSymbols]
        mov     edx,[Symbols]
        mov     ebx,Compare
        call    Sort
        mov     ecx,[NumSymbols]
        lea     edx,[edx+ecx*4]
        mov     ebx,Compare2
        call    Sort
        mov     esi,aSymbolsLoaded
        call    PutMessage
        jmp     DrawMessages;DrawDisAsm

;
; in: EAX = address
; out: ESI, CF
FindSymbol:
        cmp     [NumSymbols],0
        jnz     @F
  .Ret0:
        xor     esi,esi
        stc
        ret
  @@:
        push    ebx ecx edx
        xor     edx,edx
        mov     esi,[Symbols]
        mov     ecx,[NumSymbols]
        mov     ebx,[esi]
        cmp     [ebx],eax
        jz      .DoneZ
        jb      @F
        pop     edx ecx ebx
        jmp     .Ret0
  @@:
    ; invariant: symbols_addr[edx] < eax < symbols_addr[ecx]
    ; TODO: add meaningful label names
  .0:
        push    edx
  .1:
        add     edx,ecx
        sar     edx,1
        cmp     edx,[esp]
        jz      .Done2
        mov     ebx,[esi+edx*4]
        cmp     [ebx],eax
        jz      .Done
        ja      .2
        mov     [esp],edx
        jmp     .1
  .2:
        mov     ecx,edx
        pop     edx
        jmp     .0

  .DoneCont:
        dec     edx
  .Done:
        test    edx,edx
        jz      @F
        mov     ebx,[esi+edx*4-4]
        cmp     [ebx],eax
        jz      .DoneCont
  @@:
        pop     ecx
  .DoneZ:
        mov     esi,[esi+edx*4]
        add     esi,4
        pop     edx ecx ebx
        clc
        ret
  .Done2:
        lea     esi,[esi+edx*4]
        pop     ecx edx ecx ebx
        stc
        ret

;-----------------------------------------------------------------------------
; in: esi->name
; out: if found: CF = 0, EAX = value
;      otherwise CF = 1
FindSymbolName:
        cmp     [NumSymbols],0
        jnz     @F
  .StcRet:
        stc
        ret
  @@:
        push    ebx ecx edx edi
        push    -1
        pop     edx
        mov     ebx,[Symbols]
        mov     ecx,[NumSymbols]
        lea     ebx,[ebx+ecx*4]
    ; invariant: symbols_name[edx] < name < symbols_name[ecx]
  .0:
        push    edx
  .1:
        add     edx,ecx
        sar     edx,1
        cmp     edx,[esp]
        jz      .Done2
        call    .Cmp
        jz      .Done
        jb      .2
        mov     [esp],edx
        jmp     .1
  .2:
        mov     ecx,edx
        pop     edx
        jmp     .0
  .Done:
        pop     ecx
  .DoneZ:
        mov     eax,[ebx+edx*4]
        mov     eax,[eax]
        pop     edi edx ecx ebx
        clc
        ret

  .Done2:
        pop     edx edi edx ecx ebx
        stc
        ret
  .Cmp:
        mov     edi,[ebx+edx*4]
        push    esi
        add     edi,4
  @@:
        cmpsb
        jnz     @F
        cmp     byte [esi-1],0
        jnz     @B
  @@:
        pop     esi
        ret
