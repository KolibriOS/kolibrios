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
    jne    l.21_2
    dec    dh
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
    movzx  eax, word [edi+0xf]
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
        movzx   edi, word [edi+0xf] ;edi = cluster
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
    jz     frnoreadds_1
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
    jne    l.21_3
    dec    dh
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

; \begin{diamond}
fat_find_lfn:
; in: esi->name
;     [esp+4] = next
;     [esp+8] = first
;     [esp+C]... - possibly parameters for first and next
; out: CF=1 - file not found
;      else CF=0, esi->next name component, edi->direntry
        pusha
        lea     eax, [esp+0Ch+20h]
        call    dword [eax-4]
        jc      .reterr
        sub     esp, 262*2      ; reserve place for LFN
        mov     ebp, esp
        push    0               ; for fat_get_name: read ASCII name
.l1:
        call    fat_get_name
        jc      .l2
        call    fat_compare_name
        jz      .found
.l2:
        lea     eax, [esp+0Ch+20h+262*2+4]
        call    dword [eax-8]
        jnc     .l1
        add     esp, 262*2+4
.reterr:
        stc
        popa
        ret
.found:
        add     esp, 262*2+4
; if this is LFN entry, advance to true entry
        cmp     byte [edi+11], 0xF
        jnz     @f
        lea     eax, [esp+0Ch+20h]
        call    dword [eax-8]
        jc      .reterr
@@:
        add     esp, 8          ; CF=0
        push    esi
        push    edi
        popa
        ret

flp_root_next:
        cmp     edi, 0xD200-0x20
        jae     @f
        add     edi, 0x20
        ret     ; CF=0
@@:
; read next sector
        inc     dword [eax]
        cmp     dword [eax], 14
        jae     flp_root_first.readerr
flp_root_first:
        mov     eax, [eax]
        pusha
        add     eax, 19
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .readerr
        mov     edi, 0xD000
        ret     ; CF=0
.readerr:
        stc
        ret

flp_rootmem_first:
        mov     edi, 0x8000
        clc
        ret
flp_rootmem_next:
        add     edi, 0x20
        cmp     edi, 0x8000+14*0x200
        cmc
flp_rootmem_next_write:
flp_rootmem_begin_write:
flp_rootmem_end_write:
        ret
flp_rootmem_extend_dir:
        stc
        ret

flp_notroot_next:
        cmp     edi, 0xD200-0x20
        jae     flp_notroot_next_sector
        add     edi, 0x20
        ret     ; CF=0
flp_notroot_next_sector:
        push    ecx
        mov     ecx, [eax]
        mov     ecx, [ecx*2+0x282000]
        and     ecx, 0xFFF
        cmp     ecx, 2849
        jae     flp_notroot_first.err2
        mov     [eax], ecx
        pop     ecx
flp_notroot_first:
        mov     eax, [eax]
        cmp     eax, 2
        jb      .err
        cmp     eax, 2849
        jae     .err
        pusha
        add     eax, 31
        call    read_chs_sector
        popa
        mov     edi, 0xD000
        cmp     [FDC_Status], 0
        jnz     .err
        ret     ; CF=0
.err2:
        pop     ecx
.err:
        stc
        ret
flp_notroot_begin_write:
        pusha
        mov     eax, [eax]
        add     eax, 31
        call    read_chs_sector
        popa
        ret
flp_notroot_end_write:
        pusha
        mov     eax, [eax]
        add     eax, 31
        call    save_chs_sector
        popa
        ret
flp_notroot_next_write:
        cmp     edi, 0xD200
        jae     @f
        ret
@@:
        call    flp_notroot_end_write
        jmp     flp_notroot_next_sector
flp_notroot_extend_dir:
; find free cluster in FAT
        pusha
        xor     eax, eax
        mov     edi, 0x282000
        mov     ecx, 2849
        repnz   scasw
        jnz     .notfound
        mov     word [edi-2], 0xFFF     ; mark as last cluster
        sub     edi, 0x282000
        shr     edi, 1
        dec     edi
        mov     eax, [esp+28]
        mov     ecx, [eax]
        mov     [0x282000+ecx*2], di
        mov     [eax], edi
        xor     eax, eax
        mov     edi, 0xD000
        mov     ecx, 128
        rep     stosd
        popa
        call    flp_notroot_end_write
        mov     edi, 0xD000
        clc
        ret
