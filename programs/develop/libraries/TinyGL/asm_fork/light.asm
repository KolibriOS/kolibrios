align 4
sp128f dd 128.0

align 4
proc glopMaterial uses eax ebx ecx edi esi, context:dword, p:dword
; edi -> GLMaterial *m
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4] ;ecx = p[1]

	cmp ecx,GL_FRONT_AND_BACK ;if (mode == GL_FRONT_AND_BACK)
	jne @f
		mov dword[ebx+4],GL_FRONT ;p[1].i=GL_FRONT
		mov edi,ebp
		add edi,12
		stdcall glopMaterial,eax,edi
		mov ecx,GL_BACK
	@@:
	mov edi,eax
	add edi,GLContext.materials
	cmp ecx,GL_FRONT ;if (mode == GL_FRONT) m=&context.materials[0]
	je @f
		add edi,sizeof.GLMaterial ;else m=&context.materials[1]
	@@:

	mov ecx,4
	mov esi,ebx ;esi = &p
	add esi,12 ;esi = &p[3]
	mov ebx,[ebx+8] ;ebx = p[2]
	cmp ebx,GL_EMISSION
	jne @f
		;add edi,offs_mate_emission ;offs_mate_emission=0
		rep movsd
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_AMBIENT
	jne @f
		add edi,offs_mate_ambient
		rep movsd
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_DIFFUSE
	jne @f
		add edi,offs_mate_diffuse
		rep movsd
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_SPECULAR
	jne @f
		add edi,offs_mate_specular
		rep movsd
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_SHININESS
	jne @f
		fld dword[esi]
		add edi,offs_mate_shininess
		movsd
		mov dword[edi],SPECULAR_BUFFER_RESOLUTION
		fdiv dword[sp128f]
		fimul dword[edi]
		fistp dword[edi] ;m.shininess_i = (v[0]/128.0f)*SPECULAR_BUFFER_RESOLUTION
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_AMBIENT_AND_DIFFUSE
	jne @f
		add edi,offs_mate_ambient
		rep movsd
		sub esi,16
		;edi = &offs_mate_diffuse
		mov ecx,4
		rep movsd
		jmp .end_f
align 4
	@@: ;default
;    assert(0);
	.end_f:
	ret
endp

align 4
proc glopColorMaterial uses eax ebx ecx, context:dword, p:dword
	mov eax,[context]
	mov ebx,[p]
	mov ecx,[ebx+4] ;ecx = p[1]
	mov dword[eax+GLContext.current_color_material_mode],ecx
	mov ecx,[ebx+8] ;ecx = p[2]
	mov dword[eax+GLContext.current_color_material_type],ecx
	ret
endp

align 4
proc glopLight context:dword, p:dword
locals
	pos V4
endl
pushad
	mov eax,[context]
	mov ebx,[p]
	mov edx,[ebx+4] ;edx = p[1]

;  assert(edx >= GL_LIGHT0 && edx < GL_LIGHT0+MAX_LIGHTS );

	sub edx,GL_LIGHT0
	imul edx,sizeof.GLLight
	add edx,eax
	add edx,GLContext.lights

	mov ecx,[ebx+8] ;ecx = p[2]
	cmp ecx,GL_AMBIENT
	jne @f
		mov esi,ebx
		add esi,12
		mov edi,edx
		;add edi,offs_ligh_ambient ;offs_ligh_ambient = 0
		mov ecx,4
		rep movsd ;l.ambient=v
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_DIFFUSE
	jne @f
		mov esi,ebx
		add esi,12
		mov edi,edx
		add edi,offs_ligh_diffuse
		mov ecx,4
		rep movsd ;l.diffuse=v
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_SPECULAR
	jne @f
		mov esi,ebx
		add esi,12
		mov edi,edx
		add edi,offs_ligh_specular
		mov ecx,4
		rep movsd ;l.specular=v
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_POSITION
	jne @f
		mov edi,ebx ;ebx = [ebp+12] = [p] = &p[0]
		add edi,12 ;&p[3]
		mov esi,ebp
		sub esi,16 ;&pos
		stdcall gl_M4_MulV4, esi,dword[eax+GLContext.matrix_stack_ptr],edi
		mov edi,edx
		add edi,offs_ligh_position
		mov ecx,4
		rep movsd ;l.position=pos

		fld dword[edi-4] ;if(l.position.v[3] == 0)
		ftst
		fstsw ax
		sahf
		jne .end_i
			;mov esi,ebp
			sub esi,16 ;&pos
			mov edi,edx
			add edi,offs_ligh_norm_position
			mov ecx,3
			rep movsd ;l.norm_position=pos[1,2,3]

			;mov edi,edx
			;add edi,offs_ligh_norm_position
			sub edi,12
			stdcall gl_V3_Norm,edi ;&l.norm_position
		.end_i:
		ffree st0
		fincstp
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_SPOT_DIRECTION
	jne @f
		mov esi,ebx ;&p[0]
		add esi,12
		mov edi,edx
		add edi,offs_ligh_spot_direction
		mov ecx,3
		rep movsd ;l.spot_direction=v[0,1,2]
		;mov esi,ebx
		;add esi,12
		sub esi,12
		;mov edi,edx
		;add edi,offs_ligh_norm_spot_direction
		add edi,offs_ligh_norm_spot_direction-(offs_ligh_spot_direction+12)
		mov ecx,3
		rep movsd ;l.norm_spot_direction=v[0,1,2]
		add edx,offs_ligh_norm_spot_direction
		stdcall gl_V3_Norm,edx
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_SPOT_EXPONENT
	jne @f
		mov ecx,[ebx+12]
		mov [edi+offs_ligh_spot_exponent],ecx ;l.spot_exponent=p[3]
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_SPOT_CUTOFF
	jne .end_spot_c
		fld dword[ebp+12] ;float a=v.v[0]
