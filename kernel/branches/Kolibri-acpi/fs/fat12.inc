;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; FAT12.INC                                                    ;;
;; (C) 2005 Mario79, License: GPL                               ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


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
        cmp     [flp_status], 0
        je      reserve_flp_ok

        sti
        call    change_task
        jmp     reserve_flp

  reserve_flp_ok:

        push    eax
        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        mov     eax, [eax+CURRENT_TASK+TASKDATA.pid]
        mov     [flp_status], eax
        pop     eax
        sti
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

        mov     [save_flag], 0
        mov     [path_pointer_flp], edi
        test    esi, esi     ; return ramdisk root
        jnz     fr_noroot_1
        cmp     ebx, 224/16
        jbe     fr_do_1
        mov     eax, 5
        xor     ebx, ebx
        mov     [flp_status], ebx
        ret

fr_do_1:
        push    ebx ecx edx
        call    read_flp_root
        pop     edx ecx ebx
        cmp     [FDC_Status], 0
        jne     fdc_status_error_1
        mov     edi, edx
        dec     ebx
        shl     ebx, 9
        mov     esi, FLOPPY_BUFF
        add     esi, ebx
        shl     ecx, 9
        cld
        rep movsb
        xor     eax, eax
        xor     ebx, ebx
;    mov    eax,0 ; ok read
;    mov    ebx,0
        mov     [flp_status], eax
        ret
fdc_status_error_1:
        xor     eax, eax
        mov     [flp_status], eax
        mov     eax, 10
        or      ebx, -1
        ret

fr_noroot_1:
        sub     esp, 32
        call    expand_filename
frfloppy_1:
        test    ebx, ebx
        jnz     frfl5_1
        mov     ebx, 1
frfl5_1:
        test    ecx, ecx
        jnz     frfl6_1
        mov     ecx, 1
frfl6_1:
        dec     ebx
        push    eax
        push    eax ebx ecx edx esi edi
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jne     fdc_status_error_3_1
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 1; Сторона
        mov     [FDD_Sector], 2; Сектор
        call    SeekTrack
        mov     dh, 14
l.20_1:
        call    ReadSectWithRetr
        cmp     [FDC_Status], 0
        jne     fdc_status_error_3_1
        mov     dl, 16
        mov     edi, FDD_BUFF
        inc     [FDD_Sector]
l.21_1:
        mov     esi, eax      ;Name of file we want
        mov     ecx, 11
        cld
        rep cmpsb           ;Found the file?
        je      fifound_1     ;Yes
        add     ecx, 21
        add     edi, ecx    ;Advance to next entry
        dec     dl
        test    dl, dl
        jnz     l.21_1
        dec     dh
        test    dh, dh
        jnz     l.20_1
fdc_status_error_3:
        mov     eax, 5      ; file not found ?
        or      ebx, -1
        add     esp, 32+28
        mov     [flp_status], 0
        ret
fdc_status_error_3_2:
        cmp     [FDC_Status], 0
        je      fdc_status_error_3
fdc_status_error_3_1:
        add     esp, 32+28
        jmp     fdc_status_error_1

fifound_1:
        mov     eax, [path_pointer_flp]
        cmp     [eax+36], byte 0
        je      fifound_2
        add     edi, 0xf
        mov     eax, [edi]
        and     eax, 65535
        mov     ebx, [path_pointer_flp]
        add     ebx, 36
        call    get_cluster_of_a_path_flp
        jc      fdc_status_error_3_2
        mov     ebx, [ebx-11+28]  ;file size
        mov     [esp+20], ebx
        mov     [esp+24], ebx
        jmp     fifound_3
fifound_2:
        mov     ebx, [edi-11+28]  ;file size
        mov     [esp+20], ebx
        mov     [esp+24], ebx
        add     edi, 0xf
        mov     eax, [edi]
fifound_3:
        and     eax, 65535
        mov     [n_sector], eax      ;eax=cluster
frnew_1:
        add     eax, 31      ;bootsector+2*fat+filenames
        cmp     [esp+16], dword 0; wanted cluster ?
        jne     frfl7_1
        call    read_chs_sector
        cmp     [FDC_Status], 0
        jne     fdc_status_error_5
        mov     edi, [esp+8]
        call    give_back_application_data_1
        add     [esp+8], dword 512
        dec     dword [esp+12]   ; last wanted cluster ?
        cmp     [esp+12], dword 0
        je      frnoread_1
        jmp     frfl8_1
frfl7_1:
        dec     dword [esp+16]
frfl8_1:
        mov     edi, [n_sector]
        shl     edi, 1      ;find next cluster from FAT
        add     edi, FLOPPY_FAT
        mov     eax, [edi]
        and     eax, 4095
        mov     edi, eax
        mov     [n_sector], edi
        cmp     edi, 4095   ;eof  - cluster
        jz      frnoread2_1
        cmp     [esp+24], dword 512;eof  - size
        jb      frnoread_1
        sub     [esp+24], dword 512
        jmp     frnew_1

read_chs_sector:
        call    calculate_chs
        call    ReadSectWithRetr
        ret

frnoread2_1:
        cmp     [esp+16], dword 0; eof without read ?
        je      frnoread_1
        mov     [fdc_irq_func], fdc_null
        pop     edi esi edx ecx
        add     esp, 4
        pop     ebx; ebx <- eax : size of file
        add     esp, 36
        mov     eax, 6; end of file
        mov     [flp_status], 0
        ret

frnoread_1:
        pop     edi esi edx ecx
        add     esp, 4
        pop     ebx; ebx <- eax : size of file
        add     esp, 36
        xor     eax, eax
        mov     [flp_status], eax
        ret

fdc_status_error_5:
        pop     edi esi edx ecx
        add     esp, 4
        pop     ebx; ebx <- eax : size of file
        add     esp, 36
        jmp     fdc_status_error_1

read_flp_root:
        pusha
        call    check_label
        cmp     [FDC_Status], 0
        jne     unnecessary_root_read
        cmp     [root_read], 1
        je      unnecessary_root_read
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 1; Сторона
        mov     [FDD_Sector], 2; Сектор
        mov     edi, FLOPPY_BUFF
        call    SeekTrack
read_flp_root_1:
        call    ReadSectWithRetr
        cmp     [FDC_Status], 0
        jne     unnecessary_root_read
        push    edi
        call    give_back_application_data_1
        pop     edi
        add     edi, 512
        inc     [FDD_Sector]
        cmp     [FDD_Sector], 16
        jne     read_flp_root_1
        mov     [root_read], 1
unnecessary_root_read:
        popa
        ret


read_flp_fat:
        pusha
        call    check_label
        cmp     [FDC_Status], 0
        jne     unnecessary_flp_fat
        cmp     [flp_fat], 1
        je      unnecessary_flp_fat
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 0; Сторона
        mov     [FDD_Sector], 2; Сектор
        mov     edi, FLOPPY_BUFF
        call    SeekTrack
read_flp_fat_1:
        call    ReadSectWithRetr
        cmp     [FDC_Status], 0
        jne     unnecessary_flp_fat
        push    edi
        call    give_back_application_data_1
        pop     edi
        add     edi, 512
        inc     [FDD_Sector]
        cmp     [FDD_Sector], 19
        jne     read_flp_fat_1
        mov     [FDD_Sector], 1
        mov     [FDD_Head], 1
        call    ReadSectWithRetr
        cmp     [FDC_Status], 0
        jne     unnecessary_flp_fat
        call    give_back_application_data_1
        call    calculatefatchain_flp
        mov     [root_read], 0
        mov     [flp_fat], 1
unnecessary_flp_fat:
        popa
        ret

calculatefatchain_flp:
        pushad

        mov     esi, FLOPPY_BUFF
        mov     edi, FLOPPY_FAT

 fcnew_1:
        mov     eax, dword [esi]
        mov     ebx, dword [esi+4]
        mov     ecx, dword [esi+8]
        mov     edx, ecx
        shr     edx, 4;8 ok
        shr     dx, 4;7 ok
        xor     ch, ch
        shld    ecx, ebx, 20;6 ok
        shr     cx, 4;5 ok
        shld    ebx, eax, 12
        and     ebx, 0x0fffffff;4 ok
        shr     bx, 4;3 ok
        shl     eax, 4
        and     eax, 0x0fffffff;2 ok
        shr     ax, 4;1 ok
        mov     dword [edi], eax
        add     edi, 4
        mov     dword [edi], ebx
        add     edi, 4
        mov     dword [edi], ecx
        add     edi, 4
        mov     dword [edi], edx
        add     edi, 4
        add     esi, 12

        cmp     edi, FLOPPY_FAT+2856*2;2849 clusters
        jnz     fcnew_1

        popad
        ret

check_label:
        pushad
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 0; Сторона
        mov     [FDD_Sector], 1; Сектор
        call    SetUserInterrupts
        call    FDDMotorON
        call    RecalibrateFDD
        cmp     [FDC_Status], 0
        jne     fdc_status_error
        call    SeekTrack
        cmp     [FDC_Status], 0
        jne     fdc_status_error
        call    ReadSectWithRetr
        cmp     [FDC_Status], 0
        jne     fdc_status_error
        mov     esi, flp_label
        mov     edi, FDD_BUFF+39
        mov     ecx, 15
        cld
        rep cmpsb
        je      same_label
        mov     [root_read], 0
        mov     [flp_fat], 0
same_label:
        mov     esi, FDD_BUFF+39
        mov     edi, flp_label
        mov     ecx, 15
        cld
        rep movsb
        popad
        ret
fdc_status_error:
        popad
        ret

save_flp_root:
        pusha
        call    check_label
        cmp     [FDC_Status], 0
        jne     unnecessary_root_save
        cmp     [root_read], 0
        je      unnecessary_root_save
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 1; Сторона
        mov     [FDD_Sector], 2; Сектор
        mov     esi, FLOPPY_BUFF
        call    SeekTrack
save_flp_root_1:
        push    esi
        call    take_data_from_application_1
        pop     esi
        add     esi, 512
        call    WriteSectWithRetr
        cmp     [FDC_Status], 0
        jne     unnecessary_root_save
        inc     [FDD_Sector]
        cmp     [FDD_Sector], 16
        jne     save_flp_root_1
unnecessary_root_save:
        mov     [fdc_irq_func], fdc_null
        popa
        ret

save_flp_fat:
        pusha
        call    check_label
        cmp     [FDC_Status], 0
        jne     unnecessary_flp_fat_save
        cmp     [flp_fat], 0
        je      unnecessary_flp_fat_save
        call    restorefatchain_flp
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 0; Сторона
        mov     [FDD_Sector], 2; Сектор
        mov     esi, FLOPPY_BUFF
        call    SeekTrack
save_flp_fat_1:
        push    esi
        call    take_data_from_application_1
        pop     esi
        add     esi, 512
        call    WriteSectWithRetr
        cmp     [FDC_Status], 0
        jne     unnecessary_flp_fat_save
        inc     [FDD_Sector]
        cmp     [FDD_Sector], 19
        jne     save_flp_fat_1
        mov     [FDD_Sector], 1
        mov     [FDD_Head], 1
        call    take_data_from_application_1
        call    WriteSectWithRetr
        cmp     [FDC_Status], 0
        jne     unnecessary_flp_fat_save
        mov     [root_read], 0
unnecessary_flp_fat_save:
        mov     [fdc_irq_func], fdc_null
        popa
        ret


restorefatchain_flp:   ; restore fat chain
        pushad

        mov     esi, FLOPPY_FAT
        mov     edi, FLOPPY_BUFF

  fcnew2_1:
        mov     eax, dword [esi]
        mov     ebx, dword [esi+4]
        shl     ax, 4
        shl     eax, 4
        shl     bx, 4
        shr     ebx, 4
        shrd    eax, ebx, 8
        shr     ebx, 8
        mov     dword [edi], eax
        add     edi, 4
        mov     word [edi], bx
        add     edi, 2
        add     esi, 8

        cmp     edi, FLOPPY_BUFF+0x1200;4274 bytes - all used FAT
        jb      fcnew2_1

        mov     esi, FLOPPY_BUFF  ; duplicate fat chain
        mov     edi, FLOPPY_BUFF+0x1200
        mov     ecx, 0x1200/4
        cld
        rep movsd

        popad
        ret


save_chs_sector:
        call    calculate_chs
        call    WriteSectWithRetr
        ret

calculate_chs:
        mov     bl, [FDD_Track]
        mov     [old_track], bl
        mov     ebx, 18
        xor     edx, edx
        div     ebx
        inc     edx
        mov     [FDD_Sector], dl
        xor     edx, edx
        mov     ebx, 2
        div     ebx
        mov     [FDD_Track], al
        mov     [FDD_Head], 0
        test    edx, edx
        jz      no_head_2
        inc     [FDD_Head]
no_head_2:
        mov     dl, [old_track]
        cmp     dl, [FDD_Track]
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

        push    edx
        mov     edx, ebx

search_end_of_path_flp:
        cmp     [save_flag], 0
        jne     search_end_of_path_flp_1
        cmp     byte [edx], 0
        je      found_end_of_path_flp
        jmp     search_end_of_path_flp_2
search_end_of_path_flp_1:
        cmp     byte [edx+12], 0
        je      found_end_of_path_flp
search_end_of_path_flp_2:
        inc     edx; '/'
        call    analyze_directory_flp
        jc      directory_not_found_flp

        mov     eax, [ebx+20-2] ; read the HIGH 16bit cluster field
        mov     ax, [ebx+26]    ; read the LOW 16bit cluster field
        and     eax, 0xfff    ;[fatMASK]
        add     edx, 11         ; 8+3 (name+extension)
        jmp     search_end_of_path_flp

found_end_of_path_flp:
        inc     edx
        mov     [pointer_file_name_flp], edx
        pop     edx
        clc                     ; no errors
        ret

directory_not_found_flp:
        pop     edx
        stc                     ; errors occour
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
        push    ebx;[esp+16]
        push    ecx
        push    edx
        push    esi
        push    edi


adr56_flp:
        mov     [clust_tmp_flp], eax
        add     eax, 31
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jne     not_found_file_analyze_flp

        mov     ecx, 512/32
        mov     ebx, FDD_BUFF

adr1_analyze_flp:
        mov     esi, edx;[esp+16]
        mov     edi, ebx
        cld
        push    ecx
        mov     ecx, 11
        rep cmpsb
        pop     ecx
        je      found_file_analyze_flp

        add     ebx, 32
        loop    adr1_analyze_flp

        mov     eax, [clust_tmp_flp]
        shl     eax, 1      ;find next cluster from FAT
        add     eax, FLOPPY_FAT
        mov     eax, [eax]
        and     eax, 4095
        cmp     eax, 0x0ff8
        jb      adr56_flp
not_found_file_analyze_flp:
        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        add     esp, 4
        stc     ;file not found
        ret

found_file_analyze_flp:
        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        add     esp, 4
        clc     ;file found
        ret


; \begin{diamond}
fat_find_lfn:
; in: esi->name
;     [esp+4] = next
;     [esp+8] = first
;     [esp+C]... - possibly parameters for first and next
; out: CF=1 - file not found, eax=error code
;      else CF=0, esi->next name component, edi->direntry
        pusha
        lea     eax, [esp+0Ch+20h]
        call    dword [eax-4]
        jc      .reterr
        sub     esp, 262*2      ; reserve place for LFN
        push    0               ; for fat_get_name: read ASCII name
.l1:
        lea     ebp, [esp+4]
        call    fat_get_name
        jc      .l2
        call    fat_compare_name
        jz      .found
.l2:
        mov     ebp, [esp+8+262*2+4]
        lea     eax, [esp+0Ch+20h+262*2+4]
        call    dword [eax-8]
        jnc     .l1
        add     esp, 262*2+4
.reterr:
        mov     [esp+28], eax
        stc
        popa
        ret
.found:
        add     esp, 262*2+4
        mov     ebp, [esp+8]
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

uglobal
; this is for delete support
fd_prev_sector          dd      ?
fd_prev_prev_sector     dd      ?
endg

flp_root_next:
        cmp     edi, OS_BASE+0xD200-0x20
        jae     @f
        add     edi, 0x20
        ret     ; CF=0
@@:
; read next sector
        inc     dword [eax]
        cmp     dword [eax], 14
        jae     flp_root_first.readerr
        push    [fd_prev_sector]
        pop     [fd_prev_prev_sector]
        push    eax
        mov     eax, [eax]
        add     eax, 19-1
        mov     [fd_prev_sector], eax
        pop     eax
flp_root_first:
        mov     eax, [eax]
        pusha
        add     eax, 19
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .readerr
        mov     edi, FDD_BUFF
        ret     ; CF=0
.readerr:
        stc
        ret

flp_rootmem_first:
        mov     edi, FLOPPY_BUFF
        clc
        ret
flp_rootmem_next:
        add     edi, 0x20
        cmp     edi, FLOPPY_BUFF+14*0x200
        cmc
flp_rootmem_next_write:
flp_rootmem_begin_write:
flp_rootmem_end_write:
        ret
flp_rootmem_extend_dir:
        stc
        ret

flp_notroot_next:
        cmp     edi, OS_BASE+0xD200-0x20
        jae     flp_notroot_next_sector
        add     edi, 0x20
        ret     ; CF=0
flp_notroot_next_sector:
        push    ecx
        mov     ecx, [eax]
        push    [fd_prev_sector]
        pop     [fd_prev_prev_sector]
        add     ecx, 31
        mov     [fd_prev_sector], ecx
        mov     ecx, [(ecx-31)*2+FLOPPY_FAT]
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
        mov     edi, FDD_BUFF
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
        cmp     edi, OS_BASE+0xD200
        jae     @f
        ret
@@:
        call    flp_notroot_end_write
        jmp     flp_notroot_next_sector
flp_notroot_extend_dir:
; find free cluster in FAT
        pusha
        xor     eax, eax
        mov     edi, FLOPPY_FAT
        mov     ecx, 2849
        repnz scasw
        jnz     .notfound
        mov     word [edi-2], 0xFFF     ; mark as last cluster
        sub     edi, FLOPPY_FAT
        shr     edi, 1
        dec     edi
        mov     eax, [esp+28]
        mov     ecx, [eax]
        mov     [FLOPPY_FAT+ecx*2], di
        mov     [eax], edi
        xor     eax, eax
        mov     edi, FDD_BUFF
        mov     ecx, 128
        rep stosd
        popa
        call    flp_notroot_end_write
        mov     edi, FDD_BUFF
        clc
        ret
.notfound:
        popa
        stc
        ret

fd_find_lfn:
; in: esi+ebp -> name
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
.continue:
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
        test    ebp, ebp
        jz      @f
        mov     esi, ebp
        xor     ebp, ebp
        jmp     .continue
@@:
        mov     eax, [esp+8]
        add     eax, 31
        cmp     dword [esp], flp_root_next
        jnz     @f
        add     eax, -31+19
@@:
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
        lea     eax, [FDD_BUFF+ebx+512]
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
        movzx   edi, word [edi*2+FLOPPY_FAT]
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
        rep stosd
        pop     ecx eax
        mov     byte [edx], 1   ; version
        mov     esi, edi        ; esi points to BDFE
.main_loop:
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .error
        mov     edi, FDD_BUFF
        push    eax
