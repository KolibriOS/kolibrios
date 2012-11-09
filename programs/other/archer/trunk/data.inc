; @RCHER data area
; Written in pure assembler by Ivushkin Andrey aka Willow

if  SYS eq win
  dumpfile:
            db 'D:\Ivushkin\projects\zip\output.dmp',0
  dumpf_len=$-dumpfile
end if
outfile:
if SYS eq win
            db 'D:\Ivushkin\projects\zip\output.raw',0
end if
            dd 2
            dd 0
            dd 0
  .size     dd 0
  .out      dd output
if ~SYS eq win
  dumpfile:
            db DUMPFILE,0
  dumpf_len=$-dumpfile
end if
attrinfo:
        dd      5
        dd      0
        dd      0
        dd      0
        dd      fileattr
        db      0
        dd      filename
Finfo:
            dd 0
    .block  dd 0
            dd 0
    .count  dd BUFSIZE*1024
    .ptr    dd BUFPTR

filename:
if SYS eq win
db 'png\absolut0.png',0
;db  'fasm-1.54.tgz',0
;include 'testsu~1.inc'
;      db  'pngsuite\pngsuite.tar.gz',0  ; OK
;      db  'pngsuite\basn4a08.png',0  ; OK
;      db  'goldhill.png',0  ; beeping
;      db  'arl_logo.png',0  ; wrong out
;      db  'tech.png',0  ; wrong out
;     db  'png\lena0000.png',0
;      db 'arcs\'
;      db 'NTOSKRNL.gz'
;      db 'msg_enc.zip'
      ;db 'msgplain.zip'
;      db 'c_866.gz'
;     db 'zalman_2.png' ;OK
;     db 'arcs\headerbar.png',0 ;OK
;     db 'fonts1.png' ;OK
;     db 'fonts.png' ;
;     db 'skin.png' ;OK
;     db 'JavaPowered8.png'; OK
;     db 'opr008S8.png'
;    db  'goldhill.png'
;     db 'k3b.png',0
;      db 'image0051.png'
;     db 'window-elements.png',0
;    db 'WWW.zip',0
;     db 'png.zip',0
;    db 'zlib09.zip',0

else
  if ~ FNAME eq
     db FNAME
  end if
;    db '/hd/1/zip/png.zip',0
;    db '/hd/1/zip/files/opossum.png'
;    db '/rd/1/www.zip',0
;    db '/hd/1/zip/test2.zip',0
end if
 db 0
I_END:
rb 1024+16-($-filename)

if SYS eq win
    cr_lf db 0xa,0xd
    hnd dd ?
    cons  dd ?
    cons_in dd ?
    cparam1 dd ?
    cparam2 dd ?
end if

DKeys rd 3
Dheader rb 12
Dpassword rb PASSW_LEN

if ~SYS eq win
fileattr rd 40/4
end if

png_     dd ?
fat_     dd ?
fat_fnum dd ?
lpath    dd ?
lpath_len dd ?
png_bpp  dd ?
sline_len dd ?
IDATcount  dd ?
IDATsize  dd ?
PNG_info:
            .Width dd ?
            .Height dd ?
            .Bit_depth db ?
            .Color_type db ?
            .Compression_method db ?
            .Filter_method db ?
            .Interlace_method db ?
            .Palette rb 3*256
gpbf dw ?
file_count dd ?
filters rd 6
arc_base dd ?
outp  dd ?
unp_size  dd ?
CRC_check dd ?
_CRC32_ dd ?
CRC32table rd 256
Adler32 dd ?
child dd ?
clientPID dd ?

filesize dd ?
bits db ?
cur_byte dd ?
lastblk db ?
Flags dd ?
tblCount dw ?
tblLen dw ?
hclen db ?
max_len dw ?

align 4
fat:
  rb 4096;512
child_stack:
  rb 1024
child_stack_top:
main_stack:
  rb 1024
main_stack_top:
area:
 rb INBUF
os_work rb 4*1024

bl_count rb BITS

next_code rw BITS
tmp_clit:
    rw 20
calph:
    rw 20
sorted_clit rw 20

seql_c db ?
seql rb BITS

seqd_c db ?
seqd rb BITS

hlit dw ?
Literal rw 286
Lit_c rw 286

hdist db ?
Distance rw 32
Dist_c rw 32

output:
;rb OUTBUF

;png_image:
if SYS eq win
  rb OUTBUF
end if
