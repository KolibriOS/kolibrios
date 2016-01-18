;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 4420 $

; Disk access through BIOS
iglobal
align 4
bd_callbacks:
        dd      bd_callbacks.end - bd_callbacks     ; strucsize
        dd      0   ; no close function
        dd      0   ; no closemedia function
        dd      bd_querymedia
        dd      bd_read_interface
        dd      bd_write_interface
        dd      0   ; no flush function
        dd      0   ; use default cache size
.end:
endg

uglobal
bios_hdpos          dd 0
bios_cur_sector     dd ?
bios_read_len       dd ?
cache_chain_ptr     dd ?
int13_regs_in       rb sizeof.v86_regs
int13_regs_out      rb sizeof.v86_regs
cache_chain_size    db ?
endg
;-----------------------------------------------------------------
proc bd_read_interface stdcall uses edi, \
        userdata, buffer, startsector:qword, numsectors
        ; userdata = old [hdpos] = 80h + index in NumBiosDisks
        ; buffer = pointer to buffer for data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really read
locals
sectors_todo    dd      ?
endl
; 1. Initialize number of sectors: get number of requested sectors
; and say that no sectors were read yet.
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; 2. Acquire the global lock.
        mov     ecx, ide_mutex
        call    mutex_lock
; 3. Convert parameters to the form suitable for worker procedures.
; Underlying procedures do not know about 64-bit sectors.
; Worker procedures use global variables and edi for [buffer].
        cmp     dword [startsector+4], 0
        jnz     .fail
        and     [hd_error], 0
        mov     eax, [userdata]
        mov     [hdpos], eax
        mov     eax, dword [startsector]
        mov     edi, [buffer]
; 4. Worker procedures take one sectors per time, so loop over all sectors to read.
.sectors_loop:
        call    bd_read
        cmp     [hd_error], 0
        jnz     .fail
        mov     ecx, [numsectors]
        inc     dword [ecx]     ; one more sector is read
        dec     [sectors_todo]
        jz      .done
        inc     eax
        jnz     .sectors_loop
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, ide_mutex
        call    mutex_unlock
        or      eax, -1
        ret
.done:
        mov     ecx, ide_mutex
        call    mutex_unlock
        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------
proc bd_write_interface stdcall uses esi edi, \
        userdata, buffer, startsector:qword, numsectors
        ; userdata = old [hdpos] = 80h + index in NumBiosDisks
        ; buffer = pointer to buffer with data
        ; startsector = 64-bit start sector
        ; numsectors = pointer to number of sectors on input,
        ;  must be filled with number of sectors really written
locals
sectors_todo    dd      ?
endl
; 1. Initialize number of sectors: get number of requested sectors
; and say that no sectors were read yet.      
        mov     ecx, [numsectors]
        mov     eax, [ecx]
        mov     dword [ecx], 0
        mov     [sectors_todo], eax
; 2. Acquire the global lock.
        mov     ecx, ide_mutex
        call    mutex_lock
; 3. Convert parameters to the form suitable for worker procedures.
; Underlying procedures do not know about 64-bit sectors.
; Worker procedures use global variables and esi for [buffer].
        cmp     dword [startsector+4], 0
        jnz     .fail
        and     [hd_error], 0
        mov     eax, [userdata]
        mov     [hdpos], eax
        mov     esi, [buffer]
        lea     edi, [startsector]
        mov     [cache_chain_ptr], edi
; 4. Worker procedures take max 16 sectors per time,
; loop until all sectors will be processed.
.sectors_loop:
        mov     ecx, 16
        cmp     ecx, [sectors_todo]
        jbe     @f
        mov     ecx, [sectors_todo]
@@:
        mov     [cache_chain_size], cl
        call    bd_write_cache_chain
        cmp     [hd_error], 0
        jnz     .fail
        movzx   ecx, [cache_chain_size]
        mov     eax, [numsectors]
        add     [eax], ecx
        sub     [sectors_todo], ecx
        jz      .done
        add     [edi], ecx
        jc      .fail
        shl     ecx, 9
        add     esi, ecx
        jmp     .sectors_loop
; 5. Loop is done, either due to error or because everything is done.
; Release the global lock and return the corresponding status.
.fail:
        mov     ecx, ide_mutex
        call    mutex_unlock
        or      eax, -1
        ret
