;
;    PPP.ASM
;
;    Compile with FASM for Menuet
;    This program dials into an ISP and establishes a PPP connection
;
;    Version 11    26th January 2004
;
;    This code is a port of the PPP dialer program by Microchip, from
;    their application note AN724
;    It has been ported by Mike Hibbett mikeh@oceanfree.net for Menuet
;
;    26/1/04 - Mike Hibbett - added support for com port selected by
;                             stackcfg
;    2/5/03 - Shrirang - Added Abort Strings To sendwait to get out early
;                        if modem sends abort strings like NO CARRIER etc.
;
;    The original copyright statement follows
;//////////////////////////////////////////////////////////////////////////
;//
;//PING.C  version 1.10  July 29/99 (C)opyright by Microchip Technology Inc
;//
;//////////////////////////////////////////////////////////////////////////

FALSE               equ 0
TRUE                equ 1
DEBUG_OUTPUT        equ TRUE       ; If FALSE, not debugging info outputted
DEBUG_PPP_OUTPUT    equ TRUE        ; write PPP status msgs to debug board?
DEBUG_PORT2_OUTPUT  equ TRUE       ; write debug data also to com2


BAUD_9600       equ         12
BAUD_57600      equ         2
; The next line sets the baud rate of the connection to the modem
BAUDRATE        equ         BAUD_57600


LINE_END        equ         0x0D        ; End of input data character


; Defines for Internet constants
REQ         equ      1   ; Request options list for PPP negotiations
PPP_ACK     equ      2   ; Acknowledge options list for PPP negotiations
PPP_NAK     equ      3   ; Not acknowledged options list for PPP neg
REJ         equ      4   ; Reject options list for PPP negotiations
TERM            equ  5   ; Termination packet for LCP to close connectio
LCP_ECHO_REQ    equ  9
LCP_ECHO_REP    equ  10
IP        equ    0x0021   ; Internet Protocol packet
IPCP      equ    0x8021   ; Internet Protocol Configure Protocol packet
CCP       equ    0x80FD   ; Compression Configure Protocol packet
LCP       equ    0xC021   ; Link Configure Protocol packet
PAP       equ    0xC023   ; Password Authenication Protocol packet


MaxRx           equ         1500
MaxTx           equ         1500




use32

     org     0x0

     db      'MENUET00'      ; 8 byte id
     dd      38              ; required os
     dd      STARTAPP        ; program start
     dd      I_END           ; program image size
     dd      0x100000        ; required amount of memory
        ; esp = 0x7FFF0
     dd      0x00000000      ; reserved=no extended header


include "lang.inc"
include "macros.inc"
include "chat.inc"                  ; Hosts modem chatting routine


STARTAPP:
;    mov     eax, 0x3f8
;    mov     [comport], eax
;    mov     eax, 4
;    mov     [comirq], eax

    ; Get the com port & IRQ to use from the kernel stack config option

    mov   eax, 52                 ; Stack Interface
    mov   ebx, 0     ; read configuration word
    int   0x40
    mov  ecx, eax
    shr  ecx, 16     ; get the port address
    mov  [comport], ecx
    shr  eax, 8
    and  eax, 0x000000FF   ; get the irq
 mov  [comirq], eax


 mov  eax, [comport]
 add  eax, 0x01000000
 mov [irqtable], eax


    call    enable_port

appdsp:
    mov     eax, welcomep
    mov     [prompt], eax               ; set up prompt to display
    mov     al, [welcomep_len]
    mov     [prompt_len], al

    call    draw_window                 ; at first, draw the window


apploop:
    mov     eax, 23                     ; wait here for event
    mov     ebx, 20
    int     0x40

    cmp     eax, 1                      ; redraw request ?
    je      red
    cmp     eax, 2                      ; key in buffer ?
    je      key
    cmp     eax, 3                      ; button in buffer ?
    je      button
    mov     ebx, [comirq]
    add     ebx, 16
    cmp     eax, ebx
    je      flush_input                 ; Dont want serial data yet
    jmp     apploop

red:                                    ; redraw
    call    draw_window
    jmp     apploop

key:                                    ; key - ignore
    mov     eax, 2                      ; just read it
    int     0x40
    jmp     apploop

button:                                 ; button
    mov     eax, 17                     ; get id
    int     0x40

    cmp     ah, 1                       ; close program ?
    jne     noclose

    mov     esi, hangupWait
    mov     edi, hangupSend
    mov     edx, 1000                   ; Allow sendwait 10s
    call    sendwait

    call    disable_port

    mov     eax, -1                     ; close this program
    int     0x40
    jmp     apploop

noclose:
    cmp     ah, 2                       ; Dial Button pressed?
    jne     apploop

    mov     eax, conp
    mov     [prompt], eax               ; set up prompt to display
    mov     al,[conp_len]
    mov     [prompt_len], al
    call    draw_window

    jmp     dialloop                    ; connect to the host


    ; Data received, so get it, and throw it away at this point
flush_input:
    mov  eax,42
    mov  ebx, [comirq]
    int  0x40

    mov  eax,11                     ; This will return 0 most of the time
    int  0x40
    mov     ebx, [comirq]
    add     ebx, 16
    cmp     eax, ebx
   je   flush_input
    jmp  apploop


dialloop:
    call    modem_chat                  ; Try chatting with the modem

    cmp     eax, 1                      ; Did it work? ( = 1)
    jne     appdsp

    ; OK, we are now connected.

    mov     eax, pppOnp
    mov     [prompt], eax               ; set up prompt to display
    mov     al,[pppOnp_len]
    mov     [prompt_len], al
    call    draw_window

    mov     eax, 23
    mov     ebx, 100
    int     0x40                        ; wait for 1s to display message

    call    PPPStateMachine         ; This is the main code

    jmp     appdsp




;****************************************************************************
;    Function
;       PPPStateMachine
;
;   Description
;       Handles PPP link establishment
;
;****************************************************************************
PPPStateMachine:
    ; Start the timer
    xor     eax, eax
    call    settimer

PPPLoop:

    mov     eax, 11                 ; check event
    int     0x40
    cmp     eax, 3
    jne     PPPLred
        ; button pressed
    mov     eax, 17                     ; get id
    int     0x40


    mov     eax, hangp
    mov     [prompt], eax               ; set up prompt to display
    mov     al,[hangp_len]
    mov     [prompt_len], al
    call    draw_window

    mov     esi, hangupWait
    mov     edi, hangupSend
    mov     edx, 1000                   ; Allow sendwait 10s
    call    sendwait

    call    disable_port
    mov     eax, -1                     ; close this program
    int     0x40
    jmp     PPPLoop

PPPLred:
    cmp     eax, 1                      ; redraw request ?
    jne     PPPLoop0

    call    draw_window
    jmp     PPPLoop

PPPLoop0:
    mov     ebx, [comirq]
    add     ebx, 16
    cmp     eax, ebx
    jne     ppp_002                    ; check for tx to send


    ; we have data in the rx buffer, get it

    mov     eax, 42
    mov     ebx, [comirq]      ; ecx will return 0 =data read, 1 =no data
    int     0x40               ; or 2 =not irq owner

    inc     dword [rxbytes]

    cmp     bl, 0x7E
    jne     ppp_001a

    mov     eax, [rx_ptr]
    cmp     eax, 0
    jz      ppp_001
    mov     eax, [checksum1]
    cmp     eax, 0xf0b8
    jne     ppp_001


    movzx   eax, byte [rx_str + 3]
    mov     ah, [rx_str + 2]
    mov     [packet], eax

ppp_001:
    mov     eax, [extended]
    and     eax, 0x7e
    mov     [extended], eax
    xor     eax, eax
    mov     [rx_ptr], eax

    mov     eax, 0xffff
    mov     [checksum1], eax
    jmp     ppp_003

ppp_001a:
    cmp     bl, 0x7D
    jne     ppp_001b

    mov     eax, [extended]
    or      eax, 0x01
    mov     [extended], eax
    jmp     ppp_003

ppp_001b:
    mov     eax, [extended]
    test    eax, 0x01
    jz      ppp_001c

    xor     bl, 0x20
    and     eax, 0xFE
    mov     [extended], eax

ppp_001c:
    mov     edx, [rx_ptr]
    cmp     edx, 0
    jnz     ppp_001d
    cmp     bl, 0xff
    je      ppp_001d

    mov     [rx_str + edx], byte 0xff
    inc     edx

ppp_001d:
    cmp     edx, 1
    jnz     ppp_001e
    cmp     bl, 0x03
    je      ppp_001e

    mov     [rx_str + edx], byte 0x03
    inc     edx

ppp_001e:
    cmp     edx, 2
    jnz     ppp_001f
    test    bl, 0x01
    jz      ppp_001f

    mov     [rx_str + edx], byte 0
    inc     edx

ppp_001f:
    mov     [rx_str + edx], bl
    inc     edx
    mov     [rx_ptr], edx

    cmp     edx, MaxRx
    jle     ppp_001g
    mov     edx, MaxRx
    mov     [rx_ptr], edx

ppp_001g:
    ; do checksum calc
    mov     eax, [checksum1]
    xor     bh, bh
    xor     ax, bx
    call    calc
    mov     ebx, [checksum1]
    and     ebx, 0xffff
    shr     ebx, 8
    xor     eax, ebx
    mov     [checksum1], eax
    jmp     ppp_003

ppp_002:
    mov     eax, [tx_end]
    cmp     eax, 0
    jz      ppp_003

    mov     ebx, [tx_ptr]
    mov     cl, [tx_str + ebx]

    cmp     ebx, eax
    jne     ppp_002a
    mov     [tx_end], dword 0
    mov     cl, '~'
    jmp     ppp_002d

ppp_002a:
    mov     eax, [extended]
    and     eax, 0x02
    jz      ppp_002b

    xor     cl, 0x20
    mov     eax, [extended]
    and     eax, 0xFD
    mov     [extended], eax
    inc     [tx_ptr]
    jmp     ppp_002d

ppp_002b:
    cmp     cl, 0x20
    jl      ppp_002b1
    cmp     cl, 0x7d
    je      ppp_002b1
    cmp     cl, 0x7e
    je      ppp_002b1
    jmp     ppp_002c

ppp_002b1:
    mov     eax, [extended]
    or      eax, 0x02
    mov     [extended], eax
    mov     cl, 0x7d
    jmp     ppp_002d

ppp_002c:
    mov     eax, [tx_ptr]
    cmp     eax, 0
    jnz     ppp_002c1
    mov     cl, '~'

ppp_002c1:
    inc     [tx_ptr]

ppp_002d:
    ; Test for tx ready.

    push    ecx

wait_txd2:
    mov     eax,43
    mov     ecx, [comport]
    add     ecx, 0x80000000 + 5
    int     0x40
    and     bl, 0x40
    cmp     bl, 0
    jz      wait_txd2                  ; loop until free

    pop     ebx


    ; send the character

    inc     dword [txbytes]

    mov     ecx, [comport]
    mov     eax, 43
    int     0x40

ppp_003:
    mov     eax, [packet]
    cmp     eax, LCP
    jne     ppp_004

    mov     al, [rx_str + 4]
    cmp     al, REQ
    jne     ppp_003b

    ; Debugging output to debug board
    pusha
    mov     esi, RX_LCP_REQ
    call    debug_output
    popa

    mov     eax, [state]
    and     eax, 0xfd
    mov     [state], eax

    mov     ebx, 0xc6
    push    eax
    call    TestOptions
    pop     eax
    cmp     edx, 0
    jz      ppp_003g

    cmp     edx, 1
    jle     ppp_003h

    mov     edx, PPP_ACK
    cmp     eax, 3
    jge     ppp_003i

    or      eax, 0x02
    mov     [state], eax
    jmp     ppp_003i

ppp_003h:
    mov     bl, 0xc0
    mov     [rx_str + 10], bl
    mov     edx, PPP_NAK
    jmp     ppp_003i

ppp_003g:
    mov     edx, REJ

ppp_003i:

    mov     ebx, LCP
    mov     ecx, edx
    movzx   edx, byte [rx_str + 5]
    mov     esi, rx_str + 7
    call    MakePacket

    mov     eax, 0
    call    settimer
    jmp     ppp_003a

ppp_003b:
    cmp     al, PPP_ACK
    jne     ppp_003c

    ; Debugging output to debug board
    pusha
    mov     esi, RX_LCP_ACK
    call    debug_output
    popa

    mov     eax, [number]
    cmp     al, [rx_str+5]
    jne     ppp_003a

    mov     eax, [state]
    cmp     eax, 3
    jge     ppp_003a
    or      eax, 0x01
    mov     [state], eax
    jmp     ppp_003a

ppp_003c:
    cmp     al, PPP_NAK
    jne     ppp_003d

    ; Debugging output to debug board
    pusha
    mov     esi, RX_LCP_NAK
    call    debug_output
    popa

    mov     eax, [state]
    and     eax, 0xfe
    mov     [state], eax
    jmp     ppp_003a

ppp_003d:
    cmp     al, REJ
    jne     ppp_003e

    ; Debugging output to debug board
    pusha
    mov     esi, RX_LCP_REJ
    call    debug_output
    popa

    mov     eax, [state]
    and     eax, 0xfe
    mov     [state], eax
    jmp     ppp_003a

ppp_003e:
    cmp     al, TERM
    jne     ppp_003j
    jmp     ppp_003a

ppp_003j:
    cmp     al, LCP_ECHO_REQ
    jne     ppp_003a

    ; Debugging output to debug board
    pusha
    mov     esi, RX_LCP_ECHO_REQ
    call    debug_output
    popa

    mov     al, 0
    mov     [rx_str+8],al
    mov     [rx_str+9],al
    mov     [rx_str+10],al
    mov     [rx_str+11],al

    mov     ebx, LCP
    mov     ecx, LCP_ECHO_REP
    movzx   edx, byte [rx_str + 5]
    mov     esi, rx_str + 7
    call    MakePacket

ppp_003a:
    mov     eax, [state]
    cmp     eax, 3
    jne     ppp_013

    mov     eax, 4
    mov     [state], eax

    jmp     ppp_013


ppp_004:
    cmp     eax, PAP
    jne     ppp_005

    mov     al, [rx_str + 4]
    cmp     al, PPP_ACK
    jne     ppp_013

    ; Debugging output to debug board
    pusha
    mov     esi, RX_PAP_ACK
    call    debug_output
    popa

    mov     eax, 5
    mov     [state],eax
    jmp     ppp_013

ppp_005:
    cmp     eax, IPCP
    jne     ppp_006

    mov     al, [rx_str + 4]
    cmp     al, REQ
    jne     ppp_005a

    ; Debugging output to debug board
    pusha
    mov     esi, RX_IPCP_REQ
    call    debug_output
    popa

    mov     ebx, 0x04
    call    TestOptions
    cmp     edx, 0
    jz      ppp_005b
    mov     ecx, PPP_ACK
    mov     eax, 6
    mov     [state], eax
    jmp     ppp_005c
ppp_005b:
    mov     ecx, REJ

ppp_005c:
    mov     ebx, IPCP
    movzx   edx, byte [rx_str + 5]
    mov     esi, rx_str + 7
    call    MakePacket
    jmp     ppp_013

ppp_005a:
    cmp     al, PPP_ACK
    jne     ppp_005d

    ; Debugging output to debug board
    pusha
    mov     esi, RX_IPCP_ACK
    call    debug_output
    popa

    mov     al, [rx_str + 5]
    mov     ecx, [number]
    cmp     al, cl
    jne     ppp_013

    mov     eax, 7
    mov     [state], eax
    mov     eax, 5800
    call    settimer

    mov     eax, IPOnp
    mov     [prompt], eax               ; set up prompt to display
    mov     al,[IPOnp_len]
    mov     [prompt_len], al
    call    draw_window

    jmp     ppp_013

ppp_005d:
    cmp     al, PPP_NAK
    jne     ppp_005e

    ; Debugging output to debug board
    pusha
    mov     esi, RX_IPCP_NAK
    call    debug_output
    popa

    mov     al, [rx_str + 10]
    mov     [addr1], al
    mov     al, [rx_str + 11]
    mov     [addr2], al
    mov     al, [rx_str + 12]
    mov     [addr3], al
    mov     al, [rx_str + 13]
    mov     [addr4], al

    pusha
    call    draw_window

    mov     eax,52
    mov     ebx,3
    mov     cl, [addr4]
    shl     ecx, 8
    mov     cl, [addr3]
    shl     ecx, 8
    mov     cl, [addr2]
    shl     ecx, 8
    mov     cl, [addr1]
    int     0x40                       ; Set the stacks IP address

    popa

    mov     ebx, IPCP  ;; added 28/4/03
    mov     ecx, REQ
    movzx   edx, byte [rx_str + 5]
    mov     esi, rx_str + 7
    call    MakePacket
    jmp     ppp_013

ppp_005e:
    cmp     al, REJ
    jne     ppp_005f
    jmp     ppp_013

ppp_005f:
    cmp     al, TERM
    jne     ppp_013
    jmp     ppp_013

ppp_006:
    cmp     eax, IP
    jne     ppp_007


;;
;;
;;
;; This is where we will pass the IP packet up to the stack
;;
;;
;;
    mov     eax, 52
    mov     ebx, 6
    mov     edx, 1500   ; this should be exact amount
    mov     esi, rx_str + 4
    int     0x40

    ; Debugging output to debug board
    pusha
    mov     esi, RX_IP
    call    debug_output
    popa

    jmp     ppp_013

ppp_007:
    cmp     eax, CCP
    jne     ppp_008

    mov     al, [rx_str + 4]
    cmp     al, REQ
    jne     ppp_013

    ; Debugging output to debug board
    pusha
    mov     esi, RX_CCP_REQ
    call    debug_output
    popa

    mov     ebx, 0x04
    call    TestOptions
    cmp     edx, 0
    jz      ppp_007b
    mov     ecx, PPP_ACK
    jmp     ppp_007c
ppp_007b:
    mov     ecx, REJ

ppp_007c:
    mov     ebx, CCP
    movzx   edx, byte [rx_str + 5]
    mov     esi, rx_str + 7
    call    MakePacket

    jmp     ppp_013

ppp_008:
    cmp     eax, 0
    jz      ppp_009

    jmp     ppp_013

ppp_009:
    mov     eax, [tx_end]
    cmp     eax, 0
    jnz     ppp_010
    call    gettimer
    cmp     eax, 100
    jle     ppp_010

    mov     eax, [state]
    cmp     eax, 0
    je      ppp_009a
    cmp     eax, 2
    jne     ppp_010

ppp_009a:

    ; Debugging output to debug board
    pusha
    mov     esi, TX_LCP_REQ
    call    debug_output
    popa

    inc     [number]
    mov     eax, 0
    call    settimer

    mov     ebx, LCP
    mov     ecx, REQ
    mov     edx, [number]
    mov     esi, LCPREQStr
    call    MakePacket

    jmp     ppp_013

ppp_010:
    mov     eax, [tx_end]
    cmp     eax, 0
    jnz     ppp_011
    call    gettimer
    cmp     eax, 100
    jle     ppp_011
    mov     eax, [state]
    cmp     eax, 4
    jne     ppp_011
    mov     eax, 0
    call    settimer
    inc     [number]

    ; Debugging output to debug board
    pusha
    mov     esi, TX_PAP_REQ
    call    debug_output
    popa

    mov     ebx, PAP
    mov     ecx, REQ
    mov     edx, [number]
    mov     esi, PAPREQStr
    call    MakePacket

    jmp     ppp_013

ppp_011:
    mov     eax, [tx_end]
    cmp     eax, 0
    jnz     ppp_012
    call    gettimer
    cmp     eax, 100
    jle     ppp_012
    mov     eax, [state]
    cmp     eax, 6
    jne     ppp_012
    inc     [number]
    mov     eax, 0
    call    settimer

    ; Debugging output to debug board
    pusha
    mov     esi, TX_IPCP_REQ
    call    debug_output
    popa

    mov     ebx, IPCP
    mov     ecx, REQ
    mov     edx, [number]
    mov     esi, IPCPREQStr
    call    MakePacket

    jmp     ppp_013

ppp_012:
    mov     eax, [tx_end]
    cmp     eax, 0
    jnz     ppp_013
    mov     eax, [state]
    cmp     eax, 7
    jne     ppp_013

    ; 10ms Delay suggested by Ville
    mov     eax,23     ; over here
    mov     ebx,1
    int     0x40



    call    gettimer
    cmp     eax, 200
    jle     ppp_012a

 ; every 2s, when things are quiet, redraw window
    call    draw_window_limited

    mov     eax, 0
    call    settimer

ppp_012a:

    mov     eax, 52
    mov     ebx, 8
    mov     esi, ip_buff
    int     0x40

    cmp     eax, 0
    je      ppp_013

    call    MakeIPPacket

    ; Debugging output to debug board
    pusha
    mov     esi, TX_IP
    call    debug_output
    popa

ppp_013:
    mov     eax, [packet]
    cmp     eax, 0
    jz      PPPLoop

    mov     eax, 0
    mov     [packet], eax

    mov     edi, rx_str
    mov     ecx, MaxRx + 1
    rep     stosb
    jmp     PPPLoop



;****************************************************************************
;    Function
;       calc
;
;   Description
;       Adds a character to the CRC checksum
;       byte in lsb of eax
;
;
;****************************************************************************
calc:
    and     eax, 0xFF
    push    ecx
    mov     ecx, 8
calc_001:
    test    al, 0x01
    jz      calc_002
    shr     eax, 1
    xor     eax, 0x8408
    and     eax, 0xffff
    jmp     calc_003
calc_002:
    shr     eax, 1
calc_003:
    loop    calc_001
    pop     ecx
    ret


;****************************************************************************
;    Function
;       add2tx
;
;   Description
;       Adds a character into the tx buffer
;       byte in low byte of eax
;
;
;****************************************************************************
add2tx:
    pusha
    mov     esi, tx_str
    add     esi, [tx_ptr]
    inc     [tx_ptr]
    mov     [esi], al               ; Save byte in buffer
    mov     ecx, [checksum2]
    and     eax, 0xff
    xor     eax, ecx
    call    calc
    shr     ecx, 8
    and     ecx, 0xff
    xor     eax, ecx
    mov     [checksum2], eax
    popa
    ret



;****************************************************************************
;    Function
;       MakeIPPacket
;
;   Description
;       Creates a PPP packet for transmission to the host from the
;       IP packet extracted from the stack
;
;       IP data is in ip_buff
;
;****************************************************************************
MakeIPPacket:
    mov     [tx_ptr], dword 1
    mov     edi, tx_str
    mov     [edi], byte ' '
    mov     eax, 0xffff
    mov     [checksum2], eax
    mov     al, 0xff
    call    add2tx
    mov     al, 3
    call    add2tx
    mov     al, IP / 256
    call    add2tx
    mov     al, IP
    call    add2tx

    movzx   ecx, byte [ip_buff + 3]
    mov     ch, byte [ip_buff + 2]

    mov     esi, ip_buff

mip001:
    mov     al, byte [esi]
    call    add2tx
    inc     esi
    loop    mip001

    mov     eax, [checksum2]
    not     eax
    call    add2tx
    shr     eax, 8
    call    add2tx

    mov     eax, [tx_ptr]
    mov     [tx_end], eax
    xor     eax, eax
    mov     [tx_ptr], eax
    ret


;****************************************************************************
;    Function
;       MakePacket
;
;   Description
;       Creates a PPP packet for transmission to the host
;
;       Packet type in ebx
;       Code is in ecx
;       num is in edx
;       str is pointed to by esi
;
;****************************************************************************
MakePacket:
    mov     [tx_ptr], dword 1
    mov     edi, tx_str
    mov     [edi], byte ' '
    mov     eax, 0xffff
    mov     [checksum2], eax
    mov     al, 0xff
    call    add2tx
    mov     al, 3
    call    add2tx
    mov     al, bh                  ; packet/256
    call    add2tx
    mov     al, bl                  ; packet&255
    call    add2tx

    cmp     ebx, IP                 ; is packet type IP?
    jne     mp_001                  ; No - its a lower layer packet

    ; Do IP packet assembly

    jmp     mp_002

mp_001:
    ; Do PPP layer packet assembly
    mov     al, cl
    call    add2tx
    mov     al, dl
    call    add2tx
    mov     al, 0
    call    add2tx

    movzx   ecx, byte [esi]         ; length = *str - 3
    sub     ecx, 3

mp_002:                             ; Now copy the data acros
    mov     al, byte [esi]
    call    add2tx
    inc     esi
    loop    mp_002

    mov     eax, [checksum2]
    not     eax
    call    add2tx
    shr     eax, 8
    call    add2tx

    mov     eax, [tx_ptr]
    mov     [tx_end], eax
    xor     eax, eax
    mov     [tx_ptr], eax
    ret


;****************************************************************************
;    Function
;       TestOptions
;
;   Description
;       Test a PPP packets options fields for valid entries
;
;       option ebx
;
;       Returns result in edx, but may also modify rx_str
;
;****************************************************************************
TestOptions:
    mov     esi, 8                  ; ptr1
    mov     edi, 8                  ; ptr2
    mov     edx, 3                  ; edx is the return value
    movzx   ecx, byte [rx_str + 7]
    add     ecx, 4                  ; ecx is size
    cmp     ecx, MaxRx
    jle     to_001
    mov     ecx, MaxRx
to_001:
    cmp     esi, ecx
    jge      to_002
    mov     al, byte [esi + rx_str]
    cmp     al, 3
    jne     to_001a
    mov     al, byte [esi + rx_str + 2]
    cmp     al, 0x80
    je      to_001a
    ; bug fix for chap authenticate reject below
    mov     al, byte [esi + rx_str + 2]
    cmp     al, 0xc2
    jne     to_001a
    and     edx, 0xfd
to_001a:
    push    ecx
    mov     cl, [esi + rx_str]
    dec     cl
    mov     eax, 1
    shl     eax, cl
    and     eax, ebx
    and     eax, 0xffff
    pop     ecx
    cmp     eax, 0
    jnz     to_001b
    xor     edx,edx
to_001b:
    movzx   eax, byte [esi+rx_str+1]
    add     esi, eax
    jmp     to_001
to_002:
    ; if (!(pass&2))...
    test    edx, 2
    jnz     to_exit
    test    edx, 1
    jz      to_002a
    mov     ebx, 0xFFFB
to_002a:
    mov     esi, 8
to_002b:                            ; for loop
    cmp     esi, ecx
    jge      to_003

    push    ecx
    mov     cl, [esi + rx_str]
    dec     cl
    mov     eax, 1
    shl     eax, cl
    and     eax, ebx
    and     eax, 0xffff
    pop     ecx
    cmp     eax, 0
    jnz     to_002c
    movzx   edx, byte [esi+rx_str+1]
to_002d:
    cmp     esi, ecx
    jge      to_002b
    cmp     edx, 0
    jz      to_002b
    mov     al, [esi + rx_str]
    mov     [edi + rx_str], al
    inc     esi
    inc     edi
    dec     edx
    jmp     to_002d
to_002c:
    movzx   eax, byte [esi+rx_str+1]
    add     esi, eax
    jmp     to_002b                 ; end of for loop
to_003:
    mov     eax, edi
    sub     al, 4
    mov     [rx_str+7], al
    xor     edx, edx
    cmp     ebx, 0xfffb
    jne     to_exit
    inc     edx
to_exit:
    ; Return value in EDX
    ret



;***************************************************************************
;    Function
;        disable_port
;
;   Description;
;       Releases this applications use of the com port
;
;***************************************************************************
disable_port:
if DEBUG_PORT2_OUTPUT = TRUE
    mov      eax, 46                 ; free port area
    mov      ebx, 1

    mov      ecx, 0x2f8
    and      ecx, 0xFF0
    mov      edx, ecx
    or       edx, 0x00F
    int      0x40
end if

    mov      eax, 45                 ; free irq 4
    mov      ebx, 1
    mov      ecx, [comirq]
    int      0x40

    mov      eax, 46                 ; free port area
    mov      ebx, 1

    mov      ecx, [comport]
    and      ecx, 0xFF0
    mov      edx, ecx
    or       edx, 0x00F
    int      0x40
    ret



;***************************************************************************
;    Function
;        enable_port
;
;   Description;
;    Takes control of the com port, defining the IRQ table and initialising
;     the uart chip.
;
;***************************************************************************
enable_port:
    pusha
if DEBUG_PORT2_OUTPUT = TRUE
    mov      eax, 46
    mov      ebx, 0
    mov      ecx, 0x2f8
    and      ecx, 0xFF0
    mov      edx, ecx
    or       edx, 0x00F
    int      0x40                     ; reseve port memory to this process

    mov      eax, 45                  ; reserve irq 3
    mov      ebx, 0
    mov      ecx, 3
    int      0x40


    mov      ecx, 0x2f8             ; data format register
    add      ecx, 3
    mov      bl, 0x80               ; enable access to divisor latch
    mov      eax, 43                ; send data to device - com port setup
    int      0x40

    mov      ecx, 0x2f8          ; interrupt enable register
    inc      ecx
    mov      bl, 0                    ; No interruts enabled
    mov      eax, 43                  ; send data to device (modem)
    int      0x40

    mov      ecx, 0x2f8                 ; Divisor latch LSB
    mov      bl, BAUDRATE             ; set baud rate
    mov      eax, 43                  ; send data to device (modem)
    int      0x40

    mov      ecx, 0x2f8             ; Data format register
    add      ecx, 3
    mov      bl, 3                    ; 8 data bits
    mov      eax, 43                  ; send data to device (modem)
    int      0x40

    mov      ecx, 0x2f8        ; Modem control register
    add      ecx, 4                ; ** bl must be 0x0b for modem to dial!
    mov      bl, 0x0b              ; 0x08 -> out2 enabled. No handshaking.
       ; 0xb ->  out2 enabled, RTS/DTR enabled
    mov      eax, 43               ; send data to device (modem)
    int      0x40

;    mov      ecx, 0x2f8        ; interrupt enable register
;    inc      ecx
;    mov      bl, 1                 ; rx data interrupt enabled, othrs not
;    mov      eax, 43               ; send data to device (modem)
;    int      0x40

end if

    mov      eax, 46
    mov      ebx, 0
    mov      ecx, [comport]
    and      ecx, 0xFF0
    mov      edx, ecx
    or       edx, 0x00F
    int      0x40                     ; reseve port memory to this process

    mov      eax, 45                  ; reserve irq 4
    mov      ebx, 0
    mov      ecx, [comirq]
    int      0x40

    mov      eax, 44                  ; setup irq table
    mov      ebx, irqtable
    mov      ecx, [comirq]
    int      0x40

    mov      ecx, [comport]             ; data format register
    add      ecx, 3
    mov      bl, 0x80               ; enable access to divisor latch
    mov      eax, 43                ; send data to device - com port setup
    int      0x40

    mov      ecx, [comport]          ; interrupt enable register
    inc      ecx
    mov      bl, 0                    ; No interruts enabled
    mov      eax, 43                  ; send data to device (modem)
    int      0x40

    mov      ecx, [comport]                 ; Divisor latch LSB
    mov      bl, BAUDRATE             ; set baud rate
    mov      eax, 43                  ; send data to device (modem)
    int      0x40

    mov      ecx, [comport]             ; Data format register
    add      ecx, 3
    mov      bl, 3                    ; 8 data bits
    mov      eax, 43                  ; send data to device (modem)
    int      0x40

    mov      ecx, [comport]        ; Modem control register
    add      ecx, 4                ; ** bl must be 0x0b for modem to dial!
    mov      bl, 0x0b              ; 0x08 -> out2 enabled. No handshaking.
       ; 0xb ->  out2 enabled, RTS/DTR enabled
    mov      eax, 43               ; send data to device (modem)
    int      0x40

    mov      ecx, [comport]        ; interrupt enable register
    inc      ecx
    mov      bl, 1                 ; rx data interrupt enabled, othrs not
    mov      eax, 43               ; send data to device (modem)
    int      0x40

    mov      ecx, [comirq]
    add      ecx, 16
    mov      ebx, 1
    shl      ebx, cl
    add      ebx, 111b
    mov      eax,40                  ; enable irq 4 data
    int      0x40

    popa
    ret



;**************************************************************************
;    Function
;        draw_window
;
;   Description;
;       Normal window definition and text layout for application
;**************************************************************************
draw_window:
    mov      eax, 12                 ; function 12:tell os about windowdraw
    mov      ebx, 1                  ; 1, start of draw
    int      0x40
         ; DRAW WINDOW
    mov      eax, 0                  ; function 0 : define and draw window
    mov      ebx, 100*65536+250      ; [x start] *65536 + [x size]
    mov      ecx, 100*65536+150      ; [y start] *65536 + [y size]
    mov      edx,0x03224466            ; color of work area RRGGBB
    mov      esi,0x00334455            ; color of grab bar  RRGGBB
    mov      edi,0x00ddeeff            ; color of frames    RRGGBB
    int      0x40
         ; WINDOW LABEL
    mov      eax, 4                  ; function 4 : write text to window
    mov      ebx, 8*65536+8          ; [x start] *65536 + [y start]
    mov      ecx, 0x00ffffff         ; color of text RRGGBB
    mov      edx, labelt             ; pointer to text beginning
    mov      esi, labellen-labelt    ; text length
    int      0x40
       ; DIAL BUTTON
    mov      eax, 8                  ; function 8 : define and draw button
    mov      ebx, (50)*65536+40      ; [x start] *65536 + [x size]
    mov      ecx, 130*65536+12       ; [y start] *65536 + [y size]
    mov      edx, 2                  ; button id
    mov      esi, 0x5599cc           ; button color RRGGBB
    int      0x40

    mov      ebx, 55*65536+133       ; Draw button text
    mov      ecx, 0x00FFFFFF
    mov      edx, button1_text
    xor      eax, eax
    mov      al,  [button1_text_len]
    mov      esi, eax
    mov      eax, 4
    int      0x40
      ; DISCONNECT BUTTON
    mov      eax, 8                  ; function 8 : define and draw button
    mov      ebx, (150)*65536+65     ; [x start] *65536 + [x size]
    mov      ecx, 130*65536+12       ; [y start] *65536 + [y size]
    mov      edx, 3                  ; button id
    mov      esi, 0x5599cc           ; button color RRGGBB
    int      0x40

    mov      ebx, 155*65536+133      ; Draw button text
    mov      ecx, 0x00FFFFFF
    mov      edx, button3_text
    xor      eax, eax
    mov      al,  [button3_text_len]
    mov      esi, eax
    mov      eax, 4
    int      0x40

    mov      ebx, 5*65536+40         ; draw info text with function 4
    mov      ecx, 0x00FFFFFF
    mov      edx, [prompt]
    xor      eax, eax
    mov      al,  [prompt_len]
    mov      esi, eax
    mov      eax, 4
    int      0x40

    ; Draw IP address
    mov      edx, 10*65536+60
    mov      esi, 0x00FFFFFF
    mov      ebx, 0x00030000
    movzx    ecx, byte [addr1]
    mov      eax, 47
    int      0x40
    mov      edx, 31*65536+60
    mov      esi, 0x00FFFFFF
    mov      ebx, 0x00030000
    movzx    ecx, byte [addr2]
    mov      eax, 47
    int      0x40
    mov      edx, 52*65536+60
    mov      esi, 0x00FFFFFF
    mov      ebx, 0x00030000
    movzx    ecx, byte [addr3]
    mov      eax, 47
    int      0x40
    mov      edx, 73*65536+60
    mov      esi, 0x00FFFFFF
    mov      ebx, 0x00030000
    movzx    ecx, byte [addr4]
    mov      eax, 47
    int      0x40

    ; Status byte
    mov      edx, 100*65536+60
    mov      esi, 0x00FFFFFF
    mov      ebx, 0x00010000
    movzx    ecx, byte [state]
    mov      eax, 47
    int      0x40

    ; bytes sent / received
    mov      eax, 4                  ; function 4 : write text to window
    mov      ebx, 10*65536+80          ; [x start] *65536 + [y start]
    mov      ecx, 0x00ffffff         ; color of text RRGGBB
    mov      edx, txmsg              ; pointer to text beginning
    mov      esi, txmsglen-txmsg    ; text length
    int      0x40

    mov      eax, 4                  ; function 4 : write text to window
    mov      ebx, 10*65536+100          ; [x start] *65536 + [y start]
    mov      ecx, 0x00ffffff         ; color of text RRGGBB
    mov      edx, rxmsg              ; pointer to text beginning
    mov      esi, rxmsglen-rxmsg    ; text length
    int      0x40

    call    draw_window_limited

    mov      eax, 12                 ; end of redraw
    mov      ebx, 2
    int      0x40

    ret



draw_window_limited:
    mov     eax,13
    mov     ebx,80*65536+10*6
    mov     ecx,80*65536+10
    mov     edx,0x03224466
    int     0x40
    mov     eax,13
    mov     ebx,80*65536+10*6
    mov     ecx,100*65536+10
    mov     edx,0x03224466
    int     0x40

    mov     ebx, 0x000A0000
    mov     ecx, [txbytes]
    mov     esi, 0x00ffffff         ; color of text RRGGBB
    mov     eax, 47                  ; function 47 : write number to window
    mov     edx, 80*65536+80          ; [x start] *65536 + [y start]
    int     0x40

    mov     ebx, 0x000A0000
    mov     ecx, [rxbytes]
    mov     esi, 0x00ffffff         ; color of text RRGGBB
    mov     eax, 47                  ; function 47 : write number to window
    mov     edx, 80*65536+100          ; [x start] *65536 + [y start]
    int     0x40
    ret


;****************************************************************************
;    Function
;       settimer
;
;   Description
;       sets the general purpose timer to a given value in eax
;       All times are in 1/100s
;
;
;****************************************************************************
settimer:
    push    eax
    mov     eax, 26
    mov     ebx, 9
    int     0x40        ; get 100th second counter
    pop     ebx
    sub     eax, ebx    ; This could have some funny side effecs if PPP
   ; called within ebx seconds of startup
    mov     [timerValue], eax
    ret


;****************************************************************************
;    Function
;       gettimer
;
;   Description
;       gets the general purpose timer count in eax
;       All times are in 1/100s
;
;
;****************************************************************************
gettimer:
    mov     eax, 26
    mov     ebx, 9
    int     0x40        ; get 100th second counter

    sub     eax, [timerValue]
    ret




;****************************************************************************
;    Function
;       sendwait
;
;   Description
;       Sends a command string to the modem, then waits for a defined rsp
;
;       esi points to string to wait for
;       edi points to string to send
;       edx holds wait time, in ms
;
;       Returns 1 if OK or 0 if timeout occurred
;
;****************************************************************************
sendwait:
    mov     [sendwaitTime], edx

    ; Shrirang 2/5/03
    mov     byte [abortcnt], 0      ; reset the abort counter
    ;--!

    ; Start the timer
    xor     eax, eax
    call    settimer

    ; Check for incoming data

    xor     edx, edx
    xor     eax, eax

sw_001:
    push    eax
    push    edx

    ; Has connection timer expired?
    call    gettimer
    cmp     eax, [sendwaitTime]
    jl      sw_000

    pop     edx
    pop     eax

    xor     eax, eax

    jmp     sw_exit                 ; Exit indicating an error ( timeout )

sw_000:
    ; any data from modem?

    mov     eax,11                     ; This will return 0 most of the time
    int     0x40
    mov     ecx, eax
    pop     edx
    pop     eax


    cmp     ecx, 1                      ; redraw request ?
    je      red1
    cmp     ecx, 3                      ; button in buffer ?
    je      button1
    mov     ebx, [comirq]
    add     ebx, 16
    cmp     ecx,ebx
    jne     sw_002
    jmp     sw_000a

red1:
    pusha
    call    draw_window
    popa
    push    eax
    push    edx

    jmp     sw_000

button1:
    mov     eax, 0
    jmp     sw_exit

sw_000a:
    ; there was data, so get it

    push    edx
    push    eax
    mov     eax,42
    mov     ebx, [comirq]
    int     0x40
    pop     eax
    pop     edx


    ; Shrirang 2/5/03
    ; Now that the expected response is not got we check if we
    ; got the abort part, before we reset the fsm

    cmp     bl, 0x0d     ; AT commands normally start and end with \r\n
    je      checkabort

    cmp     bl, 0x0a
    je      checkabort

    push    eax
    xor     eax, eax
    mov     al, [abortcnt]
    mov     byte [abortres+eax], bl         ; update abort response
    inc     byte [abortcnt]
    pop     eax

    jmp     noabort


checkabort :

    cmp     byte [abortcnt], 2  ; if we got valid abort this cannot happen!
    jbe     noabortflush

    push    eax
    push    esi
    push    edi
    push    ecx

    mov     esi, abortres
    mov     edi, aborts
    xor     ecx, ecx
    mov     cl, byte [abortcnt]
    call    scanaborts                       ; scan 'em

    pop     ecx
    pop     edi
    pop     esi

    and     eax, eax
    jz      noabortdec

    pop     eax
    xor     eax, eax
    jmp     sw_exit

noabortdec:

    pop     eax

noabortflush:

    mov byte [abortcnt], 0

noabort:

;--!

    cmp     [esi+edx], bl
    je      sw_003


    xor     edx, edx

    ; Added 28/4/03
    cmp     [esi+edx], bl
    je      sw_003

    jmp     sw_001

sw_003:                             ; They are the same
    inc     edx
    cmp     [esi+edx], byte 0
    jne     sw_001


    xor     eax, eax
    inc     eax
    jmp     sw_exit

sw_002:
    ; Test for data to send to modem
    cmp     [ edi + eax ], byte 0
    je      sw_001

    ; Is it a '|' character?
    cmp     [ edi + eax ], byte '|'
    jne     sw_004

    push    eax
    call    gettimer
    cmp     eax, 100
    pop     eax
    jl      sw_001

    ; restart the timer
    push    eax
    xor     eax, eax
    call    settimer
    pop     eax

    ; Move to next character
    inc     eax
    jmp     sw_001


sw_004:
    push    edx
    push    eax

    ; restart the timer
    xor     eax, eax
    call    settimer

    ; Test for tx ready.
    ; OR, wait then send
    push    edi
    mov     eax, 5
    mov     ebx, 1
    int     0x40        ; 10ms delay
    pop     edi

    ; send the character
    pop     eax
    mov     bl, [edi + eax]

    mov     ecx, [comport]
    inc     eax
    push    eax
    mov     eax, 43
    int     0x40

    pop     eax
    pop     edx

    cmp     [ edi + eax ], byte 0
    jne     sw_001

    cmp     [ esi + edx ], byte 0
    jne     sw_001

    xor     eax, eax
    inc     eax

sw_exit:
    ; return success (1) or failure (0) in eax
    ret




if DEBUG_OUTPUT = TRUE

;****************************************************************************
;    Function
;       debug_output
;
;   Description
;       prints a description of the PPP protocol's data exchanges to the
;       debug board
;
;       esi holds ptr to msg to display
;
;       Nothing preserved; I'm assuming a pusha/popa is done before calling
;
;****************************************************************************
debug_output:
    cmp     esi, RX_IP
    jne     do_001

    call    debug_print_string

    call    debug_print_rx_ip

    mov     esi, IP_DATA1
    call    debug_print_string
    mov     esi, CRLF
    call    debug_print_string
    mov     esi, IP_DATA2
    call    debug_print_string
    ret

do_001:
    cmp     esi, TX_IP
    jne     do_002

    call    debug_print_string

    call    debug_print_tx_ip

    mov     esi, IP_DATA1
    call    debug_print_string
    mov     esi, CRLF
    call    debug_print_string
    mov     esi, IP_DATA2
    call    debug_print_string
    ret

do_002:
    ; Print PPP protocol information
if DEBUG_PPP_OUTPUT = TRUE
    call    debug_print_string
    mov     esi, CRLF
    call    debug_print_string
end if
    ret



txCom2:
    push    ecx

wait_txd2t:
    mov     eax,43
    mov     ecx,0x80000000 + 0x2f8 + 5
    int     0x40
    and     bl, 0x40
    cmp     bl, 0
    jz      wait_txd2t                  ; loop until free

    pop     ebx


    ; send the character

    mov     ecx, 0x2f8
    mov     eax, 43
    int     0x40
    ret


;****************************************************************************
;    Function
;       debug_print_string
;
;   Description
;       prints a string to the debug board
;
;       esi holds ptr to msg to display
;
;       Nothing preserved; I'm assuming a pusha/popa is done before calling
;
;****************************************************************************
debug_print_string:
    mov     cl, [esi]
    cmp     cl, 0
    jnz     dps_001
    ret

dps_001:
if DEBUG_PORT2_OUTPUT = TRUE
    pusha
    call    txCom2
    popa
end if
    mov     eax,63
    mov     ebx, 1
    push    esi
    int 0x40
    pop     esi
    inc     esi
    jmp     debug_print_string


; This is used for translating hex to ASCII for display or output
hexchars db '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'

IP_DATA1    db 'TCP  From: xxxxxxxx To: xxxxxxxx SrcP: xxxx DestP: xxxx',0
IP_DATA2    db 'Seq: xxxxxxxx Ack: xxxxxxxx Flags: xx  dataLen: xxxx',13,10,0



debug_print_rx_ip:
    mov     esi, rx_str + 4     ; The ip buffer address start

    mov     edi, IP_DATA1

    cmp     [esi+9], byte 1
    jne     rnICMP
    mov     eax,'ICMP'
    jmp     drp
rnICMP:
    cmp     [esi+9], byte 6
    jne     rnTCP
    mov     eax,'TCP '
    jmp     drp
rnTCP:
    cmp     [esi+9], byte 17
    jne     rnUDP
    mov     eax,'UDP '
    jmp     drp
rnUDP:

drp:
    mov     [edi], eax

    call    fillData

    ret


debug_print_tx_ip:
    mov     esi, ip_buff     ; The ip buffer address start

    mov     edi, IP_DATA1

    cmp     [esi+9], byte 1
    jne     tnICMP
    mov     eax,'ICMP'
    jmp     dtp
tnICMP:
    cmp     [esi+9], byte 6
    jne     tnTCP
    mov     eax,'TCP '
    jmp     dtp
tnTCP:
    cmp     [esi+9], byte 17
    jne     tnUDP
    mov     eax,'UDP '
    jmp     dtp
tnUDP:

dtp:
    mov     [edi], eax

    call    fillData

    ret


fillData:
    ; Display from IP
    mov     cl, [esi+12]
    mov     edx, 11
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+13]
    mov     edx, 13
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+14]
    mov     edx, 15
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+15]
    mov     edx, 17
    call    wbyte               ; byte in cl, dest in edi+edx

    ; Display to IP
    mov     cl, [esi+16]
    mov     edx, 24
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+17]
    mov     edx, 26
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+18]
    mov     edx, 28
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+19]
    mov     edx, 30
    call    wbyte               ; byte in cl, dest in edi+edx

    ; Only display extra data for TCP
    cmp     [esi+9], byte 6     ; TCP?
    je      nTCP

    ; display source port
    mov     [edi+32], byte 0
    mov     edi, IP_DATA2
    mov     [edi], byte 0
    ret

nTCP:
    mov     [edi+32], byte ' '

    mov     cl, [esi+20]
    mov     edx, 39
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+21]
    mov     edx, 41
    call    wbyte               ; byte in cl, dest in edi+edx

    mov     cl, [esi+22]
    mov     edx, 51
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+23]
    mov     edx, 53
    call    wbyte               ; byte in cl, dest in edi+edx


    mov     edi, IP_DATA2
    mov     [edi], byte 'S'

    mov     cl, [esi+24]
    mov     edx, 5
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+25]
    mov     edx, 7
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+26]
    mov     edx, 9
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+27]
    mov     edx, 11
    call    wbyte               ; byte in cl, dest in edi+edx

    mov     cl, [esi+28]
    mov     edx, 19
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+29]
    mov     edx, 21
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+30]
    mov     edx, 23
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     cl, [esi+31]
    mov     edx, 25
    call    wbyte               ; byte in cl, dest in edi+edx

    mov     cl, [esi+33]
    and     cl, 0x3F
    mov     edx, 35
    call    wbyte               ; byte in cl, dest in edi+edx

    ; Display the size of the received packet
    mov     dh, [esi + 2]
    mov     dl, [esi + 3]
    sub     dx, 40
    mov     cl, dh
    mov     edx, 48
    call    wbyte               ; byte in cl, dest in edi+edx
    mov     dh, [esi + 2]
    mov     dl, [esi + 3]
    sub     dx, 40
    mov     cl, dl
    mov     edx, 50
    call    wbyte               ; byte in cl, dest in edi+edx


    ret


wbyte:  ; byte in cl, dest in edi+edx, edi unchanged
    xor     eax, eax
    mov     al, cl
    shr     al, 4
    mov     bl, [eax + hexchars]
    mov     [edi+edx], bl
    inc edx
    mov     al, cl
    and     al, 0x0f
    mov     bl, [eax + hexchars]
    mov     [edi+edx], bl
    ret

else
debug_output:
    ret
end if

; DATA AREA


; debug msgs
RX_IP               db  'R: ',0
TX_IP               db  'T: ',0
CRLF                db  13,10,0
RX_LCP_REQ          db  'RX_LCP_REQ',0
RX_LCP_ACK          db  'RX_LCP_ACK',0
RX_LCP_NAK          db  'RX_LCP_NAK',0
RX_LCP_REJ          db  'RX_LCP_REJ',0
RX_LCP_ECHO_REQ     db  'RX_LCP_ECHO_REQ',0
RX_PAP_ACK          db  'RX_PAP_ACK',0
RX_IPCP_REQ         db  'RX_IPCP_REQ',0
RX_IPCP_ACK         db  'RX_IPCP_ACK',0
RX_IPCP_NAK         db  'RX_IPCP_NAK ( IP Address assigned )',0
RX_CCP_REQ          db  'RX_CCP_REQ',0
TX_LCP_REQ          db  'TX_LCP_REQ',0
TX_PAP_REQ          db  'TX_PAP_REQ',0
TX_IPCP_REQ         db  'TX_IPCP_REQ',0


; Labels for GUI buttons
button1_text        db  'DIAL'
button1_text_len    db  4
button3_text        db  'DISCONNECT'
button3_text_len    db  10

comport             dd  0
comirq              dd  0

; Pointer to prompt shown to user
prompt              dd  0
prompt_len          db  0

; Application Title
labelt              db  'PPP Dialer'
labellen:

txmsg:              db  'Tx bytes :'
txmsglen:
rxmsg:              db  'Rx bytes :'
rxmsglen:

timerValue          dd  0
sendwaitTime        dd  0


; Prompts displayed to the user
welcomep            db  'Select an option below, see ppp.txt'
welcomep_len        db  35

dialfp              db  'Connect Failed...'
dialfp_len          db  17

connectedp          db  'Connected to Host'
connectedp_len      db  17

conp                db  'Connecting to Host'
conp_len            db  18

pppOnp              db  'PPP Started'
pppOnp_len          db  11

IPOnp               db  'IP Link established'
IPOnp_len           db  19

discp               db  'Disconnected from Host'
discp_len           db  22

hangp               db  'Hanging up Modem......'
hangp_len           db  22

PPPconSend          db  0x7e,0xff,0x7d,0x23,0x08,0x08,0x08,0x08,0
PPPconWait          db  '~~',0
hangupWait          db  'ATZ',0
hangupSend          db  '|||+++|||',10,13,'ATH',10,13,'|ATZ',10,13,0

; Shrirang 2/5/03

abortres:           times(50) db 0
abortcnt                      db 0

;--!

LCPREQStr db 0x0e,0x02,0x06,0x00, 0x0a, 0x00, 0x00, 0x07, 0x02, 0x08, 0x02
PAPREQStr           db 14, 4, 'free', 4, 'free'
IPCPREQStr          db 10, 3, 6, 0, 0, 0, 0

irqtable:           dd  0x3f8 + 0x01000000  ; read port 0x3f8, byte
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
      dd  0
checksum1           dd  0
checksum2           dd  0
packet              dd  0
state               dd  0
extended            dd  0
number              dd  0
tx_end              dd  0
tx_ptr              dd  0
rx_ptr              dd  0
addr1               db  0
addr2               db  0
addr3               db  0
addr4               db  0
rxbytes             dd  0
txbytes             dd  0


; End of application code and data marker

I_END:

rx_str:             rb MaxRx + 1
tx_str:             rb MaxTx + 1
ip_buff:            rb 1500

