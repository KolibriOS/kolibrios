;
; Texture Manager
;

align 4
proc find_texture uses ebx ecx, context:dword, h:dword
	mov ebx,[context]
	mov ebx,[ebx+offs_cont_shared_state+4] ;ebx = &texture_hash_table
	mov eax,[h]
	and eax,0xff
	shl eax,2
	add eax,[ebx] ;eax = &context.shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE]

	; [eax] - указатель на текстуру, получаемую через хеш таблицу
	mov ecx,[h] ; ecx - указатель на искомую текстуру
	@@:
		cmp dword[eax],0
		je .no_found
		mov ebx,[eax]
		cmp dword[ebx+offs_text_handle],ecx
		je .found
		mov eax,[ebx+offs_text_next]
		jmp @b
	.no_found:
	xor eax,eax ;ret NULL
	.found:
	ret
endp

;static void free_texture(GLContext *c,int h)
;{
;  GLTexture *t,**ht;
;  GLImage *im;
;  int i;
;
;  t=find_texture(c,h);
;  if (t->prev==NULL) {
;    ht=&c->shared_state.texture_hash_table
;      [t->handle % TEXTURE_HASH_TABLE_SIZE];
;    *ht=t->next;
;  } else {
;    t->prev->next=t->next;
;  }
;  if (t->next!=NULL) t->next->prev=t->prev;
;
;  for(i=0;i<MAX_TEXTURE_LEVELS;i++) {
;    im=&t->images[i];
;    if (im->pixmap != NULL) gl_free(im->pixmap);
;  }
;
;  gl_free(t);
;}

;output:
; eax - указатель на память
align 4
proc alloc_texture uses ebx ecx, context:dword, h:dword

	stdcall gl_zalloc,sizeof.GLTexture

	mov ebx,[context]
	mov ebx,[ebx+offs_cont_shared_state+4] ;ebx = &texture_hash_table
	mov ecx,[h]
	and ecx,0xff
	shl ecx,2
	add ecx,[ebx] ;ecx = &context.shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE]

	mov ebx,[ecx]
	mov [eax+offs_text_next],ebx
	mov dword[eax+offs_text_prev],0 ;NULL
	cmp dword[eax+offs_text_next],0 ;NULL
	je @f
		mov [eax+offs_text_prev],eax
	@@:
	mov [ecx],eax

	mov ebx,[h]
	mov [eax+offs_text_handle],ebx

	ret
endp

align 4
proc glInitTextures uses eax edx, context:dword
	; textures
	mov edx,[context]
	mov dword[edx+offs_cont_texture_2d_enabled],0
	stdcall find_texture,edx,0
	mov dword[edx+offs_cont_current_texture],eax
	ret
endp

;void glGenTextures(int n, unsigned int *textures)
;{
;  GLContext *c=gl_get_context();
;  int max,i;
;  GLTexture *t;
;
;  max=0;
;  for(i=0;i<TEXTURE_HASH_TABLE_SIZE;i++) {
;    t=c->shared_state.texture_hash_table[i];
;    while (t!=NULL) {
;      if (t->handle>max) max=t->handle;
;      t=t->next;
;    }
;
;  }
;  for(i=0;i<n;i++) {
;    textures[i]=max+i+1;
;  }
;}
;
;
;void glDeleteTextures(int n, const unsigned int *textures)
;{
;  GLContext *c=gl_get_context();
;  int i;
;  GLTexture *t;
;
;  for(i=0;i<n;i++) {
;    t=find_texture(c,textures[i]);
;    if (t!=NULL && t!=0) {
;      if (t==c->current_texture) {
;	glBindTexture(GL_TEXTURE_2D,0);
;      }
;      free_texture(c,textures[i]);
;    }
;  }
;}

align 4
proc glopBindTexture uses eax ebx edx, context:dword, p:dword
	mov ebx,[p]
	mov edx,[context]

