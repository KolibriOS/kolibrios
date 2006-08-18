; @RCHER parser and filter routines
; Written in pure assembler by Ivushkin Andrey aka Willow

  fhs_local   equ 0x04034b50
  fhs_central equ 0x02014b50
  fhs_end     equ 0x06054b50
  fhs_enc     equ 0x08074b50

SkipASCIIZ:
    xor  eax,eax
    mov  ecx,255
    mov  edi,esi
    repne scasb
    mov  esi,edi
    ret

PrintFilename:
    pusha
    mov  esi,edx
    mov  edi,os_work
    mov  edx,edi
    rep  movsb
    mov  dword[edi],0x00a0d
    call DebugPrint
    popa
    call Add2Fat
    ret

Add2Fat:
; edx - ptr, ecx - len

    pusha
    test [Flags],LIST_MODE
    jz   .ex
    mov  ebp,8
    mov  edi,edx
    lea  ebx,[edx+ecx]
    mov  ecx,[lpath_len]
    cmp  ecx,1
    je   .lbl
    mov  esi,[lpath]
    repe cmpsb
    jne  .full
    mov  eax,[lpath_len]
    sub  dword[esp+24],eax;path_len-path
    cmp  edi,ebx
    je   .full
    mov  edx,edi
  .lbl:
    mov  ecx,[esp+24]
    mov  al,'/'
    repne scasb
    mov  eax,[fat_]
    mov  ecx,[esp+24]
    jne  .nofol
    cmp  edi,ebx
    jne  .full
    lea  ecx,[edi-1]
    sub  ecx,edx
    or   byte[eax+11],0x10
;    sub  edx,ecx
  .nofol:

    push [fat_fnum]
    pop  dword[eax+12]
    mov  edi,eax
    mov  esi,edx
  .lp1:

    mov  bl,[esi]
    lea  edx,[eax+ebp]
    inc  esi
    cmp  bl,'.'
    jne  .nodot
    lea  edi,[eax+ebp]
    mov  ebp,11
    jmp  .ll
  .nodot:
    cmp  edi,edx
    jae  .ll
    mov  [edi],bl
    inc  edi
  .ll:
    loop .lp1
    mov  ecx,11
    dec  eax
  .lp2:
    cmp  byte[eax+ecx],0
    jne  .no0
    mov  byte[eax+ecx],' '
  .no0:
    loop .lp2
    cmp  eax,child_stack-1
    jae  .full
    add  [fat_],32
  .full:
    inc  [fat_fnum]
  .ex:
    popa
    ret

;path db '/';'fasm/examples/elfexe/'
;path_len:

; Parse routines:
;   out: edx= 0 if all ok, 1 - central dir, 2-EOD
;             50 - encrypted
;             51 - not deflated
;             52 - invalid format
;             53 - dir skipped
;             1 - encrypted

; ****************************************************
ZipParse:

    call ResetFile
  .nxt:
    call ZipCrawl

    cmp  edx,3
    je  .ex
    cmp  edx,1
    je   .skipinc
if  IGNORE_DIRS eq 1
    cmp  edx,53
    jne  .skipinc
end if
    inc  [file_count]
  .skipinc:
    cmp  edx,52
    je   .er1
    cmp  edx,50
    jne   .seek
  .er1:
    Msg  edx
    ret
  .seek:
    add  eax,ecx
    mov  ebx,1
    call FileSeek
    jmp  .nxt
  .ex:
    Msg  2
    mov  eax,[file_count]
 if ~ SYS eq win
    dpd  eax
 else
    pusha
    call  int2str
    mov   edx,os_work
   	call  DebugPrint
   	popa
 end if
    Newline
;    Dump fat,160,os_work
    ret

ZipFindN:
; ecx - file #
    Msg 33
    or   [Flags],FIND_MODE
    cmp  ecx,[file_count]
    jae  .err
    push ecx
    call ResetFile
  .nxt:

    call ZipCrawl
    cmp  edx,51
    je   .ok2
  .noenc:
    test edx,edx
    jnz  .err
  .ok2:
    add  eax,ecx
    cmp  dword[esp],0
    jz   .ok
    dec  dword[esp]
    mov  ebx,1
    call FileSeek
    jmp  .nxt
  .err:
    mov  edx,4
    jmp  .ex
  .ok:
    pop  ecx
    sub  eax,[esi+18]
    add  esi,eax
    mov  edx,5
  .ex:
    and   [Flags],-1-FIND_MODE
    push edx
    Msg  edx
    pop  edx
    ret

ZipCrawl:
    mov  edx,52
    cmp  dword[esi],fhs_central
    jne  .noc
    mov  eax,46
    movzx ecx,word[esi+28]
    add  eax,ecx
    movzx ecx,word[esi+30]
    add  eax,ecx
    movzx ecx,word[esi+32]
    mov  edx,1
    ret
  .noc:
    cmp  dword[esi],fhs_end
    jne  .noe
  .edx3:
    Msg 3
    mov  edx,3
    ret
  .noe:
    cmp  dword[esi],fhs_local
    je   .loc
    cmp  dword[esi],fhs_enc
    jne  .err
    mov  eax,16
    xor  ecx,ecx
    mov  edx,1
    ret
  .loc:
    push word[esi+6]
    pop  [gpbf]
    push dword[esi+14]
    pop  [CRC_check]
    push dword[esi+22]
    pop  [unp_size]
    movzx ecx,word[esi+26]
    mov  eax,30
    lea  edx,[esi+eax]
    add  eax,ecx
if  IGNORE_DIRS eq 1
    cmp  byte[edx+ecx-1],'/'
    je   .skipdp
end if
    test [Flags],FIND_MODE
    jnz  .skipdp
    call PrintFilename
  .skipdp:
    movzx ecx,word[esi+28]
    add  eax,[esi+18]
    test [gpbf],1
    jz   .no_enc
    or   [Flags],DECRYPT_MODE  ; encrypted
    mov  edx,51
    jmp  .err
  .no_enc:
    test word[esi+8],7
    rep_err z,50
  .ok:
    xor  edx,edx
  .err:
    ret

; ***********************************************
GzipParse:
    ID1ID2 equ 0x8b1f
    FTEXT equ 1b
    FHCRC equ 10b
    FEXTRA equ 100b
    FNAME equ 1000b
    FCOMMENT equ 10000b
    mov  eax,7
    mov  ebx,2
    call FileSeek
    push dword[esi]
    pop  [CRC_check]
    push dword[esi+4]
    pop  [unp_size]
    call ResetFile
    xor  edx,edx
    cmp  word[esi],ID1ID2
    rep_err e, 52, 15
    cmp  byte[esi+2],8
    rep_err e, 52, 50
    mov  bl,[esi+3]  ; bl - FLG
    add  esi,10 ; esi->extra
    test bl,FEXTRA
    jz   .noextr
    movzx eax,word[esi]
    lea  esi,[esi+eax+2] ; esi->FNAME
  .noextr:
    test bl,FNAME
    jz   .nofname
    mov  edx,esi
    call DebugPrint
    call SkipASCIIZ
    cmp  dword[esi-5],'.tar'
    jne  .nofname
    or   [Flags],TAR_MODE
  .nofname:     ; esi->FCOMMENT
    test bl,FCOMMENT
    jz   .nocomm
    call SkipASCIIZ
  .nocomm:      ; esi->HCRC
    test bl,FHCRC
    jz   .noCRC16
    add  esi,2
  .noCRC16:
    cmp  [unp_size],OUTBUF
    jb   .sizeok2
    Msg  16
    mov  edx,15
    ret
  .sizeok2:
    xor  edx,edx
  .err:
    ret

