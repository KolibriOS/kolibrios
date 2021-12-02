;-----------------------------------------------------------------------------
;                        Working with messages
; in: esi->ASCIIZ message
PutMessageNoDraw:
        mov     edx,[MessagesPos]
    .M:
        lea     edi,[Messages+edx]
    .L:
        lodsb
        cmp     al,0
        jz      .Done
        call    TestScroll
        cmp     al,10
        jz      .NewLine
        cmp     al,'%'
        jnz     @F
        cmp     dword [esp],Z1
        jnz     .Format
    @@:
        stosb
        inc     edx
        jmp     .L

    .NewLine:
        push    edx
        mov     ecx,MSG_WIDTH
        xor     eax,eax
        xchg    eax,edx
        div     ecx
        xchg    eax,edx
        pop     edx
        test    eax,eax
        jz      .M
        sub     edx,eax
        add     edx,ecx
        jmp     .M

    .Done:
if 0
        cmp     byte [esi-2],10
        jz      .Exit
        call    TestScroll
if 1
        push    edx
        mov     ecx,MSG_WIDTH
        xor     eax,eax
        xchg    eax,edx
        div     ecx
        xchg    eax,edx
        pop     edx
        test    eax,eax
        jz      .Exit
        sub     edx,eax
        add     edx,ecx
end if
end if
    .Exit:
        mov     [MessagesPos],edx
        ret

        ; at this moment all format specs must be %<digit>X
    .Format:
        lodsb   ; get <digit>
        sub     al,'0'
        movzx   ecx,al
        lodsb
        pop     eax
        pop     ebp
        push    eax
        ; write number in ebp with ecx digits
        dec     ecx
        shl     ecx,2

    .WriteNibble:
        push    ecx
        call    TestScroll
        pop     ecx
        mov     eax,ebp
        shr     eax,cl
        and     al,0xF
        cmp     al,10
        sbb     al,69h
        das
        stosb
        inc     edx
        sub     ecx,4
        jns     .WriteNibble
        jmp     .L

TestScroll:
        cmp     edx,MSG_WIDTH*MSG_HEIGHT
        jnz     .Ret
        push    esi
        mov     edi,Messages
        lea     esi,[edi+MSG_WIDTH]
        mov     ecx,(MSG_HEIGHT-1)*MSG_WIDTH/4
        rep     movsd
        push    eax
        mov     al,' '
        push    edi
        push    MSG_WIDTH
        pop     ecx
        sub     edx,ecx
        rep     stosb
        pop     edi
        pop     eax
        pop     esi
    .Ret:
        ret

MSG_WIDTH               = DATA_WIDTH
MSG_HEIGHT              = 14                    ; in text lines

NewLine db      10,0
Prompt  db      '> ',0

uglobal
MessagesPos     dd ?
Messages        rb MSG_HEIGHT*MSG_WIDTH
endg