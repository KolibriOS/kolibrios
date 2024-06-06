;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


ERROR_SUCCESS        = 0
ERROR_DISK_BASE      = 1
ERROR_UNSUPPORTED_FS = 2
ERROR_UNKNOWN_FS     = 3
ERROR_PARTITION      = 4
ERROR_FILE_NOT_FOUND = 5
ERROR_END_OF_FILE    = 6
ERROR_MEMORY_POINTER = 7
ERROR_DISK_FULL      = 8
ERROR_FS_FAIL        = 9
ERROR_ACCESS_DENIED  = 10
ERROR_DEVICE         = 11

maxPathLength = 1000h

image_of_eax EQU esp+32
image_of_ebx EQU esp+20

; fs api for drivers
struct  FileSystem
        Next            dd ?
        Prev            dd ?
        Creat_part      dd ?  ; %FSNAME%_create_partition
        UserFuncs       dd ? ; for fs_del
        Name            rd 16 ;ascii string + \n
ends


fs_list:
        dd      fs_list
        dd      fs_list
;.look:  dd      0

; IN: ecx = %FSNAME%_create_partition
;     edx = ptr to UserFuncs
;     [esp] = fs name
;OUT: eax = list item
fs_add:
        push    ecx edx
        ; add in fs_list
        mov     eax, sizeof.FileSystem
        call    malloc
        pop     edx ecx
        test    eax, eax
        jz      .err

        mov     [eax + FileSystem.Creat_part], ecx
        mov     [eax + FileSystem.UserFuncs], edx
        mov     edx, [esp + 4]
        mov     [eax + FileSystem.Name], edx
        mov     edx, eax

        cli          ; DELETE
        list_add_tail edx, fs_list
        sti          ; DELETE
        mov     edx, ecx ; save function

        ;DEBUGF  1, 'K : FS: find partition\n'
        ; check all disks
        mov     esi, [disk_list]
.new_disk:
        cmp     dword[esi], disk_list
        jz      .end

        push    edx
        mov     eax, [esi + DISK.MediaInfo.SectorSize]
        shl     eax, 2
        stdcall kernel_alloc, eax   ;get buffer
        test    eax, eax
        pop     edx
        jz      .end ; no memory
        mov     ebx, eax

        mov     ecx, [esi + DISK.NumPartitions]
        dec     ecx
        shl     ecx, 2 ; to dword
        add     ecx, [esi + DISK.Partitions]
@@:
        mov     ebp, [ecx]
        cmp     [ebp + PARTITION.FSUserFunctions], default_fs_functions
        jnz     .no_fs

        ;DEBUGF  1, 'K : FS: found partition\n'
        push    ecx edx

        xor     eax, eax     ; first sector of the partition
        call    fs_read32_sys
        push    eax
        ;DEBUGF  1, 'K : FS: call driver func = %x\n', edx

        call    edx ; creat_partition
        add     esp, 4
        ;DEBUGF  1, 'K : FS: end call\n'
        pop     edx ecx

        test    eax, eax
        jz      .no_fs
        ; save and delete old struct
        xchg    [ecx], eax
        push    ecx edx
        call    free
        pop     edx ecx
        ;DEBUGF  1, 'K : FS: set fs for partition\n'
.no_fs:
        ;sub     ecx, 4
        cmp     ecx, [esi + DISK.Partitions]
        lea     ecx, [ecx - 4]
        jnz     @b

        push    edx
        stdcall kernel_free, ebx
        pop     edx

        mov     esi, [esi]
        jmp     .new_disk
.end:
.err:
        ret
; IN: ecx = list item
;OUT: -
;fs_del:
;
;        ret



; System function 70 security check
align 4
proc file_system_is_operation_safe stdcall, inf_struct_ptr: dword
; in:
;      inf_struct_ptr = pointer to information structure was given to sysfn70
; out: ZF = 1 if operation is safe
;      ZF = 0 if operation can cause kernel crash
        push    ebx ecx edx
        xor     ecx, ecx ; ecx - length of target buffer

        mov     ebx, [inf_struct_ptr]
        mov     edx, [ebx + 16] ; base of target buffer

        cmp     dword [ebx], 0 ; if 70.0
        jnz     .case1
        mov     ecx, dword [ebx + 12]
        jmp     .end_switch

.case1:
        cmp     dword [ebx], 1 ; if 70.1
        jnz     .case2_3
        ;mov     ecx, 32
        cmp     dword [ebx + 8], 1 ; check encoding
        jbe     .case1_304 ; if encdoing <= 1 i.e cpp866
        mov     ecx, 560 ; if unicode then bdvk block len is 560 bytes
        jmp     .case1_end
