
format MS COFF

public _i40


public _create_window
public _show_window
public _get_event
public _def_window_proc

public stb_create_window
public stb_show_window
public stb_get_event
public stb_def_window_proc

extrn _sys_create_window
extrn _sys_show_window
extrn _sys_get_event
extrn _sys_def_window_proc

section '.text' code readable align 16

align 16

_i40:
        ;   xchg bx, bx

           int 0x41
           iretd

align 4
stb_create_window:

           pushd [ecx+20]
           pushd [ecx+16]
           pushd [ecx+12]
           pushd [ecx+8]
           pushd [ecx+4]
           pushd [ecx]

           call _sys_create_window

           add esp, 24
           mov [esp + 32], eax
           ret

align 4
stb_show_window:
           pushd [ecx]
           call _sys_show_window
           add esp, 4
           mov [esp + 32], eax
           ret

align 4
stb_get_event:
           pushd [ecx]
           call _sys_get_event
           add esp, 4
           mov [esp + 32], eax
           ret

align 4
stb_def_window_proc:
           pushd [ecx]
           call _sys_def_window_proc
           add esp, 4
           mov [esp + 32], eax
           ret

align 4
_create_window:

           lea ecx, [esp+4]
           mov eax, 73
           int 0x41
           ret

align 4
_show_window:

           lea ecx, [esp+4]
           mov eax, 74
           int 0x41
           ret

align 4
_get_event:
           lea ecx, [esp+4]
           mov eax, 75
           int 0x41
           ret

align 4
_def_window_proc:

           lea ecx, [esp+4]
           mov eax, 76
           int 0x41
           ret