.l1:
        call    fat_get_name
        jc      .l2
        cmp     byte [edi+11], 0xF
        jnz     .do_bdfe
        add     edi, 0x20
        cmp     edi, OS_BASE+0xD200
        jb      .do_bdfe
        pop     eax
        inc     eax
        dec     byte [esp+262*2+12]
        jz      .done
        jns     @f
; read next sector from FAT
        mov     eax, [(eax-31-1)*2+FLOPPY_FAT]
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
        mov     edi, FDD_BUFF
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
        cmp     edi, OS_BASE+0xD200
        jb      .l1
        pop     eax
        inc     eax
        dec     byte [esp+262*2+12]
        jz      .done
        jns     @f
; read next sector from FAT
        mov     eax, [(eax-31-1)*2+FLOPPY_FAT]
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

fs_FloppyCreateFolder:
        mov     al, 1
        jmp     fs_FloppyRewrite.common

fs_FloppyRewrite:
        xor     eax, eax
.common:
        cmp     byte [esi], 0
        jz      @b
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jnz     fsfrfe
        pushad
        xor     edi, edi
        push    esi
        test    ebp, ebp
        jz      @f
        mov     esi, ebp
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     @b
        lea     edi, [esi-1]
        jmp     @b
@@:
        pop     esi
        test    edi, edi
        jnz     .noroot
        test    ebp, ebp
        jnz     .hasebp
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
.hasebp:
        mov     eax, ERROR_ACCESS_DENIED
        cmp     byte [ebp], 0
        jz      .ret1
        push    ebp
        xor     ebp, ebp
        call    fd_find_lfn
        pop     esi
        jc      .notfound0
        jmp     .common0
.noroot:
        mov     eax, ERROR_ACCESS_DENIED
        cmp     byte [edi+1], 0
        jz      .ret1
; check existence
        mov     byte [edi], 0
        push    edi
        call    fd_find_lfn
        pop     esi
        mov     byte [esi], '/'
        jnc     @f
.notfound0:
        mov     eax, ERROR_FILE_NOT_FOUND
.ret1:
        mov     [esp+28], eax
        popad
        xor     ebx, ebx
        ret
@@:
        inc     esi
.common0:
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
; found
        test    byte [edi+11], 10h
        jz      .exists_file
; found directory; if we are creating directory, return OK,
;                  if we are creating file, say "access denied"
        add     esp, 28
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
        cmp     byte [esp+28+28], 0
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
        lea     edi, [FLOPPY_FAT + eax*2] ; position in FAT
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
        repz cmpsb
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
        repnz scasb
        movi    eax, 1     ; 1 entry
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
        rep movsb
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
        cmp     byte [esp+28+28], 0
        jz      .doit
; create directory
        mov     byte [edi+11], 10h      ; attributes: folder
        mov     ecx, 32*2
        mov     edx, edi
.doit:
        lea     eax, [esp+8]
        call    dword [eax+12]  ; flush directory
        push    ecx
        push    edi
        push    0
        mov     esi, edx
        test    ecx, ecx
        jz      .done
        mov     ecx, 2849
        mov     edi, FLOPPY_FAT
        push    0       ; first cluster
.write_loop:
; allocate new cluster
        xor     eax, eax
        repnz scasw
        mov     al, ERROR_DISK_FULL
        jnz     .ret
        dec     edi
        dec     edi

        mov     eax, edi
        sub     eax, FLOPPY_FAT

        shr     eax, 1                  ; eax = cluster
        mov     word [edi], 0xFFF       ; mark as last cluster
        xchg    edi, [esp+4]
        cmp     dword [esp], 0
        jz      .first
        stosw
        jmp     @f
.first:
        mov     [esp], eax
@@:
        mov     edi, [esp+4]
        inc     ecx
; write data
        push    ecx edi
        mov     ecx, 512
        cmp     dword [esp+20], ecx
        jae     @f
        mov     ecx, [esp+20]
@@:
        mov     edi, FDD_BUFF
        cmp     byte [esp+24+28+28], 0
        jnz     .writedir
        push    ecx
        rep movsb
        pop     ecx
.writedircont:
        push    ecx
        sub     ecx, 512
        neg     ecx
        push    eax
        xor     eax, eax
        rep stosb
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
.writedir:
        push    ecx
        mov     ecx, 32/4
        push    ecx esi
        rep movsd
        pop     esi ecx
        mov     dword [edi-32], '.   '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        mov     word [edi-32+26], ax
        push    esi
        rep movsd
        pop     esi
        mov     dword [edi-32], '..  '
        mov     dword [edi-32+4], '    '
        mov     dword [edi-32+8], '    '
        mov     byte [edi-32+11], 10h
        mov     ecx, [esp+28+8]
        mov     word [edi-32+26], cx
        pop     ecx
        jmp     .writedircont

;----------------------------------------------------------------
;
;  fs_FloppyWrite - LFN variant for writing to floppy
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
fs_FloppyWrite.ret0:
        pop     eax
        xor     ebx, ebx
        ret

fs_FloppyWrite.ret11:
        push    11
        jmp     fs_FloppyWrite.ret0

