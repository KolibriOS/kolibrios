
align 4
proc initSharedState uses eax ebx, context:dword
	mov ebx,[context]

	stdcall gl_zalloc, 4*MAX_DISPLAY_LISTS
	mov [ebx+offs_cont_shared_state],eax ;...lists=gl_zalloc(...)
	stdcall gl_zalloc, 4*TEXTURE_HASH_TABLE_SIZE
	mov [ebx+offs_cont_shared_state+4],eax ;...texture_hash_table=gl_zalloc(...)

	stdcall alloc_texture, [context],0
	ret
endp

align 4
proc endSharedState uses eax ebx, context:dword
	mov ebx,[context]

;  for(i=0;i<MAX_DISPLAY_LISTS;i++) {
;    /* TODO */
;  }
	stdcall gl_free, dword[ebx+offs_cont_shared_state] ;lists
	stdcall gl_free, dword[ebx+offs_cont_shared_state+4] ;texture_hash_table
	ret
endp

align 4
proc glInit uses eax ebx ecx edx, zbuffer1:dword
	stdcall gl_zalloc,sizeof.GLContext
	mov dword[gl_ctx],eax
	mov edx,eax

	mov ecx,[zbuffer1]
	mov dword[edx+offs_cont_zb],ecx

	; allocate GLVertex array
	mov dword[edx+offs_cont_vertex_max],POLYGON_MAX_VERTEX
	stdcall gl_malloc, POLYGON_MAX_VERTEX*sizeof.GLVertex
	mov dword[edx+offs_cont_vertex],eax

	; viewport
	mov dword[edx+offs_cont_viewport+offs_vpor_xmin],0
	mov dword[edx+offs_cont_viewport+offs_vpor_ymin],0
	mov eax,[ecx+offs_zbuf_xsize]
	mov dword[edx+offs_cont_viewport+offs_vpor_xsize], eax
	mov eax,[ecx+offs_zbuf_ysize]
	mov dword[edx+offs_cont_viewport+offs_vpor_ysize], eax
	mov dword[edx+offs_cont_viewport+offs_vpor_updated],1

	; shared state
	stdcall initSharedState,edx

	; lists
	mov dword[edx+offs_cont_exec_flag],1
	mov dword[edx+offs_cont_compile_flag],0
	mov dword[edx+offs_cont_print_flag],0

	mov dword[edx+offs_cont_in_begin],0

	; lights
	mov ecx,MAX_LIGHTS
	mov eax,edx
	add eax,offs_cont_lights
	.cycle_0:
		gl_V4_New eax+offs_ligh_ambient, 0.0,0.0,0.0,1.0
		gl_V4_New eax+offs_ligh_diffuse, 1.0,1.0,1.0,1.0
		gl_V4_New eax+offs_ligh_specular, 1.0,1.0,1.0,1.0
		gl_V4_New eax+offs_ligh_position, 0.0,0.0,1.0,0.0
		gl_V3_New eax+offs_ligh_norm_position, 0.0,0.0,1.0
		gl_V3_New eax+offs_ligh_spot_direction, 0.0,0.0,-1.0
		gl_V3_New eax+offs_ligh_norm_spot_direction, 0.0,0.0,-1.0
		mov dword[eax+offs_ligh_spot_exponent],0.0
		mov dword[eax+offs_ligh_spot_cutoff],180.0
		mov dword[eax+offs_ligh_attenuation],1.0
		mov dword[eax+offs_ligh_attenuation+4],0.0
		mov dword[eax+offs_ligh_attenuation+8],0.0
		mov dword[eax+offs_ligh_enabled],0
		add eax,sizeof.GLLight
		dec ecx
		cmp ecx,0
	jg .cycle_0

	mov dword[edx+offs_cont_first_light],0 ;NULL
	gl_V4_New edx+offs_cont_ambient_light_model, 0.2,0.2,0.2,1.0
	mov dword[edx+offs_cont_local_light_model],0
	mov dword[edx+offs_cont_lighting_enabled],0
	mov dword[edx+offs_cont_light_model_two_side],0

	; default materials
	mov ecx,2 ;for(i=0;i<2;i++)
	mov eax,edx
	add eax,offs_cont_materials
	@@:
		gl_V4_New eax+offs_mate_emission, 0.0,0.0,0.0,1.0
		gl_V4_New eax+offs_mate_ambient, 0.2,0.2,0.2,1.0
		gl_V4_New eax+offs_mate_diffuse, 0.8,0.8,0.8,1.0
		gl_V4_New eax+offs_mate_specular, 0.0,0.0,0.0,1.0
		mov dword[eax+offs_mate_shininess], 0.0
		add eax,sizeof.GLMaterial
	loop @b

	mov dword[edx+offs_cont_current_color_material_mode],GL_FRONT_AND_BACK
	mov dword[edx+offs_cont_current_color_material_type],GL_AMBIENT_AND_DIFFUSE
	mov dword[edx+offs_cont_color_material_enabled],0

	; textures
	stdcall glInitTextures,edx

	; default state
	mov dword[edx+offs_cont_current_color],1.0
	mov dword[edx+offs_cont_current_color+4],1.0
	mov dword[edx+offs_cont_current_color+8],1.0
	mov dword[edx+offs_cont_current_color+12],1.0
	mov dword[edx+offs_cont_longcurrent_color],65535
	mov dword[edx+offs_cont_longcurrent_color+4],65535
	mov dword[edx+offs_cont_longcurrent_color+8],65535

	mov dword[edx+offs_cont_current_normal],1.0
	mov dword[edx+offs_cont_current_normal+4],0.0
	mov dword[edx+offs_cont_current_normal+8],0.0
	mov dword[edx+offs_cont_current_normal+12],0.0

	mov dword[edx+offs_cont_current_edge_flag],1
  
	mov dword[edx+offs_cont_current_tex_coord],0.0
	mov dword[edx+offs_cont_current_tex_coord+4],0.0
	mov dword[edx+offs_cont_current_tex_coord+8],0.0
	mov dword[edx+offs_cont_current_tex_coord+12],1.0

	mov dword[edx+offs_cont_polygon_mode_front],GL_FILL
	mov dword[edx+offs_cont_polygon_mode_back],GL_FILL

	mov dword[edx+offs_cont_current_front_face],0 ;0 = GL_CCW  1 = GL_CW
	mov dword[edx+offs_cont_current_cull_face],GL_BACK
	mov dword[edx+offs_cont_current_shade_model],GL_SMOOTH
	mov dword[edx+offs_cont_cull_face_enabled],0
  
	; clear
	mov dword[edx+offs_cont_clear_color],0.0
	mov dword[edx+offs_cont_clear_color+4],0.0
	mov dword[edx+offs_cont_clear_color+8],0.0
	mov dword[edx+offs_cont_clear_color+12],0.0
	mov dword[edx+offs_cont_clear_depth],0

	; selection
	mov dword[edx+offs_cont_render_mode],GL_RENDER
	mov dword[edx+offs_cont_select_buffer],0 ;NULL
	mov dword[edx+offs_cont_name_stack_size],0

	; matrix
	mov dword[edx+offs_cont_matrix_mode],0

	mov dword[edx+offs_cont_matrix_stack_depth_max],MAX_MODELVIEW_STACK_DEPTH
	mov dword[edx+offs_cont_matrix_stack_depth_max+4],MAX_PROJECTION_STACK_DEPTH
	mov dword[edx+offs_cont_matrix_stack_depth_max+8],MAX_TEXTURE_STACK_DEPTH

	mov ecx,3 ;for(i=0;i<3;i++)
	mov ebx,edx
	@@:
		mov edi,[ebx+offs_cont_matrix_stack_depth_max]
		shl edi,6 ;вместо imul edi,sizeof(M4)
		stdcall gl_zalloc,edi
		mov [ebx+offs_cont_matrix_stack],eax
		mov [ebx+offs_cont_matrix_stack_ptr],eax
		add ebx,4
	loop @b

	stdcall glMatrixMode,GL_PROJECTION
	call glLoadIdentity
	stdcall glMatrixMode,GL_TEXTURE
	call glLoadIdentity
	stdcall glMatrixMode,GL_MODELVIEW
	call glLoadIdentity

	mov dword[edx+offs_cont_matrix_model_projection_updated],1

	; opengl 1.1 arrays
	mov dword[edx+offs_cont_client_states],0
  
	; opengl 1.1 polygon offset
	mov dword[edx+offs_cont_offset_states],0
  
	; clear the resize callback function pointer
	mov dword[edx+offs_cont_gl_resize_viewport],0 ;NULL
  
	; specular buffer
	mov dword[edx+offs_cont_specbuf_first],0 ;NULL
	mov dword[edx+offs_cont_specbuf_used_counter],0
	mov dword[edx+offs_cont_specbuf_num_buffers],0

	; depth test
	mov dword[edx+offs_cont_depth_test],0

	ret
endp

align 4
proc glClose uses eax
	call gl_get_context
	stdcall endSharedState,eax
	stdcall gl_free,eax
	ret
endp
