
SRV_GETVERSION   equ 0
PORT_OPEN        equ 1
PORT_CLOSE       equ 2
PORT_RESET       equ 3
PORT_SETMODE     equ 4
PORT_GETMODE     equ 5
PORT_SETMCR      equ 6
PORT_GETMCR      equ 7
PORT_READ        equ 8
PORT_WRITE       equ 9

RATE_19200       equ 15
LCR_8BIT         equ  0x03
MCR_OUT_2        equ  0x08

io.handle        equ esp
io.code          equ esp+4
io.input         equ esp+8
io.inp_size      equ esp+12
io.output        equ esp+16
io.out_size      equ esp+20

; retval
;  ebx= service version
;  eax= error code
;    0= no error
;   -1= common error

align 4
init_uart:
           mov eax, 68
           mov ebx, 16
           mov ecx, szUart
           int 0x40

           mov [Uart], eax
           test eax, eax
           jz .fail

           push 0              ;storage for version
           mov eax, esp
           xor ebx, ebx

           push 4              ;.out_size
           push eax            ;.output
           push ebx            ;.inp_size
           push ebx            ;.input
           push SRV_GETVERSION ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ebx             ;version
           ret
.fail:
           or eax, -1
           ret

; param
;  eax= port
;
; retval
;  ebx= connection
;  eax= error code

align 4
uart_open:

           push 0              ;connection
           mov ebx, esp
           push eax            ;port
           mov eax, esp

           push 4              ;.out_size
           push ebx            ;.output
           push 4              ;.inp_size
           push eax            ;.input
           push PORT_OPEN      ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 28         ;io_control+port
           pop ebx             ;connection
           ret

; param
;  eax= connection
;
; retval
;  eax= error code

align 4
uart_close:
           push eax            ;connection
           mov eax, esp
           xor ebx, ebx

           push ebx            ;.out_size
           push ebx            ;.output
           push 4              ;.inp_size
           push eax            ;.input
           push PORT_CLOSE     ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;io_contol
           int 0x40
           add esp, 24+4       ;io_control
           ret

; param
;  eax= connection
;
; retval
;  eax= error code

align 4
uart_reset:
           push eax            ;connection
           mov eax, esp
           xor ebx, ebx

           push ebx            ;.out_size
           push ebx            ;.output
           push 4              ;.inp_size
           push eax            ;.input
           push PORT_RESET     ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;io_contol
           int 0x40
           add esp, 24+4       ;io_control
           ret

; param
;  eax= connection
;  ebx= rate
;  ecx= mode
;
; retval
;  eax= error code

align 4
uart_setmode:
           push ecx            ;mode
           push ebx            ;rate
           push eax            ;connection
           mov eax, esp
           xor ecx, ecx

           push ecx            ;.out_size
           push ecx            ;.output
           push 12             ;.inp_size
           push eax            ;.input
           push PORT_SETMODE   ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;io_contol
           int 0x40
           add esp, 24+12      ;io_control
           ret

; param
;  eax= connection
;  ebx= modem control value
;
; retval
;  eax= error code

align 4
uart_set_mcr:
           push ebx            ;mcr
           push eax            ;connection
           mov eax, esp
           xor ecx, ecx

           push ecx            ;.out_size
           push ecx            ;.output
           push 8              ;.inp_size
           push eax            ;.input
           push PORT_SETMCR    ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;io_contol
           int 0x40
           add esp, 24+8       ;io_control
           ret

; param
;  eax= connection
;  ebx= buffer
;  ecx= size
;
; retval
;  ebx= bytes read
;  eax= error code

align 4
uart_read:
           push 0              ;bytes read
           push ecx            ;size
           push ebx            ;buffer
           push eax            ;connection
           mov eax, esp
           lea ebx, [esp+12]

           push 4              ;.out_size
           push ebx            ;.output
           push 12             ;.inp_size
           push eax            ;.input
           push PORT_READ      ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;io_contol
           int 0x40
           add esp, 24+12      ;io_control
           pop ebx
           ret

; param
;  eax= connection
;  ebx= buffer
;  ecx= size
;
; retval
;  eax= error code

align 4
uart_write:
           push ecx            ;size
           push ebx            ;buffer
           push eax            ;connection
           mov eax, esp
           xor ecx, ecx

           push ecx            ;.out_size
           push ecx            ;.output
           push 12             ;.inp_size
           push eax            ;.input
           push PORT_WRITE     ;.code
           push [Uart]         ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;io_contol
           int 0x40
           add esp, 24+12      ;io_control
           ret

align 4
Uart        rd 1

szUart      db 'UART',0


