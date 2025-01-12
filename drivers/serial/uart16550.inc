BASE_FREQ = 1843200
BASE_DIV = 16

THR_REG = 0 ; transtitter/reciever
IER_REG = 1 ; interrupt enable
IIR_REG = 2 ; interrupt info
FCR_REG = 2 ; FIFO control
LCR_REG = 3 ; line control
MCR_REG = 4 ; modem control
LSR_REG = 5 ; line status
MSR_REG = 6 ; modem status
SCR_REG = 7 ; scratch

DLL_REG = THR_REG ; divisor latch (LSB)
DLM_REG = IER_REG ; divisor latch (MSB)

LCR_5BIT   = 0x00
LCR_6BIT   = 0x01
LCR_7BIT   = 0x02
LCR_8BIT   = 0x03
LCR_STOP_1 = 0x00
LCR_STOP_2 = 0x04
LCR_PARITY = 0x08
LCR_EVEN   = 0x10
LCR_STICK  = 0x20
LCR_BREAK  = 0x40
LCR_DLAB   = 0x80

LSR_DR   = 0x01 ; data ready
LSR_OE   = 0x02 ; overrun error
LSR_PE   = 0x04 ; parity error
LSR_FE   = 0x08 ; framing error
LSR_BI   = 0x10 ; break interrupt
LSR_THRE = 0x20 ; transmitter holding empty
LSR_TEMT = 0x40 ; transmitter empty
LSR_FER  = 0x80 ; FIFO error

FCR_EFIFO   = 0x01 ; enable FIFO
FCR_CRB     = 0x02 ; clear reciever FIFO
FCR_CXMIT   = 0x04 ; clear transmitter FIFO
FCR_RDY     = 0x08 ; set RXRDY and TXRDY pins
FCR_FIFO_1  = 0x00 ; 1  byte trigger
FCR_FIFO_4  = 0x40 ; 4  bytes trigger
FCR_FIFO_8  = 0x80 ; 8  bytes trigger
FCR_FIFO_14 = 0xC0 ; 14 bytes trigger

IIR_INTR  = 0x01 ; 1= no interrupts
IIR_IID   = 0x0E ; interrupt source mask

IER_RDAI  = 0x01 ; reciever data interrupt
IER_THRI  = 0x02 ; transmitter empty interrupt
IER_LSI   = 0x04 ; line status interrupt
IER_MSI   = 0x08 ; modem status interrupt

MCR_DTR   = 0x01 ; 0-> DTR=1, 1-> DTR=0
MCR_RTS   = 0x02 ; 0-> RTS=1, 1-> RTS=0
MCR_OUT1  = 0x04 ; 0-> OUT1=1, 1-> OUT1=0
MCR_OUT2  = 0x08 ; 0-> OUT2=1, 1-> OUT2=0;  enable intr
MCR_LOOP  = 0x10 ; lopback mode

MSR_DCTS  = 0x01 ; delta clear to send
MSR_DDSR  = 0x02 ; delta data set redy
MSR_TERI  = 0x04 ; trailinh edge of ring
MSR_DDCD  = 0x08 ; delta carrier detect
MSR_CTS   = 0x10
MSR_DSR   = 0x20
MSR_RI    = 0x40
MSR_DCD   = 0x80

MCR_TEST_MASK = MCR_DTR or MCR_RTS or MCR_OUT1 or MCR_OUT2 or MCR_LOOP
MSR_CHECK_MASK = MSR_CTS or MSR_DSR or MSR_RI or MSR_DCD

struct  DRV_DATA
        io_addr         dd ? ; base address of io port
        port            dd ? ; serial port descriptor
ends

; dx = base io
; al = result
macro rd_reg reg
{
        push    edx
        add     dx, reg
        in      al, dx
        pop     edx
}

; dx = base io
; al = new value
macro wr_reg reg
{
        push    edx
        add     dx, reg
        out     dx, al
        pop     edx
}

; dx = port
; ax = divisor value
proc uart_set_baud
        push    eax
        rd_reg  LCR_REG
        or      al, LCR_DLAB
        wr_reg  LCR_REG
        pop     eax
        wr_reg  DLL_REG
        shr     ax, 8
        wr_reg  DLM_REG
        rd_reg  LCR_REG
        and     al, 0x7f
        wr_reg  LCR_REG
        ret
