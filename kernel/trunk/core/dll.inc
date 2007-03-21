$Revision$

DRV_ENTRY    equ  1
DRV_EXIT     equ -1
DRV_COMPAT   equ  4  ;minimal required drivers version
DRV_CURRENT  equ  4  ;current drivers model version

DRV_VERSION equ (DRV_COMPAT shl 16) or DRV_CURRENT

align 4
proc attach_int_handler stdcall, irq:dword, handler:dword

	 mov ebx, [irq]        ;irq num
	 test ebx, ebx
	 jz .err
	 mov eax, [handler]
	 test eax, eax
	 jz .err
	 mov [irq_tab+ebx*4], eax
         stdcall enable_irq, [irq]
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
	   push eax
	   mov eax, 1
	   jmp .main
align 4
.irq_2:
	   push eax
	   mov eax, 2
	   jmp .main
align 4
.irq_3:
	   push eax
	   mov eax, 3
	   jmp .main
align 4
.irq_4:
	   push eax
	   mov eax, 4
	   jmp .main
align 4
.irq_5:
	   push eax
	   mov eax, 5
	   jmp .main
align 4
.irq_6:
	   push eax
	   mov eax, 6
	   jmp .main
align 4
.irq_7:
	   push eax
	   mov eax, 7
	   jmp .main
align 4
.irq_8:
	   push eax
	   mov eax, 8
	   jmp .main
align 4
.irq_9:
	   push eax
	   mov eax, 9
	   jmp .main
align 4
.irq_10:
	   push eax
	   mov eax, 10
	   jmp .main
align 4
.irq_11:
	   push eax
	   mov eax, 11
	   jmp .main
align 4
.irq_12:
	   push eax
	   mov eax, 12
	   jmp .main
align 4
.irq_13:
	   push eax
	   mov eax, 13
	   jmp .main
align 4
.irq_14:
	   push eax
	   mov eax, 14
	   jmp .main
align 4
.irq_15:
	   push eax
	   mov eax, 15
	   jmp .main

align 16
.main:
	   save_ring3_context
	   mov	 bx, os_data
	   mov	 ds, bx
	   mov	 es, bx

	   mov ebx, [irq_tab+eax*4]
	   test ebx, ebx
	   jz .exit

           call ebx

.exit:
	   restore_ring3_context

           cmp eax, 8
	   mov al, 0x20
           jb @f
	   out 0xa0, al
@@:
           out 0x20, al

           pop eax
	   iret

align 4
proc get_notify stdcall, p_ev:dword

.wait:
           mov ebx,[CURRENT_TASK]
           shl ebx,8
           test dword [ebx+SLOT_BASE+0xA8],EVENT_NOTIFY
	   jz @f
           and dword [ebx+SLOT_BASE+0xA8], not EVENT_NOTIFY
	   mov edi, [p_ev]
	   mov dword [edi], EV_INTR
           mov eax, [ebx+SLOT_BASE+APPDATA.event]
	   mov dword [edi+4], eax
	   ret
@@:
	   call change_task
	   jmp .wait
endp

align 4
proc pci_read32 stdcall, bus:dword, devfn:dword, reg:dword
	   xor eax, eax
	   xor ebx, ebx
	   mov ah, byte [bus]
           mov al, 6
	   mov bh, byte [devfn]
	   mov bl, byte [reg]
	   call pci_read_reg
	   ret
endp

align 4
proc pci_read8 stdcall, bus:dword, devfn:dword, reg:dword
	   xor eax, eax
	   xor ebx, ebx
	   mov ah, byte [bus]
           mov al, 4
	   mov bh, byte [devfn]
	   mov bl, byte [reg]
	   call pci_read_reg
	   ret
endp

align 4
proc pci_write8 stdcall, bus:dword, devfn:dword, reg:dword, val:dword
	   xor eax, eax
	   xor ebx, ebx
	   mov ah, byte [bus]
           mov al, 8
	   mov bh, byte [devfn]
	   mov bl, byte [reg]
           mov ecx, [val]
           call pci_write_reg
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
           test ebx, ebx
           jz .fail
           add ebx, new_app_base

           mov eax, [ebx+handle]
           cmp [eax+SRV.magic], ' SRV'
	   jne .fail

           cmp [eax+SRV.size], SRV_SIZE
	   jne .fail

           add [ebx+input], new_app_base
           add [ebx+output], new_app_base

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
reg_service:
.sz_name equ esp+4
.handler equ esp+8
           mov eax, [.sz_name]
	   test eax, eax
	   jz .fail

           mov ebx, [.handler]
	   test ebx, ebx
	   jz .fail

           mov eax, SRV_SIZE
           call malloc           ;call alloc_service
	   test eax, eax
	   jz .fail

	   mov edi, eax
           mov esi, [.sz_name]
           mov ecx, 16/4
           rep movsd

           mov [eax+SRV.magic], ' SRV'
           mov [eax+SRV.size], SRV_SIZE

           mov ebx, srv.fd-SRV_FD_OFFSET
           mov edx, [ebx+SRV.fd]
           mov [eax+SRV.fd], edx
           mov [eax+SRV.bk], ebx
           mov [ebx+SRV.fd], eax
           mov [edx+SRV.bk], eax

           mov ecx, [.handler]
           mov [eax+SRV.srv_proc], ecx
           ret 8
