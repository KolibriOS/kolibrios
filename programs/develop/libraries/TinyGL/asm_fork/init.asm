
align 4
proc initSharedState uses eax ebx, context:dword
	mov ebx,[context]

	stdcall gl_zalloc, 4*MAX_DISPLAY_LISTS
	mov [ebx+GLContext.shared_state],eax ;...lists=gl_zalloc(...)
	stdcall gl_zalloc, 4*TEXTURE_HASH_TABLE_SIZE
	mov [ebx+GLContext.shared_state+4],eax ;...texture_hash_table=gl_zalloc(...)

	stdcall alloc_texture, [context],0
	ret
endp

align 4
proc endSharedState uses eax ebx, context:dword
	mov ebx,[context]

;  for(i=0;i<MAX_DISPLAY_LISTS;i++) {
;    /* TODO */
;  }
	stdcall gl_free, dword[ebx+GLContext.shared_state] ;lists
	stdcall gl_free, dword[ebx+GLContext.shared_state+4] ;texture_hash_table
	ret
endp

align 4
proc glInit, zbuffer1:dword
pushad
	stdcall gl_zalloc,sizeof.GLContext
	mov dword[gl_ctx],eax
	mov edx,eax

	mov ecx,[zbuffer1]
	mov dword[edx+GLContext.zb],ecx

	; allocate GLVertex array
	mov dword[edx+GLContext.vertex_max],POLYGON_MAX_VERTEX
	stdcall gl_malloc, POLYGON_MAX_VERTEX*sizeof.GLVertex
	mov dword[edx+GLContext.vertex],eax

	; viewport
	xor eax,eax
	mov dword[edx+GLContext.viewport+GLViewport.xmin],eax
	mov dword[edx+GLContext.viewport+GLViewport.ymin],eax
	mov eax,[ecx+ZBuffer.xsize]
	mov dword[edx+GLContext.viewport+GLViewport.xsize],eax
	mov eax,[ecx+ZBuffer.ysize]
	mov dword[edx+GLContext.viewport+GLViewport.ysize],eax
	mov dword[edx+GLContext.viewport+GLViewport.updated],1

	; shared state
	stdcall initSharedState,edx

	; lists
	mov dword[edx+GLContext.exec_flag],1
	mov dword[edx+GLContext.compile_flag],0
	mov dword[edx+GLContext.print_flag],0

	mov dword[edx+GLContext.in_begin],0

	; lights
	mov ecx,MAX_LIGHTS
	mov eax,edx
	add eax,GLContext.lights
	.cycle_0:
		gl_V4_New eax+GLLight.ambient, 0.0,0.0,0.0,1.0
		gl_V4_New eax+GLLight.diffuse, 1.0,1.0,1.0,1.0
		gl_V4_New eax+GLLight.specular, 1.0,1.0,1.0,1.0
		gl_V4_New eax+GLLight.position, 0.0,0.0,1.0,0.0
		gl_V3_New eax+GLLight.norm_position, 0.0,0.0,1.0
		gl_V3_New eax+GLLight.spot_direction, 0.0,0.0,-1.0
		gl_V3_New eax+GLLight.norm_spot_direction, 0.0,0.0,-1.0
		mov dword[eax+GLLight.spot_exponent],0.0
		mov dword[eax+GLLight.spot_cutoff],180.0
		gl_V3_New eax+GLLight.attenuation, 1.0,0.0,0.0
		mov dword[eax+GLLight.enabled],0
		add eax,sizeof.GLLight
		dec ecx
		cmp ecx,0
	jg .cycle_0

	mov dword[edx+GLContext.first_light],0 ;NULL
	gl_V4_New edx+GLContext.ambient_light_model, 0.2,0.2,0.2,1.0
	mov dword[edx+GLContext.local_light_model],0
	mov dword[edx+GLContext.lighting_enabled],0
	mov dword[edx+GLContext.light_model_two_side],0

	; default materials
	mov ecx,2 ;for(i=0;i<2;i++)
	lea eax,[edx+GLContext.materials]
	@@:
		gl_V4_New eax+GLMaterial.emission, 0.0,0.0,0.0,1.0
		gl_V4_New eax+GLMaterial.ambient, 0.2,0.2,0.2,1.0
		gl_V4_New eax+GLMaterial.diffuse, 0.8,0.8,0.8,1.0
		gl_V4_New eax+GLMaterial.specular, 0.0,0.0,0.0,1.0
		mov dword[eax+GLMaterial.shininess], 0.0
		add eax,sizeof.GLMaterial
	loop @b

	mov dword[edx+GLContext.current_color_material_mode],GL_FRONT_AND_BACK
	mov dword[edx+GLContext.current_color_material_type],GL_AMBIENT_AND_DIFFUSE
	mov dword[edx+GLContext.color_material_enabled],0

	; textures
	stdcall glInitTextures,edx

	; default state
	gl_V4_New edx+GLContext.current_color, 1.0,1.0,1.0,1.0
	gl_V3_New edx+GLContext.longcurrent_color, 65535,65535,65535

	gl_V4_New edx+GLContext.current_normal, 1.0,0.0,0.0,0.0

	mov dword[edx+GLContext.current_edge_flag],1
  
	gl_V4_New edx+GLContext.current_tex_coord, 0.0,0.0,0.0,1.0

	mov dword[edx+GLContext.polygon_mode_front],GL_FILL
	mov dword[edx+GLContext.polygon_mode_back],GL_FILL

	mov dword[edx+GLContext.current_front_face],0 ;0 = GL_CCW  1 = GL_CW
	mov dword[edx+GLContext.current_cull_face],GL_BACK
	mov dword[edx+GLContext.current_shade_model],GL_SMOOTH
	mov dword[edx+GLContext.cull_face_enabled],0
  
	; clear
	gl_V4_New edx+GLContext.clear_color, 0.0,0.0,0.0,0.0
	mov dword[edx+GLContext.clear_depth],0

	; selection
	mov dword[edx+GLContext.render_mode],GL_RENDER
	mov dword[edx+GLContext.select_buffer],0 ;NULL
	mov dword[edx+GLContext.name_stack_size],0

	; matrix
	mov dword[edx+GLContext.matrix_mode],0

	gl_V3_New edx+GLContext.matrix_stack_depth_max,\
		MAX_MODELVIEW_STACK_DEPTH,MAX_PROJECTION_STACK_DEPTH,MAX_TEXTURE_STACK_DEPTH

	mov ecx,3 ;for(i=0;i<3;i++)
	mov ebx,edx
	@@:
		mov edi,[ebx+GLContext.matrix_stack_depth_max]
		shl edi,6 ;вместо imul edi,sizeof(M4)
		stdcall gl_zalloc,edi
		mov [ebx+GLContext.matrix_stack],eax
		mov [ebx+GLContext.matrix_stack_ptr],eax
		add ebx,4
	loop @b

	stdcall glMatrixMode,GL_PROJECTION
	call glLoadIdentity
	stdcall glMatrixMode,GL_TEXTURE
	call glLoadIdentity
	stdcall glMatrixMode,GL_MODELVIEW
	call glLoadIdentity

	mov dword[edx+GLContext.matrix_model_projection_updated],1

	; opengl 1.1 arrays
	mov dword[edx+GLContext.client_states],0
  
	; opengl 1.1 polygon offset
	mov dword[edx+GLContext.offset_states],0
  
	; clear the resize callback function pointer
	mov dword[edx+GLContext.gl_resize_viewport],0 ;NULL
  
	; specular buffer
	mov dword[edx+GLContext.specbuf_first],0 ;NULL
	mov dword[edx+GLContext.specbuf_used_counter],0
	mov dword[edx+GLContext.specbuf_num_buffers],0

	; depth test
	mov dword[edx+GLContext.depth_test],0
popad
	ret
endp

align 4
proc glClose uses eax
	call gl_get_context
	stdcall endSharedState,eax
	stdcall gl_free,eax
	ret
endp
