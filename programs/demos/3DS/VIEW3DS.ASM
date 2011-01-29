
; application : View3ds ver. 0.054 - tiny .3ds files viewer.
; compiler    : FASM  1.67.11
; system      : KolibriOS/MenuetOS
; author      : Macgub aka Maciej Guba
; email       : macgub3@wp.pl
; web         : www.menuet.xt.pl
; Fell free to use this intro in your own distribution of KolibriOS/MenuetOS.
; Special greetings to all MenuetOS maniax in the world.
; I hope because my intros Christian Belive will be near to each of You.


; Some adjustments made by Madis Kalme
; madis.kalme@mail.ee
; I tried optimizing it a bit, but don't know if it was successful. The objects
; can be:
; 1) Read from a file (*.3DS standard)
; 2) Written in manually (at the end of the code)
;include 'proc32.inc'

SIZE_X equ 512
SIZE_Y equ 512				       ;      /////     I want definitely
TIMEOUT equ 10				       ;     ------     say:
ROUND equ 10				       ;     \ @ @/     keep smiling every
TEX_X equ 512	  ; texture width              ;      \ ./    / day.
TEX_Y equ 512	  ;         height             ;       \/    /
TEX_SHIFT equ 9  ; texture width shifting      ;     __||__ /
TEXTURE_SIZE EQU (TEX_X * TEX_Y)-1	       ;   /|      |
TEX equ  SHIFTING ;  TEX={SHIFTING | FLUENTLY} ;  / \      /
FLUENTLY = 0				       ; /   |    |
SHIFTING = 1				       ;     ------
CATMULL_SHIFT equ 8			       ;      |  |
LIGHT_SIZE equ 22			       ;      |  |
NON   =   0				       ;     -/  \-
MMX   =   1
SSE   =   2
Ext   =   MMX			;Ext={ NON | MMX | SSE}

; 0 for short names (Menuet-compatible), 1 for long names (Kolibri features)
USE_LFN = 1

use32
	org    0x0
	db     'MENUET01'	; 8 byte id
	dd     0x01		; header version
	dd     START		; start of code
	dd     I_END		; size of image
	dd     MEM_END		; memory for app
	dd     MEM_END		; esp
	dd     I_Param		; I_Param
	dd     0x0		; I_Icon

START:	  ; start of execution
	cld
	call   alloc_buffer_mem
	call   read_param
	call   read_from_disk	 ; read, if all is ok eax = 0
	cmp    eax,0
	jne    .gen
	call   read_tp_variables ; init points and triangles count variables
	cmp    eax,0
	je     .gen
	jmp    .malloc
    .gen:
     if USE_LFN
	mov    [triangles_count_var],1000
	mov    [points_count_var],1000
	call   alloc_mem_for_tp
     end if
	call   generate_object
	jmp    .opt
    .malloc:
     if USE_LFN
	call   alloc_mem_for_tp
     end if
	call   read_from_file
    .opt:
	call   optimize_object1     ;  proc in file b_procs.asm
				    ;  set point(0,0,0) in center and  calc all coords
				    ;  to be in <-1.0,1.0>
	call   normalize_all_light_vectors
	call   init_triangles_normals2
	call   init_point_normals
   ;    call   init_envmap2
	call   init_envmap_cub
	call   generate_texture2
	call   init_sincos_tab

	call   do_color_buffer	 ; intit color_map
	mov    edi,bumpmap
	call   calc_bumpmap
	call   calc_bumpmap_coords   ; bump and texture mapping
	call   draw_window


still:

	mov	eax,23		; wait here for event with timeout
	mov	ebx,TIMEOUT
	cmp	[speed_flag],1
	jne	.skip
	mov	eax,11
    .skip:
	int	0x40

	cmp	eax,1		; redraw request ?
	je	red
	cmp	eax,2		; key in buffer ?
	je	key
	cmp	eax,3		; button in buffer ?
	je	button

	jmp	noclose

    red:			; redraw
	call	draw_window

	jmp	noclose

    key:			; key
	mov	eax,2		; just read it and ignore
	int	0x40
	jmp	noclose

    button:			; button
	mov	eax,17		; get id
	int	0x40

	cmp	ah,1		; button id=1 ?
	jne	@f

	mov	eax,-1		; close this program
	int	0x40
    @@:
	cmp	ah,30
	jge	add_vec_buttons
	call	update_flags	      ; update flags and write labels of flags

				      ; do other operations according to flag
	cmp	ah,3		      ; ah = 3 -> shading model
	jne	.next_m6
	cmp	[dr_flag],2
	jne	@f
	call	init_envmap2
 ;       call    init_envmap_cub2
     @@:
	cmp	[dr_flag],4
	jne	@f
	call	generate_texture2

     @@:
     .next_m6:
				      ; ah = 5 -> scale-
	cmp	ah,5
	jne	@f
	mov	[scale],0.7
	fninit
	fld	[rsscale]
	fmul	[scale]
	fstp	[rsscale]

      @@:
	cmp	ah,6		     ; ah = 6 ->  scale+
	jne	@f
	mov	[scale],1.3
	fninit
	fld	[rsscale]
	fmul	[scale]
	fstp	[rsscale]

      @@:
	cmp	ah,9	; lights random                 ;    'flat'  0
	jne	.next_m5				;    'grd '  1
	call	make_random_lights			;    'env '  2
	call	normalize_all_light_vectors		;    'bump'  3
	call	do_color_buffer   ; intit color_map     ;    'tex '  4
	cmp	[emboss_flag],1 			;    'pos '  5
	je	@f					;    'dots'  6
	cmp	[dr_flag],8
	jge	@f
	cmp	[dr_flag],2				;    'txgr'  7
	jl	.next_m5			    ;    '2tex'  8
	cmp	[dr_flag],3				;    'btex'  9
	jg	.next_m5
     @@:
	call	init_envmap2	; update env map if shading model = environment or bump
    .next_m5:
	cmp	 ah,11
	je	 @f
	cmp	 ah,12
	je	 @f
	cmp	 ah,13
	jne	 .next_m4
      @@:
	call	 mirror
     .next_m4:
	cmp	 ah,14
	jne	 @f
	call	 exchange
     @@:
	cmp	 ah,15
	jne	 @f
	cmp	 [emboss_flag],1
	call	 init_envmap2
     @@:
;        cmp      ah,17
;        jne      .next_m
;        cmp      [move_flag],2
;        jne      @f
;        call     draw_window             ; redraw other labels to navigation buttons
;      @@:
;        cmp      [move_flag],0
;        jne      .next_m
;        call     draw_window             ; redraw other labels to navigation buttons
     .next_m:
	cmp	 ah,18
	jne	 .next_m2
     if USE_LFN
	mov	 [re_alloc_flag],1	 ; reallocate memory
	mov	 [triangles_count_var],1000
	mov	 [points_count_var],1000
	call	 alloc_mem_for_tp
	mov	 [re_alloc_flag],0
     end if
	mov	 bl,[generator_flag]
	or	 bl,bl
	jz	 .next_m2
	cmp	 bl,1
	jne	 @f
	call	 generate_object
	jmp	 .calc_norm
      @@:
	cmp	 bl,4
	jg	 @f
	movzx	 ax,bl		      ; ax < - object number
	call	 generate_object2
	jmp	.calc_norm
      @@:
	call	generate_object3
      .calc_norm:
	call	optimize_object1
	call	init_triangles_normals2
	call	init_point_normals
	call	calc_bumpmap_coords   ; bump and texture mapping

     .next_m2:
	cmp	 ah,19
	je	 @f
	cmp	 ah,20
	jne	 .next_m3
     @@:
	mov	 edi,bumpmap
	call	 calc_bumpmap
     .next_m3:
	cmp	ah,21		 ; re map bumps, texture coordinates
	jne	@f
	call	calc_bumpmap_coords
      @@:
	jmp	noclose


			       ; there are 6 navigation buttons each
   add_vec_buttons:	       ; can move: object, camera,.. list is open
			       ;
	cmp	ah,30
	jne	.next
	cmp	[move_flag],1
	je	@f
;        cmp     [move_flag],2
;        je      .set_light1
	sub	[vect_y],10
	jmp	.next
      @@:
	sub	[yobs],10   ;  observator = camera position
	jmp	.next

;--------------------------------------------------
;      .set_light1:          ;  r -
;        movzx   ebx,[light_no_flag]  ; * 22
;        mov     ecx,ebx
;        shl     ebx,4
;        shl     ecx,1
;        add     ebx,ecx
;        shl     ecx,1
;        add     ebx,ecx
;        add     ebx,lights+6    ; 6 -> light vector size
;
;        movzx   ecx,[light_comp_flag]
;        lea     ecx,[ecx*3}
;        add     ebx,ecx         ; ebx ->  color to set

;---------------------------------------------------
      .next:
	cmp	ah,31
	jne	.next1
	cmp	[move_flag],1
	je	@f

	add	[vect_z],10
	jmp	.next1
      @@:
	add	[zobs],10	  ;  observator = camera position
     .next1:
	cmp	ah,33
	jne	.next2
	cmp	[move_flag],1
	je	@f

	sub	[vect_x],10
	jmp	.next2
      @@:
	sub	[xobs],10	  ;  observator = camera position
      .next2:
	cmp	ah,32
	jne	.next3
	cmp	[move_flag],1
	je	@f

	add	[vect_x],10
	jmp	.next3
      @@:
	add	[xobs],10	  ;  observator = camera position
      .next3:
	cmp	ah,34
	jne	.next4
	cmp	[move_flag],1
	je	@f

	sub	[vect_z],10
	jmp	.next4
      @@:
	sub	[zobs],10	  ;  observator = camera position
      .next4:
	cmp	ah,35
	jne	.next5
	cmp	[move_flag],1
	je	@f

      ;  call    add_vector
	add	[vect_y],10
	jmp	.next5
      @@:
	add	[yobs],10	  ;  observator = camera position
      .next5:



    noclose:

	cmp	[r_flag],2
	jne	.no_x
	inc	[angle_x]
	and	[angle_x],0xff
	mov	[angle_z],0
	jmp	.end_rot

      .no_x:
	cmp	[r_flag],0
	jne	.no_y
	inc	[angle_y]
	and	[angle_y],0xff
	mov	[angle_z],0
	jmp	.end_rot

      .no_y:
	cmp	[r_flag],1
	jne	.end_rot
	mov	cx,[angle_x]
	inc	cx
	and	cx,0xff
	mov	[angle_z],0
	mov	[angle_y],cx
	mov	[angle_x],cx
     .end_rot:

	mov	esi,angle_x
	mov	edi,matrix
	call	make_rotation_matrix
;    RDTSC
;    push eax
	mov	esi,[points_normals_ptr]
	mov	edi,[points_normals_rot_ptr]
	mov	ebx,matrix
	movzx	ecx,[points_count_var]
	call	rotary

	mov	esi,matrix
	call	add_scale_to_matrix

	mov	esi,[points_ptr]
	mov	edi,[points_rotated_ptr]
	mov	ebx,matrix
	movzx	ecx,[points_count_var]
	call	rotary

;    RDTSC
;    pop    ebx
;    sub    eax,ebx
;    sub    eax,41
;    push   eax

	mov	esi,[points_rotated_ptr]
	mov	edi,[points_translated_ptr]
	movzx	ecx,[points_count_var]
	call	translate_points

;        cmp     [dr_flag],5
;        jne     @f
;        call    calc_attenuation_light
;     @@:
	cmp	[fire_flag],0
	jne	@f
	call	clrscr		; clear the screen
     @@:
	cmp	[catmull_flag],1  ;non sort if Catmull = on
	je	.no_sort
	call	sort_triangles
      .no_sort:
	cmp	[dr_flag],7	  ; fill if 2tex and texgrd
	jge	@f
	cmp	[catmull_flag],0  ;non fill if Catmull = off
	je	.non_f
	cmp	[dr_flag],6	  ; non fill if dots
	je	.non_f
      @@:
	call	fill_Z_buffer	  ; make background
     .non_f:
    RDTSC
    push eax
	cmp	[dr_flag],6
	jne	@f
	call	 draw_dots
	jmp	 .blurrr
      @@:
	call	draw_triangles	; draw all triangles from the list
      .blurrr:
	cmp	[fire_flag],0
	jne	@f
	cmp	[blur_flag],0
	je	.no_blur  ; no blur, no fire
	movzx	ecx,[blur_flag]
	call	blur_screen    ; blur and fire
	jmp	.no_blur
    @@:
	cmp	[emboss_flag],0
	jne	.emb	       ; if emboss=true -> no fire
	movzx	ecx,[fire_flag]
	call	blur_screen    ; blur and fire
    .no_blur:		       ; no blur, no fire
	cmp	[emboss_flag],0
	je	@f
     .emb:
	call	do_emboss

      @@:


    RDTSC
    sub eax,[esp]
    sub eax,41
;    pop     eax

    mov     ecx,10
  .dc:
    xor     edx,edx
    mov     edi,10
    div     edi
    add     dl,30h
    mov     [STRdata+ecx-1],dl
    loop    .dc
    pop eax

    mov     eax,7	    ; put image
    mov     ebx,screen
    mov     ecx,SIZE_X shl 16 + SIZE_Y
    mov     edx,5 shl 16 + 20
    int     0x40


    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,5*65536+23 	   ; [x start] *65536 + [y start]
    mov  ecx,-1
    mov  edx,STRdata		   ; pointer to text beginning
    mov  esi,10 		   ; text length
    int  40h



   jmp	   still

;--------------------------------------------------------------------------------
;-------------------------PROCEDURES---------------------------------------------
;--------------------------------------------------------------------------------
include "TEX3.INC"
include "FLAT_CAT.INC"
include "TEX_CAT.INC"
include "BUMP_CAT.INC"
include "3DMATH.INC"
include "GRD3.INC"
include "FLAT3.INC"
include "BUMP3.INC"
include "B_PROCS.INC"
include "A_PROCS.INC"
include "GRD_CAT.INC"
include "BUMP_TEX.INC"
include "GRD_TEX.INC"
include "TWO_TEX.INC"

alloc_buffer_mem:
    movzx    ecx,[size_x]
    movzx    eax,[size_y]
    mul      ecx
    lea      ecx,[eax*3]
    push     ecx
    shl      eax,2
    add      ecx,eax
    add      ecx,MEM_END
    mov      ebx,1
    mov      eax,64	; allocate mem  - resize app mem
    int      0x40
    mov      [screen_ptr],MEM_END
    mov      [Zbuffer_ptr],MEM_END
    pop      ecx
    add      [Zbuffer_ptr],ecx
ret

update_flags:
; updates flags and writing flag description
; in    ah - button number
	push	ax
	mov	edi,menu
      .ch_another:
	cmp	ah,byte[edi]	 ; ah = button id
	jne	@f
	mov	bl,byte[edi+11]  ; max_flag + 1
	cmp	bl,255
	je	.no_write
	inc	byte[edi+12]	 ; flag
	cmp	byte[edi+12],bl
	jne	.write
	mov	byte[edi+12],0
	jmp	.write
      @@:
	add	edi,17
	cmp	byte[edi],-1
	jne	.ch_another
     .write:
;     clreol   {pascal never dies}
;          * eax = 13 - function number
;  * ebx = [coordinate on axis x]*65536 + [size on axis x]
;  * ecx = [coordinate on axis y]*65536 + [size on axis y]
;  * edx = color 0xRRGGBB or 0x80RRGGBB for gradient fill

	mov	eax,13				 ; function 13 write rectangle
	movzx	ecx,byte[edi]
	sub	cl,2
	lea	ecx,[ecx*3]
	lea	ecx,[ecx*5]
	add	ecx,28
	shl	ecx,16
	add	ecx,14				;  ecx = [coord y]*65536 + [size y]
	mov	ebx,(SIZE_X+12+70)*65536+25	; [x start] *65536 + [size x]
	mov	edx,0x00000000			;  color  0x00RRGGBB
	int	0x40

	mov	eax,4				; function 4 : write text to window
	movzx	ebx,byte[edi]
	sub	bl,2
	lea	ebx,[ebx*3]
	lea	ebx,[ebx*5]
	add	ebx,(SIZE_X+12+70)*65536+28	; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff			; font 1 & color ( 0xF0RRGGBB )
	movzx	edx,byte[edi+12]		; current flag
	shl	edx,2				; * 4 = text length
	add	edx,dword[edi+13]		; pointer to text beginning
	mov	esi,4				; text length -
						; flag description 4 characters
	int	0x40

     .no_write:
	pop	ax
ret
normalize_all_light_vectors:
	mov	edi,lights
     @@:
	call	normalize_vector	   ;       3dmath.inc
	add	edi,LIGHT_SIZE
	cmp	edi,lightsend	;ecx
	jl	@b
ret

calc_bumpmap_coords:	  ; map texture, bump
;macro .comment222
;                                ; planar mapping
;        mov     esi,points
;        mov     edi,tex_points
;      @@:
;         add     esi,2
;         movsd
;         cmp     dword[esi],dword -1
;         jne     @b

;      .Pi2  equ dword[ebp-4]

;      mov   ebp,esp
;      sub   esp,4

      fninit
      fldpi
      fadd	st,st
      mov	esi,[points_ptr]
      mov	edi,tex_points
      movzx	ecx,[points_count_var]
      inc	ecx
;      cmp       [map_tex_flag],1
;      jne       .cylindric
      ; spherical mapping around y axle

   @@:
      fld	dword[esi]     ; x coord
      fld	dword[esi+8]   ; z coord
      fpatan		       ; arctg(st1/st)
;      fdiv      .Pi2
      fdiv	st0,st1
      fimul	[tex_x_div2]
      fiadd	[tex_x_div2]
      fistp	word[edi]      ; x

      fld	dword[esi+4]   ; y coord
      fld	dword[esi]     ; x
      fmul	st,st0
      fld	dword[esi+4]   ; y
      fmul	st,st0
      fld	dword[esi+8]   ; z
      fmul	st,st0
      faddp
      faddp
      fsqrt
      fpatan
      fldpi
      fdivp
      fimul    [tex_y_div2]
      fiadd    [tex_y_div2]
      fistp    word[edi+2]     ; y

      add      esi,12
      add      edi,4
      loop     @b
      ffree    st0
;      jmp      .end_map
;  .cylindric:
;       fld     dword[esi]     ; around y axle
;       fld     dword[esi+8]
;       fpatan
;       fdiv    st0,st1
;       fimul   [tex_x_div2]
;       fiadd   [tex_x_div2]
;       fistp   word[edi]

;       fld     dword[esi+4]
;       fimul   [tex_y_div2]
;       fiadd   [tex_y_div2]
;       fistp   word[edi+2]

;       add     esi,12
;       add     edi,4
;       loop    .cylindric
;       ffree    st0
;;      mov      esp,ebp
;   .end_map:
ret


