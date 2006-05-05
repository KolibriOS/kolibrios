; System function 70 - files with long names (LFN)
; diamond, 2006

iglobal
; in this table names must be in lowercase
rootdirs:
        db      2,'rd'
        dd      fs_OnRamdisk
        dd      fs_NextRamdisk
        db      7,'ramdisk'
        dd      fs_OnRamdisk
        dd      fs_NextRamdisk
        db      2,'fd'
        dd      fs_OnFloppy
        dd      fs_NextFloppy
        db      10,'floppydisk'
        dd      fs_OnFloppy
        dd      fs_NextFloppy
        db      3,'hd0'
        dd      fs_OnHd0
        dd      fs_NextHd0
        db      3,'hd1'
        dd      fs_OnHd1
        dd      fs_NextHd1
        db      3,'hd2'
        dd      fs_OnHd2
        dd      fs_NextHd2
        db      3,'hd3'
        dd      fs_OnHd3
        dd      fs_NextHd3
        db      0

virtual_root_query:
        dd      fs_HasRamdisk
        du      'rd',0
        dd      fs_HasFloppy
        du      'fd',0
        dd      fs_HasHd0
        du      'hd0',0
        dd      fs_HasHd1
        du      'hd1',0
        dd      fs_HasHd2
        du      'hd2',0
        dd      fs_HasHd3
        du      'hd3',0
        dd      0
endg

file_system_lfn:
; in: eax->fileinfo block
; operation codes:
; 0 : read file
; 1 : read folder
; 2 : rewrite file - not implemented yet
; 3 : write/append to file - not implemented yet
; 4 : start application - not implemented yet
; 5 : delete file - not implemented yet
; 6 : create directory - not implemented yet
; 7 : rename file/directory - not implemented yet
; 8 : get file attributes structure - not implemented yet

        add     eax, std_application_base_address
; parse file name
        xchg    ebx, eax
        lea     esi, [ebx+20]
        lodsb
        cmp     al, '/'
        jz      @f
.notfound:
        mov     dword [esp+36], 5       ; file not found
        ret
@@:
        cmp     byte [esi], 0
        jz      .rootdir
        mov     edi, rootdirs-8
        xor     ecx, ecx
        push    esi
.scan1:
        pop     esi
        add     edi, ecx
        scasd
        scasd
        mov     cl, byte [edi]
        jecxz   .notfound
        inc     edi
        push    esi
@@:
        lodsb
        or      al, 20h
        scasb
        loopz   @b
        jnz     .scan1
        lodsb
        cmp     al, '/'
        jz      .found1
        test    al, al
        jnz     .scan1
        pop     eax
; directory /xxx
.maindir:
        cmp     dword [ebx], 1
        jnz     .access_denied
        xor     eax, eax
        mov     ebp, [ebx+12]
        mov     edx, [ebx+16]
        add     edx, std_application_base_address
        mov     ebx, [ebx+4]
        mov     esi, [edi+4]
; ebx=first block, ebp=number of blocks, edx=return area, esi='Next' handler
        mov     edi, edx
        mov     ecx, 32/4
        rep     stosd
        mov     byte [edx], 1   ; version
.maindir_loop:
        call    esi
        jc      .maindir_done
        inc     dword [edx+8]
        dec     ebx
        jns     .maindir_loop
        dec     ebp
        js      .maindir_loop
        inc     dword [edx+4]
        mov     dword [edi], 0x10       ; attributes: folder
        mov     dword [edi+4], 1        ; name type: UNICODE
        push    eax
        xor     eax, eax
        add     edi, 8
        mov     ecx, 40/4-2
        rep     stosd
        pop     eax
        push    eax edx
; convert number in eax to decimal UNICODE string
        push    edi
        push    -'0'
        mov     cl, 10
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     al, '0'
        stosw
        jnz     @b
        mov     byte [edi-1], 0
        pop     edi
        add     edi, 520
        pop     edx eax
        jmp     .maindir_loop
