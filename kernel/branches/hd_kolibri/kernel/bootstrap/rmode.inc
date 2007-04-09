; SAVE REAL MODE VARIABLES
        mov     ax, [0x2f0000 + 0x9031]
        mov     [IDEContrRegsBaseAddr], ax
; --------------- APM ---------------------
    mov    eax, [0x2f0000 + 0x9040]    ; entry point
    mov    dword[apm_entry], eax
    mov    word [apm_entry + 4], apm_code_32 - gdts

    mov    eax, [0x2f0000 + 0x9044]    ; version & flags
    mov    [apm_vf], eax
; -----------------------------------------
;        movzx eax,byte [0x2f0000+0x9010]  ; mouse port
;        mov   [0xF604],byte 1  ;al
        mov     al, [0x2F0000+0x901F]   ; DMA writing
        mov     [allow_dma_write], al
        mov   al,[0x2f0000+0x9000]        ; bpp
        mov   [ScreenBPP],al
        movzx eax,word [0x2f0000+0x900A]  ; X max
        dec   eax
        mov   [ScreenWidth],eax
        mov   [screen_workarea.right],eax
        movzx eax,word [0x2f0000+0x900C]  ; Y max
        dec   eax
        mov   [ScreenHeight],eax
        mov   [screen_workarea.bottom],eax
        movzx eax,word [0x2f0000+0x9008]  ; screen mode
        mov   [SCR_MODE],eax
        mov   eax,[0x2f0000+0x9014]       ; Vesa 1.2 bnk sw add
        mov   [BANK_SWITCH],eax
        mov   [BytesPerScanLine],word 640*4         ; Bytes PerScanLine
        cmp   [SCR_MODE],word 0x13          ; 320x200
        je    @f
        cmp   [SCR_MODE],word 0x12          ; VGA 640x480
        je    @f
        mov   ax,[0x2f0000+0x9001]        ; for other modes
        mov   [BytesPerScanLine],ax
      @@:
