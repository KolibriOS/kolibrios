   ; DATA AREA  ************************************

	i3		dw	3
	i256		dw	256
	i255d		dd	255
	dot_max 	dd	1.0	 ; dot product max and min
	dot_min 	dd	0.0
	env_const	dd	1.05
	correct_tex	dw	255
	tex_x_div2	dw	TEX_X  / 2
	tex_y_div2	dw	TEX_Y  / 2
	xobs		dw	0 ;SIZE_X / 2 ;200 ;observer  = camera
	yobs		dw	0 ;SIZE_Y / 2 ;200 ;coordinates
	zobs		dw	-500
	size_x		dw	SIZE_X
	size_y		dw	SIZE_Y

	re_alloc_flag db 0
	angle_counter dw 0
	piD180	      dd 0.017453292519943295769236907684886
	piD128	      dd 0.024544
	const6	      dw 6,6,6,6
	x_offset      dw SIZE_X / 2
	y_offset      dw SIZE_Y / 2
	z_offset      dw 0
	rsscale       dd 175.0		 ; next real scale
	vect_x	      dw SIZE_X / 2
	vect_y	      dw SIZE_Y / 2
	vect_z	      dw 0
	angle_x       dw 0
	angle_y       dw 0
	angle_z       dw 0
	sin_amplitude dd 50
	sin_frq       dd 0.7
	sin_delta     dd 0.07	 ; wave frequency granularity


	menu:
	       db 2		   ; button number  = index
	       db 'rotary    '	   ; label
	       db 3		   ; max flag  + 1 , if = 255, no flag
 r_flag        db 1		   ; flag
	       dd axl_f 	   ; offset to flags description

	       db 3
	       db 'shd. model'
	       db 12
 dr_flag       db 0
	       dd shd_f

	       db 4
	       db 'speed     '
	       db 2
 speed_flag    db 0
	       dd spd_f

	       db 5
	       db 'zoom out  '
	       db 255
	       db ?
	       dd ?

	       db 6
	       db 'zoom in   '
	       db 255
	       db ?
	       dd ?

	       db 7
	       db 'catmull   '
	       db 2
catmull_flag   db 1
	       dd onoff_f

	       db 8
	       db 'culling   '
	       db 2
culling_flag   db 1
	       dd onoff_f

	       db 9
	       db 'rand.light'
	       db 255
	       db ?
	       dd ?

	       db 10
	       db 'blur      '
	       db 6
blur_flag      db 0
	       dd blur_f

	       db 11
	       db 'mirror x  '
	       db 2
mirr_x_flag    db 0
	       dd onoff_f

	       db 12
	       db 'mirror y  '
	       db 2
mirr_y_flag    db 0
	       dd onoff_f

	       db 13
	       db 'mirror z  '
	       db 2
mirr_z_flag    db 0
	       dd onoff_f

	       db 14
	       db 'xchg      '
	       db 4
xchg_flag      db 0
	       dd xchg_f

	       db 15
	       db 'emboss    '
	       db 2
emboss_flag    db 0
	       dd onoff_f

	       db 16
	       db 'fire      '
	       db 3
fire_flag      db 0
	       dd blur_f

	       db 17
	       db 'move      '
	       db 3
move_flag      db 0
	       dd move_f

		  db 18
		  db 'generate  '
		  db 6
generator_flag	  db 0
		  dd blur_f

		  db 19
		  db 'bumps     '
		  db 2
bumps_flag	  db 0
		  dd bumps_f

		  db 20
		  db 'bumps deep'
		  db 4
bumps_deep_flag   db 3
		  dd bumps_d_f

		  db 21
		  db 're-map tex'
		  db 255
map_tex_flag	  db ? ;1
		  dd ?	;bumps_d_f

		  db 22
		  db 'bright +  '
		  db 6
inc_bright_flag   db 0 ;1
		  dd blur_f

		  db 23
		  db 'bright -  '
		  db 6
dec_bright_flag   db 0 ;1
		  dd blur_f

		  db 24
		  db 'wav effect'
		  db 2
sinus_flag	  db 0
		  dd onoff_f

;                  db 24
;                  db 'max       '
;                  db 2
;max_flag          db 0 ;1
;                  dd blur_f
;
;                  db 25
;                  db 'min       '
;                  db 2
;min_flag          db 0 ;1
;                  dd blur_f



;                  db 21
;                  db 'light No. '
;                  db 3
;light_no_flag     db 0
;                  dd bumps_d_f

