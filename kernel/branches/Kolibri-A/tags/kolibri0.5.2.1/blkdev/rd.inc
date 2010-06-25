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
   mov dword [edi],eax
   add  edi,4
   mov dword [edi],ebx
   add  edi,4
   mov dword [edi],ecx
   add  edi,4
   mov dword [edi],edx
   add  edi,4   
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
   mov dword [edi],eax
   add  edi,4
   mov word [edi],bx
   add  edi,2
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
        jcxz rdfs2    ;if last cluster=0
        jmp  rdfs1    ;if not last
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
        cmp    [esp+12],dword 0
        je     frnoread
        jmp    frfl8
      frfl7:
        dec    dword [esp+16]
      frfl8:
        shl    edi,1                    ;find next cluster from FAT
        add    edi,0x280000
        movzx  eax,word [edi]
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
        jb     flnsa
        sub    eax,512
        mov    [esp+12],eax
        mov    eax,[esp+16]
        add    eax,512
        mov    [esp+16],eax
        jmp    frnewds

     flnsa:
        dec    edi
        dec    edi
        mov    [edi],word 4095          ; mark end of file - last cluster

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
