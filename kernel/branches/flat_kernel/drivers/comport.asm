
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
MCR_OUT_2        equ  0x08     ;0-> OUT2=1, 1-> OUT2=0
MCR_LOOP         equ  0x10     ;lopback mode

MSR_DCTS         equ  0x01     ;delta clear to send
MSR_DDSR         equ  0x02     ;delta data set redy
MSR_TERI         equ  0x04     ;trailinh edge of ring
MSR_DDCD         equ  0x08     ;delta carrier detect

COM_THR          equ 0x3f8     ;transtitter/reciever
COM_IER          equ 0x3f9     ;interrupt enable
COM_IIR          equ 0x3fA     ;interrupt info
COM_LCR          equ 0x3FB     ;line control
COM_MCR          equ 0x3FC     ;modem control
COM_LSR          equ 0x3FD     ;line status
COM_MSR          equ 0x3FE     ;modem status

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

COM_1            equ  0
COM_2            equ  1
COM_3            equ  2
COM_4            equ  3

COM_1_IRQ        equ  4
COM_2_IRQ        equ  3
TRANSMIT         equ  1


struc COMPORT
{
   .base         dd ?
   .lcr_reg      dd ?
   .mcr_reg      dd ?
   .rate         dd ?
   .mode         dd ?
   .state        dd ?
   .connection   dd ?

   .rcvr_rp      dd ?
   .rcvr_wp      dd ?
   .rcvr_free    dd ?
   .rcvr_count   dd ?

   .xmit_rp      dd ?
   .xmit_wp      dd ?
   .xmit_free    dd ?
   .xmit_count   dd ?
   .xmit_buffer  rb 128
   .rcvr_buffer  rb 128
}
virtual at 0
  COMPORT COMPORT
end virtual

COMPORT_SIZE  equ 256+15*4

UART_VERSION  equ 0x00000000

init_com:
           mov eax, COMPORT_SIZE
           call malloc
           test eax, eax
           jz .fail

           mov [com1], eax
           mov edi, eax
           mov ecx, COMPORT_SIZE/4
           xor eax, eax
           cld
           rep stosd

           call reset

           stdcall attach_int_handler, COM_1_IRQ, com_isr
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
           mov eax, [eax]
           mov [eax], dword UART_VERSION
           xor eax, eax
           ret
@@:
           cmp eax, PORT_OPEN
           jne @F
           call open_port
           ret


           mov esi, [ebx+input]
           mov ecx, [esi]
           mov edx, [com1]
           cmp [edx+COMPORT.connection], ecx
           je @F
           mov edx, [com2]
           cmp [edx+COMPORT.connection], ecx
           jne .fail

           mov edi, [ebx+output]
           call [uart_func+eax*4]  ;edx, esi, edi
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


open_port:
           ret

; param
;  edx= port
;  esi= input data
;  edi= output data
;
; retval
;  eax=0 success
;  eax <>0 error

align 4
close_port:

           call reset
           mov [edx+COMPORT.connection], 0
           xor eax, eax
           ret


; set mode 2400 bod 8-bit
; clear FIFO
; clear pending interrupts

align 4
reset:
           mov eax, RATE_2400
           mov ebx, LCR_8BIT+LCR_STOP_1
           call set_mode

           mov eax,FCR_EFIFO+FCR_CRB+FCR_CXMIT+FCR_FIFO_14
           mov edx, COM_IIR
           out dx, al

.clear_RB:
           mov edx, COM_LSR
           in al, dx
           test eax, LSR_DR
           jz @F

           mov edx, COM_THR
           in al, dx
           jmp .clear_RB
@@:
           mov eax,IER_RDAI+IER_THRI+IER_LSI
           mov edx, COM_IER
           out dx, al

.clear_IIR:
           mov edx, COM_IIR
           in al, dx
           test al, IIR_INTR
           jnz .done

           shr eax, 1
           and eax, 3
           jnz @F

           mov edx, COM_MSR
           in al, dx
           jmp .clear_IIR
@@:
           cmp eax, 1
           je .clear_IIR

           cmp eax, 2
           jne @F

           mov edx, COM_THR
           in al, dx
           jmp .clear_IIR
@@:
           mov edx, COM_LSR
           in al, dx
           jmp .clear_IIR

