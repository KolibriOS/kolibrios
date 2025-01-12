format PE DLL native 0.05
entry START

L_DBG = 1
L_ERR = 2

__DEBUG__ = 0
__DEBUG_LEVEL__ = L_DBG

SERIAL_RING_BUF_SIZE = 32768

API_VERSION = 1

section '.flat' readable writable executable

include '../struct.inc'
include '../proc32.inc'
include '../fdo.inc'
include '../macros.inc'
include '../peimport.inc'

include 'common.inc'
include 'ring_buf.inc'
include 'uart16550.inc'

struct  SERIAL_OBJ
        magic           dd ?
        destroy         dd ?
        fd              dd ?
        bk              dd ?
        pid             dd ?
        port            dd ? ; pointer to SERIAL_PORT
ends

struct  SERIAL_PORT
        fd              dd ?
        bk              dd ?
        id              dd ? ; unique port number
        mtx             MUTEX
        con             dd ? ; pointer to SERIAL_OBJ
        drv             dd ? ; pointer to struct SP_DRIVER
        drv_data        dd ? ; pointer to driver-defined data
        rx_buf          RING_BUF
        tx_buf          RING_BUF
        conf            SP_CONF
ends

proc START c, reason:dword
        cmp     [reason], DRV_ENTRY
        jne     .fail

        mov     ecx, port_list_mutex
        invoke  MutexInit

        stdcall uart_probe, 0x3f8, 4
        stdcall uart_probe, 0x2f8, 3
        stdcall uart_probe, 0x3e8, 4
        stdcall uart_probe, 0x2e8, 3
        invoke  RegService, drv_name, service_proc
        ret

  .fail:
        xor     eax, eax
        ret
endp

srv_calls:
        dd      service_proc.get_version
        dd      service_proc.drv_add_port
        dd      service_proc.drv_remove_port
        dd      service_proc.drv_handle_event
        dd      service_proc.open
        dd      service_proc.close
        dd      service_proc.setup
        dd      service_proc.read
        dd      service_proc.write
        ; TODO enumeration
srv_calls_end:

proc service_proc stdcall uses ebx esi edi, ioctl:dword
        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]
        cmp     eax, (srv_calls_end - srv_calls) / 4
        jae     .err
        jmp     dword [srv_calls + eax * 4]

  .get_version:
        cmp     [edx + IOCTL.out_size], 4
        jb      .err
        mov     edx, [edx + IOCTL.output]
        mov     dword [edx], API_VERSION
        xor     eax, eax
        ret

  .drv_add_port:
        ; in:
        ;  +0: driver
        ;  +4: driver data
        cmp     [edx + IOCTL.inp_size], 8
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        mov     ecx, [ebx]
        mov     edx, [ebx + 4]
        call    add_port
        ret

  .drv_remove_port:
        ; in:
        ;  +0: port handle
        cmp     [edx + IOCTL.inp_size], 4
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        mov     ecx, [ebx]
        call    remove_port
        ret

  .drv_handle_event:
        ; in:
        ;  +0: port handle
        ;  +4: event
        ;  +8: count
        ;  +12: buf
        cmp     [edx + IOCTL.inp_size], 16
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        mov     eax, [ebx]
        mov     edx, [ebx + 4]
        mov     ecx, [ebx + 8]
        mov     esi, [ebx + 12]
        call    handle_event
        ret

  .open:
        ; in:
        ;  +0 port number
        ;  +4 addr to SERIAL_CONF
        ; out:
        ;  +0 port handle if success
        cmp     [edx + IOCTL.inp_size], 8
        jb      .err
        cmp     [edx + IOCTL.out_size], 4
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        mov     ecx, [edx + IOCTL.output]
        stdcall sp_open, [ebx], [ebx + 4], ecx
        ret

  .close:
        ; in:
        ;  +0 port handle
        cmp     [edx + IOCTL.inp_size], 4
        jb      .err
        mov     ecx, [edx + IOCTL.input]
        mov     ecx, [ecx]
        call    sp_close
        ret

  .setup:
        ; in:
        ;  +0 port handle
        ;  +4 addr to SERIAL_CONF
        ; out:
        ;  +0 result
        cmp     [edx + IOCTL.inp_size], 8
        jb      .err
        cmp     [edx + IOCTL.out_size], 4
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        push    edx
        mov     eax, [ebx]
        mov     esi, [ebx + 4]
        call    sp_setup
        pop     edx
        mov     ebx, [edx + IOCTL.output]
        mov     [ebx], eax
        ret

  .read:
        ; in:
        ;  +0 port handle
        ;  +4 addr of dest buf
        ;  +8 count to read
        ; out:
        ;  +0 bytes read
        cmp     [edx + IOCTL.inp_size], 12
        jb      .err
        cmp     [edx + IOCTL.out_size], 4
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        push    edx
        mov     eax, [ebx]
        mov     edi, [ebx + 4]
        mov     ecx, [ebx + 8]
        call    sp_read
        pop     edx
        mov     ebx, [edx + IOCTL.output]
        mov     [ebx], ecx
        ret

  .write:
        ; in:
        ;  +0 port handle
        ;  +4 addr to source buf
        ;  +8 count to write
        ; out:
        ;  +0 bytes written
        cmp     [edx + IOCTL.inp_size], 12
        jb      .err
        cmp     [edx + IOCTL.out_size], 4
        jb      .err
        mov     ebx, [edx + IOCTL.input]
        push    edx
        mov     eax, [ebx]
        mov     esi, [ebx + 4]
        mov     ecx, [ebx + 8]
        call    sp_write
        pop     edx
        mov     ebx, [edx + IOCTL.output]
        mov     [ebx], ecx
        ret

  .err:
        or      eax, -1
        ret
endp

; struct SERIAL_PORT __fastcall *add_port(const struct SP_DRIVER *drv, const void *drv_data);
align 4
proc add_port uses edi
        DEBUGF  L_DBG, "serial.sys: add port drv=%x drv_data=%x\n", ecx, edx

        mov     eax, [ecx + SP_DRIVER.size]
        cmp     eax, sizeof.SP_DRIVER
        jne     .fail

        ; alloc memory for serial port descriptor
        push    ecx
        push    edx
        movi    eax, sizeof.SERIAL_PORT
        invoke  Kmalloc
        pop     edx
        pop     ecx
        test    eax, eax
        jz      .fail

        ; initialize fields of descriptor
        mov     edi, eax
        mov     [edi + SERIAL_PORT.drv], ecx
        mov     [edi + SERIAL_PORT.drv_data], edx
        lea     ecx, [edi + SERIAL_PORT.mtx]
        invoke  MutexInit
        and     [edi + SERIAL_PORT.con], 0

        mov     ecx, port_list_mutex
        invoke  MutexLock

        ; TODO obtain unused id's
        mov     eax, [port_count]
        mov     [edi + SERIAL_PORT.id], eax
        inc     [port_count]

        ; add port to linked list
        mov     eax, port_list
        mov     ecx, [eax + SERIAL_PORT.bk]
        mov     [edi + SERIAL_PORT.bk], ecx
        mov     [edi + SERIAL_PORT.fd], eax
        mov     [ecx + SERIAL_PORT.fd], edi
        mov     [eax + SERIAL_PORT.bk], edi

        DEBUGF  L_DBG, "serial.sys: add port %x with id %x\n", edi, [edi + SERIAL_PORT.id]

        mov     ecx, port_list_mutex
        invoke  MutexUnlock

        mov     eax, edi
        ret

  .fail:
        xor     eax, eax
        ret
endp