.case1_304:
        mov     ecx, 304 ; if cp866 then bdvk block len is 304 bytes
.case1_end:
        imul    ecx, dword [ebx + 12] ; multiply bdvk length by their count
        add     ecx, 32 ; add result header len
        jmp     .end_switch

.case2_3:
        cmp     dword [ebx], 3
        ja      .case5 ; if subfn > 3
        mov     ecx, dword [ebx + 12]
        jmp     .end_switch

.case5:
        cmp     dword [ebx], 5
        jnz     .case6
        mov     ecx, 40
        jmp     .end_switch

.case6:
        cmp     dword [ebx], 6
        jnz     .switch_none
        mov     ecx, 32
        jmp     .end_switch

.switch_none:
        cmp     ecx, ecx
        jmp     .ret

.end_switch:
        stdcall is_region_userspace, edx, ecx
.ret:
        pop     edx ecx ebx
        ret
endp

sys_fileSystemUnicode: ; with user pointer correctness checking
; in: ebx -> f.80 parameter structure
        stdcall file_system_is_operation_safe, ebx
        jz      @f

        DEBUGF  1, "sysfn80 addr error\n"
        mov     dword [image_of_eax], ERROR_MEMORY_POINTER
        ret
@@:
        jmp     fileSystemUnicode

;System function 70
sys_file_system_lfn: ; with user pointer correctness checking
; in: ebx -> f.70 parameter structure
        stdcall file_system_is_operation_safe, ebx
        jz      @f

        DEBUGF  1, "sysfn70 addr error\n"
        mov     dword [image_of_eax], ERROR_MEMORY_POINTER
        ret
@@:
        jmp     file_system_lfn

;file_system_lfn_protected returns values not in registers, but in their images
;on stack. Make a short wrapper to actually return values in registers.
file_system_lfn_protected_registers:
        pushad
        call    file_system_lfn_protected
        popad
        ret

file_system_lfn_protected:
        pushad
        call    protect_from_terminate
        call    file_system_lfn
        call    unprotect_from_terminate
        popad
        mov     [image_of_eax], eax
        mov     [image_of_ebx], ebx
        ret

fileSystemUnicode:
; in: ebx -> f.80 parameter structure
        mov     edi, [ebx+20]
        mov     esi, [ebx+24]
        jmp     @f

file_system_lfn:
; in: ebx -> f.70 parameter structure
        xor     edi, edi
        lea     esi, [ebx+20]
        cmp     byte [esi], 0
        jnz     @f
        mov     esi, [ebx+21]
@@:
        cmp     word [esi], '/'
        jnz     @f
        cmp     edi, 2
        jnz     .rootdir
        cmp     dword[esi], '/'
        jz      .rootdir
@@:
        stdcall kernel_alloc, maxPathLength
        push    eax ebx
        xchg    eax, edi
        call    getFullPath
        pop     ebx ebp
        test    eax, eax
        jz      .notfound
        cmp     dword[ebx], 7   ; start application
        jnz     @f
        mov     edx, [ebx+4]
        mov     ecx, [ebx+8]
        mov     ebx, ebp
        call    fs_execute
        mov     [image_of_eax], eax
        ret

@@:
        lea     esi, [ebp+2]
        mov     ax, [esi]
        or      ax, 2020h
        cmp     ax, 'cd'
        jz      .CD
        call    dyndisk_handler ; not returns if success
.notfound:
        stdcall kernel_free, ebp
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
        push    ebp
        call    dword[fs_CdServices + eax*4]
        pop     ebp
        mov     [image_of_eax], eax
        mov     [image_of_ebx], ebx
@@:
        call    free_cd_channel
        and     [cd_status], 0
        stdcall kernel_free, ebp
        ret

.nextCD:
        test    eax, eax    ; partition number
        jnz     @f
        inc     eax     ; /cdX/1
        ret

@@:
        stc
        ret

.maindir:   ; list partitions
        mov     esi, .nextCD
        xor     ecx, ecx
.maindir_noesi:     ; backjump from dyndisk_handler
        push    ebp
        mov     ebp, ecx
        call    kernel_free
        mov     edi, [ebx+16]   ; buffer
        cmp     byte [ebx], 5
        jz      .deviceInfo
        cmp     byte [ebx], 1   ; read folder?
        jnz     .access_denied
        push    ebp
        pushd   [ebx+4]         ; first block
        mov     ebp, [ebx+12]   ; the number of blocks to read
        mov     ebx, [ebx+8]    ; flags
        mov     ecx, 32/4
        mov     edx, edi
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
        cmp     ebx, 2
        jz      .uni
