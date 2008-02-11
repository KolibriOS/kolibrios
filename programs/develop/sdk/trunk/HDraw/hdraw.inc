
SRV_GETVERSION   equ 0
SOLID_FILL       equ 1
LINE2P           equ 2

io.handle        equ esp
io.code          equ esp+4
io.input         equ esp+8
io.inp_size      equ esp+12
io.output        equ esp+16
io.out_size      equ esp+20

IOSIZE           equ 24

; retval
;  ebx= service version
;  eax= error code
;    0= no error
;   -1= common error

align 4

init_HDraw:
           mov eax, 68
           mov ebx, 16
           mov ecx, szHDraw
           int 0x40

           mov [HDraw], eax
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
           push [HDraw]        ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, IOSIZE
           pop ebx             ;version
           ret
.fail:
           or eax, -1
           ret

; param
;  eax= color
;  ebx= x
;  ecx= y
;  edx= w
;  esi= h

; retval
;  eax= error code

align 4
solid_fill:

           push esi
           push edx
           push ecx
           push ebx
           push eax

           xor eax, eax
           mov ebx, esp        ;FILL

           push eax            ;.out_size
           push eax            ;.output
           push 5              ;.inp_size
           push ebx            ;.input
           push SOLID_FILL     ;.code
           push [HDraw]        ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, (IOSIZE+5*4) ;io_control+FILL
           ret

align 4
line2p:
           push esi            ;y2
           push edx            ;x2
           push ecx            ;y1
           push ebx            ;x1
           push eax            ;color

           xor eax, eax
           mov ebx, esp        ;LINE2P

           push eax            ;.out_size
           push eax            ;.output
           push 5              ;.inp_size
           push ebx            ;.input
           push LINE2P         ;.code
           push [HDraw]        ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, (IOSIZE+5*4) ;io_control+LINE2P
           ret


align 4
HDraw      rd 1

szHDraw    db 'HDRAW',0


