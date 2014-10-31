;строки с именами функций
op_table_str:
macro ADD_OP a,b,c
{
	db 'gl',`a,' ',c,0
}
include 'opinfo.inc'

;указатели на функции ;static void (*op_table_func[])(GLContext *,GLParam *)=
op_table_func:
macro ADD_OP a,b,c
{
	dd glop#a
}
include 'opinfo.inc'

;число параметров в функциях
op_table_size:
macro ADD_OP a,b,c
{
	dd b+1
}
include 'opinfo.inc'


;output:
; eax = context.shared_state.lists[list]
align 4
proc find_list uses ebx, context:dword, list:dword
	mov eax,[context]
	mov eax,[eax+offs_cont_shared_state]
	mov ebx,[list]
	shl ebx,2
	add eax,ebx
	mov eax,[eax]
	ret
endp

;static void delete_list(GLContext *c,int list)
;{
;  GLParamBuffer *pb,*pb1;
;  GLList *l;

;  l=find_list(c,list);
;  assert(l != NULL);

;  /* free param buffer */
;  pb=l->first_op_buffer;
;  while (pb!=NULL) {
;    pb1=pb->next;
;    gl_free(pb);
;    pb=pb1;
;  }

;  gl_free(l);
;  c->shared_state.lists[list]=NULL;
;}

;static GLList *alloc_list(GLContext *c,int list)
;{
;  GLList *l;
;  GLParamBuffer *ob;

;  l=gl_zalloc(sizeof(GLList));
;  ob=gl_zalloc(sizeof(GLParamBuffer));

;  ob->next=NULL;
;  l->first_op_buffer=ob;

;  ob->ops[0].op=OP_EndList;

;  c->shared_state.lists[list]=l;
;  return l;
;}

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
proc gl_compile_op uses eax ebx, context:dword, p:dword
	mov eax,[context]
;  int op,op_size;
;  GLParamBuffer *ob,*ob1;
;  int index,i;

;  op=p[0].op;
;  op_size=op_table_size[op];
;  index=c->current_op_buffer_index;
;  ob=c->current_op_buffer;

;  /* we should be able to add a NextBuffer opcode */
;  if ((index + op_size) > (OP_BUFFER_MAX_SIZE-2)) {

;    ob1=gl_zalloc(sizeof(GLParamBuffer));
;    ob1->next=NULL;

;    ob->next=ob1;
;    ob->ops[index].op=OP_NextBuffer;
;    ob->ops[index+1].p=(void *)ob1;

;    c->current_op_buffer=ob1;
;    ob=ob1;
;    index=0;
;  }

;  for(i=0;i<op_size;i++) {
;    ob->ops[index]=p[i];
;    index++;
;  }
;  c->current_op_buffer_index=index;
	ret
endp

align 4
proc gl_add_op uses eax ebx ecx, p:dword ;GLParam*
if DEBUG
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

	cmp dword[eax+offs_cont_exec_flag],0
	je @f
		push ebx
		push eax
		mov ecx,dword[ebx] ;ecx = OP_...
		shl ecx,2
		lea ebx,[op_table_func]
		add ecx,ebx
		call dword[ecx] ;op_table_func[op](c,p)
	@@:
	cmp dword[eax+offs_cont_compile_flag],0
	je @f
		stdcall gl_compile_op,eax,[p]
	@@:
	cmp dword[eax+offs_cont_print_flag],0
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

;void glopCallList(GLContext *c,GLParam *p)
;{
;  GLList *l;
;  int list,op;

;  list=p[1].ui;
;  l=find_list(c,list);
;  if (l == NULL) gl_fatal_error("list %d not defined",list);
;  p=l->first_op_buffer->ops;

;  while (1) {
;    op=p[0].op;
;    if (op == OP_EndList) break;
;    if (op == OP_NextBuffer) {
;      p=(GLParam *)p[1].p;
;    } else {
;      op_table_func[op](c,p);
;      p+=op_table_size[op];
;    }
;  }
;}

;void glNewList(unsigned int list,int mode)
;{
;  GLList *l;
;  GLContext *c=gl_get_context();
;
;  assert(mode == GL_COMPILE || mode == GL_COMPILE_AND_EXECUTE);
;  assert(c->compile_flag == 0);
;
;  l=find_list(c,list);
;  if (l!=NULL) delete_list(c,list);
;  l=alloc_list(c,list);
;
;  c->current_op_buffer=l->first_op_buffer;
;  c->current_op_buffer_index=0;
;  
;  c->compile_flag=1;
;  c->exec_flag=(mode == GL_COMPILE_AND_EXECUTE);
;}

;void glEndList(void)
;{
;  GLContext *c=gl_get_context();
;  GLParam p[1];

;  assert(c->compile_flag == 1);

;  /* end of list */
;  p[0].op=OP_EndList;
;  gl_compile_op(c,p);

;  c->compile_flag=0;
;  c->exec_flag=1;
;}

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

;unsigned int glGenLists(int range)
;{
;  GLContext *c=gl_get_context();
;  int count,i,list;
;  GLList **lists;

;  lists=c->shared_state.lists;
;  count=0;
;  for(i=0;i<MAX_DISPLAY_LISTS;i++) {
;    if (lists[i]==NULL) {
;      count++;
;      if (count == range) {
;       list=i-range+1;
;       for(i=0;i<range;i++) {
;         alloc_list(c,list+i);
;       }
;       return list;
;      }
;    } else {
;      count=0;
;    }
;  }
;  return 0;
;}