init_envmap2:	      ; do env_map using many light sources
;env_map 512 x 512 x 3 bytes
.temp  equ word   [ebp-2]
.nEy   equ word  [ebp-4]
.nEx   equ word  [ebp-6]
.col_r equ    [ebp-8]
.col_g equ    [ebp-9]
.col_b equ    [ebp-10]

	 push	  ebp
	 mov	  ebp,esp
	 sub	  esp,20
	 mov	  edi,envmap
	 fninit

	 mov	  dx,- TEX_Y / 2 ;256   ; dx - vertical coordinate = y
    .ie_ver:
	 mov	  cx,- TEX_X / 2 ;256   ; cx - horizontal coord = x
    .ie_hor:
	 xor	  ebx,ebx
	 mov	  dword .col_b, 0
     .light:
	 lea	  esi,[lights+ebx]
	 fld	  dword[esi]	 ; light vector x cooficient
	 fimul	  [tex_x_div2] ;[i256]
	 mov	  .temp,cx
	 fisubr   .temp
	 fistp	  .nEx
	 fld	  dword[esi+4]	 ; light vector y cooficient
	 fimul	  [tex_y_div2] ;[i256]
	 mov	  .temp,dx
	 fisubr   .temp
	 fistp	  .nEy

	 cmp	  .nEx,- TEX_X / 2 ;256
	 jl	  .update_counters
	 cmp	  .nEy,- TEX_Y / 2 ;256
	 jl	  .update_counters
	 cmp	  .nEx,TEX_X / 2 ;256
	 jg	  .update_counters
	 cmp	  .nEy,TEX_Y / 2 ;256
	 jg	  .update_counters

	 fild	  .nEx
	 fmul	  st,st0
	 fild	  .nEy
	 fmul	  st,st0
	 faddp
	 fsqrt
	 fisubr   [i256]
	 fmul	  [env_const]
	 fidiv	  [i256]   ; st - 'virtual' dot product

	 fcom	  [dot_max]
	 fstsw	  ax
	 sahf
	 jb	  @f
	 ffree	  st
	 fld1	  ;[dot_max]
      @@:
	 fcom	  [dot_min]
	 fstsw	  ax
	 sahf
	 ja	  @f
	 ffree	  st
	 fldz	  ;[dot_min]
      @@:
	 push	  ebp
	 movzx	  ax,byte[esi+21]
	 push	  ax  ;- shines
	 mov	  al,byte[esi+14]   ; b    orginal color
	 push	  ax
	 mov	  al,byte[esi+13]   ; g
	 push	  ax
	 mov	  al,byte[esi+12]   ; r
	 push	  ax
	 mov	  al,byte[esi+20]   ; b     max color
	 push	  ax
	 mov	  al,byte[esi+19]   ; g
	 push	  ax
	 mov	  al,byte[esi+18]   ; r
	 push	  ax
	 mov	  al,byte[esi+17]   ; b    min col
	 push	  ax
	 mov	  al,byte[esi+16]   ; g
	 push	  ax
	 mov	  al,byte[esi+15]   ; r
	 push	  ax
	 push	  eax	      ; earlier - dot pr
      ;  fstp     .dot_product
      ;  push     .dot_product
	 call	  calc_one_col
	 pop	  ebp
	 ; eax-0x00rrggbb
	 cmp	  al,.col_b
	 jbe	  @f
	 mov	  .col_b,al
   @@:			      ;  eax - ggbb00rr
	 shr	  ax,8
	 cmp	  al,.col_g
	 jbe	  @f
	 mov	  .col_g,al
   @@:			      ;  eax - bb0000gg
	 shr	  eax,16
	 cmp	  al,.col_r
	 jbe	  @f
	 mov	  .col_r,al
   @@:
   .update_counters:			 ; update and jump when neccesery
	 add	  ebx,LIGHT_SIZE
	 cmp	  bx,[all_lights_size]
	 jl	  .light    ; next_light
	 mov	  eax,dword .col_b
	 stosd
	 dec	  edi

	 inc	  cx
	 cmp	  cx,TEX_X / 2 ;256
	 jne	  .ie_hor

	 inc	  dx
	 cmp	  dx,TEX_Y / 2 ;256
	 jne	 .ie_ver

	 mov	 esp,ebp
	 pop	 ebp
ret

do_color_buffer:	 ; do color buffer for Gouraud, flat shading
;env_map 512 x 512 x 3 bytes    ; many lights using
.temp  equ word   [ebp-2]
.nz    equ dword  [ebp-6]  ; dword
.ny    equ dword  [ebp-10]
.nx    equ dword  [ebp-14]
.col_r equ    [ebp-16]
.col_g equ    [ebp-17]
.col_b equ    [ebp-18]

	 push	  ebp
	 mov	  ebp,esp
	 sub	  esp,20
	 mov	  edi,color_map
	 fninit

	 mov	  dx,- TEX_Y / 2 ;-256   ; dx - vertical coordinate = y
    .ie_ver:
	 mov	  cx,- TEX_X / 2 ;256   ; cx - horizontal coord = x
    .ie_hor:
	 mov	  .temp,cx
	 fild	  .temp
	 fidiv	  [i256]   ;st = Nx - vector normal x cooficient
	 fst	  .nx
	 fmul	  st,st0
	 mov	  .temp,dx
	 fild	  .temp
	 fidiv	  [i256]   ; st = Ny - vector normal y coeficient
	 fst	  .ny
	 fmul	  st,st0
	 faddp
	 fld1
	 fchs
	 faddp
	 fabs
	 fsqrt
	 fchs
	 fstp	  .nz		   ; st - Nz - vect normal z coeficient
	 xor	  ebx,ebx
	 mov	  dword .col_b, 0
     .light:
	 push	  edi	;env_map
	 lea	  esi,[lights+ebx]
	 lea	  edi,.nx
	 call	  dot_product
	 pop	  edi
	 fcom	  [dot_min]
	 fstsw	  ax
	 sahf
	 ja	  .env_ok1  ;compare with dot_max
	 ffree	  st

	jmp	  .update_counters
      .env_ok1:
	 fcom	 [dot_max]
	 fstsw	 ax
	 sahf
	 jb	 .env_ok2     ; calc col
	 ffree	 st
	 jmp	 .update_counters
      .env_ok2: 	   ;calc col
	 push	  ebp
	 movzx	  ax,byte[esi+21]
	 push	  ax  ;- shines
	 mov	  al,byte[esi+14]   ; b    orginal color
	 push	  ax
	 mov	  al,byte[esi+13]   ; g
	 push	  ax
	 mov	  al,byte[esi+12]   ; r
	 push	  ax
	 mov	  al,byte[esi+20]   ; b     max color
	 push	  ax
	 mov	  al,byte[esi+19]   ; g
	 push	  ax
	 mov	  al,byte[esi+18]   ; r
	 push	  ax
	 mov	  al,byte[esi+17]   ; b    min col
	 push	  ax
	 mov	  al,byte[esi+16]   ; g
	 push	  ax
	 mov	  al,byte[esi+15]   ; r
	 push	  ax
	 push	  eax	      ; earlier - dot pr
      ;  fstp     .dot_product
      ;  push     .dot_product
	 call	  calc_one_col
	 pop	  ebp
	 ; eax-0x00rrggbb
	 cmp	  al,.col_b
	 jbe	  @f
	 mov	  .col_b,al
   @@:
	 shr	  ax,8
	 cmp	  al,.col_g
	 jbe	  @f
	 mov	  .col_g,al
   @@:
	 shr	  eax,16
	 cmp	  al,.col_r
	 jbe	  @f
	 mov	  .col_r,al
  @@:
 .update_counters:				    ; update and jump when neccesery
	add	ebx,LIGHT_SIZE
	cmp	bx,[all_lights_size]
	jl	.light	  ; next_light
	mov	eax,dword .col_b
	stosd
	dec	edi

	inc	cx
	cmp	cx,TEX_X / 2 ;256
	jne	.ie_hor

	inc	dx
	cmp	dx,TEX_X / 2 ;256
	jne	.ie_ver

    .env_done:
	 mov	 esp,ebp
	 pop	 ebp
ret
if 0
init_triangles_normals:
	mov	ebx,triangles_normals
	mov	ebp,triangles
     @@:
	push	ebx
	mov	ebx,vectors
	movzx	esi,word[ebp]	       ; first point index
	lea	esi,[esi*3]
	lea	esi,[points+esi*2]     ; esi - pointer to 1st 3d point
	movzx	edi,word[ebp+2]        ; second point index
	lea	edi,[edi*3]
	lea	edi,[points+edi*2]     ; edi - pointer to 2nd 3d point
	call	make_vector
	add	ebx,12
	mov	esi,edi
	movzx	edi,word[ebp+4]        ; third point index
	lea	edi,[edi*3]
	lea	edi,[points+edi*2]
	call	make_vector
	mov	edi,ebx 		; edi - pointer to 2nd vector
	mov	esi,ebx
	sub	esi,12			; esi - pointer to 1st vector
	pop	ebx
	call	cross_product
	mov	edi,ebx
	call	normalize_vector
	add	ebp,6
	add	ebx,12
	cmp	dword[ebp],-1
	jne	@b
ret
end if
init_point_normals:
.x equ dword [ebp-4]
.y equ dword [ebp-8]
.z equ dword [ebp-12]
.point_number equ word [ebp-26]
.hit_faces    equ word [ebp-28]

	fninit
	mov	  ebp,esp
	sub	  esp,28
	mov	  edi,[points_normals_ptr]
	mov	  .point_number,0
    .ipn_loop:
	mov	  .hit_faces,0
	mov	  .x,0
	mov	  .y,0
	mov	  .z,0
	mov	  esi,[triangles_ptr]
	xor	  ecx,ecx	       ; ecx - triangle number
    .ipn_check_face:
	xor	  ebx,ebx	       ; ebx - 'position' in one triangle
    .ipn_check_vertex:
	movzx	  eax,word[esi+ebx]    ;  eax - point_number
	cmp	  ax,.point_number
	jne	  .ipn_next_vertex
	push	  esi
	mov	  esi,ecx
	lea	  esi,[esi*3]
       ; lea       esi,[triangles_normals+esi*4]
	shl	  esi,2
	add	  esi,[triangles_normals_ptr]

	fld	  .x
	fadd	  dword[esi+vec_x]	 ; vec_x this defined in 3dmath.asm - x cooficient
	fstp	  .x			 ; of normal vactor
	fld	  .y
	fadd	  dword[esi+vec_y]
	fstp	  .y
	fld	  .z
	fadd	  dword[esi+vec_z]
	fstp	  .z
	pop	  esi
	inc	  .hit_faces
	jmp	  .ipn_next_face
    .ipn_next_vertex:
	add	  ebx,2
	cmp	  ebx,6
	jne	  .ipn_check_vertex
    .ipn_next_face:
	add	  esi,6
	inc	  ecx
	cmp	  cx,[triangles_count_var]
	jne	  .ipn_check_face

	fld	  .x
	fidiv	  .hit_faces
	fstp	  dword[edi+vec_x]
	fld	  .y
	fidiv	  .hit_faces
	fstp	  dword[edi+vec_y]
	fld	  .z
	fidiv	  .hit_faces
	fstp	  dword[edi+vec_z]
	call	  normalize_vector
	add	  edi,12  ;type vector 3d
	inc	  .point_number
	mov	  dx,.point_number
	cmp	  dx,[points_count_var]
	jne	  .ipn_loop

	mov	  esp,ebp
