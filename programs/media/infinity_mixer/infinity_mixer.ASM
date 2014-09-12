;
;   KolibriOS system mixer. Version 0.2
;
;   Author:         Asper
;   Date of issue:  15.08.2010
;   Compiler:       FASM
;   Target:         KolibriOS
;

use32
        org     0x0

        db      'MENUET01'      ; 8 byte id
        dd      38              ; required os
        dd      STARTAPP        ; program start
        dd      I_END           ; program image size
        dd      I_MEM           ; required amount of memory
        dd      I_MEM           ; stack heap
        dd      0x0, 0x0

include 'aspapi.INC'
include '../../macros.inc'
;include 'string.inc'


;include 'debug.inc'

DEBUG    equ 0 ;1

SRV_GETVERSION        equ  0
DEV_PLAY              equ  1
DEV_STOP              equ  2
DEV_CALLBACK          equ  3
DEV_SET_BUFF          equ  4
DEV_NOTIFY            equ  5
DEV_SET_MASTERVOL     equ  6
DEV_GET_MASTERVOL     equ  7
DEV_GET_INFO          equ  8


;C_UNKNOWN     equ ?
C_BLACK        equ 0x000000
C_GREY         equ 0xB0B0B0    ; 0xC0C0C0
C_BLUE         equ 0x1070D0
C_GREEN        equ 0x70D010
C_RED          equ 0xD01010
C_ORANGE       equ 0xD07010
C_YELLOW       equ 0xE0E010
C_PURPLE       equ 0x7010D0
C_PINK         equ 0xFF00FF
;C_RESERVED     equ ?
C_WHITE        equ 0xFFFFFF
;C_OTHER        equ ?


C_KEYCOLOR1   equ 0x0050D250 ; Control button color 1
C_KEYCOLOR2   equ 0x00CBE1E1 ; Control button color 2
C_TEXTCOLOR   equ 0x80000000 ; Button caption color
BUT_W         equ 20
BUT_H         equ 20
;WIN_X         equ 30000 ; I hope nobody has screen
;WIN_Y         equ 30000 ; with such a big resolution :)
WIN_W         equ 250;390
WIN_H         equ 180
WIN_COLOR     equ 0x505050 ;0x0099BB
WIN_X_CENTER  equ WIN_W/2

FIRST_CTRL_BUTTON_ID  equ 7
MAX_CONTROLS_NUM equ 15 ;  Bad bounding :/

struc CONTROL
{
  .level dw ?
  .num_steps dw ?
}

; widget node for parsing
;struc HDA_GNODE
;{
;  .nid                dw ?    ;NID of this widget
;  .nconns             dw ?    ;number of input connections
;  .conn_list          dd ?
;  .slist              dw ?    ;temporay list
;                      dw ?
;
;  .wid_caps           dd ?    ;widget capabilities
;  .type               db ?    ;widget type
;  .pin_ctl            db ?    ;pin controls
;  .checked            db ?    ;the flag indicates that the node is already parsed
;  .pin_caps           dd ?    ;pin widget capabilities
;  .def_cfg            dd ?    ;default configuration
;  .amp_out_caps       dd ?    ;AMP out capabilities
;  .amp_in_caps        dd ?    ;AMP in capabilities
;  .next               dd ? ;        struct list_head list
;  .sizeof:
;}

;virtual at 0
;  HDA_GNODE  HDA_GNODE
;end virtual

STARTAPP:
        ; Initialize memory
;        mcall   68, 11
;        or      eax,eax
;        jz      close_app


        ; Load sound system driver
        mov     eax, 68
        mov     ebx, 16
        mov     ecx, drivername
        int     0x40
        mov     [hDriver], eax
        test    eax, eax
        jnz     @f
loaderr:
        if DEBUG
                print   "Can't load driver"
        end if
        jmp     close_app
  @@:
        mov     [is_hda], 1
        call    GetDriverVer
        mov     eax, [driver_ver]
        cmp     ax, 0x11DA
        je      @f
        mov     [is_hda], 0
  @@:
        cmp     [is_hda], 0
        je      .init_ac97_controls
        ; Init volume controls
        stdcall GetMasterVol, 0
        mov     ebx, eax
        mov     [LVolume.level], ax
        shr     eax, 16
        mov     [LVolume.num_steps], ax
        stdcall GetMasterVol, 1
        mov     [RVolume.level], ax
        shr     eax, 16
        mov     [RVolume.num_steps], ax
        jmp     .controls_init_complete
        ; Asper: temporary workaround on ac97 codecs [
  .init_ac97_controls:        ;jmp     @f
        mov     [is_hda], 0
        mov     [LVolume.num_steps], 31
        mov     [RVolume.num_steps], 31
        mov     ax, [LVolume.level]
        call    convert_ac97_volume_to_level
        mov     [LVolume.level], ax
        mov     [RVolume.level], ax
        ; Asper: temporary workaround on ac97 codecs ]
  .controls_init_complete:
        if DEBUG
                print   "Left level"
                movzx   eax, [LVolume.level]
                dph     eax
                newline
                print   "Left num_steps"
                movzx   eax, [LVolume.num_steps]
                dph     eax
                newline
                print   "Right level"
                movzx   eax, [RVolume.level]
                dph     eax
                newline
                print   "Right num_steps"
                movzx   eax, [RVolume.num_steps]
                dph     eax
                newline
        end if

        mcall   66, 1, 1  ; Set keyboard mode to get scancodes.

        mcall   37,0
        mov     cx,ax
        shl     ecx,16
        mov     cx,WIN_H
        mov     ebx,eax
        mov     bx,WIN_W

red:
        call    draw_window

still:
        mcall   10               ; Wait for an event in the queue.

        cmp     al,1                  ; redraw request ?
        jz      red
        cmp     al,2                  ; key in buffer ?
        jz      key
        cmp     al,3                  ; button in buffer ?
        jz      button

        jmp     still

key:
        mcall   2

        cmp     ah, 1 ;Esc
        je      close_app

;        cmp     ah, 5  ; '4'
;        jne     @f
;        stdcall GetMasterVol, 0
;        jmp     still
;       @@:
;
;        cmp     ah, 6  ; '5'
;        jne     @f
;        stdcall GetMasterVol, 1
;        jmp     still
;       @@:

;        cmp     ah, 8  ; '7'
;        jne     @f
;        call    GetDriverVer
;        jmp     still
;       @@:

        ;Left channel Up
        cmp     ah, 71  ; 'Home'
        jne     @f
        movzx   eax, [LVolume.level]
        inc     ax
        cmp     ax, [LVolume.num_steps]
        jg      still
        mov     [LVolume.level], ax
        call    convert_level_to_ac97_volume ;Asper: temporary workaround on ac97
        stdcall SetMasterVol, 0, eax
        jmp     red
       @@:

        ;Left channel Down
        cmp     ah, 79  ; 'End'
        jne     @f
        movzx   eax, [LVolume.level]
        dec     ax
        cmp     ax, 0
        jl      still
        mov     [LVolume.level], ax
        call    convert_level_to_ac97_volume ;Asper: temporary workaround on ac97
        stdcall SetMasterVol, 0, eax
        jmp     red
       @@:

        ;Right channel Up
        cmp     ah, 73  ; 'PageUp'
        jne     @f
        movzx   eax, [RVolume.level]
        inc     ax
        cmp     ax, [RVolume.num_steps]
        jg      still
        mov     [RVolume.level], ax
        call    convert_level_to_ac97_volume ;Asper: temporary workaround on ac97
        movzx   edx, [is_hda]
        stdcall SetMasterVol, edx, eax
        jmp     red
       @@:

        ;Right channel Down
        cmp     ah, 81  ; 'PageDown'
        jne     @f
        movzx   eax, [RVolume.level]
        dec     ax
        cmp     ax, 0
        jl      still
        mov     [RVolume.level], ax
        call    convert_level_to_ac97_volume ;Asper: temporary workaround on ac97
    .RVdown:
        movzx   edx, [is_hda]
        stdcall SetMasterVol, edx, eax
        jmp     red
       @@:

        jmp     still

button:
        mcall   17             ; Get pressed button code
        cmp     ah, 1               ; Test x button
        je      close_app
        jmp     still


draw_window:
        ;start_draw_window WIN_X,WIN_Y,WIN_W,WIN_H,WIN_COLOR,caption;, 12;labellen-labelt
        mcall   0,,,34000000h+WIN_COLOR,,caption


        stdcall draw_button,  WIN_X_CENTER-BUT_W-55, 30+BUT_H*0,BUT_W,BUT_H,2,C_PINK,aNoText,0,0
        stdcall draw_button,  WIN_X_CENTER-BUT_W-55, 30+BUT_H*1+10,BUT_W,BUT_H,3,C_GREEN,aNoText,0,0
        stdcall draw_button,  WIN_X_CENTER-BUT_W-55, 30+BUT_H*2+20,BUT_W,BUT_H,4,C_BLUE,aNoText,0,0

        stdcall draw_button,  WIN_X_CENTER-BUT_W/2+55, 30+BUT_H*0,BUT_W,BUT_H,5,C_GREY,aNoText,0,0
        stdcall draw_button,  WIN_X_CENTER-BUT_W/2+55, 30+BUT_H*1+10,BUT_W,BUT_H,6,C_BLACK,aNoText,0,0
        stdcall draw_button,  WIN_X_CENTER-BUT_W/2+55, 30+BUT_H*2+20,BUT_W,BUT_H,7,C_ORANGE,aNoText,0,0

        movzx   eax, [LVolume.level]
        movzx   edx, [LVolume.num_steps]
        stdcall draw_volume_control, WIN_X_CENTER-BUT_W-25, 20, 110, aLVolume, C_BLACK, eax, edx
        movzx   eax, [RVolume.level]
        movzx   edx, [RVolume.num_steps]
        stdcall draw_volume_control, WIN_X_CENTER+25, 20, 110, aRVolume, C_BLACK, eax, edx

        stdcall rectangle, WIN_X_CENTER-BUT_W-56, 29+BUT_H*0, BUT_W+2, BUT_H+2, C_BLACK
        stdcall rectangle, WIN_X_CENTER-BUT_W-56, 29+BUT_H*1+10, BUT_W+2, BUT_H+2, C_BLACK
        stdcall rectangle, WIN_X_CENTER-BUT_W-56, 29+BUT_H*2+20, BUT_W+2, BUT_H+2, C_BLACK

        stdcall rectangle, WIN_X_CENTER-BUT_W/2+54, 29+BUT_H*0, BUT_W+2, BUT_H+2, C_BLACK
        stdcall rectangle, WIN_X_CENTER-BUT_W/2+54, 29+BUT_H*1+10, BUT_W+2, BUT_H+2, C_GREEN
        stdcall rectangle, WIN_X_CENTER-BUT_W/2+54, 29+BUT_H*2+20, BUT_W+2, BUT_H+2, C_BLACK

        stdcall rectangle, WIN_X_CENTER-BUT_W-57, 28+BUT_H*0, BUT_W+4, BUT_H+4, C_PINK
        stdcall rectangle, WIN_X_CENTER-BUT_W-57, 28+BUT_H*1+10, BUT_W+4, BUT_H+4, C_GREEN
        stdcall rectangle, WIN_X_CENTER-BUT_W-57, 28+BUT_H*2+20, BUT_W+4, BUT_H+4, C_BLUE

        stdcall rectangle, WIN_X_CENTER-BUT_W/2+53, 28+BUT_H*0, BUT_W+4, BUT_H+4, C_GREY
        stdcall rectangle, WIN_X_CENTER-BUT_W/2+53, 28+BUT_H*1+10, BUT_W+4, BUT_H+4, C_BLACK;
        stdcall rectangle, WIN_X_CENTER-BUT_W/2+53, 28+BUT_H*2+20, BUT_W+4, BUT_H+4, C_ORANGE


        end_draw_window
ret



proc draw_volume_control stdcall, x:dword, y:dword, ysize:dword, name:dword, textcolor:dword, step:dword, max_steps:dword
        mov     ebx, [x]
        mov     edx, [y]
        mov     eax, [ysize]
        stdcall bar, ebx, edx, 10, eax, C_GREEN
        mov     ecx, [max_steps]
        cmp     ecx, [step]
        je      .l2

        push    eax  edx
        mov     cl,  byte [max_steps]
        test    cl, cl
        jnz     @f
        mov     ecx, [ysize]
        jmp     .l1
  @@:
        div     cl
        mov     cl,  byte [step]
        mul     cl
        mov     ecx, [ysize]
        sub     ecx, eax
  .l1:
        pop     edx  eax
        stdcall bar, ebx, edx, 10, ecx, C_RED
  .l2:
        dec     ebx
        dec     edx
        inc     eax
        stdcall rectangle, ebx, edx, 11, eax, C_BLACK


        add     edx, eax
        add     edx, 10
        sub     ebx, 8
        stdcall outtextxy, ebx, edx, [name], 0, C_GREEN or 0x80000000
        ret
endp

;____________________________________
GetDriverVer:
        push    ebx ecx
        mov     [ioctl_code], SRV_GETVERSION
        and     [inp_size], 0
        mov     [outp_size], 4
        mov     [output], driver_ver
        mcall   68,17,ioctl
        test    eax, eax
        jnz     loaderr
        if DEBUG
                push    eax
                dps     "1.) Driver version = "
                dph     [driver_ver]
                newline
                pop     eax
        end if
        pop     ecx ebx
ret

;____________________________________
Stop:
        and     [ioctl_code], DEV_STOP
        and     [inp_size], 4
        mov     [input], 0 ;? stream
        mov     [outp_size], 0
        mov     [output], 0
        mcall   68,17,ioctl
        test    eax, eax
        jnz     loaderr
        if DEBUG
                print   "Stop device"
        end if
ret

;____________________________________
proc GetMasterVol stdcall, channel:dword
        push    ebx ecx
        mov     [ioctl_code], DEV_GET_MASTERVOL
        mov     [inp_size], 1
        mov     eax, [channel]
        mov     [input], eax
        mov     [outp_size], 4
        mov     [output], master_vol
        mcall   68,17,ioctl
        if DEBUG
                push    eax
                dps     "Get Master volume = "
                dph     [master_vol]
                newline
                pop     eax
        end if
        mov     eax, [master_vol]
        pop     ecx ebx
ret
endp
;____________________________________
proc SetMasterVol stdcall, channel:dword, val:dword
        push    ebx ecx
        mov     [ioctl_code], DEV_SET_MASTERVOL
        mov     [inp_size], 4
        mov     eax, [channel]
        shl     eax, 24
        or      eax, [val]
        mov     [master_vol], eax
        mov     [input], master_vol
        mov     [outp_size], 0
        mcall   68,17,ioctl
        if DEBUG
                push    eax
                dps     "Set Master volume = "
                dph     [val]
                dps     "    ch = "
                dph     [channel]
                newline
                pop     eax
        end if
        pop     ecx ebx
ret
endp

;____________________________________
;GetDevInfo: ; Get pins configurations or controls
;        mov     [ioctl_code], DEV_GET_INFO
;        and     [inp_size], 0
;        mov     [outp_size], 9*4
;        mov     [output], ctrl_info
;        mcall   68,17,ioctl
;        if DEBUG
;                push    eax
;                print   "CTRL_INFO"
;                dps     ".pci_cmd = "
;                dph     [ctrl_info]
;                newline
;                dps     ".irq = "
;                dph     [ctrl_info+4]
;                newline
;                ;dps     ".glob_cntrl = "
;                dps     ".VIA_REG_OFFSET_CONTROL = "
;                dph     [ctrl_info+8]
;                newline
;                ;dps     ".glob_sta = "
;                dps     ".VIA_REG_OFFSET_STATUS = "
;                dph     [ctrl_info+12]
;                newline
;                ;dps     ".codec_io_base = "
;                dps     ".VIA_REG_OFFSET_TABLE_PTR = "
;                dph     [ctrl_info+16]
;                newline
;                dps     ".ctrl_io_base = "
;                dph     [ctrl_info+20]
;                newline
;                ;dps     ".codec_mem_base = "
;                dps     ".VIA_REG_OFFSET_STOP_IDX = "
;                dph     [ctrl_info+24]
;                newline
;                ;dps     ".ctrl_mem_base = "
;                dps     ".VIA_REG_OFFSET_CURR_COUNT = "
;                dph     [ctrl_info+28]
;                newline
;                dps     ".codec_id = "
;                dph     [ctrl_info+32]
;                newline
;                pop     eax
;        end if
;ret


convert_level_to_ac97_volume:
        push   ebx ecx edx
        cmp    [is_hda], 0
        jne    .not_ac97

        mov     [LVolume.level], ax
        mov     [RVolume.level], ax

        mov    bx, ax
        mov    ax, [LVolume.num_steps]
        sub    ax, bx
        mov    cx, 150
        mul    cx
        neg    eax
  .not_ac97:
        pop    edx ecx ebx
ret


convert_ac97_volume_to_level:
        push   ebx ecx edx
        cmp    [is_hda], 0
        jne    .not_ac97

        neg    eax
        mov    cx, 150
        div    cx
        mov    bx, ax
        mov    ax, [LVolume.num_steps]
        sub    ax, bx
  .not_ac97:
        pop    edx ecx ebx
ret


close_app:
        or      eax, -1
        int     0x40
;____________________________________
caption db 'Sound Mixer',0
drivername db 'SOUND',0

;aRamdisk db '/rd/1/',0

;aStartDriver db 'Start driver',0
;aStop        db 'Stop',0
;aGetMVol     db 'Get Master Volume',0
;aSetMVol    db 'Set Master Volume',0
;aGetDevInfo  db 'Get device info',0
aNoText      db 0

aLVolume     db 'Left',0
aRVolume     db 'Right',0

; Jack types
;aLineOut     db 'Line Out',0
;aSpeaker     db 'Speaker',0
;aHPOut       db 'HP Out',0
;aSideSpk     db '',0
;aCD          db 'CD',0
;aSPDIFOut    db 'SPDIF Out',0
;aDigitalOut  db 'Digital Out',0
;aModemLine   db 'Modem Line',0
;aModemHand   db 'Modem Hand',0
;aLineIn      db 'Line In',0
;aAUX         db 'AUX',0
;aMic         db 'Mic',0
;aTelephony   db 'Telephony',0
;aSPDIFIn     db 'SPDIF In',0
;aDigitalIn   db 'Digital In',0
;aReserved    db 'Reserved',0
;aOther       db 'Other',0

;Jack locations
;base
;aNA          db 'N/A',0
;aRear        db 'Rear',0
;aFront       db 'Front',0
;aLeft        db 'Left',0
;aRight       db 'Right',0
;aTop         db 'Top',0
;aBottom      db 'Bottom',0
;special
;aRearPanel   db 'Rear Panel',0
;aDriveBar    db 'Drive Bar',0
;aRiser       db 'Riser',0
;aHDMI        db 'HDMI',0
;aATAPI       db 'ATAPI',0
;aMobileIn    db 'Mobile-In',0
;aMobileOut   db 'Mobile-Out',0


I_END:

align 4
ioctl:
hDriver         rd 1
ioctl_code      rd 1
input           rd 1
inp_size        rd 1
output          rd 1
outp_size       rd 1

driver_ver      rd 1
master_vol      rd 1

;ctrl_info       rd 9

is_hda          rb 1

LVolume CONTROL
RVolume CONTROL

align 4
rb 2048 ; stack
I_MEM:

