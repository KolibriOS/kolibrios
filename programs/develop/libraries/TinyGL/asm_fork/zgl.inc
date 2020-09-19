include 'opengl_const.inc'
include 'zbuffer.inc'
include 'zmath.inc'

offs_X equ 0
offs_Y equ 4
offs_Z equ 8
offs_W equ 12

;enum { OP_ ## a , ... }
sum1 equ 0
macro ADD_OP a,b,c
{
	OP_#a equ sum1
	sum1 equ (sum1+1)
}
include 'opinfo.inc'


;initially # of allocated GLVertexes (will grow when necessary)
POLYGON_MAX_VERTEX equ 16

;Max # of specular light pow buffers
MAX_SPECULAR_BUFFERS equ 8
;# of entries in specular buffer
SPECULAR_BUFFER_SIZE equ 1024
;specular buffer granularity
SPECULAR_BUFFER_RESOLUTION equ 1024


MAX_MODELVIEW_STACK_DEPTH  equ 32
MAX_PROJECTION_STACK_DEPTH equ 8
MAX_TEXTURE_STACK_DEPTH    equ 8
MAX_NAME_STACK_DEPTH equ 64
MAX_TEXTURE_LEVELS   equ 11
MAX_LIGHTS equ 16

VERTEX_HASH_SIZE equ 1031

MAX_DISPLAY_LISTS equ 1024
OP_BUFFER_MAX_SIZE equ 512

TGL_OFFSET_FILL    equ 0x1
TGL_OFFSET_LINE    equ 0x2
TGL_OFFSET_POINT   equ 0x4

struct GLSpecBuf
	shininess_i dd ? ;int
	last_used dd ? ;int
	buf rd SPECULAR_BUFFER_SIZE+1 ;float[SPECULAR_BUFFER_SIZE+1]
	next dd ? ;struct GLSpecBuf*
ends

offs_spec_shininess_i equ 0
offs_spec_last_used equ 4
offs_spec_buf equ 8
offs_spec_next equ 8+4*(SPECULAR_BUFFER_SIZE+1)

struct GLLight
	ambient V4
	diffuse V4
	specular V4
	position V4	
	spot_direction V3
	spot_exponent dd ? ;float
	spot_cutoff dd ? ;float
	attenuation rd 3 ;float[3]
	; precomputed values
	cos_spot_cutoff dd ? ;float
	norm_spot_direction V3
	norm_position V3
	; we use a linked list to know which are the enabled lights
	enabled dd ? ;int
	next dd ? ;struct GLLight*
	prev dd ? ;struct GLLight*
ends

struct GLMaterial
	emission V4
	ambient  V4
	diffuse  V4
	specular V4
	shininess dd ? ;float

	; computed values
	shininess_i dd ? ;int
	do_specular dd ? ;int
ends

struct GLViewport
	xmin dd ? ;int
	ymin dd ? ;int
	xsize dd ? ;int
	ysize dd ? ;int
	scale V3
	trans V3
	updated dd ? ;int
ends

struct GLParamBuffer
	ops rd OP_BUFFER_MAX_SIZE ;GLParam[OP_BUFFER_MAX_SIZE]
	next dd ? ;struct GLParamBuffer*
ends

offs_gpbu_ops equ 0
offs_gpbu_next equ 4*OP_BUFFER_MAX_SIZE

struct GLList
	first_op_buffer dd ? ;GLParamBuffer*
	; TODO: extensions for an hash table or a better allocating scheme
ends

struct GLVertex
	edge_flag dd ? ;int
	normal V3
	coord V4
	tex_coord V4
	color V4

	; computed values
	ec V4 ; eye coordinates
	pc V4 ; coordinates in the normalized volume
	clip_code dd ? ;int ; clip code
	zp ZBufferPoint ; integer coordinates for the rasterization
ends

struct GLImage
	pixmap dd ? ;void*
	xsize dd ? ;int
	ysize dd ? ;int
	xsize_log2 dd ? ;unsigned int
	s_bound dd ? ;unsigned int
	t_bound dd ? ;unsigned int
ends

offs_imag_pixmap equ 0
offs_imag_xsize equ 4
offs_imag_ysize equ 8
offs_imag_xsize_log2 equ 12
offs_imag_s_bound equ 16
offs_imag_t_bound equ 20

; textures

TEXTURE_HASH_TABLE_SIZE equ 256 ;должно быть кратное 2, в коде берется остаток от деления (через and быстрее чем div)

struct GLTexture
	images rb sizeof.GLImage * MAX_TEXTURE_LEVELS ;GLImage[MAX_TEXTURE_LEVELS]
	handle dd ? ;int
	next dd ? ;struct GLTexture*
	prev dd ? ;struct GLTexture*
ends

offs_text_images equ 0
offs_text_handle equ sizeof.GLImage*MAX_TEXTURE_LEVELS
offs_text_next equ 4+offs_text_handle
offs_text_prev equ 8+offs_text_handle

; shared state

struct GLSharedState
	lists dd ? ;GLList**
	texture_hash_table dd ? ;GLTexture**
ends


; display context

struct GLContext
	; Z buffer
	zb dd ? ;ZBuffer*

	; lights
	lights rb sizeof.GLLight * MAX_LIGHTS ;GLLight[MAX_LIGHTS]
	first_light dd ? ;GLLight*
	ambient_light_model V4
	local_light_model dd ? ;int
	lighting_enabled dd ? ;int
	light_model_two_side dd ? ;int

	; materials
	materials rb sizeof.GLMaterial * 2 ;GLMaterial[2]
	color_material_enabled dd ? ;int
	current_color_material_mode dd ? ;int
	current_color_material_type dd ? ;int

	; textures
	current_texture dd ? ;GLTexture*
	texture_2d_enabled dd ? ;int

	; shared state
	shared_state GLSharedState

	; current list
	current_op_buffer dd ? ;GLParamBuffer*
	current_op_buffer_index dd ? ;int
	exec_flag dd ? ;int
	compile_flag dd ? ;int
	print_flag dd ? ;int

	; matrix

	matrix_mode dd ? ;int режим активного вида матрицы (один из 3-х: GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE)
	matrix_stack rd 3 ;*M4[3] указатель на начало массива матриц
	matrix_stack_ptr rd 3 ;*M4[3] указатель на активную матрицу из массива
	matrix_stack_depth_max rd 3 ;int[3] максимальное число матриц в массивах matrix_stack

	matrix_model_view_inv M4
	matrix_model_projection M4
	matrix_model_projection_updated dd ? ;int
	matrix_model_projection_no_w_transform dd ? ;int 
	apply_texture_matrix dd ? ;int

	; viewport
	viewport GLViewport

	; current state
	polygon_mode_back dd ? ;int
	polygon_mode_front dd ? ;int

	current_front_face dd ? ;int
	current_shade_model dd ? ;int
	current_cull_face dd ? ;int
	cull_face_enabled dd ? ;int
	normalize_enabled dd ? ;int
	draw_triangle_front dd ? ;gl_draw_triangle_func
	draw_triangle_back  dd ? ;gl_draw_triangle_func

	; selection
	render_mode dd ? ;int
	select_buffer dd ? ;unsigned int*
	select_size dd ? ;int
	select_ptr dd ? ;unsigned int*
	select_hit dd ? ;unsigned int*
	select_overflow dd ? ;int
	select_hits dd ? ;int

	; names
	name_stack rd MAX_NAME_STACK_DEPTH ;unsigned int[MAX_NAME_STACK_DEPTH]
	name_stack_size dd ? ;int

	; clear
	clear_depth dd ? ;float
	clear_color V4

	; current vertex state
	current_color V4
	longcurrent_color rd 3 ;unsigned int[3] ;precomputed integer color
	current_normal V4
	current_tex_coord V4
	current_edge_flag dd ? ;int

	; glBegin / glEnd
	in_begin dd ? ;int
	begin_type dd ? ;int
	vertex_n dd ? ;int
	vertex_cnt dd ? ;int
	vertex_max dd ? ;int
	vertex dd ? ;GLVertex*

	; opengl 1.1 arrays
	vertex_array dd ? ;float*
	vertex_array_size dd ? ;int
	vertex_array_stride dd ? ;int
	normal_array dd ? ;float*
	normal_array_stride dd ? ;int
	color_array dd ? ;float*
	color_array_size dd ? ;int
	color_array_stride dd ? ;int
	texcoord_array dd ? ;float*
	texcoord_array_size dd ? ;int
	texcoord_array_stride dd ? ;int
	client_states dd ? ;int

	; opengl 1.1 polygon offset
	offset_factor dd ? ;float
	offset_units dd ? ;float
	offset_states dd ? ;int

	; specular buffer. could probably be shared between contexts, 
	; but that wouldn't be 100% thread safe
	specbuf_first dd ? ;GLSpecBuf*
	specbuf_used_counter dd ? ;int
	specbuf_num_buffers dd ? ;int

	; opaque structure for user's use
	opaque dd ? ;void*
	; resize viewport function
	gl_resize_viewport dd ? ;(struct GLContext *c,int *xsize,int *ysize)

	; depth test
	depth_test dd ? ;int
ends

align 16
gl_ctx dd ? ;extern GLContext*

align 16
proc gl_get_context
	mov eax,[gl_ctx]
	ret
endp

; this clip epsilon is needed to avoid some rounding errors after
; several clipping stages

CLIP_EPSILON dd 1.0e-5

align 4
proc gl_clipcode uses ebx, x:dword, y:dword, z:dword, w1:dword
	xor ebx,ebx

	fld1
	fadd dword[CLIP_EPSILON]
	fmul dword[w1]

	fcom dword[x]
	fstsw ax
	sahf
	ja @f
		or ebx,2
	@@:
	fcom dword[y]
	fstsw ax
	sahf
	ja @f
		or ebx,8
	@@:
	fcom dword[z]
	fstsw ax
	sahf
	ja @f
		or ebx,32
	@@:

	fchs
	fcom dword[x]
	fstsw ax
	sahf
	jbe @f
		or ebx,1
	@@:
	fcom dword[y]
	fstsw ax
	sahf
	jbe @f
		or ebx,4
	@@:
	fcom dword[z]
	fstsw ax
	sahf
	jbe @f
		or ebx,16
	@@:

	ffree st0
	fincstp

	mov eax,ebx
	ret
endp

;input:
; rf,gf,bf - значения float
; ri,gi,bi - адреса куда будут записаны rf,gf,bf преобразованые в int
align 4
proc RGBFtoRGBI uses eax, rf:dword,gf:dword,bf:dword, ri:dword,gi:dword,bi:dword
locals
	s dd ?
endl
	mov dword[s],(ZB_POINT_RED_MAX - ZB_POINT_RED_MIN)
	fild dword[s]
	fmul dword[rf]
	mov eax,[ri]
	fistp dword[eax]
	add dword[eax],ZB_POINT_RED_MIN

	mov dword[s],(ZB_POINT_GREEN_MAX - ZB_POINT_GREEN_MIN)
	fild dword[s]
	fmul dword[gf]
	mov eax,[gi]
	fistp dword[eax]
	add dword[eax],ZB_POINT_GREEN_MIN

	;bi = (unsigned int) (bf * (ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN) + ZB_POINT_BLUE_MIN);
	mov dword[s],(ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN)
	fild dword[s]
	fmul dword[bf]
	mov eax,[bi]
	fistp dword[eax]
	add dword[eax],ZB_POINT_BLUE_MIN
	ret
endp
