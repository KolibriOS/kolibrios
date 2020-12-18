format MS COFF
public EXPORTS
section '.flat' code readable align 16

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../KOSfuncs.inc'

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
include 'get.asm'
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
f_fill_tr_nl db ' lines',0
f_zbz db ' ZB_line_z',0
f_zb db ' ZB_line',0

buf_param rb 80

align 4
proc str_n_cat uses eax ecx edi esi, str1:dword, str2:dword, n_len:dword
	mov esi,[str2]
	mov ecx,[n_len]
	mov edi,[str1]
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
	mov ecx,10
	cmp eax,ecx
	jb @f
		xor edx,edx
		div ecx
		push edx
		call .str
		pop eax
	@@:
	cmp edi,esi
	jge @f
		or al,0x30
		stosb
		mov byte[edi],0
	@@:
	ret
end if

; ***
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
err_glGet db 'glGet: option not implemented',0

align 4
proc dbg_print, fun:dword, mes:dword
pushad
	mov eax,SF_BOARD
	mov ebx,SSF_DEBUG_WRITE

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
