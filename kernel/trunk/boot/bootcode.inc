;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  BOOTCODE.INC                                                ;;
;;                                                              ;;
;;  KolibriOS 16-bit loader,                                    ;;
;;                        based on bootcode for MenuetOS        ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;==========================================================================
;
;                           16 BIT FUNCTIONS
;
;==========================================================================


putchar:
; in: al=character
        mov     ah, 0Eh
        mov     bh, 0
        int     10h
        ret

print:
; in: si->string
        mov     al, 186
        call    putchar
        mov     al, ' '
        call    putchar

printplain:
; in: si->string
        pusha
        lodsb
@@:
        call    putchar
        lodsb
        cmp     al, 0
        jnz     @b
        popa
        ret

getkey:
; get number in range [bl,bh] (bl,bh in ['0'..'9'])
; in: bx=range
; out: ax=digit (1..9, 10 for 0)
        mov     ah, 0
        int     16h
        cmp     al, bl
        jb      getkey
        cmp     al, bh
        ja      getkey
        push    ax
        call    putchar
        pop     ax
        and     ax, 0Fh
        jnz     @f
        mov     al, 10
@@:
        ret

setcursor:
; in: dl=column, dh=row
        mov     ah, 2
        mov     bh, 0
        int     10h
        ret

macro _setcursor row,column
{
        mov     dx, row*256 + column
        call    setcursor
}

boot_read_floppy:
        push    si
        xor     si, si
        mov     ah, 2   ; read
@@:
        push    ax
        int     0x13
        pop     ax
        jnc     @f
        inc     si
        cmp     si, 10
        jb      @b
        mov     si, badsect
sayerr_plain:
        call    printplain
        jmp     $
@@:
        pop     si
        ret

;=========================================================================
;
;                           16 BIT CODE
;
;=========================================================================

include 'bootvesa.inc'                 ;Include source for boot vesa 

start_of_code:
        cld
; \begin{diamond}[02.12.2005]
; if bootloader sets ax = 'KL', then ds:si points to loader block
        cmp     ax, 'KL'
        jnz     @f
        mov     word [cs:cfgmanager.loader_block], si
        mov     word [cs:cfgmanager.loader_block+2], ds
@@:
; \end{diamond}[02.12.2005]

; if bootloader sets cx = 'HA' and dx = 'RD', then bx contains identifier of source hard disk
; (see comment to bx_from_load)
        cmp     cx, 'HA'
        jnz     no_hd_load
        cmp     dx,'RD'
        jnz     no_hd_load
        mov     word [cs:bx_from_load], bx              ; {SPraid}[13.03.2007]
no_hd_load:

; set up stack
        mov     ax, 3000h
        mov     ss, ax
        mov     sp, 0EC00h
; set up segment registers
        push    cs
        pop     ds
        push    cs
        pop     es

; set videomode
        mov     ax, 3
        int     0x10

if lang eq ru
 ; Load & set russian VGA font (RU.INC)
        mov     bp, RU_FNT1             ; RU_FNT1 - First part
        mov     bx, 1000h               ; 768 bytes
        mov     cx, 30h                 ; 48 symbols
        mov     dx, 80h                 ; 128 - position of first symbol
        mov     ax, 1100h
        int     10h

        mov     bp, RU_FNT2             ; RU_FNT2 -Second part
        mov     bx, 1000h               ; 512 bytes
        mov     cx, 20h                 ; 32 symbols
        mov     dx, 0E0h                ; 224 - position of first symbol
        mov     ax, 1100h
        int     10h
 ; End set VGA russian font
else if lang eq et
        mov     bp, ET_FNT              ; ET_FNT1
        mov     bx, 1000h               ;
        mov     cx, 255                 ; 256 symbols
        xor     dx, dx                  ; 0 - position of first symbol
        mov     ax, 1100h
        int     10h
end if

; draw frames
        push    0xb800
        pop     es
        xor     di, di
        mov     ah, 1*16+15

; draw top
        mov     si, d80x25_top
        mov     cx, d80x25_top_num * 80