.done:
        mov     ecx, ide_mutex
        call    mutex_unlock
        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------
; This is a stub.
proc bd_querymedia stdcall, hd_data, mediainfo
        mov     eax, [mediainfo]
        mov     [eax+DISKMEDIAINFO.Flags], 0
        mov     [eax+DISKMEDIAINFO.SectorSize], 512
        or      dword [eax+DISKMEDIAINFO.Capacity], 0xFFFFFFFF
        or      dword [eax+DISKMEDIAINFO.Capacity+4], 0xFFFFFFFF
        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------
bd_read:
        push    eax
        push    edx
        mov     edx, [bios_hdpos]
        cmp     edx, [hdpos]
        jne     .notread
        mov     edx, [bios_cur_sector]
        cmp     eax, edx
        jb      .notread
        add     edx, [bios_read_len]
        dec     edx
        cmp     eax, edx
        ja      .notread
        sub     eax, [bios_cur_sector]
        shl     eax, 9
        add     eax, (OS_BASE+0x99000)
        push    ecx esi
        mov     esi, eax
        mov     ecx, 512/4
        cld
        rep movsd
        pop     esi ecx
        pop     edx
        pop     eax
        ret
.notread:
        push    ecx
        mov     dl, 42h
        mov     ecx, 16
        call    int13_call
        pop     ecx
        test    eax, eax
        jnz     .v86err
        test    edx, edx
        jz      .readerr
        mov     [bios_read_len], edx
        mov     edx, [hdpos]
        mov     [bios_hdpos], edx
        pop     edx
        pop     eax
        mov     [bios_cur_sector], eax
        jmp     bd_read
.readerr:
.v86err:
        pop     edx
        pop     eax
        mov     [hd_error], 1
        jmp     hd_read_error
;-----------------------------------------------------------------
bd_write_cache_chain:
        pusha
        mov     edi, OS_BASE + 0x99000
        movzx   ecx, [cache_chain_size]
        push    ecx
        shl     ecx, 9-2
        rep movsd
        pop     ecx
        mov     dl, 43h
        mov     eax, [cache_chain_ptr]
        mov     eax, [eax]
        call    int13_call
        test    eax, eax
        jnz     .v86err
        cmp     edx, ecx
        jnz     .writeerr
        popa
        ret
.v86err:
.writeerr:
        popa
        mov     [hd_error], 1
        jmp     hd_write_error
;-----------------------------------------------------------------
int13_call:
; Because this code uses fixed addresses,
; it can not be run simultaniously by many threads.
; In current implementation it is protected by common mutex 'ide_status'
        mov     word [OS_BASE + 510h], 10h          ; packet length
        mov     word [OS_BASE + 512h], cx           ; number of sectors
        mov     dword [OS_BASE + 514h], 99000000h   ; buffer 9900:0000
        mov     dword [OS_BASE + 518h], eax
        and     dword [OS_BASE + 51Ch], 0
        push    ebx ecx esi edi
        mov     ebx, int13_regs_in
        mov     edi, ebx
        mov     ecx, sizeof.v86_regs/4
        xor     eax, eax
        rep stosd
        mov     byte [ebx+v86_regs.eax+1], dl
        mov     eax, [hdpos]
        lea     eax, [BiosDisksData+(eax-80h)*4]
        mov     dl, [eax]
        mov     byte [ebx+v86_regs.edx], dl
        movzx   edx, byte [eax+1]
;        mov     dl, 5
        test    edx, edx
        jnz     .hasirq
        dec     edx
        jmp     @f
.hasirq:
        pushad
        stdcall enable_irq, edx
        popad
@@:
        mov     word [ebx+v86_regs.esi], 510h
        mov     word [ebx+v86_regs.ss], 9000h
        mov     word [ebx+v86_regs.esp], 09000h
        mov     word [ebx+v86_regs.eip], 500h
        mov     [ebx+v86_regs.eflags], 20200h
        mov     esi, [sys_v86_machine]
        mov     ecx, 0x502
        push    fs
        call    v86_start
        pop     fs
        and     [bios_hdpos], 0
        pop     edi esi ecx ebx
        movzx   edx, byte [OS_BASE + 512h]
        test    byte [int13_regs_out+v86_regs.eflags], 1
        jnz     @f
        mov     edx, ecx
@@:
        ret