endp

proc uart_probe stdcall uses ebx esi edi, io_addr:dword, irqn:dword
        xor     ebx, ebx ; 0 = reserve
        mov     ecx, [io_addr]
        lea     edx, [ecx + 7]
        push    ebp
        invoke  ReservePortArea
        pop     ebp
        test    eax, eax
        jnz     .err

        mov     edx, [io_addr]

        ; enable loopback
        mov     al, MCR_LOOP
        wr_reg  MCR_REG

        ; read status
        rd_reg  MSR_REG
        and     al, MSR_CHECK_MASK
        test    al, al
        jnz     .free_port

        ; set test signals
        mov     al, MCR_TEST_MASK
        wr_reg  MCR_REG

        ; check signals
        rd_reg  MSR_REG
        and     al, MSR_CHECK_MASK
        cmp     al, MSR_CHECK_MASK
        jnz     .free_port

        DEBUGF  L_DBG, "uart16550: found serial port %x\n", [io_addr]

        ; initialize port
        xor     ax, ax
        wr_reg  MCR_REG
        wr_reg  IER_REG
        wr_reg  LCR_REG
        wr_reg  FCR_REG

        mov     eax, sizeof.DRV_DATA
        invoke  Kmalloc
        test    eax, eax
        jz      .free_port
        mov     edi, eax

        mov     eax, [io_addr]
        mov     [edi + DRV_DATA.io_addr], eax

        invoke  AttachIntHandler, [irqn], uart_int_handler, edi
        test    eax, eax
        jz      .free_desc

        ; register port
        lea     ecx, [uart_drv]
        mov     edx, edi
        call    add_port
        test    eax, eax
        jz      .free_desc   ; TODO detach_int_handler?

        ; save port handle
        mov     [edi + DRV_DATA.port], eax
        ret

  .free_desc:
        mov     eax, edi
        invoke  Kfree

  .free_port:
        xor     ebx, ebx
        inc     ebx ; 1 = release
        mov     ecx, [io_addr]
        lea     edx, [ecx + 7]
        push    ebp
        invoke  ReservePortArea
        pop     ebp

  .err:
        ret
endp

proc uart_int_handler c uses ebx esi edi, data:dword
locals
        .buf    db ?
endl
        mov     edi, [data]
        mov     edx, [edi + DRV_DATA.io_addr]
        xor     ebx, ebx

  .read_iir:
        rd_reg  IIR_REG
        test    al, IIR_INTR
        jnz     .exit

        inc     ebx
        and     ax, IIR_IID
        shr     ax, 1

        ; check source
        test    ax, ax
        jz      .modem
        cmp     ax, 1
        jz      .xmit
        cmp     ax, 2
        jz      .recv
        cmp     ax, 3
        jz      .status
        jmp     .exit

  .modem:
        ; read MSR for clear interrupt
        rd_reg  MSR_REG
        jmp     .read_iir

  .xmit:
        push    edx
        mov     eax, [edi + DRV_DATA.port]
        mov     edx, SERIAL_EVT_TXE
        mov     ecx, 1
        lea     esi, [.buf]
        call    handle_event
        pop     edx

        test    eax, eax
        jz      .no_data

        mov     al, [.buf]
        wr_reg  THR_REG
        jmp     .read_iir

  .no_data:
        ; disable THR empty interrupt
        rd_reg  IER_REG
        and     ax, not IER_THRI
        wr_reg  IER_REG
        jmp     .read_iir

  .recv:
        ; read byte
        rd_reg  THR_REG
        push    edx
        mov     [.buf], al
        mov     eax, [edi + DRV_DATA.port]
        mov     edx, SERIAL_EVT_RXNE
        mov     ecx, 1
        lea     esi, [.buf]
        call    handle_event
        pop     edx

        ; check for more recevied bytes
        rd_reg  LSR_REG
        test    al, LSR_DR
        jnz     .recv
        jmp     .read_iir

  .status:
        rd_reg  LSR_REG
        jmp     .read_iir

  .fifo:
        jmp     .read_iir

  .exit:
        xchg    eax, ebx
        ret
endp

proc uart_startup stdcall, data:dword, conf:dword
        DEBUGF  L_DBG, "uart16550: startup %x %x\n", [data], [conf]
        ; enable and reset fifo, 1 byte trigger level
        mov     ecx, [data]
        mov     edx, [ecx + DRV_DATA.io_addr]
        mov     ax, FCR_EFIFO or FCR_CRB or FCR_CXMIT
        wr_reg  FCR_REG
        ; configure at startup
        stdcall uart_reconf, [data], [conf]
        ret
endp

proc uart_shutdown stdcall, data:dword
        DEBUGF  L_DBG, "uart16550: shutdown %x\n", [data]
        ; disable interrupts
        mov     ecx, [data]
        mov     edx, [ecx + DRV_DATA.io_addr]
        xor     ax, ax
        wr_reg  IER_REG
        ret
endp

proc uart_reconf stdcall uses ebx esi, data:dword, conf:dword
locals
        divisor dw ?
        lcr     dw ?
endl
        ; calc divisor = BASE_FREQ / BASE_DIV / baudrate
        mov     esi, [conf]
        xor     edx, edx
        mov     eax, BASE_FREQ / BASE_DIV
        div     [esi + SP_CONF.baudrate]
        test    edx, edx
        jnz     .fail
        test    eax, eax
        jz      .fail
        mov     [divisor], ax

        ; calc word size
        xor     eax, eax
        mov     al, [esi + SP_CONF.word_size]
        cmp     al, 8
        ja      .fail
        sub     al, 5
        jb      .fail
        mov     [lcr], ax

        ; calc parity
        mov     al, [esi + SP_CONF.parity]
        xor     bx, bx
        cmp     al, SERIAL_CONF_PARITY_NONE
        je      .parity_ok
        or      bl, LCR_PARITY
        cmp     al, SERIAL_CONF_PARITY_ODD
        je      .parity_ok
        or      bl, LCR_EVEN
        cmp     al, SERIAL_CONF_PARITY_EVEN
        je      .parity_ok
        mov     bl, LCR_STICK or LCR_PARITY
        cmp     al, SERIAL_CONF_PARITY_MARK
        je      .parity_ok
        or      bl, LCR_EVEN
        cmp     al, SERIAL_CONF_PARITY_SPACE
        jne     .fail
  .parity_ok:
        mov     [lcr], bx

        ; calc  stop bits
        mov     bx, LCR_STOP_1
        mov     al, [esi + SP_CONF.stop_bits]
        cmp     al, 1
        je      .stop_bits_ok
        or      bx, LCR_STOP_2
        cmp     al, 2
        jne     .fail
  .stop_bits_ok:
        or      [lcr], bx

        mov     esi, [data]
        mov     edx, [esi + DRV_DATA.io_addr]

        spin_lock_irqsave
        rd_reg  IER_REG
        and     ax, IER_RDAI or IER_LSI
        wr_reg  IER_REG
        spin_unlock_irqrestore

        mov     ax, [divisor]
        call    uart_set_baud

        mov     bx, [lcr]
        wr_reg  LCR_REG

        mov     al, MCR_DTR or MCR_OUT1 or MCR_OUT2
        wr_reg  MCR_REG

        ; enable rx interrupt
        mov     al, IER_RDAI or IER_LSI
        wr_reg  IER_REG

        xor     eax, eax
        ret
  .fail:
        mov     eax, SERIAL_API_ERR_CONF
        ret
endp

proc uart_tx stdcall, data:dword
        mov     ecx, [data]
        mov     edx, [ecx + DRV_DATA.io_addr]
        spin_lock_irqsave
        rd_reg  IER_REG
        or      ax, IER_THRI
        wr_reg  IER_REG
        spin_unlock_irqrestore
        ret
endp

align 4
uart_drv:
        dd      uart_drv_end - uart_drv
        dd      uart_startup
        dd      uart_shutdown
        dd      uart_reconf
        dd      uart_tx
uart_drv_end:
