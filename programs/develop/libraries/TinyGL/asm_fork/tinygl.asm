format MS COFF
public EXPORTS
section '.flat' code readable align 16

include '../../../../../programs/proc32.inc'
include '../../../../../programs/macros.inc'

DEBUG equ 0

include 'zgl.inc'
include 'zmath.asm'
include 'clip.asm'
include 'vertex.asm'
include 'api.asm'
include 'list.asm' ;gl_add_op
include 'init.asm'
include 'matrix.asm' ;gl_print_matrix
include 'texture.asm'
include 'misc.asm'
include 'clear.asm'
include 'light.asm'
include 'select.asm'
;include 'get.asm'
;include 'error.asm'
include 'zbuffer.asm'
include 'zline.asm'
;include 'zdither.asm'
include 'ztriangle.asm'
include 'image_util.asm'
;include 'msghandling.asm'
include 'arrays.asm'
include 'specbuf.asm'

include 'kosgl.asm'
include 'glu.asm'

if DEBUG
include 'info_fun_float.inc'

align 4
txt_nl db 13,10,0
txt_sp db ' ',0
txt_op db 'Op_code',0
txt_zp_sp db ', ',0
m_1 db '(1)',13,10,0
m_2 db '(2)',13,10,0
m_3 db '(3)',13,10,0
m_4 db '(4)',13,10,0
m_5 db '(5)',13,10,0
m_6 db '(6)',13,10,0
m_7 db '(7)',13,10,0
m_8 db '(8)',13,10,0
m_9 db '(9)',13,10,0

buf_param rb 80

align 4
proc str_n_cat uses eax ecx edi esi, str1:dword, str2:dword, n_len:dword
	mov esi,dword[str2]
	mov ecx,dword[n_len]
	mov edi,dword[str1]
	stdcall str_len,edi
	add edi,eax
	cld
	repne movsb
	mov byte[edi],0
	ret
endp

;input:
; eax - число
; edi - буфер для строки
; len - длинна буфера
;output:
align 4
proc convert_int_to_str, len:dword
pushad
	mov esi,[len]
	add esi,edi
	dec esi
	call .str
popad
	ret
endp

align 4
.str:
	mov ecx,0x0a ;задается система счисления изменяются регистры ebx,eax,ecx,edx входные параметры eax - число
	;преревод числа в ASCII строку входные данные ecx=система счисленя edi адрес куда записывать, будем строку, причем конец переменной 
	cmp eax,ecx ;сравнить если в eax меньше чем в ecx то перейти на @@-1 т.е. на pop eax
	jb @f
		xor edx,edx ;очистить edx
		div ecx   ;разделить - остаток в edx
		push edx  ;положить в стек
		call .str ;перейти на саму себя т.е. вызвать саму себя и так до того момента пока в eax не станет меньше чем в ecx
		pop eax
	@@: ;cmp al,10 ;проверить не меньше ли значение в al чем 10 (для системы счисленя 10 данная команда - лишная))
	cmp edi,esi
	jge @f
		or al,0x30 ;данная команда короче  чем две выше
		stosb	   ;записать элемент из регистра al в ячеку памяти es:edi
		mov byte[edi],0 ;в конец строки ставим 0, что-бы не вылазил мусор
	@@:
	ret	   ;пока в стеке храниться кол-во вызовов то столько раз мы и будем вызываться
end if

; ***
glVertex2d: ;(double ,double)
glVertex2dv: ;(double *)
glVertex3d: ;(double ,double ,double)
glVertex3dv: ;(double *)
glVertex4d: ;(double ,double ,double, double )
glVertex4dv: ;(double *)
glColor3d: ;(double ,double ,double)
glColor3dv: ;(double *)
glColor4d: ;(double ,double ,double, double )
glColor4dv: ;(double *)
glNormal3d: ;(double ,double ,double)
glNormal3dv: ;(double *)
glTexCoord1f: ;(float)
glTexCoord1d: ;(double)
glTexCoord1fv: ;(float *)
glTexCoord1dv: ;(double *)
glTexCoord2d: ;(double ,double)
glTexCoord2dv: ;(double *)
glTexCoord3f: ;(float ,float ,float)
glTexCoord3d: ;(double ,double ,double)
glTexCoord3fv: ;(float *)
glTexCoord3dv: ;(double *)
glTexCoord4d: ;(double ,double ,double, double )
glTexCoord4fv: ;(float *)
glTexCoord4dv: ;(double *)
glGetIntegerv: ;(int pname,int *params)
glGetFloatv: ;(int pname, float *v)

align 4
err_0 db 'Error while initializing Z buffer',13,10,0
f_zb_opn db ' ZB_open',0
err_1 db 'gl_malloc(sizeof.ZBuffer)==0',13,10,0
err_2 db 'gl_malloc(xsize*ysize*4)==0',13,10,0
err_3 db 'bit mode not correct',13,10,0
err_4 db 'error while resizing display',13,10,0
err_5 db 'size too small',13,10,0
err_6 db 'unsupported option',13,10,0
err_7 db 'assert(target == GL_TEXTURE_2D && texture >= 0)',13,10,0
err_8 db 'combinaison of parameters not handled',13,10,0
err_9 db 'GL_INVALID_ENUM',13,10,0
f_zbz db ' ZB_line_z',0
f_zb db ' ZB_line',0
f_find_l db 'find_list',0
f_alloc_l db 'alloc_list',0
f_fill_tr db 'ZB_fillTriangle...',0
f_fill_tr_nl db ' lines',0
f_fill_tr_nll db ' len',0

align 4
proc dbg_print, fun:dword, mes:dword
pushad
	mov eax,63
	mov ebx,1

	mov esi,[fun]
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
	mov cl,':'
	int 0x40
	mov cl,' '
	int 0x40
	mov esi,[mes]
	@@:
		mov cl,byte[esi]
		int 0x40
		inc esi
		cmp byte[esi],0
		jne @b
popad
	ret
endp

align 16
EXPORTS:
macro E_LIB n
{
	dd sz_#n, n
}
include 'export.inc'
	dd 0,0
macro E_LIB n
{
	sz_#n db `n,0
}
include 'export.inc'
