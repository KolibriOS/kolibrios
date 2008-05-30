format MS COFF

include "proc32.inc"

section '.text' align 16 code readable executable

public _InitHeap@4
public _UserAlloc@4
public _UserFree@4

public _GetNotify@4
public _CreateThread@8
public _GetMousePos@4

public _get_fileinfo@8
public _create_file@4
public _read_file@20
public _write_file@20

public _get_key@4
public _remap_key@4
public _get_button_id

public _GetScreenSize@8

public _DrawWindow@36
public _make_button@24
public _draw_bar@20

public _write_text@20
public _debug_out@4
public _debug_out_hex@4
public _create_thread@12


public _memset

struc FILEIO
{   .cmd            dd ?
    .offset         dd ?
                    dd ?
    .count          dd ?
    .buff           dd ?
                    db ?
    .name           dd ?
};

struc CTRL_INFO
{   .pci_cmd        dd  ?
    .irq            dd  ?
    .glob_cntrl     dd  ?
    .glob_sta       dd  ?
    .codec_io_base  dd  ?
    .ctrl_io_base   dd  ?
    .codec_mem_base dd  ?
    .ctrl_mem_base  dd  ?
    .codec_id       dd  ?
}
CTRL_INFO_SIZE      equ 9*4

align 4
_GetScreenSize@8:
           .x equ esp+12
           .y equ esp+16 

           push ebx
           push ecx
           mov eax, 14
           int 0x40
           mov ebx, [.y]
           movzx ecx, ax
           inc ecx
           mov [ebx], ecx
           mov ebx, [.x]
           shr eax, 16
           inc eax
           mov [ebx], eax
           pop ecx
           pop ebx
           ret 8

align 4
_create_thread@12:
.thr_proc    equ esp+8
.param       equ esp+12
.stack_size  equ esp+16

           push ebx
           
           mov eax, 68
           mov ebx, 12
           mov ecx, [.stack_size]
           add ecx, 4095
           and ecx, -4096
           int 0x40
           test eax, eax
           jz .fail

           lea edx, [eax+ecx-12]
           mov [edx], dword .exit_point
           mov ebx, [.param]
           mov [edx+4], ebx
           mov [edx+8], ecx

           mov eax, 51
           mov ebx, 1
           mov ecx, [.thr_proc]
           int 0x40
           pop ebx
           ret 12
.fail:
           not eax
           pop ebx
           ret 12
align 4
.exit_point:
           pop ecx
           mov eax, 68
           mov ebx, 13
           int 0x40
           mov eax, -1
           int 0x40

restore .thr_proc
restore .param
restore .stack_size

align 4
proc _get_button_id
           mov   eax,17
           int   0x40
           test  al,al
           jnz   @F
           shr   eax,8
           ret
@@:
           xor   eax,eax
           dec   eax
           ret
endp

align 4
proc _get_fileinfo@8 stdcall, name:dword, info:dword
           push ebx
           push ecx
           push esi
           push edi
           xor eax, eax
           mov ebx, [name]
           mov ecx, [info]

           mov [fileio.cmd], 5
           mov [fileio.offset], eax
           mov [fileio.offset+4], eax
           mov [fileio.count], eax
           mov [fileio.buff], ecx
           mov byte [fileio.buff+4], al
           mov [fileio.name], ebx

           mov eax, 70
           lea ebx, [fileio]
           int 0x40
           pop edi
           pop esi
           pop ecx
           pop ebx
           ret
endp

align 4
proc _create_file@4 stdcall, name:dword
           push ebx
           xor eax, eax
           mov ebx, [name]

           mov [fileio.cmd], 2
           mov [fileio.offset], eax
           mov [fileio.offset+4], eax
           mov [fileio.count], eax
           mov [fileio.buff], eax
           mov byte [fileio.buff+4], al
           mov [fileio.name], ebx

           mov eax, 70
           lea ebx, [fileio]
           int 0x40
           pop ebx
           ret
endp

align 4
proc _read_file@20 stdcall,name:dword, buff:dword, offset:dword,\
                                     count:dword,reads:dword
           push ebx
           push esi
           push edi
           push edx
           xor eax, eax
           mov ebx, [name]
           mov edx, [offset]
           mov esi, [buff]
           mov edi, [count]

           mov [fileio.cmd], eax
           mov [fileio.offset], edx
           mov [fileio.offset+4], eax
           mov [fileio.count], edi
           mov [fileio.buff], esi
           mov byte [fileio.buff+4], al
           mov [fileio.name], ebx

           mov eax, 70
           lea ebx, [fileio]
           int 0x40
           mov esi, [reads]
           test esi, esi
           jz @f
           mov [esi], ebx
