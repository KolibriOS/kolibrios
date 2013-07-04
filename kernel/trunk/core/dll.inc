;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


DRV_COMPAT   equ  5  ;minimal required drivers version
DRV_CURRENT  equ  6  ;current drivers model version

DRV_VERSION equ (DRV_COMPAT shl 16) or DRV_CURRENT
PID_KERNEL  equ 1    ;os_idle thread



align 4
proc get_notify stdcall, p_ev:dword

.wait:
        mov     ebx, [current_slot]
        test    dword [ebx+APPDATA.event_mask], EVENT_NOTIFY
        jz      @f
        and     dword [ebx+APPDATA.event_mask], not EVENT_NOTIFY
        mov     edi, [p_ev]
        mov     dword [edi], EV_INTR
        mov     eax, [ebx+APPDATA.event]
        mov     dword [edi+4], eax
        ret
@@:
        call    change_task
        jmp     .wait
endp

align 4
proc pci_read32 stdcall, bus:dword, devfn:dword, reg:dword
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
        mov     ah, byte [bus]
        mov     al, 6
        mov     bh, byte [devfn]
        mov     bl, byte [reg]
        call    pci_read_reg
        pop     ebx
        ret
endp

align 4
proc pci_read16 stdcall, bus:dword, devfn:dword, reg:dword
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
        mov     ah, byte [bus]
        mov     al, 5
        mov     bh, byte [devfn]
        mov     bl, byte [reg]
        call    pci_read_reg
        pop     ebx
        ret
endp

align 4
proc pci_read8 stdcall, bus:dword, devfn:dword, reg:dword
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
        mov     ah, byte [bus]
        mov     al, 4
        mov     bh, byte [devfn]
        mov     bl, byte [reg]
        call    pci_read_reg
        pop     ebx
        ret
endp

align 4
proc pci_write8 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
        mov     ah, byte [bus]
        mov     al, 8
        mov     bh, byte [devfn]
        mov     bl, byte [reg]
        mov     ecx, [val]
        call    pci_write_reg
        pop     ebx
        ret
endp

align 4
proc pci_write16 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
        mov     ah, byte [bus]
        mov     al, 9
        mov     bh, byte [devfn]
        mov     bl, byte [reg]
        mov     ecx, [val]
        call    pci_write_reg
        pop     ebx
        ret
endp

align 4
proc pci_write32 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
        mov     ah, byte [bus]
        mov     al, 10
        mov     bh, byte [devfn]
        mov     bl, byte [reg]
        mov     ecx, [val]
        call    pci_write_reg
        pop     ebx
        ret
endp

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size


align 4
proc srv_handler stdcall, ioctl:dword
        mov     esi, [ioctl]
        test    esi, esi
        jz      .err

        mov     edi, [esi+handle]
        cmp     [edi+SRV.magic], ' SRV'
        jne     .fail

        cmp     [edi+SRV.size], sizeof.SRV
        jne     .fail

;        stdcall [edi+SRV.srv_proc], esi
        mov     eax, [edi+SRV.srv_proc]
        test    eax, eax
        jz      .fail
        stdcall eax, esi
        ret
.fail:
        xor     eax, eax
        not     eax
        mov     [esi+output], eax
        mov     [esi+out_size], 4
        ret
.err:
        xor     eax, eax
        not     eax
        ret
endp

; param
;  ecx= io_control
;
; retval
;  eax= error code

align 4
srv_handlerEx:
        cmp     ecx, OS_BASE
        jae     .fail

        mov     eax, [ecx+handle]
        cmp     [eax+SRV.magic], ' SRV'
        jne     .fail

        cmp     [eax+SRV.size], sizeof.SRV
        jne     .fail

;        stdcall [eax+SRV.srv_proc], ecx
        mov     eax, [eax+SRV.srv_proc]
        test    eax, eax
        jz      .fail
        stdcall eax, ecx
        ret
.fail:
        or      eax, -1
        ret

restore  handle
restore  io_code
restore  input
restore  inp_size
restore  output
restore  out_size

align 4
proc get_service stdcall, sz_name:dword
        mov     eax, [sz_name]
        test    eax, eax
        jnz     @F
        ret
@@:
        mov     edx, [srv.fd]
@@:
        cmp     edx, srv.fd-SRV.fd
        je      .not_load

        stdcall strncmp, edx, [sz_name], 16
        test    eax, eax
        je      .ok

        mov     edx, [edx+SRV.fd]
        jmp     @B
.not_load:
        pop     ebp
        jmp     load_driver
.ok:
        mov     eax, edx
        ret
endp

reg_service:
        xor     eax, eax
        mov     ecx, [esp+8]
        jecxz   .nothing
        push    sizeof.SRV
        push    ecx
        pushd   [esp+12]
        call    reg_service_ex
.nothing:
        ret     8

reg_usb_driver:
        push    sizeof.USBSRV
        pushd   [esp+12]
        pushd   [esp+12]
        call    reg_service_ex
        test    eax, eax
        jz      .nothing
        mov     ecx, [esp+12]
        mov     [eax+USBSRV.usb_func], ecx