@@:
        pop     eax
        add     eax, '0'
        stosb
        test    eax, eax
        jnz     @b
        pop     edi edx eax
        cmp     ebx, 3
        jz      @f
        add     edi, 264
        jmp     .maindir_loop

.uni:
        pop     eax
        add     eax, '0'
        stosw
        test    eax, eax
        jnz     .uni
        pop     edi edx eax
@@:
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

.deviceInfo:
        test    ebp, ebp
        jz      @f
        mov     eax, dword[ebp+DISK.MediaInfo.Capacity]
        mov     edx, dword[ebp+DISK.MediaInfo.Capacity+4]
        mov     ecx, [ebp + DISK.MediaInfo.SectorSize]
        bsf     ecx, ecx
        shld    edx, eax, cl
        shl     eax, cl
        mov     [edi+36], edx
        mov     [edi+32], eax
@@:
        and     dword[image_of_eax], 0
        ret

.rootdir:   ; / - virtual root folder
        cmp     byte [ebx], 5
        jz      @b
        cmp     byte [ebx], 1   ; read folder?
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
        push    edi
        lea     esi, [esp+8]
        cmp     ebx, 2
        jz      .uni2
@@:
        lodsb
        stosb
        test    eax, eax
        jnz     @b
        pop     edi eax
        cmp     ebx, 3
        jz      @f
        add     edi, 264
        jmp     .rootdir_loop

.uni2:
        lodsb
        stosw
        test    eax, eax
        jnz     .uni2
        pop     edi eax
@@:
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

;-----------------------------------------------------------------------------
process_replace_file_name:
; in: [esi] = virtual path
; out: [esi]+[ebp] = physical path
        xor     edi, edi
        xor     ebp, ebp
.loop:
        cmp     edi, [full_file_name_table.size]
        jae     .notfound
        push    esi edi
        shl     edi, 7
        add     edi, [full_file_name_table]
@@:
        cmp     byte [edi], 0
        jz      .dest_done
        lodsb
        test    al, al
        jz      .cont
        scasb
        jz      @b
        or      al, 20h
        cmp     [edi-1], al
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
.found:
        pop     edi eax
        shl     edi, 7
        add     edi, [full_file_name_table]
        mov     ebp, esi
        lea     esi, [edi+64]
.notfound:
        ret

;-----------------------------------------------------------------------------
uglobal
addDirSeal db  ?
endg

sys_current_directory:  ; sysfunction 30
        mov     eax, [current_slot]
        mov     edi, [eax+APPDATA.cur_dir]
        xor     eax, eax
        dec     ebx
        jz      .set
        dec     ebx
        jz      .get
        dec     ebx
        jz      .mount_additional_directory
        mov     eax, edx
        dec     ebx
        jz      .set
        mov     eax, esi
        dec     ebx
        jz      .get
@@:
        ret

.mount_additional_directory:
; in: ecx -> dir name+dir path (128)
        mov     al, 1
        xchg    [addDirSeal], al
        test    al, al
        jnz     @b
        mov     esi, ecx
        mov     edi, sysdir_name1
        mov     ecx, 64
        rep movsb   ; copying fake directory name
        mov     byte [edi-1], 0
        mov     cl, 63
        call    cp866toUTF8_string
        mov     byte [edi], 0
        mov     [full_file_name_table.size], 2
        ret

.get:
; in: ecx -> buffer, edx = length, eax = encoding
        stdcall is_region_userspace, ecx, edx
        jz      @f

        ; if illegal buffer given
        xor     edx, edx
        jmp     .ret
@@:

        mov     esi, edi
        inc     esi
        mov     edi, ecx
        cmp     edx, maxPathLength
        jc      @f
        mov     edx, maxPathLength
@@:
        mov     ecx, edx
        jecxz   .ret
        cmp     eax, 2
        jz      .get16
        cmp     eax, 3
        jz      .get8
@@:
        dec     ecx
        js      @f
        call    utf8to16
        call    uni2ansi_char
        stosb
        test    al, al
        jnz     @b
        sub     edx, ecx
@@:
        mov     byte [edi-1], 0
.ret:
        mov     [esp+32], edx
        ret

.get8:
        push    edi
        mov     edi, esi
        xor     eax, eax
        repnz scasb
        sub     edx, ecx
        mov     ecx, edx
        pop     edi
        rep movsb
        jmp     @b