@@:
        lodsb
        stosw
        loop    @b
; draw spaces
        mov     si, space_msg
        mov     dx, 25 - d80x25_top_num - d80x25_bottom_num
dfl1:
        push    si
        mov     cx, 80
@@:
        lodsb
        stosw
        loop    @b
        pop     si
        dec     dx
        jnz     dfl1
; draw bottom
        mov     si, d80x25_bottom
        mov     cx, d80x25_bottom_num * 80
@@:
        lodsb
        stosw
        loop    @b

        mov     byte [space_msg+80], 0    ; now space_msg is null terminated

        _setcursor d80x25_top_num,0


; TEST FOR 386+

        mov     bx, 0x4000
        pushf
        pop     ax
        mov     dx, ax
        xor     ax, bx
        push    ax
        popf
        pushf
        pop     ax
        and     ax, bx
        and     dx, bx
        cmp     ax, dx
        jnz     cpugood
        mov     si, not386
sayerr:
        call    print
        jmp     $
     cpugood:

        push    0
        popf
        sti

; set up esp
        movzx   esp, sp

        push    0
        pop     es
        and     word [es:0x9031], 0
; \begin{Mario79}
; find HDD IDE DMA PCI device
; check for PCI BIOS
        mov     ax, 0xB101
        int     0x1A
        jc      .nopci
        cmp     edx, 'PCI '
        jnz     .nopci
; find PCI class code
; class 1 = mass storage
; subclass 1 = IDE controller
; a) class 1, subclass 1, programming interface 0x80
        mov     ax, 0xB103
        mov     ecx, 1*10000h + 1*100h + 0x80
        xor     si, si  ; device index = 0
        int     0x1A
        jnc     .found
; b) class 1, subclass 1, programming interface 0x8A
        mov     ax, 0xB103
        mov     ecx, 1*10000h + 1*100h + 0x8A
        xor     si, si  ; device index = 0
        int     0x1A
        jnc     .found
; c) class 1, subclass 1, programming interface 0x85
        mov     ax, 0xB103
        mov     ecx, 1*10000h + 1*100h + 0x85
        xor     si, si
        int     0x1A
        jc      .nopci
.found:
; get memory base
        mov     ax, 0xB10A
        mov     di, 0x20        ; memory base is config register at 0x20
        int     0x1A
        jc      .nopci
        and     cx, 0xFFF0      ; clear address decode type
        mov     [es:0x9031], cx
.nopci:
; \end{Mario79}

        mov     al, 0xf6        ; Ñáðîñ êëàâèàòóðû, ðàçðåøèòü ñêàíèðîâàíèå
        out     0x60, al
        xor     cx, cx
wait_loop:       ; variant 2
; reading state of port of 8042 controller
        in      al, 64h
        and     al, 00000010b  ; ready flag
; wait until 8042 controller is ready
        loopnz  wait_loop

;;;/diamond today   5.02.2008
; set keyboard typematic rate & delay
        mov     al, 0xf3
        out     0x60, al
        xor     cx, cx
@@:
        in      al, 64h
        test    al, 2
        loopnz  @b
        mov     al, 0
        out     0x60, al
        xor     cx, cx
@@:
        in      al, 64h
        test    al, 2
        loopnz  @b
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; --------------- APM ---------------------
        and     word [es:0x9044], 0     ; ver = 0.0 (APM not found)
        mov     ax, 0x5300
        xor     bx, bx
        int     0x15
        jc      apm_end                 ; APM not found
        test    cx, 2
        jz      apm_end                 ; APM 32-bit protected-mode interface not supported
        mov     [es:0x9044], ax         ; Save APM Version
        mov     [es:0x9046], cx         ; Save APM flags

        ; Write APM ver ----
        and     ax, 0xf0f
        add     ax, '00'
        mov     si, msg_apm
        mov     [si + 5], ah
        mov     [si + 7], al
        _setcursor 0, 3
        call    printplain
        ; ------------------

        mov     ax, 0x5304              ; Disconnect interface
        xor     bx, bx
        int     0x15
        mov     ax, 0x5303              ; Connect 32 bit mode interface
        xor     bx, bx
        int     0x15

        mov     [es:0x9040], ebx
        mov     [es:0x9050], ax
        mov     [es:0x9052], cx
        mov     [es:0x9054], dx