ret
;===============================================================

init_triangles_normals2:
	mov	ebx,[triangles_normals_ptr]
	mov	ebp,[triangles_ptr]
     @@:
	push	ebx
	mov	ebx,vectors
	movzx	esi,word[ebp]	       ; first point index
	lea	esi,[esi*3]
;        lea     esi,[points+esi*2]     ; esi - pointer to 1st 3d point
	shl	esi,2
	add	esi,[points_ptr]
	movzx	edi,word[ebp+2] 	 ; first point index
	lea	edi,[edi*3]
	shl	edi,2
	add	edi,[points_ptr]
;        movzx   edi,word[ebp+2]        ; second point index
;        lea     edi,[edi*3]
;        lea     edi,[points+edi*2]     ; edi - pointer to 2nd 3d point
	call	make_vector_r
	add	ebx,12
	mov	esi,edi
	movzx	edi,word[ebp+4]        ; third point index
	lea	edi,[edi*3]
	shl	edi,2
	add	edi,[points_ptr]
;        lea     edi,[points+edi*2]
	call	make_vector_r
	mov	edi,ebx 		; edi - pointer to 2nd vector
	mov	esi,ebx
	sub	esi,12			; esi - pointer to 1st vector
	pop	ebx
	call	cross_product
	mov	edi,ebx
	call	normalize_vector
	add	ebp,6
	add	ebx,12
	cmp	dword[ebp],-1
	jne	@b
ret


;=================================================================
sort_triangles:
	mov	esi,[triangles_ptr]
	mov	edi,triangles_with_z
	mov	ebp,[points_translated_ptr]

    make_triangle_with_z:	;makes list with triangles and z position
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	movzx	ecx,word[ebp+eax*2+4]

	movzx	eax,word[esi+2]
	lea	eax,[eax*3]
	add	cx,word[ebp+eax*2+4]

	movzx	eax,word[esi+4]
	lea	eax,[eax*3]
	add	cx,word[ebp+eax*2+4]

	mov	ax,cx
       ; cwd
       ; idiv    word[i3]
	movsd			; store vertex coordinates
	movsw
	stosw			; middle vertex coordinate  'z' in triangles_with_z list
	cmp	dword[esi],-1
	jne	make_triangle_with_z
	movsd			; copy end mark
	mov	eax,4
	lea	edx,[edi-8-trizdd]
     ;   lea     edx, [edi-8]
     ;   sub     edx,[triangles_w_z_ptr]
	mov	[high],edx
	call	quicksort
	mov	eax,4
	mov	edx,[high]
	call	insertsort
	jmp	end_sort

    quicksort:
	mov	ecx,edx
	sub	ecx,eax
	cmp	ecx,32
	jc	.exit
	lea	ecx,[eax+edx]
	shr	ecx,4
	lea	ecx,[ecx*8-4];
;        mov     edi,[triangles_w_z_ptr]
;        mov     ebx,[edi+eax]
;        mov     esi,[edi+ecx]
;        mov     edi,[edi+edx]
	mov	ebx,[trizdd+eax]; trizdd[l]
	mov	esi,[trizdd+ecx]; trizdd[i]
	mov	edi,[trizdd+edx]; trizdd[h]
	cmp	ebx,esi
	jg	@f		; direction NB! you need to negate these to invert the order
      if Ext=NON
	mov	[trizdd+eax],esi
	mov	[trizdd+ecx],ebx
	mov	ebx,[trizdd+eax-4]
	mov	esi,[trizdd+ecx-4]
	mov	[trizdd+eax-4],esi
	mov	[trizdd+ecx-4],ebx
	mov	ebx,[trizdd+eax]
	mov	esi,[trizdd+ecx]
      else
;        push    ebx
;        mov     ebx,[triangles_w_z_ptr]
;        movq    mm0,[ebx+eax-4]
;        movq    mm1,[ebx+ecx-4]
;        movq    [ebx+ecx-4],mm0
;        movq    [ebx+eax-4],mm1
;        pop     ebx
	movq	mm0,[trizdq+eax-4]
	movq	mm1,[trizdq+ecx-4]
	movq	[trizdq+ecx-4],mm0
	movq	[trizdq+eax-4],mm1
	xchg	ebx,esi
      end if
      @@:
	cmp	ebx,edi
	jg	@f		; direction
      if Ext=NON
	mov	[trizdd+eax],edi
	mov	[trizdd+edx],ebx
	mov	ebx,[trizdd+eax-4]
	mov	edi,[trizdd+edx-4]
	mov	[trizdd+eax-4],edi
	mov	[trizdd+edx-4],ebx
	mov	ebx,[trizdd+eax]
	mov	edi,[trizdd+edx]
      else
;        push    ebx
;        mov     ebx,[triangles_w_z_ptr]
;        movq    mm0,[ebx+eax-4]
;        movq    mm1,[ebx+edx-4]
;        movq    [ebx+edx-4],mm0
;        movq    [ebx+eax-4],mm1
	movq	mm0,[trizdq+eax-4]
	movq	mm1,[trizdq+edx-4]
	movq	[trizdq+edx-4],mm0
	movq	[trizdq+eax-4],mm1
;        pop     ebx
	xchg	ebx,edi
      end if
      @@:
	cmp	esi,edi
	jg	@f		; direction
      if Ext=NON
	mov	[trizdd+ecx],edi
	mov	[trizdd+edx],esi
	mov	esi,[trizdd+ecx-4]
	mov	edi,[trizdd+edx-4]
	mov	[trizdd+ecx-4],edi
	mov	[trizdd+edx-4],esi
      else
;        push    ebx
;        mov     ebx,[triangles_w_z_ptr]
;        movq    mm0,[ebx+ecx-4]
;        movq    mm1,[ebx+edx-4]
;        movq    [ebx+edx-4],mm0
;        movq    [ebx+ecx-4],mm1
;        pop     ebx

	movq	mm0,[trizdq+ecx-4]
	movq	mm1,[trizdq+edx-4]
	movq	[trizdq+edx-4],mm0
	movq	[trizdq+ecx-4],mm1
	xchg	ebx,esi
      end if
      @@:
	mov	ebp,eax 	; direction
	add	ebp,8	   ;   j
      if Ext=NON
	mov	esi,[trizdd+ebp]
	mov	edi,[trizdd+ecx]
	mov	[trizdd+ebp],edi
	mov	[trizdd+ecx],esi
	mov	esi,[trizdd+ebp-4]
	mov	edi,[trizdd+ecx-4]
	mov	[trizdd+ecx-4],esi
	mov	[trizdd+ebp-4],edi
      else
;        push    ebx
;        mov     ebx,[triangles_w_z_ptr]
;        movq    mm0,[ebx+ebp-4]
;        movq    mm1,[ebx+ecx-4]
;        movq    [ebx+ecx-4],mm0
;        movq    [ebx+ebp-4],mm1
;        pop     ebx

	movq	mm0,[trizdq+ebp-4]
	movq	mm1,[trizdq+ecx-4]
	movq	[trizdq+ecx-4],mm0
	movq	[trizdq+ebp-4],mm1
      end if
	mov	ecx,edx    ;   i; direction
	mov	ebx,[trizdd+ebp]; trizdd[j]
;        mov     ebx, [triangles_w_z_ptr]
;        add     ebx, ebp

 ;       push    eax
 ;       mov     eax, [triangles_w_z_ptr]
      .loop:
	sub	ecx,8		; direction
	cmp	[trizdd+ecx],ebx
;        cmp     [eax+ecx],ebx
	jl	.loop		; direction
      @@:
	add	ebp,8		; direction
	cmp	[trizdd+ebp],ebx
;        cmp     [eax+ebp],ebx
	jg	@b		; direction
	cmp	ebp,ecx
	jge	@f		; direction
      if Ext=NON
	mov	esi,[trizdd+ecx]
	mov	edi,[trizdd+ebp]
	mov	[trizdd+ebp],esi
	mov	[trizdd+ecx],edi
	mov	edi,[trizdd+ecx-4]
	mov	esi,[trizdd+ebp-4]
	mov	[trizdd+ebp-4],edi
	mov	[trizdd+ecx-4],esi
      else
;        movq    mm0,[eax+ecx-4]
;        movq    mm1,[eax+ebp-4]
;        movq    [eax+ebp-4],mm0
;        movq    [eax+ecx-4],mm1
	movq	mm0,[trizdq+ecx-4]
	movq	mm1,[trizdq+ebp-4]
	movq	[trizdq+ebp-4],mm0
	movq	[trizdq+ecx-4],mm1
      end if
	jmp	.loop
;        pop     eax
      @@:
      if Ext=NON
	mov	esi,[trizdd+ecx]
	mov	edi,[trizdd+eax+8]
	mov	[trizdd+eax+8],esi
	mov	[trizdd+ecx],edi
	mov	edi,[trizdd+ecx-4]
	mov	esi,[trizdd+eax+4]
	mov	[trizdd+eax+4],edi
	mov	[trizdd+ecx-4],esi
      else
;        push    edx
;        mov     edx,[triangles_w_z_ptr]
;        movq    mm0,[edx+ecx-4]
;        movq    mm1,[edx+eax+4]; dir
;        movq    [edx+eax+4],mm0; dir
;        movq    [edx+ecx-4],mm1
;        pop     edx

	movq	mm0,[trizdq+ecx-4]
	movq	mm1,[trizdq+eax+4]; dir
	movq	[trizdq+eax+4],mm0; dir
	movq	[trizdq+ecx-4],mm1
      end if
	add	ecx,8
	push	ecx edx
	mov	edx,ebp
	call	quicksort
	pop	edx eax
	call	quicksort
      .exit:
    ret
    insertsort:
	mov	esi,eax
      .start:
	add	esi,8
	cmp	esi,edx
	ja	.exit
	mov	ebx,[trizdd+esi]
