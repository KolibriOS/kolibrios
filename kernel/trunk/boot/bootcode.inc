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
        test    al, al
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
sayerr_badsect:
        mov     si, badsect
sayerr_plain:
        call    printplain
        jmp     $
@@:
        pop     si
        ret

; convert abs. sector number (AX) to BIOS T:H:S
; sector number = (abs.sector%BPB_SecPerTrk)+1
; pre.track number = (abs.sector/BPB_SecPerTrk)
; head number = pre.track number%BPB_NumHeads
; track number = pre.track number/BPB_NumHeads
; Return: cl - sector number
;         ch - track number
;         dl - drive number (0 = a:)
;         dh - head number
conv_abs_to_THS:
        push    bx
        mov     bx, word [BPB_SecPerTrk]
        xor     dx, dx
        div     bx
        inc     dx
        mov     cl, dl                          ; cl = sector number
        mov     bx, word [BPB_NumHeads]
        xor     dx, dx
        div     bx
        ; !!!!!!! ax = track number, dx = head number
        mov     ch, al                          ; ch=track number
        xchg    dh, dl                          ; dh=head number
        mov     dl, 0                           ; dl=0 (drive 0 (a:))
        pop     bx
        retn
; needed variables
BPB_SecPerTrk   dw      0                       ; sectors per track
BPB_NumHeads    dw      0                       ; number of heads
BPB_FATSz16     dw      0                       ; size of FAT
BPB_RootEntCnt  dw      0                       ; count of root dir. entries
BPB_BytsPerSec  dw      0                       ; bytes per sector
BPB_RsvdSecCnt  dw      0                       ; number of reserved sectors
BPB_TotSec16    dw      0                       ; count of the sectors on the volume
BPB_SecPerClus  db      0                       ; number of sectors per cluster
BPB_NumFATs     db      0                       ; number of FAT tables
abs_sector_adj  dw      0                       ; adjustment to make abs. sector number
end_of_FAT      dw      0                       ; end of FAT table
FirstDataSector dw      0                       ; begin of data

;=========================================================================
;
;                           16 BIT CODE
;
;=========================================================================

include 'bootvesa.inc'                 ;Include source for boot vesa 
if defined extended_primary_loader
include 'parsers.inc'
end if

start_of_code:

if defined extended_primary_loader
; save data from primary loader
        mov     word [cs:bootcallback], si
        mov     word [cs:bootcallback+2], ds
        push    cs
        pop     ds
        mov     [bootdevice], ax
        mov     [bootfs], bx

; set up stack
        mov     ax, 3000h
        mov     ss, ax
        mov     sp, 0EC00h

; try to load configuration file
        mov     ax, 1
        mov     di, config_file_struct
        call    [bootcallback]
        cld
        push    cs
        pop     es
; bx=0 - ok, bx=1 - part of file loaded, assume this is ok
        cmp     bx, 1
        ja      .config_bad
; configuration file was loaded, parse
; if length is too big, use first 0FFFFh bytes
        test    dx, dx
        jz      @f
        mov     ax, 0FFFFh
@@:
; ds:si will be pointer to current data, dx = limit
        xchg    ax, dx
        push    4000h
        pop     ds
        xor     si, si
.parse_loop:
; skip spaces
        cmp     si, dx
        jae     .parse_done
        lodsb
        cmp     al, ' '
        jbe     .parse_loop
        dec     si
; loop over all possible configuration values
        mov     bx, config_file_variables
.find_variant:
; get length
        mov     cx, [es:bx]
; zero length = end of list
        jecxz   .find_newline
; skip over length
        inc     bx
        inc     bx
        mov     di, bx
; skip over string
        add     bx, cx
; test whether we have at least cx symbols left
        mov     ax, cx
        add     ax, si
        jc      .next_variant1
        cmp     ax, dx
        jae     .next_variant1
; save current position
        push    si
; compare strings
        repz cmpsb
        jnz     .next_variant2
; strings are equal; look for "=" with possible spaces before and after
@@:
        cmp     si, dx
        jae     .next_variant2
        lodsb
        cmp     al, ' '
        jbe     @b
        cmp     al, '='
        jnz     .next_variant2
; ok, we found the true variant
; ignore saved position on the stack
        pop     ax
; call the parser
        call    word [es:bx]
; line parsed, find next
.find_newline:
        cmp     si, dx
        jae     .parse_done
        lodsb
        cmp     al, 13
        jz      .parse_loop
        cmp     al, 10
        jz      .parse_loop
        jmp     .find_newline
.next_variant2:
; continue to the next variant, restoring current position
        pop     si
.next_variant1:
; continue to the next variant
; skip over the parser
        inc     bx
        inc     bx
        jmp     .find_variant
.parse_done:
.config_bad:

; set up segment registers
        push    cs
        pop     ds
else
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
        cmp     dx, 'RD'
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
end if

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

if ~ defined extended_primary_loader
;CHECK current of code
        cmp     [cfgmanager.loader_block], -1
        jz      noloaderblock
        les     bx, [cfgmanager.loader_block]
        cmp     byte [es:bx], 1
        mov     si, loader_block_error
        jnz     sayerr
        push    0
        pop     es
end if

noloaderblock:
; DISPLAY VESA INFORMATION
        call    print_vesa_info
        call    calc_vmodes_table
        call    check_first_parm   ;check and enable cursor_pos

; \begin{diamond}[30.11.2005]
cfgmanager:
; settings:
; a) preboot_graph = graphical mode
;    preboot_gprobe = probe this mode?
; b) preboot_dma  = use DMA access?
; c) preboot_vrrm = use VRR?
; d) preboot_device = from what boot?

; determine default settings
if ~ defined extended_primary_loader
        mov     [.bSettingsChanged], 0
end if

;.preboot_gr_end:
        mov     di, preboot_device
; if image in memory is present and [preboot_device] is uninitialized,
; set it to use this preloaded image
        cmp     byte [di], 0
        jnz     .preboot_device_inited
if defined extended_primary_loader
        inc     byte [di]
        cmp     byte [bootdevice], 'f' ; floppy?
        jz      .preboot_device_inited
        inc     byte [di]
else
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
end if
.preboot_device_inited:
; following 4 lines set variables to 1 if its current value is 0
        cmp     byte [di+preboot_dma-preboot_device], 1
        adc     byte [di+preboot_dma-preboot_device], 0
        cmp     byte [di+preboot_biosdisk-preboot_device], 1
        adc     byte [di+preboot_biosdisk-preboot_device], 0
;; default value for VRR is OFF
;        cmp     byte [di+preboot_vrrm-preboot_device], 0
;        jnz    @f
;        mov    byte [di+preboot_vrrm-preboot_device], 2
;@@:
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
;        mov     si, vrrm_msg
;        cmp     [preboot_vrrm], 1
;        call    .say_on_off
        mov     si, preboot_device_msg
        call    print
        mov     al, [preboot_device]
if defined extended_primary_loader
        and     eax, 3
else
        and     eax, 7
end if
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
        xor     ax, ax
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
        rep stosw
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
;        cmp     al, 'c'
;        jz      .change_c
        cmp     al, 'c'         ; 'd'
        jnz     .show_remarks
        _setcursor 15,0
        mov     si, bdev
        call    print
if defined extended_primary_loader
        mov     bx, '12'
else
        mov     bx, '14'
end if
        call    getkey
        mov     [preboot_device], al
        _setcursor 13,0
.d:
if ~ defined extended_primary_loader
        mov     [.bSettingsChanged], 1
end if
        call    clear_vmodes_table             ;clear vmodes_table
        jmp     .printcfg
.change_a:
.loops:
        call    draw_vmodes_table
        _setcursor 25,0         ; out of screen
        xor     ax, ax
        int     0x16
;        call    clear_table_cursor             ;clear current position of cursor

        mov     si, word [cursor_pos]

        cmp     ah, 0x48;x,0x48E0               ; up
        jne     .down
        cmp     si, modes_table
        jbe     .loops
        sub     word [cursor_pos], size_of_step
        jmp     .loops

.down:
        cmp     ah, 0x50;x,0x50E0               ; down
        jne     .pgup
        cmp     word[es:si+10], -1
        je      .loops
        add     word [cursor_pos], size_of_step
        jmp     .loops

.pgup:
        cmp     ah, 0x49                ; page up
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

.pgdn:
        cmp     ah, 0x51                ; page down
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

.enter:
        cmp     al, 0x0D;x,0x1C0D               ; enter
        jne     .loops
        push    word [cursor_pos]
        pop     bp
        push    word [es:bp]
        pop     word [x_save]
        push    word [es:bp+2]
        pop     word [y_save]
        push    word [es:bp+6]
        pop     word [number_vm]
        mov     word [preboot_graph], bp          ;save choose
        
        jmp     .d

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
;.change_c:
;        _setcursor 15,0
;        mov     si, vrrmprint
;        call    print
;        mov     bx, '12'
;        call    getkey
;        mov     [preboot_vrrm], al
;        _setcursor 12,0
;        jmp     .d
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.say_on_off:
        pushf
        call    print
        mov     si, on_msg
        popf
        jz      @f
        mov     si, off_msg
@@:
        jmp     printplain
; novesa and vervesa strings are not used at the moment of executing this code
virtual at novesa
.oldtimer dd ?
.starttime dd ?
if ~ defined extended_primary_loader
.bSettingsChanged db ?
end if
.timer dd ?
end virtual
if ~ defined extended_primary_loader
.loader_block dd -1
end if
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
if defined extended_primary_loader
        sub     ax, [preboot_timeout]
else
        sub     ax, 18*5
end if
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
@@:
        mov     [time_str+9], cl
else if lang eq et
        cmp     al, 1
        ja      @f
        mov     [time_str+9], ' '
        mov     [time_str+10], ' '
@@:
else
; wait 5/4/3/2 seconds, 1 second
        cmp     al, 1
        mov     cl, 's'
        ja      @f
        mov     cl, ' '
@@:
        mov     [time_str+9], cl
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
if ~ defined extended_primary_loader
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
        retf                          ;call back
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
end if
; \end{diamond}[02.12.2005]

; ASK GRAPHICS MODE

        call    set_vmode

; GRAPHICS ACCELERATION
; force yes
        mov     [es:0x901C], byte 1

; DMA ACCESS TO HD

        mov     al, [preboot_dma]
        mov     [es:0x901F], al

;; VRR_M USE
;
;        mov     al,[preboot_vrrm]
;        mov     [es:0x9030], al
        mov     [es:0x901E], byte 1

; BOOT DEVICE

        mov     al, [preboot_device]
        dec     al
        mov     [boot_dev], al

; GET MEMORY MAP
include '../detect/biosmem.inc'

; READ DISKETTE TO MEMORY

        cmp     [boot_dev], 0
        jne     no_sys_on_floppy
        mov     si, diskload
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

; TODO: !!!! read only first sector and set variables !!!!!
; ...
; TODO: !!! then read flippy image track by track
        
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

; read only first sector and fill variables
        mov     cx, 0x0001      ; first logical sector
        xor     dx, dx          ; head = 0, drive = 0 (a:)
        mov     al, 1           ; read one sector
        mov     bx, 0xB000      ; es:bx -> data area
        call    boot_read_floppy
; fill the necessary parameters to work with a floppy
        mov     ax, word [es:bx+24]
        mov     word [BPB_SecPerTrk], ax
        mov     ax, word [es:bx+26]
        mov     word [BPB_NumHeads], ax
        mov     ax, word [es:bx+17]
        mov     word [BPB_RootEntCnt], ax
        mov     ax, word [es:bx+14]
        mov     word [BPB_RsvdSecCnt], ax
        mov     ax, word [es:bx+19]
        mov     word [BPB_TotSec16], ax
        mov     al, byte [es:bx+13]
        mov     byte [BPB_SecPerClus], al
        mov     al, byte [es:bx+16]
        mov     byte [BPB_NumFATs], al
;<Lrz> 18.11.2008
        mov     ax, word [es:bx+22]
        mov     word [BPB_FATSz16], ax
        mov     cx, word [es:bx+11]
        mov     word [BPB_BytsPerSec], cx

; count of clusters in FAT12 ((size_of_FAT*2)/3)
;        mov     ax, word [BPB_FATSz16]
;        mov     cx, word [BPB_BytsPerSec]
;end <Lrz> 18.11.2008
        xor     dx, dx
        mul     cx
        shl     ax, 1
        mov     cx, 3
        div     cx              ; now ax - number of clusters in FAT12
        mov     word [end_of_FAT], ax

; load first FAT table
        mov     cx, 0x0002      ; startcyl,startsector          ; TODO!!!!!
        xor     dx, dx          ; starthead,drive
        mov     al, byte [BPB_FATSz16]     ; no of sectors to read
        add     bx, word [BPB_BytsPerSec]  ; es:bx -> data area
        call    boot_read_floppy
        mov     bx, 0xB000

; and copy them to extended memory
        mov     si, movedesc
        mov     [si+8*2+3], bh          ; from
        
        mov     ax, word [BPB_BytsPerSec]
        shr     ax, 1                   ; words per sector
        mov     cx, word [BPB_RsvdSecCnt]
        add     cx, word [BPB_FATSz16]
        mul     cx
        push    ax                      ; save to stack count of words in boot+FAT
        xchg    ax, cx
        
        push    es
        push    ds
        pop     es
        mov     ah, 0x87
        int     0x15
        pop     es
        test    ah, ah
        jz      @f
sayerr_floppy:
        mov     dx, 0x3f2
        mov     al, 0
        out     dx, al
sayerr_memmove:
        mov     si, memmovefailed
        jmp     sayerr_plain
@@:
        pop     ax                      ; restore from stack count of words in boot+FAT
        shl     ax, 1                   ; make bytes count from count of words
        and     eax, 0ffffh
        add     dword [si+8*3+2], eax

; copy first FAT to second copy
; TODO: BPB_NumFATs !!!!!
        add     bx, word [BPB_BytsPerSec]       ; !!! TODO: may be need multiply by BPB_RsvdSecCnt !!!
        mov     byte [si+8*2+3], bh     ; bx - begin of FAT
        
        mov     ax, word [BPB_BytsPerSec]
        shr     ax, 1                   ; words per sector
        mov     cx, word [BPB_FATSz16]
        mul     cx
        mov     cx, ax                  ; cx - count of words in FAT

        push    es
        push    ds
        pop     es
        mov     ah, 0x87
        int     0x15
        pop     es
        test    ah, ah
        jnz     sayerr_floppy
        
        mov     ax, cx
        shl     ax, 1
        and     eax, 0ffffh             ; ax - count of bytes in FAT
        add     dword [si+8*3+2], eax
        
; reading RootDir
; TODO: BPB_NumFATs
        add     bx, ax
        add     bx, 100h
        and     bx, 0ff00h                      ; bx - place in buffer to write RootDir
        push    bx

        mov     bx, word [BPB_BytsPerSec]
        shr     bx, 5                           ; divide bx by 32
        mov     ax, word [BPB_RootEntCnt]
        xor     dx, dx
        div     bx
        push    ax                              ; ax - count of RootDir sectors

        mov     ax, word [BPB_FATSz16]
        xor     cx, cx
        mov     cl, byte [BPB_NumFATs]
        mul     cx
        add     ax, word [BPB_RsvdSecCnt]       ; ax - first sector of RootDir

        mov     word [FirstDataSector], ax
        pop     bx
        push    bx
        add     word [FirstDataSector], bx      ; Begin of data region of floppy
        
; read RootDir
        call    conv_abs_to_THS
        pop     ax
        pop     bx                              ; place in buffer to write
        push    ax
        call    boot_read_floppy                ; read RootDir into buffer
; copy RootDir
        mov     byte [si+8*2+3], bh             ; from buffer
        pop     ax                              ; ax = count of RootDir sectors
        mov     cx, word [BPB_BytsPerSec]
        mul     cx
        shr     ax, 1
        mov     cx, ax                          ; count of words to copy
        push    es
        push    ds
        pop     es
        mov     ah, 0x87
        int     0x15
        pop     es

        mov     ax, cx
        shl     ax, 1
        and     eax, 0ffffh             ; ax - count of bytes in RootDir
        add     dword [si+8*3+2], eax   ; add count of bytes copied

; Reading data clusters from floppy
        mov     byte [si+8*2+3], bh
        push    bx

        mov     di, 2                   ; First data cluster
.read_loop:
        mov     bx, di
        shr     bx, 1                   ; bx+di = di*1.5
        jnc     .even
        test    word [es:bx+di+0xB200], 0xFFF0  ; TODO: may not be 0xB200 !!!
        jmp     @f
.even:
        test    word [es:bx+di+0xB200], 0xFFF   ; TODO: may not be 0xB200 !!!

@@:
        jz      .skip
; read cluster di
;.read:
        ;conv cluster di to abs. sector ax
        ; ax = (N-2) * BPB_SecPerClus + FirstDataSector
        mov     ax, di
        sub     ax, 2
        xor     bx, bx
        mov     bl, byte [BPB_SecPerClus]
        mul     bx
        add     ax, word [FirstDataSector]
        call    conv_abs_to_THS
        pop     bx
        push    bx
        mov     al, byte [BPB_SecPerClus]       ; number of sectors in cluster
        call    boot_read_floppy
        push    es
        push    ds
        pop     es
        pusha
;
        mov     ax, word [BPB_BytsPerSec]
        xor     cx, cx
        mov     cl, byte [BPB_SecPerClus]
        mul     cx
        shr     ax, 1                           ; ax = (BPB_BytsPerSec * BPB_SecPerClus)/2
        mov     cx, ax                          ; number of words to copy (count words in cluster)
;
        mov     ah, 0x87
        int     0x15                            ; copy data
        test    ah, ah
        popa
        pop     es
        jnz     sayerr_floppy
; skip cluster di
.skip:
        mov     ax, word [BPB_BytsPerSec]
        xor     cx, cx
        mov     cl, byte [BPB_SecPerClus]
        mul     cx
        and     eax, 0ffffh             ; ax - count of bytes in cluster
        add     dword [si+8*3+2], eax

        mov     ax, word [end_of_FAT]   ; max cluster number
        pusha
; draw percentage
; total clusters: ax
; read clusters: di
        xchg    ax, di
        mov     cx, 100
        mul     cx
        div     di
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
        cmp     di, word [end_of_FAT]   ; max number of cluster
        jnz     .read_loop
        pop     bx                      ; clear stack

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

if defined extended_primary_loader
        cmp     [boot_dev], 1
        jne     no_sys_from_primary
; load kolibri.img using callback from primary loader
        and     word [movedesc + 24 + 2], 0
        mov     byte [movedesc + 24 + 4], 10h
; read in blocks of 64K until file is fully loaded
        mov     ax, 1
.repeat:
        mov     di, image_file_struct
        call    [bootcallback]
        push    cs
        pop     ds
        push    cs
        pop     es
        cmp     bx, 1
        ja      sayerr_badsect
        push    bx
        mov     si, movedesc
        and     word [si + 16 + 2], 0
        mov     byte [si + 16 + 4], 4
        mov     ah, 87h
        mov     cx, 8000h
        int     15h
        pop     bx
        test    ah, ah
        jnz     sayerr_memmove
        inc     byte [si + 24 + 4]
        test    bx, bx
        jz      no_sys_from_primary
        mov     ax, 2
        jmp     .repeat
no_sys_from_primary:
end if

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