.maindir_done:
        mov     ebx, [edx+8]
        xor     eax, eax
        dec     ebp
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        mov     [esp+36], eax
        mov     [esp+24], ebx
        ret
; directory /
.rootdir:
        cmp     dword [ebx], 1  ; read folder?
        jz      .readroot
.access_denied:
        mov     dword [esp+36], 10      ; access denied
        ret

.readroot:
; virtual root folder - special handler
        mov     esi, virtual_root_query
        mov     ebp, [ebx+12]
        mov     edx, [ebx+16]
        add     edx, std_application_base_address
        mov     ebx, [ebx+4]
        xor     eax, eax
; eax=0, ebx=first block, ebp=number of blocks, edx=return area
        mov     edi, edx
        mov     ecx, 32/4
        rep     stosd
        mov     byte [edx], 1   ; version
.readroot_loop:
        cmp     dword [esi], eax
        jz      .readroot_done
        call    dword [esi]
        add     esi, 4
        test    eax, eax
        jnz     @f
.readroot_next:
        or      ecx, -1
        xchg    esi, edi
        repnz   scasw
        xchg    esi, edi
        jmp     .readroot_loop
@@:
        xor     eax, eax
        inc     dword [edx+8]
        dec     ebx
        jns     .readroot_next
        dec     ebp
        js      .readroot_next
        inc     dword [edx+4]
        mov     dword [edi], 0x10       ; attributes: folder
        mov     dword [edi+4], 1        ; name type: UNICODE
        add     edi, 8
        mov     ecx, 40/4-2
        rep     stosd
        push    edi
@@:
        lodsw
        stosw
        test    eax, eax
        jnz     @b
        pop     edi
        add     edi, 520
        jmp     .readroot_loop
.readroot_done:
        mov     ebx, [edx+8]
        xor     eax, eax
        dec     ebp
        js      @f
        mov     al, ERROR_END_OF_FILE
@@:
        mov     [esp+36], eax
        mov     [esp+24], ebx
        ret

.found1:
        pop     eax
        cmp     byte [esi], 0
        jz      .maindir
; read partition number
        xor     ecx, ecx
        xor     eax, eax
@@:
        lodsb
        cmp     al, '/'
        jz      .done1
        test    al, al
        jz      .done1
        sub     al, '0'
        cmp     al, 9
        ja      .notfound
        imul    ecx, 10
        add     ecx, eax
        jmp     @b
.done1:
        test    ecx, ecx
        jz      .notfound
        test    al, al
        jnz     @f
        dec     esi
@@:
; now [edi] contains handler address, ecx - partition number,
; esi points to ASCIIZ string - rest of name
        jmp     dword [edi]

; handlers for devices
; in: ecx = 0 => query virtual directory /xxx
; in: ecx = partition number
;     esi -> relative (for device) name
;     ebx -> fileinfo
; out: [esp+36]=image of eax, [esp+24]=image of ebx

fs_OnRamdisk:
        cmp     ecx, 1
        jnz     file_system_lfn.notfound
        mov     eax, [ebx]
        cmp     eax, 1
        ja      .not_impl
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        add     edx, std_application_base_address
        add     ebx, 4
        call    dword [fs_RamdiskServices + eax*4]
        mov     [esp+36], eax
        mov     [esp+24], ebx
        ret
.not_impl:
        mov     dword [esp+36], 2       ; not implemented
        ret

fs_RamdiskServices:
        dd      fs_RamdiskRead
        dd      fs_RamdiskReadFolder

fs_OnFloppy:
        cmp     ecx, 2
        ja      file_system_lfn.notfound
        mov     eax, [ebx]
        cmp     eax, 1
        ja      fs_OnRamdisk.not_impl
        call    reserve_flp
        mov     [flp_number], cl
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        add     edx, std_application_base_address
        add     ebx, 4
        call    dword [fs_FloppyServices + eax*4]
        and     [flp_status], 0
        mov     [esp+36], eax
        mov     [esp+24], ebx
        ret