fs_FloppyWrite:
        cmp     byte [esi], 0
        jz      @b
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jnz     .ret11
        pushad
        call    fd_find_lfn
        jnc     .found
        popad
        push    ERROR_FILE_NOT_FOUND
        jmp     .ret0
.found:
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

; extend file if needed
        add     ecx, ebx
        jc      .eof    ; FAT does not support files larger than 4GB
        push    eax     ; save directory cluster
        push    0       ; return value=0

        call    get_time_for_file
        mov     [edi+22], ax            ; last write time
        call    get_date_for_file
        mov     [edi+24], ax            ; last write date
        mov     [edi+18], ax            ; last access date

        push    dword [edi+28]          ; save current file size
        cmp     ecx, [edi+28]
        jbe     .length_ok
        cmp     ecx, ebx
        jz      .length_ok
        call    floppy_extend_file
        jnc     .length_ok
        mov     [esp+4], eax
; floppy_extend_file can return two error codes: FAT table error or disk full.
; First case is fatal error, in second case we may write some data
        cmp     al, ERROR_DISK_FULL
        jz      .disk_full
        pop     eax
        pop     eax
        mov     [esp+4+28], eax
        pop     eax
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
        pop     eax
        mov     [esp+4+28], eax ; eax=return value
        pop     eax
        sub     edx, [esp+20]
        mov     [esp+16], edx   ; ebx=number of written bytes
        popad
        ret
.length_ok:
; save FAT & directory
; note that directory must be saved first because save_flp_fat uses buffer at 0xD000
        mov     esi, [edi+28]
        movzx   edi, word [edi+26]      ; starting cluster
        mov     eax, [esp+8]
        pusha
        call    save_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .device_err
        call    save_flp_fat
        cmp     [FDC_Status], 0
        jz      @f
.device_err:
        mov     byte [esp+4], 11
        jmp     .ret
@@:

; now ebx=start pos, ecx=end pos, both lie inside file
        sub     ecx, ebx
        jz      .ret
        call    SetUserInterrupts
.write_loop:
; skip unmodified sectors
        cmp     dword [esp], 0x200
        jb      .modify
        sub     ebx, 0x200
        jae     .skip
        add     ebx, 0x200
.modify:
        lea     eax, [edi+31]   ; current sector
; get length of data in current sector
        push    ecx
        sub     ebx, 0x200
        jb      .hasdata
        neg     ebx
        xor     ecx, ecx
        jmp     @f
.hasdata:
        neg     ebx
        cmp     ecx, ebx
        jbe     @f
        mov     ecx, ebx
@@:
; load sector if needed
        cmp     dword [esp+4], 0        ; we don't need to read uninitialized data
        jz      .noread
        cmp     ecx, 0x200      ; we don't need to read sector if it is fully rewritten
        jz      .noread
        cmp     ecx, esi        ; (same for the last sector)
        jz      .noread
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jz      @f
.device_err2:
        pop     ecx
        jmp     .device_err
@@:
.noread:
; zero uninitialized data if file was extended (because floppy_extend_file does not this)
        push    eax ecx edi
        xor     eax, eax
        mov     ecx, 0x200
        sub     ecx, [esp+4+12]
        jbe     @f
        mov     edi, FDD_BUFF
        add     edi, [esp+4+12]
        rep stosb
@@:
; zero uninitialized data in the last sector
        mov     ecx, 0x200
        sub     ecx, esi
        jbe     @f
        mov     edi, FDD_BUFF
        add     edi, esi
        rep stosb
@@:
        pop     edi ecx eax
; copy new data
        push    eax
        mov     eax, edx
        neg     ebx
        jecxz   @f
        add     ebx, FDD_BUFF+0x200
        call    memmove
        xor     ebx, ebx
@@:
        pop     eax
; save sector
        pusha
        call    save_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .device_err2
        add     edx, ecx
        sub     [esp], ecx
        pop     ecx
        jz      .done
.skip:
.next_cluster:
        movzx   edi, word [edi*2+FLOPPY_FAT]
        sub     esi, 0x200
        jae     @f
        xor     esi, esi
@@:
        sub     dword [esp], 0x200
        jae     .write_loop
        and     dword [esp], 0
        jmp     .write_loop
.done:
        mov     [fdc_irq_func], fdc_null
        jmp     .ret

floppy_extend_file.zero_size:
        xor     eax, eax
        jmp     floppy_extend_file.start_extend

; extends file on floppy to given size (new data area is undefined)
; in: edi->direntry, ecx=new size
; out: CF=0 => OK, eax=0
;      CF=1 => error, eax=code (ERROR_FAT_TABLE or ERROR_DISK_FULL)
floppy_extend_file:
        push    ecx
; find the last cluster of file
        movzx   eax, word [edi+26]      ; first cluster
        mov     ecx, [edi+28]
        jecxz   .zero_size
@@:
        sub     ecx, 0x200
        jbe     @f
        mov     eax, [eax*2+FLOPPY_FAT]
        and     eax, 0xFFF
        jz      .fat_err
        cmp     eax, 0xFF8
        jb      @b