@@:
           pop edx
           pop edi
           pop esi
           pop ebx
           ret
endp


align 4
proc _write_file@20 stdcall,name:dword, buff:dword, offset:dword,\
                                     count:dword,writes:dword
           push ebx
           push esi
           push edi
           push edx
           xor eax, eax
           mov ebx, [name]
           mov edx, [offset]
           mov esi, [buff]
           mov edi, [count]

           mov [fileio.cmd], 3
           mov [fileio.offset], edx
           mov [fileio.offset+4], eax
           mov [fileio.count], edi
           mov [fileio.buff], esi
           mov byte [fileio.buff+4], al
           mov [fileio.name], ebx

           mov eax, 70
           lea ebx, [fileio]
           int 0x40
           mov esi, [writes]
           test esi, esi
           jz @f
           mov [esi], ebx
@@:
           pop edx
           pop edi
           pop esi
           pop ebx
           ret
endp

align 4
proc _get_key@4 stdcall, key:dword
           push ebx
           push ecx
           mov eax, 2
           int 0x40
           mov ebx, [key]
           mov ecx, eax
           shr ecx, 8
           mov [ebx], ecx
           movzx eax, al
           pop ecx
           pop ebx
           ret
endp

align 4
proc _InitHeap@4 stdcall, heap_size:dword
           push ebx
           push ecx
           mov eax, 68
           mov ebx, 11
           mov ecx, [heap_size]
           int 0x40
           pop ecx
           pop ebx
           ret
endp

align 4
proc _UserAlloc@4 stdcall, alloc_size:dword
           push ebx
           push ecx
           mov eax, 68
           mov ebx, 12
           mov ecx, [alloc_size]
           int 0x40
           pop ecx
           pop ebx
           ret
endp

align 4
proc _UserFree@4 stdcall, pmem:dword
           push ebx
           push ecx
           mov eax, 68
           mov ebx, 13
           mov ecx, [pmem]
           int 0x40
           pop ecx
           pop ebx
           ret
endp           

align 4
proc _GetNotify@4 stdcall, p_ev:dword
           push ebx
           push ecx
           mov eax, 68
           mov ebx, 14
           mov ecx, [p_ev]
           int 0x40
           pop ecx
           pop ebx
           ret
endp

align 4
proc _CreateThread@8 stdcall, fn:dword, p_stack:dword
              push ebx
              push ecx
              push edx
              mov eax, 51
              mov ebx, 1
              mov ecx, [fn]
              mov edx,[p_stack]
              int 0x40
              pop edx
              pop ecx
              pop ebx
              ret
endp

align 4
proc _GetMousePos@4 stdcall,rel_type:dword
           push ebx
           mov eax, 37
           mov ebx, [rel_type]
           int 0x40
           pop ebx
           ret
endp

align 4
proc _DrawWindow@36 stdcall, x:dword, y:dword, sx:dword, sy:dword,\
                               workcolor:dword, style:dword, captioncolor:dword,\
                               windowtype:dword, bordercolor:dword
           push ebx
           push ecx
           push edx
           push edi
           push esi
           
           mov ebx, [x]
           mov ecx, [y]
           shl ebx, 16
           shl ecx, 16
           mov bx, word [sx]
           mov cx,  word [sy]
           mov  edx,[style]
           shl  edx,24
           add  edx,[workcolor]
           mov  esi,[windowtype]
           shl  esi,24
           add  esi,[captioncolor]
           mov  edi,[bordercolor]
           xor  eax,eax
           int  0x40
           pop esi
           pop edi
           pop edx
           pop ecx
           pop ebx
           ret
endp

align 4
_make_button@24:
;arg1 - x
;arg2 - y
;arg3 - xsize
;arg4 - ysize
;arg5 - id
;arg6 - color

  push  ebx
  push  ecx
  push  esi
  mov   ebx,[esp+12]
  shl   ebx,16
  mov   bx,[esp+20]
  mov   ecx,[esp+16]
  shl   ecx,16
  mov   cx,[esp+24]
  mov   edx,[esp+28]
  mov   esi,[esp+32]
  mov   eax,8
  int   0x40
  pop   esi ecx ebx
  ret   24

