;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; RAMDISK functions                                                    ;;
;; (C) 2004 Ville Turjanmaa, License: GPL                               ;;
;; Addings by M.Lisovin                                                 ;;
;; LFN support by diamond                                               ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; calculate fat chain

calculatefatchain:

   pushad

   mov  esi,0x100000+512
   mov  edi,0x280000

 fcnew:
   mov  eax,dword [esi]
   mov  ebx,dword [esi+4]
   mov  ecx,dword [esi+8]
   mov  edx,ecx
   shr  edx,4   ;8 ok
   shr  dx,4    ;7 ok
   xor  ch,ch
   shld ecx,ebx,20 ;6 ok
   shr  cx,4     ;5 ok
   shld ebx,eax,12
   and  ebx,0x0fffffff  ;4 ok
   shr  bx,4    ;3 ok
   shl  eax,4
   and  eax,0x0fffffff  ;2 ok
   shr  ax,4  ;1 ok
   mov  dword [edi],eax
   mov  dword [edi+4],ebx
   mov  dword [edi+8],ecx
   mov  dword [edi+12],edx
   add  edi,16
   add  esi,12

   cmp  edi,0x280000+2856*2   ;2849 clusters
   jnz  fcnew

   popad
   ret


restorefatchain:   ; restore fat chain

   pushad

   mov  esi,0x280000
   mov  edi,0x100000+512

  fcnew2:
   mov  eax,dword [esi]
   mov  ebx,dword [esi+4]
   shl  ax,4
   shl  eax,4
   shl  bx,4
   shr  ebx,4
   shrd eax,ebx,8
   shr  ebx,8
   mov  dword [edi],eax
   mov  word [edi+4],bx
   add  edi,6
   add  esi,8

   cmp  edi,0x100000+512+4278     ;4274 bytes - all used FAT
   jb   fcnew2

   mov  esi,0x100000+512           ; duplicate fat chain
   mov  edi,0x100000+512+0x1200
   mov  ecx,1069        ;4274/4
   cld
   rep  movsd

   popad
   ret


ramdisk_free_space:
;---------------------------------------------
;
; returns free space in edi
; rewr.by Mihasik
;---------------------------------------------

        push   eax ebx ecx

        mov  edi,0x280000 ;start of FAT
        xor  ax,ax    ;Free cluster=0x0000 in FAT
        xor  ebx,ebx  ;counter
        mov  ecx,2849 ;2849 clusters
        cld
    rdfs1:
        repne scasw
        jnz  rdfs2    ;if last cluster not 0
        inc  ebx
        test    ecx, ecx
        jnz     rdfs1
    rdfs2:
        shl  ebx,9    ;free clusters*512
        mov  edi,ebx

        pop    ecx ebx eax
        ret


expand_filename:
;---------------------------------------------
;
; exapand filename with '.' to 11 character
; eax - pointer to filename
;---------------------------------------------

        push esi edi ebx

        mov  edi,esp                  ; check for '.' in the name
        add  edi,12+8

        mov  esi,eax

        mov  eax,edi
        mov  [eax+0],dword '    '
        mov  [eax+4],dword '    '
        mov  [eax+8],dword '    '

      flr1:

        cmp  [esi],byte '.'
        jne  flr2
        mov  edi,eax
        add  edi,7
        jmp  flr3

      flr2:

        mov  bl,[esi]
        mov  [edi],bl

      flr3:

        inc  esi
        inc  edi

        mov  ebx,eax
        add  ebx,11

        cmp  edi,ebx
        jbe  flr1

        pop  ebx edi esi
        ret

fileread:
;----------------------------------------------------------------
;
;  fileread - sys floppy
;
;  eax  points to filename 11 chars
;  ebx  first wanted block       ; 1+ ; if 0 then set to 1
;  ecx  number of blocks to read ; 1+ ; if 0 then set to 1
;  edx  mem location to return data
;  esi  length of filename 12*X 0=root
;
;  ret ebx = size or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
        test   ebx,ebx ;if ebx=0 - set to 1
        jnz    frfl5
        inc    ebx
      frfl5:
        test   ecx,ecx ;if ecx=0 - set to 1
        jnz    frfl6
        inc    ecx
      frfl6:
        test   esi,esi          ; return ramdisk root
        jnz    fr_noroot        ;if not root
        cmp    ebx,14           ;14 clusters=root dir
        ja     oorr
        cmp    ecx,14
        ja     oorr
        jmp    fr_do
      oorr:
        mov    eax,5            ;out of root range (fnf)
        xor    ebx,ebx
        dec    ebx              ;0xffffffff
        ret

      fr_do:                    ;reading rootdir
        mov    edi,edx
        dec    ebx
        push   edx
        mov    edx,ecx
        add    edx,ebx
        cmp    edx,15     ;ebx+ecx=14+1
        pushf
        jbe    fr_do1
        sub    edx,14
        sub    ecx,edx
      fr_do1:
        shl    ebx,9
        mov    esi,0x100000+512*19
        add    esi,ebx
        shl    ecx,7
        cld
        rep    movsd
        popf
        pop    edx
        jae    fr_do2
        xor    eax,eax ; ok read
        xor    ebx,ebx
        ret
      fr_do2:        ;if last cluster
        mov    eax,6  ;end of file
        xor    ebx,ebx
        ret

     fr_noroot:

        sub    esp,32
        call   expand_filename

        dec    ebx

        push   eax

        push   eax ebx ecx edx esi edi
        call   rd_findfile
        je     fifound
        add    esp,32+28   ;if file not found
        ret

     fifound:

        mov    ebx,[edi-11+28]          ;file size
        mov    [esp+20],ebx
        mov    [esp+24],ebx
        add    edi,0xf
        movzx  eax,word [edi]
        mov    edi,eax                  ;edi=cluster

      frnew:

        add    eax,31                   ;bootsector+2*fat+filenames
        shl    eax,9                    ;*512
        add    eax,0x100000             ;image base
        mov    ebx,[esp+8]
        mov    ecx,512                  ;[esp+4]

        cmp    [esp+16],dword 0         ; wanted cluster ?
        jne    frfl7
        call   memmove
        add    [esp+8],dword 512
        dec    dword [esp+12]           ; last wanted cluster ?
        je     frnoread
        jmp    frfl8
      frfl7:
        dec    dword [esp+16]
      frfl8:
        movzx  eax,word [edi*2+0x280000]	; find next cluster from FAT
        mov    edi,eax
        cmp    edi,4095                 ;eof  - cluster
        jz     frnoread2

        cmp    [esp+24],dword 512       ;eof  - size
        jb     frnoread
        sub    [esp+24],dword 512

        jmp    frnew

      frnoread2:

        cmp    [esp+16],dword 0         ; eof without read ?
        je     frnoread

        pop    edi esi edx ecx
        add    esp,4
        pop    ebx     ; ebx <- eax : size of file
        add    esp,36
        mov    eax,6   ; end of file
        ret

      frnoread:

        pop    edi esi edx ecx
        add    esp,4
        pop    ebx     ; ebx <- eax : size of file
        add    esp,36
        xor    eax,eax  ;read ok
        ret

filedelete:
;--------------------------------------------
;
; filedelete - sys floppy
; in:
; eax -  pointer to filename 11 chars
;
; out:
; eax - 0 = successful, 5 = file not found
;
;--------------------------------------------

        sub    esp,32
        call   expand_filename

        push   eax ebx ecx edx esi edi

        call   rd_findfile
        je     fifoundd
        pop    edi esi edx ecx ebx eax ;file not found
        add    esp,32
        mov    eax,5
        ret

     fifoundd:

        mov    [edi-11],byte 0xE5       ;mark filename deleted
        add    edi,0xf
        movzx  eax,word [edi]
        mov    edi,eax                  ;edi = cluster

      frnewd:

        shl    edi,1                    ;find next cluster from FAT
        add    edi,0x280000
        movzx  eax,word [edi]
        mov    [edi],word 0x0           ;clear fat chain cluster
        mov    edi,eax
        cmp    edi,dword 0xff8          ;last cluster ?
        jb     frnewd

        pop    edi esi edx ecx ebx eax
        add    esp,32
        xor    eax,eax       ; file found
        ret



filesave:
;----------------------------------------------------------
;
; filesave - sys floppy
;
; eax points to filename 11 chars
;
;        eax      ; pointer to file name
;        ebx      ; buffer
;        ecx      ; count to write in bytes
;        edx      ; 0 create new , 1 append
;
;-----------------------------------------------------------

        sub  esp,32
        call expand_filename
        test edx,edx
        jnz  fsdel
        pusha
        call filedelete
        popa

      fsdel:

        call   ramdisk_free_space
        cmp    ecx,edi
        jbe    rd_do_save
        add    esp,32
        mov    eax,8    ;disk full
        ret

      rd_do_save:

        push   eax ebx ecx edx esi edi

        mov    edi,0x100000+512*18+512  ;Point at directory
        mov    edx,224 +1
        ; find an empty spot for filename in the root dir
     l20ds:
        dec    edx
        jz     frnoreadds
     l21ds:
        cmp    [edi],byte 0xE5
        jz     fifoundds
        cmp    [edi],byte 0x0
        jz     fifoundds
        add    edi,32                   ; Advance to next entry
        jmp    l20ds
     fifoundds:

        push   edi                      ; move the filename to root dir
        mov    esi,[esp+4+20]
        mov    ecx,11
        cld
        rep    movsb
        pop    edi
        mov    edx,edi
        add    edx,11+0xf               ; edx <- cluster save position
        mov    ebx,[esp+12]             ; save file size
        mov    [edi+28],ebx
        mov    [edi+11],byte 0x20       ; attribute
; Ivan Poddubny 11/12/2003:
call get_date_for_file   ; from FAT32.INC
mov [edi+24],ax          ; date
call get_time_for_file   ; from FAT32.INC
mov [edi+22],ax          ; time
; End
        mov    edi,0x280000            ;pointer to first cluster
        mov    ecx,2849
        cld
      frnewds:
        xor    ax,ax
        repne  scasw
        mov    ebx,2848
        sub    ebx,ecx
        mov    [edx],bx                 ; save next cluster pos. to prev cl.
        mov    edx,edi                  ; next save pos abs mem add
        dec    edx
        dec    edx
        call   fdc_filesave
        pusha                           ; move save to floppy cluster
        add    ebx,31
        shl    ebx,9
        add    ebx,0x100000
        mov    eax,[esp+32+16]
        mov    ecx,512
        call   memmove
        popa

        mov    eax,[esp+12]
        cmp    eax,512
        jbe    flnsa
        sub    eax,512
        mov    [esp+12],eax
        add     dword [esp+16], 512
        jmp    frnewds

     flnsa:
        mov    [edi-2],word 4095          ; mark end of file - last cluster

      frnoreadds:

        pop    edi esi edx ecx ebx eax
        add    esp,32

;        pusha
;        cli
;        call   fdc_commitfile
;        sti
;        popa

        xor    eax,eax ;ok write
        ret

   rd_findfile:
   ;by Mihasik
   ;IN: eax - pointer to filename OUT: filestring+11 in edi or notZero in flags and fnf in eax,ebx

        mov    edi,0x100000+512*18+512  ;Point at directory
        cld
    rd_newsearch:
        mov    esi,eax
        mov    ecx,11
        rep    cmpsb
        je     rd_ff
        add    cl,21
        add    edi,ecx
        cmp    edi,0x100000+512*33
        jb     rd_newsearch
        mov    eax,5      ;if file not found - eax=5
        xor    ebx,ebx
        dec    ebx    ;ebx=0xffffffff and zf=0
     rd_ff:
        ret

; \begin{diamond}

uni2ansi_str:
; convert UNICODE zero-terminated string to ASCII-string (codepage 866)
; in: esi->source, edi->buffer (may be esi=edi)
; destroys: eax,esi,edi
        lodsw
        test    ax, ax
        jz      .done
        cmp     ax, 0x80
        jb      .ascii
        cmp     ax, 0x401
        jz      .yo1
        cmp     ax, 0x451
        jz      .yo2
        cmp     ax, 0x410
        jb      .unk
        cmp     ax, 0x440
        jb      .rus1
        cmp     ax, 0x450
        jb      .rus2
.unk:
        mov     al, '_'
        jmp     .doit
.yo1:
        mov     al, 'ð'
        jmp     .doit
.yo2:
        mov     al, 'ñ'
        jmp     .doit
.rus1:
; 0x410-0x43F -> 0x80-0xAF
        add     al, 0x70
        jmp     .doit
.rus2:
; 0x440-0x44F -> 0xE0-0xEF
        add     al, 0xA0
.ascii:
.doit:
        stosb
        jmp     uni2ansi_str
.done:
        mov     byte [edi], 0
        ret

ansi2uni_char:
; convert ANSI character in al to UNICODE character in ax, using cp866 encoding
        mov     ah, 0
; 0x00-0x7F - trivial map
        cmp     al, 0x80
        jb      .ret
; 0x80-0xAF -> 0x410-0x43F
        cmp     al, 0xB0
        jae     @f
        add     ax, 0x410-0x80
.ret:
        ret
@@:
; 0xE0-0xEF -> 0x440-0x44F
        cmp     al, 0xE0
        jb      .unk
        cmp     al, 0xF0
        jae     @f
        add     ax, 0x440-0xE0
        ret
; 0xF0 -> 0x401
; 0xF1 -> 0x451
@@:
        cmp     al, 'ð'
        jz      .yo1
        cmp     al, 'ñ'
        jz      .yo2
.unk:
        mov     al, '_'         ; ah=0
        ret
.yo1:
        mov     ax, 0x401
        ret
.yo2:
        mov     ax, 0x451
        ret

char_toupper:
; convert character to uppercase, using cp866 encoding
; in: al=symbol
; out: al=converted symbol
        cmp     al, 'a'
        jb      .ret
        cmp     al, 'z'
        jbe     .az
        cmp     al, ' '
        jb      .ret
        cmp     al, 'à'
        jb      .rus1
        cmp     al, 'ï'
        ja      .ret
; 0xE0-0xEF -> 0x90-0x9F
        sub     al, 'à'-''
.ret:
        ret
.rus1:
; 0xA0-0xAF -> 0x80-0x8F
.az:
        and     al, not 0x20
        ret

fat_get_name:
; in: edi->FAT entry
; out: CF=1 - no valid entry
; else CF=0 and ebp->ASCIIZ-name
; (maximum length of filename is 255 (wide) symbols without trailing 0,
;  but implementation requires buffer 261 words)
; destroys eax
        cmp     byte [edi], 0
        jz      .no
        cmp     byte [edi], 0xE5
        jnz     @f
.no:
        stc
        ret
@@:
        cmp     byte [edi+11], 0xF
        jz      .longname
        push    ecx
        push    edi ebp
        test    byte [ebp-4], 1
        jnz     .unicode_short

        mov     eax, [edi]
        mov     ecx, [edi+4]
        mov     [ebp], eax
        mov     [ebp+4], ecx

        mov     ecx, 8
@@:
        cmp     byte [ebp+ecx-1], ' '
        loope    @b

        mov     eax, [edi+8]
        cmp     al, ' '
        je      .done
        shl     eax, 8
        mov     al, '.'

        lea ebp, [ebp+ecx+1]
        mov     [ebp], eax
        mov     ecx, 3
@@:
        rol eax, 8
        cmp al, ' '
        jne .done
        loop   @b
        dec ebp
.done:
        and     byte [ebp+ecx+1], 0   ; CF=0
        pop     ebp edi ecx
        ret
.unicode_short:
        mov     ecx, 8
        push    ecx
@@:
        mov     al, [edi]
        inc     edi
        call    ansi2uni_char
        mov     [ebp], ax
        inc     ebp
        inc     ebp
        loop    @b
        pop     ecx
@@:
        cmp     word [ebp-2], ' '
        jnz     @f
        dec     ebp
        dec     ebp
        loop    @b
@@:
        mov     word [ebp], '.'
        inc     ebp
        inc     ebp
        mov     ecx, 3
        push    ecx
@@:
        mov     al, [edi]
        inc     edi
        call    ansi2uni_char
        mov     [ebp], ax
        inc     ebp
        inc     ebp
        loop    @b
        pop     ecx
@@:
        cmp     word [ebp-2], ' '
        jnz     @f
        dec     ebp
        dec     ebp
        loop    @b
        dec     ebp
        dec     ebp
@@:
        and     word [ebp], 0	; CF=0
        pop     ebp edi ecx
        ret
.longname:
; LFN
        mov     al, byte [edi]
        and     eax, 0x3F
        dec     eax
        cmp     al, 20
        jae     .no     ; ignore invalid entries
        mov     word [ebp+260*2], 0     ; force null-terminating for orphans
        imul    eax, 13*2
        add     ebp, eax
        test    byte [edi], 0x40
        jz      @f
        mov     word [ebp+13*2], 0
@@:
        push    eax
; now copy name from edi to ebp ...
        mov     eax, [edi+1]
        mov     [ebp], eax      ; symbols 1,2
        mov     eax, [edi+5]
        mov     [ebp+4], eax	; 3,4
        mov     eax, [edi+9]
        mov     [ebp+8], ax	; 5
        mov     eax, [edi+14]
        mov     [ebp+10], eax	; 6,7
        mov     eax, [edi+18]
        mov     [ebp+14], eax	; 8,9
        mov     eax, [edi+22]
        mov     [ebp+18], eax	; 10,11
        mov     eax, [edi+28]
        mov     [ebp+22], eax	; 12,13
; ... done
        pop     eax
        sub     ebp, eax
        test    eax, eax
        jz      @f
; if this is not first entry, more processing required
        stc
        ret
@@:
; if this is first entry:
        test    byte [ebp-4], 1
        jnz     .ret
; buffer at ebp contains UNICODE name, convert it to ANSI
        push    esi edi
        mov     esi, ebp
        mov     edi, ebp
        call    uni2ansi_str
        pop     edi esi
.ret:
        clc
        ret

fat_compare_name:
; compares ASCIIZ-names, case-insensitive (cp866 encoding)
; in: esi->name, ebp->name
; out: if names match: ZF=1 and esi->next component of name
;      else: ZF=0, esi is not changed
; destroys eax
        push    ebp esi
.loop:
        mov     al, [ebp]
        inc     ebp
        call    char_toupper
        push    eax
        lodsb
        call    char_toupper
        cmp     al, [esp]
        jnz     .done
        pop     eax
        test    al, al
        jnz     .loop
        dec     esi
        pop     eax
        pop     ebp
        xor     eax, eax	; set ZF flag
        ret
.done:
        cmp     al, '/'
        jnz     @f
        cmp     byte [esp], 0
        jnz     @f
        mov     [esp+4], esi
@@:
        pop     eax
        pop     esi ebp
        ret

fat_time_to_bdfe:
; in: eax=FAT time
; out: eax=BDFE time
        push    ecx edx
        mov     ecx, eax
        mov     edx, eax
        shr     eax, 11
        shl     eax, 16	; hours
        and     edx, 0x1F
        add     edx, edx
        mov     al, dl	; seconds
        shr     ecx, 5
        and     ecx, 0x3F
        mov     ah, cl	; minutes
        pop     edx ecx
        ret

fat_date_to_bdfe:
        push    ecx edx
        mov     ecx, eax
        mov     edx, eax
        shr     eax, 9
        add     ax, 1980
        shl     eax, 16	; year
        and     edx, 0x1F
        mov     al, dl	; day
        shr     ecx, 5
        and     ecx, 0xF
        mov     ah, cl	; month
        pop     edx ecx
        ret

bdfe_to_fat_time:
        push    edx
        mov     edx, eax
        shr     eax, 16
        and     dh, 0x3F
        shl     eax, 6
        or      al, dh
        shr     dl, 1
        and     dl, 0x1F
        shl     eax, 5
        or      al, dl
        pop     edx
        ret

bdfe_to_fat_date:
        push    edx
        mov     edx, eax
        shr     eax, 16
        sub     ax, 1980
        and     dh, 0xF
        shl     eax, 4
        or      al, dh
        and     dl, 0x1F
        shl     eax, 5
        or      al, dl
        pop     edx
        ret

fat_entry_to_bdfe:
; convert FAT entry at edi to BDFE (block of data of folder entry) at esi, advance esi
; destroys eax
        mov     eax, [ebp-4]
        mov     [esi+4], eax	; ASCII/UNICODE name
fat_entry_to_bdfe2:
        movzx   eax, byte [edi+11]
        mov     [esi], eax	; attributes
        movzx   eax, word [edi+14]
        call    fat_time_to_bdfe
        mov     [esi+8], eax	; creation time
        movzx   eax, word [edi+16]
        call    fat_date_to_bdfe
        mov     [esi+12], eax	; creation date
        and     dword [esi+16], 0	; last access time is not supported on FAT
        movzx   eax, word [edi+18]
        call    fat_date_to_bdfe
        mov     [esi+20], eax	; last access date
        movzx   eax, word [edi+22]
        call    fat_time_to_bdfe
        mov     [esi+24], eax	; last write time
        movzx   eax, word [edi+24]
        call    fat_date_to_bdfe
        mov     [esi+28], eax	; last write date
        mov     eax, [edi+28]
        mov     [esi+32], eax	; file size (low dword)
        xor     eax, eax
        mov     [esi+36], eax	; file size (high dword)
        test    ebp, ebp
        jz      .ret
        push    ecx edi
        lea     edi, [esi+40]
        mov     esi, ebp
        test    byte [esi-4], 1
        jz      .ansi
        mov     ecx, 260/2
        rep     movsd
        mov     [edi-2], ax
@@:
        mov     esi, edi
        pop     edi ecx
.ret:
        ret
.ansi:
        mov     ecx, 264/4
        rep     movsd
        mov     [edi-1], al
        jmp     @b

bdfe_to_fat_entry:
; convert BDFE at edx to FAT entry at edi
; destroys eax
; attributes byte
        test    byte [edi+11], 8        ; volume label?
        jnz     @f
        mov     al, [edx]
        and     al, 0x27
        and     byte [edi+11], 0x10
        or      byte [edi+11], al
@@:
        mov     eax, [edx+8]
        call    bdfe_to_fat_time
        mov     [edi+14], ax            ; creation time
        mov     eax, [edx+12]
        call    bdfe_to_fat_date
        mov     [edi+16], ax            ; creation date
        mov     eax, [edx+20]
        call    bdfe_to_fat_date
        mov     [edi+18], ax            ; last access date
        mov     eax, [edx+24]
        call    bdfe_to_fat_time
        mov     [edi+22], ax            ; last write time
        mov     eax, [edx+28]
        call    bdfe_to_fat_date
        mov     [edi+24], ax            ; last write date
        ret

ramdisk_root_first:
        mov     edi, 0x100000+512*19
        clc
        ret
ramdisk_root_next:
        add     edi, 0x20
        cmp     edi, 0x100000+512*33
        cmc
        ret

ramdisk_root_extend_dir:
        stc
        ret

uglobal
; this is for delete support
rd_prev_sector          dd      ?
rd_prev_prev_sector     dd      ?
endg

ramdisk_notroot_next:
        add     edi, 0x20
        test    edi, 0x1FF
        jz      ramdisk_notroot_next_sector
        ret     ; CF=0
ramdisk_notroot_next_sector:
        push    ecx
        mov     ecx, [eax]
        push    [rd_prev_sector]
        pop     [rd_prev_prev_sector]
        mov     [rd_prev_sector], ecx
        mov     ecx, [ecx*2+0x280000]
        and     ecx, 0xFFF
        cmp     ecx, 2849
        jae     ramdisk_notroot_first.err2
        mov     [eax], ecx
        pop     ecx
ramdisk_notroot_first:
        mov     eax, [eax]
        cmp     eax, 2
        jb      .err
        cmp     eax, 2849
        jae     .err
        shl     eax, 9
        lea     edi, [eax+(31 shl 9)+0x100000]
        clc
        ret
.err2:
        pop     ecx
.err:
        stc
        ret
ramdisk_notroot_next_write:
        test    edi, 0x1FF
        jz      ramdisk_notroot_next_sector
ramdisk_root_next_write:
        ret

ramdisk_notroot_extend_dir:
        pusha
        xor     eax, eax
        mov     edi, 0x280000
        mov     ecx, 2849
        repnz   scasw
        jnz     .notfound
        mov     word [edi-2], 0xFFF
        sub     edi, 0x280000
        shr     edi, 1
        dec     edi
        mov     eax, [esp+28]
        mov     ecx, [eax]
        mov     [0x280000+ecx*2], di
        mov     [eax], edi
        shl     edi, 9
        add     edi, (31 shl 9)+0x100000
        mov     [esp], edi
        xor     eax, eax
        mov     ecx, 128
        rep     stosd
        popa
        clc
        ret
.notfound:
        popa
        stc
        ret

rd_find_lfn:
; in: esi->name
; out: CF=1 - file not found
;      else CF=0 and edi->direntry
        push    esi edi
        push    0
        push    ramdisk_root_first
        push    ramdisk_root_next
.loop:
        call    fat_find_lfn
        jc      .notfound
        cmp     byte [esi], 0
        jz      .found
        test    byte [edi+11], 10h
        jz      .notfound
        movzx   eax, word [edi+26]
        mov     [esp+8], eax
        mov     dword [esp+4], ramdisk_notroot_first
        mov     dword [esp], ramdisk_notroot_next
        jmp     .loop
.notfound:
        add     esp, 12
        pop     edi esi
        stc
        ret
.found:
        mov     eax, [esp+8]
        add     esp, 16         ; CF=0
        pop     esi
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskRead - LFN variant for reading sys floppy
;
;  esi  points to filename
;  ebx  pointer to 64-bit number = first wanted byte, 0+
;       may be ebx=0 - start from first byte
;  ecx  number of bytes to read, 0+
;  edx  mem location to return data
;
;  ret ebx = bytes read or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
fs_RamdiskRead:
        cmp     byte [esi], 0
        jnz     @f
        or      ebx, -1
        mov     eax, 10         ; access denied
        ret
@@:
        push    edi
        call    rd_find_lfn
        jnc     .found
        pop     edi
        or      ebx, -1
        mov     eax, 5          ; file not found
        ret
.found:
        test    ebx, ebx
        jz      .l1
        cmp     dword [ebx+4], 0
        jz      @f
        xor     ebx, ebx
.reteof:
        mov     eax, 6          ; EOF
        pop     edi
        ret
@@:
        mov     ebx, [ebx]
.l1:
        push    ecx edx
        push    0
        mov     eax, [edi+28]
        sub     eax, ebx
        jb      .eof
        cmp     eax, ecx
        jae     @f
        mov     ecx, eax
        mov     byte [esp], 6           ; EOF
@@:
        movzx   edi, word [edi+26]	; cluster
.new:
        jecxz   .done
        test    edi, edi
        jz      .eof
        cmp     edi, 0xFF8
        jae     .eof
        lea     eax, [edi+31]           ; bootsector+2*fat+filenames
        shl     eax, 9                  ; *512
        add     eax, 0x100000           ; image base
; now eax points to data of cluster
        sub     ebx, 512
        jae     .skip
        lea     eax, [eax+ebx+512]
        neg     ebx
        push    ecx
        cmp     ecx, ebx
        jbe     @f
        mov     ecx, ebx
@@:
        mov     ebx, edx
        call    memmove
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        xor     ebx, ebx
.skip:
        movzx   edi, word [edi*2+0x280000]      ; find next cluster from FAT
        jmp     .new
.eof:
        mov     ebx, edx
        pop     eax edx ecx
        sub     ebx, edx
        jmp     .reteof
.done:
        mov     ebx, edx
        pop     eax edx ecx edi
        sub     ebx, edx
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskReadFolder - LFN variant for reading sys floppy folder
;
;  esi  points to filename; only root is folder on ramdisk
;  ebx  pointer to structure 32-bit number = first wanted block
;                          & flags (bitfields)
; flags: bit 0: 0=ANSI names, 1=UNICODE names
;  ecx  number of blocks to read, 0+
;  edx  mem location to return data
;
;  ret ebx = size or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
fs_RamdiskReadFolder:
        push    edi
        cmp     byte [esi], 0
        jz      .root
        call    rd_find_lfn
        jnc     .found
        pop     edi
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
.found:
        test    byte [edi+11], 0x10
        jnz     .found_dir
        pop     edi
        or      ebx, -1
        mov     eax, ERROR_ACCESS_DENIED
        ret
.found_dir:
        movzx   eax, word [edi+26]
        add     eax, 31
        push    0
        jmp     .doit
.root:
        mov     eax, 19
        push    14
.doit:
        push    esi ecx ebp
        sub     esp, 262*2      ; reserve space for LFN
        mov     ebp, esp
        push    dword [ebx+4]   ; for fat_get_name: read ANSI/UNICODE names
        mov     ebx, [ebx]
; init header
        push    eax ecx
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep     stosd
        mov     byte [edx], 1   ; version
        pop     ecx eax
        mov     esi, edi        ; esi points to block of data of folder entry (BDFE)
.main_loop:
        mov     edi, eax
        shl     edi, 9
        add     edi, 0x100000
        push    eax
.l1:
        call    fat_get_name
        jc      .l2
        cmp     byte [edi+11], 0xF
        jnz     .do_bdfe
        add     edi, 0x20
        test    edi, 0x1FF
        jnz     .do_bdfe
        pop     eax
        inc     eax
        dec     byte [esp+262*2+16]
        jz      .done
        jns     @f
; read next sector from FAT
        mov     eax, [(eax-31-1)*2+0x280000]
        and     eax, 0xFFF
        cmp     eax, 0xFF8
        jae     .done
        add     eax, 31
        mov     byte [esp+262*2+16], 0
@@:
        mov     edi, eax
        shl     edi, 9
        add     edi, 0x100000
        push    eax
.do_bdfe:
        inc     dword [edx+8]   ; new file found
        dec     ebx
        jns     .l2
        dec     ecx
        js      .l2
        inc     dword [edx+4]  ; new file block copied
        call    fat_entry_to_bdfe
.l2:
        add     edi, 0x20
        test    edi, 0x1FF
        jnz     .l1
        pop     eax
        inc     eax
        dec     byte [esp+262*2+16]
        jz      .done
        jns     @f
; read next sector from FAT
        mov     eax, [(eax-31-1)*2+0x280000]
        and     eax, 0xFFF
        cmp     eax, 0xFF8
        jae     .done
        add     eax, 31
        mov     byte [esp+262*2+16], 0
@@:
        jmp     .main_loop
.done:
        add     esp, 262*2+4
        pop     ebp
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        pop     ecx esi edi edi
        ret

iglobal
label fat_legal_chars byte
; 0 = not allowed
; 1 = allowed only in long names
; 3 = allowed
        times 32 db 0
;                 ! " # $ % & ' ( ) * + , - . /
        db      1,3,0,3,3,3,3,3,3,3,0,1,1,3,3,0
;               0 1 2 3 4 5 6 7 8 9 : ; < = > ?
        db      3,3,3,3,3,3,3,3,3,3,0,1,0,1,0,0
;               @ A B C D E F G H I J K L M N O
        db      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
;               P Q R S T U V W X Y Z [ \ ] ^ _
        db      3,3,3,3,3,3,3,3,3,3,3,1,0,1,3,3
;               ` a b c d e f g h i j k l m n o
        db      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
;               p q r s t u v w x y z { | } ~
        db      3,3,3,3,3,3,3,3,3,3,3,3,0,3,3,0
endg

fat_name_is_legal:
; in: esi->(long) name
; out: CF set <=> legal
; destroys eax
        push    esi
        xor     eax, eax
@@:
        lodsb
        test    al, al
        jz      .done
        cmp     al, 80h
        jae     .big
        test    [fat_legal_chars+eax], 1
        jnz     @b
.err:
        pop     esi
        clc
        ret
.big:
; 0x80-0xAF, 0xE0-0xEF
        cmp     al, 0xB0
        jb      @b
        cmp     al, 0xE0
        jb      .err
        cmp     al, 0xF0
        jb      @b
        jmp     .err
.done:
        sub     esi, [esp]
        cmp     esi, 257
        pop     esi
        ret

fat_next_short_name:
; in: edi->8+3 name
; out: name corrected
;      CF=1 <=> error
        pushad
        mov     ecx, 8
        mov     al, '~'
        std
        push    edi
        add     edi, 7
        repnz   scasb
        pop     edi
        cld
        jz      .tilde
; tilde is not found, insert "~1" at end
        add     edi, 6
        cmp     word [edi], '  '
        jnz     .insert_tilde
@@:     dec     edi
        cmp     byte [edi], ' '
        jz      @b
        inc     edi
.insert_tilde:
        mov     word [edi], '~1'
        popad
;       clc     ; CF already cleared
        ret
.tilde:
        push    edi
        add     edi, 7
        xor     ecx, ecx
@@:
; after tilde may be only digits and trailing spaces
        cmp     byte [edi], '~'
        jz      .break
        cmp     byte [edi], ' '
        jz      .space
        cmp     byte [edi], '9'
        jnz     .found
        dec     edi
        jmp     @b
.space:
        dec     edi
        inc     ecx
        jmp     @b
.found:
        inc     byte [edi]
.succ:
        pop     edi
        popad
        clc
        ret
.break:
        jecxz   .noplace
        inc     edi
        mov     al, '1'
@@:
        xchg    al, [edi]
        inc     edi
        cmp     al, ' '
        mov     al, '0'
        jnz     @b
        jmp     .succ
.noplace:
        dec     edi
        cmp     edi, [esp]
        jz      .err
        add     dword [esp], 8
        mov     word [edi], '~1'
        inc     edi
        inc     edi
@@:
        mov     byte [edi], '0'
        inc     edi
        cmp     edi, [esp]
        jb      @b
        pop     edi
        popad
        ;clc    ; automatically
        ret
.err:
        pop     edi
        popad
        stc
        ret

fat_gen_short_name:
; in: esi->long name
;     edi->buffer (8+3=11 chars)
; out: buffer filled
        pushad
        mov     eax, '    '
        push    edi
        stosd
        stosd
        stosd
        pop     edi
        xor     eax, eax
        push    8
        pop     ebx
        lea     ecx, [edi+8]
.loop:
        lodsb
        test    al, al
        jz      .done
        call    char_toupper
        cmp     al, ' '
        jz      .space
        cmp     al, 80h
        ja      .big
        test    [fat_legal_chars+eax], 2
        jnz     .symbol
.inv_symbol:
        mov     al, '_'
        or      bh, 1
.symbol:
        cmp     al, '.'
        jz      .dot
.normal_symbol:
        dec     bl
        jns     .store
        mov     bl, 0
.space:
        or      bh, 1
        jmp     .loop
.store:
        stosb
        jmp     .loop
.big:
        cmp     al, 0xB0
        jb      .normal_symbol
        cmp     al, 0xE0
        jb      .inv_symbol
        cmp     al, 0xF0
        jb      .normal_symbol
        jmp     .inv_symbol
.dot:
        test    bh, 2
        jz      .firstdot
        pop     ebx
        add     ebx, edi
        sub     ebx, ecx
        push    ebx
        cmp     edi, ecx
        jbe     .skip
@@:
        dec     edi
        mov     al, ' '
        xchg    al, [edi]
        dec     ebx
        mov     [ebx], al
        cmp     edi, ecx
        ja      @b
.skip:
        mov     bh, 3
        jmp     @f
.firstdot:
        cmp     bl, 8
        jz      .space
        push    edi
        or      bh, 2
@@:
        mov     edi, ecx
        mov     bl, 3
        jmp     .loop
.done:
        test    bh, 2
        jz      @f
        pop     edi
@@:
        lea     edi, [ecx-8]
        test    bh, 1
        jz      @f
        call    fat_next_short_name
@@:
        popad
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskRewrite - LFN variant for writing ramdisk
;  fs_RamdiskCreateFolder - create folder on ramdisk
;
;  esi  points to file/folder name
;  ebx  ignored (reserved)
;  ecx  number of bytes to write, 0+ (ignored for folders)
;  edx  mem location to data (ignored for folders)
;
;  ret ebx = number of written bytes
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
@@:
        mov     eax, ERROR_ACCESS_DENIED
        xor     ebx, ebx
        ret

fs_RamdiskCreateFolder:
        mov     al, 1           ; create folder
        jmp     fs_RamdiskRewrite.common

fs_RamdiskRewrite:
        xor     eax, eax        ; create file
.common:
        cmp     byte [esi], 0
        jz      @b
        pushad
        xor     ebp, ebp
        push    esi
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        lea     ebp, [esi-1]
        jmp     @b
@@:
        pop     esi
        test    ebp, ebp
        jnz     .noroot
        push    ramdisk_root_extend_dir
        push    ramdisk_root_next_write
        push    ebp
        push    ramdisk_root_first
        push    ramdisk_root_next
        jmp     .common1
.noroot:
; check existence
        mov     byte [ebp], 0
        call    rd_find_lfn
        mov     byte [ebp], '/'
        lea     esi, [ebp+1]
        jnc     @f
        mov     eax, ERROR_FILE_NOT_FOUND
.ret1:
        mov     [esp+28], eax
        popad
        xor     ebx, ebx
        ret
@@:
        test    byte [edi+11], 0x10     ; must be directory
        mov     eax, ERROR_ACCESS_DENIED
        jz      .ret1
        movzx   ebp, word [edi+26]      ; ebp=cluster
        mov     eax, ERROR_FAT_TABLE
        cmp     ebp, 2
        jb      .ret1
        cmp     ebp, 2849
        jae     .ret1
        push    ramdisk_notroot_extend_dir
        push    ramdisk_notroot_next_write
        push    ebp
        push    ramdisk_notroot_first
        push    ramdisk_notroot_next
.common1:
        call    fat_find_lfn
        jc      .notfound
; found
        test    byte [edi+11], 10h
        jz      .exists_file
; found directory; if we are creating directory, return OK,
;                  if we are creating file, say "access denied"
        add     esp, 20
        popad
        test    al, al
        mov     eax, ERROR_ACCESS_DENIED
        jz      @f
        mov     al, 0
@@:
        xor     ebx, ebx
        ret
.exists_file:
; found file; if we are creating directory, return "access denied",
;             if we are creating file, delete existing file and continue
        cmp     byte [esp+20+28], 0
        jz      @f
        add     esp, 20
        popad
        mov     eax, ERROR_ACCESS_DENIED
        xor     ebx, ebx
        ret
@@:
; delete FAT chain
        push    edi
        xor     eax, eax
        mov     dword [edi+28], eax     ; zero size
        xchg    ax, word [edi+26]       ; start cluster
        test    eax, eax
        jz      .done1
@@:
        cmp     eax, 0xFF8
        jae     .done1
        lea     edi, [RAMDISK_FAT + eax*2] ; position in FAT
        xor     eax, eax
        xchg    ax, [edi]
        jmp     @b
.done1:
        pop     edi
        call    get_time_for_file
        mov     [edi+22], ax
        call    get_date_for_file
        mov     [edi+24], ax
        mov     [edi+18], ax
        or      byte [edi+11], 20h      ; set 'archive' attribute
        jmp     .doit
.notfound:
; file is not found; generate short name
        call    fat_name_is_legal
        jc      @f
        add     esp, 20
        popad
        mov     eax, ERROR_FILE_NOT_FOUND
        xor     ebx, ebx
        ret
@@:
        sub     esp, 12
        mov     edi, esp
        call    fat_gen_short_name
.test_short_name_loop:
        push    esi edi ecx
        mov     esi, edi
        lea     eax, [esp+12+12+8]
        mov     [eax], ebp
        call    dword [eax-4]
        jc      .found
.test_short_name_entry:
        cmp     byte [edi+11], 0xF
        jz      .test_short_name_cont
        mov     ecx, 11
        push    esi edi
        repz    cmpsb
        pop     edi esi
        jz      .short_name_found
.test_short_name_cont:
        lea     eax, [esp+12+12+8]
        call    dword [eax-8]
        jnc     .test_short_name_entry
        jmp     .found
.short_name_found:
        pop     ecx edi esi
        call    fat_next_short_name
        jnc     .test_short_name_loop
.disk_full:
        add     esp, 12+20
        popad
        mov     eax, ERROR_DISK_FULL
        xor     ebx, ebx
        ret
.found:
        pop     ecx edi esi
; now find space in directory
; we need to save LFN <=> LFN is not equal to short name <=> generated name contains '~'
        mov     al, '~'
        push    ecx edi
        mov     ecx, 8
        repnz   scasb
        push    1
        pop     eax     ; 1 entry
        jnz     .notilde
; we need ceil(strlen(esi)/13) additional entries = floor((strlen(esi)+12+13)/13) total
        xor     eax, eax
@@:
        cmp     byte [esi], 0
        jz      @f
        inc     esi
        inc     eax
        jmp     @b
@@:
        sub     esi, eax
        add     eax, 12+13
        mov     ecx, 13
        push    edx
        cdq
        div     ecx
        pop     edx
.notilde:
        push    -1
        push    -1
; find <eax> successive entries in directory
        xor     ecx, ecx
        push    eax
        lea     eax, [esp+12+8+12+8]
        mov     [eax], ebp
        call    dword [eax-4]
        pop     eax
.scan_dir:
        cmp     byte [edi], 0
        jz      .free
        cmp     byte [edi], 0xE5
        jz      .free
        xor     ecx, ecx
.scan_cont:
        push    eax
        lea     eax, [esp+12+8+12+8]
        call    dword [eax-8]
        pop     eax
        jnc     .scan_dir
        push    eax
        lea     eax, [esp+12+8+12+8]
        call    dword [eax+8]           ; extend directory
        pop     eax
        jnc     .scan_dir
        add     esp, 8+8+12+20
        popad
        mov     eax, ERROR_DISK_FULL
        xor     ebx, ebx
        ret
.free:
        test    ecx, ecx
        jnz     @f
        mov     [esp], edi
        mov     ecx, [esp+8+8+12+8]
        mov     [esp+4], ecx
        xor     ecx, ecx
@@:
        inc     ecx
        cmp     ecx, eax
        jb      .scan_cont
; found!
; calculate name checksum
        push    esi ecx
        mov     esi, [esp+8+8]
        mov     ecx, 11
        xor     eax, eax
@@:
        ror     al, 1
        add     al, [esi]
        inc     esi
        loop    @b
        pop     ecx esi
        pop     edi
        pop     dword [esp+8+12+8]
; edi points to last entry in free chunk
        dec     ecx
        jz      .nolfn
        push    esi
        push    eax
        mov     al, 40h
.writelfn:
        or      al, cl
        mov     esi, [esp+4]
        push    ecx
        dec     ecx
        imul    ecx, 13
        add     esi, ecx
        stosb
        mov     cl, 5
        call    .read_symbols
        mov     ax, 0xF
        stosw
        mov     al, [esp+4]
        stosb
        mov     cl, 6
        call    .read_symbols
        xor     eax, eax
        stosw
        mov     cl, 2
        call    .read_symbols
        pop     ecx
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+4]   ; next write
        xor     eax, eax
        loop    .writelfn
        pop     eax
        pop     esi
.nolfn:
        xchg    esi, [esp]
        mov     ecx, 11
        rep     movsb
        mov     word [edi], 20h         ; attributes
        sub     edi, 11
        pop     esi ecx
        add     esp, 12
        mov     byte [edi+13], 0        ; tenths of a second at file creation time
        call    get_time_for_file
        mov     [edi+14], ax            ; creation time
        mov     [edi+22], ax            ; last write time
        call    get_date_for_file
        mov     [edi+16], ax            ; creation date
        mov     [edi+24], ax            ; last write date
        mov     [edi+18], ax            ; last access date
        and     word [edi+20], 0        ; high word of cluster
        and     word [edi+26], 0        ; low word of cluster - to be filled
        and     dword [edi+28], 0       ; file size - to be filled
        cmp     byte [esp+20+28], 0
        jz      .doit
; create directory
        mov     byte [edi+11], 10h         ; attributes: folder
        mov     ecx, 32*2
        mov     edx, edi
.doit:
        push    edx
        push    ecx
        push    edi
        add     edi, 26         ; edi points to low word of cluster
        push    edi
        jecxz   .done
        mov     ecx, 2849
        mov     edi, RAMDISK_FAT
.write_loop:
; allocate new cluster
        xor     eax, eax
        repnz   scasw
        jnz     .disk_full2
        dec     edi
        dec     edi
        lea     eax, [edi-(RAMDISK_FAT)]
        shr     eax, 1                  ; eax = cluster
        mov     word [edi], 0xFFF       ; mark as last cluster
        xchg    edi, [esp]
        stosw
        pop     edi
        push    edi
        inc     ecx
; write data
        cmp     byte [esp+16+20+28], 0
        jnz     .writedir
        shl     eax, 9
        add     eax, RAMDISK+31*512
.writefile:
        mov     ebx, edx
        xchg    eax, ebx
        push    ecx
        mov     ecx, 512
        cmp     dword [esp+12], ecx
        jae     @f
        mov     ecx, [esp+12]
@@:
        call    memmove
        add     edx, ecx
        sub     [esp+12], ecx
        pop     ecx
        jnz     .write_loop
.done:
        mov     ebx, edx
        pop     edi edi ecx edx
        sub     ebx, edx
        mov     [edi+28], ebx
        add     esp, 20
        mov     [esp+16], ebx
        popad
        xor     eax, eax
        ret
.disk_full2:
        mov     ebx, edx
        pop     edi edi ecx edx
        sub     ebx, edx
        mov     [edi+28], ebx
        add     esp, 20
        mov     [esp+16], ebx
        popad
        push    ERROR_DISK_FULL
        pop     eax
        ret
.writedir:
        mov     edi, eax
        shl     edi, 9
        add     edi, RAMDISK+31*512
        mov     esi, edx
        mov     ecx, 32/4
        push    ecx
        rep     movsd
        mov     dword [edi-32], '.   '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        mov     word [edi-32+26], ax
        mov     esi, edx
        pop     ecx
        rep     movsd
        mov     dword [edi-32], '..  '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        mov     eax, [esp+16+8]
        mov     word [edi-32+26], ax
        pop     edi edi ecx edx
        add     esp, 20
        popad
        xor     eax, eax
        xor     ebx, ebx
        ret

.read_symbol:
        or      ax, -1
        test    esi, esi
        jz      .retFFFF
        lodsb
        test    al, al
        jnz     ansi2uni_char
        xor     eax, eax
        xor     esi, esi
.retFFFF:
        ret

.read_symbols:
        call    .read_symbol
        stosw
        loop    .read_symbols
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskWrite - LFN variant for writing to sys floppy
;
;  esi  points to filename
;  ebx  pointer to 64-bit number = first wanted byte, 0+
;       may be ebx=0 - start from first byte
;  ecx  number of bytes to write, 0+
;  edx  mem location to data
;
;  ret ebx = bytes written (maybe 0)
;      eax = 0 ok write or other = errormsg
;
;--------------------------------------------------------------
@@:
        push    ERROR_ACCESS_DENIED
fs_RamdiskWrite.ret0:
        pop     eax
        xor     ebx, ebx
        ret

fs_RamdiskWrite:
        cmp     byte [esi], 0
        jz      @b
        pushad
        call    rd_find_lfn
        jnc     .found
        popad
        push    ERROR_FILE_NOT_FOUND
        jmp     .ret0
.found:
; must not be directory
        test    byte [edi+11], 10h
        jz      @f
        popad
        push    ERROR_ACCESS_DENIED
        jmp     .ret0
@@:
; FAT does not support files larger than 4GB
        test    ebx, ebx
        jz      .l1
        cmp     dword [ebx+4], 0
        jz      @f
.eof:
        popad
        push    ERROR_END_OF_FILE
        jmp     .ret0
@@:
        mov     ebx, [ebx]
.l1:
; now edi points to direntry, ebx=start byte to write,
; ecx=number of bytes to write, edx=data pointer
        call    fat_update_datetime

; extend file if needed
        add     ecx, ebx
        jc      .eof    ; FAT does not support files larger than 4GB
        push    0       ; return value=0
        cmp     ecx, [edi+28]
        jbe     .length_ok
        cmp     ecx, ebx
        jz      .length_ok
        call    ramdisk_extend_file
        jnc     .length_ok
; ramdisk_extend_file can return two error codes: FAT table error or disk full.
; First case is fatal error, in second case we may write some data
        mov     [esp], eax
        cmp     al, ERROR_DISK_FULL
        jz      .disk_full
        pop     eax
        mov     [esp+28], eax
        popad
        xor     ebx, ebx
        ret
.disk_full:
; correct number of bytes to write
        mov     ecx, [edi+28]
        cmp     ecx, ebx
        ja      .length_ok
.ret:
        pop     eax
        mov     [esp+28], eax   ; eax=return value
        sub     edx, [esp+20]
        mov     [esp+16], edx   ; ebx=number of written bytes
        popad
        ret
.length_ok:
; now ebx=start pos, ecx=end pos, both lie inside file
        sub     ecx, ebx
        jz      .ret
        movzx   edi, word [edi+26]      ; starting cluster
.write_loop:
        sub     ebx, 0x200
        jae     .next_cluster
        push    ecx
        neg     ebx
        cmp     ecx, ebx
        jbe     @f
        mov     ecx, ebx
@@:
        mov     eax, edi
        shl     eax, 9
        add     eax, 0x100000+31*512+0x200
        sub     eax, ebx
        mov     ebx, eax
        mov     eax, edx
        call    memmove
        xor     ebx, ebx
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        jz      .ret
.next_cluster:
        movzx   edi, word [edi*2+0x280000]
        jmp     .write_loop

ramdisk_extend_file.zero_size:
        xor     eax, eax
        jmp     ramdisk_extend_file.start_extend

; extends file on ramdisk to given size, new data area is filled by 0
; in: edi->direntry, ecx=new size
; out: CF=0 => OK, eax=0
;      CF=1 => error, eax=code (ERROR_FAT_TABLE or ERROR_DISK_FULL)
ramdisk_extend_file:
        push    ecx
; find the last cluster of file
        movzx   eax, word [edi+26]      ; first cluster
        mov     ecx, [edi+28]
        jecxz   .zero_size
@@:
        sub     ecx, 0x200
        jbe     @f
        mov     eax, [eax*2+0x280000]
        and     eax, 0xFFF
        jz      .fat_err
        cmp     eax, 0xFF8
        jb      @b
.fat_err:
        pop     ecx
        push    ERROR_FAT_TABLE
        pop     eax
        stc
        ret
@@:
        push    eax
        mov     eax, [eax*2+0x280000]
        and     eax, 0xFFF
        cmp     eax, 0xFF8
        pop     eax
        jb      .fat_err
; set length to full number of sectors and make sure that last sector is zero-padded
        sub     [edi+28], ecx
        push    eax edi
        mov     edi, eax
        shl     edi, 9
        lea     edi, [edi+0x100000+31*512+0x200+ecx]
        neg     ecx
        xor     eax, eax
        rep     stosb
        pop     edi eax
.start_extend:
        pop     ecx
; now do extend
        push    edx esi
        mov     esi, 0x280000+2*2       ; start scan from cluster 2
        mov     edx, 2847               ; number of clusters to scan
.extend_loop:
        cmp     [edi+28], ecx
        jae     .extend_done
; add new sector
        push    ecx
        mov     ecx, edx
        push    edi
        mov     edi, esi
        jecxz   .disk_full
        push    eax
        xor     eax, eax
        repnz   scasw
        pop     eax
        jnz     .disk_full
        mov     word [edi-2], 0xFFF
        mov     esi, edi
        mov     edx, ecx
        sub     edi, 0x280000
        shr     edi, 1
        dec     edi     ; now edi=new cluster
        test    eax, eax
        jz      .first_cluster
        mov     [0x280000+eax*2], di
        jmp     @f
.first_cluster:
        pop     eax     ; eax->direntry
        push    eax
        mov     [eax+26], di
@@:
        push    edi
        shl     edi, 9
        add     edi, 0x100000+31*512
        xor     eax, eax
        mov     ecx, 512/4
        rep     stosd
        pop     eax     ; eax=new cluster
        pop     edi     ; edi->direntry
        pop     ecx     ; ecx=required size
        add     dword [edi+28], 0x200
        jmp     .extend_loop
.extend_done:
        mov     [edi+28], ecx
        pop     esi edx
        xor     eax, eax        ; CF=0
        ret
.disk_full:
        pop     edi ecx
        pop     esi edx
        stc
        push    ERROR_DISK_FULL
        pop     eax
        ret

fat_update_datetime:
        call    get_time_for_file
        mov     [edi+22], ax            ; last write time
        call    get_date_for_file
        mov     [edi+24], ax            ; last write date
        mov     [edi+18], ax            ; last access date
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskSetFileEnd - set end of file on ramdisk
;
;  esi  points to filename
;  ebx  points to 64-bit number = new file size
;  ecx  ignored (reserved)
;  edx  ignored (reserved)
;
;  ret eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_RamdiskSetFileEnd:
        cmp     byte [esi], 0
        jnz     @f
.access_denied:
        push    ERROR_ACCESS_DENIED
        jmp     .ret
@@:
        push    edi
        call    rd_find_lfn
        jnc     @f
        pop     edi
        push    ERROR_FILE_NOT_FOUND
.ret:
        pop     eax
        ret
@@:
; must not be directory
        test    byte [edi+11], 10h
        jz      @f
        pop     edi
        jmp     .access_denied
@@:
; file size must not exceed 4Gb
        cmp     dword [ebx+4], 0
        jz      @f
        pop     edi
        push    ERROR_END_OF_FILE
        jmp     .ret
@@:
; set file modification date/time to current
        call    fat_update_datetime
        mov     eax, [ebx]
        cmp     eax, [edi+28]
        jb      .truncate
        ja      .expand
        pop     edi
        xor     eax, eax
        ret
.expand:
        push    ecx
        mov     ecx, eax
        call    ramdisk_extend_file
        pop     ecx
        pop     edi
        ret
.truncate:
        mov     [edi+28], eax
        push    ecx
        movzx   ecx, word [edi+26]
        test    eax, eax
        jz      .zero_size
; find new last sector
@@:
        sub     eax, 0x200
        jbe     @f
        movzx   ecx, word [0x280000+ecx*2]
        jmp     @b
@@:
; zero data at the end of last sector
        push    ecx
        mov     edi, ecx
        shl     edi, 9
        lea     edi, [edi+0x100000+31*512+eax+0x200]
        mov     ecx, eax
        neg     ecx
        xor     eax, eax
        rep     stosb
        pop     ecx
; terminate FAT chain
        lea     ecx, [0x280000+ecx+ecx]
        push    dword [ecx]
        mov     word [ecx], 0xFFF
        pop     ecx
        and     ecx, 0xFFF
        jmp     .delete
.zero_size:
        and     word [edi+26], 0
.delete:
; delete FAT chain starting with ecx
; mark all clusters as free
        cmp     ecx, 0xFF8
        jae     .deleted
        lea     ecx, [0x280000+ecx+ecx]
        push    dword [ecx]
        and     word [ecx], 0
        pop     ecx
        and     ecx, 0xFFF
        jmp     .delete
.deleted:
        pop     ecx
        pop     edi
        xor     eax, eax
        ret

fs_RamdiskGetFileInfo:
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2  ; unsupported
        ret
@@:
        push    edi
        call    rd_find_lfn
fs_GetFileInfo_finish:
        jnc     @f
        pop     edi
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
@@:
        push    esi ebp
        xor     ebp, ebp
        mov     esi, edx
        and     dword [esi+4], 0
        call    fat_entry_to_bdfe2
        pop     ebp esi
        pop     edi
        xor     eax, eax
        ret

fs_RamdiskSetFileInfo:
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2  ; unsupported
        ret
@@:
        push    edi
        call    rd_find_lfn
        jnc     @f
        pop     edi
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
@@:
        call    bdfe_to_fat_entry
        pop     edi
        xor     eax, eax
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskExecute - LFN variant for executing on sys floppy
;
;  esi  points to ramdisk filename (e.g. 'launcher')
;  ebp  points to full filename (e.g. '/rd/1/launcher')
;  dword [ebx] = flags
;  dword [ebx+4] = cmdline
;
;  ret ebx,edx destroyed
;      eax > 0 - PID, < 0 - error
;
;--------------------------------------------------------------
fs_RamdiskExecute:
        mov     edx, [ebx]
        mov     ebx, [ebx+4]
        test    ebx, ebx
        jz      @f
        add     ebx, std_application_base_address
@@:

;----------------------------------------------------------------
;
; fs_RamdiskExecute.flags - second entry
;
;  esi  points to ramdisk filename (kernel address)
;  ebp  points to full filename
;  edx  flags
;  ebx  cmdline (kernel address)
;
;  ret  eax > 0 - PID, < 0 - error
;
;--------------------------------------------------------------

.flags:
        cmp     byte [esi], 0
        jnz     @f
; cannot execute root!
        mov     eax, -ERROR_ACCESS_DENIED
        ret
@@:
        push    edi
        call    rd_find_lfn
        jnc     .found
        pop     edi
        mov     eax, -ERROR_FILE_NOT_FOUND
        ret
.found:
        movzx   eax, word [edi+26]      ; cluster
        push    eax
        push    dword [edi+28]          ; size
        push    .DoRead
        call    fs_execute
        add     esp, 12
        pop     edi
        ret

.DoRead:
; read next block
; in: eax->parameters, edi->buffer
; out: eax = error code
        pushad
        cmp     dword [eax], 0  ; file size
        jz      .eof
        mov     edx, [eax+4]    ; cluster
        lea     esi, [edx+31]
        shl     esi, 9
        add     esi, 0x100000
        mov     ecx, 512/4
        rep     movsd
        mov     ecx, [eax]
        sub     ecx, 512
        jae     @f
        add     edi, ecx
        neg     ecx
        push    eax
        xor     eax, eax
        rep     stosb
        pop     eax
@@:
        mov     [eax], ecx
        mov     dx, [edx*2+0x280000]
        mov     [eax+4], dx     ; high word is already zero
        popad
        xor     eax, eax
        ret
.eof:
        popad
        mov     eax, 6
        ret

;----------------------------------------------------------------
;
;  fs_RamdiskDelete - delete file or empty folder from ramdisk
;
;  esi  points to filename
;
;  ret  eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_RamdiskDelete:
        cmp     byte [esi], 0
        jnz     @f
; cannot delete root!
.access_denied:
        push    ERROR_ACCESS_DENIED
.pop_ret:
        pop     eax
        ret
@@:
        and     [rd_prev_sector], 0
        and     [rd_prev_prev_sector], 0
        push    edi
        call    rd_find_lfn
        jnc     .found
        pop     edi
        push    ERROR_FILE_NOT_FOUND
        jmp     .pop_ret
.found:
        cmp     dword [edi], '.   '
        jz      .access_denied2
        cmp     dword [edi], '..  '
        jz      .access_denied2
        test    byte [edi+11], 10h
        jz      .dodel
; we can delete only empty folders!
        movzx   eax, word [edi+26]
        push    ebx
        mov     ebx, eax
        shl     ebx, 9
        add     ebx, RAMDISK + 31*0x200 + 2*0x20
.checkempty:
        cmp     byte [ebx], 0
        jz      .empty
        cmp     byte [ebx], 0xE5
        jnz     .notempty
        add     ebx, 0x20
        test    ebx, 0x1FF
        jnz     .checkempty
        movzx   eax, word [RAMDISK_FAT + eax*2]
        test    eax, eax
        jz      .empty
        mov     ebx, eax
        shl     ebx, 9
        add     ebx, RAMDISK + 31*0x200
        jmp     .checkempty
.notempty:
        pop     ebx
.access_denied2:
        pop     edi
        jmp     .access_denied
.empty:
        pop     ebx
.dodel:
        movzx   eax, word [edi+26]
; delete folder entry
        mov     byte [edi], 0xE5
; delete LFN (if present)
.lfndel:
        test    edi, 0x1FF
        jnz     @f
        cmp     [rd_prev_sector], 0
        jz      @f
        cmp     [rd_prev_sector], -1
        jz      .lfndone
        mov     edi, [rd_prev_sector]
        push    [rd_prev_prev_sector]
        pop     [rd_prev_sector]
        or      [rd_prev_prev_sector], -1
        shl     edi, 9
        add     edi, RAMDISK + 31*0x200 + 0x200
@@:
        sub     edi, 0x20
        cmp     byte [edi], 0xE5
        jz      .lfndone
        cmp     byte [edi+11], 0xF
        jnz     .lfndone
        mov     byte [edi], 0xE5
        jmp     .lfndel
.lfndone:
; delete FAT chain
        test    eax, eax
        jz      .done
        lea     eax, [RAMDISK_FAT + eax*2]
        push    dword [eax]
        and     word [eax], 0
        pop     eax
        and     eax, 0xFFF
        jmp     .lfndone
.done:
        pop     edi
        xor     eax, eax
        ret

; \end{diamond}
