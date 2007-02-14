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
space_msg:	line_space
verstr:
;		line_space
; version string
	db 186,32
	repeat 78
		load a byte from version+%-1
		if a = 13
			break
		end if
		db a
	end repeat
	repeat 78 - ($-verstr)
		db	' '
	end repeat
	db 32,186
		line_half
d80x25_top_num = 4
d80x25_bottom:
          db       186,' KolibriOS based on MenuetOS and comes with ABSOLUTELY '
          db       'NO WARRANTY            ',186
          db       186,' See file COPYING for details                          '
          db       '                       ',186
          line_full_bottom
d80x25_bottom_num = 3

novesa    db      "Display: EGA/CGA",13,10,0
vervesa   db      "Version of Vesa: Vesa x.x",13,10,0
vervesa_off=22
msg_apm	  db      " APM x.x ", 0
gr_mode   db      186," Vesa 2.0+ 16 M LFB:  [1] 640x480, [2] 800x600, "
          db      "[3] 1024x768, [4] 1280x1024",13,10
          db      186," Vesa 1.2  16 M Bnk:  [5] 640x480, [6] 800x600, "
          db      "[7] 1024x768, [8] 1280x1024",13,10
          db      186," EGA/CGA   256 Colors:  [9] 320x200, "
          db      "VGA 16 Colors: [0]  640x480",13,10
          db      186," Select mode: ",0
bt24      db      "Bits Per Pixel: 24",13,10,0
bt32      db      "Bits Per Pixel: 32",13,10,0
vrrmprint db      "Apply VRR? (picture frequency greater than 60Hz"
          db      " only for transfers:",13,10
          db      186," 1024*768->800*600 and 800*600->640*480) [1-yes,2-no]:",0
;askmouse  db      " Mouse at:"
;          db      " [1] PS/2 (USB), [2] Com1, [3] Com2."
;          db      " Select port [1-3]: ",0
;no_com1   db      13,10,186, " No COM1 mouse",0
;no_com2   db      13,10,186, " No COM2 mouse",0         
ask_dma   db      "Use DMA for HDD writing? [1-yes/2-no]: ",0
;gr_direct db      186," Use direct LFB writing? "
;          db      "[1-yes/2-no] ? ",0
;mem_model db      13,10,186," Motherboard memory [1-16 Mb / 2-32 Mb / "
;          db      "3-64Mb / 4-128 Mb / 5-256 Mb] ? ",0
;bootlog   db      13,10,186," After bootlog display [1-continue/2-pause] ? ",0
bdev      db      "Load ramdisk from [1-floppy; 2-C:\kolibri.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-use preloaded ram-image from kernel restart]: ",0
probetext db      13,10,13,10,186," Use standart graphics mode? [1-yes, "
          db      "2-probe bios (Vesa 3.0)]: ",0
;memokz256 db      13,10,186," RAM 256 Mb",0
;memokz128 db      13,10,186," RAM 128 Mb",0
;memokz64  db      13,10,186," RAM 64 Mb",0
;memokz32  db      13,10,186," RAM 32 Mb",0
;memokz16  db      13,10,186," RAM 16 Mb",0
prnotfnd  db      "Fatal - Videomode not found.",0
;modena    db      "Fatal - VBE 0x112+ required.",0
not386    db      "Fatal - CPU 386+ required.",0
btns      db      "Fatal - Can't determine color depth.",0
fatalsel  db      "Fatal - Graphics mode not supported by hardware.",0
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
mode1     db      "640x480",0
mode2     db      "800x600",0
mode3     db      "1024x768",0
mode4     db      "1280x1024",0
modes_msg dw mode4-0x10000,mode1-0x10000,mode2-0x10000,mode3-0x10000
modevesa20 db     " with LFB",0
modevesa12 db     ", VESA 1.2 Bnk",0
mode9     db      "320x200, EGA/CGA 256 colors",0
mode10    db      "640x480, VGA 16 colors",0
probeno_msg db    " (standard mode)",0
probeok_msg db    " (check nonstandard modes)",0
dma_msg   db      " [b] Use DMA for HDD writing:",0
on_msg    db      " on",13,10,0
off_msg   db      " off",13,10,0
vrrm_msg  db      " [c] Use VRR:",0
preboot_device_msg db " [d] Floppy image: ",0
preboot_device_msgs dw 0,pdm1-0x10000,pdm2-0x10000,pdm3-0x10000
pdm1      db      "real floppy",13,10,0
pdm2      db      "C:\kolibri.img (FAT32)",13,10,0
pdm3      db      "use already loaded image",13,10,0
loading_msg db    "Loading KolibriOS...",0
save_quest db     "Remember current settings? [y/n]: ",0
loader_block_error db "Bootloader data invalid, I cannot continue. Stopped.",0