PngParse:
    ID1 equ 0x474e5089
    ID2 equ 0x0a1a0a0d
    FDICT equ 100000b
    InitIDAT equ 2
    mov  [IDATcount],InitIDAT
    call ResetFile
    cmp  dword[esi],ID1
    rep_err e, 52, 18
    cmp  dword[esi+4],ID2
    rep_err e, 52, 18
    add  esi,8
    cmp  dword[esi+4],'IHDR'
    rep_err e,52, 18
    or   [Flags],PNG_MODE
    memcpy_esi PNG_info,13,8
    mov  eax,[PNG_info.Width]
    bswap eax
    mov  [PNG_info.Width],eax
    mov  eax,[PNG_info.Height]
    bswap eax
    mov  ebx,eax
    mov  [PNG_info.Height],eax
    call scanline_calc
;    dps  'All='
    cmp  [PNG_info.Color_type],3
    jne  .nopal
    shl  eax,3
    inc  eax
  .nopal:
    inc  eax
    imul eax,ebx
    mov  [unp_size],eax
;    dpd  eax
    add  esi,25
    cmp  byte[esi-5],0
    rep_err e,52,29
  .nxt_sec:
    lodsd
    bswap eax ; eax - section size
    push eax
    lodsd
    mov  edi,Png_ch
    mov  ecx,(E_ch-Png_ch) / 4
    repne scasd
    pop  eax
    mov  ebx,[esi-4]
    mov  edx,os_work
    mov  [edx],ebx
    mov  dword[edx+4],0x0a0d
  .dp:
    sub  edi,Png_ch
    shr  edi,2  ; edi- chunk #
 if SHOW_PNG_SEC eq 1
    call DebugPrint
 end if
    cmp  edi,1
    jne  .noend
    mov  edx,21
    jmp  .err
  .noend:
    cmp  edi,2
    jne  .noplte
    memcpy_esi PNG_info.Palette,eax
    jmp  .noidat
   .noplte:
    cmp  edi,3
    jne  .noidat
    mov  [IDATsize],eax
    cmp  [IDATcount],InitIDAT
    jne  .ex
    mov  [bits],8
  if RBLOCK eq 4
    lodsd
  else
    lodsb
  end if
    call setcurb
    rbits 0,16
    test ah,FDICT
    jz   .ex
    rbits 0,32
    add  [IDATcount],4
    jmp  .ex
   .noidat:
    add  eax,4
    mov  ebx,1
    call FileSeek
    jmp  .nxt_sec
   .ex:
    xor  edx,edx
   .err:
    ret

Png_ch:
    dd 'IEND','PLTE','IDAT','????'
E_ch:

ZipDecrypt:
    push edi
    mov  ecx,3
    mov  edi,Dheader
    rep  movsd
    pop  edi
    call QueryPwd
    jecxz .ex
    push esi
    mov  [DKeys],  305419896
    mov  [DKeys+4],591751049
    mov  [DKeys+8],878082192
    xor  eax,eax
    mov  esi,Dpassword
  .enc_init:
    lodsb
    call UKeys
    loop .enc_init
    mov  ecx,12
    mov  esi,Dheader
  .dec_header:
    call decrypt_byte
    xor  al,[esi]
    call UKeys
    mov  [esi],al
    inc  esi
    loop .dec_header
    mov  eax,[CRC_check]
    pop  esi
  .ex:
    ret

QueryPwd:
; out: ecx - passwd len
if  SYS eq win
    Msg 32
    invoke ReadConsole,[cons_in],Dpassword,PASSW_LEN,cparam1,NULL
    test eax,eax
    jnz  .inp_ok
    xor  ecx,ecx
    jmp  .ex
  .inp_ok:
    mov  ecx,[cparam1]
    cmp  ecx,PASSW_LEN
    je   .ex
    sub  ecx,2
else
end if
  .ex:
    ret

UKeys:
; in: al - char
    pusha
    mov  edi,134775813
    mov  ebx,DKeys
    mov  esi,os_work
    mov  byte[esi],al
    mov  ecx,1
    push dword[ebx]
    pop  [CRC32]
    call UCRC
    push [CRC32]
    pop  dword[ebx]
    mov  eax,[ebx]
    and  eax,0xff
    add  eax,[ebx+4]
    mul  edi
    inc  eax
    mov  [ebx+4],eax
    shr  eax,24
    mov  byte[esi],al
    push dword[ebx+8]
    pop  [CRC32]
    call UCRC
    push [CRC32]
    pop  dword[ebx+8]
    popa
    ret

decrypt_byte:
; out: al
    push ebx edx
    movzx ebx,word[DKeys+8]
    or   ebx,2
    mov  eax,ebx
    xor  eax,1
    mul  ebx
    shr  eax,8
    pop  edx ebx
    ret

setcurb:
; in: eax
    test [Flags],DECRYPT_MODE
    jz   .noenc
    push eax
    call decrypt_byte
    xor  al,byte[esp]
    add  esp,4
    call UKeys
  .noenc:
    mov  [cur_byte],eax
    ret

TarParse:
    mov  esi,output
;    call ResetFile
  .nxt:
    call TarCrawl
;    wait
    cmp  edx,3
    je   ZipParse.ex
if  IGNORE_DIRS eq 1
    cmp  edx,53
    jne  .skipinc
end if
    inc  [file_count]
  .skipinc:
    add  eax,ecx
;    mov  ebx,1
    add  esi,eax
;    call FileSeek
    jmp  .nxt

TarFindN:
; in:  ecx - file number
; ecx - file #
    Msg 33
    cmp  ecx,[file_count]
    jae  .err
    push ecx
    mov  esi,output
;    call ResetFile
  .nxt:
    call TarCrawl
if  IGNORE_DIRS eq 1
    cmp  edx,53
    je  .seek
end if
    test edx,edx
    jnz  .err
    cmp  dword[esp],0
    jz   .ok
    dec  dword[esp]
  .seek:
    add  eax,ecx
;    mov  ebx,1
    add  esi,eax
;    call FileSeek
    jmp  .nxt
  .err:
    mov  edx,4
    jmp  .ex
  .ok:
    pop  ecx
    add  esi,eax
    mov  edx,5
  .ex:
    Msg  edx
    ret

TarCrawl:
    cmp  byte[esi],0
    jz   ZipCrawl.edx3
    push esi
    mov  ecx,11
    add  esi,0x7c
    call Octal_str
    mov  esi,[esp]
    mov  [outfile.size],eax
    call SkipASCIIZ
if  IGNORE_DIRS eq 1
    cmp  byte[esi-2],'/'
    je   .skipdp
end if
    mov  edx,[esp]
    lea  ecx,[esi-1]
    sub  ecx,edx
    call PrintFilename
  .skipdp:
    mov  ecx,[outfile.size]
    jecxz .zerolen
    shr  ecx,9
    inc  ecx
    shl  ecx,9
  .zerolen:
    mov  eax,512
    pop  esi
    jmp  ZipCrawl.ok

Octal_str:
; in:  esi - ASCIIZ octal string
;      ecx - its length
; out: eax - value
    push esi ebx ecx
    xor  ebx,ebx
    xor  eax,eax
  .jec:
    jecxz .zero
    cmp  byte[esi+ecx-1],' '
    jne  .lp
    dec  ecx
    jmp  .jec
  .lp:
    lodsb
    shl  ebx,3
    cmp  eax,' '
    je   .space
    lea  ebx,[ebx+eax-'0']
  .space:
    loop .lp
    mov  eax,ebx
  .zero:
    pop  ecx ebx esi
    ret

TRAILING_BUF equ 2048
SfxParse:
    call ResetFile
    cmp  word[esi],'MZ'
    rep_err e, 34
    mov  eax,TRAILING_BUF
    mov  ecx,eax
    mov  ebx,2
    call FileSeek
    mov  edi,esi
    mov  al,'P'
  .lp:
    repne scasb
    cmp  dword[edi-1],fhs_end
    je   .end_found
    jecxz .err
    jmp   .lp
  .end_found:
    dec   edi
    mov   esi,edi
    mov   eax,[edi+12]
    neg   eax
    mov   ebx,1
    call  FileSeek
    push  dword[esi+42]
    pop   [arc_base]
  .err:
    ret

scanline_calc:
    movzx ecx,byte[PNG_info.Color_type]
    mov  eax,1
    cmp  cl,3
    je   .palette
    test cl,2
    jz  .notriple
    add  eax,2
  .notriple:
    test cl,4
    jz   .calc_bpp
    inc  eax
  .calc_bpp:
    mul  [PNG_info.Bit_depth]
  .palette:
    mov  ecx,eax   ; in bits
    shr  eax,3     ; in bytes
    test eax,eax
    jnz  .noz
    inc  eax
  .noz:
    mov  [png_bpp],eax
    mov  eax,[PNG_info.Width]
    mov  ebp,eax
    imul ecx
    shr  eax,3
    test eax,eax
    jnz  .noz2
    inc  eax
  .noz2:
    ret

; Created:  May 31, 2005
FiltCall:
dd PngFilter.nofilt,Filt_sub,Filt_up,Filt_av,Filt_paeth,PngFilter.nofilt
PngFilter:
; esi - filtered uncompressed image data
; edi - destination
    call scanline_calc
    mov  [sline_len],eax   ; scanline length
    push edi
    and  [Flags],not 1
    mov  ecx,[PNG_info.Height]
  .scanline:
;    Msg 9,1
    push ecx
    lodsb
    movzx eax,al
    cmp  eax,5
    jb   .f_ok
    mov  eax,5
  .f_ok:
    inc  dword[filters+eax*4]
    jmp  dword[FiltCall+eax*4]
  .nofilt:
    mov  dl,[PNG_info.Color_type]
    cmp  dl,3
    jne  .nopalette
    lodsb
    mov  [cur_byte],eax
    mov  [bits],8
    mov  ecx,ebp
  .pixel:
    push ecx
    movzx ecx,[PNG_info.Bit_depth]
    call rb_png
    push esi
    lea  esi,[eax+eax*2]
    add  esi,PNG_info.Palette
    call PngStore
    pop  esi
    pop  ecx
    loop .pixel
    cmp  [bits],8
    jne  .lp
    dec  esi
  .lp:
    pop  ecx
    loop .sl
    jmp  .sl2
  .sl:
;//
MV equ 1
;    mov  eax,ecx
;    and  eax,1 shl MOVE_SLINE_LEV-1
;    jnz  .scanline
;stop
if MV eq 0
    push ecx
    mov  ecx,edi
    sub  ecx,esi
    sub  [outp],esi
    mov  edi,output
    add  [outp],edi
    rep  movsb
    mov  esi,output
    pop  ecx
    pop  eax
    push [outp]
end if
;;//
    jmp  .scanline
  .sl2:
;//
;    call MoveScanline
    sub  edi,[outp]
;//
;    sub  edi,[esp]
    pop  eax
    ret

  .nopalette:
    test  dl,2
    jz   .notriple1
  .__:
    mov  ecx,[PNG_info.Width]
  .RGBcp:
    call PngStore
    add  esi,[png_bpp]
    loop .RGBcp
    jmp  .lp
  .notriple1:
    test dl,dl
    jz  .gray
    cmp  dl,4
    jne .__
;    Msg 31
;    ud2
  .gray:
;    stop
    push ecx
    mov  ecx,[PNG_info.Width]
    mov  [bits],8
    lodsb
    mov  [cur_byte],eax
  .gray2:
    push ecx
    movzx ecx,[PNG_info.Bit_depth]
    push ecx
    call rb_png
    pop  ecx
    cmp  ecx,8
    jbe  .lo
    add  esi,2
    shr  eax,8
    jmp  .stsb
  .lo:
    neg  ecx
    add  ecx,8
    shl  eax,cl
  .stsb:
    mov  ecx,3
    rep  stosb
    pop  ecx
    loop .gray2
    dec  esi
    pop  ecx
    jmp  .lp

Filt_sub:
;    dps  '-'
    mov  ecx,[sline_len]
    sub  ecx,[png_bpp]
    push esi edi
    mov  edi,esi
    add  edi,[png_bpp]
  .scan:    ; esi - previous, edi - current
    lodsb
    add  [edi],al
    inc  edi
    loop .scan

    pop  edi esi
;    dps  '-'
    jmp  PngFilter.nofilt

Filt_up:
    cmp  ecx,[PNG_info.Height]
    je   PngFilter.nofilt
    push esi edi
    mov  ecx,[sline_len]
    mov  edi,esi
    sub  esi,ecx
    dec  esi
    jmp  Filt_sub.scan

Filt_av:
    pusha
    mov  ecx,[sline_len]
    mov  ebp,[PNG_info.Height]
    mov  edx,[png_bpp] ; edx-raw
    neg  edx
    mov  ebx,ecx
    sub  ebx,[png_bpp]
    mov  edi,esi
    sub  esi,ecx
    dec  esi           ; esi-prior
  .lpavg:
    xor  eax,eax
    cmp  [esp+24h],ebp
    je   .1stl
    movzx eax,byte[esi]
  .1stl:
    cmp  ecx,ebx
    ja   .leftbad
    push ecx
    movzx ecx,byte[edi+edx]
    add  eax,ecx
    pop  ecx
  .leftbad:
    shr  eax,1
    add  [edi],al
    inc  esi
    inc  edi
    loop .lpavg
    popa
    jmp  PngFilter.nofilt

Filt_paeth:
    pusha
    mov  ecx,[sline_len]
    mov  edx,[png_bpp]
    neg  edx
    lea  ebp,[ecx+edx] ; left edge
    mov  edi,esi
    sub  esi,ecx
    dec  esi
  .lpaeth:
    push ecx
    movzx eax,byte[edi+edx]
    movzx ebx,byte[esi]
    movzx ecx,byte[esi+edx]
    push eax
    mov  eax,[esp+28h]
    cmp  eax,[PNG_info.Height] ; 1st line
    jne  .no1stlineok
    xor  ebx,ebx
    xor  ecx,ecx
  .no1stlineok:
    pop  eax
    cmp  [esp],ebp     ; ecx
    jbe  .leftok      ; x-bpp>=0
    xor  eax,eax
    xor  ecx,ecx
  .leftok:
    pusha   ; eax-28, ebx-16, ecx-24
    lea  edx,[eax+ebx]
    sub  edx,ecx        ; p=edx
    sub  eax,edx        ; pa := abs(p - a)
    jge  .eaxp
    neg  eax
  .eaxp:
    sub  ebx,edx        ; pb := abs(p - b)
    jge  .ebxp
    neg  ebx
  .ebxp:
    sub  ecx,edx        ; pc := abs(p - c)
    jge  .ecxp
    neg  ecx
  .ecxp:
    cmp  eax,ebx
    ja   .noa
    cmp  eax,ecx
    jbe  .ex            ; pa-min
  .noa:
    cmp  ebx,ecx
    ja   .nob
    mov  eax,[esp+16]
    jmp  .ex2
  .nob:
    mov  eax,[esp+24]
  .ex2:
    mov  [esp+28],eax
  .ex:
    popa
    add  [edi],al
    inc  esi
    inc  edi
    pop  ecx
    loop .lpaeth
    popa
    jmp  PngFilter.nofilt

rb_png: ; eax-dest; ecx-count
    push ecx
    xor  eax,eax
  .shift:
    rol  byte[cur_byte],1
    rcl  eax,1
  .dec:
    dec  [bits]
    jnz  .loop1
  .push:
    push dword[esi]
    pop  [cur_byte]
    mov  [bits],8
    inc  esi
  .loop1:
    loop .shift
    pop  ecx
    ret

PngStore:
    push esi
    cmp  [PNG_info.Bit_depth],8
    jbe  .lo
    add  esi,3
  .lo:
  if ~ SYS eq win
    mov  esi,[esi]
    bswap esi
    shr  esi,8
    mov  [edi],esi
    add  edi,3
  else
    movsw
    movsb
  end if
    pop  esi
    ret

FiltStats:
    pusha
    xor  ebx,ebx
    mov  edx,23
    mov  ecx,6
  .lp:
    push ecx edx
    Msg  edx
    mov  eax,[filters+ebx*4]
    DebugPrintDec
    pop  edx ecx
    inc  edx
    inc  ebx
    loop .lp
    Newline
    popa
    ret