.done:
           mov edi, rcvr_buff
           xor eax, eax
           mov ecx, 256/4

           mov [rcvr_rp], edi
           mov [rcvr_wp], edi
           mov [rcvr_free], 128
;;           mov [rcvr_count], 16

           mov [xmit_rp], xmit_buff
           mov [xmit_wp], xmit_buff
           mov [xmit_free], 128
           mov [xmit_count], 16       ;FIFO free

           cld
           rep stosd
           ret

; param
;  eax= rate constant
;  ebx= mode bits

align 4
set_mode:
           cmp eax, RATE_115200
           ja .fail

           cmp ebx, LCR_BREAK
           jae .fail

           mov [rate], eax
           mov [mode], ebx

           mov cx, [divisor+eax*2]

           mov dx, COM_LCR
           in al, dx
           or al, 0x80
           out dx, al

           mov dx, COM_THR
           mov al, cl
           out dx, al

           inc dx
           mov al, ch
           out dx, al

           mov dx, COM_LCR
           mov eax, ebx
           out dx, al
.fail:
           ret

; param
;  eax= rate constant

align 4
set_rate:
           cmp eax, RATE_115200
           ja .fail

           mov [rate], eax
           mov bx, [divisor+eax*2]

           mov dx, COM_LCR
           in al, dx
           or al, 0x80
           out dx, al

           mov dx, COM_THR
           mov al, bl
           out dx, al

           inc dx
           mov al, bh
           out dx, al

           mov dx, COM_LCR
           mov eax, [lcr_reg]
           out dx, al
.fail:
           ret

align 4
transmit:
           push esi
           mov edx, COM_THR

           mov [xmit_count], 16

           pushfd
           cli

           mov esi, [xmit_rp]
           mov ecx, [xmit_free]

           cmp ecx, 128
           je .exit
@@:
           and esi, 127
           mov al, [xmit_buff+esi]
           inc esi

           out dx, al
           inc ecx
           dec [xmit_count]
           jz .done

           cmp ecx, 128
           jne @B
.done:
           add esi, xmit_buff
           mov [xmit_rp], esi
           mov [xmit_free], ecx
           mov [com_state], TRANSMIT
.exit:
           popfd
           pop esi
           ret


; eax= src
; ebx= count

align 4
comm_send:
           mov edi, [xmit_wp]
           mov esi, eax
.write:
           test ebx, ebx
           jz .done
.wait:
           cmp [xmit_free], 0
           jne .fill

           cmp [com_state], TRANSMIT
           je .wait

           call transmit
           jmp .write
.fill:
           mov ecx, xmit_buff+128
           sub ecx, edi
           cmp ecx, [xmit_free]
           jb @F

           mov ecx, [xmit_free]
@@:
           cmp ecx, ebx
           jb @F
           mov ecx, ebx
@@:
           sub [xmit_free], ecx
           sub ebx, ecx

           cld
           rep movsb

           cmp edi, xmit_buff+128
           jb .write
           sub edi, 128
           jmp .write

.done:
           cmp [com_state], TRANSMIT
           je @F
           call transmit
@@:
           ret

align 4
com_isr:

.get_info:
           mov dx, COM_IIR
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
           mov edx, COM_LSR
           in al, dx
           ret

align 4
isr_recieve:
           mov edx, COM_THR
           in al, dx
           ret

align 4
isr_modem:
            mov edx, COM_MSR
            in al, dx
            ret



align 4
uart_func   dd 0            ;get version
            dd 0            ;open port
            dd close_port


isr_action  dd isr_modem
            dd transmit
            dd isr_recieve
            dd isr_line


divisor     dw 2304, 1536, 1047, 857, 768, 384
            dw  192,   96,   64,  58,  48,  32
            dw   24,   16,   12,   6,   3,   2, 1



sz_uart_srv db 'UART',0

;version      dd 0x00040000


align 4

com1        dd ?
com2        dd ?

rcvr_rp     dd ?
rcvr_wp     dd ?
rcvr_free   dd ?
rcvr_count  dd ?

xmit_rp     dd ?
xmit_wp     dd ?
xmit_free   dd ?
xmit_count  dd ?

lcr_reg     dd ?
mcr_reg     dd ?
rate        dd ?
mode        dd ?
com_state   dd ?

connection  dd ?

align 128
rcvr_buff   rb 128
xmit_buff   rb 128