align 4
; u32 __fastcall *remove_port(struct SERIAL_PORT *port);
proc remove_port uses esi
        mov     esi, ecx
        mov     ecx, port_list_mutex
        invoke  MutexLock

        lea     ecx, [esi + SERIAL_PORT.mtx]
        invoke  MutexLock

        mov     eax, [esi + SERIAL_PORT.con]
        test    eax, eax
        jz      @f
        push    esi
        call    sp_destroy
        pop     esi
  @@:

        mov     eax, [esi + SERIAL_PORT.fd]
        mov     edx, [esi + SERIAL_PORT.bk]
        mov     [edx + SERIAL_PORT.fd], eax
        mov     [eax + SERIAL_PORT.bk], edx
        DEBUGF  L_DBG, "serial.sys: remove port %x with id %x\n", esi, [esi + SERIAL_PORT.id]
        mov     eax, esi
        invoke  Kfree

        mov     ecx, port_list_mutex
        invoke  MutexUnlock

        xor     eax, eax
        ret
endp

align 4
; @param eax port
; @param edx event
; @param ecx count
; @param esi buffer
; @return eax count
proc handle_event uses edi
        mov     edi, eax
        cmp     edx, SERIAL_EVT_RXNE
        jz      .rx
        cmp     edx, SERIAL_EVT_TXE
        jz      .tx
        xor     eax, eax
        jmp     .exit
  .rx:
        lea     eax, [edi + SERIAL_PORT.rx_buf]
        stdcall ring_buf_write, eax, esi, ecx
        jmp     .exit
  .tx:
        lea     eax, [edi + SERIAL_PORT.tx_buf]
        stdcall ring_buf_read, eax, esi, ecx
        ; fallthrough
  .exit:
        ret
endp

align 4
proc sp_validate_conf
        mov     eax, [ecx + SP_CONF.size]
        cmp     eax, sizeof.SP_CONF
        jnz     .fail
        mov     eax, [ecx + SP_CONF.baudrate]
        test    eax, eax
        jz      .fail
        mov     al, [ecx + SP_CONF.word_size]
        cmp     al, 8
        jne     .fail ; TODO implement different word size
        mov     al, [ecx + SP_CONF.stop_bits]
        cmp     al, 1
        jne     .fail ; TODO implement different stop bits count
        mov     al, [ecx + SP_CONF.parity]
        cmp     al, SERIAL_CONF_PARITY_NONE
        jne     .fail ; TODO implement parity
        mov     al, [ecx + SP_CONF.flow_ctrl]
        cmp     al, SERIAL_CONF_FLOW_CTRL_NONE
        jne     .fail ; TODO implement flow control
  .ok:
        xor     eax, eax
        ret
  .fail:
        or      eax, -1
        ret
endp