apm_end:
        _setcursor d80x25_top_num, 0

;CHECK current of code
        cmp     [cfgmanager.loader_block], -1
        jz      noloaderblock
        les     bx, [cfgmanager.loader_block]
        cmp     byte [es:bx], 1
        mov     si, loader_block_error
        jnz     sayerr
        push    0
        pop     es

noloaderblock:
; DISPLAY VESA INFORMATION
         call    print_vesa_info
         call    calc_vmodes_table
         call    check_first_parm  ;check and enable cursor_pos


; \begin{diamond}[30.11.2005]
cfgmanager:
; settings:
; a) preboot_graph = graphical mode
;    preboot_gprobe = probe this mode?
; b) preboot_dma  = use DMA access?
; c) preboot_vrrm = use VRR?
; d) preboot_device = from what boot?

; determine default settings
        mov     [.bSettingsChanged], 0

;.preboot_gr_end:
        mov     di, preboot_device
; if image in memory is present and [preboot_device] is uninitialized,
; set it to use this preloaded image
        cmp     byte [di], 0
        jnz     .preboot_device_inited
        cmp     [.loader_block], -1
        jz      @f
        les     bx, [.loader_block]
        test    byte [es:bx+1], 1
        jz      @f
        mov     byte [di], 3
        jmp     .preboot_device_inited
@@:
; otherwise, set [preboot_device] to 1 (default value - boot from floppy)
        mov     byte [di], 1
.preboot_device_inited:
; following 6 lines set variables to 1 if its current value is 0
        cmp     byte [di+preboot_dma-preboot_device], 1
        adc     byte [di+preboot_dma-preboot_device], 0
        cmp     byte [di+preboot_biosdisk-preboot_device], 1
        adc     byte [di+preboot_biosdisk-preboot_device], 0
        cmp     byte [di+preboot_vrrm-preboot_device], 1
        adc     byte [di+preboot_vrrm-preboot_device], 0
; notify user
        _setcursor 5,2

        mov     si, linef
        call    printplain
        mov     si, start_msg
        call    print
        mov     si, time_msg
        call    print
; get start time
        call    .gettime
        mov     [.starttime], eax
        mov     word [.timer], .newtimer
        mov     word [.timer+2], cs
.printcfg:
        _setcursor 9,0
        mov     si, current_cfg_msg
        call    print
        mov     si, curvideo_msg
        call    print

            call    draw_current_vmode

        mov     si, usebd_msg
        cmp     [preboot_biosdisk], 1
        call    .say_on_off
        mov     si, vrrm_msg
        cmp     [preboot_vrrm], 1
        call    .say_on_off
        mov     si, preboot_device_msg
        call    print
        mov     al, [preboot_device]
        and     eax, 7
        mov     si, [preboot_device_msgs+eax*2]
        call    printplain
.show_remarks:
; show remarks in gray color
        mov     di, ((21-num_remarks)*80 + 2)*2
        push    0xB800
        pop     es
        mov     cx, num_remarks
        mov     si, remarks
.write_remarks:
        lodsw
        push    si
        xchg    ax, si
        mov     ah, 1*16+7      ; background: blue (1), foreground: gray (7)
        push    di
.write_remark:
        lodsb
        test    al, al
        jz      @f
        stosw
        jmp     .write_remark
@@:
        pop     di
        pop     si
        add     di, 80*2
        loop    .write_remarks
.wait:
        _setcursor 25,0         ; out of screen
; set timer interrupt handler
        cli
        push    0
        pop     es
        push    dword [es:8*4]
        pop     dword [.oldtimer]
        push    dword [.timer]
        pop     dword [es:8*4]    
