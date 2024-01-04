format MS COFF

include '../proc32.inc'

public _load_libconsole

macro public_jmp name, size
{
  public _#name#@#size
  _#name#@#size: jmp [name]
}

section '.text' align 16 code readable executable

;void* __fastcall getprocaddr(export, name)
align 4
getprocaddress:
        push    esi
        push    edi

        xor     eax, eax
        test    ecx, ecx        ; If hlib = 0 then goto .end
        jz      .end
.next:
        cmp     [ecx], dword 0  ; If end of export table then goto .end
        jz      .end

        xor     eax, eax
        mov     esi, [ecx]
        mov     edi, edx        ; name
.next_:
        lodsb
        scasb
        jne     .fail
        or      al, al
        jnz     .next_
        jmp     .ok
.fail:
        add     ecx, 8
        jmp     .next
.ok:
        mov eax, [ecx + 4]      ; return address
.end:
        pop     edi
        pop     esi
        ret


;void fastcall dll_link(export, import)

align 4
dll_link:
        push    esi
        push    ecx
        mov     esi, edx
        test    esi, esi
        jz      .done
.next:
        mov     edx, [esi]
        test    edx, edx
        jz      .done
        mov     ecx, [esp]
        call    getprocaddress
        test    eax, eax
        jz      .done
        mov     [esi], eax
        add     esi, 4
        jmp     .next
.done:
        pop     ecx
        pop     esi
        ret

align 4
dll_load:
        push    ebp
        push    ebx
        push    esi
        push    edi

        mov     ebp, [esp+20]
.next_lib:
        mov     edx, [ebp]
        test    edx, edx
        jz      .exit

        mov     esi, [ebp+4]
        mov     edi, s_libdir.fname
@@:
        lodsb
        stosb
        test  al, al
        jnz @b

        mov     eax, 68
        mov     ebx, 19
        mov     ecx, s_libdir
        int     0x40
        test    eax, eax
        jz      .fail

        mov     ecx, eax
        call    dll_link
        mov     eax, [ecx]
        cmp     dword[eax], 'lib_'
        jnz     @f

        mov     esi, [ecx+4]

        pushad
        mov     eax, mem.Alloc
        mov     ebx, mem.Free
        mov     ecx, mem.ReAlloc
        mov     edx, dll_load
        call    esi
        popad
@@:
        add     ebp, 8
        jmp     .next_lib
.exit:
        pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        xor     eax, eax
        ret 4
.fail:
        pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        inc     eax
        ret 4

align 4
_load_libconsole:
        push    ebx
        mov     eax, 40
        mov     ebx, 1 shl 8
        int     0x40
        pop     ebx

        push    @IMPORT
        call    dll_load
        test    eax, eax
        jnz     .fail
        push    1
        call    [con_start]
        xor     eax, eax
.fail:
        ret

proc mem.Alloc, size
        push    ebx ecx
        mov     ecx, [size]
        mov     eax, 68
        mov     ebx, 12
        int     0x40
        pop     ecx ebx
        ret
endp
;-----------------------------------------------------------------------------
proc mem.ReAlloc, mptr, size
        push    ebx ecx edx
        mov     ecx, [size]
        test    ecx, ecx
        jz      @f
@@:
        mov     edx, [mptr]
        test    edx, edx
        jz      @f
@@:
        mov     eax, 68
        mov     ebx, 20
        int     0x40
        test    eax, eax
        jz  @f
@@:
        pop edx ecx ebx
        ret
endp
;-----------------------------------------------------------------------------
proc mem.Free, mptr
        push    ebx ecx
        mov     ecx,[mptr]
        test    ecx,ecx
        jz  @f
@@:
        mov     eax, 68
        mov     ebx, 13
        int     0x40
        pop ecx ebx
        ret
endp

public_jmp  con_init, 20
public_jmp  con_exit, 4
public_jmp  con_gets, 8
public_jmp  con_gets2, 12
public_jmp  con_cls, 0
public_jmp  con_getch2, 0
public_jmp  con_getch, 0
public_jmp  con_kbhit, 0
public_jmp  con_set_cursor_pos, 8
public_jmp  con_get_cursor_pos, 8
public_jmp  con_write_string, 8
public_jmp  con_write_asciiz, 4
public_jmp  con_get_flags, 0
public_jmp  con_set_flags, 4
public_jmp  con_set_title, 4
public_jmp  con_get_font_height, 0
public_jmp  con_get_cursor_height, 0
public_jmp  con_set_cursor_height, 4

section '.data' align 16 readable writable

; -------------------------
macro library [lname,fname]
{
  forward
    dd __#lname#_library_table__,__#lname#_library_name__
  common
    dd 0
  forward
    align 4
    __#lname#_library_name__ db fname,0
}

macro import lname,[name,sname]
{
  common
    align 4
    __#lname#_library_table__:
  forward
    if used name
      name dd __#name#_import_name__
    end if
  common
    dd 0
  forward
    if used name
      align 4
      __#name#_import_name__ db sname,0
    end if
}

align   4
@IMPORT:

library console,        'console.obj'

import  console, \
        con_start,      'START', \
        con_init,       'con_init', \
        con_exit,       'con_exit', \
        con_gets,       'con_gets', \
        con_gets2,      'con_gets2', \
        con_cls,        'con_cls', \
        con_getch2,     'con_getch2', \
        con_getch,      'con_getch', \
        con_kbhit,      'con_kbhit', \
        con_set_cursor_pos, 'con_set_cursor_pos',\
        con_get_cursor_pos, 'con_get_cursor_pos', \
        con_write_string, 'con_write_string',\
        con_write_asciiz, 'con_write_asciiz', \
        con_get_flags,  'con_get_flags', \
        con_set_flags,  'con_set_flags', \
        con_set_title,  'con_set_title', \
        con_get_font_height, 'con_get_font_height', \
        con_get_cursor_height, 'con_get_cursor_height', \
        con_set_cursor_height, 'con_set_cursor_height'
        
s_libdir:
  db '/sys/lib/'
  .fname rb 32