align 4
proc sp_open stdcall uses ebx esi edi, port_id:dword, conf:dword, handle:dword
        DEBUGF  L_DBG, "serial.sys: sp_open %x %x %x\n", [port_id], [conf], [handle]

        mov     ecx, [conf]
        call    sp_validate_conf
        test    eax, eax
        jz      @f
        mov     eax, SERIAL_API_ERR_CONF
        ret
  @@:
        mov     edi, [conf]

        ; get access to the serial ports list
        mov     ecx, port_list_mutex
        invoke  MutexLock

        ; find port by id
        mov     eax, [port_id]
        mov     esi, port_list
  .find_port:
        mov     esi, [esi + SERIAL_PORT.fd]
        cmp     esi, port_list
        jz      .not_found
        mov     ecx, [esi + SERIAL_PORT.id]
        cmp     ecx, eax
        jz      .found
        jmp     .find_port

  .not_found:
        DEBUGF  L_DBG, "serial.sys: port not found\n"
        mov     eax, SERIAL_API_ERR_PORT_INVALID
        jmp     .unlock_list

  .found:
        DEBUGF  L_DBG, "serial.sys: found port %x\n", esi

        ; get access to serial port
        lea     ecx, [esi + SERIAL_PORT.mtx]
        invoke  MutexLock

        ; availability check
        cmp     [esi + SERIAL_PORT.con], 0
        jz      .open
        mov     eax, SERIAL_API_ERR_PORT_BUSY
        jmp     .unlock_port

  .open:
        ; create rx and tx ring buffers
        lea     ecx, [esi + SERIAL_PORT.rx_buf]
        mov     edx, SERIAL_RING_BUF_SIZE
        call    ring_buf_create
        test    eax, eax
        jnz     @f
        jmp     .unlock_port
  @@:
        lea     ecx, [esi + SERIAL_PORT.tx_buf]
        mov     edx, SERIAL_RING_BUF_SIZE
        call    ring_buf_create
        test    eax, eax
        jnz     @f
        jmp     .free_rx_buf
  @@:
        invoke  GetPid
        mov     ebx, eax
        mov     eax, sizeof.SERIAL_OBJ
        invoke  CreateObject
        test    eax, eax
        jnz     @f
        or      eax, -1
        jmp     .free_tx_buf
  @@:
        DEBUGF  L_DBG, "serial.sys: created object %x\n", eax

        ; save port handle
        mov     ecx, [handle]
        mov     [ecx], eax
        mov     [eax + SERIAL_OBJ.magic], 'UART'
        mov     [eax + SERIAL_OBJ.destroy], sp_destroy
        mov     [eax + SERIAL_OBJ.port], esi

        ; fill fields
        mov     [esi + SERIAL_PORT.con], eax
        ; copy conf
        mov     eax, [edi + SP_CONF.size]
        mov     [esi + SERIAL_PORT.conf + SP_CONF.size], eax
        mov     eax, [edi + SP_CONF.baudrate]
        mov     [esi + SERIAL_PORT.conf + SP_CONF.baudrate], eax
        mov     eax, dword [edi + SP_CONF.word_size]
        mov     dword [esi + SERIAL_PORT.conf + SP_CONF.word_size], eax

        ; tell driver about port open
        mov     ebx, [esi + SERIAL_PORT.drv]
        mov     ecx, [esi + SERIAL_PORT.drv_data]
        stdcall dword [ebx + SP_DRIVER.startup], ecx, edi
        test    eax, eax
        jz      .unlock_port
        ; on error fallthrough
        push    eax
        mov     eax, [esi + SERIAL_PORT.con]
        invoke  DestroyObject
        and     [esi + SERIAL_PORT.con], 0
        pop     eax

  .free_tx_buf:
        push    eax
        lea     ecx, [esi + SERIAL_PORT.tx_buf]
        call    ring_buf_destroy
        pop     eax

  .free_rx_buf:
        push    eax
        lea     ecx, [esi + SERIAL_PORT.rx_buf]
        call    ring_buf_destroy
        pop     eax

  .unlock_port:
        push    eax
        lea     ecx, [esi + SERIAL_PORT.mtx]
        invoke  MutexUnlock
        pop     eax

  .unlock_list:
        push    eax
        mov     ecx, port_list_mutex
        invoke  MutexUnlock
        pop     eax

        ret
endp

align 4
; @param ecx serial port handle
proc sp_close uses ebx esi
        mov     eax, ecx
        cmp     [eax + SERIAL_OBJ.magic], 'UART'
        je      .ok
        or      eax, -1
        ret
  .ok:
        mov     ebx, [eax + SERIAL_OBJ.port]
        push    eax
        lea     ecx, [ebx + SERIAL_PORT.mtx]
        invoke  MutexLock
        pop     eax

        push    ebx
        call    sp_destroy
        pop     ebx

        lea     ecx, [ebx + SERIAL_PORT.mtx]
        invoke  MutexUnlock

        xor     eax, eax
        ret
endp