.nothing:
        ret     12

proc reg_service_ex stdcall, name:dword, handler:dword, srvsize:dword

        push    ebx

        xor     eax, eax

        cmp     [name], eax
        je      .fail

;        cmp     [handler], eax
;        je      .fail

        mov     eax, [srvsize]
        call    malloc
        test    eax, eax
        jz      .fail

        push    esi
        push    edi
        mov     edi, eax
        mov     esi, [name]
        movsd
        movsd
        movsd
        movsd
        pop     edi
        pop     esi

        mov     [eax+SRV.magic], ' SRV'
        mov     [eax+SRV.size], sizeof.SRV

        mov     ebx, srv.fd-SRV.fd
        mov     edx, [ebx+SRV.fd]
        mov     [eax+SRV.fd], edx
        mov     [eax+SRV.bk], ebx
        mov     [ebx+SRV.fd], eax
        mov     [edx+SRV.bk], eax

        mov     ecx, [handler]
        mov     [eax+SRV.srv_proc], ecx
        pop     ebx
        ret
.fail:
        xor     eax, eax
        pop     ebx
        ret
endp

align 4
proc get_proc stdcall, exp:dword, sz_name:dword

        mov     edx, [exp]
.next:
        mov     eax, [edx]
        test    eax, eax
        jz      .end

        push    edx
        stdcall strncmp, eax, [sz_name], 16
        pop     edx
        test    eax, eax
        jz      .ok

        add     edx, 8
        jmp     .next
.ok:
        mov     eax, [edx+4]
.end:
        ret
endp

align 4
proc get_coff_sym stdcall, pSym:dword,count:dword, sz_sym:dword

@@:
        stdcall strncmp, [pSym], [sz_sym], 8
        test    eax, eax
        jz      .ok
        add     [pSym], 18
        dec     [count]
        jnz     @b
        xor     eax, eax
        ret
.ok:
        mov     eax, [pSym]
        mov     eax, [eax+8]
        ret
endp

align 4
proc get_curr_task
        mov     eax, [CURRENT_TASK]
        shl     eax, 8
        ret
endp

align 4
proc get_fileinfo stdcall, file_name:dword, info:dword
           locals
             cmd     dd ?
             offset  dd ?
                     dd ?
             count   dd ?
             buff    dd ?
                     db ?
             name    dd ?
           endl

        xor     eax, eax
        mov     ebx, [file_name]
        mov     ecx, [info]

        mov     [cmd], 5
        mov     [offset], eax
        mov     [offset+4], eax
        mov     [count], eax
        mov     [buff], ecx
        mov     byte [buff+4], al
        mov     [name], ebx

        mov     eax, 70
        lea     ebx, [cmd]
        int     0x40
        ret
endp

align 4
proc read_file stdcall,file_name:dword, buffer:dword, off:dword,\
                                     bytes:dword
           locals
             cmd     dd ?
             offset  dd ?
                     dd ?
             count   dd ?
             buff    dd ?
                     db ?
             name    dd ?
           endl

        xor     eax, eax
        mov     ebx, [file_name]
        mov     ecx, [off]
        mov     edx, [bytes]
        mov     esi, [buffer]

        mov     [cmd], eax
        mov     [offset], ecx
        mov     [offset+4], eax
        mov     [count], edx
        mov     [buff], esi
        mov     byte [buff+4], al
        mov     [name], ebx

        pushad
        lea     ebx, [cmd]
        call    file_system_lfn_protected
        popad
        ret
endp

; description
;  allocate kernel memory and loads the specified file
;
; param
;  file_name= full path to file
;
; retval
;  eax= file image in kernel memory
;  ebx= size of file
;
; warging
;  You mast call kernel_free() to delete each file
;  loaded by the load_file() function

align 4
proc load_file stdcall, file_name:dword
           locals
             attr       dd ?
             flags      dd ?
             cr_time    dd ?
             cr_date    dd ?
             acc_time   dd ?
             acc_date   dd ?
             mod_time   dd ?
             mod_date   dd ?
             file_size  dd ?

             file       dd ?
             file2      dd ?
           endl

        push    esi
        push    edi

        lea     eax, [attr]
        stdcall get_fileinfo, [file_name], eax
        test    eax, eax
        jnz     .fail

        mov     eax, [file_size]
        cmp     eax, 1024*1024*16
        ja      .fail

        stdcall kernel_alloc, [file_size]
        mov     [file], eax
        test    eax, eax
        jz      .fail

        stdcall read_file, [file_name], eax, dword 0, [file_size]
        cmp     ebx, [file_size]
        jne     .cleanup

        mov     eax, [file]
        cmp     dword [eax], 0x4B43504B
        jne     .exit
        mov     ebx, [eax+4]
        mov     [file_size], ebx
        stdcall kernel_alloc, ebx

        test    eax, eax
        jz      .cleanup

        mov     [file2], eax

        pushad
        mov     ecx, unpack_mutex
        call    mutex_lock
        popad

        stdcall unpack, [file], eax

        pushad
        mov     ecx, unpack_mutex
        call    mutex_unlock
        popad

        stdcall kernel_free, [file]
        mov     eax, [file2]
        mov     ebx, [file_size]
