;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

ERROR_SUCCESS        = 0
ERROR_DISK_BASE      = 1
ERROR_UNSUPPORTED_FS = 2
ERROR_UNKNOWN_FS     = 3
ERROR_PARTITION      = 4
ERROR_FILE_NOT_FOUND = 5
ERROR_END_OF_FILE    = 6
ERROR_MEMORY_POINTER = 7
ERROR_DISK_FULL      = 8
ERROR_FAT_TABLE      = 9 ;deprecated
ERROR_FS_FAIL        = 9
ERROR_ACCESS_DENIED  = 10
ERROR_DEVICE         = 11
ERROR_OUT_OF_MEMORY  = 12

image_of_eax EQU esp+32
image_of_ebx EQU esp+20

; System function 70

file_system_lfn_protected:
        pushad
        call    protect_from_terminate
        call    file_system_lfn
        call    unprotect_from_terminate
        popad
        mov     [image_of_eax], eax
        mov     [image_of_ebx], ebx
        ret

file_system_lfn:
; in: ebx -> parameter structure
;   operation codes:
; 0 = read file
; 1 = read folder
; 2 = create/rewrite file
; 3 = write/append to file
; 4 = set file end
; 5 = get file info
; 6 = set file info
; start application
; 8 = delete file/folder
; 9 = create folder
        lea     esi, [ebx+20]
        lodsb
        test    al, al
        jnz     @f
        mov     esi, [esi]
        lodsb
@@:
        lea     ebp, [esi-1]
if 0
        cmp     [ebx], dword 0
        jne     .1
        DEBUGF 1,'read file %s\n',ebp
        jmp     @f
.1:
        cmp     [ebx], dword 5
        jne     @f
        DEBUGF 1,'get file attributes %s\n',ebp
@@:
end if
        cmp     dword[ebx], 7   ; start application
        jne     @f
        mov     edx, [ebx+4]
        mov     ebx, [ebx+8]
        call    fs_execute      ; ebp, ebx, edx
        mov     [image_of_eax], eax
        ret

@@:
        cmp     al, '/'
        jz      .notcurdir
        dec     esi
        mov     ebp, esi
        test    al, al
        jnz     @f
        xor     ebp, ebp
@@:
        mov     esi, [current_slot]
        mov     esi, [esi+APPDATA.cur_dir]
        jmp     .parse_normal

.notcurdir:
        cmp     byte [esi], 0
        jz      .rootdir
        call    process_replace_file_name
.parse_normal:
        mov     ax, [esi]
        or      ax, 2020h
        cmp     ax, 'cd'
        jz      .CD
        call    dyndisk_handler ; not returns if success
.notfound:
        mov     dword[image_of_eax], ERROR_FILE_NOT_FOUND
        ret

.CD:
        add     esi, 2
        xor     eax, eax
        lodsb       ; disk number
        sub     eax, '0'
        cmp     eax, 10
        jnc     .notfound
        mov     edi, eax
        lodsb
        test    eax, eax
        jz      .maindir
        cmp     al, '/'
        jnz     .notfound
        lodsb       ; partition number
        test    eax, eax
        jz      .maindir
        cmp     al, '1'
        jnz     .notfound
        cmp     byte [esi], '/'
        jnz     @f
        inc     esi
@@:
        cmp     byte [esi], 0
        jnz     @f
        test    ebp, ebp
        jz      @f
        mov     esi, ebp
        xor     ebp, ebp
@@:
        call    reserve_cd
        mov     eax, edi
        bt      eax, 0
        setc    [DiskNumber]
        bt      eax, 1
        setc    [ChannelNumber]
        inc     [ChannelNumber]
        inc     eax
        mov     [cdpos], eax
        call    reserve_cd_channel
        mov     eax, edi
        not     eax
        and     eax, 3
        shl     eax, 1
        inc     eax
        shr     edi, 2
        mov     dword[image_of_eax], ERROR_FILE_NOT_FOUND
        bt      [edi*5+DRIVE_DATA+1], ax
        jnc     @f
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        mov     eax, [ebx]
        mov     dword[image_of_eax], ERROR_UNSUPPORTED_FS
        cmp     eax, fs_NumCdServices
        jae     @f
        add     ebx, 4
        call    dword[fs_CdServices + eax*4]
        call    free_cd_channel
        and     [cd_status], 0
        mov     [image_of_eax], eax
        mov     [image_of_ebx], ebx
        ret

@@:
        call    free_cd_channel
        and     [cd_status], 0
        ret

.nextCD:
        test    eax, eax    ; partition number
        stc
        jnz     @f      ; no more partitions
        mov     al, 1   ; /cdX/1
        clc
@@:
        ret

.maindir:   ; list partitions
        mov     esi, .nextCD
.maindir_noesi:     ; backjump from dyndisk_handler
        cmp     dword[ebx], 1
        jnz     .access_denied  ; read folder?
        push    ecx
        mov     ebp, [ebx+12]   ; the number of blocks to read
        mov     edx, [ebx+16]   ; where to write the result
        push    dword[ebx+4]    ; first block
        mov     ebx, [ebx+8]    ; flags
        mov     ecx, 32/4
        mov     edi, edx
        xor     eax, eax
        rep stosd
        mov     byte [edx], 1   ; version
.maindir_loop:
        call    esi
        jc      .maindir_done
        inc     dword[edx+8]
        dec     dword[esp]
        jns     .maindir_loop
        dec     ebp
        js      .maindir_loop
        inc     dword[edx+4]
        mov     dword[edi], 16      ; attributes: folder
        mov     dword[edi+4], ebx   ; name encoding
        push    eax
        mov     ecx, 32/4
        add     edi, 8
        xor     eax, eax
        rep stosd
        pop     eax
        push    eax edx edi
; convert number in eax to decimal string
        push    -'0'
        mov     ecx, 10
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
        cmp     ebx, 1
        jz      .uni
@@:
        pop     eax
        add     eax, '0'
        stosb
        test    eax, eax
        jnz     @b
        pop     edi edx eax
        add     edi, 264
        jmp     .maindir_loop

.uni:
        pop     eax
        add     eax, '0'
        stosw
        test    eax, eax
        jnz     .uni
        pop     edi edx eax
        add     edi, 520
        jmp     .maindir_loop

.maindir_done:
        pop     eax eax
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ebp
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        mov     [image_of_eax], eax
        mov     [image_of_ebx], ebx
        ret

.access_denied:
        mov     dword[image_of_eax], ERROR_ACCESS_DENIED
        ret

.rootdir:   ; / - virtual root folder
        cmp     dword[ebx], 1   ; read folder?
        jnz     .access_denied
        mov     ebp, [ebx+12]   ; number of blocks
        mov     edx, [ebx+16]   ; return area
        push    dword[ebx+4]    ; first block
        mov     ebx, [ebx+8]    ; flags
        mov     ecx, 32/4
        mov     edi, edx
        xor     eax, eax
        rep stosd
        mov     byte [edx], 1   ; version
        sub     esp, 16
.rootdir_loop:
        push    edi
        lea     edi, [esp+4]
        call    dyndisk_enum_root
        pop     edi
        test    eax, eax
        jz      .rootdirCD
        inc     dword[edx+8]
        dec     dword[esp+16]
        jns     .rootdir_loop
        dec     ebp
        js      .rootdir_loop
        inc     dword[edx+4]
        mov     dword[edi], 16      ; attributes: folder
        mov     dword[edi+4], ebx   ; name encoding
        push    eax
        mov     ecx, 32/4
        add     edi, 8
        xor     eax, eax
        rep stosd
        push    esi edi
        lea     esi, [esp+12]
        cmp     ebx, 1
        jz      .uni2
@@:
        lodsb
        stosb
        test    eax, eax
        jnz     @b
        pop     edi esi eax
        add     edi, 264
        jmp     .rootdir_loop

.uni2:
        lodsb
        stosw
        test    eax, eax
        jnz     .uni2
        pop     edi esi eax
        add     edi, 520
        jmp     .rootdir_loop

.rootdirCD:
        add     esp, 16
        or      esi, -1
.rootdirCD_loop:
        inc     esi
        cmp     esi, 10
        jnc     .rootdir_done
        mov     eax, esi
        not     eax
        and     eax, 3
        shl     eax, 1
        inc     eax
        mov     ecx, esi
        shr     ecx, 2
        bt      [ecx*5+DRIVE_DATA+1], ax
        jnc     .rootdirCD_loop
        inc     dword[edx+8]
        dec     dword[esp]
        jns     .rootdirCD_loop
        dec     ebp
        js      .rootdirCD_loop
        inc     dword[edx+4]
        mov     dword[edi], 16      ; attributes: folder
        mov     dword[edi+4], ebx   ; name encoding
        mov     ecx, 32/4
        add     edi, 8
        xor     eax, eax
        rep stosd
        mov     eax, esi
        add     eax, '0'
        cmp     ebx, 1
        jz      @f
        mov     word [edi], 'cd'
        mov     [edi+2], ax
        add     edi, 264
        jmp     .rootdirCD_loop

@@:
        mov     dword[edi], 640063h
        mov     [edi+4], eax
        add     edi, 520
        jmp     .rootdirCD_loop

.rootdir_done:
        pop     eax
        mov     ebx, [edx+4]
        xor     eax, eax
        dec     ebp
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        mov     [image_of_eax], eax
        mov     [image_of_ebx], ebx
        ret

fs_NotImplemented:
        mov     eax, 2
        ret

;-----------------------------------------------------------------------------
fs_CdServices:
        dd      fs_CdRead
        dd      fs_CdReadFolder
        dd      fs_NotImplemented
        dd      fs_NotImplemented
        dd      fs_NotImplemented
        dd      fs_CdGetFileInfo
        dd      fs_NotImplemented
        dd      0
        dd      fs_NotImplemented
        dd      fs_NotImplemented
fs_NumCdServices = ($ - fs_CdServices)/4
;-----------------------------------------------------------------------------
process_replace_file_name:
; in: [esi] = virtual path
; out: [esi]+[ebp] = physical path
        pushfd
        cli
        mov     ebp, [full_file_name_table]
        xor     edi, edi
.loop:
        cmp     edi, [full_file_name_table.size]
        jae     .notfound
        push    esi edi
        shl     edi, 7
        add     edi, ebp
@@:
        cmp     byte [edi], 0
        jz      .dest_done
        lodsb
        test    al, al
        jz      .cont
        or      al, 20h
        scasb
        jz      @b
.cont:
        pop     edi esi
        inc     edi
        jmp     .loop

.dest_done:
        cmp     byte [esi], 0
        jz      .found
        cmp     byte [esi], '/'
        jnz     .cont
        inc     esi
.found:
        pop     edi eax
        shl     edi, 7
        add     edi, ebp
        mov     ebp, esi
        cmp     byte [esi], 0
        lea     esi, [edi+64]
        jnz     .ret
.notfound:
        xor     ebp, ebp
.ret:
        popfd
        ret

;-----------------------------------------------------------------------------
uglobal
lock_flag_for_f30_3 rb 1
endg

sys_current_directory:  ; sysfunction 30
        mov     eax, [current_slot]
        mov     edi, [eax+APPDATA.cur_dir]
        dec     ebx
        jz      .set
        dec     ebx
        jz      .get
        dec     ebx
        jz      .mount_additional_directory
        ret

.mount_additional_directory:
; in: ecx -> dir name+dir path (128)
        cmp     [lock_flag_for_f30_3], 1    ; check lock
        je      @f
        mov     esi, ecx
        mov     edi, sysdir_name1
        mov     ecx, 63
        pushfd
        cli
        cld
        rep movsb   ; copying fake directory name
        inc     esi
        xor     eax, eax
        stosb       ; terminator of name, in case if we get the inlet trash
        mov     ecx, 63
        rep movsb   ; copying real directory path for mounting
        xor     eax, eax
        stosb
; increase the pointer of inputs for procedure "process_replace_file_name"
        mov     [full_file_name_table.size], 2
        mov     [lock_flag_for_f30_3], 1    ; lock
        popfd
@@:
        ret

