
;OS_BASE         equ 0x80000000
;new_app_base    equ 0x60400000
;PROC_BASE       equ OS_BASE+0x0080000

struc IOCTL
{  .handle      dd ?
   .io_code     dd ?
   .input       dd ?
   .inp_size    dd ?
   .output      dd ?
   .out_size    dd ?
}


;public START
;public service_proc
;public version

DEBUG            equ   1

DRV_ENTRY        equ   1
DRV_EXIT         equ  -1

THR_REG          equ  0;  x3f8   ;transtitter/reciever
IER_REG          equ  1;  x3f9   ;interrupt enable
IIR_REG          equ  2;  x3fA   ;interrupt info
LCR_REG          equ  3;  x3FB   ;line control
MCR_REG          equ  4;  x3FC   ;modem control
LSR_REG          equ  5;  x3FD   ;line status
MSR_REG          equ  6;  x3FE   ;modem status

LCR_5BIT         equ  0x00
LCR_6BIT         equ  0x01
LCR_7BIT         equ  0x02
LCR_8BIT         equ  0x03
LCR_STOP_1       equ  0x00
LCR_STOP_2       equ  0x04
LCR_PARITY       equ  0x08
LCR_EVEN         equ  0x10
LCR_STICK        equ  0x20
LCR_BREAK        equ  0x40
LCR_DLAB         equ  0x80

LSR_DR           equ  0x01     ;data ready
LSR_OE           equ  0x02     ;overrun error
LSR_PE           equ  0x04     ;parity error
LSR_FE           equ  0x08     ;framing error
LSR_BI           equ  0x10     ;break interrupt
LSR_THRE         equ  0x20     ;transmitter holding empty
LSR_TEMT         equ  0x40     ;transmitter empty
LSR_FER          equ  0x80     ;FIFO error

FCR_EFIFO        equ  0x01     ;enable FIFO
FCR_CRB          equ  0x02     ;clear reciever FIFO
FCR_CXMIT        equ  0x04     ;clear transmitter FIFO
FCR_RDY          equ  0x08     ;set RXRDY and TXRDY pins
FCR_FIFO_1       equ  0x00     ;1  byte trigger
FCR_FIFO_4       equ  0x40     ;4  bytes trigger
FCR_FIFO_8       equ  0x80     ;8  bytes trigger
FCR_FIFO_14      equ  0xC0     ;14 bytes trigger

IIR_INTR         equ  0x01     ;1= no interrupts

IER_RDAI         equ  0x01     ;reciever data interrupt
IER_THRI         equ  0x02     ;transmitter empty interrupt
IER_LSI          equ  0x04     ;line status interrupt
IER_MSI          equ  0x08     ;modem status interrupt

MCR_DTR          equ  0x01     ;0-> DTR=1, 1-> DTR=0
MCR_RTS          equ  0x02     ;0-> RTS=1, 1-> RTS=0
MCR_OUT_1        equ  0x04     ;0-> OUT1=1, 1-> OUT1=0
MCR_OUT_2        equ  0x08     ;0-> OUT2=1, 1-> OUT2=0;  enable intr
MCR_LOOP         equ  0x10     ;lopback mode

MSR_DCTS         equ  0x01     ;delta clear to send
MSR_DDSR         equ  0x02     ;delta data set redy
MSR_TERI         equ  0x04     ;trailinh edge of ring
MSR_DDCD         equ  0x08     ;delta carrier detect


RATE_50          equ  0
RATE_75          equ  1
RATE_110         equ  2
RATE_134         equ  3
RATE_150         equ  4
RATE_300         equ  5
RATE_600         equ  6
RATE_1200        equ  7
RATE_1800        equ  8
RATE_2000        equ  9
RATE_2400        equ 10
RATE_3600        equ 11
RATE_4800        equ 12
RATE_7200        equ 13
RATE_9600        equ 14
RATE_19200       equ 15
RATE_38400       equ 16
RATE_57600       equ 17
RATE_115200      equ 18

COM_1            equ  1
COM_2            equ  2
COM_3            equ  3
COM_4            equ  4
COM_MAX          equ  2    ;only two port supported

COM_1_BASE       equ 0x3F8
COM_2_BASE       equ 0x2F8

COM_1_IRQ        equ  4
COM_2_IRQ        equ  3

UART_CLOSED      equ  0
UART_TRANSMIT    equ  1
UART_STOP        equ  2

struc UART
{
   .lock         dd ?
   .base         dd ?
   .lcr_reg      dd ?
   .mcr_reg      dd ?
   .rate         dd ?
   .mode         dd ?
   .state        dd ?

   .rcvr_buff    dd ?
   .rcvr_rp      dd ?
   .rcvr_wp      dd ?
   .rcvr_count   dd ?
   .rcvr_top     dd ?

   .xmit_buff    dd ?
   .xmit_rp      dd ?
   .xmit_wp      dd ?
   .xmit_count   dd ?
   .xmit_free    dd ?
   .xmit_top     dd ?
}
virtual at 0
  UART UART
end virtual

UART_SIZE     equ 18*4

struc CONNECTION
{
   .magic       dd ?   ;'CNCT'
   .destroy     dd ?   ;internal destructor
   .fd          dd ?   ;next object in list
   .bk          dd ?   ;prev object in list
   .pid         dd ?   ;owner id

   .id          dd ?   ;reserved
   .uart        dd ?   ;uart pointer
}

virtual at 0
  CONNECTION CONNECTION
end virtual

CONNECTION_SIZE equ 7*4

UART_VERSION  equ 0x12345678        ;debug

init_uart_service:
           mov eax, UART_SIZE
           call malloc
           test eax, eax
           jz .fail

           mov [com1], eax
           mov edi, eax
           mov ecx, UART_SIZE/4
           xor eax, eax
           cld
           rep stosd

           mov eax, [com1]
           mov [eax+UART.base], COM_1_BASE

           stdcall alloc_kernel_space, 32768

           mov edi, [com1]
           mov edx, eax

           mov [edi+UART.rcvr_buff], eax
           add eax, 8192
           mov [edi+UART.rcvr_top], eax
           add eax, 8192
           mov [edi+UART.xmit_buff], eax
           add eax, 8192
           mov [edi+UART.xmit_top], eax

           call alloc_page
           test eax, eax
           jz .fail

           shr edx, 12
           or eax, PG_SW
           mov [page_tabs+edx*4], eax
           mov [page_tabs+edx*4+8], eax

           call alloc_page
           test eax, eax
           jz .fail

           or eax, PG_SW
           mov [page_tabs+edx*4+4], eax
           mov [page_tabs+edx*4+12], eax

           call alloc_page
           test eax, eax
           jz .fail

           or eax, PG_SW
           mov [page_tabs+edx*4+16], eax
           mov [page_tabs+edx*4+24], eax

           call alloc_page
           test eax, eax
           jz .fail

           or eax, PG_SW
           mov [page_tabs+edx*4+20], eax
           mov [page_tabs+edx*4+28], eax

           mov eax, [edi+UART.rcvr_buff]
           invlpg [eax]
           invlpg [eax+0x1000]
           invlpg [eax+0x2000]
           invlpg [eax+0x3000]
           invlpg [eax+0x4000]
           invlpg [eax+0x5000]
           invlpg [eax+0x6000]
           invlpg [eax+0x7000]

           mov eax, edi
           call uart_reset.internal   ;eax= uart

           stdcall attach_int_handler, COM_1_IRQ, com_1_isr
           stdcall reg_service, sz_uart_srv, uart_proc
           ret
.fail:
           xor eax, eax
           ret

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

SRV_GETVERSION  equ 0
PORT_OPEN       equ 1
PORT_CLOSE      equ 2
PORT_RESET      equ 3
PORT_SETMODE    equ 4
PORT_GETMODE    equ 5
PORT_SETMCR     equ 6
PORT_GETMCR     equ 7
PORT_READ       equ 8
PORT_WRITE      equ 9

align 4
proc uart_proc stdcall, ioctl:dword

           mov ebx, [ioctl]
           mov eax, [ebx+io_code]
           cmp eax, PORT_WRITE
           ja .fail

           cmp eax, SRV_GETVERSION
           jne @F

           mov eax, [ebx+output]
           mov [eax], dword UART_VERSION
           xor eax, eax
           ret
@@:
           cmp eax, PORT_OPEN
           jne @F

           mov ebx, [ebx+input]
           mov eax, [ebx]
           call uart_open
           mov ebx, [ioctl]
           mov ebx, [ebx+output]
           mov [ebx], ecx
           ret
@@:
           mov esi, [ebx+input]     ;input buffer
           mov edi, [ebx+output]
           call [uart_func+eax*4]
           ret
