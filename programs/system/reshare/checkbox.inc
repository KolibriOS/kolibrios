; SPDX-License-Identifier: GPL-2.0-only
;
; Reshare - Shared Resources Daemon
; Copyright (C) 2024 KolibriOS-NG Team
;
; Contributor rgmaid - Main code
; Contributor Burer  - Image conversion from 8bpp

; ====================================================================

CBOX_WIDTH     = 13
CBOX_HEIGHT    = 13
CBOX_PIXELS    = CBOX_WIDTH * CBOX_HEIGHT
CBOX_IMG_SIZE  = CBOX_PIXELS * 3 ; 24-bpp RGB, tight rows

; ====================================================================

macro RGB24 c
{
        db (c) and 0xFF, (c shr 8) and 0xFF, (c shr 16) and 0xFF
}

macro ROW13 c00, c01, c02, c03, c04, c05, c06, c07, c08, c09, c10, c11, c12
{
        RGB24 c00
        RGB24 c01
        RGB24 c02
        RGB24 c03
        RGB24 c04
        RGB24 c05
        RGB24 c06
        RGB24 c07
        RGB24 c08
        RGB24 c09
        RGB24 c10
        RGB24 c11
        RGB24 c12
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

cbox_image:
        ROW13   P00, P00, P00, P00, P00, P00, P00, P00, P00, P00, P00, P00, P01
        ROW13   P02, P01, P01, P01, P01, P01, P01, P01, P01, P01, P01, P01, P03
        ROW13   P02, P01, P01, P01, P01, P04, P04, P04, P03, P05, P05, P03, P03
        ROW13   P00, P01, P01, P01, P04, P04, P03, P03, P06, P07, P07, P08, P03
        ROW13   P00, P03, P05, P05, P03, P03, P03, P06, P09, P10, P11, P06, P03
        ROW13   P00, P05, P07, P07, P06, P03, P06, P09, P10, P12, P13, P06, P05
        ROW13   P00, P06, P10, P09, P14, P06, P09, P10, P12, P13, P06, P15, P05
        ROW13   P00, P06, P12, P10, P09, P09, P10, P12, P13, P06, P15, P05, P05
        ROW13   P00, P06, P13, P12, P10, P10, P12, P13, P06, P15, P05, P05, P05
        ROW13   P01, P08, P06, P13, P12, P12, P13, P06, P15, P05, P05, P05, P15
        ROW13   P01, P05, P15, P06, P13, P13, P06, P15, P05, P05, P05, P15, P15
        ROW13   P01, P05, P05, P15, P06, P06, P15, P05, P05, P05, P15, P15, P06
        ROW13   P03, P06, P06, P06, P06, P06, P06, P06, P06, P06, P06, P06, P06
align 4
