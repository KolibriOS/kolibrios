;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; FAT12.INC                                                            ;;
;; (C) 2005 Mario79, License: GPL                                       ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

n_sector    dd 0  ; temporary save for sector value
flp_status  dd 0
clust_tmp_flp dd 0  ; used by analyze_directory and analyze_directory_to_write
path_pointer_flp dd 0
pointer_file_name_flp dd 0
save_root_flag db 0
save_flag   db 0
root_read   db 0  ; 0-necessary to load root, 1-not to load root
flp_fat     db 0  ; 0-necessary to load fat, 1-not to load fat
flp_number  db 0  ; 1- Floppy A, 2-Floppy B
old_track   db 0  ; old value track
flp_label   rb 15 ; Label and ID of inserted floppy disk

reserve_flp:

    cli
    cmp   [flp_status],0
    je    reserve_flp_ok

    sti
    call  change_task
    jmp   reserve_flp

  reserve_flp_ok:

    push  eax
    mov   eax,[0x3000]
    shl   eax,5
    mov   eax,[eax+0x3000+4]
    mov   [flp_status],eax
    pop   eax
    sti
    ret

floppy_free_space:
;---------------------------------------------
;
; returns free space in edi
;
;---------------------------------------------
    push   eax ebx ecx
    call   read_flp_fat
    cmp    [FDC_Status],0
    jne    fdc_status_error_2
    mov    eax,0x282000
    xor    edi,edi
    mov    ecx,2847   ;1448000/512
rdfs1_1:
    mov    ebx,[eax]
    and    ebx,4095
    cmp    ebx,0
    jne    rdfs2_1
    add    edi,512
rdfs2_1:
    add    eax,2
    loop   rdfs1_1
fdc_status_error_2:
    pop    ecx ebx eax
    ret
    

   

floppy_fileread:
;----------------------------------------------------------------
;
;  fileread - sys floppy
;
;  eax  points to filename 11 chars  - for root directory
;  ebx  first wanted block       ; 1+ ; if 0 then set to 1
;  ecx  number of blocks to read ; 1+ ; if 0 then set to 1
;  edx  mem location to return data
;  esi  length of filename 12*X
;  edi  pointer to path   /fd/1/......  - for all files in nested directories
;
;  ret ebx = size or 0xffffffff file not found
;      eax = 0 ok read or other = errormsg
;            10 = access denied
;--------------------------------------------------------------

    mov    [save_flag],0
    mov    [path_pointer_flp],edi
    cmp    esi,0           ; return ramdisk root
    jne    fr_noroot_1
    cmp    ebx,224/16
    jbe    fr_do_1
    mov    eax,5
    mov    ebx,0
    mov   [flp_status],0
    ret

fr_do_1:
    push ebx ecx edx
    call  read_flp_root
    pop edx ecx ebx
    cmp    [FDC_Status],0
    jne    fdc_status_error_1
    mov    edi,edx
    dec    ebx
    shl    ebx,9
    mov    esi,0x8000
    add    esi,ebx
    shl    ecx,9
    cld
    rep    movsb
    mov    eax,0 ; ok read
    mov    ebx,0
    mov   [flp_status],0
    ret
fdc_status_error_1:
    mov   [flp_status],0
    mov    eax,10 
    mov    ebx,-1
    ret

fr_noroot_1:
    sub    esp,32
    call   expand_filename
frfloppy_1:
    cmp    ebx,0
    jne    frfl5_1
    mov    ebx,1
frfl5_1:
    cmp    ecx,0
    jne    frfl6_1
    mov    ecx,1
frfl6_1:
    dec    ebx
    push   eax
    push   eax ebx ecx edx esi edi
    call   read_flp_fat
    cmp    [FDC_Status],0
    jne    fdc_status_error_3_1
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],1      ; Сторона
    mov    [FDD_Sector],2      ; Сектор
    call    SeekTrack
    mov     dh,14
l.20_1:
    call    ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    fdc_status_error_3_1
    mov     dl,16
    mov     edi,0xD000
    inc     [FDD_Sector]