;                db 22
;                db 'light comp'
;                db 3
;light_comp_flag db 0
;                dd light_component_f

;;                db 23
;;                db 'col. comp'
;;                db 3
;;color_comp_flag db 0
;;                dd color_component_f


	       db -1	   ; end mark


flags:		   ; flags description
 shd_f:
    db 'flat'
    db 'grd '
    db 'env '
    db 'bump'
    db 'tex '
    db 'pos '
    db 'dots'
    db 'txgr'
    db '2tex'
    db 'btex'
    db 'cenv'
    db 'grdl'
 spd_f:
    db 'idle'
    db 'full'
 axl_f:
    db ' y  '
    db 'x+y '
    db ' x  '
 onoff_f:
    db 'off '
    db 'on  '
; light_component_f:
;    db 'norm '                ; diffuse   |
;    db 'min'                  ; specular  |  or sth. like this
;    db 'max '                 ; emmisive  |

;; color_component_f:
;;    db ' r  '
;;    db ' g  '
;;    db ' b  '

	  blur_f:	       ; blur, fire
	    db 'off '
bumps_d_f:  db ' 1  '
	    db ' 2  '
	    db ' 3  '
	    db ' 4  '
	    db ' 5  '

 xchg_f:
    db 'no  '
    db 'x<>y'
    db 'z<>x'
    db 'y<>z'
 move_f:
    db 'obj '
    db 'camr'
    db 'wave'
;    db 'lght'
 bumps_f:
    db 'rand'
    db 'tex '
;    db 'cscl'
base_vector:
    labelvector:
	db   'add vector'
    labelvectorend:
    labelyminus:
	db   'y -'
    labelyminusend:
    labelzplus:
	db   'z +'
    labelzplusend:
    labelxminus:
	db   'x -'
    labelxminusend:
    labelxplus:
	db   'x +'
    labelxplusend:
    labelzminus:
	db   'z -'
    labelzminusend:
    labelyplus:
	db   'y +'
    labelyplusend:

;navigation_size = $ - labelvector
;        db   'set color '
;        db   'r -'
;        db   'g +'
;        db   'b -'
;        db   'b +'
;        db   'g -'
;        db   'r +'

    labelt:
	db   'DEUS CARITAS EST'
      if Ext=MMX
	db   ' (MMX)'
      end if
      if Ext=SSE
	db   ' (SSE)'
      end if
      if Ext=SSE2
	db   ' (SSE2)'
      end if
	db   ' 0.064',0
    labellen:
	STRdata db '-1        '

    all_lights_size dw lightsend-lights

if USE_LFN

	file_info:
		dd	0
		dd	0
		dd	0
	 fsize	dd	0 ;180000 ; sizeof(workarea)
	 fptr	dd	0 ;workarea
	file_name:
		db	'/rd/1/3d/house.3ds',0

else

	file_info:
		dd 0
		dd 0
	 fsize	dd 1
		dd workarea
		dd hash_table
	file_name:
		db   '/rd/1/teapot.3ds',0
end if

I_END:

      rb 256

;=============================================
lights:
     .light_vector	  dd	  0.0,0.0,-1.0	 ; x,y,z  Z cooficient of vector must be negative
     .orginal_color_r	  db	  1		 ;  +12
     .orginal_color_g	  db	  255		   ;
     .orginal_color_b	  db	  1		 ;  +14
     .min_color_r	  db	  1		 ;
     .min_color_g	  db	  1		 ;  +16
     .min_color_b	  db	  1		 ;
     .max_color_r	  db	  255		 ;
     .max_color_g	  db	  255		 ;
     .max_color_b	  db	  255		 ;
     .shine		  db	  24		 ;  +21
 ;    LIGHT_SIZE equ ($-lights)

	dd	-0.5,-0.5,-1.0	 ; x,y,z ;  .light_vector
	db	5		 ; .orginal_color_r
	db	1		 ; .orginal_color_g
	db	135		 ; .orginal_color_b
	db	19		 ; .min_color_r
	db	19		 ; .min_color_g
	db	19		 ; .min_color_b
	db	255		 ; .max_color_r
	db	255		 ; .max_color_g
	db	255		 ; .max_color_b
	db	16		 ; .shine

	dd	0.5,0.5,-1.0	; x,y,z ;  .light_vector
	db	135		; .orginal_color_r
	db	1		; .orginal_color_g
	db	1		; .orginal_color_b
	db	19		; .min_color_r
	db	19		; .min_color_g
	db	19		; .min_color_b
	db	255		; .max_color_r
	db	255		; .max_color_g
	db	20		; .max_color_b
	db	16		; .shine
;        ALL_LIGHTS_SIZE equ ($ - lights)
;#all_lights_size dw ($ - lights) ;ALL_LIGHTS_SIZE
;===============================================

lightsend:

if USE_LFN = 0
hash_table rb 4096
SourceFile:
workarea   rb 180000
else
SourceFile:
workarea   rb 180
end if
EndFile   dd ?
align 8
	sinbeta dd	?;+32
	cosbeta dd	?

	xsub	dw	?
	zsub	dw	?;+40
	ysub	dw	?

	xx1	dw	?
	yy1	dw	?
	zz1	dw	?;+48    xx1 + 4
	xx2	dw	?
	yy2	dw	?
	zz2	dw	?      ; xx1 + 10
	xx3	dw	?;+56
	yy3	dw	?
	zz3	dw	?      ; xx1 + 16
	col1	dd	?
	col2	dd	?
	col3	dd	?
	scale	dd	? ; help scale variable
	edges_counter dd ?
   ;==
	triangles_count_var	dw ?
	points_count_var	dw ?
	triangles_ptr		dd ?
	triangles_w_z_ptr	dd ?
	triangles_normals_ptr	dd ?
	points_normals_ptr	dd ?
	points_normals_rot_ptr	dd ?
	points_ptr		dd ?
	points_rotated_ptr	dd ?
	points_translated_ptr	dd ?
	screen_ptr		dd ?
	Zbuffer_ptr		dd ?
     ;   edges_ptr               dd ?

   ;===

	point_index1	    dw ?   ;-\
	point_index2	    dw ?   ;  }  don't change order
	point_index3	    dw ?   ;-/
	temp_col	    dw ?
	high		    dd ?
	rand_seed	    dw ?
align 8
	buffer	dq	?
	errr	dd	?
	drr	dd	?
	xx	dd	?
	yy	dd	?
	xst	dd	?
	yst	dd	?
;        screen_ptr      dd     ?
;        Zbuffer_ptr     dd     ?

	matrix	rb 36
	cos_tab rd 360
	sin_tab rd 360

align 16

if USE_LFN = 0
     points:
	rw (EndFile-SourceFile)/12*3
	points_count = ($-points)/6
     triangles:
	rw  (EndFile-SourceFile)/12*3
	triangles_count = ($-triangles)/6
align 16
	real_points rd points_count*3 + 1
align 16
	rotated_points_r rd points_count*3 + 1
align 16
	points_rotated rw points_count*3 + 2 ;means translated
align 16
	triangles_normals rb triangles_count * 12 ;
align 16
	point_normals rb points_count * 12  ;one 3dvector - triple float dword x,y,z
align 16
	point_normals_rotated rb points_count * 12
align 16
	triangles_normals_rotated rb triangles_count * 12

else
    points_count = 180000/6*3
    triangles_count = 180000 / 6  ;($-triangles)/6
end if
align 16
	label trizdd dword
	label trizdq qword
	triangles_with_z rw triangles_count*4 + 2 ; triangles triple dw + z position
align 16
	vectors rb 24
;align 16
;        points_color rb 6*points_count    ; each color as word
;        sorted_triangles rw triangles_count*3 + 2
align 16
	bumpmap 	rb	TEXTURE_SIZE + 1
align 16
	bumpmap2	rb	TEXTURE_SIZE + 1
align 16
	envmap		rb	(TEXTURE_SIZE +1) * 3
align 16
	envmap_cub	rb	TEX_X * 3
align 16
	texmap		rb	(TEXTURE_SIZE +1) * 3
align 16
	color_map	rb	(TEXTURE_SIZE +1) * 3
align 16
	tex_points	rb	points_count * 4  ; bump_map  and texture coords
				; each point word x, word y
align 16
if Ext >= SSE2
	sse_repository	rb	1024
end if
  ;      SourceFile:             ; source file temporally in screen area
  ;      workarea        dd      ?

    ;    screen          rb      SIZE_X * SIZE_Y * 3   ; screen buffer
;align 16
     ;   Z_buffer        rb      SIZE_X * SIZE_Y * 4
	I_Param 	rb	256
	memStack	rb	4000 ;memory area for stack
align  16
screen: