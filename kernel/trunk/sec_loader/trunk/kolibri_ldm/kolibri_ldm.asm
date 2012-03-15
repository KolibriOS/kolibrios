;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                     ;;
;; Last modify Alexey Teplov <Lrz> 2008. All rights reserved.          ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved.        ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa             ;;
;; Distributed under terms of the GNU General Public License           ;;
;;                                                                     ;;
;;  kolibri_ldm.asm the module for Secondary Loader                    ;;
;;                                                                     ;;
;;  KolibriOS 16-bit loader module,                                    ;;
;;                        based on bootcode for KolibriOS              ;;
;;                                                                     ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "lang.inc"

macro _setcursor row,column
{
        mov     dx, row*256 + column
        call    setcursor
}
long_v_table equ 9   ;long of visible video table
size_of_step equ 10
d80x25_bottom_num equ 3
d80x25_top_num equ 4
;It's a module for Secondary Loader to load kolibri OS
;
start_of_code:
        cld
; \begin{diamond}[02.12.2005]
; if bootloader sets ax = 'KL', then ds:si points to loader block
;        cmp     ax, 'KL'
;        jnz     @f
;        mov     word [cs:cfgmanager.loader_block], si
;        mov     word [cs:cfgmanager.loader_block+2], ds
;@@:
; \end{diamond}[02.12.2005]

; if bootloader sets cx = 'HA' and dx = 'RD', then bx contains identifier of source hard disk
; (see comment to bx_from_load)
;        cmp     cx, 'HA'
;        jnz     no_hd_load
;        cmp     dx,'RD'
;        jnz     no_hd_load
;        mov     word [cs:bx_from_load], bx              ; {SPraid}[13.03.2007]
;no_hd_load:

; set up stack
        push    cs
        pop     ss
        xor     ax, ax
        mov     sp, ax
;        mov     ax, 3000h
;        mov     ss, ax
;        mov     sp, 0EC00h
; set up segment registers
        push    cs
        pop     ds
        push    cs
        pop     es

; set videomode
        mov     ax, 3
        int     0x10

;if lang eq ru
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
;else if lang eq et
;        mov     bp, ET_FNT              ; ET_FNT1
;        mov     bx, 1000h               ;
;        mov     cx, 255                 ; 256 symbols
;        xor     dx, dx                  ; 0 - position of first symbol
;        mov     ax, 1100h
;        int     10h
;end if

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
        and     word [es:BOOT_IDE_BASE_ADDR], 0
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
        mov     [es:BOOT_IDE_BASE_ADDR], cx
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
        and     word [es:BOOT_APM_VERSION], 0     ; ver = 0.0 (APM not found)
        mov     ax, 0x5300
        xor     bx, bx
        int     0x15
        jc      apm_end                 ; APM not found
        test    cx, 2
        jz      apm_end                 ; APM 32-bit protected-mode interface not supported
        mov     [es:BOOT_APM_VERSION], ax         ; Save APM Version
        mov     [es:BOOT_APM_FLAGS], cx         ; Save APM flags

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

        mov     [es:BOOT_APM_ENTRY], ebx
        mov     [es:BOOT_APM_CODE_32], ax
        mov     [es:BOOT_APM_CODE_16], cx
        mov     [es:BOOT_APM_DATA_16], dx

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
        call    check_first_parm   ;check and enable cursor_pos


; \begin{diamond}[30.11.2005]
cfgmanager:
; settings:
; a) preboot_graph = graphical mode
;    preboot_gprobe = probe this mode?
; b) preboot_dma  = use DMA access?
; c) preboot_vrrm = use VRR?

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
;        mov     si, preboot_device_msg
;        call    print
;        mov     al, [preboot_device]
;        and     eax, 7
;        mov     si, [preboot_device_msgs+eax*2]
;        call    printplain
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
        cmp     al, 'c'
        jnz     .show_remarks

        _setcursor 15,0
        mov     si, vrrmprint
        call    print
        mov     bx, '12'
        call    getkey
        mov     [preboot_vrrm], al
        _setcursor 12,0
.d:
        mov     [.bSettingsChanged], 1
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
        mov     [es:BOOT_MTRR], byte 1

; DMA ACCESS TO HD

        mov     al, [preboot_dma]
        mov     [es:BOOT_DMA], al

; VRR_M USE

;        mov     al, [preboot_vrrm]
;        mov     [es:0x9030], al

; BOOT DEVICE

        mov     al, [preboot_device]
        dec     al
        mov     [boot_dev], al






;;;;;;;;;;; set videomode
        xor     ax, ax
        mov     es, ax

        mov     ax, [es:BOOT_VESA_MODE]         ; vga & 320x200
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

        jmp     $

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
include "lang.inc"
include "bootstr.inc"     ; language-independent boot messages
;if lang eq en
;include "booteng.inc"     ; english system boot messages
;else if lang eq ru
include "bootru.inc"      ; russian system boot messages
include "ru.inc"               ; Russian font
;else if lang eq et
;include "bootet.inc"      ; estonian system boot messages
;include "et.inc"              ; Estonian font
;else
;include "bootge.inc"      ; german system boot messages
;end if

include 'macros.inc'
include 'bootvesa.inc'

include "preboot.inc"


setcursor:
; in: dl=column, dh=row
        mov     ah, 2
        mov     bh, 0
        int     10h
        ret

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