.fat_err:
        pop     ecx
        movi    eax, ERROR_FAT_TABLE
        stc
        ret
@@:
        push    eax
        mov     eax, [eax*2+FLOPPY_FAT]
        and     eax, 0xFFF
        cmp     eax, 0xFF8
        pop     eax
        jb      .fat_err
; set length to full number of sectors
        sub     [edi+28], ecx
.start_extend:
        pop     ecx
; now do extend
        push    edx esi
        mov     esi, FLOPPY_FAT+2*2       ; start scan from cluster 2
        mov     edx, 2847               ; number of clusters to scan
.extend_loop:
        cmp     [edi+28], ecx
        jae     .extend_done
; add new sector
        push    ecx
        push    edi
.scan:
        mov     ecx, edx
        mov     edi, esi
        jecxz   .disk_full
        push    eax
        xor     eax, eax
        repnz scasw
        pop     eax
        jnz     .disk_full
        mov     word [edi-2], 0xFFF
        mov     esi, edi
        mov     edx, ecx
        sub     edi, FLOPPY_FAT
        shr     edi, 1
        dec     edi     ; now edi=new cluster
        test    eax, eax
        jz      .first_cluster
        mov     [FLOPPY_FAT+eax*2], di
        jmp     @f
.first_cluster:
        pop     eax             ; eax->direntry
        push    eax
        mov     [eax+26], di
@@:
        mov     eax, edi        ; eax=new cluster
        pop     edi             ; edi->direntry
        pop     ecx             ; ecx=required size
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
        movi    eax, ERROR_DISK_FULL
        ret

;----------------------------------------------------------------
;
;  fs_FloppySetFileEnd - set end of file on floppy
;
;  esi  points to filename
;  ebx  points to 64-bit number = new file size
;  ecx  ignored (reserved)
;  edx  ignored (reserved)
;
;  ret eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_FloppySetFileEnd:
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jnz     ret11
        cmp     byte [esi], 0
        jnz     @f
.access_denied:
        push    ERROR_ACCESS_DENIED
        jmp     .ret
@@:
        push    edi
        call    fd_find_lfn
        jnc     @f
        pop     edi
        push    ERROR_FILE_NOT_FOUND
.ret:
        pop     eax
        jmp     .doret
@@:
; must not be directory
        test    byte [edi+11], 10h
        jz      @f
        pop     edi
        jmp     .access_denied
@@:
; file size must not exceed 4 Gb
        cmp     dword [ebx+4], 0
        jz      @f
        pop     edi
        push    ERROR_END_OF_FILE
        jmp     .ret
@@:
        push    eax
; set file modification date/time to current
        call    fat_update_datetime
        mov     eax, [ebx]
        cmp     eax, [edi+28]
        jb      .truncate
        ja      .expand
        pop     eax
        pushad
        call    save_chs_sector
        popad
        pop     edi
        xor     eax, eax
        cmp     [FDC_Status], 0
        jz      @f
        mov     al, 11
@@:
.doret:
        mov     [fdc_irq_func], fdc_null
        ret
.expand:
        push    ecx
        push    dword [edi+28]  ; save old size
        mov     ecx, eax
        call    floppy_extend_file
        push    eax     ; return code
        jnc     .expand_ok
        cmp     al, ERROR_DISK_FULL
        jz      .disk_full
        pop     eax ecx ecx edi edi
        jmp     .doret
.device_err:
        pop     eax
.device_err2:
        pop     ecx ecx eax edi
        push    11
        jmp     .ret
.disk_full:
.expand_ok:
; save directory & FAT
        mov     eax, [edi+28]
        xchg    eax, [esp+12]
        movzx   edi, word [edi+26]
        pusha
        call    save_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .device_err
        call    save_flp_fat
        cmp     [FDC_Status], 0
        jnz     .device_err
        call    SetUserInterrupts
; now zero new data
; edi = current cluster, [esp+12]=new size, [esp+4]=old size, [esp]=return code
.zero_loop:
        sub     dword [esp+4], 0x200
        jae     .next_cluster
        cmp     dword [esp+4], -0x200
        jz      .noread
        lea     eax, [edi+31]
        pusha
        call    read_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .err_next
.noread:
        mov     ecx, [esp+4]
        neg     ecx
        push    edi
        mov     edi, FDD_BUFF+0x200
        add     edi, [esp+8]
        xor     eax, eax
        mov     [esp+8], eax
        rep stosb
        pop     edi
        lea     eax, [edi+31]
        pusha
        call    save_chs_sector
        popa
        cmp     [FDC_Status], 0
        jz      .next_cluster
.err_next:
        mov     byte [esp], 11
.next_cluster:
        sub     dword [esp+12], 0x200
        jbe     .expand_done
        movzx   edi, word [FLOPPY_FAT+edi*2]
        jmp     .zero_loop
.expand_done:
        pop     eax ecx ecx edi edi
        jmp     .doret
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
        movzx   ecx, word [FLOPPY_FAT+ecx*2]
        jmp     @b
@@:
; we will zero data at the end of last sector - remember it
        push    ecx
; terminate FAT chain
        lea     ecx, [FLOPPY_FAT+ecx+ecx]
        push    dword [ecx]
        mov     word [ecx], 0xFFF
        pop     ecx
        and     ecx, 0xFFF
        jmp     .delete
.zero_size:
        and     word [edi+26], 0
        push    0
.delete:
; delete FAT chain starting with ecx
; mark all clusters as free
        cmp     ecx, 0xFF8
        jae     .deleted
        lea     ecx, [FLOPPY_FAT+ecx+ecx]
        push    dword [ecx]
        and     word [ecx], 0
        pop     ecx
        and     ecx, 0xFFF
        jmp     .delete
.deleted:
        mov     edi, [edi+28]
; save directory & FAT
        mov     eax, [esp+8]
        pusha
        call    save_chs_sector
        popa
        cmp     [FDC_Status], 0
        jnz     .device_err2
        call    save_flp_fat
        cmp     [FDC_Status], 0
        jnz     .device_err2
; zero last sector, ignore errors
        pop     eax
        add     eax, 31
        and     edi, 0x1FF
        jz      .truncate_done
        call    SetUserInterrupts
        pusha
        call    read_chs_sector
        popa
        add     edi, FDD_BUFF
        mov     ecx, FDD_BUFF+0x200
        sub     ecx, edi
        push    eax
        xor     eax, eax
        rep stosb
        pop     eax
        pusha
        call    save_chs_sector
        popa
.truncate_done:
        pop     ecx eax edi
        xor     eax, eax
        jmp     .doret

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
        pusha
        call    save_chs_sector
        popa
        pop     edi
        xor     eax, eax
        cmp     [FDC_Status], al
        jz      @f
        mov     al, 11
@@:
        ret

;----------------------------------------------------------------
;
;  fs_FloppyDelete - delete file or empty folder from floppy
;
;  esi  points to filename
;
;  ret  eax = 0 ok or other = errormsg
;
;--------------------------------------------------------------
fs_FloppyDelete:
        call    read_flp_fat
        cmp     [FDC_Status], 0
        jz      @f
        push    11
        jmp     .pop_ret
@@:
        cmp     byte [esi], 0
        jnz     @f
; cannot delete root!
.access_denied:
        push    ERROR_ACCESS_DENIED
.pop_ret:
        pop     eax
        ret
@@:
        and     [fd_prev_sector], 0
        and     [fd_prev_prev_sector], 0
        push    edi
        call    fd_find_lfn
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
        push    eax
        movzx   eax, word [edi+26]
        push    ebx
        pusha
        add     eax, 31
        call    read_chs_sector
        popa
        mov     ebx, FDD_BUFF + 2*0x20
.checkempty:
        cmp     byte [ebx], 0
        jz      .empty
        cmp     byte [ebx], 0xE5
        jnz     .notempty
        add     ebx, 0x20
        cmp     ebx, FDD_BUFF + 0x200
        jb      .checkempty
        movzx   eax, word [FLOPPY_FAT + eax*2]
        pusha
        add     eax, 31
        call    read_chs_sector
        popa
        mov     ebx, FDD_BUFF
        jmp     .checkempty
.notempty:
        pop     ebx
        pop     eax
.access_denied2:
        pop     edi
        jmp     .access_denied
.empty:
        pop     ebx
        pop     eax
        pusha
        call    read_chs_sector
        popa
.dodel:
        push    eax
        movzx   eax, word [edi+26]
        xchg    eax, [esp]
; delete folder entry
        mov     byte [edi], 0xE5
; delete LFN (if present)
.lfndel:
        cmp     edi, FDD_BUFF
        ja      @f
        cmp     [fd_prev_sector], 0
        jz      .lfndone
        push    [fd_prev_sector]
        push    [fd_prev_prev_sector]
        pop     [fd_prev_sector]
        and     [fd_prev_prev_sector], 0
        pusha
        call    save_chs_sector
        popa
        pop     eax
        pusha
        call    read_chs_sector
        popa
        mov     edi, FDD_BUFF+0x200
@@:
        sub     edi, 0x20
        cmp     byte [edi], 0xE5
        jz      .lfndone
        cmp     byte [edi+11], 0xF
        jnz     .lfndone
        mov     byte [edi], 0xE5
        jmp     .lfndel
.lfndone:
        pusha
        call    save_chs_sector
        popa
; delete FAT chain
        pop     eax
@@:
        cmp     eax, 2
        jb      .done
        cmp     eax, 0xFF8
        jae     .done
        lea     eax, [FLOPPY_FAT + eax*2]
        push    dword [eax]
        and     word [eax], 0
        pop     eax
        and     eax, 0xFFF
        jmp     @b
.done:
        call    save_flp_fat
        pop     edi
        xor     eax, eax
        ret

; \end{diamond}