;        mov     eax, [es:8*4]
;        mov     [.oldtimer], eax
;        mov     eax, [.timer]
;        mov     [es:8*4], eax
        sti
; wait for keypressed
        xor     ax,ax
        int     16h
        push    ax
; restore timer interrupt
;        push    0
;        pop     es
        mov     eax, [.oldtimer]
        mov     [es:8*4], eax
        mov     [.timer], eax
        _setcursor 7,0
        mov     si, space_msg
        call    printplain
; clear remarks and restore normal attributes
        push    es
        mov     di, ((21-num_remarks)*80 + 2)*2
        push    0xB800
        pop     es
        mov     cx, num_remarks
        mov     ax, ' ' + (1*16 + 15)*100h
@@:
        push    cx
        mov     cx, 76
        rep     stosw
        pop     cx
        add     di, 4*2
        loop    @b
        pop     es
        pop     ax
; switch on key
        cmp     al, 13
        jz      .continue
        or      al, 20h
        cmp     al, 'a'
        jz      .change_a
        cmp     al, 'b'
        jz      .change_b
        cmp     al, 'c'
        jz      .change_c
        cmp     al, 'd'
        jnz     .show_remarks
        _setcursor 15,0
        mov     si, bdev
        call    print
        mov     bx, '14'
        call    getkey
        mov     [preboot_device], al
        _setcursor 13,0
.d:
        mov     [.bSettingsChanged], 1
        call    clear_vmodes_table             ;clear vmodes_table
        jmp    .printcfg
.change_a:
.loops:
        call    draw_vmodes_table

        xor     ax,ax
        int     0x16
;        call    clear_table_cursor             ;clear current position of cursor

        mov     si,word [cursor_pos]

        cmp     ah,0x48;x,0x48E0               ; up
        jne     .down
        cmp     si,modes_table
        jbe     .loops
        sub     word [cursor_pos],size_of_step
        jmp     .loops

.down:  cmp     ah,0x50;x,0x50E0               ; down
        jne     .pgup
        cmp     word[es:si+10],-1
        je      .loops        
        add     word [cursor_pos],size_of_step
        jmp     .loops

.pgup:  cmp     ah,0x49                 ; page up
        jne     .pgdn
        sub     si, size_of_step*long_v_table
        cmp     si, modes_table
        jae     @f
        mov     si, modes_table
@@:
        mov     word [cursor_pos], si
        mov     si, word [home_cursor]
        sub     si, size_of_step*long_v_table
        cmp     si, modes_table
        jae     @f
        mov     si, modes_table
@@:
        mov     word [home_cursor], si
        jmp     .loops

.pgdn:  cmp     ah,0x51                 ; page down
        jne     .enter
        mov     ax, [end_cursor]
        add     si, size_of_step*long_v_table
        cmp     si, ax
        jb      @f
        mov     si, ax
        sub     si, size_of_step
@@:
        mov     word [cursor_pos], si
        mov     si, word [home_cursor]
        sub     ax, size_of_step*long_v_table
        add     si, size_of_step*long_v_table
        cmp     si, ax
        jb      @f
        mov     si, ax
@@:
        mov     word [home_cursor], si
        jmp     .loops

.enter: cmp     al,0x0D;x,0x1C0D               ; enter
        jne     .loops
        push    word [cursor_pos]
        pop     bp
        push    word [es:bp]
        pop     word [x_save]
        push    word [es:bp+2]
        pop     word [y_save]
        mov     word [preboot_graph],bp           ;save choose
        
        jmp    .d

.change_b:
        _setcursor 15,0
;        mov     si, ask_dma
;        call    print
;        mov     bx, '13'
;        call    getkey
;        mov     [preboot_dma], al
        mov     si, ask_bd
        call    print
        mov     bx, '12'
        call    getkey
        mov     [preboot_biosdisk], al
        _setcursor 11,0
        jmp     .d
