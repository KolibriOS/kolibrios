struc CTRL_INFO
{   .pci_cmd        dd  ?
    .irq            dd  ?
    .glob_cntrl     dd  ?
    .glob_sta       dd  ?
    .codec_io_base  dd  ?
    .ctrl_io_base   dd  ?
    .codec_mem_base dd  ?
    .ctrl_mem_base  dd  ?
    .codec_id       dd  ?
}
CTRL_INFO_SIZE		equ 9*4

SRV_GETVERSION		equ 0
SND_CREATE_BUFF		equ 1
SND_DESTROY_BUFF	equ 2
SND_SETFORMAT		equ 3
SND_GETFORMAT		equ 4
SND_RESET		equ 5
SND_SETPOS		equ 6
SND_GETPOS		equ 7
SND_SETBUFF		equ 8
SND_OUT			equ 9
SND_PLAY		equ 10
SND_STOP		equ 11
SND_SETVOLUME		equ 12
SND_GETVOLUME		equ 13
SND_SETPAN		equ 14
SND_GETPAN		equ 15
SND_GETBUFFSIZE		equ 16
SND_GETFREESPACE	equ 17
SND_SETTIMEBASE		equ 18
SND_GETTIMESTAMP	equ 19

DEV_SET_BUFF		equ  4
DEV_NOTIFY		equ  5
DEV_SET_MASTERVOL	equ  6
DEV_GET_MASTERVOL	equ  7
DEV_GET_INFO		equ  8

SOUND_VERSION	equ 0x0101
PCM_ALL		equ 0

PCM_OUT		equ 0x08000000
PCM_RING	equ 0x10000000
PCM_STATIC	equ 0x20000000
PCM_FLOAT	equ 0x40000000
PCM_FILTER	equ 0x80000000

PCM_2_16_48	equ 1
PCM_1_16_48	equ 2
PCM_2_16_44	equ 3
PCM_1_16_44	equ 4
PCM_2_16_32	equ 5
PCM_1_16_32	equ 6
PCM_2_16_24	equ 7
PCM_1_16_24	equ 8
PCM_2_16_22	equ 9
PCM_1_16_22	equ 10
PCM_2_16_16	equ 11
PCM_1_16_16	equ 12
PCM_2_16_12	equ 13
PCM_1_16_12	equ 14
PCM_2_16_11	equ 15
PCM_1_16_11	equ 16
PCM_2_16_8	equ 17
PCM_1_16_8	equ 18
PCM_2_8_48	equ 19
PCM_1_8_48	equ 20
PCM_2_8_44	equ 21
PCM_1_8_44	equ 22
PCM_2_8_32	equ 23
PCM_1_8_32	equ 24
PCM_2_8_24	equ 25
PCM_1_8_24	equ 26
PCM_2_8_22	equ 27
PCM_1_8_22	equ 28
PCM_2_8_16	equ 29
PCM_1_8_16	equ 30
PCM_2_8_12	equ 31
PCM_1_8_12	equ 32
PCM_2_8_11	equ 33
PCM_1_8_11	equ 34
PCM_2_8_8	equ 35
PCM_1_8_8	equ 36

; i_data
szInfinity	db 'INFINITY',0
szSound		db 'SOUND',0
; u_data
hSound		dd ?
hrdwSound	dd ? 
;------------------------------------------------------------------------------
align 4
_InitSound@4:       ;p_ver:dword
	
	push	ebx ecx
	mcall	68,16,szInfinity
	mov	[hSound],eax
	test	eax,eax
	jz	.fail
	
	mcall	68,16,szSound
	mov	[hrdwSound],eax
	test	eax,eax
	jz	.fail
	
	lea	eax,[esp+12]   ;p_ver
	xor	ebx,ebx
	
	push	4              ;.out_size
	push	eax            ;.output
	push	ebx            ;.inp_size
	push	ebx            ;.input
	push	SRV_GETVERSION ;.code
	push	[hSound]       ;.handle
	
	mov	ecx,esp        ;[handle]
	mcall	68,17
	add	esp,24
	jmp	.exit
;--------------------------------------
align 4
.fail:
	or	eax, -1
;--------------------------------------
align 4
.exit:
	pop	ecx ebx
	ret	4
;------------------------------------------------------------------------------
align 4
_CreateBuffer@12:   ;format:dword,size:dword,p_str:dword
	push	ebx ecx
	lea	eax,[esp+20]   ;p_str
	lea	ebx,[esp+12]   ;format

	push	4              ;.out_size
	push	eax            ;.output
	push	8              ;.inp_size
	push	ebx            ;.input
	push	SND_CREATE_BUFF;.code
	push	[hSound]       ;.handle

	mov	ecx,esp
	mcall	68,17
	add	esp,24       ;io_cintrol 
	pop	ecx ebx
	ret	12
;------------------------------------------------------------------------------
;align 4
;_DestroyBuffer@4:     ;str:dword
;	push	ebx ecx
;	xor	eax,eax
;	lea	ebx,[esp+12]   ;[stream]
;	
;	push	eax            ;.out_size
;	push	eax            ;.output
;	push	4              ;.inp_size
;	push	ebx            ;.input
;	push	SND_DESTROY_BUFF;.code
;	push	[hSound]       ;.handle
;	
;	mov	ecx, esp        ;[handle]
;	mcall	68,17
;	add	esp,24
;	pop	ecx ebx
;	ret	4
;------------------------------------------------------------------------------
;align 4
;_WaveOut@12:        ;str:dword, src:dword, size:dword
;	push	ebx ecx
;	xor	eax,eax
;	lea	ebx,[esp+12]   ;[stream]
;	
;	push	eax            ;.out_size
;	push	eax            ;.output
;	push	12             ;.inp_size
;	push	ebx            ;.input
;	push	SND_OUT        ;.code
;	push	dword [hSound] ;.handle
;	
;	mov	ecx,esp
;	mcall	68,17
;	add	esp,24
;	pop	ecx ebx
;	ret	12
;------------------------------------------------------------------------------
align 4
_GetBufferSize@8:      ;str:dword, p_size:dword
	push	ebx ecx
	lea	eax,[esp+16]
	lea	ebx,[esp+12]   ;[stream]
	
	push	4              ;.out_size
	push	eax            ;.output
	push	4              ;.inp_size
	push	ebx            ;.input
	push	SND_GETBUFFSIZE;.code
	push	dword [hSound] ;.handle
	
	mov	ecx,esp
	mcall	68,17
	add	esp,24
	pop	ecx ebx
	ret	8
;------------------------------------------------------------------------------           
;align 4
;_GetBufferFree@8:      ;str:dword, p_free:dword
;	push	ebx ecx
;	lea	ebx,[esp+12]      ;[stream]
;	push	0                 ;storage for free_space    
;	mov	eax,esp
;	
;	push	4                 ;.out_size
;	push	eax               ;.output
;	push	4                 ;.inp_size
;	push	ebx               ;.input
;	push	SND_GETFREESPACE  ;.code
;	push	dword [hSound]    ;.handle
;	
;	mov	ecx,esp
;	mcall	68,17
;	add	esp, 24
;	mov	ecx,[esp+20]      ; p_free  
;	pop	dword [ecx]
;	pop	ecx ebx
;	ret	8
;------------------------------------------------------------------------------
align 4
_SetBuffer@16:   ;str:dword, src:dword, offs:dword, size:dword
	push	ebx ecx
	xor	eax,eax
	lea	ebx,[esp+12]   ;[stream]

	push	eax            ;.out_size
	push	eax            ;.output
	push	16             ;.inp_size
	push	ebx            ;.input
	push	SND_SETBUFF    ;.code
	push	dword [hSound] ;.handle

	mov	ecx,esp
	mcall	68,17
	add	esp,24
	pop	ecx ebx
	ret	16
;------------------------------------------------------------------------------
align 4
_PlayBuffer@8:          ;str:dword,flags:dword
	push	ebx ecx
	xor	eax,eax
	lea	ebx,[esp+12]   ;[stream]
	
	push	eax            ;.out_size
	push	eax            ;.output
	push	8              ;.inp_size
	push	ebx            ;.input
	push	SND_PLAY       ;.code
	push	dword [hSound] ;.handle

	mov	ecx, esp
	mcall	68,17
	add	esp, 24
	pop	ecx ebx
	ret	8
;------------------------------------------------------------------------------
align 4
_StopBuffer@4:          ;str:dword
	push	ebx ecx
	xor	eax,eax
	lea	ebx,[esp+12]   ;[stream]
	
	push	eax            ;.out_size
	push	eax            ;.output
	push	4              ;.inp_size
	push	ebx            ;.input
	push	SND_STOP       ;.code
	push	dword [hSound] ;.handle
	
	mov	ecx,esp
	mcall	68,17
	add	esp,24
	pop	ecx ebx
	ret	4
;------------------------------------------------------------------------------
;align 4
;_GetBufferPos@8:     ;str:dword, p_pos:dword
;	push	ebx ecx
;	lea	eax,[esp+16]
;	lea	ebx,[esp+12]   ;[stream]
;	
;	push	4              ;.out_size
;	push	eax            ;.output
;	push	4              ;.inp_size
;	push	ebx            ;.input
;	push	SND_GETPOS     ;.code
;	push	dword [hSound] ;.handle
;	
;	mov	ecx,esp
;	mcall	68,17
;	add	esp,24
;	pop	ecx ebx
;	ret	8
;------------------------------------------------------------------------------
;align 4
;_SetBufferPos@8:        ;str:dword, offs:dword
;	push	ebx ecx
;	xor	eax,eax
;	lea	ebx,[esp+12]   ;[stream]
;	
;	push	eax            ;.out_size
;	push	eax            ;.output
;	push	8              ;.inp_size
;	push	ebx            ;.input
;	push	SND_SETPOS     ;.code
;	push	dword [hSound] ;.handle
;	
;	mov	ecx,esp
;	mcall	68,17
;	add	esp,24
;	pop	ecx ebx
;	ret	8
;------------------------------------------------------------------------------
align 4
proc _test_wav@4 stdcall, hdr:dword
	mov	eax,[hdr]
	cmp	dword [eax],0x46464952
	jne	.fail
	
	cmp	dword [eax+8],0x45564157
	jne	.fail
	
	cmp	word [eax+20],1
	jne	.fail
	
	mov	ecx,dword [eax+24]
	mov	edx,22050
	cmp	ecx,edx
	ja	.high
	je	.l_22
	
	cmp	ecx,8000
	je	.l_8
	
	cmp	ecx,11025
	je	.l_11
	
	cmp	ecx,12000
	je	.l_12
	
	cmp	ecx,16000
	je	.l_16
;--------------------------------------
align 4
.fail:
	xor	eax,eax
	ret
;--------------------------------------
align 4
.high:
	cmp	ecx,24000
	je	.LN56

	cmp	ecx,32000
	je	.LN65

	cmp	ecx,44100
	je	.LN74

	cmp	ecx,48000
	jne	.fail
	
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN79

	dec	ecx
	jne	.LN74
	
	mov	edx,19
	jmp	.done
;--------------------------------------
align 4
.LN79:
	mov	edx,20
	jmp	.done
;--------------------------------------
align 4
.LN74:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN70

	dec	ecx
	jne	.LN65
	
	mov	edx,21
	jmp	.done
;--------------------------------------
align 4
.LN70:
	mov	edx,22
	jmp	.done
;--------------------------------------
align 4
.LN65:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN61

	dec	ecx
	jne	.LN56
	
	mov	edx,23
	jmp	.done
;--------------------------------------
align 4
.LN61:
	mov	edx,24
	jmp	.done
;--------------------------------------
align 4
.LN56:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN52

	dec	ecx
	je	.LN50
;--------------------------------------
align 4
.l_22:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN43

	dec	ecx
	je	.LN41
;--------------------------------------
align 4
.l_16:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN34

	dec	ecx
	je	.LN32
;--------------------------------------
align 4
.l_12:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN25

	dec	ecx
	je	.LN23
;--------------------------------------
align 4
.l_11:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN16

	dec	ecx
	je	.LN14
;--------------------------------------
align 4
.l_8:
	movzx	ecx,word [eax+22]
	dec	ecx
	je	.LN7

	dec	ecx
	jne	.fail
	
	mov	edx,35
	jmp	.done
;--------------------------------------
align 4
.LN7:
	mov	edx,36
	jmp	.done
;--------------------------------------
align 4
.LN14:
	mov	edx,33
	jmp	.done
;--------------------------------------
align 4
.LN16:
	mov	edx,34
	jmp	.done
;--------------------------------------
align 4
.LN23:
	mov	edx,31
	jmp	.done
;--------------------------------------
align 4
.LN25:
	mov	edx,32
	jmp	.done
;--------------------------------------
align 4
.LN32:
	mov	edx,29
	jmp	.done
;--------------------------------------
align 4
.LN34:
	mov	edx,30
	jmp	.done
;--------------------------------------
align 4
.LN41:
	mov	edx,27
	jmp	.done
;--------------------------------------
align 4
.LN43:
	mov	edx,28
	jmp	.done
;--------------------------------------
align 4
.LN50:
	mov	edx,25
	jmp	.done
;--------------------------------------
align 4
.LN52:
	mov	edx,26
;--------------------------------------
align 4
.done:
	xor	ecx,ecx
	cmp	word [eax+34],16
	setne	cl
	dec	ecx
	and	ecx,-18
	add	ecx,edx
	mov	eax,ecx
	ret
endp
;------------------------------------------------------------------------------