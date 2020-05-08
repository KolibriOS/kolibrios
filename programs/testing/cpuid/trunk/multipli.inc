newrating:

cmp [freqbb], 151
jl f266

cmp [freqbb], 150
ja ft

jmp vyxod
ft:

cmp [freqbb], 180
ja f400

cmp [freqbb], 181
jl f333
jmp vyxod
f266:

;формулы вычисления XP-рейтинга Athlon-ов (Tbred, Barton):
;FSB266/256kB:   сделано
;XP+ (Freq<2000MHz) = 1000 + (Freq-1000)*1,5 = Freq*1.5-500
;XP+ (Freq>=2000MHz) = 900 + (Freq-1000)*1,5 = Freq*1.5-600

   mov [FRS], 266

   cmp [total], 2000
   jl pal

    ; Рейтинг для Паломино 2000+
    mov eax,[total]
    imul eax, 3
    shr eax,1
    sub eax, 600
    mov [rating], eax

jmp vyxod

pal:

    ; Рейтинг для Паломино 2000-
    mov eax,[total]
    imul eax, 3
    shr eax,1
    sub eax, 500
    mov [rating], eax

jmp vyxod


f333:

mov [FRS], 333


 cmp [L2], 256
 jl .Th ; Thorton

;FSB333/512kB: сделано
;XP+ (Freq<2100MHz) = Freq*1.2 + 300
;XP+ (Freq>=2100MHz) = Freq*1.2 + 400

cmp [total], 2100
jl .r2100
   mov eax,[total]
   xor edx, edx
   imul eax, 600000
   mov	ebx, 500000
   div ebx
   mov	[rating], eax
   add [rating], 400

; Rating for Barton F<2000 MHz
jmp vyxod

.r2100:
    mov eax,[total]
    xor edx, edx
    imul eax, 600000
    mov  ebx, 500000
    div ebx
    mov  [rating], eax
    add [rating], 300


 ; Rating for Barton F>2000 MHz
jmp vyxod

;FSB333/256kB: сделано
;XP+ = Freq*1.2 + 100

.Th:

    mov eax,[total]
    xor edx, edx
    imul eax, 600000
    mov  ebx, 500000
    div ebx
    mov  [rating], eax
    add [rating], 100


;Rating for Thorton


jmp vyxod

f400:
mov [FRS], 400

;FSB400/512kB:
;XP+ = Freq*1.5 - 100

  mov eax,[total]
  imul eax, 3
  shr eax,1
  sub eax, 100
  mov [rating], eax

vyxod:


ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

multipl:
;--- вычисление коэффициента умножения -
; портит регистры
; out : CL = коэфф.умножения * 10, или 0

  xor  ecx, ecx

  cmp dword[smallvendor], 'cAMD'  ; Check for Advanced Micro Devices CPU
  jne  noAMD
  cmp  byte [f], 5
  jne  noAMDK6
  mov  eax, 68	  ; AMD-K6 (p.30)
  mov  ebx, 3
  mov  edx, 0x0C0000087
  mcall    ; eax - low dword
  and  eax, 111b
  mov  cl, [athloncoef3 + eax]
  cmp  eax, 6
  jne  @f
  cmp  byte[m], 8
  jae  @f
  mov  cl, 20
@@:  ret

noAMDK6:cmp	byte [f], 6
	jne	noAMDK7
	cmp	byte [m], 5
	jna	@f
	mov	eax, 0x80000007
	cpuid
	and	edx, 6		;  voltage ID control & frequency ID control
	cmp	edx, 6
	je	AMDK7M
@@:	mov	eax, 68 	; Athlon/AthlonXP
	mov	ebx, 3
	mov	edx, 0x0C0010015
	mcall
	mov	ebx, eax
	shr	ebx, 24
	and	ebx, 0x0F
	shr	eax, 20
	jnc	@f
	add	bl, 16
@@:	mov	cl, [athloncoef + ebx]
	ret

AMDK7M: mov	eax, 68 	; AthonXP-M
	mov	ebx, 3
	mov	edx, 0xC0010042
	mcall
	and	eax, 0x1F
	mov	cl, [athlonmcoef + eax]
	ret

noAMDK7:cmp	byte [f], 0xF
	jne	noAMDK8
	mov	eax, 0x80000007
	cpuid
	and	edx, 6		;  voltage ID control & frequency ID control
	cmp	edx, 6
	je	AMDK8M
	mov	eax, 68 	; Athon64
	mov	ebx, 3
	mov	edx, 0xC0010015
	mcall
	shr	eax, 24
	and	al, 0x3F
	shr	al, 1
	add	al, 4
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret

AMDK8M: mov	eax, 68 	; Athon64-M
	mov	ebx, 3
	mov	edx, 0xC0010042
	mcall
	and	al, 0x3F
	shr	al, 1
	add	al, 4
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret
;/////////////////////////////////////////////////////////	
noAMD:	cmp	dword [smallvendor], 'ntel'	; Check for International Electronics CPU
	jne	noIntel
	cmp	byte[f], 0x0F
	jne	noIntelP4
	cmp	byte [t], 6
	jne	@f
	mov	eax, 68 	; Pentium M
	mov	ebx, 3
	mov	edx, 0x2A
	mcall
	shr	eax, 22
	and	eax, 0x1F
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret
@@:	cmp	byte [m], 2
	jae	@f
	mov	eax, 68 	; Pentium 4 / Xeon (model < 2) Willamete
	mov	ebx, 3
	mov	edx, 0x2A
	mcall
	shr	eax, 8
	and	eax, 0x0F
	mov	cl, [p4coef + eax]
	ret
@@:	mov	eax, 68 	; Pentium 4 / Xeon (model >= 2) NorthWood
	mov	ebx, 3
	mov	edx, 0x2C
	mcall
	shr	eax, 24
	and	eax, 0x1F
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret	
noIntelP4:
	cmp	byte[f], 6
	jne	noIntelP6
	;edited code for test task-1
	cmp	byte[em],0x3A	;i3,i5,i7
	je	newCPU;
	cmp	byte[em],0x2A	;celeron dekstop, mobile, Xeon E3
	je	newCPU;
	cmp	byte[em],0x2D	;i7
	je	newCPU;
	cmp	byte[em],0x25	;celeron, pentium, core i3,i5,i7
	je	newCPU;
	cmp	byte[em],0x2C	;Xeon 3000
	je	newCPU;
	cmp	byte[em],0x1E	;Clarksfield core i7
	je	newCPU;
	cmp	byte[em],0x1A	;Bloomfield-2008
	je	newCPU;
	cmp	byte[em],0x2E	;NEhlem ~2008
	je	newCPU;


	mov	eax, 68 	; Pentium Pro / Pentium II / Pentium III
	mov	ebx, 3
	mov	edx, 0x2A
	mcall
	shr	eax, 22
	test	al, 0x20
	jz	@f
	or	al, 0x10
@@:	and	eax, 0x1f

	cmp	byte[m], 0x06 ; ? 7
	ja	@f
	and	al, 0x0f
@@:
	mov	cl, [coppercoeff + eax]
	cmp	byte[m], 0x0B
	jb	@f
	mov	cl, [tualatcoeff + eax]
	
	cmp	byte[m], 0x0B
	je	@f
	mov	dl, 10		; model 0x0C - 0x0F - Dothan / Yonah  / Conroe / Merom
	mul	dl
	mov	cl, al
	
@@:	ret

noIntel:
noIntelP6:
noAMDK8:
newCPU:
mov [nomultiplier], 1
  ret