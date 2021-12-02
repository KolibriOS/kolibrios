;-----------------------------------------------------------------------------
;                           Assemble instruction event
OnAsm:  mov     esi,[CurArg]
        mov     edi,esi
        mov     ecx,255
        xor     eax,eax
        repnz   scasb
        stc
        sbb     edi,esi
        jz      .Exit
        ;
        mov     eax,[CPUYPos]
        mov     rax,[AddrBuffer+(rax-1)*sizeof.dq]
        mov     [OriginPtr],rax
        mov     al,[CodeType]
        mov     [code_type],al
        ;
        call    Assemble
        or      esi,esi
        jz      .Move
        ;
if 1
        mov     ecx,MSG_WIDTH-1
        mov     edi,ErrorLine
        push    rdi
.Loop:  lodsb
        stosb
        or      al,al
        loopnz  .Loop
        mov     [rdi-1],word 10
        pop     rsi
        and     byte [rsi],0xDF
end if
        ;
        call    PutMessageNoDraw
        jmp     .Exit
.Move:  mov     eax,[CPUYPos]
        mov     rsi,[AddrBuffer+(rax-1)*sizeof.dq]
        lea     edi,[output_data]
        mcall   69,7,[DebuggeePID],[output_size]
.Exit:  call    PrintCmdLine
        mov     [AfterKey],1
        call    ShowImage
        ret
;-----------------------------------------------------------------------------
uglobal
if used ErrorLine
ErrorLine       rb MSG_WIDTH
end if
endg