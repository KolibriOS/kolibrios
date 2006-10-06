;
;   This file is part of the Infinity sound AC97 driver.
;   (C) copyright Serge 2006
;   email: infinity_sound@mail.ru
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.

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
           test dword [ebx+PROC_BASE+0xA8],EVENT_NOTIFY
	   jz @f
           and dword [ebx+PROC_BASE+0xA8], not EVENT_NOTIFY
	   mov edi, [p_ev]
	   mov dword [edi], EV_INTR
           mov eax, [ebx+PROC_BASE+APPDATA.event]
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

align 4
proc srv_handlerEx stdcall, ioctl:dword
           mov esi, [ioctl]
           test esi, esi
           jz .err
           add esi, new_app_base

           mov edi, [esi+handle]
           cmp [edi+SRV.magic], ' SRV'
	   jne .fail

           cmp [edi+SRV.size], SRV_SIZE
	   jne .fail

           add [esi+input], new_app_base
           add [esi+output], new_app_base

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

restore  handle
restore  io_code
restore  input
restore  inp_size
restore  output
restore  out_size

align 4
proc get_service stdcall, sz_name:dword
	   locals
	     srv_ptr  dd ?
	     counter  dd ?
	    endl

	    mov eax, [sz_name]
	    test eax, eax
	    jz .fail

	    mov [srv_ptr], srv_tab
	    mov [counter], 16
@@:
	    stdcall strncmp, [srv_ptr], [sz_name], 16
	    test eax, eax
	    je .ok

	    add [srv_ptr], SRV_SIZE
	    dec [counter]
	    jnz @B
.not_load:

           stdcall find_service, [sz_name]
           test eax, eax
           jz .fail

           stdcall load_lib, eax
           test eax, eax
           jz .fail

            mov [srv_ptr], srv_tab
	    mov [counter], 16
@@:
	    stdcall strncmp, [srv_ptr], [sz_name], 16
	    test eax, eax
	    je .ok

	    add [srv_ptr], SRV_SIZE
	    dec [counter]
	    jnz @B
.fail:
	    xor eax, eax
	    ret
.ok:
	    mov eax, [srv_ptr]
	    ret
endp

align 4
proc find_service stdcall ,sz_name:dword

	   mov eax, [sz_name]
	   test eax, eax
	   jz .fail

           mov esi, services
@@:
           mov eax, [esi]
           test eax, eax
           jz .fail
           push esi
           stdcall strncmp, eax, [sz_name], 16
           pop esi
           test eax, eax
           je .ok

           add esi, 8
           jmp @B
.ok:
           mov eax, [esi+4]
           ret
.fail:
           xor eax, eax
           ret
endp

align 4
proc reg_service stdcall, sz_name:dword, handler:dword
	   locals
	     srv dd ?
	   endl

	   mov eax, [sz_name]
	   test eax, eax
	   jz .fail

	   mov ebx, [handler]
	   test ebx, ebx
	   jz .fail

	   call alloc_service
	   test eax, eax
	   jz .fail

	   mov [srv], eax
	   mov edi, eax
	   mov esi, [sz_name]
	   mov ecx, 16
	   rep movsb

	   mov edi, eax
	   mov [edi+SRV.magic], ' SRV'
	   mov [edi+SRV.size], SRV_SIZE
	   mov ebx, [handler]
	   mov [edi+SRV.srv_proc], ebx
           mov eax, [srv]
	   ret
.fail:
	   xor eax, eax
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
proc link_dll stdcall, exp:dword, imp:dword
           mov esi, [imp]
.next:
           mov eax, [esi]
           test eax, eax
           jz .end

           push esi
           stdcall get_proc, [exp], eax
           pop esi

           test eax, eax
           jz @F

           mov [esi], eax
@@:
           add esi, 4
           jmp .next
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
proc load_lib stdcall, name:dword
	   locals
	     lib   dd ?
	     base  dd ?
	     pSym  dd ?
	   endl

	   mov eax, [name]
	   mov	  ebx, 1		 ;index of first block
           mov    ecx, 32                ;number of blocks
	   mov	  edx, TMP_BUFF 	 ;temp area
	   mov	  esi, 12		 ;file name length

	   call   fileread		;read file from RD

	   cmp	  eax,0
	   jne	  .err

