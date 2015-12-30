; Some simple mathematical functions. Don't look for some logic in
; the function names :-)

; ******* Gestion des matrices 4x4 ******

if DEBUG
f_m4m db 'gl_M4_Mul',0
f_m4ml db 'gl_M4_MulLeft',0
end if

align 4
proc gl_M4_Id uses eax ecx edi, a:dword
	mov edi,[a]
	add edi,4
	mov ecx,14
	mov eax,0.0
	rep stosd
	mov eax,1.0
	stosd
	mov edi,[a]
	stosd
	add edi,16
	stosd
	add edi,16
	stosd
	ret
endp

align 4
proc gl_M4_IsId uses ebx ecx, a:dword
	mov eax,[a]
	xor ebx,ebx
	xor ecx,ecx
	.cycle_01:
		fld dword[eax]
		cmp ecx,ebx
		je .once
			ftst ;сравнение с 0.0
			fstsw ax
			sahf
			je @f
			jmp .not_1 ;если диагональные числа не равны 0.0 матрица не единичная
		.once:
			fld1
			fcomp st1 ;сравнение с 1.0
			fstsw ax
			test ah,0x40
			je .not_1 ;если не равно 1.0 матрица не единичная
		@@:
		ffree st0
		fincstp
		add eax,4
		inc ebx
		btr ebx,2
		jnc .cycle_01
	inc ecx
	bt ecx,2 ;проверяем равенство ecx==4
	jnc .cycle_01

	mov eax,1
	jmp @f
	.not_1:
		ffree st0
		fincstp
		xor eax,eax
	@@:
	ret
endp

align 4
proc gl_M4_Mul, c:dword,a:dword,b:dword
pushad
	mov edx,[c]
	xor eax,eax
	.cycle_0: ;i
		xor ebx,ebx
		.cycle_1: ;j
			fldz ;sum=0
			xor ecx,ecx
			M4_reg edi,[a],eax,0
			.cycle_2: ;k
				fld dword[edi]
				add edi,4
				M4_reg esi,[b],ecx,ebx
				fmul dword[esi]
				faddp ;sum += a[i][k] * b[k][j]
				inc ecx
				cmp ecx,4
				jl .cycle_2
			fstp dword[edx] ;c[i][j] = sum
			add edx,4
			inc ebx
			cmp ebx,4
			jl .cycle_1
		inc eax
		cmp eax,4
		jl .cycle_0
if DEBUG ;gl_M4_Mul
	stdcall dbg_print,f_m4m,txt_nl
	stdcall gl_print_matrix,[c],4
	stdcall dbg_print,txt_sp,txt_nl
end if
popad
	ret
endp

; c=c*a
align 4
proc gl_M4_MulLeft, c:dword,b:dword
locals
	i dd ?
	a M4
endl
pushad
	mov ecx,16
	mov esi,[c]
	mov edi,ebp
	sub edi,sizeof.M4
	rep movsd ;копирование матриц [a]=[c]

	mov edx,[c]
	mov dword[i],0
	mov eax,ebp
	sub eax,sizeof.M4
	.cycle_0: ;i
		xor ebx,ebx ;j=0
		.cycle_1: ;j
			fldz ;sum=0
			xor ecx,ecx ;k=0
			M4_reg edi,eax,dword[i],0
			.cycle_2: ;k
				fld dword[edi]
				add edi,4
				M4_reg esi,[b],ecx,ebx
				fmul dword[esi]
				faddp ;sum += a[i][k] * b[k][j]
				inc ecx
				cmp ecx,4
				jl .cycle_2
			fstp dword[edx] ;c[i][j] = sum
			add edx,4
			inc ebx
			cmp ebx,4
			jl .cycle_1
		inc dword[i]
		cmp dword[i],4
		jl .cycle_0
if DEBUG ;gl_M4_MulLeft
	stdcall dbg_print,f_m4ml,txt_nl
	stdcall gl_print_matrix,[c],4
	stdcall dbg_print,txt_sp,txt_nl
end if
popad
	ret
endp

align 4
proc gl_M4_Move uses ecx edi esi, a:dword,b:dword
	mov edi,[a]
	mov esi,[b]
	mov ecx,sizeof.M4/4
	rep movsd
	ret
endp

align 4
proc gl_MoveV3 uses edi esi, a:dword,b:dword
	mov edi,[a]
	mov esi,[b]
	movsd
	movsd
	movsd
	ret
endp

;void gl_MulM4V3(V3 *a,M4 *b,V3 *c)
;{
;        a->X=b->m[0][0]*c->X+b->m[0][1]*c->Y+b->m[0][2]*c->Z+b->m[0][3];
;        a->Y=b->m[1][0]*c->X+b->m[1][1]*c->Y+b->m[1][2]*c->Z+b->m[1][3];
;        a->Z=b->m[2][0]*c->X+b->m[2][1]*c->Y+b->m[2][2]*c->Z+b->m[2][3];
;}

;void gl_MulM3V3(V3 *a,M4 *b,V3 *c)
;{
;        a->X=b->m[0][0]*c->X+b->m[0][1]*c->Y+b->m[0][2]*c->Z;
;        a->Y=b->m[1][0]*c->X+b->m[1][1]*c->Y+b->m[1][2]*c->Z;
;        a->Z=b->m[2][0]*c->X+b->m[2][1]*c->Y+b->m[2][2]*c->Z;
;}

align 4
proc gl_M4_MulV4 uses ebx ecx edx, a:dword, b:dword, c:dword ;V4 *a, M4 *b, V4 *c
	mov ebx,[b]
	mov edx,[c]
	fld dword[edx]
	fld dword[edx+4]
	fld dword[edx+8]
	fld dword[edx+12]
	mov edx,[a]
	mov ecx,4
	.cycle_1:
		fld dword[ebx]    ;st0 = m[_][0]
		fmul st0,st4      ;st0 *= c.X
		fld dword[ebx+4]  ;st0 = m[_][1]
		fmul st0,st4      ;st0 *= c.Y
		faddp
		fld dword[ebx+8]  ;st0 = m[_][2]
		fmul st0,st3      ;st0 *= c.Z
		faddp
		fld dword[ebx+12] ;st0 += m[_][3]
		fmul st0,st2      ;st0 *= c.W
		faddp
		fstp dword[edx]   ;a.X = b.m[_][0]*c.X +b.m[_][1]*c.Y +b.m[_][2]*c.Z +b.m[_][3]*c.W
		add ebx,16 ;следущая строка матрицы
		add edx,4  ;следущая координата вектора
	loop .cycle_1
	ffree st0
	fincstp
	ffree st0
	fincstp
	ffree st0
	fincstp
	ffree st0
	fincstp
	ret
endp

; transposition of a 4x4 matrix
align 4
proc gl_M4_Transpose uses eax ecx edx, a:dword, b:dword
	mov eax,[a]
	mov ecx,[b]

	mov edx,[ecx]
	mov [eax],edx
	mov edx,[ecx+0x10]
	mov [eax+0x4],edx
	mov edx,[ecx+0x20]
	mov [eax+0x8],edx
	mov edx,[ecx+0x30]
	mov [eax+0x0c],edx

	mov edx,[ecx+0x4]
	mov [eax+0x10],edx
	mov edx,[ecx+0x14]
	mov [eax+0x14],edx
	mov edx,[ecx+0x24]
	mov [eax+0x18],edx
	mov edx,[ecx+0x34]
	mov [eax+0x1c],edx

	mov edx,[ecx+0x8]
	mov [eax+0x20],edx
	mov edx,[ecx+0x18]
	mov [eax+0x24],edx
	mov edx,[ecx+0x28]
	mov [eax+0x28],edx
	mov edx,[ecx+0x38]
	mov [eax+0x2c],edx

	mov edx,[ecx+0x0c]
	mov [eax+0x30],edx
	mov edx,[ecx+0x1c]
	mov [eax+0x34],edx
	mov edx,[ecx+0x2c]
	mov [eax+0x38],edx
	mov edx,[ecx+0x3c]
	mov [eax+0x3c],edx
	ret
endp

; inversion of an orthogonal matrix of type Y=M.X+P
;void gl_M4_InvOrtho(M4 *a,M4 b)
;{
;       int i,j;
;       float s;
;       for(i=0;i<3;i++)
;       for(j=0;j<3;j++) a->m[i][j]=b.m[j][i];
;       a->m[3][0]=0.0; a->m[3][1]=0.0; a->m[3][2]=0.0; a->m[3][3]=1.0;
;       for(i=0;i<3;i++) {
;               s=0;
;               for(j=0;j<3;j++) s-=b.m[j][i]*b.m[j][3];
;               a->m[i][3]=s;
;       }
;}

; Inversion of a general nxn matrix.
; Note : m is destroyed

align 4
proc Matrix_Inv uses ebx ecx edx edi esi, r:dword, m:dword, n:dword ;(float *r,float *m,int n)
locals
	max dd ? ;float
	tmp dd ?
endl

	; identitйe dans r
	mov eax,0.0
	mov ecx,[n]
	imul ecx,ecx
	mov edi,[r]
	rep stosd ;for(i=0;i<n*n;i++) r[i]=0
	mov eax,1.0
	xor ebx,ebx
	mov edi,[r]
	mov ecx,[n]
	shl ecx,2
	@@: ;for(i=0;i<n;i++)
		cmp ebx,[n]
		jge .end_0
		stosd ;r[i*n+i]=1
		add edi,ecx
		inc ebx
		jmp @b
	.end_0:

	; ebx -> n
	; ecx -> j
	; edx -> k
	; edi -> i
	; esi -> l
	mov ebx,[n]
	xor ecx,ecx
	.cycle_0: ;for(j=0;j<n;j++)
	cmp ecx,ebx
	jge .cycle_0_end
		; recherche du nombre de plus grand module sur la colonne j
		mov eax,ecx
		imul eax,ebx
		add eax,ecx
		shl eax,2
		add eax,[m]
		mov eax,[eax]
		mov [max],eax ;max=m[j*n+j]
		mov edx,ecx ;k=j
		mov edi,ecx
		inc edi
		.cycle_1: ;for(i=j+1;i<n;i++)
		cmp edi,ebx
		jge .cycle_1_end
			mov eax,edi
			imul eax,ebx
			add eax,ecx
			shl eax,2
			add eax,[m]
			fld dword[eax]
			fld st0
			fabs
			fld dword[max]
			fabs
			fcompp ;if (fabs(m[i*n+j])>fabs(max))
			fstsw ax
			sahf
			jae @f
				mov edx,edi ;k=i
				fst dword[max]
			@@:
			ffree st0 ;m[i*n+j]
			fincstp
		inc edi
		jmp .cycle_1
		.cycle_1_end:

		; non intersible matrix
		fld dword[max]
		ftst ;if (max==0)
		fstsw ax
		ffree st0
		fincstp
		sahf
		jne @f
			xor eax,eax
			inc eax
			jmp .end_f ;return 1
		@@:

		; permutation des lignes j et k
		cmp ecx,edx ;if (j!=k)
		je .cycle_2_end
			xor edi,edi
			.cycle_2: ;for(i=0;i<n;i++)
			cmp edi,ebx
			jge .cycle_2_end
				;тут пока esi != l
				mov eax,ecx
				imul eax,ebx
				add eax,edi
				shl eax,2
				add eax,[m]
				mov esi,[eax]
				mov [tmp],esi ;tmp=m[j*n+i]
				mov esi,edx
				imul esi,ebx
				add esi,edi
				shl esi,2
				add esi,[m]
				m2m dword[eax],dword[esi] ;m[j*n+i]=m[k*n+i]
				mov eax,[tmp]
				mov [esi],eax ;m[k*n+i]=tmp

				mov eax,ecx
				imul eax,ebx
				add eax,edi
				shl eax,2
				add eax,[r]
				mov esi,[eax]
				mov [tmp],esi ;tmp=r[j*n+i]
				mov esi,edx
				imul esi,ebx
				add esi,edi
				shl esi,2
				add esi,[r]
				m2m dword[eax],dword[esi] ;r[j*n+i]=r[k*n+i]
				mov eax,[tmp]
				mov [esi],eax ;r[k*n+i]=tmp
			inc edi
			jmp .cycle_2
		.cycle_2_end:

		; multiplication de la ligne j par 1/max
		fld1
		fdiv dword[max]
		fst dword[max] ;max=1/max
		xor edi,edi
		mov eax,ecx
		imul eax,ebx
		shl eax,2
		.cycle_3: ;for(i=0;i<n;i++)
		cmp edi,ebx
		jge .cycle_3_end
			add eax,[m]
			fld dword[eax]
			fmul st0,st1
			fstp dword[eax] ;m[j*n+i]*=max
			sub eax,[m]
			add eax,[r]
			fld dword[eax]
			fmul st0,st1
			fstp dword[eax] ;r[j*n+i]*=max
			sub eax,[r]
			add eax,4
		inc edi
		jmp .cycle_3
		.cycle_3_end:
		ffree st0 ;max
		fincstp

		xor esi,esi
		.cycle_4: ;for(l=0;l<n;l++)
		cmp esi,ebx
		jge .cycle_4_end
			cmp esi,ecx ;if (l!=j)
			je .if_end
			mov eax,esi
			imul eax,ebx
			add eax,ecx
			shl eax,2
			add eax,[m]
			fld dword[eax] ;t=m[l*n+j]
			xor edi,edi
			.cycle_5: ;for(i=0;i<n;i++)
			cmp edi,ebx
			jge .cycle_5_end
				mov eax,ecx
				imul eax,ebx
				add eax,edi
				shl eax,2
				add eax,[m]
				fld dword[eax]
				fmul st0,st1
				mov eax,esi
				imul eax,ebx
				add eax,edi
				shl eax,2
				add eax,[m]
				fsub dword[eax]
				fchs
				fstp dword[eax] ;m[l*n+i]-=m[j*n+i]*t
				mov eax,ecx
				imul eax,ebx
				add eax,edi
				shl eax,2
				add eax,[r]
				fld dword[eax]
				fmul st0,st1
				mov eax,esi
				imul eax,ebx
				add eax,edi
				shl eax,2
				add eax,[r]
				fsub dword[eax]
				fchs
				fstp dword[eax] ;r[l*n+i]-=r[j*n+i]*t
			inc edi
			jmp .cycle_5
			.cycle_5_end:
			ffree st0 ;t
			fincstp
			.if_end:
		inc esi
		jmp .cycle_4
		.cycle_4_end:
	inc ecx
	jmp .cycle_0
	.cycle_0_end:

	xor eax,eax ;return 0
	.end_f:
	ret
endp

; inversion of a 4x4 matrix

align 4
proc gl_M4_Inv uses eax ecx edi esi, a:dword, b:dword
locals
	tmp M4
endl
	mov esi,[b]
	mov edi,ebp
	sub edi,sizeof.M4 ;edi = &tmp
	mov ecx,16
	rep movsd
	sub edi,sizeof.M4 ;edi = &tmp
	stdcall Matrix_Inv,[a],edi,4 ;портит eax потому в uses есть eax
	ret
endp

align 4
proc gl_M4_Rotate uses eax ecx, a:dword,t:dword,u:dword
locals
	s dd ? ;float
	c dd ? ;float
	v dd ? ;int
	w dd ? ;int
endl
	mov eax,[u]
	inc eax
	cmp eax,2
	jle @f
		xor eax,eax
	@@:
	mov [v],eax
	inc eax
	cmp eax,2
	jle @f
		xor eax,eax
	@@:
	mov [w],eax
	fld dword [t]
	fsin
	fstp dword [s]
	fld dword [t]
	fcos
	fstp dword [c]

	stdcall gl_M4_Id,[a]

	M4_reg ecx,[a],[v],[v]
	mov eax,[c]
	mov [ecx],eax

	M4_reg ecx,[a],[v],[w]
	fld dword [s]
	fchs
	fstp dword [ecx]

	M4_reg ecx,[a],[w],[v]
	mov eax,[s]
	mov [ecx],eax

	M4_reg ecx,[a],[w],[w]
	mov eax,[c]
	mov [ecx],eax

	ret
endp

; inverse of a 3x3 matrix
;void gl_M3_Inv(M3 *a,M3 *m)
;{
;        float det;

;        det = m->m[0][0]*m->m[1][1]*m->m[2][2]-m->m[0][0]*m->m[1][2]*m->m[2][1]-
;                m->m[1][0]*m->m[0][1]*m->m[2][2]+m->m[1][0]*m->m[0][2]*m->m[2][1]+
;                m->m[2][0]*m->m[0][1]*m->m[1][2]-m->m[2][0]*m->m[0][2]*m->m[1][1];

;        a->m[0][0] = (m->m[1][1]*m->m[2][2]-m->m[1][2]*m->m[2][1])/det;
;        a->m[0][1] = -(m->m[0][1]*m->m[2][2]-m->m[0][2]*m->m[2][1])/det;
;        a->m[0][2] = -(-m->m[0][1]*m->m[1][2]+m->m[0][2]*m->m[1][1])/det;
 
;        a->m[1][0] = -(m->m[1][0]*m->m[2][2]-m->m[1][2]*m->m[2][0])/det;
;        a->m[1][1] = (m->m[0][0]*m->m[2][2]-m->m[0][2]*m->m[2][0])/det;
;        a->m[1][2] = -(m->m[0][0]*m->m[1][2]-m->m[0][2]*m->m[1][0])/det;

;        a->m[2][0] = (m->m[1][0]*m->m[2][1]-m->m[1][1]*m->m[2][0])/det;
;        a->m[2][1] = -(m->m[0][0]*m->m[2][1]-m->m[0][1]*m->m[2][0])/det;
;        a->m[2][2] = (m->m[0][0]*m->m[1][1]-m->m[0][1]*m->m[1][0])/det;
;}

; vector arithmetic

align 4
proc gl_V3_Norm uses ebx, a:dword
	mov ebx,[a]
	fld dword[ebx]
	fmul st0,st0
	fld dword[ebx+offs_Y]
	fmul st0,st0
	faddp
	fld dword[ebx+offs_Z]
	fmul st0,st0
	faddp
	fsqrt ;st0 = sqrt(a.X^2 +a.Y^2 +a.Z^2)
	ftst
	fstsw ax
	sahf
	je .r1 ;if (sqrt(...)==0) return 1
		fld dword[ebx] ;offs_X = 0
		fdiv st0,st1
		fstp dword[ebx] ;a.X/=sqrt(...)
		fld dword[ebx+offs_Y]
		fdiv st0,st1
		fstp dword[ebx+offs_Y] ;a.Y/=sqrt(...)
		fld dword[ebx+offs_Z]
		fdiv st0,st1
		fstp dword[ebx+offs_Z] ;a.Z/=sqrt(...)
		xor eax,eax
		jmp @f
	.r1:
		xor eax,eax
		inc eax
	@@:
	ffree st0
	fincstp
	ret
endp

macro gl_V3_New p_mem, x, y, z
{
	mov dword[p_mem],x
	mov dword[p_mem+4],y
	mov dword[p_mem+8],z
}

macro gl_V4_New p_mem, x, y, z, w
{
	mov dword[p_mem],x
	mov dword[p_mem+4],y
	mov dword[p_mem+8],z
	mov dword[p_mem+12],w
}