.fail:
	   xor eax, eax
           ret 8

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
proc get_curr_task
           mov eax,[CURRENT_TASK]
           shl eax, 8
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

           xor eax, eax
           mov ebx, [file_name]
           sub ebx, new_app_base
           mov ecx, [info]
           sub ecx, new_app_base

           mov [cmd], 5
           mov [offset], eax
           mov [offset+4], eax
           mov [count], eax
           mov [buff], ecx
           mov byte [buff+4], al
           mov [name], ebx

           mov eax, 70
           lea ebx, [cmd]
           sub ebx, new_app_base
           int 0x40
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
           sub ebx, new_app_base
           sub esi, new_app_base

           mov [cmd], eax
           mov [offset], ecx
           mov [offset+4], eax
           mov [count], edx
           mov [buff], esi
           mov byte [buff+4], al
           mov [name], ebx

           mov eax, 70
           lea ebx, [cmd]
           sub ebx, new_app_base
           int 0x40
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

           lea eax, [attr]
           stdcall get_fileinfo, [file_name], eax
           test eax, eax
           jnz .fail

           mov eax, [file_size]
           cmp eax, 1024*1024*16
           ja .fail

           stdcall kernel_alloc, [file_size]
           mov [file], eax

           stdcall read_file, [file_name], eax, dword 0, [file_size]
           cmp ebx, [file_size]
           jne .cleanup

           mov eax, [file]
           cmp dword [eax], 0x4B43504B
           jne .exit
           mov ebx, [eax+4]
           mov [file_size], ebx
           stdcall kernel_alloc, ebx

           test eax, eax
           jz .cleanup

           mov [file2], eax
           stdcall unpack, [file], eax
           stdcall kernel_free, [file]
           mov eax, [file2]
           mov ebx, [file_size]
.exit:
           push eax
           lea edi, [eax+ebx]     ;cleanup remain space
           mov ecx, 4096          ;from file end
           and ebx, 4095
           sub ecx, ebx
           xor eax, eax
           cld
           rep stosb
           mov ebx, [file_size]
           pop eax
           ret
.cleanup:
           stdcall kernel_free, [file]
.fail:
           xor eax, eax
           xor ebx, ebx
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
             file_name rb 14+16+4+1      ; '/rd/1/drivers/<up-to-16-chars>.obj'
           endl

        lea     edx, [file_name]
        mov     dword [edx], '/rd/'
        mov     dword [edx+4], '1/dr'
        mov     dword [edx+8], 'iver'
        mov     word [edx+12], 's/'
        mov     esi, [driver_name]
        lea     edi, [edx+14]
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

           test eax, eax
           jz .exit

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

           stdcall kernel_alloc, ebx
           test eax, eax
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

           stdcall kernel_free, [coff]

           mov ebx, [start]
           stdcall ebx, DRV_ENTRY
           test eax, eax
           jnz .ok

           stdcall kernel_free, [img_base]
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
           stdcall kernel_free,[img_base]
.fail:
           stdcall kernel_free, [coff]
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
           add edi, new_app_base
           mov ecx, [eax+CFS.SizeOfRawData]
           cld
           rep movsb
.next:
           add edi, 15-new_app_base
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

           lea eax, [edx+20]

           stdcall fix_coff_symbols, eax, [sym], [edx+CFH.nSymbols],\
                                     [strings], dword 0
           test eax, eax
           jnz @F

@@:
           mov edx, [coff]
           movzx ebx, [edx+CFH.nSections]
           mov edi, new_app_base
           lea eax, [edx+20]
@@:
           add [eax+CFS.VirtualAddress], edi  ;patch user space offset
           add eax, COFF_SECTION_SIZE
           dec ebx
           jnz @B

           add edx, 20
           stdcall fix_coff_relocs, [coff], edx, [sym]

           mov ebx, [coff]
           stdcall get_coff_sym,[sym],[ebx+CFH.nSymbols],szEXPORTS
           mov [exports], eax

           stdcall kernel_free, [coff]
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
create_kernel_object:

           push ebx
           call malloc
           pop ebx
           test eax, eax
           jz .fail

           mov ecx,[CURRENT_TASK]
           shl ecx,8
           add ecx, SLOT_BASE+APP_OBJ_OFFSET

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


;szSound        db 'SOUND',0
;szInfinity     db 'INFINITY',0
szHwMouse      db 'ATI2D',0

szSTART        db 'START',0
szEXPORTS      db 'EXPORTS',0
szIMPORTS      db 'IMPORTS',0

msg_unresolved db 'unresolved ',0
msg_module     db 'in module ',0
msg_version    db 'incompatible driver version',13,10,0
msg_www        db 'please visit www.kolibrios.org',13,10,0
msg_CR         db  13,10,0

align 4
create_cursor    dd 0
set_hw_cursor    dd 0
hw_restore       dd 0
