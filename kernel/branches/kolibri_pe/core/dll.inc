;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


DRV_COMPAT   equ  5  ;minimal required drivers version
DRV_CURRENT  equ  5  ;current drivers model version

DRV_VERSION equ (DRV_COMPAT shl 16) or DRV_CURRENT
PID_KERNEL  equ 1    ;os_idle thread

align 4
proc attach_int_handler stdcall, irq:dword, handler:dword, access_rights:dword

         push ebx

           mov  ebx, [irq]                   ;irq num
           test ebx, ebx
           jz   .err
           cmp  ebx, 15                      ; hidnplayr says: we only have 16 IRQ's
           ja   .err
           mov  eax, [handler]
           test eax, eax
           jz   .err
           cmp  [irq_owner + 4 * ebx], 0
           je   @f

           mov  ecx, [irq_rights + 4 * ebx]  ; Rights : 0 - full access, 1 - read only, 2 - forbidden
           test ecx, ecx
           jnz  .err

@@:
           mov  [irq_tab+ebx*4], eax

           mov  eax, [access_rights]
           mov  [irq_rights + 4 * ebx], eax

           mov  [irq_owner + 4 * ebx], PID_KERNEL  ; all handlers belong to a kernel

           stdcall enable_irq, [irq]
         pop ebx
           mov eax, 1
           ret
.err:
         pop ebx
           xor eax, eax
           ret
endp

uglobal

irq_rights   rd  16

endg

proc get_int_handler stdcall, irq:dword

           mov eax, [irq]

           cmp [irq_rights + 4 * eax], dword 1
           ja  .err

           mov eax, [irq_tab + 4 * eax]
           ret

.err:
           xor eax, eax
           ret
endp

align 4
proc  detach_int_handler

           ret
endp

align 4
proc enable_irq stdcall, irq_line:dword
           mov ebx, [irq_line]
           mov edx, 0x21
           cmp ebx, 8
           jb @F
           mov edx, 0xA1
           sub ebx,8
@@:
           in al,dx
           btr eax, ebx
           out dx, al
           ret
endp

align 16
;; proc irq_serv

irq_serv:

.irq_1:
           push 1
           jmp .main
align 4
.irq_2:
           push 2
           jmp .main
align 4
.irq_3:
           push 3
           jmp .main
align 4
.irq_4:
           push 4
           jmp .main
align 4
.irq_5:
           push 5
           jmp .main
align 4
.irq_6:
           push 6
           jmp .main
align 4
.irq_7:
           push 7
           jmp .main
align 4
.irq_8:
           push 8
           jmp .main
align 4
.irq_9:
           push 9
           jmp .main
align 4
.irq_10:
           push 10
           jmp .main
align 4
.irq_11:
           push 11
           jmp .main
align 4
.irq_12:
           push 12
           jmp .main
align 4
.irq_13:
           push 13
           jmp .main
align 4
.irq_14:
           push 14
           jmp .main
align 4
.irq_15:
           push 15
           jmp .main

align 16
.main:
           save_ring3_context
           mov   eax, [esp + 32]
           mov   bx, sel_app_data
           mov   ds, bx
           mov   es, bx

           cmp   [v86_irqhooks+eax*8], 0
           jnz   v86_irq

           mov ebx, [irq_tab+eax*4]
           test ebx, ebx
           jz .exit

           call ebx
           mov  [check_idle_semaphore],5

.exit:

           cmp dword [esp + 32], 8
           mov al, 0x20
           jb @f

           out 0xa0, al
@@:
           out 0x20, al

           restore_ring3_context
           add   esp, 4

           iret

align 4
proc get_notify stdcall, p_ev:dword

.wait:
            mov ebx,[current_slot]
            test dword [ebx+APPDATA.event_mask],EVENT_NOTIFY
            jz @f

            and dword [ebx+APPDATA.event_mask], not EVENT_NOTIFY
            mov edi, [p_ev]
            mov dword [edi], EV_INTR
            mov eax, [ebx+APPDATA.event]
            mov dword [edi+4], eax
            ret
@@:
            call change_task
            jmp .wait
endp

align 4
_PciRead32:
proc pci_read32 stdcall, bus:dword, devfn:dword, reg:dword
            push ebx
            xor eax, eax
            xor ebx, ebx
            mov ah, byte [bus]
            mov al, 6
            mov bh, byte [devfn]
            mov bl, byte [reg]
            call pci_read_reg
            pop ebx
            ret
endp

align 4
_PciRead16:
proc pci_read16 stdcall, bus:dword, devfn:dword, reg:dword
            push ebx
            xor eax, eax
            xor ebx, ebx
            mov ah, byte [bus]
            mov al, 5
            mov bh, byte [devfn]
            mov bl, byte [reg]
            call pci_read_reg
            pop ebx
            ret
endp

align 4
_PciRead8:
proc pci_read8 stdcall, bus:dword, devfn:dword, reg:dword
           push ebx
           xor eax, eax
           xor ebx, ebx
           mov ah, byte [bus]
           mov al, 4
           mov bh, byte [devfn]
           mov bl, byte [reg]
           call pci_read_reg
           pop ebx
           ret
endp

align 4
_PciWrite8:
proc pci_write8 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
           push ebx
           xor eax, eax
           xor ebx, ebx
           mov ah, byte [bus]
           mov al, 8
           mov bh, byte [devfn]
           mov bl, byte [reg]
           mov ecx, [val]
           call pci_write_reg
           pop ebx
           ret
endp

align 4
_PciWrite16:
proc pci_write16 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
           push ebx
           xor eax, eax
           xor ebx, ebx
           mov ah, byte [bus]
           mov al, 9
           mov bh, byte [devfn]
           mov bl, byte [reg]
           mov ecx, [val]
           call pci_write_reg
           pop ebx
           ret
endp

align 4
_PciWrite32:
proc pci_write32 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
           push ebx
           xor eax, eax
           xor ebx, ebx
           mov ah, byte [bus]
           mov al, 10
           mov bh, byte [devfn]
           mov bl, byte [reg]
           mov ecx, [val]
           call pci_write_reg
           pop ebx
           ret
endp

handle	   equ	IOCTL.handle
io_code    equ	IOCTL.io_code
input	   equ	IOCTL.input
inp_size   equ	IOCTL.inp_size
output	   equ	IOCTL.output
out_size   equ	IOCTL.out_size


align 4
proc srv_handler stdcall, ioctl:dword
           mov esi, [ioctl]
           test esi, esi
           jz .err

           mov edi, [esi+handle]
           cmp [edi+SRV.magic], ' SRV'
           jne .fail

           cmp [edi+SRV.size], SRV_SIZE
           jne .fail

           stdcall [edi+SRV.srv_proc], esi
           ret
.fail:
           xor eax, eax
           not eax
           mov [esi+output], eax
           mov [esi+out_size], 4
           ret
.err:
           xor eax, eax
           not eax
           ret
endp

; param
;  ebx= io_control
;
; retval
;  eax= error code

align 4
srv_handlerEx:
           cmp ebx, OS_BASE
           jae .fail

           mov eax, [ebx+handle]
           cmp [eax+SRV.magic], ' SRV'
           jne .fail

           cmp [eax+SRV.size], SRV_SIZE
           jne .fail

           stdcall [eax+SRV.srv_proc], ebx
           ret
.fail:
           or eax, -1
           ret

restore  handle
restore  io_code
restore  input
restore  inp_size
restore  output
restore  out_size

align 4
proc get_service stdcall, sz_name:dword
           mov eax, [sz_name]
           test eax, eax
           jnz @F
           ret
@@:
           mov edx, [srv.fd]
@@:
           cmp edx, srv.fd-SRV_FD_OFFSET
           je .not_load

           stdcall strncmp, edx, [sz_name], 16
           test eax, eax
           je .ok

           mov edx, [edx+SRV.fd]
           jmp @B
.not_load:
           pop ebp
           jmp load_driver
.ok:
           mov eax, edx
           ret
endp

align 4
_RegService:
proc reg_service stdcall, name:dword, handler:dword

           push ebx

           xor eax, eax

           cmp [name], eax
           je .fail

           cmp [handler], eax
           je .fail

           mov eax, SRV_SIZE
           call malloc       ;call alloc_service
           test eax, eax
           jz .fail

           push esi
           push edi
           mov edi, eax
           mov esi, [name]
           mov ecx, 16/4
           rep movsd
           pop edi
           pop esi

           mov [eax+SRV.magic], ' SRV'
           mov [eax+SRV.size], SRV_SIZE

           mov ebx, srv.fd-SRV_FD_OFFSET
           mov edx, [ebx+SRV.fd]
           mov [eax+SRV.fd], edx
           mov [eax+SRV.bk], ebx
           mov [ebx+SRV.fd], eax
           mov [edx+SRV.bk], eax

           mov ecx, [handler]
           mov [eax+SRV.srv_proc], ecx
           pop ebx
           ret
.fail:
           xor eax, eax
           pop ebx
           ret
endp

align 4
proc get_proc stdcall, exp:dword, sz_name:dword

           mov edx, [exp]
.next:
           mov eax, [edx]
           test eax, eax
           jz .end

           push edx
           stdcall strncmp, eax, [sz_name], 16
           pop edx
           test eax, eax
           jz .ok

           add edx,8
           jmp .next
.ok:
           mov eax, [edx+4]
.end:
           ret
endp

align 4
proc get_coff_sym stdcall, pSym:dword,count:dword, sz_sym:dword

@@:
           stdcall strncmp, [pSym], [sz_sym], 8
           test eax,eax
           jz .ok
           add [pSym], 18
           dec [count]
           jnz @b

           xor eax, eax
           ret
.ok:
           mov ebx, [pSym]
           mov eax, [ebx+8]
           ret
endp

align 4
get_curr_task:
           mov eax,[CURRENT_TASK]
           shl eax, 8
           ret

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

           xor eax, eax
           mov ebx, [file_name]
           mov ecx, [info]

           mov [cmd], 5
           mov [offset], eax
           mov [offset+4], eax
           mov [count], eax
           mov [buff], ecx
           mov byte [buff+4], al
           mov [name], ebx

           mov eax, 70
           lea ebx, [cmd]
           int 0x41
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

           xor eax, eax
           mov ebx, [file_name]
           mov ecx, [off]
           mov edx, [bytes]
           mov esi, [buffer]

           mov [cmd], eax
           mov [offset], ecx
           mov [offset+4], eax
           mov [count], edx
           mov [buff], esi
           mov byte [buff+4], al
           mov [name], ebx

           pushad
           push eax
           lea eax, [cmd]
           call file_system_lfn
           pop eax
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
;  You mast call mem_free() to delete each file
;  loaded by the load_file() function

align 4
_LoadFile:
_load_file@4:
proc load_file stdcall, file_name:dword
           locals
             attr   dd ?
             flags  dd ?
             cr_time    dd ?
             cr_date    dd ?
             acc_time   dd ?
             acc_date   dd ?
             mod_time   dd ?
             mod_date   dd ?
             file_size  dd ?

             file   dd ?
             file2  dd ?
           endl

           push esi
           push edi

           lea eax, [attr]
           stdcall get_fileinfo, [file_name], eax
           test eax, eax
           jnz .fail

           mov ecx, [file_size]
           cmp ecx, 1024*1024*16
           ja .fail

           mov edx, PG_SW
           call @mem_alloc@8
           mov [file], eax
           test eax, eax
           jz .fail

           stdcall read_file, [file_name], eax, dword 0, [file_size]
           cmp ebx, [file_size]
           jne .cleanup

           mov eax, [file]
           cmp dword [eax], 0x4B43504B
           jne .exit

           mov ecx, [eax+4]
           mov [file_size], ecx
           mov edx, PG_SW
           call @mem_alloc@8
           test eax, eax
           jz .cleanup

           mov [file2], eax
           stdcall unpack, [file], eax

           mov ecx, [file]
           call @mem_free@4

           mov eax, [file2]
           mov ebx, [file_size]
.exit:
           push eax
           lea edi, [eax+ebx]     ;cleanup remain space
           mov ecx, 4096          ;from file end
           and ebx, 4095
           jz  @f
           sub ecx, ebx
           xor eax, eax
           cld
           rep stosb
@@:
           mov ebx, [file_size]
           pop eax
           pop edi
           pop esi
           ret
.cleanup:
           mov ecx, [file]
           call @mem_free@4
.fail:
           xor eax, eax
           xor ebx, ebx
           pop edi
           pop esi
           ret
endp

align 4
proc get_proc_ex stdcall, proc_name:dword, imports:dword

.look_up:
           mov edx, [imports]
           test edx, edx
           jz .end
           mov edx, [edx]
           test edx, edx
           jz .end
.next:
           mov eax, [edx]
           test eax, eax
           jz .next_table

           push edx
           stdcall strncmp, eax, [proc_name], 16
           pop edx
           test eax, eax
           jz .ok

           add edx,8
           jmp .next
.next_table:
           add [imports], 4
           jmp .look_up
.ok:
           mov eax, [edx+4]
           ret
.end:
           xor eax, eax
           ret
endp

align 4
proc fix_coff_symbols stdcall, sec:dword, symbols:dword,\
		      sym_count:dword, strings:dword, imports:dword
           locals
             retval dd ?
           endl

           mov edi, [symbols]
           mov [retval], 1
.fix:
           movzx ebx, [edi+CSYM.SectionNumber]
           test ebx, ebx
           jnz .internal
           mov eax, dword [edi+CSYM.Name]
           test eax, eax
           jnz @F

           mov edi, [edi+4]
           add edi, [strings]
@@:
           push edi
           stdcall get_proc_ex, edi,[imports]
           pop edi

           xor ebx, ebx
           test eax, eax
           jnz @F

           mov esi, msg_unresolved
           call sys_msg_board_str
           mov esi, edi
           call sys_msg_board_str
           mov esi, msg_CR
           call sys_msg_board_str

           mov [retval],0
@@:
           mov edi, [symbols]
           mov [edi+CSYM.Value], eax
           jmp .next
.internal:
           cmp bx, -1
           je .next
           cmp bx, -2
           je .next

           dec ebx
           shl ebx, 3
           lea ebx, [ebx+ebx*4]
           add ebx, [sec]

           mov eax, [ebx+CFS.VirtualAddress]
           add [edi+CSYM.Value], eax
.next:
           add edi, CSYM_SIZE
           mov [symbols], edi
           dec [sym_count]
           jnz .fix
           mov eax, [retval]
           ret
endp

align 4
proc fix_coff_relocs stdcall, coff:dword, sec:dword, sym:dword
           locals
             n_sec     dd ?
           endl

           mov eax, [coff]
           movzx ebx, [eax+CFH.nSections]
           mov [n_sec], ebx
.fix_sec:
           mov esi, [sec]
           mov edi, [esi+CFS.PtrReloc]
           add edi, [coff]

           movzx ecx, [esi+CFS.NumReloc]
           test ecx, ecx
           jz .next
.next_reloc:
           mov ebx, [edi+CRELOC.SymIndex]
           add ebx,ebx
           lea ebx,[ebx+ebx*8]
           add ebx, [sym]

           mov edx, [ebx+CSYM.Value]

           cmp [edi+CRELOC.Type], 6
           je .dir_32

           cmp [edi+CRELOC.Type], 20
           jne .next_reloc
.rel_32:
           mov eax, [edi+CRELOC.VirtualAddress]
           add eax, [esi+CFS.VirtualAddress]
           sub edx, eax
           sub edx, 4
           jmp .fix
.dir_32:
           mov eax, [edi+CRELOC.VirtualAddress]
           add eax, [esi+CFS.VirtualAddress]
.fix:
           add [eax], edx
           add edi, 10
           dec ecx
           jnz .next_reloc
.next:
           add [sec], COFF_SECTION_SIZE
           dec [n_sec]
           jnz .fix_sec
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

             exports   dd ?   ;fake exports table
                   dd ?
             file_name rb 13+16+4+1  ; '/sys/drivers/<up-to-16-chars>.obj'
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
           mov [coff], eax
           test eax, eax
           jz .exit

           movzx ebx, [eax+CFH.nSections]
           lea edx, [eax+20]
           xor ecx, ecx
@@:
           add ecx, [edx+CFS.SizeOfRawData]
           add ecx, 15
           and ecx, not 15
           add edx, COFF_SECTION_SIZE
           dec ebx
           jnz @B

           mov [img_size], ecx
           mov edx, PG_SW
           call @mem_alloc@8
           test eax, eax
           mov [img_base], eax
           jz .fail

           mov [img_base], eax

           mov edi, eax
           xor eax, eax
           mov ecx, [img_size]
           add ecx, 4095
           and ecx, not 4095
           shr ecx, 2
           cld
           rep stosd

           mov edx, [coff]
           movzx ebx, [edx+CFH.nSections]
           mov edi, [img_base]
           lea eax, [edx+20]
@@:
           mov [eax+CFS.VirtualAddress], edi
           mov esi, [eax+CFS.PtrRawData]
           test esi, esi
           jnz .copy
           add edi, [eax+CFS.SizeOfRawData]
           jmp .next
.copy:
           add esi, edx
           mov ecx, [eax+CFS.SizeOfRawData]
           cld
           rep movsb
.next:
           add edi, 15
           and edi, not 15
           add eax, COFF_SECTION_SIZE
           dec ebx
           jnz @B

           mov ebx, [edx+CFH.pSymTable]
           add ebx, edx
           mov [sym], ebx
           mov ecx, [edx+CFH.nSymbols]
           add ecx,ecx
           lea ecx,[ecx+ecx*8] ;ecx*=18 = nSymbols*CSYM_SIZE
           add ecx, [sym]
           mov [strings], ecx

           lea ebx, [exports]
           mov dword [ebx], kernel_export
           mov dword [ebx+4], 0
           lea eax, [edx+20]

           stdcall fix_coff_symbols, eax, [sym], [edx+CFH.nSymbols],\
                         [strings], ebx
           test eax, eax
           jz .link_fail

           mov ebx, [coff]
           add ebx, 20
           stdcall fix_coff_relocs, [coff], ebx, [sym]

           mov ebx, [coff]
           stdcall get_coff_sym,[sym],[ebx+CFH.nSymbols],szVersion
           test eax, eax
           jz .link_fail

           mov eax, [eax]
           shr eax, 16
           cmp eax, DRV_COMPAT
           jb .ver_fail

           cmp eax, DRV_CURRENT
           ja .ver_fail

           mov ebx, [coff]
           stdcall get_coff_sym,[sym],[ebx+CFH.nSymbols],szSTART
           mov [start], eax

           mov ecx, [coff]
           call @mem_free@4

           mov ebx, [start]
           stdcall ebx, DRV_ENTRY
           test eax, eax
           jnz .ok

           mov ecx, [img_base]
           call @mem_free@4
           xor eax, eax
           ret
.ok:
           mov ebx, [img_base]
           mov [eax+SRV.base], ebx
           mov ecx, [start]
           mov [eax+SRV.entry], ecx
           ret

.ver_fail:
           mov esi, msg_CR
           call sys_msg_board_str
           mov esi, [driver_name]
           call sys_msg_board_str
           mov esi, msg_CR
           call sys_msg_board_str
           mov esi, msg_version
           call sys_msg_board_str
           mov esi, msg_www
           call sys_msg_board_str
           jmp .cleanup

.link_fail:
           mov esi, msg_module
           call sys_msg_board_str
           mov esi, [driver_name]
           call sys_msg_board_str
           mov esi, msg_CR
           call sys_msg_board_str
.cleanup:
           mov ecx, [img_base]
           call @mem_free@4
.fail:
           mov ecx, [coff]
           call @mem_free@4
.exit:
           xor eax, eax
           ret
endp

align 4
proc load_library stdcall, file_name:dword
           locals
             coff      dd ?
             sym       dd ?
             strings   dd ?
             img_size  dd ?
             img_base  dd ?
             exports   dd ?
           endl

           cli

           stdcall load_file, [file_name]
           test eax, eax
           jz .fail

           mov [coff], eax
           movzx ecx, [eax+CFH.nSections]
           xor ebx, ebx

           lea edx, [eax+20]
@@:
           add ebx, [edx+CFS.SizeOfRawData]
           add ebx, 15
           and ebx, not 15
           add edx, COFF_SECTION_SIZE
           dec ecx
           jnz @B
           mov [img_size], ebx

           call init_heap
           stdcall user_alloc, [img_size]

           test eax, eax
           jz .fail
           mov [img_base], eax

           mov edx, [coff]
           movzx ebx, [edx+CFH.nSections]
           mov edi, [img_base]
           lea eax, [edx+20]
@@:
           mov [eax+CFS.VirtualAddress], edi
           mov esi, [eax+CFS.PtrRawData]
           test esi, esi
           jnz .copy
           add edi, [eax+CFS.SizeOfRawData]
           jmp .next
.copy:
           add esi, edx
           mov ecx, [eax+CFS.SizeOfRawData]
           cld
           rep movsb
.next:
           add edi, 15 ;-new_app_base
           and edi, -16
           add eax, COFF_SECTION_SIZE
           dec ebx
           jnz @B

           mov ebx, [edx+CFH.pSymTable]
           add ebx, edx
           mov [sym], ebx
           mov ecx, [edx+CFH.nSymbols]
           add ecx,ecx
           lea ecx,[ecx+ecx*8] ;ecx*=18 = nSymbols*CSYM_SIZE
           add ecx, [sym]
           mov [strings], ecx

           lea eax, [edx+20]

           stdcall fix_coff_symbols, eax, [sym], [edx+CFH.nSymbols],\
                         [strings], dword 0
           test eax, eax
           jnz @F

@@:
           mov edx, [coff]

;           movzx ebx, [edx+CFH.nSections]
;           xor edi, edi
;           lea eax, [edx+20]
;@@:
;           add [eax+CFS.VirtualAddress], edi  ;patch user space offset
;           add eax, COFF_SECTION_SIZE
;           dec ebx
;           jnz @B

           add edx, 20
           stdcall fix_coff_relocs, [coff], edx, [sym]

           mov ebx, [coff]
           stdcall get_coff_sym,[sym],[ebx+CFH.nSymbols],szEXPORTS
           mov [exports], eax

           mov ecx, [coff]
           call @mem_free@4

           mov eax, [exports]
           ret
.fail:
           xor eax, eax
           ret
endp

align 4
proc stop_all_services

           mov edx, [srv.fd]
.next:
           cmp edx,  srv.fd-SRV_FD_OFFSET
           je .done
           cmp [edx+SRV.magic], ' SRV'
           jne .next
           cmp [edx+SRV.size], SRV_SIZE
           jne .next

           mov ebx, [edx+SRV.entry]
           mov edx, [edx+SRV.fd]
           test ebx, ebx
           jz .next

           push edx
           stdcall ebx, dword -1
           pop edx
           jmp .next
.done:
           ret
endp

; param
;  eax= size
;  ebx= pid

align 4
_CreateObject:
create_kernel_object:

           push ebx
           call malloc
           pop ebx
           test eax, eax
           jz .fail

           mov ecx,[current_slot]
           add ecx, APP_OBJ_OFFSET

           pushfd
           cli
           mov edx, [ecx+APPOBJ.fd]
           mov [eax+APPOBJ.fd], edx
           mov [eax+APPOBJ.bk], ecx
           mov [eax+APPOBJ.pid], ebx

           mov [ecx+APPOBJ.fd], eax
           mov [edx+APPOBJ.bk], eax
           popfd
.fail:
           ret

; param
;  eax= object

align 4
_DestroyObject:
destroy_kernel_object:

           pushfd
           cli
           mov ebx, [eax+APPOBJ.fd]
           mov ecx, [eax+APPOBJ.bk]
           mov [ebx+APPOBJ.bk], ecx
           mov [ecx+APPOBJ.fd], ebx
           popfd

           xor edx, edx        ;clear common header
           mov [eax], edx
           mov [eax+4], edx
           mov [eax+8], edx
           mov [eax+12], edx
           mov [eax+16], edx

           call free           ;release object memory
           ret



if 0

irq:

.irq0:
	   pusfd
	   pushad
	   push IRQ_0
	   jmp .master
.irq_1:
	   pusfd
	   pushad
	   push IRQ_1
	   jmp .master

.master:
	   mov ax, app_data
	   mov ds, eax
	   mov es, eax
	   mov ebx, [esp+4]  ;IRQ_xx
	   mov eax, [irq_handlers+ebx+4]
	   call intr_handler
	   mov ecx, [esp+4]
	   cmp [irq_actids+ecx*4], 0
	   je @F
	   in al, 0x21
	   bts eax, ecx
	   out 0x21, al
	   mov al, 0x20
	   out 0x20, al
	   jmp .restart

.slave:
	   mov ax, app_data
	   mov ds, eax
	   mov es, eax
	   mov ebx, [esp+4]  ;IRQ_xx
	   mov eax, [irq_handlers+ebx+4]
	   call intr_handler
	   mov ecx, [esp+4]
	   sub ecx, 8
	   cmp [irq_actids+ecx*4], 0
	   je @F
	   in al, 0xA1
	   bts eax, ecx
	   out 0xA1, al
	   mov al, 0x20
	   out 0xA0, al
	   out 0x20, al
.restart:
	   mov ebx, [next_slot]
	   test ebx, ebx
	   jz @F
	   mov [next_task],0
	   mov esi, [prev_slot]
	   call do_change_task
	   add esp, 4
	   iretd

end if