.exit:
        push    eax
        lea     edi, [eax+ebx]    ;cleanup remain space
        mov     ecx, 4096         ;from file end
        and     ebx, 4095
        jz      @f
        sub     ecx, ebx
        xor     eax, eax
        cld
        rep stosb
@@:
        mov     ebx, [file_size]
        pop     eax
        pop     edi
        pop     esi
        ret
.cleanup:
        stdcall kernel_free, [file]
.fail:
        xor     eax, eax
        xor     ebx, ebx
        pop     edi
        pop     esi
        ret
endp

uglobal
align 4
unpack_mutex MUTEX
endg

align 4
proc get_proc_ex stdcall uses ebx esi, proc_name:dword, imports:dword
        mov     ebx, [imports]
        test    ebx, ebx
        jz      .end
        xor     esi, esi
.look_up:

        mov     eax, [ebx+32]
        mov     eax, [OS_BASE+eax+esi*4]
        add     eax, OS_BASE
        stdcall strncmp, eax, [proc_name], 256
        test    eax, eax
        jz      .ok

        inc     esi
        cmp     esi, [ebx+24]
        jb      .look_up
.end:
        xor     eax, eax
        ret
.ok:
        mov     eax, [ebx+28]
        mov     eax, [OS_BASE+eax+esi*4]
        add     eax, OS_BASE
        ret
endp

align 4
proc fix_coff_symbols stdcall uses ebx esi, sec:dword, symbols:dword,\
                      sym_count:dword, strings:dword, imports:dword
           locals
             retval dd ?
           endl

        mov     edi, [symbols]
        mov     [retval], 1
.fix:
        movzx   ebx, [edi+COFF_SYM.SectionNumber]
        test    ebx, ebx
        jnz     .internal
        mov     eax, dword [edi+COFF_SYM.Name]
        test    eax, eax
        jnz     @F

        mov     edi, [edi+4]
        add     edi, [strings]
@@:
        push    edi
        stdcall get_proc_ex, edi, [imports]
        pop     edi

        xor     ebx, ebx
        test    eax, eax
        jnz     @F

        mov     esi, msg_unresolved
        call    sys_msg_board_str
        mov     esi, edi
        call    sys_msg_board_str
        mov     esi, msg_CR
        call    sys_msg_board_str

        mov     [retval], 0
@@:
        mov     edi, [symbols]
        mov     [edi+COFF_SYM.Value], eax
        jmp     .next
.internal:
        cmp     bx, -1
        je      .next
        cmp     bx, -2
        je      .next

        dec     ebx
        shl     ebx, 3
        lea     ebx, [ebx+ebx*4]
        add     ebx, [sec]

        mov     eax, [ebx+COFF_SECTION.VirtualAddress]
        add     [edi+COFF_SYM.Value], eax
.next:
        add     edi, sizeof.COFF_SYM
        mov     [symbols], edi
        dec     [sym_count]
        jnz     .fix
        mov     eax, [retval]
        ret
endp

align 4
proc fix_coff_relocs stdcall uses ebx esi, coff:dword, sym:dword, \
        delta:dword
           locals
             n_sec     dd ?
           endl

        mov     eax, [coff]
        movzx   ebx, [eax+COFF_HEADER.nSections]
        mov     [n_sec], ebx
        lea     esi, [eax+20]
.fix_sec:
        mov     edi, [esi+COFF_SECTION.PtrReloc]
        add     edi, [coff]

        movzx   ecx, [esi+COFF_SECTION.NumReloc]
        test    ecx, ecx
        jz      .next
.reloc_loop:
        mov     ebx, [edi+COFF_RELOC.SymIndex]
        add     ebx, ebx
        lea     ebx, [ebx+ebx*8]
        add     ebx, [sym]

        mov     edx, [ebx+COFF_SYM.Value]

        cmp     [edi+COFF_RELOC.Type], 6
        je      .dir_32

        cmp     [edi+COFF_RELOC.Type], 20
        jne     .next_reloc
.rel_32:
        mov     eax, [edi+COFF_RELOC.VirtualAddress]
        add     eax, [esi+COFF_SECTION.VirtualAddress]
        sub     edx, eax
        sub     edx, 4
        jmp     .fix
.dir_32:
        mov     eax, [edi+COFF_RELOC.VirtualAddress]
        add     eax, [esi+COFF_SECTION.VirtualAddress]
.fix:
        add     eax, [delta]
        add     [eax], edx
.next_reloc:
        add     edi, 10
        dec     ecx
        jnz     .reloc_loop
.next:
        add     esi, sizeof.COFF_SECTION
        dec     [n_sec]
        jnz     .fix_sec
.exit:
        ret
endp

align 4
proc rebase_coff stdcall uses ebx esi, coff:dword, sym:dword, \
        delta:dword
           locals
             n_sec     dd ?
           endl

        mov     eax, [coff]
        movzx   ebx, [eax+COFF_HEADER.nSections]
        mov     [n_sec], ebx
        lea     esi, [eax+20]
        mov     edx, [delta]