align 4
; @param eax port handle
; @param esi pointer to SP_CONF
; @return eax = 0 on success
proc sp_setup
        test    esi, esi
        jz      .fail
        cmp     [eax + SERIAL_OBJ.magic], 'UART'
        jne     .fail
        mov     ebx, eax
        mov     ecx, esi
        call    sp_validate_conf
        test    eax, eax
        jz      @f
        DEBUGF  L_DBG, "serial.sys: invalid conf %x\n", ecx
        mov     eax, SERIAL_API_ERR_CONF
        ret
  @@:
        ; lock mutex
        mov     edi, [ebx + SERIAL_OBJ.port]
        lea     ecx, [edi + SERIAL_PORT.mtx]
        invoke  MutexLock
        ; reconfigure port
        mov     eax, [edi + SERIAL_PORT.drv]
        mov     ecx, [edi + SERIAL_PORT.drv_data]
        stdcall dword [eax + SP_DRIVER.reconf], ecx, esi
        xor     eax, eax
        push    eax
        test    eax, eax
        jnz     @f
        ; copy conf if success
        mov     eax, [esi + SP_CONF.size]
        mov     [edi + SERIAL_PORT.conf + SP_CONF.size], eax
        mov     eax, [esi + SP_CONF.baudrate]
        mov     [edi + SERIAL_PORT.conf + SP_CONF.baudrate], eax
        mov     eax, dword [esi + SP_CONF.word_size]
        mov     dword [edi + SERIAL_PORT.conf + SP_CONF.word_size], eax
  @@:
        ; unlock mutex
        lea     ecx, [edi + SERIAL_PORT.mtx]
        invoke  MutexUnlock
        pop     eax
        ret
  .fail:
        or      eax, -1
        ret
endp

align 4
; @param eax serial obj
proc sp_destroy
        mov     esi, [eax + SERIAL_OBJ.port]
        DEBUGF  L_DBG, "serial.sys: destroy port %x\n", esi

        invoke  DestroyObject
        and     [esi + SERIAL_PORT.con], 0

        ; tell driver about port close
        mov     ebx, [esi + SERIAL_PORT.drv]
        mov     edx, [esi + SERIAL_PORT.drv_data]
        stdcall dword [ebx + SP_DRIVER.shutdown], edx

        lea     ecx, [esi + SERIAL_PORT.tx_buf]
        call    ring_buf_destroy
        lea     ecx, [esi + SERIAL_PORT.rx_buf]
        call    ring_buf_destroy
        ret
endp


align 4
; @param eax port handle
; @param ecx bytes count
; @param edi address of destination buffer
; @return eax = 0 on success and ecx = count bytes read
proc sp_read
        test    edi, edi
        jz      .fail
        test    ecx, ecx
        jz      .fail
        cmp     [eax + SERIAL_OBJ.magic], 'UART'
        jne     .fail
        mov     esi, [eax + SERIAL_OBJ.port]
        push    ecx ; last arg for ring_buf_read
        lea     ecx, [esi + SERIAL_PORT.mtx]
        invoke  MutexLock
        lea     eax, [esi + SERIAL_PORT.rx_buf]
        stdcall ring_buf_read, eax, edi
        push    eax
        lea     ecx, [esi + SERIAL_PORT.mtx]
        invoke  MutexUnlock
        pop     ecx
        xor     eax, eax
        ret
  .fail:
        or      eax, -1
        ret
endp

align 4
; @param eax port handle
; @param ecx bytes count
; @param esi address of source buffer
; @return eax = 0 on success and ecx = count bytes written
proc sp_write
        test    esi, esi
        jz      .fail
        test    ecx, ecx
        jz      .fail
        cmp     [eax + SERIAL_OBJ.magic], 'UART'
        jne     .fail

        push    ecx ; last arg for ring_buf_write
        mov     edi, [eax + SERIAL_OBJ.port]
        lea     ecx, [edi + SERIAL_PORT.mtx]
        invoke  MutexLock

        lea     ecx, [edi + SERIAL_PORT.tx_buf]
        stdcall ring_buf_write, ecx, esi
        push    eax
        test    eax, eax
        jz      @f
        mov     ebx, [edi + SERIAL_PORT.drv]
        mov     edx, [edi + SERIAL_PORT.drv_data]
        stdcall dword [ebx + SP_DRIVER.tx], edx
  @@:
        lea     ecx, [edi + SERIAL_PORT.mtx]
        invoke  MutexUnlock
        pop     ecx
        xor     eax, eax
        ret
  .fail:
        or      eax, -1
        ret
endp

drv_name    db 'SERIAL', 0
include_debug_strings

align 4
port_count       dd 0
port_list_mutex  MUTEX
port_list:
  .fd            dd port_list
  .bk            dd port_list

align 4
data fixups
end data
