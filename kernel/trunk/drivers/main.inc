;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;   (C) copyright Serge 2006-2007
;   email: infinity_sound@mail.ru


PLAY_SYNC      equ   0x80000000

PCM_ALL        equ   0

PCM_OUT        equ   0x08000000
PCM_RING       equ   0x10000000
PCM_STATIC     equ   0x20000000
PCM_FLOAT      equ   0x40000000     ;reserved
PCM_FILTER     equ   0x80000000     ;reserved

PCM_2_16_48    equ   1
PCM_1_16_48    equ   2

PCM_2_16_44    equ   3
PCM_1_16_44    equ   4

PCM_2_16_32    equ   5
PCM_1_16_32    equ   6

PCM_2_16_24    equ   7
PCM_1_16_24    equ   8

PCM_2_16_22    equ   9
PCM_1_16_22    equ  10

PCM_2_16_16    equ  11
PCM_1_16_16    equ  12

PCM_2_16_12    equ  13
PCM_1_16_12    equ  14

PCM_2_16_11    equ  15
PCM_1_16_11    equ  16

PCM_2_16_8     equ  17
PCM_1_16_8     equ  18

PCM_2_8_48     equ  19
PCM_1_8_48     equ  20

PCM_2_8_44     equ  21
PCM_1_8_44     equ  22

PCM_2_8_32     equ  23
PCM_1_8_32     equ  24

PCM_2_8_24     equ  25
PCM_1_8_24     equ  26

PCM_2_8_22     equ  27
PCM_1_8_22     equ  28

PCM_2_8_16     equ  29
PCM_1_8_16     equ  30

PCM_2_8_12     equ  31
PCM_1_8_12     equ  32

PCM_2_8_11     equ  33
PCM_1_8_11     equ  34

PCM_2_8_8      equ  35
PCM_1_8_8      equ  36

SRV_GETVERSION      equ 0
SND_CREATE_BUFF     equ 1
SND_DESTROY_BUFF    equ 2
SND_SETFORMAT       equ 3
SND_GETFORMAT       equ 4
SND_RESET           equ 5
SND_SETPOS          equ 6
SND_GETPOS          equ 7
SND_SETBUFF         equ 8
SND_OUT             equ 9
SND_PLAY            equ 10
SND_STOP            equ 11
SND_SETVOLUME       equ 12
SND_GETVOLUME       equ 13
SND_SETPAN          equ 14
SND_GETPAN          equ 15
SND_GETBUFFSIZE     equ 16
SND_GETFREESPACE    equ 17

struc STREAM
{
    .magic         dd ?   ;'WAVE'
    .destroy       dd ?   ;internal destructor
    .fd            dd ?   ;next object in list
    .bk            dd ?   ;prev object in list
    .pid           dd ?   ;owner id

    .size          dd ?
    .str_fd        dd ?
    .str_bk        dd ?
    .device        dd ?
    .format        dd ?
    .flags         dd ?

    .out_base      dd ?
    .out_wp        dd ?
    .out_rp        dd ?
    .out_count     dd ?
    .out_top       dd ?

    .r_size        dd ?
    .r_dt          dd ?
    .r_silence     dd ?
    .resample      dd ?
    .l_vol         dd ?
    .r_vol         dd ?
    .l_amp         dw ?
    .r_amp         dw ?
    .pan           dd ?

    .in_base       dd ?
    .in_size       dd ?
    .in_wp         dd ?
    .in_rp         dd ?
    .in_count      dd ?
    .in_free       dd ?
    .in_top        dd ?

    .notify_event  dd ?
    .notify_id     dd ?
}

STREAM_SIZE    equ 34*4
FD_OFFSET      equ 24

virtual at 0
 STREAM STREAM
end virtual

struc WAVE_HEADER
{   .riff_id        dd  ?
    .riff_size      dd  ?
    .riff_format    dd  ?

    .fmt_id         dd  ?
    .fmt_size       dd  ?
    .format_tag     dw  ?
    .channels       dw  ?
    .freq           dd  ?
    .bytes_sec      dd  ?
    .block_align    dw  ?
    .bits_sample    dw  ?

    .data_id        dd  ?
    .data_size      dd  ?
}

