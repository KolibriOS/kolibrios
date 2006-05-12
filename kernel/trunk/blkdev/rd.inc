;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; RAMDISK functions                                                    ;;
;; (C) 2004 Ville Turjanmaa, License: GPL                               ;;
;; Addings by M.Lisovin                                                 ;;
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
        test   edx,edx
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

  rd_getfileinfo:
     ;get date, time, size or attributes of file
     ;IN: eax - pointer to file, ebx - type of function: 12-get filesize, 13-get fileattr, 14-get filedate
     ;ecx - filelengh 0=root
     ;OUT: eax=0 - Ok or 5 - file not found ebx - date/time, size or attributes 
        test   ecx,ecx
        jnz    no_getfinfo_root
        mov    eax,5      ;if root - fnf
        xor    ebx,ebx
        dec    ebx
        ret
    no_getfinfo_root:     ;if not root
        sub    esp,32
        call   expand_filename
        call   rd_findfile
        je     fifoundi
        add    esp,32      ;if file not found
        ret
    fifoundi:    
        cmp    ebx,13
        jne    no_rd_attr
        movzx ebx,byte [edi]    ;get attributes
        jmp    rd_getfileinfo_end 
     no_rd_attr:
        cmp    ebx,14
        jne    no_rd_date
        mov    ebx,dword [edi+11] ;get date/time
        jmp    rd_getfileinfo_end
     no_rd_date:
        mov    ebx,dword [edi+17] ;get size
     rd_getfileinfo_end:
        xor    eax,eax
        add    esp,32
        ret

; \begin{diamond}

uni2ansi_str:
; convert UNICODE zero-terminated string to ASCII-string (codepage 866)
; in: esi->source, edi->buffer (may be esi=edi)
; destroys: eax,esi,edi
	lodsw
	test	ax, ax
	jz	.done
	cmp	ax, 0x80
	jb	.ascii
	cmp	ax, 0x401
	jz	.yo1
	cmp	ax, 0x451
	jz	.yo2
	cmp	ax, 0x410
	jb	.unk
	cmp	ax, 0x440
	jb	.rus1
	cmp	ax, 0x450
	jb	.rus2
.unk:
	mov	al, '_'
	jmp	.doit
.yo1:
	mov	al, 'ð'
	jmp	.doit
.yo2:
	mov	al, 'ñ'
	jmp	.doit
.rus1:
; 0x410-0x43F -> 0x80-0xAF
	add	al, 0x70
	jmp	.doit
.rus2:
; 0x440-0x44F -> 0xE0-0xEF
	add	al, 0xA0
.ascii:
.doit:
	stosb
	jmp	uni2ansi_str
.done:
	mov	byte [edi], 0
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
	cmp	al, 'a'
	jb	.ret
	cmp	al, 'z'
	jbe	.az
	cmp	al, ' '
	jb	.ret
	cmp	al, 'à'
	jb	.rus1
	cmp	al, 'ï'
	ja	.ret
; 0xE0-0xEF -> 0x90-0x9F
	sub	al, 'à'-''
.ret:
	ret
.rus1:
; 0xA0-0xAF -> 0x80-0x8F
.az:
	and	al, not 0x20
	ret

fat_get_name:
; in: edi->FAT entry
; out: CF=1 - no valid entry
; else CF=0 and ebp->ASCIIZ-name
; (maximum length of filename is 255 (wide) symbols without trailing 0,
;  but implementation requires buffer 261 words)
; destroys eax
	cmp	byte [edi], 0
	jz	.no
	cmp	byte [edi], 0xE5
	jnz	@f
.no:
	stc
	ret
@@:
	cmp	byte [edi+11], 0xF
	jz	.longname
	push	ecx
	mov	ecx, 8
        push	edi ebp ecx
        test    byte [ebp-4], 1
        jnz     .unicode_short
@@:
	mov	al, [edi]
	inc	edi
	mov	[ebp], al
	inc	ebp
	loop	@b
	pop	ecx
@@:
	cmp	byte [ebp-1], ' '
	jnz	@f
	dec	ebp
	loop	@b
@@:
	mov	byte [ebp], '.'
	inc	ebp
	mov	ecx, 3
	push	ecx
@@:
	mov	al, [edi]
	inc	edi
	mov	[ebp], al
	inc	ebp
	loop	@b
	pop	ecx
@@:
	cmp	byte [ebp-1], ' '
	jnz	@f
	dec	ebp
	loop	@b
	dec	ebp
@@:
	and	byte [ebp], 0	; CF=0
	pop	ebp edi ecx
	ret
.unicode_short:
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
	cmp	word [ebp-2], ' '
	jnz	@f
	dec	ebp
	dec     ebp
	loop	@b
@@:
	mov	word [ebp], '.'
	inc	ebp
	inc     ebp
	mov	ecx, 3
	push	ecx
@@:
	mov	al, [edi]
	inc	edi
	call    ansi2uni_char
	mov	[ebp], ax
        inc     ebp
        inc     ebp
        loop    @b
        pop     ecx
@@:
	cmp	word [ebp-2], ' '
	jnz	@f
	dec	ebp
	dec     ebp
	loop	@b
	dec	ebp
	dec     ebp
@@:
	and	word [ebp], 0	; CF=0
	pop	ebp edi ecx
	ret
.longname:
; LFN
	mov	al, byte [edi]
	and	eax, 0x3F
	dec	eax
	cmp	al, 20
	jae	.no	; ignore invalid entries
	mov	word [ebp+260*2], 0	; force null-terminating for orphans
	imul	eax, 13*2
	add	ebp, eax
	test	byte [edi], 0x40
	jz	@f
	mov	word [ebp+13*2], 0
