
align 4
proc glopClearColor uses ecx esi edi, context:dword, p:dword
	mov esi,[p]
	add esi,4
	mov edi,[context]
	add edi,offs_cont_clear_color
	mov ecx,4
	rep movsd
endp

align 4
proc glopClearDepth uses eax ebx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ebx,[ebx+4] ;ebx = p[1]
	mov dword[eax+offs_cont_clear_depth],ebx
	ret
endp

;void glopClear(GLContext *c,GLParam *p)
;{
;  int mask=p[1].i;
;  int z=0;
;  int r=(int)(c->clear_color.v[0]*65535);
;  int g=(int)(c->clear_color.v[1]*65535);
;  int b=(int)(c->clear_color.v[2]*65535);
;
;  /* TODO : correct value of Z */
;
;  ZB_clear(c->zb,mask & GL_DEPTH_BUFFER_BIT,z,
;	   mask & GL_COLOR_BUFFER_BIT,r,g,b);
;}