l.21_1:
    mov    esi,eax            ;Name of file we want
    mov    ecx,11
    cld
    rep    cmpsb            ;Found the file?
    je     fifound_1          ;Yes
    add    ecx,21
    add    edi, ecx         ;Advance to next entry
    dec    dl
    cmp    dl,0
    jne    l.21_1
    dec    dh
    cmp    dh,0
    jne    l.20_1
fdc_status_error_3:
    mov    eax,5            ; file not found ?
    mov    ebx,-1
    add    esp,32+28
    mov   [flp_status],0
    ret
fdc_status_error_3_2:
    cmp    [FDC_Status],0
    je    fdc_status_error_3
fdc_status_error_3_1:
    add    esp,32+28
    jmp    fdc_status_error_1

fifound_1:
    mov    eax,[path_pointer_flp]
    cmp    [eax+36],byte 0
    je     fifound_2
    add    edi,0xf
    mov    eax,[edi]
    and    eax,65535
    mov    ebx,[path_pointer_flp]
    add    ebx,36
    call   get_cluster_of_a_path_flp
    jc     fdc_status_error_3_2
    mov    ebx,[ebx-11+28]        ;file size
    mov    [esp+20],ebx
    mov    [esp+24],ebx
    jmp     fifound_3
fifound_2:
    mov    ebx,[edi-11+28]        ;file size
    mov    [esp+20],ebx
    mov    [esp+24],ebx
    add    edi,0xf
    mov    eax,[edi]
fifound_3:
    and    eax,65535
    mov    [n_sector],eax            ;eax=cluster
frnew_1:
    add    eax,31            ;bootsector+2*fat+filenames
    cmp    [esp+16],dword 0     ; wanted cluster ?
    jne    frfl7_1
    call   read_chs_sector
    cmp    [FDC_Status],0
    jne    fdc_status_error_5
    mov    edi,[esp+8]
    call    give_back_application_data_1
    add    [esp+8],dword 512
    dec    dword [esp+12]        ; last wanted cluster ?
    cmp    [esp+12],dword 0
    je     frnoread_1
    jmp    frfl8_1
frfl7_1:
    dec    dword [esp+16]
frfl8_1:
    mov    edi,[n_sector]
    shl    edi,1            ;find next cluster from FAT
    add    edi,0x282000
    mov    eax,[edi]
    and    eax,4095
    mov    edi,eax
    mov    [n_sector],edi
    cmp    edi,4095         ;eof  - cluster
    jz     frnoread2_1
    cmp    [esp+24],dword 512    ;eof  - size
    jb     frnoread_1
    sub    [esp+24],dword 512
    jmp    frnew_1

read_chs_sector:    
    call    calculate_chs
    call    ReadSectWithRetr
    ret

frnoread2_1:
    cmp    [esp+16],dword 0     ; eof without read ?
    je     frnoread_1
    mov    [fdc_irq_func],fdc_null
    pop    edi esi edx ecx
    add    esp,4
    pop    ebx     ; ebx <- eax : size of file
    add    esp,36
    mov    eax,6   ; end of file
    mov    [flp_status],0
    ret

frnoread_1:
    pop    edi esi edx ecx
    add    esp,4
    pop    ebx     ; ebx <- eax : size of file
    add    esp,36
    mov    eax,0
    mov    [flp_status],0
    ret

fdc_status_error_5:
    pop    edi esi edx ecx
    add    esp,4
    pop    ebx     ; ebx <- eax : size of file
    add    esp,36
    jmp    fdc_status_error_1

read_flp_root:
    pusha
    call  check_label
    cmp    [FDC_Status],0
    jne    unnecessary_root_read
    cmp   [root_read],1
    je    unnecessary_root_read
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],1      ; Сторона
    mov    [FDD_Sector],2      ; Сектор    
    mov    edi,0x8000
    call   SeekTrack
read_flp_root_1:
    call   ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    unnecessary_root_read
    push   edi
    call   give_back_application_data_1
    pop    edi
    add    edi,512
    inc    [FDD_Sector]
    cmp    [FDD_Sector],16
    jne    read_flp_root_1
    mov    [root_read],1
unnecessary_root_read:
    popa
    ret


read_flp_fat:
    pusha
    call  check_label
    cmp    [FDC_Status],0
    jne    unnecessary_flp_fat
    cmp   [flp_fat],1
    je    unnecessary_flp_fat
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],0      ; Сторона
    mov    [FDD_Sector],2      ; Сектор    
    mov    edi,0x8000
    call   SeekTrack
read_flp_fat_1:
    call   ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    unnecessary_flp_fat
    push   edi
    call   give_back_application_data_1
    pop    edi
    add    edi,512
    inc    [FDD_Sector]
    cmp    [FDD_Sector],19
    jne    read_flp_fat_1
    mov    [FDD_Sector],1
    mov    [FDD_Head],1
    call   ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    unnecessary_flp_fat
    call   give_back_application_data_1
    call   calculatefatchain_flp
    mov    [root_read],0
    mov    [flp_fat],1
unnecessary_flp_fat:
    popa
    ret

calculatefatchain_flp:
   pushad

   mov  esi,0x8000
   mov  edi,0x282000

 fcnew_1:
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

   cmp  edi,0x282000+2856*2   ;2849 clusters
   jnz  fcnew_1

   popad
   ret
  
check_label:
    pushad
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],0      ; Сторона
    mov    [FDD_Sector],1      ; Сектор   
    call   SetUserInterrupts
    call   FDDMotorON
    call   RecalibrateFDD
    cmp    [FDC_Status],0
    jne    fdc_status_error
    call   SeekTrack
    cmp    [FDC_Status],0
    jne    fdc_status_error
    call   ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    fdc_status_error    
    mov    esi,flp_label
    mov    edi,0xD000+39
    mov    ecx,15
    cld
    rep    cmpsb    
    je     same_label
    mov    [root_read],0
    mov    [flp_fat],0
same_label:
    mov    esi,0xD000+39
    mov    edi,flp_label
    mov    ecx,15
    cld
    rep    movsb
    popad
    ret
fdc_status_error:
    popad
    ret

save_flp_root:
    pusha
    call  check_label
    cmp    [FDC_Status],0
    jne    unnecessary_root_save
    cmp   [root_read],0
    je    unnecessary_root_save
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],1      ; Сторона
    mov    [FDD_Sector],2      ; Сектор    
    mov    esi,0x8000
    call   SeekTrack
save_flp_root_1:
    push   esi
    call   take_data_from_application_1
    pop    esi
    add    esi,512
    call   WriteSectWithRetr
    cmp    [FDC_Status],0
    jne    unnecessary_root_save
    inc    [FDD_Sector]
    cmp    [FDD_Sector],16
    jne    save_flp_root_1
unnecessary_root_save:
    mov    [fdc_irq_func],fdc_null
    popa
    ret
    
save_flp_fat:
    pusha
    call  check_label
    cmp    [FDC_Status],0
    jne    unnecessary_flp_fat_save
    cmp   [flp_fat],0
    je    unnecessary_flp_fat_save
    call   restorefatchain_flp
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],0      ; Сторона
    mov    [FDD_Sector],2      ; Сектор    
    mov    esi,0x8000
    call   SeekTrack
save_flp_fat_1:
    push   esi
    call   take_data_from_application_1
    pop    esi
    add    esi,512
    call   WriteSectWithRetr
    cmp    [FDC_Status],0
    jne    unnecessary_flp_fat_save
    inc    [FDD_Sector]
    cmp    [FDD_Sector],19
    jne    save_flp_fat_1
    mov    [FDD_Sector],1
    mov    [FDD_Head],1
    call   take_data_from_application_1
    call   WriteSectWithRetr
    cmp    [FDC_Status],0
    jne    unnecessary_flp_fat_save
    mov    [root_read],0
unnecessary_flp_fat_save:
    mov    [fdc_irq_func],fdc_null
    popa
    ret

    