.change_c:
        _setcursor 15,0
        mov     si, vrrmprint
        call    print
        mov     bx, '12'
        call    getkey
        mov     [preboot_vrrm], al
        _setcursor 12,0
        jmp     .d
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.say_on_off:
        pushf
        call    print
        mov     si, on_msg
        popf
        jz      @f
        mov     si, off_msg
@@:     jmp     printplain
; novesa and vervesa strings are not used at the moment of executing this code
virtual at novesa
.oldtimer dd ?
.starttime dd ?
.bSettingsChanged db ?
.timer dd ?
end virtual
.loader_block dd -1
.gettime:
        mov     ah, 0
        int     1Ah
        xchg    ax, cx
        shl     eax, 10h
        xchg    ax, dx
        ret
.newtimer:
        push    ds
        push    cs
        pop     ds
        pushf
        call    [.oldtimer]
        pushad
        call    .gettime
        sub     eax, [.starttime]
        sub     ax, 18*5
        jae     .timergo
        neg     ax
        add     ax, 18-1
        mov     bx, 18
        xor     dx, dx
        div     bx
if lang eq ru
; ¯®¤®¦¤¨â¥ 5 á¥ªã­¤, 4/3/2 á¥ªã­¤ë, 1 á¥ªã­¤ã
        cmp     al, 5
        mov     cl, ' '
        jae     @f
        cmp     al, 1
        mov     cl, 'ã'
        jz      @f
        mov     cl, 'ë'
@@:     mov     [time_str+9], cl
else if lang eq et
        cmp     al, 1
        ja      @f
        mov     [time_str+9], ' '
        mov     [time_str+10],' '
@@:
else
; wait 5/4/3/2 seconds, 1 second
        cmp     al, 1
        mov     cl, 's'
        ja      @f
        mov     cl, ' '
@@:     mov     [time_str+9], cl
end if
        add     al, '0'
        mov     [time_str+1], al
        mov     si, time_msg
        _setcursor 7,0
        call    print
        _setcursor 25,0
        popad
        pop     ds
        iret
.timergo:
        push    0
        pop     es
        mov     eax, [.oldtimer]
        mov     [es:8*4], eax
        mov     sp, 0EC00h
.continue:
        sti
        _setcursor 6,0
        mov     si, space_msg
        call    printplain
        call    printplain
        _setcursor 6,0
        mov     si, loading_msg
        call    print
        _setcursor 15,0
        cmp     [.bSettingsChanged], 0
        jz      .load
        cmp     [.loader_block], -1
        jz      .load
        les     bx, [.loader_block]
        mov     eax, [es:bx+3]
        push    ds
        pop     es
        test    eax, eax
        jz      .load
        push    eax
        mov     si, save_quest
        call    print
.waityn:
        mov     ah, 0
        int     16h
        or      al, 20h
        cmp     al, 'n'
        jz      .loadc
        cmp     al, 'y'
        jnz     .waityn
        call    putchar
        mov     byte [space_msg+80], 186
        pop     eax
        push    cs
        push    .cont
        push    eax
        retf
.loadc:
        pop     eax
.cont:
        push    cs
        pop     ds
        mov     si, space_msg
        mov     byte [si+80], 0
        _setcursor 15,0
        call    printplain
        _setcursor 15,0
.load:
; \end{diamond}[02.12.2005]

; ASK GRAPHICS MODE

        call    set_vmode

; GRAPHICS ACCELERATION
; force yes
        mov     [es:0x901C], byte 1

; DMA ACCESS TO HD

        mov     al, [preboot_dma]
        mov     [es:0x901F], al

; VRR_M USE

        mov     al,[preboot_vrrm]
        mov     [es:0x9030], al
        mov     [es:0x901E], byte 1

; BOOT DEVICE

        mov     al, [preboot_device]
        dec     al
        mov     [boot_dev], al

; READ DISKETTE TO MEMORY

;        cmp     [boot_dev],0
        jne     no_sys_on_floppy
        mov     si,diskload
        call    print
        xor     ax, ax            ; reset drive
        xor     dx, dx
        int     0x13
