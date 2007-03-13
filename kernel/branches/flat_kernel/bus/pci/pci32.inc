;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                           ;;
;;  PCI32.INC                                                ;;
;;                                                           ;;
;;  32 bit PCI driver code                                   ;;
;;                                                           ;;
;;  Version 0.2  December 21st, 2002                         ;;
;;                                                           ;;
;;  Author: Victor Prodan, victorprodan@yahoo.com            ;;
;;    Credits:                                               ;;
;;          Ralf Brown                                       ;;
;;          Mike Hibbett, mikeh@oceanfree.net                ;;
;;                                                           ;;
;;  See file COPYING for details                             ;;
;;                                                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;***************************************************************************
;   Function
;      pci_api:
;
;   Description
;       entry point for system PCI calls
;***************************************************************************

align 4

pci_api:

        cmp  [pci_access_enabled],1
        jne  no_pci_access_for_applications

        or al,al
        jnz pci_fn_1
        ; PCI function 0: get pci version (AH.AL)
        movzx eax,word [BOOT_VAR+0x9022]
        ret

pci_fn_1:
        cmp al,1
        jnz pci_fn_2

        ; PCI function 1: get last bus in AL
        mov al,[BOOT_VAR+0x9021]
        ret

pci_fn_2:
        cmp al,2
        jne pci_fn_3
        ; PCI function 2: get pci access mechanism
        mov al,[BOOT_VAR+0x9020]
        ret
pci_fn_3:

        cmp al,4
        jz pci_read_reg   ;byte
        cmp al,5
        jz pci_read_reg   ;word
        cmp al,6
        jz pci_read_reg   ;dword

        cmp al,8
        jz pci_write_reg  ;byte
        cmp al,9
        jz pci_write_reg  ;word
        cmp al,10
        jz pci_write_reg  ;dword

      no_pci_access_for_applications:

        mov eax,-1

        ret

;***************************************************************************
;   Function
;      pci_make_config_cmd
;
;   Description
;       creates a command dword  for use with the PCI bus
;       bus # in ah
;       device+func in bh (dddddfff)
;       register in bl
;
;      command dword returned in eax ( 10000000 bbbbbbbb dddddfff rrrrrr00 )
;***************************************************************************

align 4

pci_make_config_cmd:
    shl     eax,8          ; move bus to bits 16-23
    mov     ax,bx          ; combine all
    and     eax,0xffffff
    or      eax,0x80000000
    ret

;***************************************************************************
;   Function
;      pci_read_reg:
;
;   Description
;       read a register from the PCI config space into EAX/AX/AL
;       IN: ah=bus,device+func=bh,register address=bl
;           number of bytes to read (1,2,4) coded into AL, bits 0-1
;***************************************************************************

align 4

pci_read_reg:
        cmp     byte [BOOT_VAR+0x9020],2 ;what mechanism will we use?
        je      pci_read_reg_2

                ; mechanism 1
        push    esi   ; save register size into ESI
        mov     esi,eax
        and     esi,3

        call    pci_make_config_cmd
        mov     ebx,eax
                ; get current state
        mov     dx,0xcf8
        in      eax, dx
        push    eax
                ; set up addressing to config data
        mov     eax,ebx
        and     al,0xfc ; make address dword-aligned
        out     dx,eax
                ; get requested DWORD of config data
        mov     dl,0xfc
        and     bl,3
        or      dl,bl    ; add to port address first 2 bits of register address

        or      esi,esi
        jz      pci_read_byte1
        cmp     esi,1
        jz      pci_read_word1
        cmp     esi,2
        jz      pci_read_dword1
        jmp     pci_fin_read1

pci_read_byte1:
        in      al,dx
        jmp pci_fin_read1
pci_read_word1:
        in      ax,dx
        jmp pci_fin_read1
pci_read_dword1:
        in      eax,dx
        jmp     pci_fin_read1
pci_fin_read1:
                ; restore configuration control
        xchg    eax,[esp]
        mov     dx,0xcf8
        out     dx,eax

        pop     eax
        pop     esi
        ret
pci_read_reg_2:

        test    bh,128  ;mech#2 only supports 16 devices per bus
        jnz     pci_read_reg_err

        push esi   ; save register size into ESI
        mov esi,eax
        and esi,3

        push    eax
                ;store current state of config space
        mov     dx,0xcf8
        in      al,dx
        mov     ah,al
        mov     dl,0xfa
        in      al,dx

        xchg    eax,[esp]
                ; out 0xcfa,bus
        mov     al,ah
        out     dx,al
                ; out 0xcf8,0x80
        mov     dl,0xf8
        mov     al,0x80
        out     dx,al
                ; compute addr
        shr     bh,3 ; func is ignored in mechanism 2
        or      bh,0xc0
        mov     dx,bx

        or      esi,esi
        jz      pci_read_byte2
        cmp     esi,1
        jz      pci_read_word2
        cmp     esi,2
        jz      pci_read_dword2
        jmp     pci_fin_read2

pci_read_byte2:
        in      al,dx
        jmp pci_fin_read2
pci_read_word2:
        in      ax,dx
        jmp pci_fin_read2
pci_read_dword2:
        in      eax,dx
;       jmp pci_fin_read2
pci_fin_read2:

                ; restore configuration space
        xchg    eax,[esp]
        mov     dx,0xcfa
        out     dx,al
        mov     dl,0xf8
        mov     al,ah
        out     dx,al

        pop     eax
        pop     esi
        ret

pci_read_reg_err:
        xor     eax,eax
        dec     eax
        ret


;***************************************************************************
;   Function
;      pci_write_reg:
;
;   Description
;       write a register from ECX/CX/CL into the PCI config space
;       IN: ah=bus,device+func=bh,register address (dword aligned)=bl,
;           value to write in ecx
;           number of bytes to write (1,2,4) coded into AL, bits 0-1
;***************************************************************************

align 4

pci_write_reg:
        cmp byte [BOOT_VAR+0x9020],2 ;what mechanism will we use?
        je pci_write_reg_2

                ; mechanism 1
        push    esi   ; save register size into ESI
        mov     esi,eax
        and     esi,3

        call    pci_make_config_cmd
        mov     ebx,eax
                ; get current state into ecx
        mov     dx,0xcf8
        in      eax, dx
        push    eax
                ; set up addressing to config data
        mov     eax,ebx
        and     al,0xfc ; make address dword-aligned
        out     dx,eax
                ; write DWORD of config data
        mov     dl,0xfc
        and     bl,3
        or      dl,bl
        mov     eax,ecx

        or      esi,esi
        jz      pci_write_byte1
        cmp     esi,1
        jz      pci_write_word1
        cmp     esi,2
        jz      pci_write_dword1
        jmp     pci_fin_write1

pci_write_byte1:
        out     dx,al
        jmp pci_fin_write1
pci_write_word1:
        out     dx,ax
        jmp pci_fin_write1
pci_write_dword1:
        out     dx,eax
        jmp     pci_fin_write1
pci_fin_write1:

                ; restore configuration control
        pop     eax
        mov     dl,0xf8
        out     dx,eax

        xor     eax,eax
        pop     esi

        ret
pci_write_reg_2:

        test    bh,128  ;mech#2 only supports 16 devices per bus
        jnz     pci_write_reg_err


        push esi   ; save register size into ESI
        mov esi,eax
        and esi,3

        push    eax
                ;store current state of config space
        mov     dx,0xcf8
        in      al,dx
        mov     ah,al
        mov     dl,0xfa
        in      al,dx
        xchg    eax,[esp]
                ; out 0xcfa,bus
        mov     al,ah
        out     dx,al
                ; out 0xcf8,0x80
        mov     dl,0xf8
        mov     al,0x80
        out     dx,al
                ; compute addr
        shr     bh,3 ; func is ignored in mechanism 2
        or      bh,0xc0
        mov     dx,bx
                ; write register
        mov     eax,ecx

        or      esi,esi
        jz      pci_write_byte2
        cmp     esi,1
        jz      pci_write_word2
        cmp     esi,2
        jz      pci_write_dword2
        jmp     pci_fin_write2

pci_write_byte2:
        out     dx,al
        jmp pci_fin_write2
pci_write_word2:
        out     dx,ax
        jmp pci_fin_write2
pci_write_dword2:
        out     dx,eax
        jmp     pci_fin_write2
pci_fin_write2:
                ; restore configuration space
        pop     eax
        mov     dx,0xcfa
        out     dx,al
        mov     dl,0xf8
        mov     al,ah
        out     dx,al

        xor     eax,eax
        pop     esi
        ret

pci_write_reg_err:
        xor     eax,eax
        dec     eax
        ret