restorefatchain_flp:   ; restore fat chain
   pushad

   mov  esi,0x282000
   mov  edi,0x8000

  fcnew2_1:
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

   cmp  edi,0x8000+0x1200     ;4274 bytes - all used FAT
   jb   fcnew2_1

   mov  esi,0x8000           ; duplicate fat chain
   mov  edi,0x8000+0x1200
   mov  ecx,0x1200/4
   cld
   rep  movsd

   popad
   ret


floppy_filedelete:
;--------------------------------------------
;
; filedelete - sys floppy
; in:
; eax - filename 11 chars - for root directory
; edi  pointer to path   /fd/1/...... - for all files in nested directories
;
; out:
; eax - 0 = successful, 1 = file not found, 10 = access denied
;
;--------------------------------------------
    mov    [path_pointer_flp],edi
    mov    [save_flag],0
    mov    ebp,1  ; file not found as default
filedelete_newtry_1:
    sub    esp,32
    call   expand_filename
    push   eax ebx ecx edx esi edi
    call   read_flp_fat
    cmp    [FDC_Status],0
    jne    frnoreadd_1
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],1      ; Сторона
    mov    [FDD_Sector],2      ; Сектор
    call    SeekTrack
    mov     dh,14
l.20_2:
    call    ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    fdc_status_error_4
    mov     dl,16
    mov     edi,0xD000
    inc     [FDD_Sector]
l.21_2:
    mov    esi,eax            ;Name of file we want
    mov    ecx,11
    cld
    rep    cmpsb            ;Found the file?
    je     fifoundd_1          ;Yes
    add    ecx,21
    add    edi, ecx         ;Advance to next entry
    dec    dl
    cmp    dl,0
    jne    l.21_2
    dec    dh
    cmp    dh,0
    jne    l.20_2
    jmp    frnoreadd_1

fdc_status_error_4:
    pop    edi esi edx ecx ebx eax
    add    esp,32
    jmp    fdc_status_error_1

fifoundd_1:
    mov    eax,[path_pointer_flp]
    cmp    [eax+36],byte 0
    je    fifoundd_2
    add    edi,0xf
    mov    eax,[edi]
    and    eax,65535
    mov    ebx,[path_pointer_flp]
    add    ebx,36
    call   get_cluster_of_a_path_flp
    jc     frnoreadd_1_1    
    mov    edi,ebx
    add    edi,11
    jmp    fifoundd_2_1
fifoundd_2:
    dec    [FDD_Sector]
fifoundd_2_1:
    mov    [edi-11],byte 0xE5    ;mark filename deleted
    add    edi,0xf
    mov    eax,[edi]
    and    eax,65535
    mov    edi,eax            ;edi = cluster
frnewd_1:
    shl    edi,1            ;find next cluster from FAT
    add    edi,0x282000
    mov    eax,[edi]
    mov    [edi],word 0x0        ;clear fat chain cluster
    and    eax,4095
    mov    edi,eax
    cmp    edi,dword 4095        ;last cluster ?
    jz     frnoreadd2_1
    jmp    frnewd_1

frnoreadd2_1:
    call   WriteSectWithRetr
    cmp    [FDC_Status],0
    jne    fdc_status_error_4
    call   save_flp_fat
    cmp    [FDC_Status],0
    jne    fdc_status_error_4
;    pop    edi esi edx ecx ebx eax
;    add    esp,32
    mov    ebp,0       ; file found
;    jmp    filedelete_newtry_1
    jmp    frnoreadd_1

frnoreadd_1_1:
    cmp    [FDC_Status],0
    jne    fdc_status_error_4
frnoreadd_1:
    pop    edi esi edx ecx ebx eax
    add    esp,32
    mov    eax,ebp
    ret

floppy_filesave:
;----------------------------------------------------------
;
; filesave - sys floppy
;
; eax      ; pointer to file name 11 chars - for root directory
; ebx      ; buffer
; ecx      ; count to write in bytes
; edx      ; 0 create new , 1 append
; edi  pointer to path   /fd/1/......  - for all files in nested directories
;
; output : eax = 0 - ok
;                5 - file not found / directory not found 
;                8 - disk full
;               10 - access denied
;-----------------------------------------------------------
    mov    [path_pointer_flp],edi
    sub  esp,32
    call expand_filename
    cmp  edx,0
    jnz  fsdel_1
    pusha
    call floppy_filedelete
    cmp    [FDC_Status],0
    jne    fdc_status_error_6
    popa
    mov    [save_flag],1
fsdel_1:
    call   floppy_free_space
    cmp    [FDC_Status],0
    jne    fdc_status_error_6
    cmp    ecx,edi
    jb     rd_do_save_1
    add    esp,32
    mov    eax,8    ; not enough free space
    mov   [flp_status],0
    ret

fdc_status_error_6:
    popa 
    add    esp,32
    jmp    fdc_status_error_1
    
rd_do_save_1:
    push   eax ebx ecx edx esi edi
    call   read_flp_fat
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
    push   eax
    mov    eax,[path_pointer_flp]
    cmp    [eax+36],byte 0
    jne    fifoundds_2
    pop    eax
    mov    [save_root_flag],1
    call   read_flp_root
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
    mov    edi,0x8000   ;Point at directory
    mov    edx,224 +1
    ; find an empty spot for filename in the root dir
l20ds_1:
    sub    edx,1
    cmp    edx,0
    jnz    l21ds_1
    jmp    frnoreadds_1
l21ds_1:
    cmp    [edi],byte 0xE5
    jz     fifoundds_1
    cmp    [edi],byte 0x0
    jz     fifoundds_1
    add    edi,32            ; Advance to next entry
    jmp    l20ds_1

fifoundds_2:
    pop    eax
    mov    [save_root_flag],0
    mov    [FDD_Track],0      ; Цилиндр
    mov    [FDD_Head],1       ; Сторона
    mov    [FDD_Sector],2      ; Сектор
    call   SeekTrack
    mov    dh,14
l.20_3:
    call    ReadSectWithRetr
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
    mov    dl,16
    mov    edi,0xD000
    inc     [FDD_Sector]
l.21_3:
    mov    esi,eax            ;Name of file we want
    mov    ecx,11
    cld
    rep    cmpsb            ;Found the file?
    je     fifoundds_3          ;Yes
    add    ecx,21
    add    edi, ecx         ;Advance to next entry
    dec    dl
    cmp    dl,0
    jne    l.21_3
    dec    dh
    cmp    dh,0
    jne    l.20_3
fdc_status_error_8:
    pop     edi esi edx ecx ebx eax
    mov    eax,5            ; file not found ?
    mov    ebx,-1
    add    esp,32
    mov    [flp_status],0
    ret

fifoundds_3:
    add    edi,0xf
    mov    eax,[edi]
    and    eax,65535
    mov    ebx,[path_pointer_flp]
    add    ebx,36
    call   get_cluster_of_a_path_flp
    jc     fdc_status_error_7_1
found_directory_for_writing_flp:
    call   analyze_directory_to_write_flp
    jc     fdc_status_error_7_1
    mov    edi,ebx
fifoundds_1:
    push   edi            ; move the filename to root dir
    mov    esi,[esp+4+20]
    cmp    [save_root_flag],0
    jne    fifoundds_4
    mov    esi,[pointer_file_name_flp]
fifoundds_4:
    mov    ecx,11
    cld
    rep    movsb
    pop    edi
    mov    edx,edi
    add    edx,11+0xf        ; edx <- cluster save position
    mov    ebx,[esp+12]        ; save file size
    mov    [edi+28],ebx
    mov    [edi+11],byte 0x20    ; attribute
    call   get_date_for_file     ; from FAT32.INC
    mov    [edi+24],ax      ; date
    mov    [edi+18],ax      ; date
    call   get_time_for_file     ; from FAT32.INC
    mov    [edi+22],ax      ; time
    xor    ax,ax
    mov    [edi+20],ax
    mov    ebx,1            ; first cluster
    cmp    [save_root_flag],0
    jne    frnewds_1
    call   frnewds_2 
    pusha
    call   WriteSectWithRetr
    popa
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
    jmp    frnewds_3

