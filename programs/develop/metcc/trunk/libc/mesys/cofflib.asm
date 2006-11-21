format ELF
include 'proc32.inc'
section '.text' executable

;extrn 'malloc' as malloc:dword
extrn 'strncmp' as strncmp:dword
extrn 'debug_out_str' as debug_out_str
;extrn 'free' as free:dword
;extrn 'realloc' as realloc:dword
;extrn 'mf_init' as mf_init:dword

public _msys_cofflib_load
;public _msys_cofflib_link
public _msys_cofflib_getproc

proc  _msys_cofflib_load stdcall, name:dword
	mov eax, 68
	mov ebx, 19
	mov ecx, [name]
	int 0x40
	ret
endp


;align 4
;proc _msys_cofflib_link stdcall, exp:dword, imp:dword
;           stdcall debug_out_str, szFuncLink
;           mov esi, [imp]
;           test esi, esi
;           jz .done
;.next:
;           lodsd
;           test eax, eax
;           jz .done
;           stdcall _msys_cofflib_getproc, [exp], eax
;           mov [esi-4], eax
;           jmp .next
;.done:
;           ret
;endp

align 4
proc _msys_cofflib_getproc stdcall, exp:dword, sz_name:dword
	   stdcall debug_out_str,  szFuncGetProc
	   mov edx, [exp]
.next:
	   test edx, edx
	   jz .end

	   stdcall strncmp, [edx], [sz_name], 16
	   test eax, eax
	   jz .ok

	   add edx,8
	   jmp .next
.ok:
	   mov eax, [edx+4]
.end:
	   ret
endp

section '.data'
   szFuncGetProc db 'somebody did call GetProc',0x0d, 0x0a,0
   szFuncLink db 'somebody did call Link',0x0d, 0x0a,0
