;### fdc.inc ###  Menuetos floppy stuff.
;Version 0.2: Write individual tracks. / Sync ramdisk <-> floppy
;Version 0.1: Write full ramdisk to floppy.
;£loppyright Tolle.

;depends on:
;restorefatchain
;memmove
;Int 6 (sys32.inc) should call fdc_irq func.
;The ramdisk should be at 0x100000

;Keeping track of the tracks.
iglobal
  cylinder        db 0
  sector          db 1
  head            db 0

  ;Memory and dma variables.
  fdcmem          dd 0x100000
  cpymem          dd 0x100000
  dmamem          dd 0x100000
endg

uglobal
  dmasize         db 0x0
  dmamode         db 0x0
endg

iglobal
  ;function pointers.
  fdc_irq_func    dd fdc_null
  fdc_pump_func   dd fdc_null
endg

uglobal
  ;General stuff
  fdc_st0         db 0            ;status register 0 of last resultphase.
  fdc_mutex       db 0            ;wait in line. (Block calling app)
  fdc_callspending        db 0            ;mystery sauce
  fdc_settings    dd 0            ;bitfield.
endg
                                ;Bit 0  enable direct file write [yes/no]

fdc_set:                                ;ebx: fdc_settings bitfield.
mov [fdc_settings],ebx
ret

fdc_get:                                ;returns fdc_settings in ecx
mov ecx, [fdc_settings]
ret

fdc_init:                               ;start with clean tracks.
        mov edi,0xD201
        mov al,0
        mov ecx,160
        rep stosb
ret

fdc_filesave:                   ;ebx: cluster to be saved.
        pusha                   ;returns immediately. does not trigger a write.
        mov eax,ebx
        add eax,31
        mov bl,18
        div bl
        mov ah,0
        add eax,0xD201
        mov [eax],byte 1                ;This track is now dirty.
        popa
ret


fdc_writeramdisk:                       ;mark all tracks as dirty.
        mov edi,0xD201
        mov al,1
        mov ecx,160
        rep stosb
        jmp fdc_commitflush
fdc_commitfile:                 ;flush dirty tracks to floppy
        test [fdc_settings],1   ;...but only if this is really wanted by the user.
        je fdc_commitend
fdc_commitflush:
        cmp [fdc_callspending],5
        je fdc_commitend
        inc [fdc_callspending]
        cmp [fdc_callspending],1
        je fdc_commitonce
        fdc_commitend:
ret

fdc_commitonce:                 ;One at a time.
        .stall:
cli
        cmp [fdc_mutex],0
        jne .stallret
        mov [fdc_mutex],1
        jmp .goahead
        .stallret:
sti
        jmp .stall
        .goahead:
sti

fdc_commitramdisk:

        call restorefatchain
                                ;Move the bootsector to a safe place.
        mov eax,0x100000
        mov ebx,0xD000
        mov ecx,512
        call memmove
                                ;Always write the FAT table
        mov eax,0xD201
        mov [eax],byte 1
        inc eax
        mov [eax],byte 1

        mov [dmamode],0x4A      ;read from memory to floppy.
        mov [dmasize],0x1               ;read 512 bytes sectors.
        mov [fdc_irq_func],fdc_commitramdisk1
        call fdc_floppy_on              ;start floppy A: moter starts interruptflow.
        ret
fdc_commitramdisk1:
        mov [fdc_irq_func],fdc_recalibrate_result
        mov [fdc_pump_func],fdc_commitramdisk2
        call fdc_recalibrate            ;retract the head to cylinder 0, sector 1
        ret
fdc_commitramdisk2:
        mov[head],0             ;set variables.
        mov[cylinder],0
        mov [sector],1
        mov[cpymem],0x102400
        mov [fdc_pump_func],fdc_fullpump
        call fdc_write          ;fdc_write will continue interruptflow
ret

fdc_fullpump:
        add [dmamem],512
        add [sector],1
        cmp [sector],19
        jne .clusterwrite
        sub [dmamem],9216
        mov eax,[cpymem]
        mov ebx,[fdcmem]
        mov ecx,9216
        call memmove
        add [cpymem],9216
        cmp [head],0
        je .nocylinderchange
        add [cylinder],1
        .nocylinderchange:
        xor [head],1
        cmp [cylinder],80
        jne .noendofwrite
        mov[fdc_irq_func],fdc_complete
        call fdc_floppy_off
        call fdc_init
        jmp .end
        .noendofwrite:
        mov [sector],1
        .clusterwrite:
        xor eax,eax
        mov al,[cylinder]
        shl eax,1
        add al,[head]
        add eax,0xD201
        mov bl,[eax]
        cmp bl,1
        jne fdc_fullpump
        call fdc_write
        .end:
ret

fdc_write:
        call fdc_program_dma
        call fdc_seek