frnewds_1:
    call   frnewds_2
frnewds_3:
    pusha                ; move save to floppy cluster
    add    ebx,31
    mov    eax,ebx
    mov    esi,[esp+32+16]
    call   take_data_from_application_1
    call   save_chs_sector
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
    popa
    mov    eax,[esp+12]
    cmp    eax,512
    jb     flnsa_1
    sub    eax,512
    mov    [esp+12],eax
    mov    eax,[esp+16]
    add    eax,512
    mov    [esp+16],eax
    jmp    frnewds_1

frnewds_2:
    add    ebx,1
    mov    edi,ebx            ; find free cluster in FAT
    shl    edi,1
    add    edi,0x282000
    mov    eax,[edi]
    and    eax,4095
    cmp    eax,0x0
    jnz    frnewds_2
    mov    [edx],bx         ; save next cluster pos. to prev cl.
    mov    edx,edi            ; next save pos abs mem add
    ret

flnsa_1:
    mov    [edi],word 4095        ; mark end of file - last cluster
    cmp    [save_root_flag],1
    jne    flnsa_2
    call   save_flp_root
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
flnsa_2:
    call   save_flp_fat
    cmp    [FDC_Status],0
    jne    fdc_status_error_7
frnoreadds_1:
    pop    edi esi edx ecx ebx eax
    add    esp,32
    mov    eax,0
    mov   [flp_status],0
    ret

fdc_status_error_7_1:
    cmp    [FDC_Status],0
    je    fdc_status_error_8    
fdc_status_error_7:
    pop    edi esi edx ecx ebx eax
    add    esp,32
    jmp    fdc_status_error_1

save_chs_sector:
    call    calculate_chs
    call    WriteSectWithRetr
    ret
    
calculate_chs:
    mov    bl,[FDD_Track]
    mov    [old_track],bl    
    mov    ebx,18
    xor    edx,edx
    div    ebx
    inc    edx
    mov    [FDD_Sector],dl
    xor    edx,edx
    mov    ebx,2
    div    ebx
    mov    [FDD_Track],al
    mov    [FDD_Head],0
    cmp    edx,0
    je     no_head_2
    inc    [FDD_Head]
no_head_2:
    mov     dl,[old_track]
    cmp     dl,[FDD_Track]
    je      no_seek_track_1
    call    SeekTrack
no_seek_track_1:
    ret

    
get_cluster_of_a_path_flp:
;---------------------------------------------------------
; input  : EBX = pointer to a path string
;          (example: the path "/files/data/document" become
;                             "files......data.......document...0"
;          '.' = space char
;          '0' = char(0) (ASCII=0) !!! )
; output : if (CARRY=1) -> ERROR in the PATH
;          if (CARRY=0) -> EAX=cluster
;---------------------------------------------------------

    push  edx
    mov   edx,ebx

search_end_of_path_flp:
    cmp   [save_flag],0
    jne   search_end_of_path_flp_1
    cmp   byte [edx],0
    je    found_end_of_path_flp
    jmp   search_end_of_path_flp_2
search_end_of_path_flp_1:
    cmp   byte [edx+12],0
    je    found_end_of_path_flp
search_end_of_path_flp_2:
    inc   edx ; '/'
    call  analyze_directory_flp
    jc    directory_not_found_flp

    mov   eax,[ebx+20-2]        ; read the HIGH 16bit cluster field
    mov   ax,[ebx+26]           ; read the LOW 16bit cluster field
    and   eax,0xfff           ;[fatMASK]
    add   edx,11                ; 8+3 (name+extension)
    jmp   search_end_of_path_flp

found_end_of_path_flp:
    inc   edx
    mov   [pointer_file_name_flp],edx
    pop   edx
    clc                         ; no errors
    ret

directory_not_found_flp:
    pop   edx
    stc                         ; errors occour
    ret
    
analyze_directory_flp:
;--------------------------------
; input  : EAX = first cluster of the directory
;          EBX = pointer to filename
; output : IF CARRY=0 EAX = sector where th file is found
;                     EBX = pointer in buffer
;                     [buffer .. buffer+511]
;                     ECX,EDX,EDI,EDI not changed
;          IF CARRY=1
;--------------------------------
   push ebx ;[esp+16]
   push ecx
   push edx
   push esi
   push edi
   
   
adr56_flp:
   mov [clust_tmp_flp],eax
   add    eax,31
   pusha
   call   read_chs_sector
   popa
   cmp    [FDC_Status],0
   jne    not_found_file_analyze_flp

   mov ecx,512/32
   mov ebx,0xD000
   
adr1_analyze_flp:
   mov esi,edx   ;[esp+16]
   mov edi,ebx
   cld
   push ecx
   mov ecx,11
   rep cmpsb
   pop ecx
   je found_file_analyze_flp
   
   add ebx,32
   loop adr1_analyze_flp
   
    mov eax,[clust_tmp_flp]
    shl    eax,1            ;find next cluster from FAT
    add    eax,0x282000
    mov    eax,[eax]
    and    eax,4095
    cmp eax,0x0ff8
    jb  adr56_flp
not_found_file_analyze_flp:   
   pop edi
   pop esi
   pop edx
   pop ecx
   add esp,4
   stc        ;file not found
   ret
   
found_file_analyze_flp:
   pop edi
   pop esi
   pop edx
   pop ecx
   add esp,4
   clc        ;file found
   ret
    
    
analyze_directory_to_write_flp:
;--------------------------------
; input  : EAX = first cluster of the directory
; output : IF CARRY=0 EAX = sector where the file is found
;                     EBX = pointer in buffer
;                     [buffer .. buffer+511]
;                     ECX,EDX,EDI,EDI not changed
;          IF CARRY=1
;--------------------------------
   
   push ecx
   push edx
   push esi
   
adr561:
   mov [clust_tmp_flp],eax
   add    eax,31
   pusha
   call   read_chs_sector   
   popa
   cmp    [FDC_Status],0
   jne    error_found_file_analyze1

   mov ecx,512/32
   mov ebx,0xD000
   
adr1_analyze1:
   cmp byte [ebx],0x00
   je  found_file_analyze1
   cmp byte [ebx],0xe5
   je  found_file_analyze1
   
avanti:
   add ebx,32
   loop adr1_analyze1
   
   mov eax,[clust_tmp_flp]
   shl    eax,1            ;find next cluster from FAT
   add    eax,0x282000
   mov    eax,[eax]
   and    eax,4095
   cmp eax,0x0ff8
   jb  adr561
   
   call get_free_FAT               ;this block of code add a new cluster
                                   ;for the directory because the directory
                                   ;is full

   mov [edi],word 0x0fff
   
   mov eax,[clust_tmp_flp]
   shl    eax,1            ;find next cluster from FAT
   add    eax,0x282000
   sub    edi,0x282000
   mov    [eax],di

   pusha
   mov ecx,512/4
   xor eax,eax
   mov edi,0xD000
   cld
   rep stosd
   popa

   mov    eax,edi
   add    eax,31
   pusha
   call   save_chs_sector   
   popa
   cmp    [FDC_Status],0
   jne    error_found_file_analyze1
   mov    ebx,0xD000

found_file_analyze1:
   
   pop esi
   pop edx
   pop ecx
   clc        ;file found
   ret

error_found_file_analyze1:
   pop esi
   pop edx
   pop ecx
   stc
   ret   
   
get_free_FAT_flp:
;------------------------------------------
; input  :  EAX = # cluster for start the searching
; output :  EAX = # first cluster found free
;-------------------------------------------
   push ebx
  
    mov    ebx,1
check_new_flp:
    add    ebx,1
    mov    edi,ebx            ; find free cluster in FAT
    shl    edi,1
    add    edi,0x282000
    mov    eax,[edi]
    and    eax,4095
    cmp    eax,0x0
    jnz    check_new_flp

   pop ebx
   ret