.notfound:
        popa
        stc
        ret

fd_find_lfn:
; in: esi->name
; out: CF=1 - file not found
;      else CF=0 and edi->direntry, eax=directory cluster (0 for root)
        push    esi edi
        push    0
        push    flp_root_first
        push    flp_root_next
.loop:
        call    fat_find_lfn
        jc      .notfound
        cmp     byte [esi], 0
        jz      .found
        test    byte [edi+11], 10h
        jz      .notfound
        movzx   eax, word [edi+26]      ; cluster
        mov     [esp+8], eax
        mov     dword [esp+4], flp_notroot_first
        mov     dword [esp], flp_notroot_next
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
;  fs_FloppyRead - LFN variant for reading floppy
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
fs_FloppyRead:
        call    read_flp_fat
        cmp     byte [esi], 0
        jnz     @f
        or      ebx, -1
        mov     eax, 10         ; access denied
        ret
@@:
        push    edi
        call    fd_find_lfn
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
        movzx   edi, word [edi+26]
.new:
        jecxz   .done
        test    edi, edi
        jz      .eof
        cmp     edi, 0xFF8
        jae     .eof
        sub     ebx, 512
        jae     .skip
        lea     eax, [edi+31]
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .err
        lea     eax, [0xD000+ebx+512]
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
        movzx   edi, word [edi*2+0x282000]
        jmp     .new
.done:
        mov     ebx, edx
        pop     eax edx ecx edi
        sub     ebx, edx
        ret
.eof:
        mov     ebx, edx
        pop     eax edx ecx
        jmp     .reteof
.err:
        mov     ebx, edx
        pop     eax edx ecx edi
        sub     ebx, edx
        mov     al, 11
        ret

;----------------------------------------------------------------
;
;  fs_FloppyReadFolder - LFN variant for reading floppy folders
;
;  esi  points to filename
;  ebx  pointer to structure: 32-bit number = first wanted block, 0+
;                           & flags (bitfields)
; flags: bit 0: 0=ANSI names, 1=UNICODE names
;  ecx  number of blocks to read, 0+
;  edx  mem location to return data
;
;  ret ebx = blocks read or 0xffffffff folder not found
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
fs_FloppyReadFolder:
        call    read_flp_fat
        push    edi
        cmp     byte [esi], 0
        jz      .root
        call    fd_find_lfn
        jnc     .found
        pop     edi
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
.found:
        test    byte [edi+11], 0x10     ; do not allow read files
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
        push    ecx ebp
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
        pop     ecx eax
        mov     byte [edx], 1   ; version
        mov     esi, edi        ; esi points to BDFE
.main_loop:
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .error
        mov     edi, 0xD000
        push    eax
.l1:
        call    fat_get_name
        jc      .l2
        cmp     byte [edi+11], 0xF
        jnz     .do_bdfe
        add     edi, 0x20
        cmp     edi, 0xD200
        jb      .do_bdfe
        pop     eax
        inc     eax
        dec     byte [esp+262*2+12]
        jz      .done
        jns     @f
; read next sector from FAT
        mov     eax, [(eax-31-1)*2+0x282000]
        and     eax, 0xFFF
        cmp     eax, 0xFF8
        jae     .done
        add     eax, 31
        mov     byte [esp+262*2+12], 0
@@:
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .error
        mov     edi, 0xD000
        push    eax
.do_bdfe:
        inc     dword [edx+8]   ; new file found
        dec     ebx
        jns     .l2
        dec     ecx
        js      .l2
        inc     dword [edx+4]   ; new file block copied
        call    fat_entry_to_bdfe
.l2:
        add     edi, 0x20
        cmp     edi, 0xD200
        jb      .l1
        pop     eax
        inc     eax
        dec     byte [esp+262*2+12]
        jz      .done
        jns     @f
; read next sector from FAT
        mov     eax, [(eax-31-1)*2+0x282000]
        and     eax, 0xFFF
        cmp     eax, 0xFF8
        jae     .done
        add     eax, 31
        mov     byte [esp+262*2+12], 0
@@:
        jmp     .main_loop
.error:
        add     esp, 262*2+4
        pop     ebp ecx edi edi
        or      ebx, -1
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
.done:
        add     esp, 262*2+4
        pop     ebp
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ecx
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        pop     ecx edi edi
        ret

;----------------------------------------------------------------
;
;  fs_FloppyRewrite - LFN variant for writing sys floppy
;
;  esi  points to filename
;  ebx  ignored (reserved)
;  ecx  number of bytes to write, 0+
;  edx  mem location to data
;
;  ret ebx = number of written bytes
;      eax = 0 ok read or other = errormsg
;
;--------------------------------------------------------------
@@:
        mov     eax, ERROR_ACCESS_DENIED
        xor     ebx, ebx
        ret
fsfrfe2:
        popad
fsfrfe:
        mov     eax, 11
        xor     ebx, ebx
        ret

fs_FloppyRewrite:
        cmp     byte [esi], 0
        jz      @b
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jnz     fsfrfe
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
        call    read_flp_root
        cmp     [FDC_Status], 0
        jnz     fsfrfe2
        push    flp_rootmem_extend_dir
        push    flp_rootmem_end_write
        push    flp_rootmem_next_write
        push    flp_rootmem_begin_write
        xor     ebp, ebp
        push    ebp
        push    flp_rootmem_first
        push    flp_rootmem_next
        jmp     .common1
.noroot:
; check existence
        mov     byte [ebp], 0
        call    fd_find_lfn
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
        push    flp_notroot_extend_dir
        push    flp_notroot_end_write
        push    flp_notroot_next_write
        push    flp_notroot_begin_write
        push    ebp
        push    flp_notroot_first
        push    flp_notroot_next
.common1:
        call    fat_find_lfn
        jc      .notfound
; found; must not be directory
        test    byte [edi+11], 10h
        jz      @f
        add     esp, 28
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
        lea     edi, [0x282000 + eax*2] ; position in FAT
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
        add     esp, 28
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
        add     esp, 12+28
        popa
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
        jnc     .scan_dir
.fsfrfe3:
        add     esp, 8+8+12+28
        popad
        mov     eax, 11
        xor     ebx, ebx
        ret
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
        cmp     [FDC_Status], 0
        jnz     .fsfrfe3
        push    eax
        lea     eax, [esp+12+8+12+8]
        call    dword [eax+16]          ; extend directory
        pop     eax
        jnc     .scan_dir
        add     esp, 8+8+12+28
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
; edi points to first entry in free chunk
        dec     ecx
        jz      .nolfn
        push    esi
        push    eax
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+4]         ; begin write
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
        call    fs_RamdiskRewrite.read_symbols
        mov     ax, 0xF
        stosw
        mov     al, [esp+4]
        stosb
        mov     cl, 6
        call    fs_RamdiskRewrite.read_symbols
        xor     eax, eax
        stosw
        mov     cl, 2
        call    fs_RamdiskRewrite.read_symbols
        pop     ecx
        lea     eax, [esp+8+8+12+8]
        call    dword [eax+8]         ; next write
        xor     eax, eax
        loop    .writelfn
        pop     eax
        pop     esi
;        lea     eax, [esp+8+12+8]
;        call    dword [eax+12]          ; end write
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
.doit:
        lea     eax, [esp+8]
        call    dword [eax+12]  ; flush directory
        push    ecx
        push    edi
        add     edi, 26         ; edi points to low word of cluster
        push    edi
        mov     esi, edx
        jecxz   .done
        mov     ecx, 2849
        mov     edi, 0x282000
        push    0       ; first cluster
.write_loop:
; allocate new cluster
        xor     eax, eax
        repnz   scasw
        mov     al, ERROR_DISK_FULL
        jnz     .ret
        dec     edi
        dec     edi
        lea     eax, [edi-0x282000]
        shr     eax, 1                  ; eax = cluster
        mov     word [edi], 0xFFF       ; mark as last cluster
        cmp     dword [esp], 0
        jz      .first
        xchg    edi, [esp+4]
        stosw
        mov     edi, [esp+4]
        jmp     @f
.first:
        mov     [esp], eax
@@:
        inc     ecx
; write data
        push    ecx edi
        mov     ecx, 512
        cmp     dword [esp+20], ecx
        jae     @f
        mov     ecx, [esp+20]
@@:
        push    ecx
        mov     edi, 0xD000
        rep     movsb
        pop     ecx
        push    ecx
        sub     ecx, 512
        neg     ecx
        push    eax
        xor     eax, eax
        rep     stosb
        pop     eax
        add     eax, 31
        pusha
        call    save_chs_sector
        popa
        pop     ecx
        cmp     [FDC_Status], 0
        jnz     .diskerr
        sub     [esp+20], ecx
        pop     edi ecx
        jnz     .write_loop
.done:
        xor     eax, eax
.ret:
        pop     ebx edi edi ecx
        mov     [esp+28+28], eax
        lea     eax, [esp+8]
        call    dword [eax+4]
        mov     [edi+26], bx
        mov     ebx, esi
        sub     ebx, edx
        mov     [edi+28], ebx
        call    dword [eax+12]
        mov     [esp+28+16], ebx
        test    ebp, ebp
        jnz     @f
        call    save_flp_root
@@:
        add     esp, 28
        cmp     [FDC_Status], 0
        jnz     .err3
        call    save_flp_fat
        cmp     [FDC_Status], 0
        jnz     .err3
        popa
        ret
.err3:
        popa
        mov     al, 11
        xor     ebx, ebx
        ret
.diskerr:
        sub     esi, ecx
        mov     eax, 11
        pop     edi ecx
        jmp     .ret

fs_FloppyGetFileInfo:
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jnz     ret11
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2  ; unsupported
        ret
@@:
        push    edi
        call    fd_find_lfn
        jmp     fs_GetFileInfo_finish

ret11:
        mov     eax, 11
        ret

fs_FloppySetFileInfo:
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jnz     ret11
        cmp     byte [esi], 0
        jnz     @f
        mov     eax, 2  ; unsupported
        ret
@@:
        push    edi
        call    fd_find_lfn
        jnc     @f
        pop     edi
        mov     eax, ERROR_FILE_NOT_FOUND
        ret
@@:
        push    eax
        call    bdfe_to_fat_entry
        pop     eax
        test    eax, eax
        jz      .root
        add     eax, 31
        pusha
        call    save_chs_sector
        popa
        jmp     .cmn
.root:
        call    save_flp_root
.cmn:
        pop     edi
        xor     eax, eax
        cmp     [FDC_Status], 0
        jz      @f
        mov     al, 11
@@:
        ret

;----------------------------------------------------------------
;
;  fs_FloppyExecute - LFN variant for executing from floppy
;
;  esi  points to floppy filename (e.g. 'dir1/name')
;  ebp  points to full filename (e.g. '/fd/1/dir1/name')
;  dword [ebx] = flags
;  dword [ebx+4] = cmdline
;
;  ret ebx,edx destroyed
;      eax > 0 - PID, < 0 - error
;
;--------------------------------------------------------------
fs_FloppyExecute:
        mov     edx, [ebx]
        mov     ebx, [ebx+4]
        test    ebx, ebx
        jz      @f
        add     ebx, std_application_base_address
@@:

;----------------------------------------------------------------
;
; fs_FloppyExecute.flags - second entry
;
;  esi  points to floppy filename (kernel address)
;  ebp  points to full filename
;  edx  flags
;  ebx  cmdline (kernel address)
;
;  ret  eax > 0 - PID, < 0 - error
;
;--------------------------------------------------------------

.flags:
        call    read_flp_fat
        cmp     byte [esi], 0
        jnz     @f
; cannot execute root!
        mov     eax, -ERROR_ACCESS_DENIED
        ret
@@:
        push    edi
        call    fd_find_lfn
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
        mov     eax, [eax+4]    ; cluster
        add     eax, 31
        call    read_chs_sector
        cmp     [FDC_Status], 0
        jnz     .err
        pop     edi
        mov     esi, 0xD000
        push    edi
        mov     ecx, 512/4
        rep     movsd
        mov     eax, [esp+28]
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
        mov     edx, [eax+4]
        mov     dx, [edx*2+0x282000]
        mov     [eax+4], dx     ; high word is already zero
        popad
        xor     eax, eax
        ret
.eof:
        popad
        mov     eax, 6
        ret
.err:
        popad
        mov     eax, 11
        ret

; \end{diamond}
