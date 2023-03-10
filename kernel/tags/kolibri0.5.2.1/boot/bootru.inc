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
d80x25:   line_full_top
          line_space
          line_space
          line_half
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
          line_space
;          line_space
          db       186,' Kolibri OS ?᭮???? ?? Menuet OS ? ?? ?।??⠢???? '
          db       '??????? ???a?⨩.        ',186
          db       186,' ???஡??? ᬮ????? 䠩? GNU.TXT                     '
          db       '                         ',186
          line_full_bottom

novesa    db      "??????????: EGA/CGA",13,10,0
vervesa   db      "?????? VESA:  Vesa x.x ",13,10,0
vervesa_off=19
gr_mode   db      186," Vesa 2.0+ 16 M LFB:  [1] 640x480, [2] 800x600, "
          db      "[3] 1024x768, [4] 1280x1024",13,10
          db      186," Vesa 1.2  16 M Bnk:  [5] 640x480, [6] 800x600, "
          db      "[7] 1024x768, [8] 1280x1024",13,10
          db      186," EGA/CGA   256 ???⮢:  [9] 320x200, "
          db      "VGA 16 ???⮢: [0]  640x480",13,10
          db      186," ?롥???? ?????०??: ",0
bt24      db      13,10,186," ??㡨?? 梥??: 24",13,10,0
bt32      db      13,10,186," ??㡨?? 梥??: 32",13,10,0
vrrmprint db      13,10,186," ?ᯮ?짮???? VRR? (??????? ???஢ ???? 60 ??"
          db      " ⮫쪮 ??? ????室??:",13,10
          db      186," 1024*768>800*600 ? 800*600>640*480) [1-??, 2-???]: ",0
askmouse  db      "????:" ; 186, " "
          db      " [1] PS/2 (USB), [2] Com1, [3] Com2."
          db      " ?롥???? ???? [1-3]: ",0
no_com1   db      13,10,186," No COM1 mouse",0
no_com2   db      13,10,186," No COM2 mouse",0
gr_acc    db      13,10,186," Vesa 2.0+: ???????? MTRR ??? ?᪮७?? ???䨪?? "
          db      "[1-??/2-???]: ",0
gr_direct db      186," ?ᯮ?짮???? ???????? ??????????? "
          db      "[1-??/2-???]: ",0
;mem_model db      13,10,186," ???+? ?????? [1-16 Mb / 2-32 Mb / "
;          db      "3-64Mb / 4-128 Mb / 5-256 Mb]: ",0
bootlog   db      13,10,186," ???ᬮ????? ??ୠ? ????㧪?? [1-???/2-??]: ",0
bdev      db      186
          db      " ????㧨?? ??ࠧ ?? [1-??᪥??; 2-C:\menuet.img (FAT32);"
          db      13,10,186,"                    "
          db      "3-?ᯮ?짮???? 㦥 ????㦥???? ??ࠧ]: ",0
probetext db      13,10,13,10,186," ?⠭??????? ?????०??? [1-??, "
          db      "2-?஢????? ??㣨? (Vesa 3.0)]: ",0
;memokz256 db      13,10,186," RAM 256 Mb",0
;memokz128 db      13,10,186," RAM 128 Mb",0
;memokz64  db      13,10,186," RAM 64 Mb",0
;memokz32  db      13,10,186," RAM 32 Mb",0
;memokz16  db      13,10,186," RAM 16 Mb",0
prid      db      " pr.",0
prnotfnd  db      " pr. - ?訡??.",0
modena    db      "?訡?? - ?ॡ????? ?????প? VBE 0x112+.",0
not386    db      "?訡?? - ?ॡ????? ????????? 386+.",0
btns      db      13,10,186," ?訡?? - ?? ???? ??।????? ??㡨?? 梥??.",0
fatalsel  db      13,10,"?訡?? - ???࠭??? ?????०?? ?? ?????ন??????.",0
badsect   db      13,10,186," ?訡?? - ??᪥?? ???०????. ???஡???? ??????. "
          db      0
memmovefailed db  "Fatal - Int 0x15 move failed.",0
linef2    db      13,10
linef     db      13,10,0
okt       db      " ... OK",0
keyin     db      "x",0
keyinbs   db      "x",8,0
diskload  db      13,10,186," ????㧪? ??᪥??: 00 %",8,8,8,8,0
backspace db      8,0
pros      db      "00",8,8,0
leftpr    db      186," ",0
boot_dev  db      0