.fix_sec:
        mov     edi, [esi+COFF_SECTION.PtrReloc]
        add     edi, [coff]

        movzx   ecx, [esi+COFF_SECTION.NumReloc]
        test    ecx, ecx
        jz      .next
.reloc_loop:
        cmp     [edi+COFF_RELOC.Type], 6
        jne     .next_reloc
.dir_32:
        mov     eax, [edi+COFF_RELOC.VirtualAddress]
        add     eax, [esi+COFF_SECTION.VirtualAddress]
        add     [eax+edx], edx
.next_reloc:
        add     edi, 10
        dec     ecx
        jnz     .reloc_loop
.next:
        add     esi, sizeof.COFF_SECTION
        dec     [n_sec]
        jnz     .fix_sec
.exit:
        ret
endp

align 4
proc load_driver stdcall, driver_name:dword
           locals
             coff      dd ?
             sym       dd ?
             strings   dd ?
             img_size  dd ?
             img_base  dd ?
             start     dd ?

             file_name rb 13+16+4+1      ; '/sys/drivers/<up-to-16-chars>.obj'
           endl

        lea     edx, [file_name]
        mov     dword [edx], '/sys'
        mov     dword [edx+4], '/dri'
        mov     dword [edx+8], 'vers'
        mov     byte [edx+12], '/'
        mov     esi, [driver_name]
.redo:
        lea     edx, [file_name]
        lea     edi, [edx+13]
        mov     ecx, 16
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        loop    @b
@@:
        mov     dword [edi], '.obj'
        mov     byte [edi+4], 0
        stdcall load_file, edx

        test    eax, eax
        jz      .exit

        mov     [coff], eax

        movzx   ecx, [eax+COFF_HEADER.nSections]
        xor     ebx, ebx

        lea     edx, [eax+20]
@@:
        add     ebx, [edx+COFF_SECTION.SizeOfRawData]
        add     ebx, 15
        and     ebx, not 15
        add     edx, sizeof.COFF_SECTION
        dec     ecx
        jnz     @B
        mov     [img_size], ebx

        stdcall kernel_alloc, ebx
        test    eax, eax
        jz      .fail
        mov     [img_base], eax

        mov     edi, eax
        xor     eax, eax
        mov     ecx, [img_size]
        add     ecx, 4095
        and     ecx, not 4095
        shr     ecx, 2
        cld
        rep stosd

        mov     edx, [coff]
        movzx   ebx, [edx+COFF_HEADER.nSections]
        mov     edi, [img_base]
        lea     eax, [edx+20]
@@:
        mov     [eax+COFF_SECTION.VirtualAddress], edi
        mov     esi, [eax+COFF_SECTION.PtrRawData]
        test    esi, esi
        jnz     .copy
        add     edi, [eax+COFF_SECTION.SizeOfRawData]
        jmp     .next
.copy:
        add     esi, edx
        mov     ecx, [eax+COFF_SECTION.SizeOfRawData]
        cld
        rep movsb
.next:
        add     edi, 15
        and     edi, not 15
        add     eax, sizeof.COFF_SECTION
        dec     ebx
        jnz     @B

        mov     ebx, [edx+COFF_HEADER.pSymTable]
        add     ebx, edx
        mov     [sym], ebx
        mov     ecx, [edx+COFF_HEADER.nSymbols]
        add     ecx, ecx
        lea     ecx, [ecx+ecx*8];ecx*=18 = nSymbols*CSYM_SIZE
        add     ecx, [sym]
        mov     [strings], ecx

        lea     eax, [edx+20]

        stdcall fix_coff_symbols, eax, [sym], [edx+COFF_HEADER.nSymbols], \
                [strings], __exports
        test    eax, eax
        jz      .link_fail

        mov     ebx, [coff]
        stdcall fix_coff_relocs, ebx, [sym], 0

        stdcall get_coff_sym, [sym], [ebx+COFF_HEADER.nSymbols], szVersion
        test    eax, eax
        jz      .link_fail

        mov     eax, [eax]
        shr     eax, 16
        cmp     eax, DRV_COMPAT
        jb      .ver_fail

        cmp     eax, DRV_CURRENT
        ja      .ver_fail

        mov     ebx, [coff]
        stdcall get_coff_sym, [sym], [ebx+COFF_HEADER.nSymbols], szSTART
        mov     [start], eax

        stdcall kernel_free, [coff]

        mov     ebx, [start]
        stdcall ebx, DRV_ENTRY
        test    eax, eax
        jnz     .ok

        stdcall kernel_free, [img_base]

        xor     eax, eax
        ret
.ok:
        mov     ebx, [img_base]
        mov     [eax+SRV.base], ebx
        mov     ecx, [start]
        mov     [eax+SRV.entry], ecx
        ret

.ver_fail:
        mov     esi, msg_CR
        call    sys_msg_board_str
        mov     esi, [driver_name]
        call    sys_msg_board_str
        mov     esi, msg_CR
        call    sys_msg_board_str
        mov     esi, msg_version
        call    sys_msg_board_str
        mov     esi, msg_www
        call    sys_msg_board_str
        jmp     .cleanup

.link_fail:
        mov     esi, msg_module
        call    sys_msg_board_str
        mov     esi, [driver_name]
        call    sys_msg_board_str
        mov     esi, msg_CR
        call    sys_msg_board_str
.cleanup:
        stdcall kernel_free, [img_base]
.fail:
        stdcall kernel_free, [coff]
.exit:
        xor     eax, eax
        ret
endp

; in: edx -> COFF_SECTION struct
; out: eax = alignment as mask for bits to drop
coff_get_align:
; Rules:
; - if alignment is not given, use default = 4K;
; - if alignment is given and is no more than 4K, use it;
; - if alignment is more than 4K, revert to 4K.
        push    ecx
        mov     cl, byte [edx+COFF_SECTION.Characteristics+2]
        mov     eax, 1
        shr     cl, 4
        dec     cl
        js      .default
        cmp     cl, 12
        jbe     @f
.default:
        mov     cl, 12
@@:
        shl     eax, cl
        pop     ecx
        dec     eax
        ret

align 4
proc load_library stdcall, file_name:dword
           locals
             fullname  rb 260
             fileinfo  rb 40
             coff      dd ?
             img_base  dd ?
           endl

        cli

; resolve file name
        mov     ebx, [file_name]
        lea     edi, [fullname+1]
        mov     byte [edi-1], '/'
        stdcall get_full_file_name, edi, 259
        test    al, al
        jz      .fail

; scan for required DLL in list of already loaded for this process,
; ignore timestamp
        mov     esi, [CURRENT_TASK]
        shl     esi, 8
        lea     edi, [fullname]
        mov     ebx, [esi+SLOT_BASE+APPDATA.dlls_list_ptr]
        test    ebx, ebx
        jz      .not_in_process
        mov     esi, [ebx+HDLL.fd]
.scan_in_process:
        cmp     esi, ebx
        jz      .not_in_process
        mov     eax, [esi+HDLL.parent]
        add     eax, DLLDESCR.name
        stdcall strncmp, eax, edi, -1
        test    eax, eax
        jnz     .next_in_process
; simple variant: load DLL which is already loaded in this process
; just increment reference counters and return address of exports table
        inc     [esi+HDLL.refcount]
        mov     ecx, [esi+HDLL.parent]
        inc     [ecx+DLLDESCR.refcount]
        mov     eax, [ecx+DLLDESCR.exports]
        sub     eax, [ecx+DLLDESCR.defaultbase]
        add     eax, [esi+HDLL.base]
        ret
.next_in_process:
        mov     esi, [esi+HDLL.fd]
        jmp     .scan_in_process
.not_in_process:

; scan in full list, compare timestamp
        lea     eax, [fileinfo]
        stdcall get_fileinfo, edi, eax
        test    eax, eax
        jnz     .fail
        mov     esi, [dll_list.fd]
.scan_for_dlls:
        cmp     esi, dll_list
        jz      .load_new
        lea     eax, [esi+DLLDESCR.name]
        stdcall strncmp, eax, edi, -1
        test    eax, eax
        jnz     .continue_scan
.test_prev_dll:
        mov     eax, dword [fileinfo+24]; last modified time
        mov     edx, dword [fileinfo+28]; last modified date
        cmp     dword [esi+DLLDESCR.timestamp], eax
        jnz     .continue_scan
        cmp     dword [esi+DLLDESCR.timestamp+4], edx
        jz      .dll_already_loaded
.continue_scan:
        mov     esi, [esi+DLLDESCR.fd]
        jmp     .scan_for_dlls

; new DLL
.load_new:
; load file
        stdcall load_file, edi
        test    eax, eax
        jz      .fail
        mov     [coff], eax
        mov     dword [fileinfo+32], ebx

; allocate DLLDESCR struct; size is DLLDESCR.sizeof plus size of DLL name
        mov     esi, edi
        mov     ecx, -1
        xor     eax, eax
        repnz scasb
        not     ecx
        lea     eax, [ecx+sizeof.DLLDESCR]
        push    ecx
        call    malloc
        pop     ecx
        test    eax, eax
        jz      .fail_and_free_coff
; save timestamp
        lea     edi, [eax+DLLDESCR.name]
        rep movsb
        mov     esi, eax
        mov     eax, dword [fileinfo+24]
        mov     dword [esi+DLLDESCR.timestamp], eax
        mov     eax, dword [fileinfo+28]
        mov     dword [esi+DLLDESCR.timestamp+4], eax
; initialize DLLDESCR struct
        and     dword [esi+DLLDESCR.refcount], 0; no HDLLs yet; later it will be incremented
        mov     [esi+DLLDESCR.fd], dll_list
        mov     eax, [dll_list.bk]
        mov     [dll_list.bk], esi
        mov     [esi+DLLDESCR.bk], eax
        mov     [eax+DLLDESCR.fd], esi

; calculate size of loaded DLL
        mov     edx, [coff]
        movzx   ecx, [edx+COFF_HEADER.nSections]
        xor     ebx, ebx

        add     edx, 20
@@:
        call    coff_get_align
        add     ebx, eax
        not     eax
        and     ebx, eax
        add     ebx, [edx+COFF_SECTION.SizeOfRawData]
        add     edx, sizeof.COFF_SECTION
        dec     ecx
        jnz     @B
; it must be nonzero and not too big
        mov     [esi+DLLDESCR.size], ebx
        test    ebx, ebx
        jz      .fail_and_free_dll
        cmp     ebx, MAX_DEFAULT_DLL_ADDR-MIN_DEFAULT_DLL_ADDR
        ja      .fail_and_free_dll
; allocate memory for kernel-side image
        stdcall kernel_alloc, ebx
        test    eax, eax
        jz      .fail_and_free_dll
        mov     [esi+DLLDESCR.data], eax
; calculate preferred base address
        add     ebx, 0x1FFF
        and     ebx, not 0xFFF
        mov     ecx, [dll_cur_addr]
        lea     edx, [ecx+ebx]
        cmp     edx, MAX_DEFAULT_DLL_ADDR
        jb      @f
        mov     ecx, MIN_DEFAULT_DLL_ADDR
        lea     edx, [ecx+ebx]
@@:
        mov     [esi+DLLDESCR.defaultbase], ecx
        mov     [dll_cur_addr], edx

; copy sections and set correct values for VirtualAddress'es in headers
        push    esi
        mov     edx, [coff]
        movzx   ebx, [edx+COFF_HEADER.nSections]
        mov     edi, eax
        add     edx, 20
        cld
@@:
        call    coff_get_align
        add     ecx, eax
        add     edi, eax
        not     eax
        and     ecx, eax
        and     edi, eax
        mov     [edx+COFF_SECTION.VirtualAddress], ecx
        add     ecx, [edx+COFF_SECTION.SizeOfRawData]
        mov     esi, [edx+COFF_SECTION.PtrRawData]
        push    ecx
        mov     ecx, [edx+COFF_SECTION.SizeOfRawData]
        test    esi, esi
        jnz     .copy
        xor     eax, eax
        rep stosb
        jmp     .next
.copy:
        add     esi, [coff]
        rep movsb
.next:
        pop     ecx
        add     edx, sizeof.COFF_SECTION
        dec     ebx
        jnz     @B
        pop     esi

; save some additional data from COFF file
; later we will use COFF header, headers for sections and symbol table
; and also relocations table for all sections
        mov     edx, [coff]
        mov     ebx, [edx+COFF_HEADER.pSymTable]
        mov     edi, dword [fileinfo+32]
        sub     edi, ebx
        jc      .fail_and_free_data
        mov     [esi+DLLDESCR.symbols_lim], edi
        add     ebx, edx
        movzx   ecx, [edx+COFF_HEADER.nSections]
        lea     ecx, [ecx*5]
        lea     edi, [edi+ecx*8+20]
        add     edx, 20
@@:
        movzx   eax, [edx+COFF_SECTION.NumReloc]
        lea     eax, [eax*5]
        lea     edi, [edi+eax*2]
        add     edx, sizeof.COFF_SECTION
        sub     ecx, 5
        jnz     @b
        stdcall kernel_alloc, edi
        test    eax, eax
        jz      .fail_and_free_data
        mov     edx, [coff]
        movzx   ecx, [edx+COFF_HEADER.nSections]
        lea     ecx, [ecx*5]
        lea     ecx, [ecx*2+5]
        mov     [esi+DLLDESCR.coff_hdr], eax
        push    esi
        mov     esi, edx
        mov     edi, eax
        rep movsd
        pop     esi
        mov     [esi+DLLDESCR.symbols_ptr], edi
        push    esi
        mov     ecx, [edx+COFF_HEADER.nSymbols]
        mov     [esi+DLLDESCR.symbols_num], ecx
        mov     ecx, [esi+DLLDESCR.symbols_lim]
        mov     esi, ebx
        rep movsb
        pop     esi
        mov     ebx, [esi+DLLDESCR.coff_hdr]
        push    esi
        movzx   eax, [edx+COFF_HEADER.nSections]
        lea     edx, [ebx+20]
@@:
        movzx   ecx, [edx+COFF_SECTION.NumReloc]
        lea     ecx, [ecx*5]
        mov     esi, [edx+COFF_SECTION.PtrReloc]
        mov     [edx+COFF_SECTION.PtrReloc], edi
        sub     [edx+COFF_SECTION.PtrReloc], ebx
        add     esi, [coff]
        shr     ecx, 1
        rep movsd
        adc     ecx, ecx
        rep movsw
        add     edx, sizeof.COFF_SECTION
        dec     eax
        jnz     @b
        pop     esi

; fixup symbols
        mov     edx, ebx
        mov     eax, [ebx+COFF_HEADER.nSymbols]
        add     edx, 20
        mov     ecx, [esi+DLLDESCR.symbols_num]
        lea     ecx, [ecx*9]
        add     ecx, ecx
        add     ecx, [esi+DLLDESCR.symbols_ptr]

        stdcall fix_coff_symbols, edx, [esi+DLLDESCR.symbols_ptr], eax, \
                ecx, 0
;          test eax, eax
;          jnz @F
;
;@@:

        stdcall get_coff_sym, [esi+DLLDESCR.symbols_ptr], [ebx+COFF_HEADER.nSymbols], szEXPORTS
        test    eax, eax
        jnz     @F

        stdcall get_coff_sym, [esi+DLLDESCR.symbols_ptr], [ebx+COFF_HEADER.nSymbols], sz_EXPORTS
@@:
        mov     [esi+DLLDESCR.exports], eax

; fix relocs in the hidden copy in kernel memory to default address
; it is first fix; usually this will be enough, but second fix
; can be necessary if real load address will not equal assumption
        mov     eax, [esi+DLLDESCR.data]
        sub     eax, [esi+DLLDESCR.defaultbase]
        stdcall fix_coff_relocs, ebx, [esi+DLLDESCR.symbols_ptr], eax

        stdcall kernel_free, [coff]

.dll_already_loaded:
        inc     [esi+DLLDESCR.refcount]
        push    esi
        call    init_heap
        pop     esi

        mov     edi, [esi+DLLDESCR.size]
        stdcall user_alloc_at, [esi+DLLDESCR.defaultbase], edi
        test    eax, eax
        jnz     @f
        stdcall user_alloc, edi
        test    eax, eax
        jz      .fail_and_dereference
@@:
        mov     [img_base], eax
        mov     eax, sizeof.HDLL
        call    malloc
        test    eax, eax
        jz      .fail_and_free_user
        mov     ebx, [CURRENT_TASK]
        shl     ebx, 5
        mov     edx, [CURRENT_TASK+ebx+TASKDATA.pid]
        mov     [eax+HDLL.pid], edx
        push    eax
        call    init_dlls_in_thread
        pop     ebx
        test    eax, eax
        jz      .fail_and_free_user
        mov     edx, [eax+HDLL.fd]
        mov     [ebx+HDLL.fd], edx
        mov     [ebx+HDLL.bk], eax
        mov     [eax+HDLL.fd], ebx
        mov     [edx+HDLL.bk], ebx
        mov     eax, ebx
        mov     ebx, [img_base]
        mov     [eax+HDLL.base], ebx
        mov     [eax+HDLL.size], edi
        mov     [eax+HDLL.refcount], 1
        mov     [eax+HDLL.parent], esi
        mov     edx, ebx
        shr     edx, 12
        or      dword [page_tabs+(edx-1)*4], DONT_FREE_BLOCK
; copy entries of page table from kernel-side image to usermode
; use copy-on-write for user-mode image, so map as readonly
        xor     edi, edi
        mov     ecx, [esi+DLLDESCR.data]
        shr     ecx, 12
.map_pages_loop:
        mov     eax, [page_tabs+ecx*4]
        and     eax, not 0xFFF
        or      al, PG_USER
        xchg    eax, [page_tabs+edx*4]
        test    al, 1
        jz      @f
        call    free_page
@@:
        invlpg  [ebx+edi]
        inc     ecx
        inc     edx
        add     edi, 0x1000
        cmp     edi, [esi+DLLDESCR.size]
        jb      .map_pages_loop

; if real user-mode base is not equal to preferred base, relocate image
        sub     ebx, [esi+DLLDESCR.defaultbase]
        jz      @f
        stdcall rebase_coff, [esi+DLLDESCR.coff_hdr], [esi+DLLDESCR.symbols_ptr], ebx
@@:

        mov     eax, [esi+DLLDESCR.exports]
        sub     eax, [esi+DLLDESCR.defaultbase]
        add     eax, [img_base]
        ret
.fail_and_free_data:
        stdcall kernel_free, [esi+DLLDESCR.data]
.fail_and_free_dll:
        mov     eax, esi
        call    free
.fail_and_free_coff:
        stdcall kernel_free, [coff]
.fail:
        xor     eax, eax
        ret
.fail_and_free_user:
        stdcall user_free, [img_base]
.fail_and_dereference:
        mov     eax, 1  ; delete 1 reference
        call    dereference_dll
        xor     eax, eax
        ret
endp

; initialize [APPDATA.dlls_list_ptr] for given thread
; DLL is per-process object, so APPDATA.dlls_list_ptr must be
; kept in sync for all threads of one process.
; out: eax = APPDATA.dlls_list_ptr if all is OK,
; NULL if memory allocation failed
init_dlls_in_thread:
        mov     ebx, [current_slot]
        mov     eax, [ebx+APPDATA.dlls_list_ptr]
        test    eax, eax
        jnz     .ret
        push    [ebx+APPDATA.dir_table]
        mov     eax, 8
        call    malloc
        pop     edx
        test    eax, eax
        jz      .ret
        mov     [eax], eax
        mov     [eax+4], eax
        mov     ecx, [TASK_COUNT]
        mov     ebx, SLOT_BASE+256
.set:
        cmp     [ebx+APPDATA.dir_table], edx
        jnz     @f
        mov     [ebx+APPDATA.dlls_list_ptr], eax