;        mov     ebx,[triangles_w_z_ptr]
;        add     ebx,esi
      if Ext=NON
	mov	ecx,[trizdd+esi-4]
      else
;        push    ebx
;        mov     ebx,[triangles_w_z_ptr]
;        movq    mm1,[ebx+esi-4]
	movq	mm1,[trizdq+esi-4]
;        pop     ebx
      end if
	mov	edi,esi
      @@:
	cmp	edi,eax
	jna	@f
;        push    eax
;        mov     eax,[triangles_w_z_ptr]
;        cmp     [eax+edi-8],ebx
;        pop     eax
       cmp     [trizdd+edi-8],ebx
	jg	@f		   ; direction
      if Ext=NON
	mov	ebp,[trizdd+edi-8]
	mov	[trizdd+edi],ebp
	mov	ebp,[trizdd+edi-12]
	mov	[trizdd+edi-4],ebp
      else
;        push    eax
;        mov     eax,[triangles_w_z_ptr]
;        movq    mm0,[eax+edi-12]
;        movq    [eax+edi-4],mm0
	movq	mm0,[trizdq+edi-12]
	movq	[trizdq+edi-4],mm0
;        pop     eax
      end if
	sub	edi,8
	jmp	@b
      @@:
      if Ext=NON
	mov	[trizdd+edi],ebx
	mov	[trizdd+edi-4],ecx
      else
;        push    eax
;        mov     eax,[triangles_w_z_ptr]
;        movq    [eax+edi-4],mm1
	movq	[trizdq+edi-4],mm1
;        pop     eax
      end if
	jmp	.start
      .exit:
    ret
   end_sort:
    ; translate triangles_with_z to sorted_triangles
	mov	esi,triangles_with_z
;        mov      esi,[triangles_w_z_ptr]
      ;  mov     edi,sorted_triangles
	mov	 edi,[triangles_ptr]
    again_copy:
      if Ext=NON
	movsd
	movsw
	add	esi,2
      else
	movq	mm0,[esi]
	movq	[edi],mm0
	add	esi,8
	add	edi,6
      end if
	cmp	dword[esi],-1
	jne	again_copy
;      if Ext=MMX
;        emms
;      end if
	movsd  ; copy end mark too
ret

clrscr:
	mov	edi,screen
	mov	ecx,SIZE_X*SIZE_Y*3/4
	xor	eax,eax
      if Ext=NON
	rep	stosd
      else
	pxor	mm0,mm0
      @@:
	movq	[edi+00],mm0
	movq	[edi+08],mm0
	movq	[edi+16],mm0
	movq	[edi+24],mm0
	add	edi,32
	sub	ecx,8
	jnc	@b
      end if
ret


draw_triangles:
	mov esi,[triangles_ptr]
    .again_dts:
	mov ebp,[points_translated_ptr]
      if Ext=NON
	movzx	eax,word[esi]
	mov	[point_index1],ax
	lea	eax,[eax*3]
	add	eax,eax
	push	ebp
	add	ebp,eax
	mov	eax,[ebp]
	mov	dword[xx1],eax
	mov	eax,[ebp+4]
	mov	[zz1],ax
	pop	ebp


	movzx	eax,word[esi+2]
	mov	[point_index2],ax
	lea	eax,[eax*3]
	add	eax,eax
	push	ebp
	add	ebp,eax
	mov	eax,[ebp]
	mov	dword[xx2],eax
	mov	eax,[ebp+4]
	mov	[zz2],ax
	pop	ebp


	movzx	eax,word[esi+4]        ; xyz3 = [ebp+[esi+4]*6]
	mov	[point_index3],ax
	lea	eax,[eax*3]
	add	eax,eax
    ;    push    ebp
	add	ebp,eax
	mov	eax,[ebp]
	mov	dword[xx3],eax
	mov	eax,[ebp+4]
	mov	[zz3],ax
      else
	mov	eax,dword[esi]		 ; don't know MMX
	mov	dword[point_index1],eax
       ; shr     eax,16
       ; mov     [point_index2],ax
	mov	ax,word[esi+4]
	mov	[point_index3],ax
	movq	mm0,[esi]
	pmullw	mm0,qword[const6]
	movd	eax,mm0
	psrlq	mm0,16
	movd	ebx,mm0
	psrlq	mm0,16
	movd	ecx,mm0
	and	eax,0FFFFh
	and	ebx,0FFFFh
	and	ecx,0FFFFh
	movq	mm0,[ebp+eax]
	movq	mm1,[ebp+ebx]
	movq	mm2,[ebp+ecx]
	movq	qword[xx1],mm0
	movq	qword[xx2],mm1
	movq	qword[xx3],mm2
;        emms
      end if				  ; *********************************
	push esi			  ;
	fninit				  ; DO culling AT FIRST
	cmp	[culling_flag],1	  ; (if culling_flag = 1)
	jne	.no_culling
	mov	esi,point_index1	  ; *********************************
	mov	ecx,3			  ;
      @@:
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	shl	eax,2
	add	eax,[points_normals_rot_ptr]
;        lea     eax,[eax+point_normals_rotated]
	fld	dword[eax+8]		 ; *****************************
	ftst				 ; CHECKING OF Z COOFICIENT OF
	fstsw	ax			 ; NORMAL VECTOR
	sahf
	jb	@f
	ffree	st
	loop	@b
	jmp	.end_draw   ; non visable
      @@:
	ffree	st  ;is visable
      .no_culling:
	cmp	[dr_flag],0		  ; draw type flag
	je	.flat_draw
	cmp	[dr_flag],2
	je	.env_mapping
	cmp	[dr_flag],3
	je	.bump_mapping
	cmp	[dr_flag],4
	je	.tex_mapping
	cmp	[dr_flag],5
	je	.rainbow
	cmp	[dr_flag],7
	je	.grd_tex
	cmp	[dr_flag],8
	je	.two_tex
	cmp	[dr_flag],9
	je	.bump_tex
	cmp	[dr_flag],10
	je	.cubic_env_mapping
				      ; ****************
	mov	esi,point_index3      ; do Gouraud shading
	mov	ecx,3
      .again_grd_draw:
	movzx	eax,word[esi]
	shl	eax,2
	lea	eax,[eax*3]
	add	eax,[points_normals_rot_ptr]
	; texture x=(rotated point normal -> x * 255)+255
	fld	dword[eax]	 ; x cooficient of normal vector
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[esp-2]
	; texture y=(rotated point normal -> y * 255)+255
	fld	dword[eax+4]	  ; y cooficient
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[esp-4]

	movzx	 eax,word[esp-4]
	movzx	 ebx,word[esp-2]
	shl	 eax,TEX_SHIFT
	add	 eax,ebx
	lea	 eax,[eax*3+color_map]
	mov	 eax,dword[eax]
	cmp	[catmull_flag],1      ; put on stack z coordinate if necessary
	jne	 @f
	lea	 edx,[ecx*3]
	push	 word[edx*2+xx1-2]    ; zz1 ,2 ,3
      @@:
	ror	 eax,16 	      ; eax -0xxxrrggbb -> 0xggbbxxrr
	xor	 ah,ah
	push	 ax	    ;r
	rol	 eax,8		      ; eax-0xggbb00rr -> 0xbb00rrgg
	xor	 ah,ah
	push	 ax	    ;g
	shr	 eax,24
	push	 ax	    ;b

	sub	 esi,2
	dec	 cx
	jnz	 .again_grd_draw
	jmp	 .both_draw

   ;     movzx   edi,[point_index3]   ;gouraud shading according to light vector
   ;     lea     edi,[edi*3]
   ;     lea     edi,[4*edi+point_normals_rotated] ; edi - normal
   ;     mov     esi,light_vector
   ;     call    dot_product
   ;     fabs
   ;     fimul   [orginal_color_r]
   ;     fistp   [temp_col]
   ;     and     [temp_col],0x00ff
   ;     push    [temp_col]
   ;     push    [temp_col]
   ;     push    [temp_col]

   ;     movzx   edi,[point_index2]
   ;     lea     edi,[edi*3]
   ;     lea     edi,[4*edi+point_normals_rotated] ; edi - normal
   ;     mov     esi,light_vector
   ;     call    dot_product
   ;     fabs
   ;     fimul   [orginal_color_r]
   ;     fistp    [temp_col]
   ;     and     [temp_col],0x00ff
   ;     push    [temp_col]
   ;     push    [temp_col]
   ;     push    [temp_col]

   ;     movzx   edi,[point_index1]
   ;     lea     edi,[edi*3]
   ;     lea     edi,[4*edi+point_normals_rotated] ; edi - normal
   ;     mov     esi,light_vector
   ;     call    dot_product
   ;     fabs
   ;     fimul   [orginal_color_r]
   ;     fistp   [temp_col]
   ;     and     [temp_col],0x00ff
   ;     push    [temp_col]
   ;     push    [temp_col]
   ;     push    [temp_col]
   .rainbow:
	cmp	[catmull_flag],1      ; put on stack z coordinate if necessary
	jne	 @f
	push	 [zz3]
      @@:
	mov	 eax,dword[yy3]
	mov	 ebx,0x00ff00ff
	and	 eax,ebx
	push	 eax
	neg	 al
	push	 ax
	cmp	[catmull_flag],1
	jne	 @f
	push	 [zz2]
      @@:
	mov	 eax,dword[yy2]
	and	 eax,ebx
	push	 eax
	neg	 al
	push	 ax
	cmp	[catmull_flag],1
	jne	 @f
	push	 [zz1]
      @@:
	mov	 eax,dword[yy1]
	and	 eax,ebx
	push	 eax
	neg	 al
	push	 ax
    .both_draw:
	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	lea	edi,[screen]
	cmp	[catmull_flag],0
	je	@f
  ;      lea     esi,[Z_buffer]
	mov	esi,[Zbuffer_ptr]
	call	gouraud_triangle_z
	jmp	.end_draw
       @@:
	call	gouraud_triangle
	jmp	.end_draw

     .flat_draw:		     ;**************************
				     ; FLAT DRAWING
	movzx	eax,[point_index1]
	movzx	ebx,[point_index2]
	movzx	ecx,[point_index3]
	shl	eax,2
	shl	ebx,2
	shl	ecx,2
	lea	eax,[eax*3]  ;+point_normals_rotated]
	add	eax,[points_normals_rot_ptr]
	lea	ebx,[ebx*3]  ;+point_normals_rotated]
	add	ebx,[points_normals_rot_ptr]
	lea	ecx,[ecx*3]  ;+point_normals_rotated]
	add	ecx,[points_normals_rot_ptr]
	fld	dword[eax]	; x cooficient of normal vector
	fadd	dword[ebx]
	fadd	dword[ecx]
	fidiv	[i3]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	dword[esp-4]	; x temp variables
	fld	dword[eax+4]	; y cooficient of normal vector
	fadd	dword[ebx+4]
	fadd	dword[ecx+4]
	fidiv	[i3]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	dword[esp-8]   ;  y
	mov	edx,dword[esp-8]
	shl	edx,TEX_SHIFT
	add	edx,dword[esp-4]
	lea	eax,[3*edx+color_map]
	mov	edx,dword[eax]

	and	edx,0x00ffffff	  ; edx = 0x00rrggbb



     ;   mov     ax,[zz1]      ; z position depend draw
     ;   add     ax,[zz2]
     ;   add     ax,[zz3]
     ;   cwd
     ;   idiv    [i3] ;    = -((a+b+c)/3+130)
     ;   add     ax,130
     ;   neg     al
     ;   xor     edx,edx
     ;   mov     ah,al           ;set color according to z position
     ;   shl     eax,8
     ;   mov     edx,eax

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
       ; mov     edi,screen
	lea	edi,[screen]
	cmp	[catmull_flag],0
	je	@f
   ;     lea     esi,[Z_buffer]
	mov	esi,[Zbuffer_ptr]
	push	word[zz3]
	push	word[zz2]
	push	word[zz1]
	call	flat_triangle_z
	jmp	.end_draw
      @@:
	call	draw_triangle
	jmp	.end_draw
      .env_mapping:
       ; fninit
	cmp	[catmull_flag],0
	je	@f
	push	[zz3]
	push	[zz2]
	push	[zz1]
      @@:
	mov	esi,point_index1
	sub	esp,12
	mov	edi,esp
	mov	ecx,3
      @@:
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	shl	eax,2
	add	eax,[points_normals_rot_ptr]	   ;point_normals_rotated
