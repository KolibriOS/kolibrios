; --------------------------------------------------------------------------
; FILE: TMsgTable.Asm
; DATE: September 28, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
align PROC_ALIGN
TMsgTable_CountItems:
    mov     esi, [glb_pRawMsgBuffer]
    mcZeroBits eax
    mcZeroBits ecx
    mcZeroBits edx

.load_char:
    lodsb
    mcOnRegZero eax, .done

    cmp     al, 0Dh
    sete    dl
    add     ecx, edx
    jmp     .load_char

.done:
    mov     [glb_NumMsgItems], ecx
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TMsgTable_Parse:
    mov     ecx, [glb_NumMsgItems]
    shl     ecx, 2
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, ecx
    mov     [glb_pMsgItems], eax

    mov     edi, eax
    mov     esi, [glb_pRawMsgBuffer]
    mcZeroBits eax
    mov     [glb_NumMsgItems], eax

.load_char:
    lodsb
    mcOnRegZero eax, .done

    mcOnRegEqu al, '{', .begin_item
    mcOnRegEqu al, '}', .end_item
    jmp     .load_char

.begin_item:
    mov     [edi], esi
    add     edi, 4
    inc     [glb_NumMsgItems]
    jmp     .load_char

.end_item:
    lea     edx, [esi - 1]
    mov     byte [edx], 0
    jmp     .load_char

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   EDI = buffer address
;   ECX = buffer size in bytes
; --------------------------------------------------------------------------
align PROC_ALIGN
TMsgTable_Decrypt:
    pushad

.reset_key:
    mov     esi, TMsgTable_GetItem
    mcLoad8bitsToReg32 edx, 27

.next:
    lodsb
    xor     [edi], al
    inc     edi

    dec     ecx
    jz      .done

    dec     edx
    jz      .reset_key
    jmp     .next

.done:
    popad
    ret

; --------------------------------------------------------------------------
virtual at 0
loc4:
    .hFile HFILE ?
    .dwSize UINT32 ?
    .dwLoaded UINT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TMsgTable_Load:
    mcBeginLocals loc4.size

    invoke  CreateFile, str_MsgFileName, GENERIC_READ, \
            0, 0, OPEN_EXISTING, FILE_FLAG_SEQ_SCAN, 0

    mcStoreLocal loc4.hFile, eax
    cmp     eax, -1
    je      .done

    mov     ebx, esp
    invoke  GetFileSize, [ebx + loc4.hFile], 0
    mcStoreLocal loc4.dwSize, eax
    mcOnRegZero eax, .close

    inc     eax
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, eax
    mov     [glb_pRawMsgBuffer], eax
    mcOnRegZero eax, .close

    mcLoadMemberRef edi, loc4.dwLoaded

    invoke  ReadFile, [ebx + loc4.hFile], eax, \
            [ebx + loc4.dwSize], edi, 0

    mov     edi, [glb_pRawMsgBuffer]
    mcLoadLocal ecx, loc4.dwLoaded
    mcZeroBits eax
    mov     [edi + ecx], al

    call    TMsgTable_Decrypt
    call    TMsgTable_CountItems
    call    TMsgTable_Parse

.close:
    mov     ebx, esp
    invoke  CloseHandle, [ebx + loc4.hFile]

.done:
    mcEndLocals loc4.size
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = item index (1-based)
; Output:
;   ESI = points to ANSI text at specified index or empty string
; --------------------------------------------------------------------------
align PROC_ALIGN
TMsgTable_GetItem:
    cmp     ecx, [glb_NumMsgItems]
    ja      .empty

    dec     ecx
    push    edx
    mov     edx, [glb_pMsgItems]
    mov     esi, [edx + ecx*4]
    pop     edx
    ret

.empty:
    mov     esi, str_CharSizeText + 4
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = item index (1-based)
; Output:
;   EAX = string length
; --------------------------------------------------------------------------
align PROC_ALIGN
TMsgTable_GetLength:
    push    esi ecx
    call    TMsgTable_GetItem

    mcZeroBits eax
    mcZeroBits ecx

.next_char:
    lodsb
    mcOnRegZero eax, .done

    inc     ecx
    jmp     .next_char

.done:
    mov     eax, ecx
    pop     ecx esi
    ret

; --- EOF ---