ret

fdc_seek:
        mov al, 0x0f
        call fdc_write_reg
        mov al,[head]
        shl al,2
        call fdc_write_reg
        mov al,[cylinder]
        call fdc_write_reg
        mov [fdc_irq_func],fdc_seek_result
ret

fdc_seek_result:
        call fdc_sensei
        cmp al,[cylinder]
        je .succes
        call fdc_seek
        jmp .end
        .succes:
        call fdc_write_sector
        .end:
ret

fdc_write_sector:
        mov al,0x45             ;write sector command
fdc_commandphase:
        call fdc_write_reg
        mov al,[head]
        shl al,2
        call fdc_write_reg
        mov al,[cylinder]
        call fdc_write_reg
        mov al,[head]
        call fdc_write_reg
        mov al,[sector]
        call fdc_write_reg
        mov al,2                        ;Sector size (2 ~> 512 bytes)
        call fdc_write_reg
        mov al,18                       ;last sector on track.
        call fdc_write_reg
        mov al,27                       ;length of GAP3
        call fdc_write_reg
        mov al,0xFF             ;data length, ignored.
        call fdc_write_reg
        mov [fdc_irq_func],fdc_resultphase
ret

fdc_resultphase:
        call fdc_read_reg
        mov [fdc_st0],al
        mov cx,6
        .readresult:
        call fdc_read_reg
        loop .readresult
        and [fdc_st0],11000000b
        cmp [fdc_st0],byte 0
        jz .succes
        call fdc_seek
        jmp .end
        .succes:
        call [fdc_pump_func]
        .end:
ret

fdc_sensei:
        mov al,0x08             ;get interrupt status command
        call fdc_write_reg
        call fdc_read_reg               ;get result in al;
        and al,0x80
        cmp al,0x80
        je fdc_sensei           ;retry
        call fdc_read_reg
ret

fdc_program_dma:
        mov al,0
        out 0x0c,al     ; reset the flip-flop to a known state.
        mov al,6                ; mask channel 2 so we can reprogram it.
        out 0x0a,al
        mov al,[dmamode]        ; 0x46 -> Read from floppy - 0x4A Write to floppy
        out 0x0b,al
        mov al,0
        out 0x0c,al     ; reset the flip-flop to a known state.
        mov eax,[dmamem]
        out 0x04,al     ; set the channel 2 starting address to 0
        shr eax,8
        out 0x04,al
        shr eax,8
        out 0x81,al
        mov al,0
        out 0x0c, al    ; reset flip-flop
        mov al, 0xff    ;set count (actual size -1)
        out 0x5, al
        mov al, [dmasize]       ;(0x1ff = 511 / 0x23ff =9215)
        out 0x5,al
        mov al,2
        out 0xa,al
ret

fdc_recalibrate:
        mov al,0x07             ;calibrate command
        call fdc_write_reg
        mov al,0                        ;select drive 0
        call fdc_write_reg
ret

fdc_recalibrate_result:
        mov al,0x08             ;get interrupt status command
        call fdc_write_reg              ;send it
        call fdc_read_reg               ;get command in al;
        cmp al,0x80
        je fdc_recalibrate_result
        mov ah,al
        call fdc_read_reg
        cmp ah,0x70
        jne .end
        call fdc_recalibrate
        jmp .reallyend
        .end:
        call [fdc_pump_func]
        .reallyend:
ret

fdc_busy:
        .command_check:
        mov dx,0x3F4
        in al,dx
        and al,0x10
        cmp al,0x10
        je .command_check
ret

fdc_read_reg:
        status_check:
        mov dx,0x3F4
        in al,dx
        and al,0xc0
        cmp al,0xc0
        jne status_check
        mov dx, 0x3F5
        in al, dx
ret

fdc_write_reg:
        mov bl,al
        .command_check:
        mov dx,0x3F4
        in al,dx
        and al,0x80
        cmp al,0x80
        jne .command_check
        mov al,bl
        mov dx,0x3F5
        out dx,al
ret

fdc_floppy_off:
        mov al,0xC
        mov dx,0x3f2
        out dx,al
ret

fdc_floppy_on:
        mov  dx,0x3f2
        mov  al,0x0
        out  dx,al
        mov  al,0x1C
        out  dx,al
        
        mov  eax,50
        call delay_hs
ret

fdc_complete:
        mov eax,0xD000
        mov ebx,0x100000
        mov ecx,512
        call memmove

        mov [fdc_irq_func],fdc_null
        mov [fdc_mutex],0
        dec [fdc_callspending]
        cmp [fdc_callspending],0
        je  .realyend
        mov [fdc_mutex],1
        call fdc_commitramdisk
        .realyend:
ret

fdc_irq:
        call [fdc_irq_func]
fdc_null:
ret