; #
;        fld     dword[eax]
;        fmul    dword[eax+4]
;        fld1
;        fld1
;        faddp
;        fmulp
;        fimul   [correct_tex]
;        fiadd   [correct_tex]
;        fistp   word[edi]
;        mov     word[edi+2],0
;;        fistp   word[edi+2]
; # last change
	; texture x=(rotated point normal -> x * 255)+255
	fld	dword[eax]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi]
	; texture y=(rotated point normal -> y * 255)+255
	fld	dword[eax+4]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi+2]
; # end of last ch.
	add	edi,4
	add	esi,2
	loop	@b

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	esi,envmap
	cmp	[catmull_flag],0
	je	@f
  ;      mov     edx,Z_buffer
	mov	edx,[Zbuffer_ptr]
	call	tex_triangle_z
	jmp	.end_draw
      @@:
	call	tex_triangle
	jmp	.end_draw
;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     .cubic_env_mapping:
       ; fninit
	cmp	[catmull_flag],0
	je	@f
	push	[zz3]
	push	[zz2]
	push	[zz1]
      @@:
	mov	esi,point_index1
	sub	esp,12
	mov	edi,esp
	mov	ecx,3
      @@:
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	shl	eax,2
	add	eax,[points_normals_rot_ptr]  ;point_normals_rotated
; #
	fld	dword[eax]
	fmul	dword[eax+4]
	fld1
	fld1
	faddp
	fmulp
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi]
	mov	word[edi+2],0
;        fistp   word[edi+2]
; # last change
;        ; texture x=(rotated point normal -> x * 255)+255
;        fld     dword[eax]
;        fimul   [correct_tex]
;        fiadd   [correct_tex]
;        fistp   word[edi]
;        ; texture y=(rotated point normal -> y * 255)+255
;        fld     dword[eax+4]
;        fimul   [correct_tex]
;        fiadd   [correct_tex]
;        fistp   word[edi+2]
; # end of last ch.
	add	edi,4
	add	esi,2
	loop	@b

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	esi,envmap_cub
	cmp	[catmull_flag],0
	je	@f
  ;      mov     edx,Z_buffer
	mov	edx,[Zbuffer_ptr]
	call	tex_triangle_z
	jmp	.end_draw
      @@:
	call	tex_triangle
	jmp	.end_draw

;+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      .bump_mapping:
	; fninit
	cmp	[catmull_flag],0
	je	@f
;        push    Z_buffer
	push	[Zbuffer_ptr]
	push	[zz3]
	push	[zz2]
	push	[zz1]
      @@:
	mov	esi,point_index1
	sub	esp,12
	mov	edi,esp
	mov	ecx,3
      @@:
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	shl	eax,2
	add	eax,[points_normals_rot_ptr]  ;point_normals_rotated
	; texture x=(rotated point normal -> x * 255)+255
	fld	dword[eax]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi]
	; texture y=(rotated point normal -> y * 255)+255
	fld	dword[eax+4]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi+2]

	add	edi,4
	add	esi,2
	loop	@b

	movzx  esi,[point_index3]      ; bump map coords
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index2]
	shl    esi,2
	add    esi,tex_points
;       lea    esi,[esi*3]
;       lea    esi,[points+2+esi*2]
	push   dword[esi]
  ;     push   dword[xx2]
	movzx  esi,[point_index1]
	shl    esi,2
	add    esi,tex_points
;       lea     esi,[esi*3]
;       lea     esi,[points+2+esi*2]
	push   dword[esi]
   ;    push     dword[xx1]

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	esi,envmap
	mov	edx,bumpmap	       ;BUMP_MAPPING

	cmp	[catmull_flag],0
	je	@f
	call	bump_triangle_z
	jmp	.end_draw
      @@:
	call	bump_triangle
	jmp	.end_draw

      .tex_mapping:

	; fninit
	cmp	[catmull_flag],0
	je	@f
	push	[zz3]
	push	[zz2]
	push	[zz1]
      @@:
	movzx  esi,[point_index3]      ; tex map coords
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index2]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index1]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	esi,texmap
	cmp	[catmull_flag],0
	je	@f
   ;     mov     edx,Z_buffer
	mov	edx,[Zbuffer_ptr]
	call	tex_triangle_z
 ;       call    tex_plus_grd_trianlgle
	jmp	.end_draw
      @@:
	call	tex_triangle
	jmp	.end_draw
;      .ray:
;        grd_triangle according to points index
;        cmp     [catmull_flag],0
;        je      @f
;        push    [zz3]                   ; spot light with attenuation
;     @@:
;        movzx   eax,[point_index3]      ; env_map - points color list
;        shl     eax,1                   ; each color as word, 0x00rr00gg00bb..
;        lea     eax,[3*eax+bumpmap]
;        push    word[eax]
;        push    word[eax+2]
;        push    word[eax+4]
;        cmp     [catmull_flag],0
;        je      @f
;        push    [zz2]
;    @@:
;        movzx   eax,[point_index2]      ; env_map - points color list
;        shl     eax,1                   ; each color as word, 0x00rr00gg00bb..
;        lea     eax,[eax*3+bumpmap]
;        push    word[eax]
;        push    word[eax+2]
;        push    word[eax+4]
;        cmp     [catmull_flag],0
;        je      @f
;        push    [zz1]
;     @@:
;        movzx   eax,[point_index1]      ; env_map - points color list
;        shl     eax,1                   ; each color as word, 0xrr00gg00bb00..
;        lea     eax,[eax*3+bumpmap]
;        push    word[eax]
;        push    word[eax+2]
;        push    word[eax+4]
;        jmp     .both_draw

     .grd_tex:		  ; smooth shading + texture
	 push	ebp
	 mov	ebp,esp
	 sub	esp,4
	 push	ebp

	 movzx	esi,[point_index3]	; tex map coords
	 shl	esi,2
	 add	esi,tex_points
	 push	dword[esi]		; texture coords as first
	 movzx	esi,[point_index2]	; group of parameters
	 shl	esi,2
	 add	esi,tex_points
	 push	dword[esi]
	 movzx	esi,[point_index1]
	 shl	esi,2
	 add	esi,tex_points
	 push	dword[esi]

	 mov	 esi,point_index3
	 mov	 ecx,3

      .aagain_grd_draw:

	lea	 edx,[ecx*3]
	push	 word[edx*2+xx1-2]    ; zz1 ,2 ,3

	movzx	eax,word[esi]
	shl	eax,2
	lea	eax,[eax*3] ;+point_normals_rotated]
	add	eax,[points_normals_rot_ptr]
	; texture x=(rotated point normal -> x * 255)+255
	fld	dword[eax]	 ; x cooficient of normal vector
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[ebp-2]
	; texture y=(rotated point normal -> y * 255)+255
	fld	dword[eax+4]	  ; y cooficient
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[ebp-4]

	movzx	 eax,word[ebp-4]
	movzx	 ebx,word[ebp-2]
	shl	 eax,TEX_SHIFT
	add	 eax,ebx
	lea	 eax,[eax*3+color_map]
	mov	 eax,dword[eax]

	ror	 eax,16 	      ; eax -0xxxrrggbb -> 0xggbbxxrr
	xor	 ah,ah
	push	 ax	    ;r
	rol	 eax,8		      ; eax-0xggbb00rr -> 0xbb00rrgg
	xor	 ah,ah
	push	 ax	    ;g
	shr	 eax,24
	push	 ax	    ;b

	sub	 esi,2
	dec	 cx
	jnz	 .aagain_grd_draw

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	edx,texmap
	mov	esi,[Zbuffer_ptr]

	call	tex_plus_grd_triangle

	pop	ebp
	mov	esp,ebp
	pop	ebp
	jmp	.end_draw

      .two_tex:
	push	[Zbuffer_ptr]

	push	word[zz3]
	push	word[zz2]
	push	word[zz1]

	movzx  esi,[point_index3]      ; tex map coords
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index2]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index1]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]

	mov	esi,point_index1     ; env coords
	sub	esp,12
	mov	edi,esp
	mov	ecx,3
      @@:
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	shl	eax,2
	add	eax,[points_normals_rot_ptr]
	; texture x=(rotated point normal -> x * 255)+255
	fld	dword[eax]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi]
	; texture y=(rotated point normal -> y * 255)+255
	fld	dword[eax+4]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi+2]

	add	edi,4
	add	esi,2
	loop	@b

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	esi,texmap
	mov	edx,envmap

	call	two_tex_triangle_z
	jmp	.end_draw

   .bump_tex:
	movzx  esi,[point_index3]      ; tex map coords
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index2]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index1]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]

	push  dword texmap

	push  [Zbuffer_ptr]
	xor   edi,edi

	push	word[zz3]
	push	word[zz2]
	push	word[zz1]

	mov	esi,point_index1     ; env coords
	sub	esp,12
	mov	edi,esp
	mov	ecx,3
      @@:
	movzx	eax,word[esi]
	lea	eax,[eax*3]
	shl	eax,2
	add	eax,[points_normals_rot_ptr]
	; texture x=(rotated point normal -> x * 255)+255
	fld	dword[eax]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi]
	; texture y=(rotated point normal -> y * 255)+255
	fld	dword[eax+4]
	fimul	[correct_tex]
	fiadd	[correct_tex]
	fistp	word[edi+2]

	add	edi,4
	add	esi,2
	loop	@b

