format MS COFF

include "proc32.inc"

section '.text' code readable executable

public _InitHeap@4
public _UserAlloc@4
public _CreateThread@8
public _GetMousePos@4
public _get_fileinfo@8
public _read_file@20
public _get_key@4
public _get_button_id
public _DrawWindow@36
public _make_button@24
public _draw_bar@20
public _write_text@20
public _debug_out@4
public _debug_out_hex@4

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
           pop ebx
           ret
endp

align 4
proc _read_file@20 stdcall,name:dword, buff:dword, offset:dword,\
                                     count:dword,reads:dword
           push ebx
           push esi
           push edi
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
                  mov eax, 68
                  mov ebx, 11
              mov ecx, [heap_size]
                  int 0x40
                  pop ebx
                  ret
endp

align 4
proc _UserAlloc@4 stdcall, alloc_size:dword
                  push ebx
                  mov eax, 68
                  mov ebx, 12
              mov ecx, [alloc_size]
                  int 0x40
                  pop ebx
                  ret
endp

align 4
proc _CreateThread@8 stdcall, fn:dword, p_stack:dword
              push ebx
              mov eax, 51
              mov ebx, 1
              mov ecx, [fn]
              mov edx,[p_stack]
                  int 0x40
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
           push ebx edi esi
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
           pop esi edi ebx
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
      
  push  ebx esi
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
  pop   esi ebx
  ret   24

align 4
_draw_bar@20:
;arg1 - x
;arg2 - y
;arg3 - xsize
;arg4 - ysize
;arg5 - color
  push  ebx
  mov   eax,13
  mov   ebx,[esp+8]
  shl   ebx,16
  mov   bx,[esp+16]
  mov   ecx,[esp+12]
  shl   ecx,16
  mov   cx,[esp+20]
  mov   edx,[esp+24]
  int   0x40
  pop   ebx
  ret   20

_write_text@20:
;arg1 - x
;arg2 - y
;arg3 - color
;arg4 - text
;arg5 - len
  push  ebx esi
  mov   eax,4
  mov   ebx,[esp+12]
  shl   ebx,16
  mov   bx,[esp+16]
  mov   ecx,[esp+20]
  mov   edx,[esp+24]
  mov   esi,[esp+28]
  int   0x40
  pop   esi ebx
  ret   20

align 4
proc _debug_out@4 stdcall, val:dword
           push ebx
           mov  ecx,[val]
           mov  ebx,1
           mov  eax,63
           int  0x40
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

public _allmul

_allmul: 
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

align 4
_allshr:
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

public __fltused
__fltused    dd 0
           
align 4
__hexdigits db '0123456789ABCDEF'

align 4
fileio FILEIO