;      assert(a == 180 || (a>=0 && a<=90));
		fst dword[edi+offs_ligh_spot_cutoff] ;l.spot_cutoff=a
		fcom dword[an180f] ;if (a != 180)
		fstsw ax
		sahf
		je @f
			fldpi
			fmulp
			fdiv dword[an180f]
			fcos
			fstp dword[edi+offs_ligh_cos_spot_cutoff] ;l.cos_spot_cutoff=cos(a * M_PI / 180.0)
			jmp .end_f
		@@:
		ffree st0
		fincstp
		jmp .end_f
align 4
	.end_spot_c:
	cmp ecx,GL_CONSTANT_ATTENUATION
	jne @f
		mov ecx,[ebx+12]
		mov [edi+offs_ligh_attenuation],ecx ;l->attenuation[0]=p[3]
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_LINEAR_ATTENUATION
	jne @f
		mov ecx,[ebx+12]
		mov [edi+offs_ligh_attenuation+4],ecx ;l->attenuation[1]=p[3]
		jmp .end_f
align 4
	@@:
	cmp ecx,GL_QUADRATIC_ATTENUATION
	jne @f
		mov ecx,[ebx+12]
		mov [edi+offs_ligh_attenuation+8],ecx ;l->attenuation[2]=p[3]
		jmp .end_f
align 4
	@@: ;default:
;    assert(0);
	.end_f:
popad
	ret
endp

  
align 4
proc glopLightModel uses ebx ecx esi edi, context:dword, p:dword
	mov edi,[context]
	mov ebx,[p]
	mov ebx,[ebx+4]
	mov esi,[ebp+12] ;&p[0]
	add esi,8 ;&p[2]

	cmp ebx,GL_LIGHT_MODEL_AMBIENT
	jne @f
		mov ecx,4
		add edi,GLContext.ambient_light_model
		rep movsd ;for(i=0;i<4;i++) context.ambient_light_model.v[i]=v[i]
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_LIGHT_MODEL_LOCAL_VIEWER
	jne @f
		fld dword[esi] ;st0 = p[2]
		fistp dword[edi+GLContext.local_light_model]
		jmp .end_f
align 4
	@@:
	cmp ebx,GL_LIGHT_MODEL_TWO_SIDE
	jne @f
		fld dword[esi] ;st0 = p[2]
		fistp dword[edi+GLContext.light_model_two_side]
		jmp .end_f
align 4
	@@: ;default:
;    tgl_warning("glopLightModel: illegal pname: 0x%x\n", ebx);
;    //assert(0);
	.end_f:
	ret
endp

macro clampf a, min, max
{
local .o_1
local .o_2
local .end_m
	fld dword a ;if (a<=0.0)
	ftst
	fstsw ax
	sahf
	ja .o_1
		ffree st0
		fincstp
		mov eax,0.0
		jmp .end_m ;return 0.0
align 4
	.o_1:
	fld1 ;else if (a>=1.0)
	fcompp
	fstsw ax
	sahf
	ja .o_2
		mov eax,1.0
		jmp .end_m ;return 1.0
align 4
	.o_2:
	mov eax,dword a ;else return a
	.end_m:
}

