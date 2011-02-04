; CORE.ASM
; --------
; uFMOD public source code release. Provided as-is.

if VIBRATO_OR_TREMOLO
sin127  db 00h,0Ch,19h,25h,31h,3Ch,47h,51h,5Ah,62h,6Ah,70h,75h,7Ah,7Dh,7Eh
        db 7Fh,7Eh,7Dh,7Ah,75h,70h,6Ah,62h,5Ah,51h,47h,3Ch,31h,25h,19h,0Ch
endif
if INSTRUMENTVIBRATO_ON
sin64   db 00h,02h,03h,05h,06h,08h,09h,0Bh,0Ch,0Eh,10h,11h,13h,14h,16h,17h
        db 18h,1Ah,1Bh,1Dh,1Eh,20h,21h,22h,24h,25h,26h,27h,29h,2Ah,2Bh,2Ch
        db 2Dh,2Eh,2Fh,30h,31h,32h,33h,34h,35h,36h,37h,38h,38h,39h,3Ah,3Bh
        db 3Bh,3Ch,3Ch,3Dh,3Dh,3Eh,3Eh,3Eh,3Fh,3Fh,3Fh,40h,40h,40h,40h,40h
endif
if AMIGAPERIODS_ON
f0_0833 dd 8.3333336e-2
f13_375 dd 1.3375e1
endif
f0_0013 dd 1.302083375e-3
f8363_0 dd 8.3630004275e3

; Mixer ramping
Ramp:
; [arg0] - ptr. to end of buffer
; ESI    - _mod+36
; EDI    - length
; EDX    - buffer
	; LOOP THROUGH CHANNELS
	mov ecx,[esi+FMUSIC_MODULE.Channels-36]
	push ebx
	push ebp
loop_ch:
	push esi
	mov esi,[ecx+FSOUND_CHANNEL.fsptr] ; load the correct SAMPLE pointer for this channel
	test esi,esi ; if(!fsptr) skip this channel!
	jz MixExit_1
	push edx ; mix buffer
	push edx ; cur. mix buffer pointer
	mov ebx,[ecx+FSOUND_CHANNEL.mixpos]
	; Set up a mix counter. See what will happen first, will the output buffer
	; end be reached first? or will the end of the sample be reached first? whatever
	; is smallest will be the mixcount.
	push edi
CalculateLoopCount:
	cmp BYTE PTR [ecx+FSOUND_CHANNEL.speeddir],0
	mov edx,[esi+FSOUND_SAMPLE.loopstart]
	mov eax,[ecx+FSOUND_CHANNEL.mixposlo]
	jne samplesleftbackwards
	; work out how many samples left from mixpos to loop end
	add edx,[esi+FSOUND_SAMPLE.looplen]
	sub edx,ebx
	ja submixpos
	mov edx,[esi+FSOUND_SAMPLE._length]
	sub edx,ebx
submixpos:
	; edx : samples left (loopstart+looplen-mixpos)
	neg edx
	neg eax
	adc edx,ebx
samplesleftbackwards:
	; work out how many samples left from mixpos to loop start
	neg edx
	add edx,ebx
	js MixExit
	; edx:eax now contains number of samples left to mix
	mov ebp,[ecx+FSOUND_CHANNEL.speedlo]
	shrd eax,edx,5
	mov ebx,[ecx+FSOUND_CHANNEL.speedhi]
	shr edx,5
	shrd ebp,ebx,5
	jnz speedok ; divide by 0 check
	mov ebp,FREQ_40HZ_p
	mov DWORD PTR [ecx+FSOUND_CHANNEL.speedlo],FREQ_40HZ_f
speedok:
	div ebp
	xor ebp,ebp
	neg edx
	adc eax,ebp
	jz DoOutputbuffEnd
	mov ebx,OFFSET uFMOD_fopen
	cmp eax,edi
	; set a flag to say mix will end when end of output buffer is reached
	seta [ebx-22]   ; mix_endflag
	jae staywithoutputbuffend
	xchg eax,edi
staywithoutputbuffend:
if RAMP_NONE
else
	movzx eax,WORD PTR [ecx+FSOUND_CHANNEL.ramp_count]
	; VOLUME RAMP SETUP
	; Reasons to ramp
	; 1 volume change
	; 2 sample starts (just treat as volume change - 0 to volume)
	; 3 sample ends (ramp last n number of samples from volume to 0)
	; now if the volume has changed, make end condition equal a volume ramp
	test eax,eax
endif
	mov edx,[ecx+FSOUND_CHANNEL.leftvolume]
	mov ebp,[ecx+FSOUND_CHANNEL.rightvolume]
if RAMP_NONE
	mov [ecx+FSOUND_CHANNEL.ramp_leftvolume],ebp
	mov [ecx+FSOUND_CHANNEL.ramp_rightvolume],edx
else
	mov [ebx-16],edi ; mmf+4 <- remember mix count before modifying it
	jz volumerampstart
	; if it tries to continue an old ramp, but the target has changed,
	; set up a new ramp
	cmp dx,[ecx+FSOUND_CHANNEL.ramp_lefttarget]
	jne volumerampstart
	cmp bp,[ecx+FSOUND_CHANNEL.ramp_righttarget]
	je volumerampclamp ; restore old ramp
volumerampstart:
	; SETUP NEW RAMP
	mov [ecx+FSOUND_CHANNEL.ramp_lefttarget],dx
	shl edx,volumeramps_pow
	sub edx,[ecx+FSOUND_CHANNEL.ramp_leftvolume]
	xor eax,eax
	sar edx,volumeramps_pow
	mov DWORD PTR [ecx+FSOUND_CHANNEL.ramp_leftspeed],edx
	jz novolumerampL
	mov al,volumerampsteps
novolumerampL:
	mov [ecx+FSOUND_CHANNEL.ramp_righttarget],bp
	shl ebp,volumeramps_pow
	sub ebp,[ecx+FSOUND_CHANNEL.ramp_rightvolume]
	sar ebp,volumeramps_pow
	mov DWORD PTR [ecx+FSOUND_CHANNEL.ramp_rightspeed],ebp
	jz novolumerampR
	mov al,volumerampsteps
novolumerampR:
	test eax,eax
	mov [ecx+FSOUND_CHANNEL.ramp_count],ax
	jz volumerampend
volumerampclamp:
	cmp edi,eax
	jbe volumerampend ; dont clamp mixcount
	mov edi,eax
volumerampend:
	mov eax,[ecx+FSOUND_CHANNEL.ramp_leftspeed]
	mov [ebx],eax    ; ramp_leftspeed
	mov eax,[ecx+FSOUND_CHANNEL.ramp_rightspeed]
	mov [ebx+4],eax  ; ramp_rightspeed
endif
	mov [ebx-20],edi ; mmf
	; SET UP ALL OF THE REGISTERS HERE FOR THE INNER LOOP
	; edx : speed
	; ebx : mixpos
	; ebp : speed low
	; esi : destination pointer
	; edi : counter
	mov ebx,[ecx+FSOUND_CHANNEL.mixpos]
	lea ebx,[ebx*2+esi+FSOUND_SAMPLE.buff]
	push esi
	cmp BYTE PTR [ecx+FSOUND_CHANNEL.speeddir],0
	mov esi,[esp+8] ; <- cur. mix buffer
	mov edx,[ecx+FSOUND_CHANNEL.speedhi]
	mov ebp,[ecx+FSOUND_CHANNEL.speedlo]
	je MixLoop16
	; neg edx:ebp
	neg ebp
	not edx
	sbb edx,-1
	align 4
MixLoop16:
	push edi
	push edx
	movsx edi,WORD PTR [ebx]
	movsx eax,WORD PTR [ebx+2]
	mov edx,[ecx+FSOUND_CHANNEL.mixposlo]
	sub eax,edi
	shr edx,1 ; force unsigned
	imul edx
	shl edi,volumeramps_pow-1
	shld edx,eax,volumeramps_pow
	add edx,edi
if RAMP_NONE
	mov eax,edx
	imul edx,[ecx+FSOUND_CHANNEL.ramp_rightvolume]
	add [esi],edx
	imul DWORD PTR [ecx+FSOUND_CHANNEL.ramp_leftvolume]
	add [esi+4],eax
else
	xchg eax,edx
	mov edi,eax
	imul DWORD PTR [ecx+FSOUND_CHANNEL.ramp_rightvolume]
	shrd eax,edx,volumeramps_pow-1
	rol edx,1
	and edx,1
	add eax,edx
	sar eax,1
	add [esi],eax
	xchg eax,edi
	imul DWORD PTR [ecx+FSOUND_CHANNEL.ramp_leftvolume]
	shrd eax,edx,volumeramps_pow-1
	rol edx,1
	and edx,1
	add eax,edx
	mov edi,[uFMOD_fread]
	sar eax,1
	add [esi+4],eax
	mov edx,[uFMOD_fopen]
	add [ecx+FSOUND_CHANNEL.ramp_rightvolume],edi ; + cur_ramp_rightspeed
	add [ecx+FSOUND_CHANNEL.ramp_leftvolume],edx  ; + cur_ramp_leftspeed
endif
	pop edx
	xor eax,eax
	add [ecx+FSOUND_CHANNEL.mixposlo],ebp
	pop edi
	adc eax,edx
	add esi,8
	dec edi
	lea ebx,[ebx+eax*2]
	jnz MixLoop16
	mov edi,[esp+20h] ; find out how many OUTPUT samples left to mix
	mov [esp+8],esi   ; update cur. mix buffer
	sub edi,esi
	shr edi,3         ; edi <- # of samples left
	pop esi           ; esi <- sample pointer
	lea eax,[esi+FSOUND_SAMPLE.buff]
	sub ebx,eax
	shr ebx,1
	mov [ecx+FSOUND_CHANNEL.mixpos],ebx
if RAMP_NONE
	xor edx,edx
else
	; DID A VOLUME RAMP JUST HAPPEN?
	movzx edx,WORD PTR [ecx+FSOUND_CHANNEL.ramp_count]
	test edx,edx
	jz DoOutputbuffEnd
	mov eax,[mmf]
	cdq
	sub [ecx+FSOUND_CHANNEL.ramp_count],ax
	; if(!rampcount) a ramp has FINISHED, so finish the rest of the mix
	jnz DoOutputbuffEnd
	sub eax,[mmf+4]
	; clear out the ramp speeds
	mov [ecx+FSOUND_CHANNEL.ramp_leftspeed],edx
	neg eax
	mov [ecx+FSOUND_CHANNEL.ramp_rightspeed],edx
	; is it 0 because ramp ended only? or both ended together?
	; if sample ended together with ramp... problems... loop isn't handled
	sbb edx,edx
	; start again and continue rest of mix
	test edi,edx
	jnz CalculateLoopCount ; dont start again if nothing left
	xor edx,edx
endif
DoOutputbuffEnd:
	cmp [mix_endflag],dl
	jne MixExit
	movzx eax,BYTE PTR [esi+FSOUND_SAMPLE.loopmode]
	; SWITCH ON LOOP MODE TYPE
	dec eax ; check for normal loop (FSOUND_LOOP_NORMAL = 1)
	jnz CheckBidiLoop
	mov eax,[esi+FSOUND_SAMPLE.loopstart]
	mov ebp,[esi+FSOUND_SAMPLE.looplen]
	add eax,ebp
	cmp ebx,eax
	jbe rewind_ok
	sub ebx,eax
	xchg eax,ebx
	div ebp
rewind_ok:
	sub ebp,edx
	sub ebx,ebp
	jmp ChkLoop_OK
CheckBidiLoop:
	dec eax ; FSOUND_LOOP_BIDI = 2
	neg eax
	adc edx,-1
	and [ecx+FSOUND_CHANNEL.mixposlo],edx
	and [ecx+FSOUND_CHANNEL.mixpos],edx
	and [ecx+FSOUND_CHANNEL.fsptr],edx
	jz MixExit
	cmp [ecx+FSOUND_CHANNEL.speeddir],al ; FSOUND_MIXDIR_FORWARDS
	je BidiForward
BidiBackwards:
	mov eax,[esi+FSOUND_SAMPLE.loopstart]
	neg ebp
	dec eax
	sub ebp,1
	dec BYTE PTR [ecx+FSOUND_CHANNEL.speeddir] ; set FSOUND_MIXDIR_FORWARDS
	sbb eax,ebx
	mov ebx,[esi+FSOUND_SAMPLE.loopstart]
	add ebx,eax
	cmp eax,[esi+FSOUND_SAMPLE.looplen]
	jl BidiFinish
BidiForward:
	mov eax,[esi+FSOUND_SAMPLE.loopstart]
	add eax,[esi+FSOUND_SAMPLE.looplen]
	lea edx,[eax-1]
	sbb eax,ebx
	neg ebp
	xchg eax,ebx
	sub ebp,1
	adc ebx,edx
	inc BYTE PTR [ecx+FSOUND_CHANNEL.speeddir] ; go backwards
	cmp ebx,[esi+FSOUND_SAMPLE.loopstart]
	jl BidiBackwards
BidiFinish:
	mov [ecx+FSOUND_CHANNEL.mixposlo],ebp
ChkLoop_OK:
	test edi,edi
	mov [ecx+FSOUND_CHANNEL.mixpos],ebx
	jnz CalculateLoopCount
MixExit:
	pop edi
	pop eax ; discard cur. mix buffer pointer
	pop edx
MixExit_1:
	add ecx,FSOUND_CHANNEL_size
	pop esi
	cmp ecx,[esi+FMUSIC_MODULE.uFMOD_Ch-36]
	jl loop_ch
	pop ebp
	pop ebx
	ret

if AMIGAPERIODS_ON
AmigaPeriod:
; [sptr] in ECX
; note   in EAX
; ESI != 0
	mov edx,132
	push edi
	sub edx,eax
	push esi
	test eax,eax
	push edx
	movsx eax,BYTE PTR [ecx+FSOUND_SAMPLE.finetune]
	mov edi,edx
	jz _do_inc
	cdq
	shl edx,1
_do_inc:
	inc edx
exp2:
	fild DWORD PTR [esp]
	fmul DWORD PTR [f0_0833] ; /12.0f
	fld st0
	frndint
	fsub st1,st0
	fxch st1
	f2xm1
	fld1
	faddp st1,st0
	fscale
	fstp st1
	fmul DWORD PTR [f13_375] ; *13.375f
	fistp DWORD PTR [esp]
	test esi,esi
	pop ecx
	jz exp2_end
	sub edi,edx
	push edi
	xor esi,esi
	mov edi,ecx
	jmp exp2
exp2_end:
	sub ecx,edi
	test edx,edx
	jns _do_imul
	neg ecx
_do_imul:
	imul ecx
	and edx,127 ; +2^7-1
	add eax,edx
	sar eax,7
	pop esi
	add eax,edi
	pop edi
	ret
endif ; AMIGAPERIODS_ON

; DESCRIPTION: To carry out a vibrato at a certain depth and speed
if VIBRATO_OR_VOLSLIDE
	if TREMOLO_ON
	VibratoOrTremolo:
	; cptr+2 = ESI
		movzx ecx,BYTE PTR [esi+FMUSIC_CHANNEL.vibpos-2]
		mov al,[esi+FMUSIC_CHANNEL.wavecontrol-2]
		mov edx,ecx
		add dl,[esi+FMUSIC_CHANNEL.vibspeed-2]
		and edx,3Fh
		and eax,3 ; switch(cptr->wavecontrol&3)
		mov [esi+FMUSIC_CHANNEL.vibpos-2],dl
		jz vibrato_c0
		; C2 : Sqare wave
		rol ecx,27
		sbb edx,edx
		xor edx,7Fh
		or edx,1
		dec eax
		jnz vibrato_default
		; C1 : Triangle wave (ramp down)
		shr edx,24
		ror ecx,23
		add edx,ecx
		neg edx
		jmp vibrato_default
	vibrato_c0:
		; C0 : Sine wave
		; delta = 127 sin(2 Pi x/64)
		mov eax,ecx
		and ecx,1Fh
		shr eax,6
		movzx edx,BYTE PTR [OFFSET sin127+ecx]
		sbb eax,eax
		xor edx,eax
		sub edx,eax
	vibrato_default:
		movsx eax,BYTE PTR [esi+FMUSIC_CHANNEL.vibdepth-2]
		imul edx  ; delta *= cptr->vibdepth
		sar eax,5
		ret
	endif

Vibrato:
; cptr+2 = ESI
	if TREMOLO_ON
		call VibratoOrTremolo
		sar eax,1
	else
		movzx ecx,BYTE PTR [esi+FMUSIC_CHANNEL.vibpos-2]
		mov al,[esi+FMUSIC_CHANNEL.wavecontrol-2]
		mov edx,ecx
		add dl,[esi+FMUSIC_CHANNEL.vibspeed-2]
		and edx,3Fh
		and eax,3 ; switch(cptr->wavecontrol&3)
		mov [esi+FMUSIC_CHANNEL.vibpos-2],dl
		jz vibrato_c0
		; C2 : Sqare wave
		rol ecx,27
		sbb edx,edx
		xor edx,7Fh
		or edx,1
		dec eax
		jnz vibrato_default
		; C1 : Triangle wave (ramp down)
		shr edx,24
		ror ecx,23
		add edx,ecx
		neg edx
		jmp vibrato_default
	vibrato_c0:
		; C0 : Sine wave
		; delta = 127 sin(2 Pi x/64)
		mov eax,ecx
		and ecx,1Fh
		shr eax,6
		movzx edx,BYTE PTR [OFFSET sin127+ecx]
		sbb eax,eax
		xor edx,eax
		sub edx,eax
	vibrato_default:
		movsx eax,BYTE PTR [esi+FMUSIC_CHANNEL.vibdepth-2]
		imul edx  ; delta *= cptr->vibdepth
		sar eax,6
	endif
	mov [esi+FMUSIC_CHANNEL.freqdelta-2],eax
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
	ret
endif ; VIBRATO_OR_VOLSLIDE

if TREMOLO_ON
Tremolo:
; cptr+2 = ESI
	if VIBRATO_OR_VOLSLIDE
		push esi
		add esi,FMUSIC_CHANNEL.tremolopos-FMUSIC_CHANNEL.vibpos
		call VibratoOrTremolo
		pop esi
	else
		movzx ecx,BYTE PTR [esi+FMUSIC_CHANNEL.tremolopos-2]
		mov al,[esi+FMUSIC_CHANNEL.wavecontrol-2]
		mov edx,ecx
		add dl,[esi+FMUSIC_CHANNEL.tremolospeed-2]
		and edx,3Fh
		and eax,3 ; switch(cptr->wavecontrol&3)
		mov [esi+FMUSIC_CHANNEL.tremolopos-2],dl
		jz tremolo_c0
		; C2 : Sqare wave
		rol ecx,27
		sbb edx,edx
		xor edx,7Fh
		or edx,1
		dec eax
		jnz tremolo_default
		; C1 : Triangle wave (ramp down)
		shr edx,24
		ror ecx,23
		add edx,ecx
		neg edx
		jmp tremolo_default
	tremolo_c0:
		; C0 : Sine wave
		; delta = 127 sin(2 Pi x/64)
		mov eax,ecx
		and ecx,1Fh
		shr eax,6
		movzx edx,BYTE PTR [OFFSET sin127+ecx]
		sbb eax,eax
		xor edx,eax
		sub edx,eax
	tremolo_default:
		movsx eax,BYTE PTR [esi+FMUSIC_CHANNEL.tremolodepth-2]
		imul edx
		sar eax,5
	endif
	mov [esi+FMUSIC_CHANNEL.voldelta-2],eax
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	ret
endif ; TREMOLO_ON

if PORTATO_OR_VOLSLIDE
Portamento:
; cptr+2 = ESI
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
	mov eax,[esi+FMUSIC_CHANNEL.freq-2]
	mov ecx,[esi+FMUSIC_CHANNEL.portatarget-2]
	movzx edx,BYTE PTR [esi+FMUSIC_CHANNEL.portaspeed-2]
	shl edx,2
	sub eax,ecx
	jg porta_do_sub
	add eax,edx
	jg _do_trim
	jmp _no_trim
porta_do_sub:
	sub eax,edx
	jl _do_trim
_no_trim:
	add ecx,eax
_do_trim:
	mov [esi+FMUSIC_CHANNEL.freq-2],ecx
	ret
endif ; PORTATO_OR_VOLSLIDE

if VOLUME_OR_PANENVELOPE
Envelope:
; cptr+2   = ESI
; env_iptr = ECX
; control  = AL
env_type     equ -4
envstopped   equ -8
envdelta     equ -12
env_value    equ -16
valfrac      equ -20
; env_tick     = -24
sustain_l2   equ -26
sustain_l1   equ -27
sustain_loop equ -28
env_next     equ -32
env_pos      equ -36
	push edi
	push ebx
	push ebp
	mov ebp,esp
	; Initialize local vars with PAN/VOL data
	lea edi,[ecx+FMUSIC_INSTRUMENT.PANPoints]
	xor ebx,ebx
if PANENVELOPE_ON
	mov edx,DWORD PTR [edi+FMUSIC_INSTRUMENT.PANsustain-FMUSIC_INSTRUMENT.PANPoints]
	mov [ebp+sustain_loop],edx ; load PANsustain, PANLoopStart and PANLoopEnd
	mov cl,BYTE PTR [edi+FMUSIC_INSTRUMENT.PANtype-FMUSIC_INSTRUMENT.PANPoints]
	movzx edx,BYTE PTR [edi+FMUSIC_INSTRUMENT.PANnumpoints-FMUSIC_INSTRUMENT.PANPoints]
endif
	or [esi+FMUSIC_CHANNEL.notectrl-2],al ; cptr->notectrl |= control
if PANENVELOPE_ON
	if VOLUMEENVELOPE_ON
		cmp al,FMUSIC_VOLUME ; is it FMUSIC_VOLUME or FMUSIC_PAN?
	endif
	lea eax,[esi+FMUSIC_CHANNEL.envpanstopped-2]
endif
if VOLUMEENVELOPE_ON
	if PANENVELOPE_ON
		jnz pan_or_vol_ok
	endif
	; control = FMUSIC_VOLUME
	add ebx,FMUSIC_CHANNEL.envvol-FMUSIC_CHANNEL.envpan
	mov eax,DWORD PTR [edi+FMUSIC_INSTRUMENT.VOLsustain-FMUSIC_INSTRUMENT.PANPoints]
	mov [ebp+sustain_loop],eax ; load VOLsustain, VOLLoopStart and VOLLoopEnd
	mov cl,BYTE PTR [edi+FMUSIC_INSTRUMENT.VOLtype-FMUSIC_INSTRUMENT.PANPoints]
	movzx edx,BYTE PTR [edi+FMUSIC_INSTRUMENT.VOLnumpoints-FMUSIC_INSTRUMENT.PANPoints]
	lea eax,[esi+FMUSIC_CHANNEL.envvolstopped-2]
	add edi,FMUSIC_INSTRUMENT.VOLPoints-FMUSIC_INSTRUMENT.PANPoints
pan_or_vol_ok:
endif
	cmp BYTE PTR [eax],dh
	jne goto_envelope_ret
	push ecx      ; -> env_type
	push eax      ; -> envstopped
	lea ecx,[esi+ebx+FMUSIC_CHANNEL.envpanpos-2]
	lea eax,[esi+ebx+FMUSIC_CHANNEL.envpandelta-2]
	push eax      ; -> envdelta
	lea eax,[esi+ebx+FMUSIC_CHANNEL.envpan-2]
	push eax      ; -> env_value
	lea eax,[esi+ebx+FMUSIC_CHANNEL.envpanfrac-2]
	lea ebx,[esi+ebx+FMUSIC_CHANNEL.envpantick-2]
	push eax      ; -> valfrac
	mov eax,[ecx]
	cmp eax,edx   ; if(*pos>=numpoints) envelop out of bound
	push ebx      ; -> env_tick
	jge envelope_done
	movzx eax,WORD PTR [edi+eax*4]
	cmp [ebx],eax ; if(*tick == points[(*pos)<<1]) we are at the correct tick for the position
	jnz add_envdelta
	test BYTE PTR [ebp+env_type],FMUSIC_ENVELOPE_LOOP
	jz loop_ok
	movzx eax,BYTE PTR [ebp+sustain_l2]
	cmp [ecx],eax
	jnz loop_ok   ; if((type&FMUSIC_ENVELOPE_LOOP) && *pos == loopend) handle loop
	movzx eax,BYTE PTR [ebp+sustain_l1]
	mov [ecx],eax ; *pos = loopstart
	movzx eax,WORD PTR [edi+eax*4]
	mov [ebx],eax ; *tick = points[(*pos)<<1]
loop_ok:
	mov eax,[ecx]
	mov [ebp+env_pos],eax
	lea eax,[edi+eax*4]
	dec edx
	movzx ebx,WORD PTR [eax] ; get tick at this point
	cmp [ecx],edx
	mov edx,[eax+4]
	mov edi,edx
	movzx eax,WORD PTR [eax+2]
	mov [ebp+env_next],edx ; get tick at next point
	mov edx,[ebp+env_value]
	mov [edx],eax ; *value = points[(currpos<<1)+1]
	jne env_continue
	; if it is at the last position, abort the envelope and continue last value
	mov eax,[ebp+envstopped]
	inc BYTE PTR [eax] ; *envstopped = TRUE
goto_envelope_ret:
	jmp Envelope_Ret
env_continue:
	shl eax,16
	sub edi,eax
	xchg eax,edi
	xor ax,ax
	; sustain
	test BYTE PTR [ebp+env_type],FMUSIC_ENVELOPE_SUSTAIN
	jz not_sustain
	movzx edx,BYTE PTR [ebp+sustain_loop]
	cmp [ebp+env_pos],edx
	jne not_sustain
	cmp BYTE PTR [esi+FMUSIC_CHANNEL.keyoff-2],al
	je Envelope_Ret
not_sustain:
	; interpolate 2 points to find delta step
	inc DWORD PTR [ecx] ; (*pos)++
	mov ecx,[ebp+valfrac]
	mov [ecx],edi ; *valfrac = curr
	mov edi,[ebp+envdelta]
	movzx ecx,WORD PTR [ebp+env_next]
	and DWORD PTR [edi],0 ; *envdelta = 0
	sub ecx,ebx
	jz envelope_done
	cdq
	idiv ecx
	mov [edi],eax ; *envdelta = (next-curr)/(nexttick-currtick)
	jmp envelope_done
add_envdelta:
	; interpolate
	mov eax,[ebp+envdelta]
	mov ecx,[eax]
	mov eax,[ebp+valfrac]
	add [eax],ecx ; *valfrac += *envdelta
envelope_done:
	pop edx ; <- env_tick
	pop eax ; <- valfrac
	pop ecx ; <- env_value
	mov eax,[eax]
	inc DWORD PTR [edx] ; (*tick)++
	sar eax,16
	mov [ecx],eax ; *value = *valfrac >> 16
Envelope_Ret:
	leave
	pop ebx
	pop edi
	ret
endif ; VOLUME_OR_PANENVELOPE

if VOLUMEBYTE_ON
VolByte:
; volume = EDX
; cptr+2 = ESI
	sub edx,16
	jb switch_volume
	cmp edx,40h
	ja switch_volume
	; if(volume >= 0x10 && volume <= 0x50)
	mov [esi+FMUSIC_CHANNEL.volume-2],edx
switch_volume:
	mov eax,edx
	and edx,0Fh
	shr eax,4 ; switch(volume>>4)
	sub eax,5
	jz case_6
	dec eax
	jz case_7
	dec eax
	jz case_6
	dec eax
	jz case_7
	sub eax,2
	jbe case_AB
	dec eax
	jz case_C
	dec eax
	jz case_D
	dec eax
	jz case_E
	dec eax
	jnz vol_default
	; case 0xF
	test edx,edx
	jz vol_z
	shl dl,4
	mov [esi+FMUSIC_CHANNEL.portaspeed-2],dl
vol_z:
	and BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],NOT_FMUSIC_TRIGGER
if PORTATO_OR_VOLSLIDE
	mov eax,[esi+FMUSIC_CHANNEL.period-2]
	mov [esi+FMUSIC_CHANNEL.portatarget-2],eax ; cptr->portatarget = cptr->period
endif
vol_default:
	ret
case_6: ; / case 8
	neg edx
case_7: ; / case 9
	add [esi+FMUSIC_CHANNEL.volume-2],edx
	ret
case_AB:
	mov [esi+eax+FMUSIC_CHANNEL.vibspeed-1],dl
	ret
case_C:
	shl edx,4
	mov [esi+FMUSIC_CHANNEL.pan-2],edx
	xchg eax,edx
case_D:
	neg edx
case_E:
	add [esi+FMUSIC_CHANNEL.pan-2],edx
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
	ret
endif ; VOLUMEBYTE_ON

if TREMOR_ON
Tremor:
; cptr+2 = ESI
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	mov dx,WORD PTR [esi+FMUSIC_CHANNEL.tremorpos-2]
	cmp dl,dh
	jbe inc_pos
	mov ecx,[esi+FMUSIC_CHANNEL.volume-2]
	neg ecx
	mov [esi+FMUSIC_CHANNEL.voldelta-2],ecx
inc_pos:
	add dh,[esi+FMUSIC_CHANNEL.tremoroff-2]
	cmp dl,dh
	jbe Tremor_Ret
	mov dl,-1
Tremor_Ret:
	inc dl
	mov [esi+FMUSIC_CHANNEL.tremorpos-2],dl
	ret
endif ; TREMOR_ON

SetBPM:
; bpm = ECX
	test ecx,ecx
	mov eax,FSOUND_MixRate*5/2
	jz SetBPM_Ret
	cdq
	div ecx
SetBPM_Ret:
	mov DWORD PTR [_mod+FMUSIC_MODULE.mixer_samplespertick],eax
	ret

; Loads an XM stream into memory. Returns non-zero on success.
LoadXM:
loadxm_count1      equ -4
loadxm_numpat      equ -8
loadxm_fnumpat     equ -12
loadxm_count2      equ -16
loadxm_skip        equ -20
loadxm_s0loopmode  equ -38
loadxm_s0bytes     equ -42
loadxm_s0looplen   equ -48
loadxm_s0loopstart equ -52
loadxm_sample_2    equ -56
loadxm_pat_size    equ -63
loadxm_pat         equ -68
loadxm_tmp29       equ -91
loadxm_tmp27       equ -93
loadxm_tmp         equ -120
	mov eax,OFFSET _mod
	push ebp
	mov esi,eax
	mov ebp,esp
	mov edx,FMUSIC_MODULE_size
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
	xor ecx,ecx
	mov eax,[esi+FMUSIC_MODULE.mixer_samplespertick]
	push ecx ; -> loadxm_count1
	; GOTO PATTERN DATA
	lea eax,[eax+60]
	; pos  : EAX
	; org  : ECX
	; !org : Z
	call uFMOD_lseek
	add esp,-116
	push ebx
	; SAVE TRACK TITLE
if INFO_API_ON
	push 20 ; a title has max. 20 chars
	lea edx,[esi+17]
	mov edi,OFFSET szTtl
if UCODE
	xor eax,eax
endif
	pop ecx
loadxm_ttl:
	mov al,[edx]
	inc edx
	cmp al,20h ; copy only printable chars
	jl loadxm_ttl_ok
if UCODE
	stosw
else
	stosb
endif
loadxm_ttl_ok:
	dec ecx
	jnz loadxm_ttl
	xchg eax,ecx
	stosd
else
	xor eax,eax
endif
	; COUNT NUM. OF PATTERNS
	movzx ecx,WORD PTR [esi+FMUSIC_MODULE.numorders]
	movzx ebx,WORD PTR [esi+FMUSIC_MODULE.numpatternsmem]
	neg ebx
	mov edi,esi
	sbb edx,edx
	and ecx,edx
	neg ebx
	dec ecx
	movzx edx,dl
	mov [ebp+loadxm_fnumpat],ebx
if CHK4VALIDITY
	cmp ecx,edx
	ja loadxm_R
endif
loadxm_for_pats:
	mov dl,[esi+FMUSIC_MODULE.orderlist]
	cmp edx,eax
	jbe loadxm_for_continue
	xchg eax,edx
loadxm_for_continue:
	inc esi
	dec ecx
	jns loadxm_for_pats
	mov [ebp+loadxm_numpat],eax
	inc eax
	mov esi,edi
	; ALLOCATE THE PATTERN ARRAY (whatever is bigger: fnumpat or numpat) & CHANNEL POOL
	cmp eax,ebx
	jae loadxm_pats_ok2
	xchg eax,ebx
loadxm_pats_ok2:
	movzx ecx,BYTE PTR [esi+FMUSIC_MODULE.numinsts]
	imul ecx,FMUSIC_INSTRUMENT_size
	mov [esi+FMUSIC_MODULE.numpatternsmem],ax
	lea edi,[ecx+eax*FMUSIC_PATTERN_size]
	mov eax,edi
	sub edi,ecx
	movzx ecx,BYTE PTR [esi+FMUSIC_MODULE.numchannels_xm]
if CHK4VALIDITY
	cmp ecx,64
	jle loadxm_numchan_ok
	xor ecx,ecx
loadxm_numchan_ok:
endif
	mov [esi+FMUSIC_MODULE.numchannels],ecx
	mov ebx,ecx
	shl ebx,7 ; *FSOUND_CHANNEL_size*2 == *FMUSIC_CHANNEL_size
if CHK4VALIDITY
	jz loadxm_R
endif
	mov [ebp+loadxm_count2],ecx
	lea eax,[eax+ebx*2]
	; numbytes : EAX
	call alloc
	lea edx,[eax+ebx]
	mov [esi+FMUSIC_MODULE.Channels],eax
	mov [esi+FMUSIC_MODULE.uFMOD_Ch],edx
	mov ebx,FMUSIC_CHANNEL_size ; = FSOUND_CHANNEL_size*2
loop_2:
	mov BYTE PTR [eax+FSOUND_CHANNEL.speedhi],1
	mov [edx+FMUSIC_CHANNEL.cptr],eax
	add eax,ebx
	add edx,ebx
	dec DWORD PTR [ebp+loadxm_count2]
	jnz loop_2
	mov [esi],edx ; FMUSIC_MODULE.pattern
	add edi,edx
	movzx ecx,WORD PTR [esi+FMUSIC_MODULE.defaultbpm]
	mov [esi+FMUSIC_MODULE.instrument],edi
	mov edi,edx
	; bpm : ECX
	call SetBPM
	push 64
	movzx ecx,WORD PTR [esi+FMUSIC_MODULE.defaultspeed]
	pop DWORD PTR [esi+FMUSIC_MODULE.globalvolume]
	mov [esi+FMUSIC_MODULE.speed],ecx
	; ALLOCATE INSTRUMENT ARRAY
	mov eax,[ebp+loadxm_fnumpat]
	; READ & UNPACK PATTERNS
loadxm_load_pats:
	push 9
	lea eax,[ebp+loadxm_pat]
	pop edx
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
	; ALLOCATE PATTERN BUFFER
	mov eax,[ebp+loadxm_pat_size] ; length of pattern & packed pattern size
	mov ecx,[esi+FMUSIC_MODULE.numchannels]
	cmp eax,10000h
	mov [edi],eax
	movzx eax,ax
	jb loadxm_ldpats_continue ; skip an empty pattern
if CHK4VALIDITY
	cmp eax,257
	sbb edx,edx
	and eax,edx
	jz loadxm_R
endif
	mul ecx
	mov [ebp+loadxm_count2],eax
	lea eax,[eax+eax*4] ; x SIZE FMUSIC_NOTE
	; numbytes : EAX
	call alloc
	mov [edi+FMUSIC_PATTERN.data],eax
	xchg eax,ebx
loadxm_for_rowsxchan:
	push esi
	mov esi,[uFMOD_fread]
	xor edx,edx
	lea eax,[ebp+loadxm_skip]
	inc edx
	; buf  : EAX
	; size : EDX
	call esi ; uFMOD_fread
	movzx edx,BYTE PTR [ebp+loadxm_skip]
	test dl,80h
	jz loadxm_noskip
	and edx,1
	jz loadxm_nonote
	mov eax,ebx ; &nptr->note
	; buf  : EAX
	; size : EDX
	call esi ; uFMOD_fread
loadxm_nonote:
	test BYTE PTR [ebp+loadxm_skip],2
	jz loadxm_nonumber
	xor edx,edx
	lea eax,[ebx+1] ; &nptr->number
	inc edx
	; buf  : EAX
	; size : EDX
	call esi ; uFMOD_fread
loadxm_nonumber:
	test BYTE PTR [ebp+loadxm_skip],4
	jz loadxm_novolume
	xor edx,edx
	lea eax,[ebx+2] ; &nptr->volume
	inc edx
	; buf  : EAX
	; size : EDX
	call esi ; uFMOD_fread
loadxm_novolume:
	test BYTE PTR [ebp+loadxm_skip],8
	jz loadxm_noeffect
	xor edx,edx
	lea eax,[ebx+3] ; &nptr->effect
	inc edx
	; buf  : EAX
	; size : EDX
	call esi ; uFMOD_fread
loadxm_noeffect:
	test BYTE PTR [ebp+loadxm_skip],16
	jz loadxm_isnote97
	xor edx,edx
	lea eax,[ebx+4] ; &nptr->eparam
	inc edx
	jmp loadxm_skip_read
loadxm_noskip:
	test edx,edx
	jz loadxm_skip_z
	mov [ebx],dl
loadxm_skip_z:
	lea eax,[ebx+1]
	mov dl,4
loadxm_skip_read:
	; buf  : EAX
	; size : EDX
	call esi ; uFMOD_fread
loadxm_isnote97:
	pop esi
	inc ebx
	mov dl,BYTE PTR [esi+FMUSIC_MODULE.numinsts]
	cmp [ebx],dl
	jbe loadxm_number_ok
	mov BYTE PTR [ebx],0
loadxm_number_ok:
	add ebx,4
	dec DWORD PTR [ebp+loadxm_count2]
	jnz loadxm_for_rowsxchan
loadxm_ldpats_continue:
	inc DWORD PTR [ebp+loadxm_count1]
	mov eax,[ebp+loadxm_fnumpat]
	add edi,8
	cmp eax,[ebp+loadxm_count1]
	ja loadxm_load_pats
	; allocate and clean out any extra patterns
	mov ecx,[ebp+loadxm_numpat]
	cmp ecx,eax
	jb loadxm_extrapats_ok
	mov ebx,[esi] ; FMUSIC_MODULE.pattern
	mov eax,[esi+FMUSIC_MODULE.numchannels]
	push esi
	lea esi,[ebx+ecx*FMUSIC_PATTERN_size]
	lea edi,[eax+eax*4]
	shl edi,6 ; numchannels*64*SIZE FMUSIC_NOTE
loadxm_for_extrapats:
	dec DWORD PTR [ebp+loadxm_numpat]
	mov eax,edi
	mov BYTE PTR [esi],64 ; pptr->rows = 64
	; Allocate memory for pattern buffer
	; numbytes : EAX
	call alloc
	mov [esi+FMUSIC_PATTERN.data],eax
	sub esi,FMUSIC_PATTERN_size
	mov eax,[ebp+loadxm_fnumpat]
	cmp [ebp+loadxm_numpat],eax
	jae loadxm_for_extrapats
	pop esi
loadxm_extrapats_ok:
	xor eax,eax
	mov [esi+FMUSIC_MODULE.mixer_samplesleft],eax
	mov [esi+FMUSIC_MODULE.tick],eax
if PATTERNDELAY_ON
	lea edi,[esi+FMUSIC_MODULE.patterndelay]
	stosd
else
	lea edi,[esi+FMUSIC_MODULE.nextorder]
endif
	stosd
	stosd
	; Load instrument information
	mov al,BYTE PTR [esi+FMUSIC_MODULE.numinsts]
	test al,al
	jz loadxm_ret1
	mov [ebp+loadxm_count1],al
	mov ebx,[esi+FMUSIC_MODULE.instrument]
loadxm_for_instrs:
	push 33
	lea eax,[ebp+loadxm_tmp]
	pop edx
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread] ; instrument size & name
	mov esi,[ebp+loadxm_tmp] ; firstsampleoffset = tmp[0]
	mov dl,[ebp+loadxm_tmp27]
	sub esi,33
	test dl,dl
	jz loadxm_inst_ok
if CHK4VALIDITY
	xor eax,eax
	cmp DWORD PTR [ebp+loadxm_tmp29],41
	sbb ecx,ecx
	not ecx
	or edx,ecx
	cmp dl,16
	ja loadxm_R ; if(numsamples > 16) goto error
endif
	mov edx,208
	lea eax,[ebx+FMUSIC_INSTRUMENT.keymap]
	sub esi,edx
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
loadxm_inst_ok:
	xor ecx,ecx
	xchg eax,esi
	inc ecx ; SEEK_CUR
	; pos  : EAX
	; org  : ECX
	; !org : Z
	call uFMOD_lseek
	lea edx,[ebx+FMUSIC_INSTRUMENT.VOLfade]
	xor eax,eax
	mov cx,[edx]
	shl ecx,1
	cmp BYTE PTR [edx+FMUSIC_INSTRUMENT.VOLnumpoints-FMUSIC_INSTRUMENT.VOLfade],2
	mov [edx],cx ; iptr->VOLfade *= 2
	jnb ladxm_voltype_ok
	mov BYTE PTR [edx+FMUSIC_INSTRUMENT.VOLtype-FMUSIC_INSTRUMENT.VOLfade],al
ladxm_voltype_ok:
	cmp BYTE PTR [edx+FMUSIC_INSTRUMENT.PANnumpoints-FMUSIC_INSTRUMENT.VOLfade],2
	jnb loadxm_PANtype_ok
	mov BYTE PTR [edx+FMUSIC_INSTRUMENT.PANtype-FMUSIC_INSTRUMENT.VOLfade],al
loadxm_PANtype_ok:
	cmp [ebp+loadxm_tmp27],al
	je loadx_for_loadsamp_end
	mov [ebp+loadxm_numpat],eax
	mov [ebp+loadxm_fnumpat],ebx ; FMUSIC_INSTRUMENT.sample
loadxm_for_samp:
	lea eax,[ebp+loadxm_sample_2]
	mov edx,[ebp+loadxm_tmp29]
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
	mov esi,[ebp+loadxm_s0loopstart]
	mov edi,[ebp+loadxm_s0looplen]
	mov al,[ebp+loadxm_s0bytes]
	mov ecx,eax
	shr eax,4 ; sample[0].bytes >>= 4
	and al,1  ; [b 4] : 8/16 bit sample data
	mov [ebp+loadxm_s0bytes],al
	jz loadxm_s0bytes_ok
	shr DWORD PTR [ebp+loadxm_sample_2],1
	shr esi,1
	shr edi,1
loadxm_s0bytes_ok:
	mov eax,[ebp+loadxm_sample_2]
	cmp eax,esi
	jg loadxm_loopstart_ok
	mov esi,eax
loadxm_loopstart_ok:
	lea edx,[esi+edi]
	sub edx,eax
	js loadxm_looplen_ok
	sub edi,edx
loadxm_looplen_ok:
	and ecx,3 ; [b 0-1] : loop type
	jz loadxm_reset_sample
	test edi,edi
	jnz loadxm_s0loop_ok
loadxm_reset_sample:
	xor esi,esi
	xor ecx,ecx
	mov edi,eax
loadxm_s0loop_ok:
	mov [ebp+loadxm_s0loopstart],esi
	mov [ebp+loadxm_s0looplen],edi
	mov [ebp+loadxm_s0loopmode],cl
	lea eax,[eax+eax+26] ; sample[0].length*2+SIZE FSOUND_SAMPLE+4
	; numbytes : EAX
	call alloc
	mov ecx,[ebp+loadxm_fnumpat]
	mov [ecx],eax
	; memcpy(iptr->sample[count2],sample,sizeof(FSOUND_SAMPLE))
	inc DWORD PTR [ebp+loadxm_numpat]
	add DWORD PTR [ebp+loadxm_fnumpat],4
	push 5
	xchg eax,edi
	mov eax,[ebp+loadxm_numpat]
	pop ecx
	cmp al,[ebp+loadxm_tmp27]
	lea esi,[ebp+loadxm_sample_2]
	rep movsd
	jb loadxm_for_samp
	; Load sample data
	mov [ebp+loadxm_numpat],ecx
	; ebx <- FMUSIC_INSTRUMENT.sample
loadx_for_loadsamp:
	mov esi,[ebx+ecx*4]
	xor eax,eax
	mov edx,[esi]
	mov ch,[esi+FSOUND_SAMPLE.Resved]
	mov cl,[esi+FSOUND_SAMPLE.bytes]
if CHK4VALIDITY
	test edx,0FFC00000h
	jnz loadxm_R
endif
	add esi,FSOUND_SAMPLE.buff
if ADPCM_ON
	cmp ch,0ADh ; ModPlug 4-bit ADPCM
	jne loadxm_regular_samp
	inc edx
	mov edi,esi
	sar edx,1
	push ebx
	push edx
	lea edx,[edx+edx*2]
	add edi,edx ; ptr = buff+compressed_length*3
	; Read in the compression table
	lea edx,[eax+16] ; edx = 16
	lea eax,[ebp+loadxm_sample_2]
	mov ebx,eax
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
	; Read in the sample data
	pop edx
	mov eax,edi
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
	; Decompress sample data
	mov edx,esi
	xor ecx,ecx ; delta
loadxm_unpack_loop:
	cmp edx,edi
	jge loadxm_unpack_ok
	mov al,[edi]
	mov ah,al
	and al,0Fh
	xlatb
	shr ah,4
	inc edi
	add ch,al
	mov al,ah
	xlatb
	add al,ch
	shl eax,24
	or ecx,eax
	mov [edx],ecx
	shr ecx,16 ; ch <- delta
	add edx,4
	jmp loadxm_unpack_loop
loadxm_unpack_ok:
	pop ebx
	jmp loadxm_chk_loop_bidi
loadxm_regular_samp:
endif
	shl edx,cl ; sptr->length << sptr->bytes
	mov eax,esi
	; buf  : EAX
	; size : EDX
	call [uFMOD_fread]
	mov ecx,DWORD PTR [esi+FSOUND_SAMPLE._length-FSOUND_SAMPLE.buff]
	lea edi,[ecx+esi] ; buff = sptr->buff+sptr->length
	lea eax,[edi+ecx] ; ptr  = buff+sptr->length
	xor edx,edx
	cmp BYTE PTR [esi+FSOUND_SAMPLE.bytes-FSOUND_SAMPLE.buff],dl
	jne loadxm_16bit_ok
	; Promote to 16 bits
loadxm_to16bits:
	dec eax
	dec edi
	dec eax
	mov dh,[edi]
	cmp eax,edi
	mov [eax],dx
	ja loadxm_to16bits
	xor edx,edx
loadxm_16bit_ok:
	mov eax,esi
	; Do delta conversion
loadxm_do_delta_conv:
	add dx,[eax]
	mov [eax],dx
	dec ecx
	lea eax,[eax+2]
	jg loadxm_do_delta_conv
	js loadxm_loops_ok
loadxm_chk_loop_bidi:
	mov eax,DWORD PTR [esi+FSOUND_SAMPLE.looplen-FSOUND_SAMPLE.buff]
	mov ecx,DWORD PTR [esi+FSOUND_SAMPLE.loopstart-FSOUND_SAMPLE.buff]
	add eax,ecx
	cmp BYTE PTR [esi+FSOUND_SAMPLE.loopmode-FSOUND_SAMPLE.buff],2 ; LOOP_BIDI
	lea eax,[esi+eax*2]
	jnz loadxm_chk_loop_normal
	mov cx,[eax-2]
	jmp loadxm_fix_loop
loadxm_chk_loop_normal:
	cmp BYTE PTR [esi+FSOUND_SAMPLE.loopmode-FSOUND_SAMPLE.buff],1 ; LOOP_NORMAL
	jnz loadxm_loops_ok
	mov cx,WORD PTR [esi+ecx*2]
loadxm_fix_loop:
	mov [eax],cx
loadxm_loops_ok:
	inc DWORD PTR [ebp+loadxm_numpat]
	mov ecx,[ebp+loadxm_numpat]
	cmp cl,[ebp+loadxm_tmp27]
	jb loadx_for_loadsamp
loadx_for_loadsamp_end:
	add ebx,FMUSIC_INSTRUMENT_size
	dec BYTE PTR [ebp+loadxm_count1]
	jnz loadxm_for_instrs
loadxm_ret1:
	inc eax
loadxm_R:
	pop ebx
donote_R:
	leave
	ret

DoNote:
; mod+36 = ESI
var_mod         equ -4
donote_sptr     equ -8
donote_jumpflag equ -10
donote_porta    equ -12
donote_oldpan   equ -16
donote_currtick equ -20
donote_oldfreq  equ -24
donote_iptr     equ -28
	; Point our note pointer to the correct pattern buffer, and to the
	; correct offset in this buffer indicated by row and number of channels
	mov eax,[esi+FMUSIC_MODULE.order-36]
	push ebp
	movzx ebx,BYTE PTR [eax+esi+FMUSIC_MODULE.orderlist-36]
	mov ebp,esp
	mov eax,[esi+FMUSIC_MODULE.row-36]
	lea ebx,[ecx+ebx*FMUSIC_PATTERN_size]
	mov ecx,[esi+FMUSIC_MODULE.numchannels-36]
if PATTERNBREAK_ON
	if PATTERNJUMP_ON
		mov BYTE PTR [ebp+donote_jumpflag],ch
	endif
endif
	mul ecx
	lea edi,[eax+eax*4] ; x SIZE FMUSIC_NOTE
	push esi
	add edi,[ebx+FMUSIC_PATTERN.data] ; mod->pattern[mod->orderlist[mod->order]].data+(mod->row*mod->numchannels)
	sub esp,24
	; Loop through each channel in the row
	shl ecx,7 ; x FMUSIC_CHANNEL_size
	jz donote_R
	push esi
	mov esi,[esi+FMUSIC_MODULE.uFMOD_Ch-36]
	push ebx
	inc esi
	inc esi
	add ecx,esi
donote_for_channels:
	push ecx
	mov bl,[edi+FMUSIC_NOTE.eparam]
	mov al,[edi+FMUSIC_NOTE.effect]
	and ebx,0Fh
	cmp al,FMUSIC_XM_PORTATO
	je donote_doporta
	cmp al,FMUSIC_XM_PORTATOVOLSLIDE
donote_doporta:
	setz [ebp+donote_porta]
	; First store note and instrument number if there was one
	mov cl,[edi+FMUSIC_NOTE.number]
	jz donote_rem_note
	dec cl
	js donote_rem_inst
	mov [esi+FMUSIC_CHANNEL.inst-2],cl ; remember the instrument #
donote_rem_inst:
	mov cl,[edi] ; get current note
	dec ecx
	cmp cl,96
	jae donote_rem_note
	mov [esi+FMUSIC_CHANNEL.note-2],cl ; remember the note
donote_rem_note:
	movzx ecx,BYTE PTR [esi+FMUSIC_CHANNEL.inst-2]
	mov eax,[ebp+var_mod]
	imul ecx,FMUSIC_INSTRUMENT_size
	add ecx,[eax+FMUSIC_MODULE.instrument-36]
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.note-2]
	cdq
	mov al,[eax+ecx+FMUSIC_INSTRUMENT.keymap]
	cmp al,16
	mov [ebp+donote_iptr],ecx
	jae donote_set_sptr
	mov edx,[ecx+eax*4+FMUSIC_INSTRUMENT.sample]
donote_set_sptr:
	test edx,edx
	jnz donote_valid_sptr
	mov edx,OFFSET DummySamp
donote_valid_sptr:
	mov [ebp+donote_sptr],edx
if NOTEDELAY_ON
	mov ecx,[esi+FMUSIC_CHANNEL.freq-2]
	mov eax,[esi+FMUSIC_CHANNEL.volume-2]
	mov [ebp+donote_oldfreq],ecx
	mov ecx,[esi+FMUSIC_CHANNEL.pan-2]
	mov [ebp+donote_currtick],eax
	mov [ebp+donote_oldpan],ecx
endif
if TREMOLO_ON
	; if there is no more tremolo, set volume to volume + last tremolo delta
	mov al,[edi+FMUSIC_NOTE.effect]
	cmp al,FMUSIC_XM_TREMOLO
	je donote_tremolo_vol
	cmp BYTE PTR [esi+FMUSIC_CHANNEL.recenteffect-2],FMUSIC_XM_TREMOLO
	jne donote_tremolo_vol
	mov ecx,[esi+FMUSIC_CHANNEL.voldelta-2]
	add [esi+FMUSIC_CHANNEL.volume-2],ecx
donote_tremolo_vol:
	mov [esi+FMUSIC_CHANNEL.recenteffect-2],al
endif
	xor ecx,ecx
	mov [esi+FMUSIC_CHANNEL.voldelta-2],ecx
	mov [esi+FMUSIC_CHANNEL.freqdelta-2],ecx
	mov BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME_OR_FREQ
	; PROCESS NOTE
	mov cl,[edi] ; note
	dec ecx
	cmp cl,96
	jae donote_note_ok
	; get note according to relative note
	movsx eax,BYTE PTR [edx+FSOUND_SAMPLE.relative]
	add ecx,eax
	mov eax,[ebp+var_mod]
	mov [esi+FMUSIC_CHANNEL.realnote-2],cl
	; Get period according to realnote and finetune
	test BYTE PTR [eax+FMUSIC_MODULE.flags-36],1
	je donote_flagsn1
	mov eax,[ebp+donote_sptr]
	movsx eax,BYTE PTR [eax+FSOUND_SAMPLE.finetune]
	cdq
	shl ecx,6
	sub eax,edx
	sar eax,1
	lea eax,[ecx+eax-7680]
	neg eax
if AMIGAPERIODS_ON
	jmp donote_chk_porta
donote_flagsn1:
	xchg eax,ecx              ; note   : EAX
	mov ecx,[ebp+donote_sptr] ; [sptr] : ECX
	; ESI != 0
	call AmigaPeriod
donote_chk_porta:
	mov [esi+FMUSIC_CHANNEL.period-2],eax
else
	mov [esi+FMUSIC_CHANNEL.period-2],eax
donote_flagsn1:
	mov eax,[esi+FMUSIC_CHANNEL.period-2]
endif
	; Frequency only changes if there are no portamento effects
	cmp BYTE PTR [ebp+donote_porta],0
	jne donote_freq_ok
	mov [esi+FMUSIC_CHANNEL.freq-2],eax
donote_freq_ok:
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_TRIGGER
donote_note_ok:
	; PROCESS INSTRUMENT NUMBER
	cmp BYTE PTR [edi+FMUSIC_NOTE.number],0
	je donote_zcptr_ok
	mov eax,[ebp+donote_sptr]
	; DESCRIPTION: Reset current channel
	push edi
	push 9
	movzx ecx,BYTE PTR [eax+FSOUND_SAMPLE.defvol]
	mov [esi+FMUSIC_CHANNEL.volume-2],ecx
	pop ecx
	movzx eax,BYTE PTR [eax+FSOUND_SAMPLE.defpan]
	mov [esi+FMUSIC_CHANNEL.pan-2],eax
	push 64
	xor eax,eax
	pop DWORD PTR [esi+FMUSIC_CHANNEL.envvol-2]
	push 32
	lea edi,[esi+FMUSIC_CHANNEL.envvoltick-2]
	pop DWORD PTR [esi+FMUSIC_CHANNEL.envpan-2]
	mov DWORD PTR [esi+FMUSIC_CHANNEL.fadeoutvol-2],65536
	; memset(&cptr->envvoltick,0,36)
	rep stosd
	; Retrigger tremolo and vibrato waveforms
	mov cl,[esi+FMUSIC_CHANNEL.wavecontrol-2]
	pop edi
	cmp cl,4Fh
	jge z_tremolopos_ok
	mov BYTE PTR [esi+FMUSIC_CHANNEL.tremolopos-2],al ; = 0
z_tremolopos_ok:
	test cl,0Ch
	jnz z_vibpos_ok
	mov BYTE PTR [esi+FMUSIC_CHANNEL.vibpos-2],al ; = 0
z_vibpos_ok:
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME_OR_PAN
donote_zcptr_ok:
if VOLUMEBYTE_ON
	; PROCESS VOLUME BYTE
	movzx edx,BYTE PTR [edi+FMUSIC_NOTE.uvolume]
	; volume : EDX
	; cptr+2 : ESI
	call VolByte
endif
	; PROCESS KEY OFF
	cmp BYTE PTR [edi],97 ; note
	jae donote_set_keyoff
	cmp BYTE PTR [edi+FMUSIC_NOTE.effect],FMUSIC_XM_KEYOFF
	jne donote_keyoff_ok
donote_set_keyoff:
	inc BYTE PTR [esi+FMUSIC_CHANNEL.keyoff-2]
donote_keyoff_ok:
	; PROCESS ENVELOPES
if VOLUMEENVELOPE_ON
	mov ecx,[ebp+donote_iptr]
	test BYTE PTR [ecx+FMUSIC_INSTRUMENT.VOLtype],1
	jz donote_no_voltype
	mov al,FMUSIC_VOLUME
	; cptr+2   : ESI
	; env_iptr : ECX
	; control  : AL
	call Envelope
	jmp donote_volenv_ok
donote_no_voltype:
endif
	cmp BYTE PTR [esi+FMUSIC_CHANNEL.keyoff-2],0
	je donote_volenv_ok
	and DWORD PTR [esi+FMUSIC_CHANNEL.envvol-2],0
donote_volenv_ok:
if PANENVELOPE_ON
	mov ecx,[ebp+donote_iptr]
	test BYTE PTR [ecx+FMUSIC_INSTRUMENT.PANtype],1
	je donote_no_pantype
	mov al,FMUSIC_PAN
	; cptr+2   : ESI
	; env_iptr : ECX
	; control  : AL
	call Envelope
donote_no_pantype:
endif
	; PROCESS VOLUME FADEOUT
	cmp BYTE PTR [esi+FMUSIC_CHANNEL.keyoff-2],0
	mov ecx,[ebp+donote_iptr]
	je donote_fadevol_ok
	movzx eax,WORD PTR [ecx+FMUSIC_INSTRUMENT.VOLfade]
	sub [esi+FMUSIC_CHANNEL.fadeoutvol-2],eax
	jns donote_fadevol_ok
	and DWORD PTR [esi+FMUSIC_CHANNEL.fadeoutvol-2],0
donote_fadevol_ok:
	; PROCESS TICK 0 EFFECTS
	movzx eax,BYTE PTR [edi+FMUSIC_NOTE.effect]
	dec eax ; skip FMUSIC_XM_ARPEGGIO
	movzx edx,BYTE PTR [edi+FMUSIC_NOTE.eparam]
if EXTRAFINEPORTA_ON
	cmp al,32
else
	if TREMOR_ON
		cmp al,28
	else
		if MULTIRETRIG_ON
			cmp al,26
		else
			if PANSLIDE_ON
				cmp al,24
			else
				if SETENVELOPEPOS_ON
					cmp al,20
				else
					if GLOBALVOLSLIDE_ON
						cmp al,16
					else
						if SETGLOBALVOLUME_ON
							cmp al,15
						else
							if SETSPEED_ON
								cmp al,14
							else
								cmp al,13
							endif
						endif
					endif
				endif
			endif
		endif
	endif
endif
	ja donote_s1_brk
	test edx,edx
	call DWORD PTR [eax*4+S1_TBL]
donote_s1_brk:
if INSTRUMENTVIBRATO_ON
	push DWORD PTR [ebp+donote_iptr]
endif
	push DWORD PTR [ebp+donote_sptr]
	; cptr+2 : ESI
	call DoFlags
	sub esi,-(FMUSIC_CHANNEL_size)
	pop ecx
	add edi,FMUSIC_NOTE_size
	cmp esi,ecx
	jl donote_for_channels
	pop ebx
	pop esi
	leave
S1_r:
	ret
S1_TBL:
if PORTAUP_OR_DOWN_ON
	dd S1_C1
	dd S1_C1
else
	dd S1_r
	dd S1_r
endif
if PORTATO_ON
	dd S1_C3
else
	dd S1_r
endif
if VIBRATO_ON
	dd S1_C4
else
	dd S1_r
endif
if PORTATOVOLSLIDE_ON
	dd S1_C5
else
	dd S1_r
endif
if VIBRATOVOLSLIDE_ON
	dd S1_C6
else
	dd S1_r
endif
if TREMOLO_ON
	dd S1_C7
else
	dd S1_r
endif
if SETPANPOSITION_ON
	dd S1_C8
else
	dd S1_r
endif
if SETSAMPLEOFFSET_ON
	dd S1_C9
else
	dd S1_r
endif
if VOLUMESLIDE_ON
	dd S1_C10
else
	dd S1_r
endif
if PATTERNJUMP_ON
	dd S1_C11
else
	dd S1_r
endif
if SETVOLUME_ON
	dd S1_C12
else
	dd S1_r
endif
if PATTERNBREAK_ON
	dd S1_C13
else
	dd S1_r
endif
	dd S1_C14
if EXTRAFINEPORTA_ON
	if SETSPEED_ON
		dd S1_C15
	else
		dd S1_r
	endif
	if SETGLOBALVOLUME_ON
		dd S1_C16
	else
		dd S1_r
	endif
	if GLOBALVOLSLIDE_ON
		dd S1_C17
	else
		dd S1_r
	endif
	dd S1_r ; unassigned effect ordinal [18]
	dd S1_r ; unassigned effect ordinal [19]
	dd S1_r ; skip FMUSIC_XM_KEYOFF
	if SETENVELOPEPOS_ON
		dd S1_C21
	else
		dd S1_r
	endif
	dd S1_r ; unassigned effect ordinal [22]
	dd S1_r ; unassigned effect ordinal [23]
	dd S1_r ; unassigned effect ordinal [24]
	if PANSLIDE_ON
		dd S1_C25
	else
		dd S1_r
	endif
	dd S1_r ; unassigned effect ordinal [26]
	if MULTIRETRIG_ON
		dd S1_C27
	else
		dd S1_r
	endif
	dd S1_r ; unassigned effect ordinal [28]
	if TREMOR_ON
		dd S1_C29
	else
		dd S1_r
	endif
	dd S1_r ; unassigned effect ordinal [30]
	dd S1_r ; unassigned effect ordinal [31]
	dd S1_r ; unassigned effect ordinal [32]
	dd S1_C33
else
	if TREMOR_ON
		if SETSPEED_ON
			dd S1_C15
		else
			dd S1_r
		endif
		if SETGLOBALVOLUME_ON
			dd S1_C16
		else
			dd S1_r
		endif
		if GLOBALVOLSLIDE_ON
			dd S1_C17
		else
			dd S1_r
		endif
		dd S1_r
		dd S1_r
		dd S1_r
		if SETENVELOPEPOS_ON
			dd S1_C21
		else
			dd S1_r
		endif
		dd S1_r
		dd S1_r
		dd S1_r
		if PANSLIDE_ON
			dd S1_C25
		else
			dd S1_r
		endif
		dd S1_r
		if MULTIRETRIG_ON
			dd S1_C27
		else
			dd S1_r
		endif
		dd S1_r
		dd S1_C29
	else
		if MULTIRETRIG_ON
			if SETSPEED_ON
				dd S1_C15
			else
				dd S1_r
			endif
			if SETGLOBALVOLUME_ON
				dd S1_C16
			else
				dd S1_r
			endif
			if GLOBALVOLSLIDE_ON
				dd S1_C17
			else
				dd S1_r
			endif
			dd S1_r
			dd S1_r
			dd S1_r
			if SETENVELOPEPOS_ON
				dd S1_C21
			else
				dd S1_r
			endif
			dd S1_r
			dd S1_r
			dd S1_r
			if PANSLIDE_ON
				dd S1_C25
			else
				dd S1_r
			endif
			dd S1_r
			dd S1_C27
		else
			if PANSLIDE_ON
				if SETSPEED_ON
					dd S1_C15
				else
					dd S1_r
				endif
				if SETGLOBALVOLUME_ON
					dd S1_C16
				else
					dd S1_r
				endif
				if GLOBALVOLSLIDE_ON
					dd S1_C17
				else
					dd S1_r
				endif
				dd S1_r
				dd S1_r
				dd S1_r
				if SETENVELOPEPOS_ON
					dd S1_C21
				else
					dd S1_r
				endif
				dd S1_r
				dd S1_r
				dd S1_r
				dd S1_C25
			else
				if SETENVELOPEPOS_ON
					if SETSPEED_ON
						dd S1_C15
					else
						dd S1_r
					endif
					if SETGLOBALVOLUME_ON
						dd S1_C16
					else
						dd S1_r
					endif
					if GLOBALVOLSLIDE_ON
						dd S1_C17
					else
						dd S1_r
					endif
					dd S1_r
					dd S1_r
					dd S1_r
					dd S1_C21
				else
					if GLOBALVOLSLIDE_ON
						if SETSPEED_ON
							dd S1_C15
						else
							dd S1_r
						endif
						if SETGLOBALVOLUME_ON
							dd S1_C16
						else
							dd S1_r
						endif
						dd S1_C17
					else
						if SETGLOBALVOLUME_ON
							if SETSPEED_ON
								dd S1_C15
							else
								dd S1_r
							endif
							dd S1_C16
						else
							if SETSPEED_ON
								dd S1_C15
							endif
						endif
					endif
				endif
			endif
		endif
	endif
endif
if PORTAUP_OR_DOWN_ON
S1_C1:
	jz donote_xm_porta_end
	mov [esi+FMUSIC_CHANNEL.portaupdown-2],dl
donote_xm_porta_end:
	ret
endif
if PORTATO_ON
S1_C3:
	jz donote_xm_portato_end
	mov [esi+FMUSIC_CHANNEL.portaspeed-2],dl
donote_xm_portato_end:
if PORTATOVOLSLIDE_ON
	jmp donote_xm_portavolsl_end
else
	mov eax,[esi+FMUSIC_CHANNEL.period-2]
	and BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],NOT_FMUSIC_TRIGGER_OR_FRQ
	mov [esi+FMUSIC_CHANNEL.portatarget-2],eax
	ret
endif
endif
if PORTATOVOLSLIDE_ON
S1_C5:
	jz donote_xm_portavolsl_end
	mov [esi+FMUSIC_CHANNEL.volslide-2],dl
donote_xm_portavolsl_end:
	mov eax,[esi+FMUSIC_CHANNEL.period-2]
	and BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],NOT_FMUSIC_TRIGGER_OR_FRQ
	mov [esi+FMUSIC_CHANNEL.portatarget-2],eax
	ret
endif
if VIBRATO_ON
S1_C4:
	shr edx,4
	jz donote_vib_x_ok
	mov [esi+FMUSIC_CHANNEL.vibspeed-2],dl
donote_vib_x_ok:
	test ebx,ebx
	jz donote_vib_y_ok
	mov BYTE PTR [esi+FMUSIC_CHANNEL.vibdepth-2],bl
donote_vib_y_ok:
if VIBRATOVOLSLIDE_ON
	xor eax,eax
else
	; cptr+2 : ESI
	jmp Vibrato
endif
endif
if VIBRATOVOLSLIDE_ON
S1_C6:
	jz donote_xm_vibvolsl_end
	mov [esi+FMUSIC_CHANNEL.volslide-2],dl
donote_xm_vibvolsl_end:
	; cptr+2 : ESI
	jmp Vibrato
endif
if TREMOLO_ON
S1_C7:
	shr edx,4
	jz donote_trem_x_ok
	mov [esi+FMUSIC_CHANNEL.tremolospeed-2],dl
donote_trem_x_ok:
	test ebx,ebx
	jz donote_trem_y_ok
	mov [esi+FMUSIC_CHANNEL.tremolodepth-2],bl
donote_trem_y_ok:
	ret
endif
if SETPANPOSITION_ON
if PANSLIDE_ON
else
	S1_C8:
		mov [esi+FMUSIC_CHANNEL.pan-2],edx
		or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
		ret
	endif
endif
if SETSAMPLEOFFSET_ON
S1_C9:
	shl edx,8
	jz donote_soffset_ok
	mov [esi+FMUSIC_CHANNEL.sampleoffset-2],edx
donote_soffset_ok:
	mov ecx,[ebp+donote_sptr]
	mov edx,[ecx+FSOUND_SAMPLE.loopstart]
	add edx,[ecx+FSOUND_SAMPLE.looplen]
	mov eax,[esi+FMUSIC_CHANNEL.sampleoffset-2]
	cmp eax,edx
	mov ecx,[esi+FMUSIC_CHANNEL.cptr-2]
	jb donote_set_offset
	xor eax,eax
	and BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],NOT_FMUSIC_TRIGGER
	mov [ecx+FSOUND_CHANNEL.mixpos],eax
	mov [ecx+FSOUND_CHANNEL.mixposlo],eax
donote_set_offset:
	mov [ecx+FSOUND_CHANNEL.fsampleoffset],eax
	ret
endif
if VOLUMESLIDE_ON
S1_C10:
	jz donote_volslide_ok
	mov [esi+FMUSIC_CHANNEL.volslide-2],dl
donote_volslide_ok:
	ret
endif
if PATTERNJUMP_ON
S1_C11:
	mov eax,[ebp+var_mod]
	and DWORD PTR [eax+FMUSIC_MODULE.nextrow-36],0
	mov [eax+FMUSIC_MODULE.nextorder-36],edx
if PATTERNBREAK_ON
	inc BYTE PTR [ebp+donote_jumpflag]
donote_set_nextord:
endif
	movzx ecx,WORD PTR [eax+FMUSIC_MODULE.numorders-36]
	cmp [eax+FMUSIC_MODULE.nextorder-36],ecx
	jl donote_nextorder_ok
	and DWORD PTR [eax+FMUSIC_MODULE.nextorder-36],0
donote_nextorder_ok:
	ret
endif
if PATTERNBREAK_ON
S1_C13:
	shr edx,4
	lea edx,[edx+edx*4]
	lea ecx,[ebx+edx*2] ; paramx*10+paramy
	mov eax,[ebp+var_mod]
	mov [eax+FMUSIC_MODULE.nextrow-36],ecx
if PATTERNJUMP_ON
	cmp BYTE PTR [ebp+donote_jumpflag],0
	jne donote_nextorder_ok
endif
	mov ecx,[eax+FMUSIC_MODULE.order-36]
	inc ecx
	mov [eax+FMUSIC_MODULE.nextorder-36],ecx
if PATTERNJUMP_ON
	jmp donote_set_nextord
else
	movzx ecx,WORD PTR [eax+FMUSIC_MODULE.numorders-36]
	cmp [eax+FMUSIC_MODULE.nextorder-36],ecx
	jl donote_jump_pat
	and DWORD PTR [eax+FMUSIC_MODULE.nextorder-36],0
donote_jump_pat:
	ret
endif
endif
if SETSPEED_ON
S1_C15:
	cmp dl,20h
	mov ecx,edx
	jae donote_setbpm
	mov eax,[ebp+var_mod]
	mov [eax+FMUSIC_MODULE.speed-36],ecx
	ret
donote_setbpm:
	; bpm : ECX
	jmp SetBPM
endif
if GLOBALVOLSLIDE_ON
S1_C17:
	jz donote_globalvsl_ok
	mov ecx,[ebp+var_mod]
	mov [ecx+FMUSIC_MODULE.globalvsl-36],dl
donote_globalvsl_ok:
	ret
endif
if SETENVELOPEPOS_ON
S1_C21:
	test BYTE PTR [ecx+FMUSIC_INSTRUMENT.VOLtype],1
	je donote_envelope_r
	lea ebx,[ecx+FMUSIC_INSTRUMENT.VOLPoints+4]
	; Search and reinterpolate new envelope position
	movzx ecx,BYTE PTR [ecx+FMUSIC_INSTRUMENT.VOLnumpoints]
	xor eax,eax
	cmp dx,[ebx]
	jbe donote_env_endwhile
donote_envwhile:
	cmp eax,ecx ; if(currpos == iptr->VOLnumpoints) break
	je donote_env_endwhile
	inc eax
	cmp dx,[ebx+eax*4] ; if(current->eparam > iptr->VOLPoints[(currpos+1)<<1]) break
	ja donote_envwhile
donote_env_endwhile:
	mov [esi+FMUSIC_CHANNEL.envvolpos-2],eax
	; if it is at the last position, abort the envelope and continue last volume
	dec ecx
	cmp eax,ecx
	setnl [esi+FMUSIC_CHANNEL.envvolstopped-2]
	jl donote_env_continue
	movzx eax,WORD PTR [ebx+ecx*4-2]
	mov [esi+FMUSIC_CHANNEL.envvol-2],eax ; cptr->envvol = iptr->VOLPoints[((iptr->VOLnumpoints-1)<<1)+1]
donote_envelope_r:
	ret
donote_env_continue:
	mov [esi+FMUSIC_CHANNEL.envvoltick-2],edx
	mov ecx,[ebx+eax*4-4] ; get tick at this point + VOL at this point
	mov edx,ecx
	movzx ecx,cx
	mov [ebp+donote_currtick],ecx
	mov ecx,[ebx+eax*4] ; get tick at next point + VOL at next point
	mov eax,ecx
	movzx ecx,cx
	xor dx,dx
	; interpolate 2 points to find delta step
	sub ecx,[ebp+donote_currtick]
	push edx
	jz donote_no_tickdiff
	xor ax,ax
	sub eax,edx
	cdq
	idiv ecx
	xchg eax,ecx
donote_no_tickdiff:
	mov [esi+FMUSIC_CHANNEL.envvoldelta-2],ecx
	mov eax,[esi+FMUSIC_CHANNEL.envvoltick-2]
	sub eax,[ebp+donote_currtick]
	imul ecx
	pop edx
	add eax,edx
	mov [esi+FMUSIC_CHANNEL.envvolfrac-2],eax
	sar eax,16
	mov [esi+FMUSIC_CHANNEL.envvol-2],eax
	inc DWORD PTR [esi+FMUSIC_CHANNEL.envvolpos-2]
	ret
endif
if PANSLIDE_ON
S1_C25:
	jz donote_panslide_ok
	mov [esi+FMUSIC_CHANNEL.panslide-2],dl
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
donote_panslide_ok:
	ret
endif
if MULTIRETRIG_ON
S1_C27:
	jz donote_multiretrig_ok
	shr edx,4
	mov dh,bl
	mov WORD PTR [esi+FMUSIC_CHANNEL.retrigx-2],dx
donote_multiretrig_ok:
	ret
endif
if TREMOR_ON
S1_C29:
	jz donote_do_tremor
	shr edx,4
	mov dh,bl
	mov WORD PTR [esi+FMUSIC_CHANNEL.tremoron-2],dx
donote_do_tremor:
	; cptr : ESI
	jmp Tremor
endif
if EXTRAFINEPORTA_ON
S1_C33:
	shr edx,4
	dec edx
	jnz donote_paramx_n1
	test ebx,ebx
	jz donote_paramy_z1
	mov [esi+FMUSIC_CHANNEL.xtraportaup-2],bl
donote_paramy_z1:
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.xtraportaup-2]
	sub [esi+FMUSIC_CHANNEL.freq-2],eax
donote_paramx_n1:
	dec edx
	jnz donote_paramx_n2
	test ebx,ebx
	jz donote_paramy_z2
	mov [esi+FMUSIC_CHANNEL.xtraportadown-2],bl
donote_paramy_z2:
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.xtraportadown-2]
	add [esi+FMUSIC_CHANNEL.freq-2],eax
donote_paramx_n2:
endif
S2_r:
	ret
S1_C14:
	shr edx,4
	dec edx ; skip FMUSIC_XM_SETFILTER
if FINEPORTAUP_ON
else
	dec edx
endif
if PATTERNDELAY_ON
	cmp dl,13
else
	if NOTEDELAY_ON
		cmp dl,12
	else
		if FINEVOLUMESLIDE_ON
			cmp dl,10
		else
			if SETPANPOSITION16_ON
				cmp dl,7
			else
				if SETTREMOLOWAVE_ON
					cmp dl,6
				else
					if PATTERNLOOP_ON
						cmp dl,5
					else
						cmp dl,4
					endif
				endif
			endif
		endif
	endif
endif
if FINEPORTAUP_ON
	ja S2_r
else
	jae S2_r
endif
donote_do_special:
	test ebx,ebx
	jmp DWORD PTR [edx*4+S2_TBL]
S2_TBL:
if FINEPORTAUP_ON
	dd S2_C1
endif
if FINEPORTADOWN_ON
	dd S2_C2
else
	dd S2_r
endif
	dd S2_r ; skip FMUSIC_XM_SETGLISSANDO
if SETVIBRATOWAVE_ON
	dd S2_C4
else
	dd S2_r
endif
if SETFINETUNE_ON
	dd S2_C5
else
	dd S2_r
endif
if PATTERNDELAY_ON
	if PATTERNLOOP_ON
		dd S2_C6
	else
		dd S2_r
	endif
	if SETTREMOLOWAVE_ON
		dd S2_C7
	else
		dd S2_r
	endif
	if SETPANPOSITION16_ON
		dd S2_C8
	else
		dd S2_r
	endif
	dd S2_r ; skip FMUSIC_XM_RETRIG
	if FINEVOLUMESLIDE_ON
		dd S2_C10
		dd S2_C11
	else
		dd S2_r
		dd S2_r
	endif
	dd S2_r ; skip FMUSIC_XM_NOTECUT
	if NOTEDELAY_ON
		dd S2_C13
	else
		dd S2_r
	endif
	dd S2_C14
else
	if NOTEDELAY_ON
		if PATTERNLOOP_ON
			dd S2_C6
		else
			dd S2_r
		endif
		if SETTREMOLOWAVE_ON
			dd S2_C7
		else
			dd S2_r
		endif
		if SETPANPOSITION16_ON
			dd S2_C8
		else
			dd S2_r
		endif
		dd S2_r ; skip FMUSIC_XM_RETRIG
		if FINEVOLUMESLIDE_ON
			dd S2_C10
			dd S2_C11
		else
			dd S2_r
			dd S2_r
		endif
		dd S2_r
		dd S2_C13
	else
		if FINEVOLUMESLIDE_ON
			if PATTERNLOOP_ON
				dd S2_C6
			else
				dd S2_r
			endif
			if SETTREMOLOWAVE_ON
				dd S2_C7
			else
				dd S2_r
			endif
			if SETPANPOSITION16_ON
				dd S2_C8
			else
				dd S2_r
			endif
			dd S2_r
			dd S2_C10
			dd S2_C11
		else
			if SETPANPOSITION16_ON
				if PATTERNLOOP_ON
					dd S2_C6
				else
					dd S2_r
				endif
				if SETTREMOLOWAVE_ON
					dd S2_C7
				else
					dd S2_r
				endif
				dd S2_C8
			else
				if SETTREMOLOWAVE_ON
					if PATTERNLOOP_ON
						dd S2_C6
					else
						dd S2_r
					endif
					dd S2_C7
				else
					if PATTERNLOOP_ON
						dd S2_C6
					endif
				endif
			endif
		endif
	endif
endif
if FINEPORTAUP_ON
S2_C1:
	jz donote_finepup_ok
	mov [esi+FMUSIC_CHANNEL.fineportaup-2],bl
donote_finepup_ok:
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.fineportaup-2]
	shl eax,2
	sub [esi+FMUSIC_CHANNEL.freq-2],eax
	ret
endif
if FINEPORTADOWN_ON
S2_C2:
	jz donote_finepdown_ok
	mov [esi+FMUSIC_CHANNEL.fineportadown-2],bl
donote_finepdown_ok:
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.fineportadown-2]
	shl eax,2
	add [esi+FMUSIC_CHANNEL.freq-2],eax
	ret
endif
if SETVIBRATOWAVE_ON
S2_C4:
	and BYTE PTR [esi+FMUSIC_CHANNEL.wavecontrol-2],0F0h
	or BYTE PTR [esi+FMUSIC_CHANNEL.wavecontrol-2],bl
	ret
endif
if SETFINETUNE_ON
S2_C5:
	mov eax,[ebp+donote_sptr]
	mov [eax+FSOUND_SAMPLE.finetune],bl
	ret
endif
if PATTERNLOOP_ON
S2_C6:
	jnz donote_not_paramy
	mov eax,[ebp+var_mod]
	mov eax,[eax+FMUSIC_MODULE.row-36]
	mov [esi+FMUSIC_CHANNEL.patlooprow-2],eax
	ret
donote_not_paramy:
	mov cl,[esi+FMUSIC_CHANNEL.patloopno-2]
	dec cl
	jns donote_patloopno_ok
	mov ecx,ebx
donote_patloopno_ok:
	mov [esi+FMUSIC_CHANNEL.patloopno-2],cl
	jz donote_patloopno_end
	mov eax,[esi+FMUSIC_CHANNEL.patlooprow-2]
	mov ecx,[ebp+var_mod]
	mov [ecx+FMUSIC_MODULE.nextrow-36],eax
donote_patloopno_end:
	ret
endif
if SETTREMOLOWAVE_ON
S2_C7:
	and BYTE PTR [esi+FMUSIC_CHANNEL.wavecontrol-2],0Fh
	shl ebx,4
	or BYTE PTR [esi+FMUSIC_CHANNEL.wavecontrol-2],bl
	ret
endif
if SETPANPOSITION16_ON
S2_C8:
	shl ebx,4
	mov [esi+FMUSIC_CHANNEL.pan-2],ebx
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
	ret
endif
if FINEVOLUMESLIDE_ON
S2_C10:
	neg ebx
S2_C11:
	jz donote_finevols_ok
	mov [esi+FMUSIC_CHANNEL.finevslup-2],bl
donote_finevols_ok:
	movsx eax,BYTE PTR [esi+FMUSIC_CHANNEL.finevslup-2]
	sub [esi+FMUSIC_CHANNEL.volume-2],eax
	ret
endif
if NOTEDELAY_ON
S2_C13:
	mov eax,[ebp+donote_oldfreq]
	mov edx,[ebp+donote_currtick]
	mov [esi+FMUSIC_CHANNEL.freq-2],eax
	mov eax,[ebp+donote_oldpan]
	mov [esi+FMUSIC_CHANNEL.pan-2],eax
	mov BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],0
if SETVOLUME_ON
else
	mov [esi+FMUSIC_CHANNEL.volume-2],edx
	ret
endif
endif
if SETVOLUME_ON
S1_C12:
	mov [esi+FMUSIC_CHANNEL.volume-2],edx
	ret
endif
if PATTERNDELAY_ON
S2_C14:
	mov ecx,[ebp+var_mod]
	imul ebx,DWORD PTR [ecx+FMUSIC_MODULE.speed-36]
	mov [ecx+FMUSIC_MODULE.patterndelay-36],ebx
	ret
endif

DoEffs:
; mod+36 = ESI
; var_mod     equ -4
doeff_current equ -8
	; Point our note pointer to the correct pattern buffer, and to the
	; correct offset in this buffer indicated by row and number of channels
	mov eax,[esi+FMUSIC_MODULE.order-36]
	push ebp
	mov bl,[eax+esi+FMUSIC_MODULE.orderlist-36]
	mov eax,[esi+FMUSIC_MODULE.row-36]
	mul DWORD PTR [esi+FMUSIC_MODULE.numchannels-36]
	lea eax,[eax+eax*4] ; x SIZE FMUSIC_NOTE
	mov ebp,esp
	; mod->pattern[mod->orderlist[mod->order]].data+(mod->row*mod->numchannels)
	add eax,[ecx+ebx*FMUSIC_PATTERN_size+FMUSIC_PATTERN.data]
	push esi ; -> var_mod
	push eax ; -> doeff_current
	mov eax,[esi+FMUSIC_MODULE.numchannels-36]
	shl eax,7 ; x FMUSIC_CHANNEL_size
	push esi
	mov esi,[esi+FMUSIC_MODULE.uFMOD_Ch-36]
	inc esi
	inc esi
	add eax,esi
doeff_for_channels:
	push eax
	movzx edi,BYTE PTR [esi+FMUSIC_CHANNEL.inst-2]
	mov edx,[ebp+var_mod]
	imul edi,FMUSIC_INSTRUMENT_size
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.note-2]
	add edi,[edx+FMUSIC_MODULE.instrument-36]
	cdq
	mov al,[eax+edi+FMUSIC_INSTRUMENT.keymap]
	cmp al,16
	jae doeff_set_sptr
	mov edx,[edi+eax*4+FMUSIC_INSTRUMENT.sample]
doeff_set_sptr:
	test edx,edx
	jnz doeff_valid_sptr
	mov edx,OFFSET DummySamp
doeff_valid_sptr:
	xor ebx,ebx
if INSTRUMENTVIBRATO_ON
	push edi ; iptr
endif
	push edx ; sptr
	mov [esi+FMUSIC_CHANNEL.voldelta-2],ebx
	mov [esi+FMUSIC_CHANNEL.freqdelta-2],ebx
	mov [esi+FMUSIC_CHANNEL.notectrl-2],bl
	; PROCESS ENVELOPES
if VOLUMEENVELOPE_ON
	test BYTE PTR [edi+FMUSIC_INSTRUMENT.VOLtype],1
	je doeff_no_voltype
	mov al,FMUSIC_VOLUME
	mov ecx,edi
	; cptr+2   : ESI
	; env_iptr : ECX
	; control  : AL
	call Envelope
doeff_no_voltype:
endif
if PANENVELOPE_ON
	test BYTE PTR [edi+FMUSIC_INSTRUMENT.PANtype],1
	je doeff_no_pantype
	mov al,FMUSIC_PAN
	mov ecx,edi
	; cptr+2   : ESI
	; env_iptr : ECX
	; control  : AL
	call Envelope
doeff_no_pantype:
endif
	; PROCESS VOLUME FADEOUT
	cmp [esi+FMUSIC_CHANNEL.keyoff-2],bl
	je doeff_fadevol_ok
	movzx eax,WORD PTR [edi+FMUSIC_INSTRUMENT.VOLfade]
	sub [esi+FMUSIC_CHANNEL.fadeoutvol-2],eax
	jns doeff_fadevol_ns
	mov [esi+FMUSIC_CHANNEL.fadeoutvol-2],ebx
doeff_fadevol_ns:
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
doeff_fadevol_ok:
if VOLUMEBYTE_ON
	mov eax,[ebp+doeff_current]
	movzx eax,BYTE PTR [eax+FMUSIC_NOTE.uvolume]
	mov ecx,eax
	shr eax,4
	and cl,0Fh
	sub al,6
	jz doeff_case_vol6
	dec eax
	jz doeff_case_vol7
if VIBRATO_ON
	sub al,4
	jz doeff_case_volB
	dec eax
	dec eax
else
	sub al,6
endif
	jz doeff_case_volD
	dec eax
if PORTATO_ON
	jz doeff_case_volE
	dec eax
	jnz doeff_volbyte_end
	; cptr+2 : ESI
	call Portamento
	jmp doeff_volbyte_end
else
	jnz doeff_volbyte_end
endif
doeff_case_volE:
	neg ecx
doeff_case_volD:
	sub [esi+FMUSIC_CHANNEL.pan-2],ecx
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
	jmp doeff_volbyte_end
if VIBRATO_ON
doeff_case_volB:
	mov [esi+FMUSIC_CHANNEL.vibdepth-2],cl
	; cptr+2 : ESI
	call Vibrato
	jmp doeff_volbyte_end
endif
doeff_case_vol6:
	neg ecx
doeff_case_vol7:
	add [esi+FMUSIC_CHANNEL.volume-2],ecx
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
doeff_volbyte_end:
endif ; VOLUMEBYTE_ON
	mov edx,[ebp+doeff_current]
	movzx ebx,BYTE PTR [edx+FMUSIC_NOTE.eparam]
	mov ecx,ebx
	and ebx,0Fh ; grab the effect parameter y
	movzx eax,BYTE PTR [edx+FMUSIC_NOTE.effect]
	shr cl,4    ; grab the effect parameter x
if ARPEGGIO_ON
else
	dec eax
endif
if TREMOR_ON
	cmp al,29
else
	if MULTIRETRIG_ON
		cmp al,27
	else
		if PANSLIDE_ON
			cmp al,25
		else
			if GLOBALVOLSLIDE_ON
				cmp al,17
			else
				if RETRIG_ON
					cmp al,14
				else
					if NOTECUT_ON
						cmp al,14
					else
						if NOTEDELAY_ON
							cmp al,14
						else
							cmp al,10
						endif
					endif
				endif
			endif
		endif
	endif
endif
if ARPEGGIO_ON
	ja doeff_s3_brk
else
	jae doeff_s3_brk
endif
	call DWORD PTR [eax*4+S3_TBL]
doeff_s3_brk:
	; cptr+2 : ESI
	call DoFlags
	sub esi,-(FMUSIC_CHANNEL_size)
	pop eax
	add DWORD PTR [ebp+doeff_current],FMUSIC_NOTE_size
	cmp esi,eax
	jl doeff_for_channels
	pop esi
doeff_R:
	leave
S3_r:
	ret
S3_TBL:
if ARPEGGIO_ON
	dd S3_C0
endif
if PORTAUP_ON
	dd S3_C1
else
	dd S3_r
endif
if PORTADOWN_ON
	dd S3_C2
else
	dd S3_r
endif
if PORTATO_ON
	; cptr+2 : ESI
	dd Portamento
else
	dd S3_r
endif
if VIBRATO_ON
	; cptr+2 : ESI
	dd Vibrato
else
	dd S3_r
endif
if PORTATOVOLSLIDE_ON
	dd S3_C5
else
	dd S3_r
endif
if VIBRATOVOLSLIDE_ON
	dd S3_C6
else
	dd S3_r
endif
if TREMOLO_ON
	; cptr+2 : ESI
	dd Tremolo
else
	dd S3_r
endif
	dd S3_r ; skip FMUSIC_XM_SETPANPOSITION
	dd S3_r ; skip FMUSIC_XM_SETSAMPLEOFFSET
if VOLUMESLIDE_ON
	dd S3_C10
else
	dd S3_r
endif
if TREMOR_ON
	dd S3_r ; skip FMUSIC_XM_PATTERNJUMP
	dd S3_r ; slip FMUSIC_XM_SETVOLUME
	dd S3_r ; skip FMUSIC_XM_PATTERNBREAK
	dd S3_C14
	dd S3_r ; skip FMUSIC_XM_SETSPEED
	dd S3_r ; skip FMUSIC_XM_SETGLOBALVOLUME
	if GLOBALVOLSLIDE_ON
		dd S3_C17
	else
		dd S3_r
	endif
	dd S3_r ; unassigned effect ordinal [18]
	dd S3_r ; unassigned effect ordinal [19]
	dd S3_r ; skip FMUSIC_XM_KEYOFF
	dd S3_r ; skip FMUSIC_XM_SETENVELOPEPOS
	dd S3_r ; unassigned effect ordinal [22]
	dd S3_r ; unassigned effect ordinal [23]
	dd S3_r ; unassigned effect ordinal [24]
	if PANSLIDE_ON
		dd S3_C25
	else
		dd S3_r
	endif
	dd S3_r ; unassigned effect ordinal [26]
	if MULTIRETRIG_ON
		dd S3_C27
	else
		dd S3_r
	endif
	dd S3_r ; unassigned effect ordinal [28]
	; case FMUSIC_XM_TREMOR
	; cptr : ESI
	dd Tremor
else
	if MULTIRETRIG_ON
		dd S3_r
		dd S3_r
		dd S3_r
		dd S3_C14
		dd S3_r
		dd S3_r
		if GLOBALVOLSLIDE_ON
			dd S3_C17
		else
			dd S3_r
		endif
		dd S3_r
		dd S3_r
		dd S3_r
		dd S3_r
		dd S3_r
		dd S3_r
		dd S3_r
		if PANSLIDE_ON
			dd S3_C25
		else
			dd S3_r
		endif
		dd S3_r
		dd S3_C27
	else
		if PANSLIDE_ON
			dd S3_r
			dd S3_r
			dd S3_r
			dd S3_C14
			dd S3_r
			dd S3_r
			if GLOBALVOLSLIDE_ON
				dd S3_C17
			else
				dd S3_r
			endif
			dd S3_r
			dd S3_r
			dd S3_r
			dd S3_r
			dd S3_r
			dd S3_r
			dd S3_r
			dd S3_C25
		else
			if GLOBALVOLSLIDE_ON
				dd S3_r
				dd S3_r
				dd S3_r
				dd S3_C14
				dd S3_r
				dd S3_r
				dd S3_C17
			else
				if RETRIG_ON
					dd S3_r
					dd S3_r
					dd S3_r
					dd S3_C14
				else
					if NOTECUT_ON
						dd S3_r
						dd S3_r
						dd S3_r
						dd S3_C14
					else
						if NOTEDELAY_ON
							dd S3_r
							dd S3_r
							dd S3_r
							dd S3_C14
						endif
					endif
				endif
			endif
		endif

	endif
endif
if VIBRATOVOLSLIDE_ON
S3_C6:
	; cptr+2 : ESI
	call Vibrato
if VOLUMESLIDE_ON
	if PORTATOVOLSLIDE_ON
		jmp S3_C10
	endif
else
	mov cl,[esi+FMUSIC_CHANNEL.volslide-2]
	mov eax,ecx
	and eax,0Fh
	shr ecx,4
	jz doeff_volslide_ok
	; Slide up takes precedence over down
	xchg eax,ecx
	neg eax
doeff_volslide_ok:
	sub [esi+FMUSIC_CHANNEL.volume-2],eax
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	ret
endif
endif
if PORTATOVOLSLIDE_ON
S3_C5:
	; cptr+2 : ESI
	call Portamento
if VOLUMESLIDE_ON
	xor ecx,ecx
else
	movzx ecx,BYTE PTR [esi+FMUSIC_CHANNEL.volslide-2]
	mov eax,ecx
	and eax,0Fh
	shr ecx,4
	jz doeff_volslide_ok
	; Slide up takes precedence over down
	xchg eax,ecx
	neg eax
doeff_volslide_ok:
	sub [esi+FMUSIC_CHANNEL.volume-2],eax
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	ret
endif
endif
if VOLUMESLIDE_ON
S3_C10:
	mov cl,[esi+FMUSIC_CHANNEL.volslide-2]
	mov eax,ecx
	and eax,0Fh
	shr ecx,4
	jz doeff_volslide_ok
	; Slide up takes precedence over down
	xchg eax,ecx
	neg eax
doeff_volslide_ok:
	sub [esi+FMUSIC_CHANNEL.volume-2],eax
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	ret
endif
if PORTADOWN_ON
S3_C2:
	mov al,[esi+FMUSIC_CHANNEL.portaupdown-2]
DoFreqSlide:
	mov ecx,[esi+FMUSIC_CHANNEL.freq-2]
	lea ecx,[ecx+eax*4]
	cmp ecx,1
	jge DoFreqSlide_ok
	push 1
	pop ecx
DoFreqSlide_ok:
	mov [esi+FMUSIC_CHANNEL.freq-2],ecx
	and DWORD PTR [esi+FMUSIC_CHANNEL.freqdelta-2],0
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
	ret
endif
if PORTAUP_ON
S3_C1:
	mov al,[esi+FMUSIC_CHANNEL.portaupdown-2]
if PORTADOWN_ON
	neg eax
	jmp DoFreqSlide
else
	shl eax,2
	mov ecx,[esi+FMUSIC_CHANNEL.freq-2]
	sub ecx,eax
	cmp ecx,1
	jge DoFreqSlide_ok
	push 1
	pop ecx
DoFreqSlide_ok:
	mov [esi+FMUSIC_CHANNEL.freq-2],ecx
	and DWORD PTR [esi+FMUSIC_CHANNEL.freqdelta-2],0
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
	ret
endif
endif
if ARPEGGIO_ON
S3_C0:
	cmp BYTE PTR [edx+FMUSIC_NOTE.eparam],ch
	jbe doeff_arpeggio_ok
	mov eax,[ebp+var_mod]
	mov eax,[eax+FMUSIC_MODULE.tick-36]
	push 3
	cdq
	pop edi
	idiv edi
	dec edx
	jz doeff_arpeg_c1
	dec edx
	jne doeffs_enable_freq
	mov ecx,ebx
doeff_arpeg_c1:
	mov eax,[ebp+var_mod]
	test BYTE PTR [eax+FMUSIC_MODULE.flags-36],1
	je doeffs_flagsn1
doeffs_arpeggio_freqd:
	shl ecx,6
	mov [esi+FMUSIC_CHANNEL.freqdelta-2],ecx
if AMIGAPERIODS_ON
	jmp doeffs_enable_freq
endif
doeffs_flagsn1:
if AMIGAPERIODS_ON
	movzx eax,BYTE PTR [esi+FMUSIC_CHANNEL.realnote-2]
	mov edi,eax
	add eax,ecx     ; note   : EAX
	mov ecx,[esp+4] ; [sptr] : ECX
	; ESI != 0
	call AmigaPeriod
	xchg eax,edi    ; note   : EAX
	mov ecx,[esp+4] ; [sptr] : ECX
	; ESI != 0
	call AmigaPeriod
	sub edi,eax
	mov [esi+FMUSIC_CHANNEL.freqdelta-2],edi
endif
doeffs_enable_freq:
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
doeff_arpeggio_ok:
	ret
endif
if MULTIRETRIG_ON
S3_C27:
	mov cl,[esi+FMUSIC_CHANNEL.retrigy-2]
	test ecx,ecx
	jz doeff_multiretrig_ok
	mov eax,[ebp+var_mod]
	mov eax,[eax+FMUSIC_MODULE.tick-36]
	cdq
	idiv ecx
	test edx,edx
	jnz doeff_multiretrig_ok
	mov cl,[esi+FMUSIC_CHANNEL.retrigx-2]
	and ecx,0Fh
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_TRIGGER
	dec ecx
	mov eax,[esi+FMUSIC_CHANNEL.volume-2]
	js doeff_multiretrig_ok
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	call DWORD PTR [ecx*4+S4_TBL]
	mov [esi+FMUSIC_CHANNEL.volume-2],eax
S4_C2:
	dec eax
S4_C1:
	dec eax
doeff_multiretrig_ok:
	ret
S4_C5:
	sub eax,8
S4_C4:
	sub eax,4
S4_C3:
	sub eax,4
	ret
S4_C6:
	shl eax,1
	push 3
	cdq
	pop ecx
	idiv ecx
	ret
S4_C14:
	lea eax,[eax+eax*2]
S4_C7:
	sar eax,1
	ret
S4_C10:
	inc eax
S4_C9:
	inc eax
	ret
S4_C13:
	add eax,8
S4_C12:
	add eax,4
S4_C11:
	add eax,4
	ret
S4_C15:
	shl eax,1
S4_r:
	ret
S4_TBL:
	dd S4_C1
	dd S4_C2
	dd S4_C3
	dd S4_C4
	dd S4_C5
	dd S4_C6
	dd S4_C7
	dd S4_r
	dd S4_C9
	dd S4_C10
	dd S4_C11
	dd S4_C12
	dd S4_C13
	dd S4_C14
	dd S4_C15
endif
if PANSLIDE_ON
S3_C25:
	mov cl,[esi+FMUSIC_CHANNEL.panslide-2]
	mov eax,ecx
	and eax,0Fh
	mov edx,[esi+FMUSIC_CHANNEL.pan-2]
	shr ecx,4
	; Slide right takes precedence over left
	jnz doeff_pan_slide_right
	sub edx,eax
	jns doeff_panslide_ok
	mov edx,ecx
	jmp doeff_panslide_ok
doeff_pan_slide_right:
	add ecx,edx
	cdq
	dec dl ; edx = 255
	cmp ecx,edx
	jg doeff_panslide_ok
	mov edx,ecx
doeff_panslide_ok:
if SETPANPOSITION_ON
S1_C8:
endif
	mov [esi+FMUSIC_CHANNEL.pan-2],edx
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
	ret
endif
if GLOBALVOLSLIDE_ON
S3_C17:
	mov ecx,[ebp+var_mod]
	mov edx,[ecx+FMUSIC_MODULE.globalvolume-36]
	mov al,[ecx+FMUSIC_MODULE.globalvsl-36]
	test al,0F0h
	; Slide up takes precedence over down
	jnz doeff_gvsl_slide_up
	and eax,0Fh
	sub edx,eax
	jns set_global_vol
	cdq
	xor eax,eax
doeff_gvsl_slide_up:
	shr eax,4
	add edx,eax
set_global_vol:
	if SETGLOBALVOLUME_ON
	else
		cmp edx,64
		jle doeff_setglobalvol
		push 64
		pop edx
doeff_setglobalvol:
		mov [ecx+FMUSIC_MODULE.globalvolume-36],edx
		ret
	endif
endif
if SETGLOBALVOLUME_ON
S1_C16:
	cmp edx,64
	mov ecx,[ebp+var_mod]
	jle do_setglobalvol
	push 64
	pop edx
do_setglobalvol:
	mov [ecx+FMUSIC_MODULE.globalvolume-36],edx
	ret
endif
S3_C14:
if RETRIG_ON
	cmp cl,FMUSIC_XM_RETRIG
	je doeff_do_retrig
endif
if NOTECUT_ON
	sub cl,FMUSIC_XM_NOTECUT
	jz doeff_do_notecut
endif
if NOTEDELAY_ON
	if NOTECUT_ON
		dec cl
	else
		cmp cl,FMUSIC_XM_NOTEDELAY
	endif
	jne doeff_special_r
	mov ecx,[ebp+var_mod]
	xor eax,eax
	cmp [ecx+FMUSIC_MODULE.tick-36],ebx
	jne doeff_notectrl_z
	mov edx,[esp+4]
	mov DWORD PTR [esi+FMUSIC_CHANNEL.fadeoutvol-2],65536
	movzx edx,BYTE PTR [edx+FSOUND_SAMPLE.defvol]
	lea ecx,[eax+9]
	mov [esi+FMUSIC_CHANNEL.volume-2],edx
	mov edx,[esi+FMUSIC_CHANNEL.envvol-2]
	lea edi,[esi+FMUSIC_CHANNEL.envvoltick-2]
	cmp ecx,edx
	sbb edx,edx
	and edx,64
	mov [esi+FMUSIC_CHANNEL.envvol-2],edx
	; memset(&cptr->envvoltick,0,36)
	rep stosd
	; Retrigger tremolo and vibrato waveforms
	mov cl,[esi+FMUSIC_CHANNEL.wavecontrol-2]
	cmp cl,4Fh
	jge z2_tremolopos_ok
	mov BYTE PTR [esi+FMUSIC_CHANNEL.tremolopos-2],al ; = 0
z2_tremolopos_ok:
	test cl,0Ch
	jnz z2_vibpos_ok
	mov BYTE PTR [esi+FMUSIC_CHANNEL.vibpos-2],al ; = 0
z2_vibpos_ok:
	mov eax,[esi+FMUSIC_CHANNEL.period-2]
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOL_OR_FREQ_OR_TR
	mov [esi+FMUSIC_CHANNEL.freq-2],eax
if VOLUMEBYTE_ON
	mov eax,[ebp+doeff_current]
	movzx edx,BYTE PTR [eax+FMUSIC_NOTE.uvolume]
	; volume : EDX
	; cptr   : ESI
	jmp VolByte
else
	ret
endif
doeff_notectrl_z:
	mov BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],al
doeff_special_r:
endif
	ret
if NOTECUT_ON
doeff_do_notecut:
	mov ecx,[ebp+var_mod]
	cmp [ecx+FMUSIC_MODULE.tick-36],ebx
	jne doeff_notecut_ok
	and DWORD PTR [esi+FMUSIC_CHANNEL.volume-2],0
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
doeff_notecut_ok:
	ret
endif
if RETRIG_ON
doeff_do_retrig:
	test ebx,ebx
	jz doeff_retrig_ok
	mov eax,[ebp+var_mod]
	mov eax,[eax+FMUSIC_MODULE.tick-36]
	cdq
	idiv ebx
	test edx,edx
	jne doeff_retrig_ok
	or BYTE PTR [esi+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOL_OR_FREQ_OR_TR
doeff_retrig_ok:
	ret
endif

; DESCRIPTION: Process sample flags
DoFlags:
; cptr+2 - ESI
flags_iptr equ 16
flags_sptr equ 12
	mov ebx,esi
	push esi
	push edi
if INSTRUMENTVIBRATO_ON
	; THIS GETS ADDED TO PREVIOUS FREQDELTAS
	mov esi,[esp+flags_iptr]
	xor eax,eax
	movzx edx,BYTE PTR [esi+FMUSIC_INSTRUMENT.VIBtype]
	lea edi,[ebx+FMUSIC_CHANNEL.ivibpos-2]
	dec edx ; switch(iptr->VIBtype)
	js ivib_case_0
	mov al,128
	jz ivib_case_1
	dec edx
	jz ivib_case_2
	; case 3 / default
	mov dl,-128 ; -128 = 384 on a 1byte scope
	sub edx,[edi]
trim:
	movzx edx,dl
	sub eax,edx
	sar eax,1 ; delta = (128-((384-cptr->ivibpos)&0xFF))>>1 (case 3)
	          ; delta = (128-((cptr->ivibpos+128)&0xFF))>>1 (case 2)
	jmp ivib_end_switch
ivib_case_2:
	mov edx,[edi]
	sub edx,eax
	jmp trim
ivib_case_1:
	cmp [edi],eax
	sbb edx,edx
	and edx,eax
	lea eax,[edx-64] ; delta = +/- 64
	jmp ivib_end_switch
ivib_case_0:
	; delta = 64 sin(2 Pi x/256)
	mov ecx,[edi]
	mov edx,ecx
	and ecx,7Fh
	cmp ecx,65
	adc eax,-1
	xor ecx,eax
	and eax,129
	add ecx,eax
	shr edx,8
	movzx eax,BYTE PTR [OFFSET sin64+ecx]
	sbb edx,edx
	xor eax,edx
	sub eax,edx
ivib_end_switch:
	movzx edx,BYTE PTR [esi+FMUSIC_INSTRUMENT.iVIBdepth]
	imul edx,eax ; delta *= iptr->iVIBdepth
	movzx eax,BYTE PTR [esi+FMUSIC_INSTRUMENT.VIBsweep]
	test eax,eax
	jz sweep_ok
	push edi
	xchg eax,edi
	mov eax,[ebx+FMUSIC_CHANNEL.ivibsweeppos-2]
	imul edx
	idiv edi
	xchg eax,edx ; delta *= cptr->ivibsweeppos/iptr->VIBsweep
	xchg eax,edi ; iptr->VIBsweep
	pop edi
sweep_ok:
	sar edx,6
	add [ebx+FMUSIC_CHANNEL.freqdelta-2],edx ; cptr->freqdelta += delta>>6
	mov edx,[ebx+FMUSIC_CHANNEL.ivibsweeppos-2]
	inc edx
	cmp edx,eax
	jle sweeppos_ok
	xchg eax,edx
sweeppos_ok:
	movzx eax,BYTE PTR [esi+FMUSIC_INSTRUMENT.VIBrate]
	add eax,[edi] ; cptr->ivibpos += iptr->VIBrate
	mov [ebx+FMUSIC_CHANNEL.ivibsweeppos-2],edx
	neg ah
	or BYTE PTR [ebx+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
	neg ah
	sbb ah,0 ; ivibpos - 256
	mov [edi],eax
endif ; INSTRUMENTVIBRATO_ON
	mov esi,[ebx+FMUSIC_CHANNEL.cptr-2]
	test BYTE PTR [ebx+FMUSIC_CHANNEL.notectrl-2],FMUSIC_TRIGGER
	jz no_trig
	; TRIGGER: allocate a channel
	cmp DWORD PTR [esi+FSOUND_CHANNEL.fsptr],0
	jz no_swap
	; Swap between channels to avoid sounds cutting each other off and causing a click
	mov ecx,esi
	sub ecx,DWORD PTR [OFFSET _mod+FMUSIC_MODULE.Channels]
	shr ecx,7 ; /FSOUND_CHANNEL_size + 1
	sbb edi,edi
	and edi,-FSOUND_CHANNEL_size*2
	lea edi,[esi+edi+FSOUND_CHANNEL_size]
	push edi
	; Copy the whole channel except it's trailing data
	push (FSOUND_CHANNEL_size-20)/4
	mov [ebx+FMUSIC_CHANNEL.cptr-2],edi
	pop ecx
	rep movsd
	; This should cause the old channel to ramp out nicely
	mov [esi+FSOUND_CHANNEL.actualvolume-FSOUND_CHANNEL.fsptr],ecx
	mov [esi+FSOUND_CHANNEL.leftvolume-FSOUND_CHANNEL.fsptr],ecx
	mov [esi+FSOUND_CHANNEL.rightvolume-FSOUND_CHANNEL.fsptr],ecx
	pop esi
no_swap:
	lea edi,[esi+FSOUND_CHANNEL.fsptr]
	mov eax,[esp+flags_sptr]
	stosd ; fsptr
	; START THE SOUND!
	xor eax,eax
	mov edx,[esi+FSOUND_CHANNEL.fsampleoffset]
	stosd ; mixposlo
	stosd ; ramp_leftvolume
	stosd ; ramp_rightvolume
	stosd ; ramp_count, speeddir
	mov [esi+FSOUND_CHANNEL.fsampleoffset],eax
	mov [esi+FSOUND_CHANNEL.mixpos],edx
no_trig:
	mov eax,[ebx+FMUSIC_CHANNEL.volume-2]
	xor ecx,ecx
	cdq
	not edx
	and eax,edx
	cmp eax,64
	jle volume_le64
	lea eax,[ecx+64]
volume_le64:
	dec cl ; ecx <- 255
	mov [ebx+FMUSIC_CHANNEL.volume-2],eax
	test BYTE PTR [ebx+FMUSIC_CHANNEL.notectrl-2],FMUSIC_VOLUME
	jz no_volume
	add eax,[ebx+FMUSIC_CHANNEL.voldelta-2]          ; 6 bit  (64)
	imul DWORD PTR [ebx+FMUSIC_CHANNEL.envvol-2]     ; 6 bit  (64)
	imul DWORD PTR [_mod+FMUSIC_MODULE.globalvolume] ; 6 bit  (64)
	imul ecx ; eax *= 255
	imul DWORD PTR [ebx+FMUSIC_CHANNEL.fadeoutvol-2] ; 16 bit (65536)
	; eax:edx /= (2 * 64 * 64 * 64 * 65536)
	sar edx,3
	mov eax,[esi+FSOUND_CHANNEL.actualpan]
	mov [esi+FSOUND_CHANNEL.actualvolume],edx
	mov edi,edx
	imul edx
	idiv ecx
	mov [esi+FSOUND_CHANNEL.leftvolume],eax ; leftvolume <- volume*actualpan/255
	mov eax,ecx
	sub eax,[esi+FSOUND_CHANNEL.actualpan]
	imul edi
	idiv ecx
	mov [esi+FSOUND_CHANNEL.rightvolume],eax
no_volume:
	test BYTE PTR [ebx+FMUSIC_CHANNEL.notectrl-2],FMUSIC_PAN
	jz no_pan
	mov edi,128
	mov eax,[ebx+FMUSIC_CHANNEL.pan-2]
	sub eax,edi
	cdq
	xor eax,edx
	sub eax,edx
	sub edi,eax
	mov eax,[ebx+FMUSIC_CHANNEL.envpan-2]
	sar edi,5
	sub eax,32
	imul edi
	add eax,[ebx+FMUSIC_CHANNEL.pan-2]
	cdq
	not edx
	and eax,edx   ; if(pan < 0) pan = 0
	mov edi,[esi] ; actualvolume
	cmp eax,ecx
	mov edx,ecx
	xchg eax,edi
	jae pan_ae255
	imul edi
	idiv ecx
	mov edx,edi
pan_ae255:
	xchg eax,edx
	mov [esi+FSOUND_CHANNEL.actualpan],eax
	mov [esi+FSOUND_CHANNEL.leftvolume],edx
	not al ; 255 - pan
	imul DWORD PTR [esi] ; actualvolume
	idiv ecx
	mov [esi+FSOUND_CHANNEL.rightvolume],eax
no_pan:
	test BYTE PTR [ebx+FMUSIC_CHANNEL.notectrl-2],FMUSIC_FREQ
	jz no_freq
	mov ecx,[ebx+FMUSIC_CHANNEL.freq-2]
	xor edx,edx
	add ecx,[ebx+FMUSIC_CHANNEL.freqdelta-2]
	lea eax,[edx+40] ; f = 40 Hz
	jle freq_bounds_ok
	test BYTE PTR [_mod+FMUSIC_MODULE.flags],1
	jnz modflags_n1
	mov eax,0DA7790h
	div ecx
	xor edx,edx
	jmp freq_bounds_ok
modflags_n1:
	mov eax,4608
	sub eax,[ebx+FMUSIC_CHANNEL.freq-2]
	add eax,[ebx+FMUSIC_CHANNEL.freqdelta-2]
	push eax
	fild DWORD PTR [esp]
	fmul DWORD PTR [f0_0013]
	fld st0
	frndint
	fsub st1,st0
	fxch st1
	f2xm1
	fld1
	faddp st1,st0
	fscale
	fstp st1
	fmul DWORD PTR [f8363_0]
	fistp DWORD PTR [esp]
	pop eax ; freq = 625.271028*exp2((4608-period)/64)
freq_bounds_ok:
	mov ebx,FSOUND_MixRate
	div ebx
	mov [esi+FSOUND_CHANNEL.speedhi],eax
	div ebx
	mov [esi+FSOUND_CHANNEL.speedlo],eax
no_freq:
	pop edi
	pop esi
if INSTRUMENTVIBRATO_ON
	ret 8
else
	ret 4
endif