.fail:
           or eax, -1
           ret

endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size


; param
;  esi=  input buffer
;        +0 connection
;
; retval
;  eax= error code

align 4
uart_reset:
           mov eax, [esi]
           cmp [eax+APPOBJ.magic], 'CNCT'
           jne .fail

           cmp [eax+APPOBJ.destroy], uart_close.destroy
           jne .fail

           mov eax, [eax+CONNECTION.uart]
           test eax, eax
           jz .fail

; set mode 2400 bod 8-bit
; disable DTR & RTS
; clear FIFO
; clear pending interrupts
;
; param
;  eax= uart

align 4
.internal:
           mov esi, eax
           mov [eax+UART.state], UART_CLOSED
           mov edx, [eax+UART.base]
           add edx, MCR_REG
           xor eax, eax
           out dx, al        ;clear DTR & RTS

           mov eax, esi
           mov ebx, RATE_2400
           mov ecx, LCR_8BIT+LCR_STOP_1
           call uart_set_mode.internal

           mov edx, [esi+UART.base]
           add edx, IIR_REG
           mov eax,FCR_EFIFO+FCR_CRB+FCR_CXMIT+FCR_FIFO_14
           out dx, al
.clear_RB:
           mov edx, [esi+UART.base]
           add edx, LSR_REG
           in al, dx
           test eax, LSR_DR
           jz @F

           mov edx, [esi+UART.base]
           in al, dx
           jmp .clear_RB
@@:
           mov edx, [esi+UART.base]
           add edx, IER_REG
           mov eax,IER_RDAI+IER_THRI+IER_LSI
           out dx, al
.clear_IIR:
           mov edx, [esi+UART.base]
           add edx, IIR_REG
           in al, dx
           test al, IIR_INTR
           jnz .done

           shr eax, 1
           and eax, 3
           jnz @F

           mov edx, [esi+UART.base]
           add edx, MSR_REG
           in al, dx
           jmp .clear_IIR
@@:
           cmp eax, 1
           je .clear_IIR

           cmp eax, 2
           jne @F

           mov edx, [esi+UART.base]
           in al, dx
           jmp .clear_IIR
@@:
           mov edx, [esi+UART.base]
           add edx, LSR_REG
           in al, dx
           jmp .clear_IIR
.done:
           mov edi, [esi+UART.rcvr_buff]
           mov ecx, 8192/4
           xor eax, eax

           mov [esi+UART.rcvr_rp], edi
           mov [esi+UART.rcvr_wp], edi
           mov [esi+UART.rcvr_count], eax

           cld
           rep stosd

           mov edi, [esi+UART.xmit_buff]
           mov ecx, 8192/4

           mov [esi+UART.xmit_rp], edi
           mov [esi+UART.xmit_wp], edi
           mov [esi+UART.xmit_count], eax
           mov [esi+UART.xmit_free], 8192

           rep stosd
           ret               ;eax= 0
.fail:
           or eax, -1
           ret

; param
;  esi=  input buffer
;        +0 connection
;        +4 rate
;        +8 mode
;
; retval
;  eax= error code

align 4
uart_set_mode:
           mov eax, [esi]
           cmp [eax+APPOBJ.magic], 'CNCT'
           jne .fail

           cmp [eax+APPOBJ.destroy], uart_close.destroy
           jne .fail

           mov eax, [eax+CONNECTION.uart]
           test eax, eax
           jz .fail

           mov ebx, [esi+4]
           mov ecx, [esi+8]

; param
;  eax= uart
;  ebx= baud rate
;  ecx= mode

align 4
.internal:
           cmp ebx, RATE_115200
           ja .fail

           cmp ecx, LCR_BREAK
           jae .fail

           mov [eax+UART.rate], ebx
           mov [eax+UART.mode], ecx

           mov esi, eax
           mov bx, [divisor+ebx*2]

           mov edx, [esi+UART.base]
           push edx
           add edx, LCR_REG
           in al, dx
           or al, 0x80
           out dx, al

           pop edx
           mov al, bl
           out dx, al

           inc dx
           mov al, bh
           out dx, al

           add edx, LCR_REG-1
           mov eax, ecx
           out dx, al
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret

; param
;  esi=  input buffer
;        +0 connection
;        +4 modem control reg valie
;
; retval
;  eax= error code

align 4
uart_set_mcr:

           mov eax, [esi]
           cmp [eax+APPOBJ.magic], 'CNCT'
           jne .fail

           cmp [eax+APPOBJ.destroy], uart_close.destroy
           jne .fail

           mov eax, [eax+CONNECTION.uart]
           test eax, eax
           jz .fail

           mov ebx, [esi+4]

           mov [eax+UART.mcr_reg], ebx
           mov edx, [eax+UART.base]
           add edx, MCR_REG
           mov al, bl
           out dx, al
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret

; param
;  eax= port
;
; retval
;  ecx= connection
;  eax= error code

align 4
uart_open:
           dec eax
           cmp eax, COM_MAX
           jae .fail

           mov esi, [com1+eax*4]             ;uart
           push esi
.do_wait:
           cmp dword [esi+UART.lock],0
           je .get_lock
           call change_task
           jmp .do_wait
.get_lock:
           mov eax, 1
           xchg eax, [esi+UART.lock]
           test eax, eax
           jnz .do_wait

           mov eax, esi                  ;uart
           call uart_reset.internal

           mov ebx, [CURRENT_TASK]
           shl ebx, 5
           mov ebx, [CURRENT_TASK+ebx+4]
           mov eax, CONNECTION_SIZE
           call create_kernel_object
           pop esi                       ;uart
           test eax, eax
           jz .fail

           mov [eax+APPOBJ.magic], 'CNCT'
           mov [eax+APPOBJ.destroy], uart_close.destroy
           mov [eax+CONNECTION.uart], esi
           mov ecx, eax
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
restore .uart

; param
;  esi= input buffer

align 4
uart_close:
           mov eax, [esi]
           cmp [eax+APPOBJ.magic], 'CNCT'
           jne .fail

           cmp [eax+APPOBJ.destroy], uart_close.destroy
           jne .fail
.destroy:
           push [eax+CONNECTION.uart]
           call destroy_kernel_object   ;eax= object
           pop eax                      ;eax= uart
           test eax, eax
           jz .fail

           mov [eax+UART.state], UART_CLOSED
           mov [eax+UART.lock], 0 ;release port
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret


; param
;  eax= uart
;  ebx= baud rate

align 4
set_rate:
           cmp ebx, RATE_115200
           ja .fail

           mov [eax+UART.rate], ebx
           mov bx, [divisor+ebx*2]

           mov edx, [eax+UART.base]
           add edx, LCR_REG
           in al, dx
           push eax
           or al, 0x80
           out dx, al

           sub edx, LCR_REG
           mov al, bl
           out dx, al

           inc edx
           mov al, bh
           out dx, al

           pop eax
           add edx, LCR_REG-1
           out dx, al
.fail:
           ret


; param
;   ebx= uart

align 4
transmit:
           push esi
           push edi

           mov edx, [ebx+UART.base]

           pushfd
           cli

           mov esi, [ebx+UART.xmit_rp]
           mov ecx, [ebx+UART.xmit_count]
           test ecx, ecx
           je .stop

           cmp ecx, 16
           jbe @F
           mov ecx, 16
@@:
           sub [ebx+UART.xmit_count], ecx
           add [ebx+UART.xmit_free], ecx
           cld
@@:
           lodsb
           out dx, al
           dec ecx
           jnz @B

           cmp esi,[ebx+UART.xmit_top]
           jb @F
           sub esi, 8192
@@:
           mov [ebx+UART.xmit_rp], esi

           cmp [ebx+UART.xmit_count], 0
           je .stop

           mov [ebx+UART.state], UART_TRANSMIT
           jmp @F
.stop:
           mov [ebx+UART.state], UART_STOP
@@:
           popfd
           pop edi
           pop esi
           ret


; param
;  esi=  input buffer
;        +0 connection
;        +4 dst buffer
;        +8 dst size
;  edi=  output buffer
;        +0 bytes read

; retval
;  eax= error code

align 4
uart_read:
           mov eax, [esi]
           cmp [eax+APPOBJ.magic], 'CNCT'
           jne .fail

           cmp [eax+APPOBJ.destroy], uart_close.destroy
           jne .fail

           mov eax, [eax+CONNECTION.uart]
           test eax, eax
           jz .fail

           mov ebx, [esi+8]    ;dst size
           mov ecx, [eax+UART.rcvr_count]
           cmp ecx, ebx
           jbe @F
           mov ecx, ebx
@@:
           mov [edi], ecx      ;bytes read
           test ecx, ecx
           jz .done

           push ecx

           mov edi, [esi+4]    ;dst
           mov esi, [eax+UART.rcvr_rp]
           cld
           rep movsb
           pop ecx

           cmp esi, [eax+UART.rcvr_top]
           jb @F
           sub esi, 8192
@@:
           mov [eax+UART.rcvr_rp], esi
           sub [eax+UART.rcvr_count], ecx
.done:
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret

; param
;  esi=  input buffer
;        +0 connection
;        +4 src buffer
;        +8 src size
;
; retval
;  eax= error code

align 4
uart_write:
           mov eax, [esi]
           cmp [eax+APPOBJ.magic], 'CNCT'
           jne .fail

           cmp [eax+APPOBJ.destroy], uart_close.destroy
           jne .fail

           mov eax, [eax+CONNECTION.uart]
           test eax, eax
           jz .fail

           mov ebx, [esi+4]
           mov edx, [esi+8]

; param
;  eax= uart
;  ebx= src
;  edx= count

align 4
.internal:
           mov esi, ebx
           mov edi, [eax+UART.xmit_wp]
.write:
           test edx, edx
           jz .fail
.wait:
           cmp [eax+UART.xmit_free], 0
           jne .fill

           cmp [eax+UART.state], UART_TRANSMIT
           je .wait

           mov ebx, eax
           push edx
           call transmit
           pop edx
           mov eax, ebx
           jmp .write
.fill:
           mov ecx, [eax+UART.xmit_free]
           cmp ecx, edx
           jbe @F
           mov ecx, edx
@@:
           push ecx
           cld
           rep movsb
           pop ecx
           sub [eax+UART.xmit_free], ecx
           add [eax+UART.xmit_count], ecx
           sub edx, ecx
           jnz .wait
.done:
           cmp edi, [eax+UART.xmit_top]
           jb @F
           sub edi, 8192
@@:
           mov [eax+UART.xmit_wp], edi
           cmp [eax+UART.state], UART_TRANSMIT
           je @F
           mov ebx, eax
           call transmit
@@:
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret


align 4
com_2_isr:
           mov ebx, [com2]
           jmp com_1_isr.get_info
align 4
com_1_isr:
           mov ebx, [com1]
.get_info:
           mov edx, [ebx+UART.base]
           add edx, IIR_REG
           in  al, dx

           test al, IIR_INTR
           jnz .done

           shr eax, 1
           and eax, 3

           call [isr_action+eax*4]
           jmp .get_info
.done:
           ret

align 4
isr_line:
           mov edx, [ebx+UART.base]
           add edx, LSR_REG
           in al, dx
           ret

align 4
isr_recieve:
           mov esi, [ebx+UART.base]
           add esi, LSR_REG
           mov edi, [ebx+UART.rcvr_wp]
           xor ecx, ecx
           cld
.read:
           mov edx, esi
           in al, dx
           test eax, LSR_DR
           jz .done

           mov edx, [ebx+UART.base]
           in al, dx
           stosb
           inc ecx
           jmp .read
.done:
           cmp edi, [ebx+UART.rcvr_top]
           jb @F
           sub edi, 8192
@@:
           mov [ebx+UART.rcvr_wp], edi
           add [ebx+UART.rcvr_count], ecx
           ret

align 4
isr_modem:
           mov edx, [ebx+UART.base]
           add edx, MSR_REG
           in al, dx
           ret


align 4
com1        dd 0
com2        dd 0

align 4
uart_func   dd 0                ;SRV_GETVERSION
            dd 0                ;PORT_OPEN
            dd uart_close       ;PORT_CLOSE
            dd uart_reset       ;PORT_RESET
            dd uart_set_mode    ;PORT_SETMODE
            dd 0                ;PORT_GETMODE
            dd uart_set_mcr     ;PORT_SETMODEM
            dd 0                ;PORT_GETMODEM
            dd uart_read        ;PORT_READ
            dd uart_write       ;PORT_WRITE

isr_action  dd isr_modem
            dd transmit
            dd isr_recieve
            dd isr_line

;version      dd 0x00040000

divisor     dw 2304, 1536, 1047, 857, 768, 384
            dw  192,   96,   64,  58,  48,  32
            dw   24,   16,   12,   6,   3,   2, 1



sz_uart_srv db 'UART',0