;        push  dword 1 shl 16 + 1  ; emap coords
;        push  dword 127 shl 16 + 1
;        push  dword 127 shl 16 + 127

	movzx  esi,[point_index3]      ; bump map coords
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]
	movzx  esi,[point_index2]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]

	movzx  esi,[point_index1]
	shl    esi,2
	add    esi,tex_points
	push   dword[esi]

;        push  dword 1 shl 16 + 127
;        push  dword 127 shl 16 + 127
;        push  dword 1 shl 16 + 1  ; bump coords

	mov	eax,dword[xx1]
	ror	eax,16
	mov	ebx,dword[xx2]
	ror	ebx,16
	mov	ecx,dword[xx3]
	ror	ecx,16
	mov	edi,screen
	mov	esi,envmap
	mov	edx,bumpmap

	call bump_tex_triangle_z

      .end_draw:
	pop	esi
	add	esi,6
	cmp	dword[esi],-1
	jne	.again_dts
ret


fill_Z_buffer:
	mov	eax,0x70000000
      ;  mov     edi,Z_buffer
	mov	edi,[Zbuffer_ptr]
	mov	ecx,SIZE_X*SIZE_Y
	rep	stosd
ret

read_from_file:
	fninit
	mov	edi,[triangles_ptr]
	xor	ebx,ebx
	xor	ebp,ebp
	mov	[points_count_var],0
	mov	[triangles_count_var],0
   if USE_LFN = 0
	mov	esi,SourceFile
   else
	mov	esi,[fptr]
   end if
	cmp	[esi],word 4D4Dh
	jne	.exit ;Must be legal .3DS file
;        cmp     dword[esi+2],EndFile-SourceFile
;        jne     .exit ;This must tell the length
	mov	eax,dword[esi+2]
  ;      cmp     eax,[fsize]
  ;      jne     .exit

	add	eax,esi
	mov	[EndFile],eax	 ;

	add	esi,6
      @@:
	cmp	[esi],word 3D3Dh
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	add	esi,6
      .find4k:
	cmp	[esi],word 4000h
	je	@f
	add	esi,[esi+2]
	cmp	esi,[EndFile]
	jc	.find4k
	jmp	.exit
      @@:
	add	esi,6
      @@:
	cmp	[esi],byte 0
	je	@f
	inc	esi
	jmp	@b
      @@:
	inc	esi
      @@:
	cmp	[esi],word 4100h
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	add	esi,6
      @@:
	cmp	[esi],word 4110h
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	movzx	ecx,word[esi+6]
	add	[points_count_var],cx

	mov	edx,ecx
	add	esi,8
     @@:
	push	edi
	mov	edi,[points_ptr]
	push	dword[esi+4]
	pop	dword[edi+ebx*2+0]
	push	dword[esi+8]
	pop	dword[edi+ebx*2+4]
	push	dword[esi+0]
	pop	dword[edi+ebx*2+8]
	pop	edi
;        fld     dword[esi+4]
;        fstp    dword[real_points+ebx*2+0]  ; x
;        fld     dword[esi+8]
;        fstp   dword[real_points+ebx*2+4]  ; y
;        fld     dword[esi+0]
;        fstp   dword[real_points+ebx*2+8]  ; z

	add	ebx,6
	add	esi,12
	dec	ecx
	jnz	@b
      @@:
  ;      mov     dword[points+ebx],-1
	push	edi
	mov	edi,[points_ptr]
	mov	dword[edi+ebx*2],-1	   ; end mark (not always in use)
	pop	edi
      @@:
	cmp	[esi],word 4120h
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	movzx	ecx,word[esi+6]
	add	[triangles_count_var],cx
	add	esi,8
	;mov     edi,triangles
      @@:
	movsd
	movsw
	add	word[edi-6],bp
	add	word[edi-4],bp
	add	word[edi-2],bp
	add	esi,2
	dec	ecx
	jnz	@b
	add	ebp,edx
	jmp	.find4k
	mov	eax,-1 ;<---mark if OK
      .exit:
	mov	dword[edi],-1
ret

read_tp_variables:	      ; read [triangles_count_var] and  [points_count_var]
			      ; and  allocate memory
	xor	ebx,ebx
	xor	ebp,ebp
	mov	[points_count_var],bx
	mov	[triangles_count_var],bx
   if USE_LFN = 0
	mov	esi,SourceFile
   else
	mov	esi,[fptr]
   end if

	cmp	[esi],word 4D4Dh
	je	@f ;Must be legal .3DS file
	xor	eax,eax
	ret
    @@:
	mov	eax,dword[esi+2]
	cmp	eax,[fsize] ;This must tell the length
	je	@f
	xor	eax,eax
	ret
     @@:
	add	eax,esi
	mov	[EndFile],eax	 ;

	add	esi,6
      @@:
	cmp	[esi],word 3D3Dh
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	add	esi,6
      .find4k:
	cmp	[esi],word 4000h
	je	@f
	add	esi,[esi+2]
	cmp	esi,[EndFile]
	jc	.find4k
	jmp	.exit
      @@:
	add	esi,6
      @@:
	cmp	[esi],byte 0
	je	@f
	inc	esi
	jmp	@b
      @@:
	inc	esi
      @@:
	cmp	[esi],word 4100h
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	add	esi,6
      @@:
	cmp	[esi],word 4110h
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	movzx	ecx,word[esi+6]
	add	[points_count_var],cx

	mov	edx,ecx
	add	esi,8
     @@:

	add	ebx,6
	add	esi,12
     ;   dec     ecx
	loop	 @b
      @@:

      @@:
	cmp	[esi],word 4120h
	je	@f
	add	esi,[esi+2]
	jmp	@b
      @@:
	movzx	ecx,word[esi+6]
	add	[triangles_count_var],cx
	add	esi,8

      @@:
	add	esi,8
	dec	ecx
	jnz	@b
;        xor     ecx,ecx
	add	ebp,edx
	jmp	.find4k
	mov	eax,-1 ;<---mark if OK
      .exit:
ret

if USE_LFN
alloc_mem_for_tp:
	mov	eax, 68
	cmp	[re_alloc_flag],1
	jz	@f
	mov	ebx, 12
	jmp	.alloc
    @@:
	mov	ebx,20
    .alloc:
	movzx	ecx, [triangles_count_var]
	inc	ecx
	lea	ecx, [ecx*3]
	add	ecx, ecx
	mov	edx,[triangles_ptr]
	int	0x40		       ;  -> allocate memory to triangles
	mov	[triangles_ptr], eax   ;  -> eax = pointer to allocated mem

;        mov     eax, 68
;        mov     ebx, 12
;        movzx   ecx, [triangles_count_var]
;        shl     ecx, 4
;        int     0x40
;        mov     [triangles_w_z_ptr], eax   ; for trainagles_with_z list
					    ; ststic  memory

	mov	eax, 68
	movzx	ecx, [triangles_count_var]
	lea	ecx, [3+ecx*3]
	shl	ecx, 2
	mov	edx,[triangles_normals_ptr]
	int	0x40			       ;  -> allocate memory for triangles normals
	mov	[triangles_normals_ptr], eax   ;  -> eax = pointer to allocated mem

	mov	eax, 68
	movzx	ecx, [points_count_var]
	lea	ecx,[3+ecx*3]
	shl	ecx, 2
	mov	edx,[points_normals_ptr]
	int	0x40
	mov	[points_normals_ptr], eax

	mov	eax, 68
    ;    mov     ebx, 12
	movzx	ecx, [points_count_var]
	lea	ecx,[3+ecx*3]
	shl	ecx, 2
	mov	edx,[points_normals_rot_ptr]
	int	0x40
	mov	[points_normals_rot_ptr], eax

	mov	eax, 68
	mov	edx,[points_ptr]
	int	0x40
	mov	[points_ptr], eax

	mov	eax, 68
	mov	edx,[points_rotated_ptr]
	int	0x40
	mov	[points_rotated_ptr], eax

	mov	eax, 68
	movzx	ecx, [points_count_var]
	inc	ecx
	shl	ecx, 3
	mov	edx,[points_translated_ptr]
	int	0x40
	mov	[points_translated_ptr], eax
ret
end if


read_from_disk:
if USE_LFN
;-
    mov     eax, 70
    mov     ebx, file_info
    mov     dword[ebx], 5	   ;  -> subfunction number
    int     0x40		   ;  -> read file size
    mov     ebx, [fptr]
    mov     ebx, dword[ebx+32]
    inc     ebx
    mov     [fsize], ebx

    mov     eax, 68
    mov     ebx, 11
    int     0x40		   ;  -> create heap

    mov     eax, 68
    mov     ebx, 12
    mov     ecx, [fsize]
    int     0x40		   ;  -> allocate memory for file
    mov     [fptr], eax 	   ;  -> eax = pointer to allocated mem

    mov     eax, 70
    mov     ebx, file_info
    mov     dword[ebx],0
    int     0x40		   ; -> read file

    mov     [fsize],ebx

    cmp     eax,6
    jnz     @f
    xor     eax,eax	;;;;---
  @@:
else
    mov      eax,58
    mov      ebx,file_info
    int      0x40

    mov      eax,ebx
    shr      eax,9
    inc      eax
    mov      [fsize],eax
;    mov      ecx,ebx
;    add      ecx,MEM_END
;    mov      ebx,1
;    mov      eax,64     ; allocate mem  - resize app mem
			; for points and  triangles
    int      0x40

    mov      eax,58
    mov      ebx,file_info
    int      0x40
end if
  ;  eax = 0   -> ok file loaded
ret
read_param:
    mov        esi,I_Param
    cmp        dword[esi],0
    je	       .end
    cmp        byte[esi],'/'
    je	       .copy
    mov        edi,esi
    mov        ecx,25	; 25 - would be enought
    repe       scasb
    jne        .end
    dec        edi
    mov        esi,edi
 .copy:
    mov 	edi,file_name
    mov 	ecx,50
    rep 	movsd
 .end:
ret
buttons:				      ; draw some buttons (all but navigation and close )
	mov	edi,menu
      .again:
	mov	eax,8			      ; function 8 : define and draw button
	mov	ebx,(SIZE_X+10)*65536+62      ; [x start] *65536 + [x size]
	movzx	ecx,byte[edi]		      ; button id = position+2
	sub	cl,2
	lea	ecx,[ecx*5]
	lea	ecx,[ecx*3]
	add	ecx,25
	shl	ecx,16
	add	ecx,12
	movzx	edx,byte[edi]			; button id
	mov	esi,0x6688dd			; button color RRGGBB
	int	0x40
	 ; BUTTON  LABEL
	mov	eax,4				; function 4 : write text to window
	movzx	ebx,byte[edi]
	sub	bl,2				; button id, according to position
	lea	ebx,[ebx*3]
	lea	ebx,[ebx*5]
	add	ebx,(SIZE_X+12)*65536+28	; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff			; font 1 & color ( 0xF0RRGGBB )
	lea	edx,[edi+1]			; pointer to text beginning
	mov	esi,10				; text length
	int	0x40
	cmp	byte[edi+11],255		; if max_flag=255
	je	@f				; skip
	; flag description
;       mov     eax,4                           ; function 4 : write text to window
;       movzx   ebx,byte[edi]
;       sub     bl,2
;       lea     ebx,[ebx*3]
;       lea     ebx,[ebx*5]
;       add     ebx,(SIZE_X+12+70)*65536+28     ; [x start] *65536 + [y start]
	add	ebx,70*65536
;       mov     ecx,0x20ddeeff                  ; font 1 & color ( 0xF0RRGGBB )
	movzx	edx,byte[edi+12]		; current flag
	shl	edx,2				; * 4 = text length
	add	edx,dword[edi+13]		; pointer to text beginning
	mov	esi,4				; text length
	int	0x40

    @@:
	add	edi,17
	cmp	byte[edi],-1
	jnz	.again
ret
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
    draw_window:
	mov	eax,12		; function 12:tell os about windowdraw
	mov	ebx,1		; 1, start of draw
	int	0x40

	; DRAW WINDOW
	mov	eax,0		; function 0 : define and draw window
	mov	ebx,100*65536;+SIZE_X;+80+30 ; [x start] *65536 + [x size]
	mov	ecx,100*65536;+SIZE_Y;+30    ; [y start] *65536 + [y size]
	mov	bx,[size_x]
	add	bx,110
	mov	cx,[size_y]
	add	cx,30
	mov	edx,0x14000000	; color of work area RRGGBB,8->color gl
	mov	esi,0x805080d0	; color of grab bar  RRGGBB,8->color gl
	mov	edi,0x005080d0	; color of frames    RRGGBB
	int	0x40

	; WINDOW LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,8*65536+8	; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelt	; pointer to text beginning
	mov	esi,labellen-labelt	; text length
	int	0x40

	; CLOSE BUTTON
;       mov     eax,8           ; function 8 : define and draw button
;       movzx   ebx,[size_x]
;       shl     ebx,16
;       add     ebx, 91 shl 16 + 12
;;        mov     ebx,(SIZE_X+80+30-19)*65536+12     ; [x start] *65536 + [x size]
;       mov     ecx,5*65536+12  ; [y start] *65536 + [y size]
;       mov     edx,1           ; button id
;       mov     esi,0x6688dd    ; button color RRGGBB
;       int     0x40

	call	buttons 	; more buttons

	.Y_ADD equ 1   ;-> offset of 'add vector' buttons

	; ADD VECTOR LABEL      ; add vector buttons - 30 ++
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+12)*65536+(168+15*(13+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelvector      ; pointer to text beginning
	mov	esi,labelvectorend-labelvector	   ; text length
    ;    cmp     [move_flag],2
    ;    jne     @f
    ;    add     edx,navigation_size
    ;  @@:
	int	0x40
	 ; VECTOR Y- BUTTON
	mov	eax,8		; function 8 : define and draw button
	mov	ebx,(SIZE_X+30)*65536+20     ; [x start] *65536 + [x size]
	mov	ecx,(165+15*(14+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
	mov	edx,30		 ; button id
	mov	esi,0x6688dd	; button color RRGGBB
	int	0x40
	;VECTOR Y- LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+32)*65536+(168+15*(14+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelyminus      ; pointer to text beginning
	mov	esi,labelyminusend-labelyminus	   ; text length
	cmp	[move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
	int	0x40
	; VECTOR Z+ BUTTON
	mov	eax,8		; function 8 : define and draw button
	mov	ebx,(SIZE_X+51)*65536+21     ; [x start] *65536 + [x size]
	mov	ecx,(165+15*(14+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
	mov	edx,31		 ; button id
	mov	esi,0x6688dd	; button color RRGGBB
	int	0x40
	;VECTOR Z+ LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+53)*65536+(168+15*(14+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelzplus	    ; pointer to text beginning
	mov	esi,labelzplusend-labelzplus	 ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:

	int	0x40
	; VECTOR x- BUTTON
	mov	eax,8		; function 8 : define and draw button
	mov	ebx,(SIZE_X+10)*65536+21     ; [x start] *65536 + [x size]
	mov	ecx,(165+15*(15+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
	mov	edx,32		 ; button id
	mov	esi,0x6688dd	; button color RRGGBB
	int	0x40
	;VECTOR x- LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+12)*65536+(168+15*(15+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelxminus      ; pointer to text beginning
	mov	esi,labelxminusend-labelxminus	   ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
	int	0x40
	; VECTOR x+ BUTTON
	mov	eax,8		; function 8 : define and draw button
	mov	ebx,(SIZE_X+51)*65536+21     ; [x start] *65536 + [x size]
	mov	ecx,(165+15*(15+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
	mov	edx,33		 ; button id
	mov	esi,0x6688dd	; button color RRGGBB
	int	0x40
	;VECTOR x+ LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+53)*65536+(168+15*(15+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelxplus	    ; pointer to text beginning
	mov	esi,labelxplusend-labelxplus	 ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
	int	0x40
	; VECTOR z- BUTTON
	mov	eax,8		; function 8 : define and draw button
	mov	ebx,(SIZE_X+10)*65536+62-41	; [x start] *65536 + [x size]
	mov	ecx,(25+140+15*(16+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
	mov	edx,34		 ; button id
	mov	esi,0x6688dd	; button color RRGGBB
	int	0x40
	;VECTOR z- LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+12)*65536+(168+15*(16+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelzminus      ; pointer to text beginning
	mov	esi,labelzminusend-labelzminus	   ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
	int	0x40
       ;VECTOR Y+ BUTTON
	mov	eax,8		; function 8 : define and draw button
	mov	ebx,(SIZE_X+10+20)*65536+20	; [x start] *65536 + [x size]
	mov	ecx,(165+15*(16+.Y_ADD))*65536+12  ; [y start] *65536 + [y size]
	mov	edx,35		 ; button id
	mov	esi,0x6688dd	; button color RRGGBB
	int	0x40
	;VECTOR Y+ LABEL
	mov	eax,4		; function 4 : write text to window
	mov	ebx,(SIZE_X+32)*65536+(168+15*(16+.Y_ADD))   ; [x start] *65536 + [y start]
	mov	ecx,0x20ddeeff	; font 1 & color ( 0xF0RRGGBB )
	mov	edx,labelyplus	    ; pointer to text beginning
	mov	esi,labelyplusend-labelyplus	 ; text length
   ;     cmp     [move_flag],2
   ;     jne     @f
   ;     add     edx,navigation_size
   ;   @@:
	int	0x40

	mov	eax,12		; function 12:tell os about windowdraw
	mov	ebx,2		; 2, end of draw
	int	0x40
	ret


   ; DATA AREA  ************************************

   include 'data.inc'

MEM_END:
