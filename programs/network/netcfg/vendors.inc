macro VendorID a,b {
        dw a
        dd _vtmp#a
        \iglobal
                _vtmp#a db b
                db      0
        \};endg
}

macro ClassID a,b,c,d {
        db a
        db b
        local tt

        if c eq
                db 0
                db 1
                tt = 'x'
        else
                db c
                db 0
                tt = c
        end if
        
        dd _ctmp#a#b#tt
        \iglobal
                _ctmp#a#b#tt db d
                db      0
        \};endg
}

;--------------------------------------
VendorsTab:
VendorID        0x018a, 'LevelOne'                     ; Actually they misprogrammed their boards
VendorID        0x02ac, 'Efficient Networks, Inc.'
VendorID        0x021b, 'Compaq'
VendorID        0x1011, 'Digital Equipment Corp.'
VendorID        0x1022, 'Advanced Micro Devices'
VendorID        0x1039, 'Silicon Integrated Systems'
VendorID        0x10B7, '3com Corp.'
VendorID        0x10DE, 'Nvidia Corp.'
VendorID        0x10EC, 'Realtek Semiconductor Corp.'
VendorID        0x1106, 'VIA Technologies'
VendorID        0x1113, 'Accton Technology'
VendorID        0x1186, 'D-link system Inc.'
VendorID        0x11DB, 'Sega Enterprises Ltd'
VendorID        0x1259, 'Allied Telesyn International'
VendorID        0x126C, 'Nortel Networks Corp.'
VendorID        0x1317, 'Admtek Inc.'
VendorID        0x13D1, 'Abocom Systems'
VendorID        0x1432, 'Edimax Computer Co'
VendorID        0x14EA, 'Planex Communications Inc.'
VendorID        0x1500, 'Lantech Computer Company'
VendorID        0x1516, 'Myson Technology Inc.'
VendorID        0x166D, 'Broadcom Corp.'
VendorID        0x168C, 'Atheros Communications Inc.'
VendorID        0x16EC, 'U.S. Robotics'
VendorID        0x1743, 'Peppercon AG'
VendorID        0x17F3, 'RDC Semiconductor'
VendorID        0x4033, 'Addtron Technology Co, Inc.'
VendorID        0x8086, 'Intel Corp.'
VendorID        0,      'Unknown'       ; <- terminator

;--------------------------------------
Classes:
; Class, Subclass, [Interface], Name
ClassID    2,    0,     , 'Ethernet'
ClassID    2,    1,     , 'Token Ring'
ClassID    2,    2,     , 'FDDI'
ClassID    2,    3,     , 'ATM'
ClassID    2,    4,     , 'ISDN'
ClassID    2,    5,     , 'WorldFip'
ClassID    2,    6,     , 'PICMG 2.14'
ClassID    2, 0x80,     , 'misc'

ClassID 0x0d, 0x20,     , 'Wlan (802.11a)'
ClassID 0x0d, 0x21,     , 'Wlan (802.11b)'


ClassID 0xff, 0xff,     , 'Unknown'     ; <- terminator