; do we boot from CD-ROM?
        mov     ah, 41h
        mov     bx, 55AAh
        xor     dx, dx
        int     0x13
        jc      .nocd
        cmp     bx, 0AA55h
        jnz     .nocd
        mov     ah, 48h
        push    ds
        push    es
        pop     ds
        mov     si, 0xa000
        mov     word [si], 30
        int     0x13
        pop     ds
        jc      .nocd
        push    ds
        lds     si, [es:si+26]
        test    byte [ds:si+10], 40h
        pop     ds
        jz      .nocd
; yes - read all floppy by 18 sectors
        mov     cx, 0x0001      ; startcyl,startsector
.a1:
        push    cx dx
        mov     al, 18
        mov     bx, 0xa000
        call    boot_read_floppy
        mov     si, movedesc
        push    es
        push    ds
        pop     es
        mov     cx, 256*18
        mov     ah, 0x87
        int     0x15
        pop     es
        pop     dx cx
        test    ah, ah
        jnz     sayerr_floppy
        add     dword [si+8*3+2], 512*18
        inc     dh
        cmp     dh, 2
        jnz     .a1
        mov     dh, 0
        inc     ch
        cmp     ch, 80
        jae     ok_sys_on_floppy
        pusha
        mov     al, ch
        shr     ch, 2
        add     al, ch
        aam
        xchg    al, ah
        add     ax, '00'
        mov     si, pros
        mov     [si], ax
        call    printplain
        popa
        jmp     .a1
.nocd:
; no - read only used sectors from floppy
; now load floppy image to memory
; at first load boot sector and first FAT table
        mov     cx, 0x0001      ; startcyl,startsector
        xor     dx, dx          ; starthead,drive
        mov     al, 1+9         ; no of sectors to read
        mov     bx, 0xB000      ; es:bx -> data area
        call    boot_read_floppy
; and copy them to extended memory
        mov     si, movedesc
        mov     [si+8*2+3], bh
        push    es
        push    ds
        pop     es
        mov     cx, 256*10
        mov     ah, 0x87
        int     0x15
        test    ah, ah
        jz      @f
sayerr_floppy:
        mov     dx, 0x3f2
        mov     al, 0
        out     dx, al
        mov     si, memmovefailed
        jmp     sayerr_plain
@@:
        add     dword [si+8*3+2], 512*10
; copy FAT to second copy
        mov     byte [si+8*2+3], 0xB2
        mov     cx, 256*9
        mov     ah, 0x87
        int     0x15
        pop     es
        test    ah, ah
        jnz     sayerr_floppy
        add     dword [si+8*3+2], 512*9
; calculate total number of sectors to read
        mov     ax, 1+9+14      ; boot+FAT+root
        mov     di, 0xB203
.calc_loop:
        test    word [es:di], 0xFFF
        jz      @f
        inc     ax
@@:
        test    word [es:di+1], 0xFFF0
        jz      @f
        inc     ax
@@:
        add     di, 3
        cmp     di, 0xB200+1440*3
        jb      .calc_loop
        push    ax
        mov     bp, 1+9         ; already read sectors
; now read rest
        mov     byte [si+8*2+3], 0xA0
        mov     di, 2-14        ; absolute sector-31
        mov     cx, 0x0002      ; cylinder=0, sector=2
        mov     dx, 0x0100      ; head=1, disk=0
.read_loop:
; determine whether sector must be read
        cmp     di, 2
        jl      .read
        mov     bx, di
        shr     bx, 1
        jnc     .even
        test    word [es:bx+di+0xB200], 0xFFF0
        jmp     @f
.even:
        test    word [es:bx+di+0xB200], 0xFFF
@@:
        jz      .skip
.read:
        mov     bx, 0xA000
        mov     al, 1           ; 1 sector
        call    boot_read_floppy
        inc     bp
        push    es
        push    ds
        pop     es
        pusha
        mov     cx, 256
        mov     ah, 0x87
        int     0x15
        test    ah, ah
        popa
        pop     es
        jnz     sayerr_floppy
.skip:
        add     dword [si+8*3+2], 512
        inc     cx
        cmp     cl, 19
        jnz     @f
        mov     cl, 1
        inc     dh
        cmp     dh, 2
        jnz     @f
        mov     dh, 0
        inc     ch
@@:
        pop     ax
        push    ax
        pusha
; draw percentage
; total sectors: ax
; read sectors: bp
        xchg    ax, bp
        mov     cx, 100
        mul     cx
        div     bp
        aam
        xchg    al, ah
        add     ax, '00'
        mov     si, pros
        cmp     [si], ax
        jz      @f
        mov     [si], ax
        call    printplain
@@:
        popa
        inc     di
        cmp     di, 2880-31
        jnz     .read_loop

;        mov     cx, 0x0001      ; startcyl,startsector
;        xor     dx, dx          ; starthead,drive
;        push    word 80*2               ; read no of sect
;       reads:
;        pusha
;        xor     si,si
;       newread:
;        mov     bx,0xa000               ; es:bx -> data area
;        mov     ax,0x0200+18            ; read, no of sectors to read
;        int     0x13
;        test    ah, ah
;        jz      goodread
;        inc    si
;        cmp     si,10
;        jnz     newread
;        mov     si,badsect-0x10000
;sayerr_plain:
;        call    printplain
;        jmp     $
;       goodread:
;        ; move -> 1mb
;        mov     si,movedesc-0x10000
;        push    es
;        push    ds
;        pop     es
;        mov     cx,256*18
;        mov     ah,0x87
;        int     0x15
;        pop    es
;
;        test    ah,ah                  ; was the move successfull ?
;        je      goodmove
;        mov     dx,0x3f2              ; floppy motor off
;        mov     al,0
;        out     dx,al
;        mov     si,memmovefailed-0x10000
;        jmp    sayerr_plain
;      goodmove:
;
;    add    dword [movedesc-0x10000+0x18+2], 512*18
;        popa
;        inc     dh
;        cmp     dh,2
;        jnz     bb2
;        mov     dh,0
;        inc     ch
;        pusha                        ; print prosentage
;        mov     si,pros-0x10000
;    shr    ch, 2
;    mov    al, '5'
;    test    ch, 1
;    jnz    @f
;    mov    al, '0'
;@@:
;    mov    [si+1], al
;    shr    ch, 1
;    add    ch, '0'
;    mov    [si], ch
;        call    printplain
;        popa
;       bb2:
;        pop     ax
;        dec     ax
;        push    ax
;        jnz     reads
;       readdone:
;        pop     ax

ok_sys_on_floppy:
        mov     si, backspace2
        call    printplain
        mov     si, okt
        call    printplain
no_sys_on_floppy:
        xor     ax, ax          ; reset drive
        xor     dx, dx
        int     0x13
        mov     dx, 0x3f2       ; floppy motor off
        mov     al, 0
        out     dx, al


; SET GRAPHICS

        xor     ax, ax
        mov     es, ax

        mov     ax, [es:0x9008]         ; vga & 320x200
        mov     bx, ax
        cmp     ax, 0x13
        je      setgr
        cmp     ax, 0x12
        je      setgr
        mov     ax, 0x4f02              ; Vesa
setgr:
        int     0x10
        test    ah, ah
        mov     si, fatalsel
        jnz     v_mode_error
; set mode 0x12 graphics registers:
        cmp     bx, 0x12
        jne     gmok2

        mov     al, 0x05
        mov     dx, 0x03ce
        push    dx
        out     dx, al      ; select GDC mode register
        mov     al, 0x02
        inc     dx
        out     dx, al      ; set write mode 2

        mov     al, 0x02
        mov     dx, 0x03c4
        out     dx, al      ; select VGA sequencer map mask register
        mov     al, 0x0f
        inc     dx
        out     dx, al      ; set mask for all planes 0-3

        mov     al, 0x08
        pop     dx
        out     dx, al      ; select GDC bit mask register
                           ; for writes to 0x03cf
gmok2:
        push    ds
        pop     es
