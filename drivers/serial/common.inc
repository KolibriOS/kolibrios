SERIAL_COMPATIBLE_API_VER = 0 ; increments in case of breaking changes

SERIAL_API_GET_VERSION = 0
SERIAL_API_SRV_ADD_PORT = 1
SERIAL_API_SRV_REMOVE_PORT = 2
SERIAL_API_SRV_HANDLE_EVENT = 3
SERIAL_API_OPEN_PORT = 4
SERIAL_API_CLOSE_PORT = 5
SERIAL_API_SETUP_PORT = 6
SERIAL_API_READ = 7
SERIAL_API_WRITE = 8

SERIAL_API_ERR_PORT_INVALID = 1
SERIAL_API_ERR_PORT_BUSY = 2
SERIAL_API_ERR_CONF = 3

SERIAL_EVT_TXE = 1 ; tx fifo or register is empty, all data has been sent
SERIAL_EVT_RXNE = 2 ; rx fifo or register is not empty

SERIAL_CONF_PARITY_NONE = 0
SERIAL_CONF_PARITY_EVEN = 1
SERIAL_CONF_PARITY_ODD = 2
SERIAL_CONF_PARITY_MARK = 3
SERIAL_CONF_PARITY_SPACE = 4

SERIAL_CONF_STOP_BITS_1 = 0
SERIAL_CONF_STOP_BITS_1_5 = 1
SERIAL_CONF_STOP_BITS_2 = 2

SERIAL_CONF_FLOW_CTRL_NONE = 0

struct  SP_DRIVER
        size            dd ? ; size of this struct
        startup         dd ? ; int __stdcall (*startup)(void *drv_data, const struct serial_conf *conf);
        shutdown        dd ? ; int __stdcall (*shutdown)(void *drv_data);
        reconf          dd ? ; int __stdcall (*reconf)(void *drv_data, const struct serial_conf *conf);
        tx              dd ? ; int __stdcall (*tx)(void *drv_data);
ends

struct  SP_CONF
        size            dd ? ; size of this struct
        baudrate        dd ?
        word_size       db ?
        stop_bits       db ?
        parity          db ?
        flow_ctrl       db ?
ends

proc serial_add_port stdcall, drv:dword, drv_data:dword
locals
        handler    dd ?
        io_code    dd ?
        input      dd ?
        inp_size   dd ?
        output     dd ?
        out_size   dd ?
endl
        mov     eax, [serial_drv_entry]
        mov     [handler], eax
        mov     [io_code], SERIAL_API_SRV_ADD_PORT
        lea     eax, [drv]
        mov     [input], eax
        mov     [inp_size], 8
        xor     eax, eax
        mov     [output], eax
        mov     [out_size], eax
        lea     eax, [handler]
        push    esi edi
        invoke  ServiceHandler, eax
        pop     edi esi
        ret
endp

proc serial_remove_port stdcall, port:dword
locals
        handler    dd ?
        io_code    dd ?
        input      dd ?
        inp_size   dd ?
        output     dd ?
        out_size   dd ?
endl
        mov     eax, [serial_drv_entry]
        mov     [handler], eax
        mov     [io_code], SERIAL_API_SRV_REMOVE_PORT
        lea     eax, [port]
        mov     [input], eax
        mov     [inp_size], 4
        xor     eax, eax
        mov     [output], eax
        mov     [out_size], eax
        lea     eax, [handler]
        push    esi edi
        invoke  ServiceHandler, eax
        pop     edi esi
        ret
endp

; see SERIAL_EVT_*
proc serial_handle_event stdcall, port:dword, event:dword, count:dword, buff:dword
locals
        handler    dd ?
        io_code    dd ?
        input      dd ?
        inp_size   dd ?
        output     dd ?
        out_size   dd ?
endl
        mov     eax, [serial_drv_entry]
        mov     [handler], eax
        mov     [io_code], SERIAL_API_SRV_HANDLE_EVENT
        lea     eax, [port]
        mov     [input], eax
        mov     [inp_size], 16
        xor     eax, eax
        mov     [output], eax
        mov     [out_size], eax
        lea     eax, [handler]
        push    esi edi
        invoke  ServiceHandler, eax
        pop     edi esi
        ret
endp

proc serial_port_init
        lea     ecx, [serial_drv_name]
        mcall   SF_SYS_MISC, SSF_LOAD_DRIVER
        mov     [serial_drv_handle], eax
        ret
endp

proc serial_port_get_version stdcall, version:dword
locals
        .handler    dd ?
        .io_code    dd ?
        .input      dd ?
        .inp_size   dd ?
        .output     dd ?
        .out_size   dd ?
endl
        mov     eax, [serial_drv_handle]
        mov     [.handler], eax
        mov     dword [.io_code], SERIAL_API_GET_VERSION
        mov     [.input], 0
        mov     dword [.inp_size], 0
        mov     eax, [version]
        mov     [.output], eax
        mov     dword [.out_size], 4

        lea     ecx, [.handler]
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER
        
        ret
endp

proc serial_port_open stdcall uses ebx, port_id:dword, conf:dword, handle:dword
locals
        .handler    dd ?
        .io_code    dd ?
        .input      dd ?
        .inp_size   dd ?
        .output     dd ?
        .out_size   dd ?
endl
        push    [conf]
        push    [port_id]
        mov     eax, [serial_drv_handle]
        mov     [.handler], eax
        mov     dword [.io_code], SERIAL_API_OPEN_PORT
        mov     [.input], esp
        mov     dword [.inp_size], 8
        mov     eax, [handle]
        mov     [.output], eax
        mov     dword [.out_size], 4

        lea     ecx, [.handler]
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER

        add     esp, 8
        ret
endp

proc serial_port_close stdcall uses ebx, handle:dword
locals
        .handler    dd ?
        .io_code    dd ?
        .input      dd ?
        .inp_size   dd ?
        .output     dd ?
        .out_size   dd ?
endl
        push    [handle]
        mov     eax, [serial_drv_handle]
        mov     [.handler], eax
        mov     dword [.io_code], SERIAL_API_CLOSE_PORT
        mov     [.input], esp
        mov     dword [.inp_size], 4
        mov     dword [.output], 0
        mov     dword [.out_size], 0

        lea     ecx, [.handler]
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER

        add     esp, 4
        ret
endp

proc serial_port_setup stdcall uses ebx, handle:dword, conf:dword
locals
        .handler    dd ?
        .io_code    dd ?
        .input      dd ?
        .inp_size   dd ?
        .output     dd ?
        .out_size   dd ?
endl
        push    [conf]
        push    [handle]
        mov     eax, [serial_drv_handle]
        mov     [.handler], eax
        mov     dword [.io_code], SERIAL_API_SETUP_PORT
        mov     [.input], esp
        mov     dword [.inp_size], 8
        sub     esp, 4
        mov     [.output], esp
        mov     dword [.out_size], 4

        lea     ecx, [.handler]
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER

        pop     eax
        add     esp, 8
        ret
endp

proc serial_port_read stdcall uses ebx, handle:dword, dest:dword, count_ptr:dword
locals
        .handler    dd ?
        .io_code    dd ?
        .input      dd ?
        .inp_size   dd ?
        .output     dd ?
        .out_size   dd ?
endl
        mov     eax, [count_ptr]
        push    dword [eax]
        push    [dest]
        push    [handle]
        mov     eax, [serial_drv_handle]
        mov     [.handler], eax
        mov     dword [.io_code], SERIAL_API_READ
        mov     [.input], esp
        mov     dword [.inp_size], 12
        sub     esp, 4
        mov     [.output], esp
        mov     [.out_size], 4

        lea     ecx, [.handler]
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER

        pop     ecx
        mov     edx, [count_ptr]
        mov     [edx], ecx
        add     esp, 12
        ret
endp

proc serial_port_write stdcall uses ebx, handle:dword, src:dword, count_ptr:dword
locals
        .handler    dd ?
        .io_code    dd ?
        .input      dd ?
        .inp_size   dd ?
        .output     dd ?
        .out_size   dd ?
endl
        mov     eax, [count_ptr]
        push    dword [eax]
        push    [src]
        push    [handle]
        mov     eax, [serial_drv_handle]
        mov     [.handler], eax
        mov     dword [.io_code], SERIAL_API_WRITE
        mov     [.input], esp
        mov     dword [.inp_size], 12
        sub     esp, 4
        mov     dword [.output], esp
        mov     dword [.out_size], 4

        lea     ecx, [.handler]
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER

        pop     ecx
        mov     edx, [count_ptr]
        mov     [edx], ecx
        add     esp, 12
        ret
endp

align 4
serial_drv_name   db "SERIAL", 0
serial_drv_handle dd ?
