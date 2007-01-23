;
;   This file is part of the Infinity sound AC97 driver.
;   (C) copyright Serge 2006
;   email: infinity_sound@mail.ru
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.

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

PCM_2_8_48     equ  17
PCM_1_8_48     equ  18

PCM_2_8_44     equ  19
PCM_1_8_44     equ  20

PCM_2_8_32     equ  21
PCM_1_8_32     equ  22

PCM_2_8_24     equ  23
PCM_1_8_24     equ  24

PCM_2_8_22     equ  25
PCM_1_8_22     equ  26

PCM_2_8_16     equ  27
PCM_1_8_16     equ  28

PCM_2_8_12     equ  29
PCM_1_8_12     equ  30

PCM_2_8_11     equ  31
PCM_1_8_11     equ  32

SND_PLAY       equ   1
SND_STOP       equ   2

;  struc SND_DEV
;{ .magic          dd 0
;  .size           dd 0
;  .count          dd 0
;                  dd 0
;  .snd_buff       dd 16 dup (0)
;}

;virtual at 0
; SND_DEV SND_DEV
;end virtual

;SND_DEV_SIZE     equ 80


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

    .work_buff     dd ?
    .work_read     dd ?
    .work_write    dd ?
    .work_count    dd ?
    .work_top      dd ?
    .r_size        dd ?
    .r_end         dd ?
    .r_dt          dd ?
    .r_silence     dd ?

    .base          dd ?
    .limit         dd ?
    .seg_0         dd ?
    .lim_0         dd ?
    .seg_1         dd ?
    .lim_1         dd ?
    .curr_seg      dd ?

    .buff_size     dd ?
    .notify_off1   dd ?
    .notify_off2   dd ?
;    .notify_task   dd ?
    .resample      dd ?
}

STREAM_SIZE    equ 31*4
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