;  assert(p[1].i == GL_TEXTURE_2D && texture >= 0);

	;[ebx+8] = p[2]
	stdcall find_texture, edx,dword[ebx+8]
	cmp eax,0 ;NULL
	jne @f
		stdcall alloc_texture, edx,dword[ebx+8]
	@@:
	mov [edx+offs_cont_current_texture],eax
	ret
endp

align 4
proc glopTexImage2D, context:dword, p:dword
;{
;  int target=p[1].i;
;  int level=p[2].i;
;  int components=p[3].i;
;  int width=p[4].i;
;  int height=p[5].i;
;  int border=p[6].i;
;  int format=p[7].i;
;  int type=p[8].i;
;  void *pixels=p[9].p;
;  GLImage *im;
;  unsigned char *pixels1;
;  int do_free;
;
;  if (!(target == GL_TEXTURE_2D && level == 0 && components == 3 && 
;        border == 0 && format == GL_RGB &&
;        type == GL_UNSIGNED_BYTE)) {
;    gl_fatal_error("glTexImage2D: combinaison of parameters not handled");
;  }
;  
;  do_free=0;
;  if (width != 256 || height != 256) {
;    pixels1 = gl_malloc(256 * 256 * 3);
;    /* no interpolation is done here to respect the original image aliasing ! */
;    gl_resizeImageNoInterpolate(pixels1,256,256,pixels,width,height);
;    do_free=1;
;    width=256;
;    height=256;
;  } else {
;    pixels1=pixels;
;  }
;
;  im=&c->current_texture->images[level];
;  im->xsize=width;
;  im->ysize=height;
;  if (im->pixmap!=NULL) gl_free(im->pixmap);
;#if TGL_FEATURE_RENDER_BITS == 24 
;  im->pixmap=gl_malloc(width*height*3);
;  if(im->pixmap) {
;      memcpy(im->pixmap,pixels1,width*height*3);
;  }
;#elif TGL_FEATURE_RENDER_BITS == 32
;  im->pixmap=gl_malloc(width*height*4);
;  if(im->pixmap) {
;      gl_convertRGB_to_8A8R8G8B(im->pixmap,pixels1,width,height);
;  }
;#elif TGL_FEATURE_RENDER_BITS == 16
;  im->pixmap=gl_malloc(width*height*2);
;  if(im->pixmap) {
;      gl_convertRGB_to_5R6G5B(im->pixmap,pixels1,width,height);
;  }
;#else
;#error TODO
;#endif
;  if (do_free) gl_free(pixels1);
	ret
endp

; TODO: not all tests are done
align 4
proc glopTexEnv, context:dword, p:dword

;  int target=p[1].i;
;  int pname=p[2].i;
;  int param=p[3].i;
;
;  if (target != GL_TEXTURE_ENV) {
;  error:
;    gl_fatal_error("glTexParameter: unsupported option");
;  }
;
;  if (pname != GL_TEXTURE_ENV_MODE) goto error;
;
;  if (param != GL_DECAL) goto error;
	ret
endp

; TODO: not all tests are done
align 4
proc glopTexParameter, context:dword, p:dword

;  int target=p[1].i;
;  int pname=p[2].i;
;  int param=p[3].i;
;  
;  if (target != GL_TEXTURE_2D) {
;  error:
;    gl_fatal_error("glTexParameter: unsupported option");
;  }
;
;  switch(pname) {
;  case GL_TEXTURE_WRAP_S:
;  case GL_TEXTURE_WRAP_T:
;    if (param != GL_REPEAT) goto error;
;    break;
;  }
	ret
endp

align 4
proc glopPixelStore, context:dword, p:dword

;  int pname=p[1].i;
;  int param=p[2].i;
;
;  if (pname != GL_UNPACK_ALIGNMENT ||
;      param != 1) {
;    gl_fatal_error("glPixelStore: unsupported option");
;  }
	ret
endp