@@:
	push	eax
; now copy name from edi to ebp ...
	mov	eax, [edi+1]
	mov	[ebp], eax	; symbols 1,2
	mov	eax, [edi+5]
	mov	[ebp+4], eax	; 3,4
	mov	eax, [edi+9]
	mov	[ebp+8], ax	; 5
	mov	eax, [edi+14]
	mov	[ebp+10], eax	; 6,7
	mov	eax, [edi+18]
	mov	[ebp+14], eax	; 8,9
	mov	eax, [edi+22]
	mov	[ebp+18], eax	; 10,11
	mov	eax, [edi+28]
	mov	[ebp+22], eax	; 12,13
; ... done
	pop	eax
	sub	ebp, eax
	test	eax, eax
	jz	@f
; if this is not first entry, more processing required
	stc
	ret
@@:
; if this is first entry:
        test    byte [ebp-4], 1
        jnz     .ret
; buffer at ebp contains UNICODE name, convert it to ANSI
	push	esi edi
	mov	esi, ebp
	mov	edi, ebp
	call	uni2ansi_str
	pop	edi esi
.ret:
	clc
	ret

fat_compare_name:
; compares ASCIIZ-names, case-insensitive (cp866 encoding)
; in: esi->name, ebp->name
; out: if names match: ZF=1 and esi->next component of name
;      else: ZF=0, esi is not changed
; destroys eax
	push	ebp esi
.loop:
	mov	al, [ebp]
	inc	ebp
	call	char_toupper
	push	eax
	lodsb
	call	char_toupper
	cmp	al, [esp]
	jnz	.done
	pop	eax
	test	al, al
	jnz	.loop
	dec	esi
	pop	eax
	pop	ebp
	xor	eax, eax	; set ZF flag
	ret
.done:
	cmp	al, '/'
	jnz	@f
	cmp	byte [esp], 0
	jnz	@f
	mov	[esp+4], esi
@@:
	pop	eax
	pop	esi ebp
	ret

fat_time_to_bdfe:
; in: eax=FAT time
; out: eax=BDFE time
	push	ecx edx
	mov	ecx, eax
	mov	edx, eax
	shr	eax, 11
	shl	eax, 16	; hours
	and	edx, 0x1F
	add	edx, edx
	mov	al, dl	; seconds
	shr	ecx, 5
	and	ecx, 0x3F
	mov	ah, cl	; minutes
	pop	edx ecx
	ret

fat_date_to_bdfe:
	push	ecx edx
	mov	ecx, eax
	mov	edx, eax
	shr	eax, 9
	add	ax, 1980
	shl	eax, 16	; year
	and	edx, 0x1F
	mov	al, dl	; day
	shr	ecx, 5
	and	ecx, 0xF
	mov	ah, cl	; month
	pop	edx ecx
	ret

fat_entry_to_bdfe:
; convert FAT entry at edi to BDFE (block of data of folder entry) at esi, advance esi
; destroys eax
        movzx   eax, byte [edi+11]
        mov     [esi], eax	; attributes
        mov     eax, [ebp-4]
        mov     [esi+4], eax	; ASCII/UNICODE name
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
        ret
.ansi:
        mov     ecx, 264/4
        rep     movsd
        mov     [edi-1], al
        jmp     @b

rd_find_lfn:
; in: esi->name
; out: CF=1 - file not found
;      else CF=0 and edi->direntry
        push    esi ebp edi
        sub     esp, 262*2      ; allocate space for LFN
        mov     ebp, esp        ; ebp points to buffer
        push    0               ; for fat_get_name: read ASCII name
        mov     edi, 0x100000+512*19    ; to root dir
.l1:
        call    fat_get_name
        jc      .l2
        call    fat_compare_name
        jz      .found
.l2:
        add     edi, 0x20
        cmp     edi, 0x100000+512*33
        jb      .l1
.notfound:
        add     esp, 262*2+4
        pop     edi ebp esi
        stc
        ret
.found:
; found
; if this is LFN entry, advance to true entry
        cmp     byte [edi+11], 0xF
        jnz     @f
        add     edi, 0x20
@@:
; folders are not supported
        cmp     byte [esi], 0
        jnz     .notfound
        add     esp, 262*2+4+4  ; CF=0
        pop     ebp esi
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
        cmp     byte [esi], 0
        jz      @f
; ramdisk doesn't support folders
        mov     eax, ERROR_ACCESS_DENIED
        or      ebx, -1
        ret
@@:
        push    esi edi ecx
; init header
        push    ecx
        mov     edi, edx
        mov     ecx, 32/4
        xor     eax, eax
        rep     stosd
        mov     byte [edx], 1   ; version
        pop     ecx
        push    ebp
        sub     esp, 262*2      ; allocate space for LFN
        mov     ebp, esp
        push    dword [ebx+4]   ; for fat_get_name: read ANSI/UNICODE name
        mov     ebx, [ebx]
; read root
        mov     esi, edi        ; esi points to block of data of folder entry (BDFE)
        mov     edi, 0x100000+512*19
.l1:
        call    fat_get_name
        jc      .l2
        cmp     byte [edi+11], 0xF
        jnz     @f
        add     edi, 0x20
@@:
        inc     dword [edx+8]   ; new file found
        dec     ebx
        jns     .l2
        dec     ecx
        js      .l2
        inc     dword [edx+4]  ; new file block copied
        call    fat_entry_to_bdfe
.l2:
        add     edi, 0x20
        cmp     edi, 0x100000+512*33
        jb      .l1
        add     esp, 262*2+4
        pop     ebp
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        pop     ecx edi esi
        ret

; \end{diamond}