align 4
_draw_bar@20:
;arg1 - x
;arg2 - y
;arg3 - xsize
;arg4 - ysize
;arg5 - color
  push  ebx ecx
  mov   eax,13
  mov   ebx,[esp+12]
  shl   ebx,16
  mov   bx,[esp+20]
  mov   ecx,[esp+16]
  shl   ecx,16
  mov   cx,[esp+24]
  mov   edx,[esp+28]
  int   0x40
  pop   ecx ebx
  ret   20

_write_text@20:
;arg1 - x
;arg2 - y
;arg3 - color
;arg4 - text
;arg5 - len
  push  ebx ecx esi
  mov   eax,4
  mov   ebx,[esp+16]
  shl   ebx,16
  mov   bx,[esp+20]
  mov   ecx,[esp+24]
  mov   edx,[esp+28]
  mov   esi,[esp+32]
  int   0x40
  pop   esi ecx ebx
  ret   20

align 4
proc _debug_out@4 stdcall, val:dword
           push ebx
           push ecx
           mov  ecx,[val]
           mov  ebx,1
           mov  eax,63
           int  0x40
           pop ecx
           pop ebx
           ret
endp

align 4
proc _debug_out_hex@4 stdcall val:dword
           locals
             count dd ?
           endl

           mov [count], 8
.new_char:
           rol [val], 4
           mov ecx, [val]
           and ecx, 0x0f
           mov cl,byte [__hexdigits+ecx]
           mov eax, 63
           mov ebx, 1
           int 0x40
           dec [count]
           jnz .new_char
           ret
endp

align 4
proc _remap_key@4 stdcall, key:dword

           mov eax, [key]
           and eax, 0x7F
           movzx eax, byte [keymap+eax]
           ret
endp

align 4
_memset:
           mov     edx,[esp + 0ch]
           mov     ecx,[esp + 4]

           test    edx,edx
           jz      short toend

           xor     eax,eax
           mov     al,[esp + 8]

           push    edi
           mov     edi,ecx

           cmp     edx,4
           jb      tail

           neg     ecx
           and     ecx,3
           jz      short dwords

           sub     edx,ecx
adjust_loop:
           mov     [edi],al
           add     edi,1
           sub     ecx,1
           jnz     adjust_loop

dwords:
        mov     ecx,eax
        shl     eax,8
        add     eax,ecx
        mov     ecx,eax
        shl     eax,10h
        add     eax,ecx

        mov     ecx,edx
        and     edx,3
        shr     ecx,2
        jz      tail

                cld
        rep     stosd
main_loop_tail:
        test    edx,edx
        jz      finish


tail:
        mov     [edi],al
        add     edi,1

        sub     edx,1
        jnz     tail

finish:
        mov     eax,[esp + 8]
        pop     edi

        ret

toend:
        mov     eax,[esp + 4]

        ret

;public __allmul

__allmul:
        mov eax, [esp+8]
        mov ecx, [esp+16]
        or ecx,eax
        mov ecx, [esp+12]
        jnz .hard
        mov eax, [esp+4]
        mul ecx
        ret 16
.hard:
        push ebx
        mul ecx
        mov ebx,eax
        mov eax, [esp+8]
        mul dword [esp+20]
        add ebx,eax
        mov eax,[esp+8]
        mul ecx
        add edx,ebx
        pop ebx
        ret 16

;public __allshr

align 4
__allshr:
        cmp cl,64
        jae .sign

        cmp cl, 32
        jae .MORE32
        shrd eax,edx,cl
        sar edx,cl
        ret
.MORE32:
        mov     eax,edx
        sar     edx,31
        and     cl,31
        sar     eax,cl
        ret
.sign:
        sar     edx,31
        mov     eax,edx
        ret


;public _scalbn

align 4
proc _scalbn
	          fild	dword [esp+12]
	          fld	qword [esp+4]
	          fscale
	          fstp	st1
	          ret
endp


;public  __alloca_probe_8
;public  __alloca_probe_16

__alloca_probe_16:                       ; 16 byte aligned alloca

        push    ecx
        lea     ecx, [esp + 8]          ; TOS before entering this function
        sub     ecx, eax                ; New TOS
        and     ecx, (16 - 1)           ; Distance from 16 bit align (align down)
        add     eax, ecx                ; Increase allocation size
        sbb     ecx, ecx                ; ecx = 0xFFFFFFFF if size wrapped around
        or      eax, ecx                ; cap allocation size on wraparound
        pop     ecx                     ; Restore ecx
        jmp     __chkstk