;           mov eax, [TMP_BUFF+CFH.pSymTable]
;           add eax, TMP_BUFF
;           mov [pSym], eax

;           mov [TMP_BUFF+20+CFS.VirtualAddress], eax

	   stdcall kernel_alloc, [TMP_BUFF+20+CFS.SizeOfRawData]
	   mov [base], eax

           test eax, eax
           jnz @f
@@:
	   mov [TMP_BUFF+20+CFS.VirtualAddress], eax
	   mov ebx, [TMP_BUFF+CFH.pSymTable]
	   add ebx, TMP_BUFF
	   mov [pSym], ebx

	   stdcall LinkSection, TMP_BUFF, TMP_BUFF+20, ebx

	   mov edi, [base]
           test edi, edi
           jnz @f
@@:
	   mov esi, [TMP_BUFF+20+CFS.PtrRawData]
	   add esi, TMP_BUFF
	   mov ecx, [TMP_BUFF+20+CFS.SizeOfRawData]
	   rep movsb

	   call alloc_dll
           test eax, eax
           jnz @f
@@:
	   mov [lib], eax

	   mov edi, eax
	   mov esi, [name]
	   mov ecx, 16
	   rep movsb

	   stdcall get_coff_sym,[pSym],[TMP_BUFF+CFH.nSymbols],szSTART
	   mov edi, [lib]
	   add eax, [base]
	   mov [edi+LIB.lib_start], eax
	   mov ebx, [base]
	   mov [edi+LIB.lib_base], ebx

	   stdcall get_coff_sym,[pSym],[TMP_BUFF+CFH.nSymbols], szEXPORTS
	   mov edi, [lib]
	   add eax, [base]
	   mov [edi+LIB.export], eax

	   stdcall get_coff_sym,[pSym],[TMP_BUFF+CFH.nSymbols], szIMPORTS
	   mov edi, [lib]
	   add eax, [base]
	   mov [edi+LIB.import], eax

	   stdcall link_dll, kernel_export, eax

	   mov edi, [lib]
           call [edi+LIB.lib_start]

	   mov eax, [lib]
	   ret
.err:
	   xor eax, eax
	   ret

endp

align 4
proc LinkSection stdcall, pCoff:dword, pSec:dword, pSym:dword
	   locals
	     pCode     dd ?
	   endl

	   mov esi, [pSec]
	   mov eax, [esi+CFS.PtrRawData]
	   add eax, [pCoff]
	   mov [pCode], eax

	   mov edi, [esi+CFS.PtrReloc]
	   add edi, [pCoff]

	   movzx edx, [esi+CFS.NumReloc]
	   mov eax, edx
	   lea edx, [edx+edx*8]
	   add edx, eax
	   add edx, edi
.l_0:
	   cmp edi, edx
	   jae .exit

	   mov ebx, [edi+CRELOC.SymIndex]
	   add ebx,ebx
	   lea ebx,[ebx+ebx*8]

	   add ebx, [pSym]

	   mov ecx, [ebx+CSYM.Value]
	   add ecx, [esi+CFS.VirtualAddress]

	   mov eax, [edi+CRELOC.VirtualAddress]
	   add eax, [pCode]
	   add [eax], ecx
	   add edi, 10
	   jmp .l_0

.exit:
	   ret
endp

proc get_curr_task
           mov eax,[CURRENT_TASK]
           shl eax, 8
           ret
endp

drv_sound      db 'UNISOUNDOBJ', 0
drv_infinity   db 'INFINITYOBJ', 0

szSound        db 'SOUND',0
szInfinity     db 'INFINITY',0

szSTART        db 'START',0
szEXPORTS      db 'EXPORTS',0
szIMPORTS      db 'IMPORTS',0

align 16
services:
           dd szSound,    drv_sound
           dd szInfinity, drv_infinity
           dd 0
