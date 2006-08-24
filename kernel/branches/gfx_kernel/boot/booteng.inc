;======================================================================
;
;                           BOOT DATA
;
;======================================================================

macro line_full_top {
          db       201
          times    78   db 205
          db       187
}
macro line_full_bottom {
          db       200
          times    78   db 205
          db       188
}
macro line_half {
          db      186,' '
          times    76   db 0xc4
          db      ' ',186
}
macro line_space {
          db      186
          times   78 db 32
          db      186
}

d80x25_top:
                line_full_top
verstr2:
;               line_space
; version string
        db 186,32
        repeat 78
                load a byte from version+%-1
                if a = 13
                        break
                end if
                db a
        end repeat
        repeat 78 - ($-verstr2)
                db      ' '
        end repeat
        db 32,186
verstr:
                line_half
space_msg:      line_space
d80x25_top_num = 3
d80x25_bottom:
          db       186,' KolibriOS based on MenuetOS and comes with ABSOLUTELY '
          db       'NO WARRANTY            ',186
          db       186,' See file COPYING for details                          '
          db       '                       ',186
          line_full_bottom
d80x25_bottom_num = 3

msg_apm	  db      " APM x.x ", 0

novesa    db      "Display: EGA/CGA",13,10,0
s_vesa    db      "VESA version: "
  .ver    db      "?.? ("
  .mem    db      "??? Mbytes)",13,10,0
gr_mode   db      "Select mode: ",13,10,0
s_bpp     db      13,10,186," Bits per pixel: "
  .bpp    dw      "??"
          db      13,10,0
vrrmprint db      "Apply VRR? (picture frequency greater than 60Hz"
          db      " only for transfers:",13,10
          db      186," 1024*768->800*600 and 800*600->640*480) [1-yes,2-no]:",0
gr_acc    db      "Vesa 2.0+ : MTRR graphics acceleration [1-yes/2-no] ? ",0
bdev      db      "Load ramdisk from [1-floppy; 2-C:\menuet.img (FAT32);"
          db      "                   3-use preloaded ram-image from kernel restart]: ",0
not386    db      "Fatal - CPU 386+ required.",0
fatalsel  db      13,10,"Error - Selected mode is not supported.",0
badsect   db      13,10,186," Fatal - Bad sector. Replace floppy.",0
memmovefailed db  13,10,186," Fatal - Int 0x15 move failed.",0
okt       db      " ... OK"
linef     db      13,10,0
diskload  db      "Loading diskette: 00 %",8,8,8,8,0
pros      db      "00"
backspace2 db     8,8,0
boot_dev  db      0  ; 0=floppy, 1=hd
start_msg db      "Press [abcd] to change settings, press [Enter] to continue booting",13,10,0
time_msg  db      " or wait "
time_str  db      " 5 seconds"
          db      " before automatical continuation",13,10,0
current_cfg_msg db "Current settings:",13,10,0
curvideo_msg db   " [a] Videomode: ",0
modevesa20 db     " with LFB",0
modevesa12 db     ", VESA 1.2 Bnk",0
mode9     db      "320x200, EGA/CGA 256 colors",0
mode10    db      "640x480, VGA 16 colors",0
mtrr_msg  db      " [b] Use MTRR for graphics acceleration:",0
on_msg    db      " on",13,10,0
off_msg   db      " off",13,10,0
vrrm_msg  db      " [c] Use VRR:",0
preboot_device_msg db " [d] Floppy image: ",0
preboot_device_msgs dw 0,pdm1-0x10000,pdm2-0x10000,pdm3-0x10000
pdm1      db      "real floppy",13,10,0
pdm2      db      "C:\menuet.img (FAT32)",13,10,0
pdm3      db      "use already loaded image",13,10,0
loading_msg db    "Loading KolibriOS...",0
save_quest db     "Remember current settings? [y/n]: ",0
loader_block_error db "Bootloader data invalid, I cannot continue. Stopped.",0

_oem db 'oem: ',0

db 5
s_ven_intel  db 'Intel'
db 2
s_ven_s3     db 'S3'
;db 5
;s_ven_bochs  db 'Bochs'
;db 8
;s_ven_vmware db 'V M ware'

s_mode    db      "  ????-????-?? (?)  ",0
s_mode1   db      "  0640-0480-04 (a)    0320-0200-08 (b)  ",13,10,0

;_tl db 'ÚÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄ¿',13,10,\
;       '³ resolution ³   4 ³   8 ³  15 ³  16 ³  24 ³  32 ³',13,10,\
;       'ÃÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄ´',13,10,0
;_rs db '³ ????x????  ³  -  ³  -  ³  -  ³  -  ³  -  ³  -  ³',13,10,0
;_bt db 'ÀÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÙ',13,10,0

_tl db 186,'                         ÚÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄ¿',13,10,\
       186,'                         ³   4 ³   8 ³  15 ³  16 ³  24 ³  32 ³',13,10,\
       186,'             ÚÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÅÄ¿',13,10,0
_rs db 186,'             ³ ????x???? ³  ?  ³  ?  ³  ?  ³  ?  ³  ?  ³  ?  ³Û³',13,10,0
_bt db 186,'             ÀÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÁÄÙ',13,10,0


_sel1 db 0x1A,0
_sel2 db 0x1B,0
