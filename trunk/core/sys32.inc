;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                               ;;
;;  MenuetOS process management, protected ring3                 ;;
;;                                                               ;;
;;  Distributed under GPL. See file COPYING for details.         ;;
;;  Copyright 2003 Ville Turjanmaa                               ;;
;;                                                               ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 32

; GDT TABLE

gdts:

        dw     gdte-$-1
        dd     gdts
        dw     0

int_code_l:
os_code_l:

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10011010b
        db     0x00

int_data_l:
os_data_l:

        dw     0xffff
        dw     0x0000
        db     0x00
        dw     11011111b *256 +10010010b
        db     0x00

graph_data_l:

        dw     0x3ff
        dw     0x0000
        db     0x00
        dw     11010000b *256 +11110010b
        db     0x00

tss0_l:
      times (max_processes+10) dd 0,0

tss0i_l:
      times 0x41 dq 0 ;(256+10) dd 0,0

app_code_l:
      times (max_processes+10) dd 0,0

app_data_l:
      times (max_processes+10) dd 0,0

tss0sys_l:
      times (max_processes+10) dd 0,0

gdte:



; <IP 05.02.2005>
idtreg:
     dw   8*0x41-1
     dd   idts+8

label idts at 0xB100-8
;idte = idts + 8 + 0x60

; </IP> ; old code below:


;align 32

;idts:
;      dw  idte-$-1
;      dd  idts+8
;      dw  0

;      times 0x62 dd 0,0

;idte:

build_process_gdt_tss_pointer:

        mov    ecx,tss_data
        mov    edi,0
      setgdtl2:
        mov    [edi+gdts+ tss0 +0], word tss_step
        mov    [edi+gdts+ tss0 +2], cx
        mov    eax,ecx
        shr    eax,16
        mov    [edi+gdts+ tss0 +4], al
        mov    [edi+gdts+ tss0 +7], ah
        mov    [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b
        add    ecx,tss_step
        add    edi,8
        cmp    edi,8*(max_processes+5)
        jbe    setgdtl2

        ret

;build_process_gdt_gate_pointer:

;        mov    edi,0
;        mov    dx,tss0
;      setidtl1:
;        mov    ecx,[esi]
;        mov    [edi+gdts+ tss0t +0], word 0
;        mov    [edi+gdts+ tss0t +2], dx
;        mov    [edi+gdts+ tss0t +4], word 11100101b*256
;        mov    [edi+gdts+ tss0t +6], word 0
;        add    dx,8
;        add    edi,8
;        cmp    edi,8*(max_processes+5)
;        jb     setidtl1

;        ret

build_interrupt_table:

        mov  [l.eflags],dword 0x11002
        mov  [l.ss0], int_data
        ;mov  [l.ss1], ring1_data
        ;mov  [l.ss2], ring2_data
        mov  [l.esp0], 0x52000
        mov  [l.esp1], 0x53000
        mov  [l.esp2], 0x54000

        mov  eax,cr3
        mov  [l.cr3],eax
        mov  [l.cs],int_code
        mov  [l.ss],int_data
        mov  [l.ds],int_data
        mov  [l.es],int_data
        mov  [l.fs],int_data
        mov  [l.gs],int_data

        mov  eax,sys_int
        mov  [l.esp],0x720000
        mov  edi,0x290000

     newint:
        push edi
        mov  ebx,[eax]
        mov  [l.eip],ebx
        mov  esi,tss_sceleton
        mov  ecx,120/4
        cld
        rep  movsd
        pop  edi

        add  edi,128
        add  [l.esp],1024
        add  eax,4

        cmp  eax,sys_int+4*0x40 ;0x60
        jbe  newint  ;jb

;;

        mov    esi,boot_sched_3_2
        call   boot_log

        mov    ecx,0x290000
        mov    edi,0
      setgdtl2i:
        mov    [edi+gdts+ tss0i +0], word 128
        mov    [edi+gdts+ tss0i +2], cx
        mov    eax,ecx
        shr    eax,16
        mov    [edi+gdts+ tss0i +4], al
        mov    [edi+gdts+ tss0i +7], ah
        mov    [edi+gdts+ tss0i +5], word 01010000b *256 +11101001b
        add    ecx,128
        add    edi,8
        cmp    edi,8*0x40 ;0x60
        jbe    setgdtl2i

;;

        mov    esi,boot_sched_3_3
        call   boot_log

        mov    edi,0
        mov    edx,tss0i
      setidtl2:
        mov    [edi+idts+ 8 +0], word 0
        mov    [edi+idts+ 8 +2], dx
        mov    [edi+idts+ 8 +4], word 10000101b*256 ; task gate DPL=0
;        cmp    edi,0x40*8
;        jne    no_sw_int
;        mov    [edi+idts+ 8 +4], word 11100101b*256 ; task gate DPL=3
;      no_sw_int:
        mov    [edi+idts+ 8 +6], word 0
        add    edx,8
        add    edi,8

        cmp    edi,8*0x40 ;0x60
        jbe    setidtl2 ;jb

; <Ivan Poddubny 06.02.2005>
; THIS CODE WON'T WORK ;-(
; because each process's 0-level stack points to the same area
; and if task switch occurs and another process is being interrupted
; a stack overflow happens
; The only way to solve that problem is to disable interrupts
; while 0x40-handler is working
; Then we have to make all entries in the IDT INTERRUPT gates, not TASK
;        mov    edi, idts+8
;        mov    esi, sys_int
;        mov    ecx, 32
;     @@:
;        mov    eax, [esi]
;        mov    [edi],   ax           ; lower part of offset
;        mov    [edi+2], word os_code ; segment selector
;        shr    eax, 16
;        mov    [edi+4], word 10001110b shl 8 ; interrupt descriptor
;        mov    [edi+6], ax
;        add    esi, 4
;        add    edi, 8
;        dec    ecx
;        jnz    @b
; </Ivan Poddubny>

        ret

build_syscall_interrupt_table:

        mov  [l.eflags],dword 0x11002
        mov  [l.ss0], int_data   ;code
        ;mov  [l.ss1], ring1_data ;code
        ;mov  [l.ss2], ring2_data ;code
        mov  [l.esp0], 0x52000
        mov  [l.esp1], 0x53000
        mov  [l.esp2], 0x54000

        mov  eax,cr3
        mov  [l.cr3],eax
        mov  [l.cs],int_code
        mov  [l.ss],int_data
        mov  [l.ds],int_data
        mov  [l.es],int_data
        mov  [l.fs],int_data
        mov  [l.gs],int_data

        mov  [l.esp],sysint_stack_data
        mov  edi,0x298000

     newint2:
        push edi
        mov  ebx,i40
        mov  [l.eip],ebx
        mov  esi,tss_sceleton
        mov  ecx,120/4
        cld
        rep  movsd
        pop  edi

        add  [l.esp],4096
        add  edi,128
        ;add  eax,4

        cmp  edi,0x298000+128*(max_processes+5)
        jb   newint2

;;

        mov    ecx,0x298000
        mov    edi,0
      setgdtl2i2:
        mov    [edi+gdts+ tss0sys +0], word 128
        mov    [edi+gdts+ tss0sys +2], cx
        mov    eax,ecx
        shr    eax,16
        mov    [edi+gdts+ tss0sys +4], al
        mov    [edi+gdts+ tss0sys +7], ah
        mov    [edi+gdts+ tss0sys +5], word 01010000b *256 +11101001b
        add    ecx,128
        add    edi,8
        cmp    edi,8*(max_processes+5)
        jbe    setgdtl2i2

;;

        ;mov    dx,tss0sys
        mov    edi,8*0x40+idts+8
        mov    [edi + 0], word 0
        mov    [edi + 2], word tss0sys ;dx
        mov    [edi + 4], word 11100101b*256
        mov    [edi + 6], word 0

        mov    edi,8*0x38+idts+8
        mov    eax, i38
        mov    [edi],   ax           ; lower part of offset
        mov    [edi+2], word os_code ; segment selector
        shr    eax, 16
        mov    [edi+4], word 11101110b shl 8 ; 32-bit interrupt gate, DPL 3
        mov    [edi+6], ax

        ret


align 4
i38:
        ; load data selectors
        pushfd
        push    ds es
        push    eax
        mov     ax, os_data
        mov     ds, ax
        mov     es, ax
        pop     eax

        pushad
        push    edi
        mov     edi, eax
        mov     eax, ebx
        mov     ebx, ecx
        mov     ecx, edx
        mov     esi, [esp]
        and     edi, 0xFF
        call    dword [servetable+edi*4]
        add     esp, 4
        popad
        pop     es ds
        popfd
iret


iglobal
  sys_int:
    dd   s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,sa,sb,sc,sd,se,sf

    dd   s10        ,s11      ;  ,i_unknown12,i_unknown13
;    dd   i_unknown14,i_unknown15,i_unknown16,i_unknown17
;    dd   i_unknown18,i_unknown19,i_unknown1a,i_unknown1b
;    dd   i_unknown1c,i_unknown1d,i_unknown1e,i_unknown1f
    times 14 dd unknown_interrupt

    dd   irq0  ,irq1  ,p_irq2 ,p_irq3 ,p_irq4 ,p_irq5,p_irq6 ,p_irq7
    dd   p_irq8,p_irq9,p_irq10,p_irq11,p_irq12,irqD  ,p_irq14,p_irq15

    ;dd   i_unknown30,i_unknown31,i_unknown32,i_unknown33
    ;dd   i_unknown34,i_unknown35,i_unknown36,i_unknown37
    ;dd   i_unknown38,i_unknown39,i_unknown3a,i_unknown3b
    ;dd   i_unknown3c,i_unknown3d,i_unknown3e,i_unknown3f
    times 16 dd unknown_interrupt

    dd   i40
endg

uglobal
 tss_sceleton:
  l.back   dw 0,0
  l.esp0   dd 0
  l.ss0    dw 0,0
  l.esp1   dd 0
  l.ss1    dw 0,0
  l.esp2   dd 0
  l.ss2    dw 0,0
  l.cr3    dd 0
  l.eip    dd 0
  l.eflags dd 0
  l.eax    dd 0
  l.ecx    dd 0
  l.edx    dd 0
  l.ebx    dd 0
  l.esp    dd 0
  l.ebp    dd 0
  l.esi    dd 0
  l.edi    dd 0
  l.es     dw 0,0
  l.cs     dw 0,0
  l.ss     dw 0,0
  l.ds     dw 0,0
  l.fs     dw 0,0
  l.gs     dw 0,0
  l.ldt    dw 0,0
  l.trap   dw 0
  l.io     dw 0
endg

s0:
        cli

        mov   [error_interrupt],0x0
        mov   [error_interrupt_entry],dword s0

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task


s1:
        cli

        mov   [error_interrupt],0x1
        mov   [error_interrupt_entry],dword s1

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s2:
        cli

        mov   [error_interrupt],0x2
        mov   [error_interrupt_entry],dword s2

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s3:
        cli

        mov   [error_interrupt],0x3
        mov   [error_interrupt_entry],dword s3

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s4:
        cli

        mov   [error_interrupt],0x4
        mov   [error_interrupt_entry],dword s4

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s5:
        cli

        mov   [error_interrupt],0x5
        mov   [error_interrupt_entry],dword s5

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s6:
        cli

        mov   [error_interrupt],0x6
        mov   [error_interrupt_entry],dword s6

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task


;;;;;;;;;;;;;;;;;;;;;;;
;; FPU ERROR HANDLER ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
s7:
        ; <IP 05.02.2004>
        cli
        ; </IP>
        mov    edi, 7*8
        mov    [edi+gdts+ tss0i +5], word 01010000b *256 +11101001b

        mov    edi,[0x3000]
        shl    edi, 3
        mov    [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b


        ; save a copy of current task's TSS to fpu_tss
        mov    esi,[0x3000]
        imul   esi,tss_step
        add    esi,tss_data
        mov    edi,fpu_tss
        mov    ecx,120/4
        cld
        rep    movsd

        ; get base address of our TSS and...
        mov    esi,[0x3000]
        imul   esi,tss_step
        add    esi,tss_data

        ; ...init segments, stack, eip, flags
        mov    word [esi+l.cs-tss_sceleton],int_code
        mov    word [esi+l.ss-tss_sceleton],int_data
        mov    word [esi+l.ds-tss_sceleton],int_data
        mov    word [esi+l.es-tss_sceleton],int_data
        mov    word [esi+l.fs-tss_sceleton],int_data
        mov    word [esi+l.gs-tss_sceleton],int_data
        mov    dword [esi+l.esp-tss_sceleton],fpu_stack+4*8
        mov    dword [esi+l.eip-tss_sceleton],fpu_handler
        mov    dword [esi+l.eflags-tss_sceleton],0x11002

        ; then execute this task
        mov    ebx, [0x3000]
        shl    ebx,3
        add    ebx, tss0 ;t
        mov    [0xB004], bx

        jmp    pword [0xB000]

        jmp    s7

s8:
        cli

        mov   [error_interrupt],0x8
        mov   [error_interrupt_entry],dword s8

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s9:
        cli

        mov   [error_interrupt],0x9
        mov   [error_interrupt_entry],dword s9

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

sa:
        cli

        mov   [error_interrupt],0xa
        mov   [error_interrupt_entry],dword sa

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

sb:
        cli

        mov   [error_interrupt],0xb
        mov   [error_interrupt_entry],dword sb

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

sc:
        cli

        mov   [error_interrupt],0xc
        mov   [error_interrupt_entry],dword sc

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

sd:
        cli

        mov   [error_interrupt],0xd
        mov   [error_interrupt_entry],dword sd

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

se:
        cli

        mov   [error_interrupt],0xe
        mov   [error_interrupt_entry],dword se

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

sf:
        cli

        mov   [error_interrupt],0xf
        mov   [error_interrupt_entry],dword sf

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s10:
        cli

        mov   [error_interrupt],0x10
        mov   [error_interrupt_entry],dword s10

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

s11:
        cli

        mov   [error_interrupt],0x11
        mov   [error_interrupt_entry],dword s11

        call  show_error_parameters

        mov   edx,[0x3010]
        mov   [edx+0xa],byte 4

        jmp   change_task

writehex:

      pusha

      mov  ecx,eax
      mov  ax,word [printerrorat]
      shl  eax,16
      mov  ax,[esp+32+4]
      sub  ax,60
      mov  edx,1
      mov  esi,8
      mov  ebx,0xffffff
    whl1:
      push ecx
      and  ecx,0xf
      add  ecx,hexletters
      mov  edi,1

      mov  cl,[ecx]
      mov  edi,[write_error_to]
      mov  [edi],cl
      dec  [write_error_to]

      pop  ecx
      shr  ecx,4
      sub  eax,6*65536
      dec  esi
      jnz  whl1

      popa
      ret

iglobal
  hexletters  db '0123456789ABCDEF'

  error_interrupt         dd  -1
  error_interrupt_entry   dd  -1

  printerrorat            dd 300

  process_error  db 'K : Process - forced terminate INT: 00000000',13,10,0
  process_pid    db 'K : Process - forced terminate PID: 00000000',13,10,0
  process_eip    db 'K : Process - forced terminate EIP: 00000000',13,10,0
  system_error   db 'K : Kernel error',13,10,0
endg

uglobal
  write_error_to  dd  0x0
endg

show_error_parameters:
        
        mov    [write_error_to],process_pid+43
        mov    eax,[0x3000]
        shl    eax, 5
        mov    eax,[0x3000+4+eax]
        call   writehex
        
        mov    [write_error_to],process_error+43
        mov    eax,[error_interrupt]
        call   writehex
        
        mov    eax,[0x3000]
        shl    eax,8
        cmp    [0x80000+eax+0xB0],dword 0
        jnz    .system_error

        mov    eax,[0x3000]
        imul   eax,tss_step
        mov    eax,[eax+tss_data+l.eip-tss_sceleton]
.out_eip:        
        mov    [write_error_to],process_eip+43
        call   writehex

        mov    esi,process_error
        call   sys_msg_board_str

        mov    esi,process_pid
        call   sys_msg_board_str

        mov    esi,process_eip
        call   sys_msg_board_str

        ret
        
.system_error:
        mov    esi,system_error
        call   sys_msg_board_str
        mov    eax,[0x3000]
        shl    eax,7
        mov    eax,[eax+0x298000+l.eip-tss_sceleton]
        jmp    .out_eip        


irq5:

     call   restore_caller

     mov    dx,word [sb16]
     add    dx,0xe
     in     al,dx

;     mov    byte [SB16_Status],0

     mov    [check_idle_semaphore],5

     mov    al,0x20
     out    0x20,al

     call  return_to_caller

     jmp   irq5

irqD:

     call   restore_caller

     mov   dx,0xf0
     mov   al,0
     out   dx,al

     mov   dx,0xa0
     mov   al,0x20
     out   dx,al
     mov   dx,0x20
     out   dx,al

     mov   ds,cx
     mov   es,cx
     mov   fs,cx

     call  return_to_caller

     jmp   irqD

p_irq2:

     call  restore_caller

     mov   edi,2               ; 1
     call  irqhandler          ; 2/5

     call  return_to_caller

     jmp   p_irq2

p_irq3:

     call  restore_caller

     mov   edi,3
     call  irqhandler

     call  return_to_caller

     jmp   p_irq3

p_irq4:

     call  restore_caller

     mov   edi,4
     call  irqhandler

     call  return_to_caller

     jmp   p_irq4

p_irq5:

     call  restore_caller

     mov   edi,5
     call  irqhandler

     call  return_to_caller

     jmp   p_irq5

p_irq6:

     call  restore_caller

     call  fdc_irq

     mov   edi,6
     call  irqhandler

     call  return_to_caller

     jmp   p_irq6

p_irq7:

     call  restore_caller

     mov   edi,7
     call  irqhandler

     call  return_to_caller

     jmp   p_irq7

p_irq8:

     call  restore_caller

     mov   edi,8
     call  irqhandler

     call  return_to_caller

     jmp   p_irq8

p_irq9:

     call  restore_caller

     mov   edi,9
     call  irqhandler

     call  return_to_caller

     jmp   p_irq9

p_irq10:

     call  restore_caller

     mov   edi,10
     call  irqhandler

     call  return_to_caller

     jmp   p_irq10

p_irq11:

     call  restore_caller

     mov   edi,11
     call  irqhandler

     call  return_to_caller

     jmp   p_irq11

p_irq12:

      call  restore_caller

      mov   edi,12
      call  irqhandler

      call  return_to_caller

      jmp   p_irq12

p_irq13:

     call  restore_caller

     mov   edi,13
     call  irqhandler

     call  return_to_caller

     jmp   p_irq13

p_irq14:

     call  restore_caller
     
     mov   edi,14
     call  irqhandler

     call  return_to_caller

     jmp   p_irq14

p_irq15:

     call  restore_caller

     mov   edi,15
     call  irqhandler

     call  return_to_caller

     jmp   p_irq15



align 4
restore_caller:

      mov    edi,[0x3000]
      shl    edi, 3
      mov    [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b

      ret

align 4
return_to_caller:

      mov   ebx,[0x3000]
      shl   bx,3
      add   bx,tss0;t
      mov   [tss_irq12],bx

           db    0xea
           dd    0
tss_irq12  dw    tss0;t

           ret

uglobal
  irqh dd 0x0
endg

irqhandler:

     push   edi

     mov    esi,edi          ; 1
     shl    esi,6            ; 1
     add    esi,irq00read    ; 1
     shl    edi,12           ; 1
     add    edi,0x2E0000

     mov    [check_idle_semaphore],5

   irqnewread:

     mov    dx,[esi]         ; 2+

     cmp    dx,0             ; 1
     jz     irqover
     cmp    [esi+3],byte 1   ; 2     ; byte read
     jne    noirqbyte        ; 4-11

     in     al,dx

     mov    edx,[edi]
     cmp    edx,4000
     je     irqfull
     mov    ebx,edi
     add    ebx,0x10
     add    ebx,edx
     mov    [ebx],al
     inc    edx
     mov    [edi],edx

     add    esi,4
     jmp    irqnewread

   noirqbyte:


     cmp    [esi+3],byte 2     ; word read
     jne    noirqword

     in     ax,dx

     mov    edx,[edi]
     cmp    edx,4000
     je     irqfull
     mov    ebx,edi
     add    ebx,0x10
     add    ebx,edx
     mov    [ebx],ax
     add    edx,2
     mov    [edi],edx
     add    esi,4
     jmp    irqnewread

   noirqword:
   irqfull:
   irqover:

     mov    al,0x20            ; ready for next irq
     out    0x20,al

     pop    ebx
     cmp    ebx,7
     jbe    noa0
     out    0xa0,al
   noa0:

     ret


; this code should never get control!
; applications can use only 0x40 interrupt
unknown_interrupt:
 @@: call   change_task
     jmp    @b



compare_to_thread:

         push ebx

         mov  eax,edx
         shl  eax, 3
         add  eax,gdts+ app_code-3
         mov  ebx,[eax]
         cmp  ebx,[old_code_0]
         jne  ctt0
         mov  ebx,[eax+4]
         cmp  ebx,[old_code_1]
         jne  ctt0

         pop  ebx
         mov  eax,1
         ret

       ctt0:

         pop  ebx
         mov  eax,0
         ret



check_for_thread_mem:

         pusha

         mov  ecx,[0x3004]
       cftm0:
         mov  eax,ecx
         shl  eax, 8
         add  eax,gdts+ app_code-3
         mov  ebx,[eax]
         cmp  ebx,[old_code_0]
         jne  cftm1
         mov  ebx,[eax+4]
         cmp  ebx,[old_code_1]
         jne  cftm1

         mov  eax,ecx  ; new code segments
         shl  eax, 3 ;imul eax,8
         add  eax,gdts+ app_code-3

         mov  ebx,[new_code_0]
         mov  [eax],ebx
         mov  ebx,[new_code_1]
         mov  [eax+4],ebx

         mov  eax,ecx  ; new data segments
         shl  eax, 3

         add  eax,gdts+ app_data-3

         mov  ebx,[new_data_0]
         mov  [eax],ebx
         mov  ebx,[new_data_1]
         mov  [eax+4],ebx

         cmp  [new_pos],0   ; new memory position segments
         je   no_new_postition_for_thread
         mov  eax,ecx
         shl  eax, 5
         add  eax,0x3000
         mov  ebx,[new_pos]
         mov  [eax+0x10],ebx
       no_new_postition_for_thread:

         mov  eax,ecx       ; new amount of memory
         shl  eax, 8
         add  eax,0x80000
         mov  ebx,[new_amount]
         mov  [eax+0x8C],ebx

       cftm1:

         dec  ecx
         jnz  cftm0

         popa

         ret


save_for_thread_check:

        ; save for thread check

        pusha
        mov    esi,[0x3000]
        ;imul   esi,8
        shl    esi, 3
        add    esi,gdts+ app_code-3 +0
        mov    edi,old_code_0
        mov    ecx,8
        cld
        rep    movsb
        popa

        ret


save_new_position_for_threads:

        ; new code segment for thread check
        pusha
        mov    esi,[0x3000]
        ;imul   esi,8
        shl    esi, 3
        add    esi,gdts+ app_code-3 +0
        mov    edi,new_code_0
        mov    ecx,8
        cld
        rep    movsb
        popa

        ; new data segment for thread check
        pusha
        mov    esi,[0x3000]
        ;imul   esi,8
        shl    esi, 3
        add    esi,gdts+ app_data-3 +0
        mov    edi,new_data_0
        mov    ecx,8
        cld
        rep    movsb
        popa

        ret

set_application_table_status:
        push eax

        mov  eax,[0x3000]
        ;imul eax,32
        shl  eax, 5
        add  eax,0x3000+4
        mov  eax,[eax]

        mov  [application_table_status],eax

        pop  eax

        ret


clear_application_table_status:
        push eax

        mov  eax,[0x3000]
        ;imul eax,32
        shl  eax, 5
        add  eax,0x3000+4
        mov  eax,[eax]

        cmp  eax,[application_table_status]
        jne  apptsl1
        mov  [application_table_status],0
      apptsl1:

        pop  eax

        ret

uglobal
  old_code_0 dd 0x0
  old_code_1 dd 0x0

  ;

  new_code_0 dd 0x0
  new_code_1 dd 0x0

  new_data_0 dd 0x0
  new_data_1 dd 0x0

  new_pos    dd 0x0
  new_amount dd 0x0
endg


sys_resize_app_memory:
        ; eax = 1 - resize
        ;     ebx = new amount of memory

        cmp    eax,1
        jne    no_application_mem_resize
        
        mov    eax,[0x3010]
        cmp    dword [eax+0x10],std_application_base_address
        jz     new_mem_resize ;resize for new type of processes

        add    ebx,4095
        shr    ebx,12
        shl    ebx,12
        mov    ebp,ebx

        ; wait for process table to be free

      rsm0:

        cli
        cmp   [application_table_status],0
        je    rsm1
        sti
        call  change_task
        jmp   rsm0

      rsm1:

        call  set_application_table_status
        sti

        cmp    ebx,0 ; other than zero
        je     mem_resize_unsuccess

        call   save_for_thread_check

        ; find a free place

        mov    esi,[0xfe84]  ; application memory start
        mov    edi,ebp
        add    edi,esi
        dec    edi

      rfgdt:

        mov    edx,2

      rfindgdtl1:

        call   compare_to_thread
        cmp    eax,1
        je     rfindfl3

        mov    ecx,edx
        shl    ecx,3

        ; eax run base -> ebx limit

        mov    al,[ecx+gdts+ app_code-3 +4]
        mov    ah,[ecx+gdts+ app_code-3 +7]
        shl    eax,16
        mov    ax,[ecx+gdts+ app_code-3 +2]
        
;!!mem
        cmp    eax,std_application_base_address
        jz     rfindfl3
;!!mem

        movzx  ebx,word [ecx+gdts+ app_code-3 +0]
        shl    ebx,12
        add    ebx,eax

        cmp    eax,edi
        jg     rfindfl3
        cmp    ebx,esi
        jb     rfindfl3

        add    esi,4096
        add    edi,4096

        cmp    edi,[0xfe8c]        ; < c_memory
        jbe    rfgdt

        jmp    rfind_free_ret_2   ;; not enough memory

      rfindfl3:

        inc    edx
        cmp    edx,[0x3004]
        jbe    rfindgdtl1

     rfindfl1:
     rthread_c:

        mov    ecx,[0x3000]
        shl    ecx,3

        inc    edi
        sub    edi,esi
        add    edi,4095
        shr    edi,12
        dec    edi

        ; code

        mov    eax,esi
        mov    ebx,edi

        mov    [ecx+gdts+ app_code-3 +2], ax              ;  base  0:15
        shr    eax,16
        mov    [ecx+gdts+ app_code-3 +4], al              ;  base  23:16
        mov    [ecx+gdts+ app_code-3 +7], ah              ;  base  31:24
        mov    [ecx+gdts+ app_code-3 +0], bx              ;  limit

        ; data

        mov    eax,esi
        mov    [ecx+gdts+ app_data-3 +2], ax              ;  base  0:15
        shr    eax,16
        mov    [ecx+gdts+ app_data-3 +4], al              ;  base  23:16
        mov    [ecx+gdts+ app_data-3 +7], ah              ;  base  31:24

        movzx  edx,word [ecx+gdts+ app_code-3 +0]         ;  save limit

        mov    [ecx+gdts+ app_data-3 +0], bx              ;  limit

        and    ebx,0xffff

        cmp    ebx,edx  ; copy smaller from memory sizes
        jge    noedxebxxchg
        mov    edx,ebx
      noedxebxxchg:

        movzx  ecx,dx
        shl    ecx,12
        add    ecx,4096

        mov    edi,esi

        mov    eax,[0x3010]
        mov    esi,[eax+0x10]

        mov    [eax+0x10],edi  ; new memory position

        mov    eax,[0x3000]    ; new memory size
        shl    eax,8
        add    eax,0x80000
        mov    [eax+0x8c],ebp

        mov    [new_pos],edi    ; new position for threads
        mov    [new_amount],ebp ; new amount of mem for threads

        cmp    esi,edi
        je     no_app_move

        cld
        rep    movsb           ; move the app image to the new position

      no_app_move:

        call   save_new_position_for_threads
        call   check_for_thread_mem

        mov    [application_table_status],0

        mov    [esp+36],dword 0 ; eax <- 0 ; successfull

        ret

     rfind_free_ret_2:

     mem_resize_unsuccess:

        mov    [application_table_status],0

        mov    [esp+36],dword 1 ; eax <- 1 ; unsuccessfull

        ret

     no_application_mem_resize:


        ret


align 4
find_free_mem:

        push   eax
        push   ebx
        push   ecx
        push   edx
        push   edi

        call   find_free_process_slot
        mov    eax,[new_process_place]

        cmp    eax,max_processes
        jg     find_free_ret_2

        cmp    [thread_create],1
        je     thread_c

        mov    esi,[0xfe84]
        add    edi,esi
        dec    edi

        mov    eax,2
        cmp    dword [0x3004],1
        je     findf4

     fgdt:

        mov    edx,2

      findgdtl1:

        mov    ecx,edx
        shl    ecx,3

        ; eax run base -> ebx limit

        mov    al,[ecx+gdts+ app_code-3 +4]
        mov    ah,[ecx+gdts+ app_code-3 +7]
        shl    eax,16
        mov    ax,[ecx+gdts+ app_code-3 +2]
;!!mem
        cmp    eax,std_application_base_address
        jz     findfl3
;!!mem

        movzx  ebx,word [ecx+gdts+ app_code-3 +0]
        shl    ebx,12
        add    ebx,eax

        cmp    eax,edi
        jg     findfl3
        cmp    ebx,esi
        jb     findfl3

        add    esi,4096
        add    edi,4096

        cmp    edi,[0xfe8c]        ; < c_memory
        jbe    fgdt

        jmp    find_free_ret_2

      findfl3:

        inc    edx
        cmp    edx,[check_processes]
        jbe    findgdtl1

     findfl1:
     thread_c:

         mov    eax,[new_process_place]

     findf4:

        mov    [first_gdt_search],eax
        mov    [gdt_place],eax

        mov    ecx,eax
        shl    ecx,3

        inc    edi
        sub    edi,esi
        add    edi,4095
        shr    edi,12
        dec    edi

        ; code

        mov    eax,esi
        mov    ebx,edi

        mov    [ecx+gdts+ app_code-3 +2], ax                   ;  base  0:15
        shr    eax,16
        mov    [ecx+gdts+ app_code-3 +4], al                   ;  base  23:16
        mov    [ecx+gdts+ app_code-3 +7], ah                   ;  base  31:24
        mov    [ecx+gdts+ app_code-3 +0], bx                   ;  limit
        mov    [ecx+gdts+ app_code-3 +5], word 11010000b *256 +11111010b

        ; data

        mov    eax,esi
        mov    [ecx+gdts+ app_data-3 +2], ax                   ;  base  0:15
        shr    eax,16
        mov    [ecx+gdts+ app_data-3 +4], al                   ;  base  23:16
        mov    [ecx+gdts+ app_data-3 +7], ah                   ;  base  31:24
        mov    [ecx+gdts+ app_data-3 +0], bx                   ;  limit
        mov    [ecx+gdts+ app_data-3 +5], word 11010000b *256 +11110010b

        push   esi
        mov    esi,process_loading
        call   sys_msg_board_str
        pop    esi

      find_free_ret:

        pop    edi
        pop    edx
        pop    ecx
        pop    ebx
        pop    eax
        ret

      find_free_ret_2:

        cmp    [dec3004],0
        je     no3004inc
        dec    dword [0x3004]
      no3004inc:

        pop    edi
        pop    edx
        pop    ecx
        pop    ebx
        pop    eax
        mov    esi,0
        ret


get_app_params:

    push eax

    cmp  [0x90000+6],word '00'
    jne  no_00_header

    mov  eax,[0x90000+12]
    mov  [app_start],eax
    mov  eax,[0x90000+16]
    mov  [app_i_end],eax
    mov  eax,[0x90000+20]
    mov  [app_mem],eax
    shr  eax,1
    sub  eax,0x10
    mov  [app_esp],eax
    mov  eax,[0x90000+24]
    mov  [app_i_param],eax
    mov  [app_i_icon],dword 0

    pop  eax
    mov  esi,1
    ret

  no_00_header:


    cmp  [0x90000+6],word '01'
    jne  no_01_header

    mov  eax,[0x90000+12]
    mov  [app_start],eax
    mov  eax,[0x90000+16]
    mov  [app_i_end],eax
    mov  eax,[0x90000+20]
    mov  [app_mem],eax
    mov  eax,[0x90000+24]
    mov  [app_esp],eax
    mov  eax,[0x90000+28]
    mov  [app_i_param],eax
    mov  eax,[0x90000+32]
    mov  [app_i_icon],eax

    pop  eax
    mov  esi,1
    ret

   no_01_header:

    pop  eax
    mov  esi,0
    ret


start_application_fl:
    jmp new_start_application_fl

;************************************************************************

start_application_floppy:
    jmp  new_start_application_floppy

;********************************************************************

start_application_hd:
    jmp   new_start_application_hd

uglobal
  threadstring       dd  0x0
  new_process_place  dd  0x0
  check_processes    dd  0x0
  dec3004            db  0x0
  app_start    dd  0x0
  app_i_end    dd  0x0
  app_mem      dd  0x0
  app_esp      dd  0x0
  app_i_param  dd  0x0
  app_i_icon   dd  0x0
  app_mem_pos  dd  0x0
  thread_create    dd 0x0
  gdt_place        dd 0x0
endg

iglobal
  hd_app_string      db  'HDAPP       '
  process_loading    db 'K : Process - loading ',13,10,0
  process_running    db 'K : Process - done',13,10,0
  first_gdt_search   dd 0x2
endg


sys_threads:

; eax=1 create thread
;
;   ebx=thread start
;   ecx=thread stack value
;
; on return : eax = pid
jmp new_sys_threads
    cli
    cmp   [application_table_status],0
    je    stth9
    sti
    call  change_task
    jmp   sys_threads
  stth9:

    call  set_application_table_status

    sti

    cmp  eax,1
    jne  no_sys_thread_create
    cli

    mov  eax,[0x3010]
    mov  eax,[eax+0x10]
    mov  [app_mem_pos],eax

    mov  [app_i_param],0
    mov  [app_i_icon],0

    mov  [app_start],ebx
    mov  [app_esp],ecx

    mov  ebx,[0x3000]
    shl  ebx,8
    add  ebx,0x80000
    mov  [threadstring],ebx
    mov  ebx,[ebx+0x8c]
    mov  [app_mem],ebx

    mov  esi,[app_mem_pos]
    mov  edi,[app_mem]
    add  edi,esi
    dec  edi
    mov  [thread_create],1
    call find_free_mem
    cmp  esi,0
    jne  th_cr1
    mov  [application_table_status],0
    mov  eax,1   ; no free memory
    sti
    ret
  th_cr1:
    push dword 0
    push dword [threadstring]
    jmp  add_app_parameters
  no_sys_thread_create:

    mov  eax,-1
    mov  [application_table_status],0
    ret


find_free_process_slot:

    pusha

    mov   ebx,[0x3004]
    mov   [check_processes],ebx
    inc   ebx
    mov   [new_process_place],ebx

    mov   ebx,2

  newfps:

    mov   eax,ebx
    ;imul  eax,0x20
    shl   eax, 5
    add   eax,0x3000+0xa
    cmp   [eax],byte 9
    je    ffpl

    inc   ebx
    cmp   ebx,[0x3004]
    jbe   newfps

    ;mov   [dec3004],0
    mov   [dec3004],1
    shl   ebx,5
    mov   [0x3000+0xa+ebx],byte 9
    inc   dword [0x3004]

    popa
    ret

  ffpl:

    ;mov   [dec3004],1
    ;dec   dword [0x3004]
    mov   [dec3004],0
    mov   [new_process_place],ebx

    popa
    ret


add_app_parameters:
; returns:  eax = pid or -1 if unsuccesfull
    cmp   [app_i_param],dword 0     ; parameter
    jz    no_app_params
    xor   eax, eax
    mov   edi,[app_i_param]
    add   edi,[app_mem_pos]
    mov   ecx,256/4
    cld
    rep   stosd
    mov   esi,[esp+4]
    test  esi, esi
    jz    no_app_params
    mov   eax,[app_i_param]
    add   eax,[app_mem_pos]
    mov   edi,eax
    mov   ecx,256
    cld
  app_new_param:
    cmp   [esi],byte 0
    jz    no_app_params
    movsb
    loop  app_new_param
  no_app_params:

    ;inc   dword [0x3004]            ; increase number of processes
    mov   ebx,[new_process_place]

    mov   edi,ebx                   ; clear 0x80000 (256 bytes)
    shl   edi,8
    add   edi,0x80000
    mov   ecx,256 / 4
    mov   eax,0
    cld
    rep   stosd

    shl   ebx,5                     ; * 32 +0x3000
    add   ebx,0x3000

    mov   al,byte [new_process_place] ; screen id ?
    mov   [ebx+0xe],al

    mov   [ebx],dword 1+2+4         ; report events: windowdraw, key, button

    inc   dword [process_number]    ; process id number
    mov   eax,[process_number]
    mov   [ebx+4],eax

    mov   ecx,ebx                   ; set draw limits
    add   ecx,draw_data-0x3000
    mov   [ecx+0],dword 0
    mov   [ecx+4],dword 0
    mov   eax,[0xfe00]
    mov   [ecx+8],eax
    mov   eax,[0xfe04]
    mov   [ecx+12],eax

    mov   eax,[app_mem_pos]        ; position in memory
    mov   [ebx+0x10],eax

    ; TSS
    xor   ebx,ebx
    cmp   [thread_create],ebx
    jnz   clone_cr3_table
    mov   eax,[new_process_place]
    call  create_app_cr3_table
    jmp   set_cr3
clone_cr3_table:
;    mov   eax,[new_process_place]
;    mov   ebx,[0x3000]
;    call  addreference_app_cr3_table
    mov   eax,[0x3000]
    call  get_cr3_table
set_cr3:
    add   eax,8+16
    mov   [l.cr3],eax
    
    mov   eax,[app_start]
    mov   [l.eip],eax
    mov   eax,[app_esp]
    mov   [l.esp],eax

    mov   ebx,[new_process_place]              ; gdt's
    shl   ebx,3

    mov   ax,app_code
    add   ax,bx
    mov   [l.cs],ax
    mov   ax,app_data
    add   ax,bx
    mov   [l.ss],ax
    mov   [l.ds],ax
    mov   [l.es],ax
    mov   [l.fs],ax
    mov   ax,graph_data
    mov   [l.gs],ax
    mov   [l.io],word 128
    mov   [l.eflags],dword 0x11202
    mov   [l.ss0], os_data
    ;mov   [l.ss1], ring1_data
    ;mov   [l.ss2], ring2_data
    ; [Ivan 07.03.2005]
    mov   [l.esp0], 0x8000 ;0x55000 ; used by i38 handler
    ; [/Ivan 07.03.2005]
    mov   [l.esp1], 0x56000
    mov   [l.esp2], 0x57000

    mov   eax,tss_sceleton          ; move tss to tss_data+
    mov   ebx,[new_process_place]
    imul  ebx,tss_step
    add   ebx,tss_data
    mov   ecx,120
    call  memmove


    ; Add IO access table

    or   eax, -1
    mov  edi, [new_process_place]
    imul edi, tss_step
    add  edi, tss_data + 128
    mov  ecx, 2048 ; for 2048 * 4 * 8 bits = 65536 ports
    cld
    rep  stosd

    ; make sure gdt is pointing to the process
    ; and not to i40 handler

    mov   ecx,ebx
    mov   edi,[new_process_place]
;    imul  edi,8
    shl   edi, 3

    mov   [edi+gdts+ tss0 +0], word tss_step     ; limit 0:15
    mov   [edi+gdts+ tss0 +2], cx                ; base  0:15
    mov   eax,ecx
    shr   eax,16
    mov   [edi+gdts+ tss0 +4], al                ; base  23:16
    mov   [edi+gdts+ tss0 +7], ah                ; base  31:24
    mov   [edi+gdts+ tss0 +5], word 01010000b *256 +11101001b

    pop   eax
    pop   ebx

    mov   ebx,[new_process_place]              ; save name of the process
    shl   ebx,8
    add   ebx,0x80000
    mov   ecx,11
    call  memmove

    mov   ebx,[new_process_place]              ; save image size
    shl   ebx,8
    add   ebx,0x80000+0x8C
    mov   eax,[app_mem]
    mov   [ebx],eax

    mov   [0xf400],byte 0           ; empty keyboard buffer
    mov   [0xf500],byte 0           ; empty button buffer

    mov   [application_table_status],0
    mov   eax,[process_number]

    mov   ebx,[new_process_place]
    shl   ebx, 5 ;imul  ebx,0x20
    mov   [0x3000+ebx+0xa],byte 0

    mov   edi,[new_process_place]
    shl   edi,5
    add   edi,window_data

    mov   ebx,[new_process_place]
    movzx  esi, word [0xC000 + ebx*2]
    lea    esi, [0xC400 + esi*2]
    call   windowactivate

    sti

    push   esi
    mov    esi,process_running
    call   sys_msg_board_str
    pop    esi

    ret

iglobal
  process_terminating   db 'K : Process - terminating',13,10,0
  process_terminated    db 'K : Process - done',13,10,0
endg


terminate: ; terminate application
    push   esi
    mov    esi,process_terminating
    call   sys_msg_board_str
    pop    esi
;start memory manager code    
;    mov    eax,esi
;    call   MEM_Heap_Clean
;end memory manager code
    

    cli
    cmp   [application_table_status],0
    je    term9
    sti
    call  change_task
    jmp   terminate
  term9:

    call  set_application_table_status
    
    mov    eax,esi
    call   dispose_app_cr3_table

    mov   [first_gdt_search],0x2   ; start gdt search from beginning

    cmp   [prev_user_of_fpu],esi   ; if user fpu last -> fpu user = 1
    jne   fpu_ok_1
    mov   [prev_user_of_fpu],1
  fpu_ok_1:

    mov   [0xf400],byte 0           ; empty keyboard buffer
    mov   [0xf500],byte 0           ; empty button buffer

    mov   ecx,esi                               ; clear memory reserv.
    shl   ecx,3
    mov   [ecx+gdts+ app_code-3 +0],dword 0
    mov   [ecx+gdts+ app_code-3 +4],dword 0
    mov   [ecx+gdts+ app_data-3 +0],dword 0
    mov   [ecx+gdts+ app_data-3 +4],dword 0

    mov   edi, esi
;    shl   edi, 5
;    add   edi, 0x3000
;    cmp   [edi+0xa],byte 3  ; if normal terminate then clear int40 handler
;    jne   nocl40

;    mov   edi,esi   ; free the used interrupt 0x40 handler
;    shl   edi, 8 ;imul  edi,256
;    mov   eax,[edi+0x80000+0xb0]

;    cmp   eax,0     ; is application using a systemcall interrupt  ?
;    je    nocl40

    mov   [usedi40+eax],byte 0

;    mov   edi,8
;    imul  edi,eax
    mov   edi, eax
    shl   edi, 3
    mov   [edi+tss0sys_l +5], word 01010000b *256 +11101001b

;    mov   edi,128
;    imul  edi,eax
    mov   edi, eax
    shl   edi, 7
    mov   [edi+0x298000+l.eip-tss_sceleton],dword i40
    mov   [edi+0x298000+l.eflags-tss_sceleton],dword 0x11002

    mov   ebx,eax
    shl   ebx, 12 ;imul  ebx,4096
    add   ebx,sysint_stack_data
    mov   [edi+0x298000+l.esp-tss_sceleton],ebx

  nocl40:

    mov   ecx,esi                 ; remove buttons
  bnewba2:
    mov   edi,[0xfe88]
    mov   eax,edi
    cld
    movzx ebx,word [edi]
    inc   bx
  bnewba:
    dec   bx
    jz    bnmba
    add   eax,0x10
    cmp   cx,[eax]
    jnz   bnewba
    pusha
    mov   ecx,ebx
    inc   ecx
    shl   ecx,4
    mov   ebx,eax
    add   eax,0x10
    call  memmove
    dec   dword [edi]
    popa
    jmp   bnewba2
  bnmba:

    pusha     ; save window coordinates for window restoring
    cld
    shl   esi,5
    add   esi,window_data
    mov   ax,[esi+0]
    mov   word [dlx],ax
    mov   bx,[esi+8]
    add   ax,bx
    mov   word [dlxe],ax
    mov   ax,[esi+4]
    mov   word [dly],ax
    mov   bx,[esi+12]
    add   ax,bx
    mov   word [dlye],ax
    mov   [esi+0],word 0
    mov   [esi+8],word 5
    mov   ax,[0xFE04]
    mov   [esi+4],ax
    mov   [esi+12],word 5
    xor   eax, eax
    mov   [esi+16],eax;dword 0
    mov   [esi+20],eax;dword 0
    mov   [esi+24],eax;dword 0
    mov   [esi+28],eax;dword 0
    popa

    pusha
    mov   edi,esi
    shl   edi,5
    add   edi,window_data
    mov   ecx,32/4
    xor   eax, eax
  ;  cld
    rep   stosd

    mov   eax,[0xFE04]      ; set window to start from maxy+1
    add   eax,2

    mov   edi,esi
    shl   edi,5
    add   edi,window_data
    mov   [edi+4],eax

    popa

    pusha
    mov   edi,esi
    shl   edi,5
    add   edi,draw_data
    mov   ecx,32/4
    xor   eax, eax
  ;  cld
    rep   stosd
    popa

    pusha         ; at 0x80000+
    mov   edi,esi
    shl   edi,8
    add   edi,0x80000
    mov   ecx,256/4
    xor   eax, eax
  ;  cld
    rep   stosd
    popa

    pusha          ; name to spaces
    mov   edi,esi
    shl   edi,8
    add   edi,0x80000
    mov   ecx,11
    mov   eax,' '
  ;  cld
    rep   stosb
    popa

    pusha                ; C000 --> C400
    mov   eax, 0xc000
    mov   esi, 0
  nlc40:
    add   eax, 2
    inc   esi
    cmp   esi, [0x3004]
    jae   nlc41
    movzx ecx, word [eax]
    mov   [0xC400 + ecx*2], si
    jmp   nlc40
  nlc41:
 ; ivan 08.12.2004 begin
    ;mov   ebx, [0x3004]
    ;dec   ebx
    ;lea   esi, [0xC400 + ebx*2]
    ;call  windowactivate
 ; ivan 08.12.2004 end
    popa

    pusha ; remove hd1 reservation
    mov   edx,esi
    shl   edx, 5 ;imul  edx,0x20
    add   edx,0x3000
    mov   edx,[edx+4]
    cmp   [hd1_status],edx
    jne   no_hd1_s_remove
    mov   [hd1_status],0
  no_hd1_s_remove:
    popa

    pusha ; remove all irq reservations
    mov   edx,esi
    shl   edx, 5 ;imul  edx,0x20
    add   edx,0x3000
    mov   edx,[edx+4]
    mov   edi,irq_owner
    mov   ecx,16
  newirqfree:
    cmp   [edi],edx
    jne   nofreeirq
    mov   [edi],dword 0
  nofreeirq:
    add    edi,4
    loop   newirqfree
    popa


    pusha                     ; remove all port reservations

    mov   edx,esi
    shl   edx, 5 ;imul  edx,0x20
    add   edx,0x3000
    mov   edx,[edx+4]

  rmpr0:

    mov   esi,[0x2d0000]

    cmp   esi,0
    je    rmpr9

  rmpr3:

    mov   edi,esi
    shl   edi,4
    add   edi,0x2d0000

    cmp   edx,[edi]
    je    rmpr4

    dec   esi
    jnz   rmpr3

    jmp   rmpr9

  rmpr4:

    mov   ecx,256
    sub   ecx,esi
    shl   ecx,4

    mov   esi,edi
    add   esi,16
    cld
    rep   movsb

    dec   dword [0x2d0000]

    jmp   rmpr0

  rmpr9:

    popa
    mov  edi,esi         ; do not run this process slot
    shl  edi, 5 ;imul edi,0x20
;    add  edi,0x3000
    mov  [edi+0x300A],byte 9
;    call  systest
    sti  ; .. and life goes on

;    movzx eax,word [dlx]
;    movzx ebx,word [dly]
;    movzx ecx,word [dlxe]
;    movzx edx,word [dlye]
    call  calculatescreen

    xor   eax, eax
    xor   esi, esi
    call  redrawscreen

    mov   [0xfff4],byte 0  ; no mouse background
    mov   [0xfff5],byte 0  ; draw mouse

    mov   [application_table_status],0

    mov   esi,process_terminated
    call  sys_msg_board_str

;* start code - fix error redraw for terminate (2) - Mario79
;     cmp  [draw_present],1
;     je  no_draw_background_temp
;     mov   [0xfff0],byte 1
;no_draw_background_temp:
;    mov   [draw_present],0
    ret
;draw_present db 0
;* end code - fix error redraw for terminate (2) - Mario79

iglobal
  boot_sched_1    db   'Building gdt tss pointer',0
  ;boot_sched_2    db   'Building gdt gate pointer',0
  boot_sched_3    db   'Building interrupt table - TSS',0
  boot_sched_3_2  db   'Building interrupt table - GDT',0
  boot_sched_3_3  db   'Building interrupt table - IDT',0
  boot_sched_4    db   'Building syscall interrupt table',0
endg


build_scheduler:
      ; { Ivan 06.03.2005
        mov    edi, usedi40
        mov    ecx, 256/4
        xor    eax, eax
        rep    stosd
      ; } Ivan 06.03.2005

        mov    esi,boot_sched_1
        call   boot_log
        call   build_process_gdt_tss_pointer

;        mov    esi,boot_sched_2
;        call   boot_log
;        call   build_process_gdt_gate_pointer

        mov    esi,boot_sched_3
        call   boot_log
        call   build_interrupt_table

        mov    esi,boot_sched_4
        call   boot_log
        call   build_syscall_interrupt_table

        ret