@@:
        add     ebx, 256
        dec     ecx
        jnz     .set
.ret:
        ret

; in: eax = number of references to delete, esi -> DLLDESCR struc
dereference_dll:
        sub     [esi+DLLDESCR.refcount], eax
        jnz     .ret
        mov     eax, [esi+DLLDESCR.fd]
        mov     edx, [esi+DLLDESCR.bk]
        mov     [eax+DLLDESCR.bk], edx
        mov     [edx+DLLDESCR.fd], eax
        stdcall kernel_free, [esi+DLLDESCR.coff_hdr]
        stdcall kernel_free, [esi+DLLDESCR.data]
        mov     eax, esi
        call    free
.ret:
        ret

destroy_hdll:
        push    ebx ecx esi edi
        push    eax
        mov     ebx, [eax+HDLL.base]
        mov     esi, [eax+HDLL.parent]
        mov     edx, [esi+DLLDESCR.size]
; The following actions require the context of application where HDLL is mapped.
; However, destroy_hdll can be called in the context of OS thread when
; cleaning up objects created by the application which is destroyed.
; So remember current cr3 and set it to page table of target.
        mov     eax, [ecx+APPDATA.dir_table]
; Because we cheat with cr3, disable interrupts: task switch would restore
; page table from APPDATA of current thread.
; Also set [current_slot] because it is used by user_free.
        pushf
        cli
        push    [current_slot]
        mov     [current_slot], ecx
        mov     ecx, cr3
        push    ecx
        mov     cr3, eax
        push    ebx     ; argument for user_free
        mov     eax, ebx
        shr     ebx, 12
        push    ebx
        mov     esi, [esi+DLLDESCR.data]
        shr     esi, 12
.unmap_loop:
        push    eax
        mov     eax, 2
        xchg    eax, [page_tabs+ebx*4]
        mov     ecx, [page_tabs+esi*4]
        and     eax, not 0xFFF
        and     ecx, not 0xFFF
        cmp     eax, ecx
        jz      @f
        call    free_page
@@:
        pop     eax
        invlpg  [eax]
        add     eax, 0x1000
        inc     ebx
        inc     esi
        sub     edx, 0x1000
        ja      .unmap_loop
        pop     ebx
        and     dword [page_tabs+(ebx-1)*4], not DONT_FREE_BLOCK
        call    user_free
; Restore context.
        pop     eax
        mov     cr3, eax
        pop     [current_slot]
        popf
; Ok, cheating is done.
        pop     eax
        push    eax
        mov     esi, [eax+HDLL.parent]
        mov     eax, [eax+HDLL.refcount]
        call    dereference_dll
        pop     eax
        mov     edx, [eax+HDLL.bk]
        mov     ebx, [eax+HDLL.fd]
        mov     [ebx+HDLL.bk], edx
        mov     [edx+HDLL.fd], ebx
        call    free
        pop     edi esi ecx ebx
        ret

; ecx -> APPDATA for slot, esi = dlls_list_ptr
destroy_all_hdlls:
        test    esi, esi
        jz      .ret
.loop:
        mov     eax, [esi+HDLL.fd]
        cmp     eax, esi
        jz      free
        call    destroy_hdll
        jmp     .loop
.ret:
        ret

align 4
stop_all_services:
        push    ebp
        mov     edx, [srv.fd]
.next:
        cmp     edx, srv.fd-SRV.fd
        je      .done
        cmp     [edx+SRV.magic], ' SRV'
        jne     .next
        cmp     [edx+SRV.size], sizeof.SRV
        jne     .next

        mov     ebx, [edx+SRV.entry]
        mov     edx, [edx+SRV.fd]
        test    ebx, ebx
        jz      .next

        push    edx
        mov     ebp, esp
        push    0
        push    -1
        call    ebx
        mov     esp, ebp
        pop     edx
        jmp     .next
.done:
        pop     ebp
        ret

; param
;  eax= size
;  ebx= pid

align 4
create_kernel_object:

        push    ebx
        call    malloc
        pop     ebx
        test    eax, eax
        jz      .fail

        mov     ecx, [current_slot]
        add     ecx, APP_OBJ_OFFSET

        pushfd
        cli
        mov     edx, [ecx+APPOBJ.fd]
        mov     [eax+APPOBJ.fd], edx
        mov     [eax+APPOBJ.bk], ecx
        mov     [eax+APPOBJ.pid], ebx

        mov     [ecx+APPOBJ.fd], eax
        mov     [edx+APPOBJ.bk], eax
        popfd
.fail:
        ret

; param
;  eax= object

align 4
destroy_kernel_object:

        pushfd
        cli
        mov     ebx, [eax+APPOBJ.fd]
        mov     ecx, [eax+APPOBJ.bk]
        mov     [ebx+APPOBJ.bk], ecx
        mov     [ecx+APPOBJ.fd], ebx
        popfd

        xor     edx, edx       ;clear common header
        mov     [eax], edx
        mov     [eax+4], edx
        mov     [eax+8], edx
        mov     [eax+12], edx
        mov     [eax+16], edx

        call    free           ;release object memory
        ret