.get16:
        shr     ecx, 1
        shr     edx, 1
@@:
        dec     ecx
        js      @f
        call    utf8to16
        stosw
        test    ax, ax
        jnz     @b
        sub     edx, ecx
@@:
        shl     edx, 1
        mov     word [edi-2], 0
        jmp     .ret

.set:
        mov     esi, ecx
getFullPath:
; in: esi -> file path, eax = string encoding, edi -> destination
; out: UTF-8 string (with marker), eax = length, 0 -> error
        test    eax, eax
        jnz     @f
        cmp     byte [esi], 4
        jnc     @f
        cmp     byte [esi], 0
        jz      @f
        lodsb
@@:
        cmp     byte [esi], '/'
        jnz     .relative
        cmp     eax, 2
        jnz     @f
        cmp     word [esi], '/'
        jnz     .relative
        inc     esi
        inc     esi
        jmp     .start

@@:
        inc     esi
        cmp     byte [esi], 4
        jnc     .start
        lodsb
        cmp     byte [esi], '/'
        jnz     .start
        inc     esi
.start:
        push    eax edi
        call    process_replace_file_name
        mov     edi, [esp]
        mov     ecx, maxPathLength
        mov     al, 3
        mov     ah, '/'
        stosw
        sub     ecx, 2
        test    ebp, ebp
        jz      .absolute
@@:
        lodsb
        stosb
        dec     ecx
        test    al, al
        jnz     @b
        mov     esi, ebp
        dec     edi
.absolute:
        cmp     byte [esp+4], 2
        jz      .utf16
        cmp     byte [esp+4], 3
        jz      .utf8
        call    cp866toUTF8_string
        jns     .end
        jmp     .fail

.utf8:
        dec     ecx
        js      .fail
        lodsb
        stosb
        test    al, al
        jz      .end
        jmp     .utf8

.utf16:
        call    UTF16to8_string
        jns     .end
.fail:
        mov     byte [edi], 0
        pop     eax eax
        xor     eax, eax
        ret

.relative:
        push    eax edi
        mov     ebx, esi
        mov     edi, [current_slot]
        mov     edi, [edi+APPDATA.cur_dir]
        mov     edx, edi
        mov     ecx, maxPathLength
        xor     eax, eax
        repnz scasb
        mov     esi, edi
        mov     edi, [esp]
        jecxz   .fail
        cmp     byte [ebx], 0
        jz      .set_ok
        dec     esi
        cmp     edx, edi    ; is destination equal to cur_dir?
        mov     edi, esi
        jz      @f
        mov     edi, [esp]
        mov     ecx, esi
        sub     ecx, edx
        mov     esi, edx
        mov     edx, edi
        rep movsb
@@:
        mov     byte [edi], '/'
        inc     edi
        mov     esi, ebx
        mov     ecx, edx
        add     ecx, maxPathLength
        sub     ecx, edi
        jmp     .absolute

.set_ok:
        cmp     edx, edi    ; is destination equal to cur_dir?
        jz      @f
        mov     ecx, esi
        sub     ecx, edx
        mov     esi, edx
        rep movsb
@@:
        pop     eax
        sub     edi, eax
        pop     eax
        mov     eax, edi
        ret

.end:
        or      ecx, -1
        mov     edi, [esp]
        xor     eax, eax
        push    edi
        repnz scasb
        not     ecx
        pop     edi
.parse:
        mov     al, '/'
        repnz scasb
        jecxz   @b
        cmp     byte [edi], '.'
        jnz     .parse
        mov     esi, edi
@@:
        lodsw
        sub     ecx, 2
        cmp     ax, './'
        jz      @b
        cmp     ax, '..'
        jnz     @f
        cmp     byte [esi], '/'
        jnz     @f
        mov     edx, ecx
        mov     ecx, edi
        sub     ecx, [esp]
        sub     ecx, 2
        jc      .fail
        sub     edi, 2
        lodsb
        dec     edx
        std
        repnz scasb
        cld
        add     edi, 2
        mov     ecx, edx
        jmp     @b

@@:
        sub     esi, 2
        add     ecx, 2
        cmp     esi, edi
        jz      .parse
        push    edi ecx
        rep movsb
        pop     ecx edi
        jmp     .parse

include "parse_fn.inc"
include "fs_common.inc"
include "iso9660.inc"   ; read for CD filesystem
include "fat.inc"
include "exfat.inc"
include "ntfs.inc"
include "ext.inc"
include "xfs.asm"