align 4
proc gl_enable_disable_light uses eax ebx ecx, context:dword, light:dword, v:dword
	mov eax,[context]
	mov ebx,[light]
	imul ebx,sizeof.GLLight
	add ebx,eax
	add ebx,GLContext.lights

	xor ecx,ecx
	cmp dword[ebx+offs_ligh_enabled],0
	jne @f
		not ecx
	@@:
	and ecx,[v]
	or ecx,ecx
	jz @f
		;if (v && !l.enabled)
		mov dword[ebx+offs_ligh_enabled],1
		mov ecx,[eax+GLContext.first_light]
		mov [ebx+offs_ligh_next],ecx
		mov [eax+GLContext.first_light],ebx ;context.first_light = l
		mov dword[ebx+offs_ligh_prev],0 ;l.prev = NULL
		jmp .end_f
align 4
	@@:
	xor ecx,ecx
	cmp dword[v],0
	jne @f
		not ecx
	@@:
	and ecx,[ebx+offs_ligh_enabled]
	or ecx,ecx
	jz .end_f
		;else if (!v && l.enabled)
		mov dword[ebx+offs_ligh_enabled],0 ;l.enabled = 0
		mov ecx,[ebx+offs_ligh_next]
		cmp dword[ebx+offs_ligh_prev],0 ;if (l.prev == NULL)
		jne .els_0
			mov [eax+GLContext.first_light],ecx	;context.first_light = l.next
			jmp @f
align 4
		.els_0:
			mov eax,[ebx+offs_ligh_prev]
			mov [eax+offs_ligh_next],ecx ;l.prev.next = l.next
		@@:
		cmp dword[ebx+offs_ligh_next],0
		je .end_f
			mov ecx,[ebx+offs_ligh_prev]
			mov eax,[ebx+offs_ligh_next]
			mov [eax+offs_ligh_prev],ecx ;l.next.prev = l.prev
	.end_f:
	ret
endp

align 4
fl_1e_3 dd 1.0e-3

; non optimized lightening model
align 16
proc gl_shade_vertex, context:dword, v:dword
locals
	R dd ? ;float ebp-96
	G dd ? ;float ebp-92
	B dd ? ;float ebp-88
	A dd ? ;float ebp-84
	s V3 ;ebp-80
	d V3 ;ebp-68
	tmp dd ? ;float ebp-56
	att dd ? ;float ebp-52
	dot_spot dd ? ;float ebp-48
	lR dd ? ;float ebp-44
	lB dd ? ;float ebp-40
	lG dd ? ;float ebp-36
	twoside dd ? ;int ebp-32
	idx dd ? ;int ebp-28
	n V3 ;ebp-24
	vcoord V3 ;ebp-12
endl
pushad
; ebx -> GLLight *l
; ecx -> GLMaterial *m
; esi -> GLVertex *v

;n = v.normal
;d = l.position
;   diffuse light
;dot = d*n
;dot_spot = d*l.norm_spot_direction
;   specular light
;s = d-v.ec
;dot_spec = n*s

	mov esi,[v]
	mov edx,[context]
	mov ecx,edx
	add ecx,GLContext.materials ;ecx(m) = &context.materials[0]
	mov eax,[edx+GLContext.light_model_two_side]
	mov [twoside],eax

	add esi,offs_vert_normal
	mov edi,ebp
	sub edi,24 ;edi = &n
	movsd ;n.X=v.normal.X
	movsd ;n.Y=v.normal.Y
	movsd ;n.Z=v.normal.Z
	mov esi,[v]

	fld dword[edx+GLContext.ambient_light_model]
	fmul dword[ecx+offs_mate_ambient]
	fadd dword[ecx] ;offs_mate_emission=0
	fstp dword[R] ;R=m.emission.v[0]+m.ambient.v[0]*context.ambient_light_model.v[0]
	fld dword[edx+GLContext.ambient_light_model+4]
	fmul dword[ecx+offs_mate_ambient+4]
	fadd dword[ecx+offs_mate_emission+4]
	fstp dword[G]
	fld dword[edx+GLContext.ambient_light_model+8]
	fmul dword[ecx+offs_mate_ambient+8]
	fadd dword[ecx+offs_mate_emission+8]
	fstp dword[B]
	clampf [ecx+offs_mate_diffuse+12],0,1
	mov [A],eax ;A=clampf(m.diffuse.v[3],0,1)

	mov ebx,[edx+GLContext.first_light]
	.cycle_0: ;for(l=context.first_light;l!=NULL;l=l.next)
		or ebx,ebx
		jz .cycle_0_end

		; ambient
		fld dword[ecx+offs_mate_ambient]
		fmul dword[ebx] ;offs_ligh_ambient=0
		fstp dword[lR] ;lR=l.ambient.v[0] * m.ambient.v[0]
		fld dword[ecx+offs_mate_ambient+4]
		fmul dword[ebx+offs_ligh_ambient+4]
		fstp dword[lG] ;lG=l.ambient.v[1] * m.ambient.v[1]
		fld dword[ecx+offs_mate_ambient+8]
		fmul dword[ebx+offs_ligh_ambient+8]
		fstp dword[lB] ;lB=l.ambient.v[2] * m.ambient.v[2]

		fld dword[ebx+offs_ligh_position+offs_W]
		ftst ;if (l.position.v[3] == 0)
		fstsw ax
		sahf
		jne .els_0
			; light at infinity
			ffree st0 ;l.position.v[3]
			fincstp
			mov eax,[ebx+offs_ligh_norm_position]
			mov [d],eax ;d.X=l.norm_position.v[0]
			mov eax,[ebx+offs_ligh_norm_position+offs_Y]
			mov [d+offs_Y],eax ;d.Y=l.norm_position.v[1]
			mov eax,[ebx+offs_ligh_norm_position+offs_Z]
			mov [d+offs_Z],eax ;d.Z=l.norm_position.v[2]
			mov dword[att],1.0
			jmp .els_0_end
align 4
		.els_0:
			; distance attenuation
			ffree st0 ;l.position.v[3]
			fincstp
			fld dword[ebx+offs_ligh_position]
			fsub dword[esi+offs_vert_ec]
			fstp dword[d] ;d.X=l.position.v[0]-v.ec.v[0]
			fld dword[ebx+offs_ligh_position+offs_Y]
			fsub dword[esi+offs_vert_ec+offs_Y]
			fstp dword[d+offs_Y] ;d.Y=l.position.v[1]-v.ec.v[1]
			fld dword[ebx+offs_ligh_position+offs_Z]
			fsub dword[esi+offs_vert_ec+offs_Z]
			fstp dword[d+offs_Z] ;d.Z=l.position.v[2]-v.ec.v[2]
			fld dword[d]
			fmul st0,st0
			fld dword[d+offs_Y]
			fmul st0,st0
			faddp
			fld dword[d+offs_Z]
			fmul st0,st0
			faddp
			fsqrt ;dist=sqrt(d.X^2+d.Y^2+d.Z^2)
			fcom dword[fl_1e_3]
			fstsw ax
			sahf
			jbe @f ;if (dist>1.0e-3)
				fld1
				fdiv st0,st1
				fld dword[d]
				fmul st0,st1
				fstp dword[d]
				fld dword[d+offs_Y]
				fmul st0,st1
				fstp dword[d+offs_Y]
				fld dword[d+offs_Z]
				fmul st0,st1
				fstp dword[d+offs_Z]
				ffree st0 ;1.0/dist
				fincstp
			@@:
			fld dword[ebx+offs_ligh_attenuation+8]
			fmul st0,st1 ;st0 = dist * l.attenuation[2]
			fadd dword[ebx+offs_ligh_attenuation+4]
			fmul st0,st1
			fadd dword[ebx+offs_ligh_attenuation]
			fld1
			fdiv st0,st1
			fstp dword[att] ;att = 1.0f/(l.attenuation[0]+dist*(l.attenuation[1]+dist*l.attenuation[2]))
			ffree st0 ;1.0
			fincstp
			ffree st0 ;dist
			fincstp
		.els_0_end:
		fld dword[d]
		fmul dword[n]
		fld dword[d+offs_Y]
		fmul dword[n+offs_Y]
		faddp
		fld dword[d+offs_Z]
		fmul dword[n+offs_Z]
		faddp ;dot = d.X*n.X+d.Y*n.Y+d.Z*n.Z
		cmp dword[twoside],0 ;if (twoside && dot < 0)
		je @f
		ftst ;if (dot<0)
		fstsw ax
		sahf
		jae @f
			fchs ;dot = -dot
		@@:
		ftst ;if (dot>0)
		fstsw ax
		sahf
		jbe .if0_end
			; diffuse light
			fld dword[ecx+offs_mate_diffuse]
			fmul dword[ebx+offs_ligh_diffuse]
			fmul st0,st1
			fadd dword[lR]
			fstp dword[lR] ;lR+=dot * l.diffuse.v[0] * m.diffuse.v[0]
			fld dword[ecx+offs_mate_diffuse+4]
			fmul dword[ebx+offs_ligh_diffuse+4]
			fmul st0,st1
			fadd dword[lG]
			fstp dword[lG] ;lG+=dot * l.diffuse.v[1] * m.diffuse.v[1]
			fld dword[ecx+offs_mate_diffuse+8]
			fmul dword[ebx+offs_ligh_diffuse+8]
			fmul st0,st1
			fadd dword[lB]
			fstp dword[lB] ;lB+=dot * l.diffuse.v[2] * m.diffuse.v[2]
			ffree st0 ;dot
			fincstp

			; spot light
			fld dword[ebx+offs_ligh_spot_cutoff]
			fcomp dword[an180f] ;if (l.spot_cutoff != 180)
			fstsw ax
			sahf
			je .if1_end
				fld dword[ebx+offs_ligh_norm_spot_direction]
				fmul dword[d]
				fld dword[ebx+offs_ligh_norm_spot_direction+offs_Y]
				fmul dword[d+offs_Y]
				faddp
				fld dword[ebx+offs_ligh_norm_spot_direction+offs_Z]
				fmul dword[d+offs_Z]
				faddp
				fchs
				fst dword[dot_spot]
				cmp dword[twoside],0 ;if (twoside && dot_spot < 0)
				je @f
				ftst ;if (dot_spot<0)
				fstsw ax
				sahf
				jae @f
					fchs ;dot_spot = -dot_spot
				@@:
				fcom dword[ebx+offs_ligh_cos_spot_cutoff] ;if (dot_spot < l.cos_spot_cutoff)
				fstsw ax
				sahf
				jae .els_1
					; no contribution
					ffree st0 ;dot_spot
					fincstp
					mov ebx,[ebx+offs_ligh_next]
					jmp .cycle_0 ;continue
align 4
				.els_1:
					; TODO: optimize
					fld dword[ebx+offs_ligh_spot_exponent]
					ftst ;if (l.spot_exponent > 0)
					fstsw ax
					sahf
					jbe @f
						fxch st1 ;dot_spot <--> l.spot_exponent
						;Вычисляем x^y
						;fld y
						;fld x
						fyl2x ;Стек FPU теперь содержит: st0=z=y*log2(x):
						;Теперь считаем 2**z:
						fld st0 ;Создаем еще одну копию z
						frndint ;Округляем
						fsubr st0,st1  ;st1=z, st0=z-trunc(z)
						f2xm1  ;st1=z, st0=2**(z-trunc(z))-1
						fld1
						faddp  ;st1=z, st0=2**(z-trunc(z))
						fscale ;st1=z, st0=(2**trunc(z))*(2**(z-trunc(z)))=2**t
						fxch st1
						fstp st ;Результат остается на вершине стека st0
						fmul dword[att]
						fstp dword[att] ;att=att*pow(dot_spot,l.spot_exponent)
						jmp .if1_end_f1
align 4
					@@:
					ffree st0 ;l.spot_exponent
					fincstp
					.if1_end_f1:
					ffree st0 ;dot_spot
					fincstp
			.if1_end:

			; specular light
			cmp dword[edx+GLContext.local_light_model],0 ;if (c.local_light_model)
			je .els_2
				mov eax,[esi+offs_vert_ec]
				mov [vcoord],eax ;vcoord.X=v.ec.X
				mov eax,[esi+offs_vert_ec+offs_Y]
				mov [vcoord+offs_Y],eax ;vcoord.Y=v.ec.Y
				mov eax,[esi+offs_vert_ec+offs_Z]
				mov [vcoord+offs_Z],eax ;vcoord.Z=v.ec.Z
				mov eax,ebp
				sub eax,12 ;eax = &vcoord
				stdcall gl_V3_Norm, eax
				fld dword[d]
				fsub dword[vcoord]
				fstp dword[s] ;s.X=d.X-vcoord.X
				fld dword[d+offs_Y]
				fsub dword[vcoord+offs_Y]
				fstp dword[s+offs_Y] ;s.Y=d.Y-vcoord.Y
				fld dword[d+offs_Z]
				fsub dword[vcoord+offs_Z]
				fstp dword[s+offs_Z] ;s.Z=d.Z-vcoord.Z
				jmp .els_2_end
align 4
			.els_2:
				mov eax,[d]
				mov [s],eax ;s.X=d.X
				mov eax,[d+offs_Y]
				mov [s+offs_Y],eax ;s.Y=d.Y
				fld1
				fadd dword[d+offs_Z]
				fstp dword[s+offs_Z] ;s.Z=d.Z+1.0
			.els_2_end:
			fld dword[n]
			fmul dword[s]
			fld dword[n+offs_Y]
			fmul dword[s+offs_Y]
			faddp
			fld dword[n+offs_Z]
			fmul dword[s+offs_Z]
			faddp ;dot_spec = n.X*s.X +n.Y*s.Y +n.Z*s.Z
			cmp dword[twoside],0 ;if (twoside && dot_spec < 0)
			je @f
			ftst ;if (dot_spec < 0)
			fstsw ax
			sahf
			jae @f
				fchs ;dot_spec = -dot_spec
			@@:
			ftst ;if (dot_spec > 0)
			fstsw ax
			sahf
			jbe .if0_end
				fld dword[s]
				fmul st0,st0
				fld dword[s+offs_Y]
				fmul st0,st0
				faddp
				fld dword[s+offs_Z]
				fmul st0,st0
				faddp ;st0 = s.X^2 +s.Y^2 +s.Z^2
				fsqrt
				fcom dword[fl_1e_3]
				fstsw ax
				sahf
				jbe @f ;if (tmp > 1.0e-3)
					fdiv st1,st0 ;dot_spec /= tmp
				@@:
				ffree st0 ;tmp
				fincstp

				; TODO: optimize
				; testing specular buffer code
				; dot_spec= pow(dot_spec,m.shininess)
				stdcall specbuf_get_buffer, edx, dword[ecx+offs_mate_shininess_i], dword[ecx+offs_mate_shininess]
				mov edi,eax ;edi = specbuf
				mov dword[idx],SPECULAR_BUFFER_SIZE ;idx = SPECULAR_BUFFER_SIZE

				;idx = (int)(dot_spec*SPECULAR_BUFFER_SIZE)
				fimul dword[idx]
				fistp dword[idx]				
				;if (idx > SPECULAR_BUFFER_SIZE) idx = SPECULAR_BUFFER_SIZE
				cmp dword[idx],SPECULAR_BUFFER_SIZE
				jle @f
					mov dword[idx],SPECULAR_BUFFER_SIZE
				@@:
				shl dword[idx],2
				add edi,dword[idx]
				fld dword[edi+offs_spec_buf] ;dot_spec = specbuf.buf[idx]
				fld dword[ebx+offs_ligh_specular]
				fmul st0,st1
				fmul dword[ecx+offs_mate_specular]
				fadd dword[lR]
				fstp dword[lR] ;lR+=dot_spec * l.specular.v[0] * m.specular.v[0]
				fld dword[ebx+offs_ligh_specular+4]
				fmul st0,st1
				fmul dword[ecx+offs_mate_specular+4]
				fadd dword[lG]
				fstp dword[lG] ;lG+=dot_spec * l.specular.v[1] * m.specular.v[1]
				fld dword[ebx+offs_ligh_specular+8]
				fmul st0,st1
				fmul dword[ecx+offs_mate_specular+8]
				fadd dword[lB]
				fstp dword[lB] ;lB+=dot_spec * l.specular.v[2] * m.specular.v[2]
		.if0_end:
		ffree st0 ;dot [or] dot_spec
		fincstp
		.if2_end:

		fld dword[att]
		fld dword[lR]
		fmul st0,st1
		fadd dword[R]
		fstp dword[R] ;R += att * lR
		fld dword[lG]
		fmul st0,st1
		fadd dword[G]
		fstp dword[G] ;G += att * lG
		fld dword[lB]
		fmul st0,st1
		fadd dword[B]
		fstp dword[B] ;B += att * lB
		ffree st0 ;att
		fincstp
		mov ebx,[ebx+offs_ligh_next]
		jmp .cycle_0
align 4
	.cycle_0_end:

	clampf [R],0,1
	mov [esi+offs_vert_color],eax ;v.color.v[0]=clampf(R,0,1)
	clampf [G],0,1
	mov [esi+offs_vert_color+4],eax ;v.color.v[1]=clampf(G,0,1)
	clampf [B],0,1
	mov [esi+offs_vert_color+8],eax ;v.color.v[2]=clampf(B,0,1)
	mov eax,[A]
	mov [esi+offs_vert_color+12],eax ;v.color.v[3]=A
popad
	ret
endp

