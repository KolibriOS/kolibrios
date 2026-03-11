; SPDX-License-Identifier: GPL-2.0-only
;
; Reshare - Shared Resources Daemon
;
; Copyright (C) 2024-2026 KolibriOS Team
; Copyright (C) 2024-2026 KolibriOS-NG Team

; ====================================================================

CBOX_WIDTH     = 13
CBOX_HEIGHT    = 13
CBOX_PIXELS    = CBOX_WIDTH * CBOX_HEIGHT
CBOX_IMG_SIZE  = CBOX_PIXELS * 3       ; 24-bpp BGR, tight rows

; ====================================================================

; Palette entry: 4 bytes BGR0 (padded for dword-indexed lookup)
macro BGRA c
{
        db (c) and 0xFF, (c shr 8) and 0xFF, (c shr 16) and 0xFF, 0
}

P00     = 0x04D4FC      ; 0xRRGGBB
P01     = 0x04C8FC
P02     = 0x04DCFC
P03     = 0x04B4FC
P04     = 0x04BCFC
P05     = 0x04A9FC
P06     = 0x0498F9
P07     = 0xD2F2FC
P08     = 0x04A5FA
P09     = 0xFFFFFF
P10     = 0xF6FDFD
P11     = 0xAFF9FC
P12     = 0x95E0F7
P13     = 0x0464BC
P14     = 0x2CDCFC
P15     = 0x049FFA

cbox_palette:
        BGRA P00
        BGRA P01
        BGRA P02
        BGRA P03
        BGRA P04
        BGRA P05
        BGRA P06
        BGRA P07
        BGRA P08
        BGRA P09
        BGRA P10
        BGRA P11
        BGRA P12
        BGRA P13
        BGRA P14
        BGRA P15

; 4-bit palette indices, one byte per pixel (upper nibble unused — values 0..15)
cbox_indexed:
        db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1     ; row  1
        db  2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3     ; row  2
        db  2, 1, 1, 1, 1, 4, 4, 4, 3, 5, 5, 3, 3     ; row  3
        db  0, 1, 1, 1, 4, 4, 3, 3, 6, 7, 7, 8, 3     ; row  4
        db  0, 3, 5, 5, 3, 3, 3, 6, 9,10,11, 6, 3     ; row  5
        db  0, 5, 7, 7, 6, 3, 6, 9,10,12,13, 6, 5     ; row  6
        db  0, 6,10, 9,14, 6, 9,10,12,13, 6,15, 5     ; row  7
        db  0, 6,12,10, 9, 9,10,12,13, 6,15, 5, 5     ; row  8
        db  0, 6,13,12,10,10,12,13, 6,15, 5, 5, 5     ; row  9
        db  1, 8, 6,13,12,12,13, 6,15, 5, 5, 5,15     ; row 10
        db  1, 5,15, 6,13,13, 6,15, 5, 5, 5,15,15     ; row 11
        db  1, 5, 5,15, 6, 6,15, 5, 5, 5,15,15, 6     ; row 12
        db  3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6     ; row 13