max_cur_dir equ 0x1000  ; length

.get:       ; in: ecx -> buffer, edx = length
        mov     ebx, edi    ; buffer
        push    ecx
        push    edi
        xor     eax, eax
        mov     ecx, max_cur_dir
        repne scasb
        jnz     .error
        sub     edi, ebx
        inc     edi
        mov     [esp+32+8], edi ; return in eax
        cmp     edx, edi
        jbe     @f
        mov     edx, edi
@@:
        pop     esi
        pop     edi
        cmp     edx, 1
        jbe     .ret
        mov     al, '/'
        stosb
        mov     ecx, edx
        rep movsb
.ret:
        ret

.error:
        add     esp, 8
        or      dword[esp+32], -1
        ret

.set:
        pop     eax
        push    max_cur_dir
        push    edi
        push    eax
        mov     ebx, ecx
get_full_file_name:
; in: ebx -> file name, [esp+4] -> destination, [esp+8] = max length
; destroys all registers
        push    ebp
        mov     esi, [current_slot]
        mov     esi, [esi+APPDATA.cur_dir]
        mov     edx, esi
@@:
        inc     esi
        cmp     byte [esi-1], 0
        jnz     @b
        dec     esi
        cmp     byte [ebx], '/'
        jz      .set_absolute
        mov     edi, [esp+8]
.relative:
        cmp     byte [ebx], 0
        jz      .set_ok
        cmp     word [ebx], '.'
        jz      .set_ok
        cmp     word [ebx], './'
        jnz     @f
        add     ebx, 2
        jmp     .relative

@@:
        cmp     word [ebx], '..'
        jnz     .doset_relative
        cmp     byte [ebx+2], 0
        jz      @f
        cmp     byte [ebx+2], '/'
        jnz     .doset_relative
@@:
        dec     esi
        cmp     byte [esi], '/'
        jnz     @b
        add     ebx, 3
        jmp     .relative

.set_ok:
        cmp     edx, edi        ; is destination equal to APPDATA.cur_dir?
        jz      .set_ok.cur_dir
        sub     esi, edx
        cmp     esi, [esp+12]
        jb      .set_ok.copy
.fail:
        mov     byte [edi], 0
        xor     eax, eax
        pop     ebp
        ret     8

.set_ok.copy:
        mov     ecx, esi
        mov     esi, edx
        rep movsb
        mov     byte [edi], 0
.ret.ok:
        mov     al, 1
        pop     ebp
        ret     8

.set_ok.cur_dir:
        mov     byte [esi], 0
        jmp     .ret.ok

.doset_relative:
        cmp     edx, edi
        jz      .doset_relative.cur_dir
        sub     esi, edx
        cmp     esi, [esp+12]
        jae     .fail
        mov     ecx, esi
        mov     esi, edx
        mov     edx, edi
        rep movsb
        jmp     .doset_relative.copy

.doset_relative.cur_dir:
        mov     edi, esi
.doset_relative.copy:
        add     edx, [esp+12]
        mov     byte [edi], '/'
        inc     edi
        cmp     edi, edx
        jae     .overflow
@@:
        mov     al, [ebx]
        inc     ebx
        stosb
        test    al, al
        jz      .ret.ok
        cmp     edi, edx
        jb      @b
.overflow:
        dec     edi
        jmp     .fail

.set_absolute:
        lea     esi, [ebx+1]
        call    process_replace_file_name
        mov     edi, [esp+8]
        mov     edx, [esp+12]
        add     edx, edi
.set_copy:
        lodsb
        stosb
        test    al, al
        jz      .set_part2
.set_copy_cont:
        cmp     edi, edx
        jb      .set_copy
        jmp     .overflow

.set_part2:
        mov     esi, ebp
        xor     ebp, ebp
        test    esi, esi
        jz      .ret.ok
        mov     byte [edi-1], '/'
        jmp     .set_copy_cont

include "parse_fn.inc"
include "fs_common.inc"
include "iso9660.inc"   ; read for CD filesystem
include "fat.inc"
include "ntfs.inc"
include "ext.inc"
include "xfs.asm"
