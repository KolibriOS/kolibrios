;строки с именами функций
op_table_str:
macro ADD_OP a,b,c
{
	db 'gl',`a,' ',c,0
}
include 'opinfo.inc'

;указатели на функции ;static void (*op_table_func[])(GLContext *,GLParam *)=
align 4
op_table_func:
macro ADD_OP a,b,c
{
	dd glop#a
}
include 'opinfo.inc'

;число параметров в функциях
align 4
op_table_size:
macro ADD_OP a,b,c
{
	dd b+1
}
include 'opinfo.inc'

;коды функций у которых нет входных параметров
align 4
macro ADD_OP a,b,c
{
if b eq 0
	op_#a dd OP_#a
end if
}
include 'opinfo.inc'


;output:
; eax = context.shared_state.lists[list]
align 4
proc find_list uses ebx, context:dword, list:dword
	mov eax,[context]
	mov eax,[eax+GLContext.shared_state]
	mov ebx,[list]
	lea eax,[eax+4*ebx]
	mov eax,[eax]
	ret
endp

align 4
proc delete_list uses eax ebx ecx edx, context:dword, list:dword
	mov ebx,[context]
	stdcall find_list,ebx,[list]
	mov edx,eax
;  assert(l != NULL);

	; free param buffer
	mov eax,[edx] ;eax = GLList.first_op_buffer
	@@:
	cmp eax,0
	je .end_w
		mov ecx,[eax+offs_gpbu_next]
		stdcall gl_free,eax
		mov eax,ecx
		jmp @b
	.end_w:

	stdcall gl_free,edx
	mov ebx,[ebx+GLContext.shared_state] ;ebx = &context.shared_state.lists
	mov ecx,[list]
	lea ebx,[ebx+4*ecx]
	mov dword[ebx],0 ;=NULL
	ret
endp

align 4
proc alloc_list uses ebx ecx, context:dword, list:dword
	stdcall gl_zalloc,sizeof.GLParamBuffer
	mov ecx,eax
	stdcall gl_zalloc,sizeof.GLList

	mov dword[ecx+offs_gpbu_next],0 ;ob.next=NULL
	mov dword[eax],ecx ;l.first_op_buffer=ob

	mov dword[ecx+offs_gpbu_ops],OP_EndList ;ob.ops[0].op=OP_EndList

	mov ebx,[context]
	mov ebx,[ebx+GLContext.shared_state]
	mov ecx,[list]
	lea ebx,[ebx+4*ecx]
	mov [ebx],eax ;context.shared_state.lists[list]=l
	ret
endp

;void gl_print_op(FILE *f,GLParam *p)
;{
;  int op;
;  char *s;

;  op=p[0].op;
;  p++;
;  s=op_table_str[op];
;  while (*s != 0) {
;    if (*s == '%') {
;      s++;
;      switch (*s++) {
;      case 'f':
;       fprintf(f,"%g",p[0].f);
;       break;
;      default:
;       fprintf(f,"%d",p[0].i);
;       break;
;      }
;      p++;
;    } else {
;      fputc(*s,f);
;      s++;
;    }
;  }
;  fprintf(f,"\n");
;}

align 4
proc gl_compile_op, context:dword, p:dword
pushad
	mov edx,[context]

	mov ecx,[p]
	mov ecx,[ecx] ;код операции
	lea ecx,[op_table_size+4*ecx]
	mov ecx,[ecx] ;ecx = кол-во параметров в компилируемой функции
	mov ebx,[edx+GLContext.current_op_buffer_index]
	mov eax,[edx+GLContext.current_op_buffer]

	; we should be able to add a NextBuffer opcode
	lea esi,[ebx+ecx]
	cmp esi,(OP_BUFFER_MAX_SIZE-2)
	jle @f
		mov edi,eax
		stdcall gl_zalloc,sizeof.GLParamBuffer
		mov dword[eax+offs_gpbu_next],0 ;=NULL

		mov dword[edi+offs_gpbu_next],eax
		lea esi,[edi+4*ebx]
		mov dword[esi+offs_gpbu_ops],OP_NextBuffer
		mov dword[esi+offs_gpbu_ops+4],eax

		mov dword[edx+GLContext.current_op_buffer],eax
		xor ebx,ebx
	@@:

	mov esi,[p]
	@@:
		lea edi,[eax+4*ebx]
		movsd
		inc ebx
	loop @b
	mov dword[edx+GLContext.current_op_buffer_index],ebx
popad
	ret
endp

align 4
proc gl_add_op uses eax ebx ecx, p:dword ;GLParam*
if DEBUG ;gl_add_op
push edi esi
	mov ebx,[p]
	mov ebx,[ebx]
	lea eax,[op_table_str]
	@@:
		cmp ebx,0
		je @f
		cmp byte[eax],0
		jne .no_dec
			dec ebx
		.no_dec:
		inc eax
		jmp @b
	@@:
	stdcall dbg_print,eax,txt_nl

	mov esi,eax
	mov word[NumberSymbolsAD],3
	mov ebx,[p]
	lea edi,[buf_param]
	mov byte[edi],0
	mov ecx,80
	.cycle_0:
		cmp byte[esi],'%'
		jne .no_param
			cmp ebx,[p]
			je @f
				stdcall str_n_cat,edi,txt_zp_sp,2
				stdcall str_len,edi
				add edi,eax
			@@:
			add ebx,4
			inc esi

			cmp byte[esi],'f'
			jne @f
				fld dword[ebx]
				fstp qword[Data_Double]
				call DoubleFloat_to_String
				stdcall str_cat, edi,Data_String
			@@:
			cmp byte[esi],'d'
			jne @f
				stdcall str_len,edi
				add edi,eax
				sub ecx,eax
				mov eax,dword[ebx]
				stdcall convert_int_to_str,ecx
			@@:
		.no_param:
		inc esi
		cmp byte[esi],0
		jne .cycle_0
	stdcall str_cat, edi,txt_nl
	stdcall dbg_print,txt_sp,buf_param
pop esi edi
end if
	call gl_get_context
	mov ebx,[p]

	cmp dword[eax+GLContext.exec_flag],0
	je @f
		push ebx
		push eax
		mov ecx,dword[ebx] ;ecx = OP_...
		shl ecx,2
		lea ebx,[op_table_func]
		add ecx,ebx
		call dword[ecx] ;op_table_func[op](c,p)
	@@:
	call gl_get_context
	cmp dword[eax+GLContext.compile_flag],0
	je @f
		stdcall gl_compile_op,eax,[p]
	@@:
	cmp dword[eax+GLContext.print_flag],0
	je @f
		;gl_print_op(stderr,p);
	@@:
	ret
endp

; this opcode is never called directly
align 4
proc glopEndList, context:dword, p:dword
;  assert(0);
	ret
endp

; this opcode is never called directly
align 4
proc glopNextBuffer, context:dword, p:dword
;  assert(0);
	ret
endp

align 4
proc glopCallList uses eax ebx ecx edx edi, context:dword, p:dword
	mov edx,[context]
	mov ebx,[p]

	stdcall find_list,edx,[ebx+4]
	or eax,eax
	jnz @f
		;gl_fatal_error("list %d not defined",[ebx+4])
	@@:
	mov edi,[eax] ;edi = &GLList.first_op_buffer.ops

align 4
	.cycle_0: ;while (1)
	cmp dword[edi],OP_EndList
	je .end_f ;if (op == OP_EndList) break
	cmp dword[edi],OP_NextBuffer
	jne .els_0 ;if (op == OP_NextBuffer)
		mov edi,[edi+4] ;p=p[1].p
		jmp .cycle_0
	.els_0:
		mov ecx,dword[edi] ;ecx = OP_...
		shl ecx,2
		lea ebx,[op_table_func]
		add ecx,ebx
		stdcall dword[ecx],edx,edi ;op_table_func[op](context,p)

		mov ecx,dword[edi] ;ecx = OP_...
		shl ecx,2
		lea ebx,[op_table_size]
		add ecx,ebx
		mov ecx,[ecx]
		shl ecx,2
		add edi,ecx ;edi += op_table_size[op]
	jmp .cycle_0
	.end_f:
	ret
endp

align 4
proc glNewList uses eax ebx, list:dword, mode:dword
	call gl_get_context
	mov ebx,eax

;  assert(mode == GL_COMPILE || mode == GL_COMPILE_AND_EXECUTE);
;  assert(ebx->compile_flag == 0);

	stdcall find_list,ebx,[list]
	cmp eax,0
	je @f
		stdcall delete_list,ebx,[list]
	@@:
	stdcall alloc_list,ebx,[list]

	mov eax,[eax] ;eax = GLList.first_op_buffer
	mov [ebx+GLContext.current_op_buffer],eax
	mov dword[ebx+GLContext.current_op_buffer_index],0

	mov dword[ebx+GLContext.compile_flag],1
	xor eax,eax
	cmp dword[mode],GL_COMPILE_AND_EXECUTE
	jne @f
		inc eax ;eax = (mode == GL_COMPILE_AND_EXECUTE)
	@@:
	mov [ebx+GLContext.exec_flag],eax
	ret
endp

align 4
proc glEndList uses eax
	call gl_get_context

;  assert(c->compile_flag == 1);

	; end of list
	stdcall gl_compile_op,eax,op_EndList

	mov dword[eax+GLContext.compile_flag],0
	mov dword[eax+GLContext.exec_flag],1
	ret
endp

;output:
; eax = (find_list(gl_get_context,list) != NULL)
align 4
proc glIsList, list:dword
	call gl_get_context
	stdcall find_list, eax,[list]
	cmp eax,0 ;NULL
	je @f
		mov eax,1
	@@:
	ret
endp

align 4
proc glGenLists uses ebx ecx edx edi esi, range:dword
	call gl_get_context
	mov edi,eax

	mov ebx,[eax+GLContext.shared_state] ;ebx=context.shared_state.lists
	xor edx,edx ;count=0
	mov ecx,MAX_DISPLAY_LISTS
	xor esi,esi
	.cycle_0: ;for(esi=0;esi<MAX_DISPLAY_LISTS;esi++)
		cmp dword[ebx],0 ;if (ebx[i]==NULL)
		jne .els_0
			inc edx
			cmp edx,[range] ;if (count == range)
			jne .els_1
			mov ecx,[range]
			inc esi
			sub esi,ecx ;esi = (esi-range+1)
			.cycle_1: ;for(i=0;i<range;i++)
				stdcall alloc_list,edi,esi
				inc esi
			loop .cycle_1
			mov eax,esi
			jmp .end_f
		.els_0:
			xor edx,edx ;count=0
		.els_1:
		add ebx,4
		inc esi
	loop .cycle_0
	xor eax,eax
	.end_f:
	ret
endp