fs_FloppyServices:
        dd      fs_FloppyRead
        dd      fs_FloppyReadFolder

fs_OnHd0:
        call    reserve_hd1
        mov     [hdbase], 0x1F0
        mov     [hdid], 0
        push    1
        jmp     fs_OnHd
fs_OnHd1:
        call    reserve_hd1
        mov     [hdbase], 0x1F0
        mov     [hdid], 0x10
        push    2
        jmp     fs_OnHd
fs_OnHd2:
        call    reserve_hd1
        mov     [hdbase], 0x170
        mov     [hdid], 0
        push    3
        jmp     fs_OnHd
fs_OnHd3:
        call    reserve_hd1
        mov     [hdbase], 0x170
        mov     [hdid], 0x10
        push    4
fs_OnHd:
        pop     eax
        mov     [hdpos], eax
        cmp     ecx, 0x100
        jae     .nf
        cmp     cl, [0x40001+eax]
        jbe     @f
.nf:
        and     [hd1_status], 0
        mov     dword [esp+36], 5       ; not found
        ret
@@:
        mov     [fat32part], ecx
        push    ebx esi
        call    choice_necessity_partition_1
        pop     esi ebx
        mov     ecx, [ebx+12]
        mov     edx, [ebx+16]
        add     edx, std_application_base_address
        mov     eax, [ebx]
        cmp     eax, 1
        ja      .not_impl
        add     ebx, 4
        call    dword [fs_HdServices + eax*4]
        and     [hd1_status], 0
        mov     [esp+36], eax
        mov     [esp+24], ebx
        ret
.not_impl:
        and     [hd1_status], 0
        mov     dword [esp+36], 2       ; not implemented
        ret

fs_HdServices:
        dd      fs_HdRead
        dd      fs_HdReadFolder

fs_HasRamdisk:
        mov     al, 1   ; we always have ramdisk
        ret

fs_HasFloppy:
        cmp     byte [0x40000], 0
        setnz   al
        ret

fs_HasHd0:
        mov     al, [0x40001]
        and     al, 11000000b
        cmp     al, 01000000b
        setz    al
        ret
fs_HasHd1:
        mov     al, [0x40001]
        and     al, 00110000b
        cmp     al, 00010000b
        setz    al
        ret
fs_HasHd2:
        mov     al, [0x40001]
        and     al, 00001100b
        cmp     al, 00000100b
        setz    al
        ret
fs_HasHd3:
        mov     al, [0x40001]
        and     al, 00000011b
        cmp     al, 00000001b
        setz    al
        ret

; fs_NextXXX functions:
; in: eax = partition number, from which start to scan
; out: CF=1 => no more partitions
;      CF=0 => eax=next partition number

fs_NextRamdisk:
; we always have /rd/1
        test    eax, eax
        stc
        jnz     @f
        mov     al, 1
        clc
@@:
        ret

fs_NextFloppy:
; we have /fd/1 iff (([0x40000] and 0xF0) != 0) and /fd/2 iff (([0x40000] and 0x0F) != 0)
        test    byte [0x40000], 0xF0
        jz      .no1
        test    eax, eax
        jnz     .no1
        inc     eax
        ret     ; CF cleared
.no1:
        test    byte [0x40000], 0x0F
        jz      .no2
        cmp     al, 2
        jae     .no2
        mov     al, 2
        clc
        ret
.no2:
        stc
        ret

; on hdx, we have partitions from 1 to [0x40002+x]
fs_NextHd0:
        push    0
        jmp     fs_NextHd
fs_NextHd1:
        push    1
        jmp     fs_NextHd
fs_NextHd2:
        push    2
        jmp     fs_NextHd
fs_NextHd3:
        push    3
fs_NextHd:
        pop     ecx
        movzx   ecx, byte [0x40002+ecx]
        cmp     eax, ecx
        jae     fs_NextFloppy.no2
        inc     eax
        clc
        ret
