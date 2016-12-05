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
ERROR_FS_FAIL        = 9
ERROR_ACCESS_DENIED  = 10
ERROR_DEVICE         = 11
ERROR_OUT_OF_MEMORY  = 12

maxPathLength = 1000h

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
        lea     ebp, [ebx+20]
        cmp     byte [ebp], 0
        jnz     @f
        mov     ebp, [ebx+21]
@@:
        cmp     word [ebp], '/'
        jz      .rootdir
        cmp     byte [ebp], 4
        jnc     @f
        cmp     byte [ebp], 0
        jz      @f
        cmp     word [ebp+1], '/'
        jnz     @f
        cmp     byte [ebp], 2
        jnz     .rootdir
        cmp     word [ebp+3], 0
        jz      .rootdir
@@:
        cmp     dword[ebx], 7   ; start application
        jnz     @f
        mov     edx, [ebx+4]
        mov     ebx, [ebx+8]
        call    fs_execute      ; ebp, ebx, edx
        mov     [image_of_eax], eax
        ret

@@:
        stdcall kernel_alloc, maxPathLength
        push    ebx
        mov     ebx, ebp
        mov     ebp, eax
        stdcall get_full_file_name, eax, maxPathLength
        pop     ebx
        test    eax, eax
        jz      .notfound
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
        stc
        jnz     @f      ; no more partitions
        mov     al, 1   ; /cdX/1
        clc
@@:
        ret

.maindir:   ; list partitions
        mov     esi, .nextCD
.maindir_noesi:     ; backjump from dyndisk_handler
        push    ebp
        mov     ebp, ecx
        call    kernel_free
        cmp     dword[ebx], 1
        jnz     .access_denied  ; read folder?
        push    ebp
        pushd   [ebx+4]         ; first block
        mov     ebp, [ebx+12]   ; the number of blocks to read
        mov     edx, [ebx+16]   ; buffer
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
        push    edi
        lea     esi, [esp+8]
        cmp     ebx, 1
        jz      .uni2
@@:
        lodsb
        stosb
        test    eax, eax
        jnz     @b
        pop     edi eax
        add     edi, 264
        jmp     .rootdir_loop

.uni2:
        lodsb
        stosw
        test    eax, eax
        jnz     .uni2
        pop     edi eax
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
        dec     ebx
        jz      .set
        dec     ebx
        jz      .get
        dec     ebx
        jz      .mount_additional_directory
        dec     ebx
        jz      .get16
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
        mov     ecx, 63
        rep movsb   ; copying fake directory name
        inc     esi
        xor     eax, eax
        stosb       ; terminator of name, in case if we get the inlet trash
        mov     cl, 63
        cmp     word [esi], 2
        jz      .utf16
        call    cp866toUTF8_string
@@:
        mov     byte [edi], 0
        mov     [full_file_name_table.size], 2
        ret

.utf16:
        add     esi, 2
        call    UTF16to8_string
        jmp     @b

.get:       ; in: ecx -> buffer, edx = length
        mov     esi, edi
        inc     esi
        mov     edi, ecx
        cmp     edx, maxPathLength
        jc      @f
        mov     edx, maxPathLength
@@:
        mov     ecx, edx
@@:
        dec     ecx
        js      @f
        call    utf8to16
        call    uni2ansi_char
        stosb
        test    al, al
        jnz     @b
        sub     edx, ecx
        mov     ecx, edx
@@:
        mov     [esp+32], ecx
        ret

.get16:
        mov     esi, edi
        inc     esi
        mov     edi, ecx
        cmp     edx, maxPathLength
        jc      @f
        mov     edx, maxPathLength
@@:
        shr     edx, 1
        mov     ecx, edx
@@:
        dec     ecx
        js      @f
        call    utf8to16
        stosw
        test    ax, ax
        jnz     @b
        sub     edx, ecx
        mov     ecx, edx
@@:
        mov     [esp+32], ecx
        ret

.set:
        pop     eax
        push    maxPathLength
        push    edi
        push    eax
        mov     ebx, ecx
get_full_file_name:
; in: ebx -> file name, [esp+4] -> destination, [esp+8] = max length
; out: UTF-8 string, eax=0 -> out of length
        push    ebp ebx
        cmp     byte [ebx], 0
        jz      .set_relative
        mov     esi, ebx
        cmp     byte [ebx], 4
        jnc     @f
        inc     esi
@@:
        cmp     byte [esi], '/'
        jnz     .set_relative
        inc     esi
        cmp     byte [esi], 4
        jnc     @f
        mov     ebx, esi
        inc     esi
        cmp     byte [esi], '/'
        jnz     .start
        inc     esi
@@:
        cmp     byte [ebx], 2
        jnz     .start
        inc     esi
.start:
        call    process_replace_file_name
        mov     edi, [esp+12]
        mov     ecx, [esp+16]
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
        cmp     byte [ebx], 2
        jz      .utf16
        cmp     byte [ebx], 3
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
        xor     eax, eax
        pop     ebx ebp
        ret     8

.set_relative:
        mov     edi, [current_slot]
        mov     edi, [edi+APPDATA.cur_dir]
        mov     edx, edi
        mov     ecx, [esp+16]
        xor     eax, eax
        repnz scasb
        mov     esi, edi
        dec     esi
        mov     edi, [esp+12]
        jecxz   .fail
        cmp     byte [ebx], 0
        jz      .set_ok
        cmp     byte [ebx], 4
        jnc     @f
        inc     ebx
@@:
        cmp     edx, edi    ; is destination equal to cur_dir?
        mov     edi, esi
        jz      @f
        mov     edi, [esp+12]
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
        add     ecx, [esp+16]
        sub     ecx, edi
        mov     ebx, [esp]
        jmp     .absolute

.set_ok:
        cmp     edx, edi    ; is destination equal to cur_dir?
        jz      @f
        mov     ecx, esi
        sub     ecx, edx
        mov     esi, edx
        rep movsb
        mov     byte [edi], 0
@@:
        mov     al, 1
        pop     ebx ebp
        ret     8

.end:
        or      ecx, -1
        mov     edi, [esp+12]
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
        sub     ecx, [esp+12]
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
include "ntfs.inc"
include "ext.inc"
; include "xfs.asm"

xfs_create_partition:
        xor     eax, eax
        ret
