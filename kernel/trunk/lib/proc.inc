macro export [name,sname]
{
  forward
    dd __#name#_export_name__,name
  common
    dd 0
  forward
    align 4
    __#name#_export_name__ db sname,0
}

copy_s:
  ; eax - куда
  ; ebx - откуда
  ; cl - до чего
  ; выход - eax - сколько скопировано
  pushad
  push eax
@@:
 mov ch,[ebx]
 cmp ch,cl
 je @f
 mov [eax],ch
 inc ebx
 inc eax
 jmp @b
@@:
 mov [eax], byte 0
 pop ebx
 sub eax,ebx
 mov [esp+0x1c],eax
 popad
 ret

strcmp:
  ; esi - строка 1
  ; edi - строка 2
  ; выход - eax=-1 - неравно
		xor	eax,eax
		pushad
	    @@: lodsb
		scasb
		jne	.fail
		or	al,al
		jnz	@b
		jmp	.ok
  .fail:	or	eax,-1
		mov [esp+0x1c],eax
  .ok:		 popad
		ret



proc get_filesize stdcall, file_name:dword
	   locals
	     cmd     dd ?
	     offset  dd ?
			     dd ?
	     count   dd ?
	     buff    dd ?
			 db ?
	     name    dd ?

	     info	dd ?
	     flags	dd ?
	     cr_time	dd ?
	     cr_date	dd ?
	     acc_time	dd ?
	     acc_date	dd ?
	     mod_time	dd ?
	     mod_date	dd ?
	     file_size	dd ?

	     file	dd ?
	     file2	dd ?


	   endl

	   xor eax, eax
	   mov ebx, [file_name]
	   mov ecx,info-ebp
	   add ecx,ebp

	   mov [cmd], 5
	   mov [offset], eax
	   mov [offset+4], eax
	   mov [count], eax
	   mov [buff], ecx
	   mov byte [buff+4], al
	   mov [name], ebx

	   mov eax, 70
	   lea ebx, [cmd]
	   int 0x40

	   mov eax,[file_size]

	   ret
endp