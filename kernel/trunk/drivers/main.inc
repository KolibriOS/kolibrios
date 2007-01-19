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
{   .magic         dd 0
    .size          dd 0
    .device        dd 0
    .format        dd 0
    .flags         dd 0

    .work_buff     dd 0
    .work_read     dd 0
    .work_write    dd 0
    .work_count    dd 0
    .work_top      dd 0
    .r_buff        dd 0
    .r_size        dd 0
    .r_end         dd 0
    .r_dt          dd 0
    .r_silence     dd 0

    .base          dd 0
    .limit         dd 0
    .seg_0         dd 0
    .lim_0         dd 0
    .seg_1         dd 0
    .lim_1         dd 0
    .curr_seg      dd 0

    .buff_size     dd 0
    .notify_off1   dd 0
    .notify_off2   dd 0
    .notify_task   dd 0
    .resample      dd 0
}

STREAM_SIZE    equ 27*4

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