alloca_8:                               ; 8 byte aligned alloca
__alloca_probe_8:

        push    ecx
        lea     ecx, [esp+8]          ; TOS before entering this function
        sub     ecx, eax                ; New TOS
        and     ecx, (8 - 1)            ; Distance from 8 bit align (align down)
        add     eax, ecx                ; Increase allocation Size
        sbb     ecx, ecx                ; ecx = 0xFFFFFFFF if size wrapped around
        or      eax, ecx                ; cap allocation size on wraparound
        pop     ecx                     ; Restore ecx
        jmp     __chkstk

;public __chkstk
;public _alloca_probe

align 4
;_alloca_probe:
__chkstk:
        push    ecx
        lea     ecx, [esp+8-4]          ; TOS before entering function + size for ret value
        sub     ecx, eax                ; new TOS

; Handle allocation size that results in wraparound.
; Wraparound will result in StackOverflow exception.

        sbb     eax, eax                ; 0 if CF==0, ~0 if CF==1
        not     eax                     ; ~0 if TOS did not wrapped around, 0 otherwise
        and     ecx, eax                ; set to 0 if wraparound

        mov     eax, esp                ; current TOS
        and     eax, -4096              ; Round down to current page boundary

cs10:
        cmp     ecx, eax                ; Is new TOS
        jb      short cs20              ; in probed page?
        mov     eax, ecx                ; yes.
        pop     ecx
        xchg    esp, eax                ; update esp
        mov     eax, [eax]              ; get return address
        mov     [esp], eax              ; and put it at new TOS
        ret

; Find next lower page and probe
cs20:
        sub     eax, 4096               ; decrease by PAGESIZE
        test    [eax],eax     ; probe page.
        jmp     short cs10

public __ftol2_sse

align 4
__ftol2_sse:
           push ebp
           mov ebp, esp
           sub esp, 20
           and esp, 0xFFFFFFF0
           fld st0
           fst dword [esp+18]
           fistp qword [esp+10]
           fild qword [esp+10]
           mov edx, [esp+18]
           mov eax, [esp+10]
           test eax, eax
           jz .QnaNZ

.not_QnaNZ:
           fsubp st1, st0
           test edx, edx
           jns .pos
           fstp dword [esp]
           mov ecx, [esp]
           xor ecx, 0x80000000
           add ecx, 0x7FFFFFFF
           adc eax, 0
           mov edx, [esp+14]
           adc edx, 0
           jmp .exit
.pos:
           fstp dword [esp]
           mov ecx, [esp]
           add ecx, 0x7FFFFFFF
           sbb eax, 0
           jmp .exit
.QnaNZ:
           mov edx, [esp+14]
           test edx, 0x7FFFFFFF
           jne .not_QnaNZ
           fstp dword [esp+18]
           fstp dword [esp+18]
.exit:
           leave
           ret

section '.data' align 16 data readable writable

align 16
__hexdigits db '0123456789ABCDEF'

         ;  0    1    2    3    4    5    6    7
         ;  8    9    a    b    c    d    e    f

keymap:
     db      0,  27, '1', '2', '3', '4', '5', '6'   ;00
     db    '7', '8', '9', '0', '-', '=',0x7F, 0x9   ;08
     db    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i'   ;10
     db    'o', 'p', '[', ']',  13,0x9D, 'a', 's'   ;18
     db    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'   ;20
     db      0, '~',0xB6, '|',0x7A,0x87, 'c', 'v'   ;28
     db    'b', 'n', 'm', ',', '.', '/',0xB6, '*'   ;30
     db   0xB8, ' ',   0,0xBB,0xBC,0xBD,0xBE,0xBF   ;38
     db   0xC0,0xC1,0xC2,0xC3,0xC4,   0,   0,   0   ;40
     db   0xAD,   0,   0,0xAC,   0,0xAE,   0,   0   ;48
     db   0xAF,   0,   0,   0,   0,   0,   0,   0   ;50
     db      0,   0,   0,   0,   0,   0,   0,   0   ;58
     db      0,   0,   0,   0,   0,   0,   0,   0   ;60
     db      0,   0,   0,   0,   0,   0,   0,   0   ;68
     db      0,   0,   0,   0,   0,   0,   0,   0   ;70
     db      0,   0,   0,   0,   0,   0,   0,   0   ;78

public ___sse2_available
___sse2_available dd 0

public __fltused
__fltused    dd 0

align 4
fileio FILEIO
