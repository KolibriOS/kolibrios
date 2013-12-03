SSMIX_CHANMODE_FREE   equ 0
SSMIX_CHANMODE_SINGLE equ 1
SSMIX_CHANMODE_REPEAT equ 2
SSMIX_CHANMODE_SINGLE_WITHOUT_RESET equ 3
SSMIX_CHANMODE_WAITRESET equ 4

include  'snd.inc'
include  'snd_const.inc'
include  'proc32.inc'
;include  'macros.inc'
;include  'debug.inc'

SSMIX_CHANNEL_COUNT equ 32

DEBUG equ 0 ; 0


; инициализация микшера
ssmix_init:
; выделить память для каналов микшера
    mov  eax, 68
    mov  ebx, 12
    mov  ecx, 4*8*SSMIX_CHANNEL_COUNT
    int  0x40
    mov  [ssmix_channels], eax
; выделить память для стека потока микшера
    mov  eax, 68
    mov  ebx, 12
    mov  ecx, 4096
    int  0x40
    mov  [ssmix_thread_stack], eax
; включить микшер
    mov  [ssmix_state], 1
; установить громкости стереоканалов
    mov  [ssmix_volumeL], 128
    mov  [ssmix_volumeR], 128
; создать поток
    mov  eax, 51
    mov  ebx, 1
    mov  ecx, ssmix_thread_proc
    mov  edx,[ssmix_thread_stack]
    add  edx,4092
    int 0x40

    ret



; завершение работы микшера
ssmix_release:
; выключить микшер
    mov  [ssmix_state], 0

    ret



; освобождение памяти
ssmix_free:
; освободить память каналов микшера
    mov  eax, 68
    mov  ebx, 13
    mov  ecx, [ssmix_channels]
    int  0x40
; освободить память стека потока микшера
    mov  eax, 68
    mov  ebx, 13
    mov  ecx, [ssmix_thread_stack]
    int  0x40

    ret

; установить громкость
ssmix_setvolume:  ; stdcall
		  ; LeftVolume: dword, RightVolume: dword

    push  eax
    mov   eax, [esp+8]	 ; громкость левого канала
    cmp   eax, 255
    jle   @f
    mov   eax, 255
  @@:
    cmp   eax, 0
    jge   @f
    mov   eax, 0
  @@:
    mov   [ssmix_volumeL], eax

    mov   eax, [esp+12]   ; громкость правого канала
    cmp   eax, 255
    jle   @f
    mov   eax, 255
  @@:
    cmp   eax, 0
    jge   @f
    mov   eax, 0
  @@:
    mov   [ssmix_volumeR], eax
    pop   eax

    ret


; воспроизвести трек
ssmix_playtrack:  ; stdcall
		  ; p_starttrack:dword, p_endtrack:dword, volumeL: dword, volumeR: dword; playmode:dword;
		  ; return eax - number of channel, if eax=-1 then no free channel

    push  ebx ecx
    mov   ebx, [ssmix_channels]
    mov   ecx, SSMIX_CHANNEL_COUNT
  .channel_loop:
    mov   eax, [ebx]
    cmp   eax, SSMIX_CHANMODE_FREE
    je	  .find_free_channel
    add   ebx, 32
    dec   ecx
    jnz   .channel_loop
    mov   eax, -1
    jmp   .end
  .find_free_channel:
    mov   eax, [esp+28]   ; режим воспроизведения
    mov   [ebx], eax
    mov   eax, [esp+20]   ; громкость левого канала
    imul  eax, [ssmix_volumeL]
    sar   eax, 7
    mov   [ebx+4], eax
    mov   eax, [esp+24]   ; громкость правого канала
    imul  eax, [ssmix_volumeR]
    sar   eax, 7
    mov   [ebx+8], eax
    mov   eax, [esp+12]   ; начало трека
    mov   [ebx+16], eax
    mov   [ebx+20], eax
    mov   eax, [esp+16]   ; конец трека
    mov   [ebx+24], eax
    mov   eax, SSMIX_CHANNEL_COUNT
    sub   eax, ecx
  .end:
    pop   ecx ebx

    ret   20


; остановить трек по номеру канала
ssmix_stoptrack:    ; stdcall
		    ; numchannel: dword
    push  eax ebx
    mov   ebx, [esp+12]
    cmp   ebx, 0
    jl	  .end
    cmp   ebx, SSMIX_CHANNEL_COUNT
    jge   .end
    shl   ebx, 5
    add   ebx, [ssmix_channels]
    mov   eax, SSMIX_CHANMODE_FREE
    mov   [ebx], eax
  .end:
    pop   ebx eax

    ret   4


; остановить все треки
ssmix_stopalltracks:

    push  eax ebx ecx
    mov   eax, SSMIX_CHANMODE_FREE
    mov   ebx, [ssmix_channels]
    mov   ecx, SSMIX_CHANNEL_COUNT
  .channel_loop:
    mov   [ebx], eax
    add   ebx, 32
    dec   ecx
    jnz   .channel_loop
    pop   ecx ebx eax

    ret


; поток микшера
ssmix_thread_proc:

    stdcall  _InitSound@4, ssmix_driver_version
    test eax, eax
    jz	@f
    if DEBUG
       print "ssmix. Cannot load sound driver"
    end if
    jmp  .end
  @@:

    mov  ecx, [ssmix_driver_version]
    cmp  cx, SOUND_VERSION
    jae  @f
    shr  ecx, 16
    cmp  ecx, SOUND_VERSION
    jbe  @f
    if DEBUG
       print "ssmix. Sound version mismatch"
    end if
    jmp  .end
  @@:

    stdcall _CreateBuffer@12, PCM_2_16_11+PCM_RING, 0, ssmix_hbuffer
    test eax, eax
    jz	@f
    if DEBUG
       print "ssmix. create buffer error"
    end if
    jmp  .end
  @@:

    stdcall _GetBufferSize@8, [ssmix_hbuffer], ssmix_buffer_size
    test eax, eax
    jz	@f
    if DEBUG
       print "ssmix. getbuffersize procedure error"
    end if
    jmp  .destroy_buffer
  @@:

    mov  ecx, [ssmix_buffer_size]
    shr  ecx, 1
    mov  [ssmix_buffer_size], ecx
    if DEBUG
	print "ssmix. buffer size"
	dph  ecx
	newline
    end if

    ; выделить память для буффера воспроизведения
    mov  eax, 68
    mov  ebx, 12
    mov  ecx, [ssmix_buffer_size]
    int  0x40
    test eax, eax
    jnz  @f
    if DEBUG
       print "ssmix. malloc memory error"
    end if
    jmp  .destroy_buffer
  @@: 
    mov  [ssmix_pbuffer], eax

    stdcall _PlayBuffer@8, [ssmix_hbuffer], 0
    test eax, eax
    jz	 @f
    if DEBUG
       print "ssmix. playbuffer error"
    end if
    jmp  .free_buffer_memory
  @@:

; цикл воспроизведения
  .mixloop:
; получить данные события от драйвера
    mov  eax, 68
    mov  ebx, 14
    mov  ecx, ssmix_driver_notify_struct
    int  0x40
; проверка на code
    mov  eax, [ssmix_driver_notify_struct]
    cmp  eax, 0xFF000001
    jne  .mixloop
; проверка на stream
    mov  eax, [ssmix_driver_notify_struct+8]
    cmp  eax, [ssmix_hbuffer]
    jne  .mixloop
; получить смещение в буффере
    mov  eax, [ssmix_driver_notify_struct+12]
    mov  [ssmix_buffer_offset], eax
; цикл заполнения буффера
    mov  ebx, [ssmix_pbuffer]
    mov  ecx, [ssmix_buffer_size]
    shr  ecx, 2
  .fill_buffer_loop:
    push  ebx ecx
; цикл смешивания каналов
    mov   esi, 0    ; сумма для левого стереоканала
    mov   edi, 0    ; сумма для правого стереоканала
    mov   edx, [ssmix_channels]
    mov   ecx, SSMIX_CHANNEL_COUNT
  .channel_loop:
    mov   eax, [edx]
    cmp   eax, SSMIX_CHANMODE_FREE
    je	  .channel_end
    cmp   eax, SSMIX_CHANMODE_WAITRESET
    je	  .channel_end
    push  ecx
; смешивание каналов с учетом громкости
    mov   ebx, [edx+4]
    mov   ecx, [edx+16]
    movsx eax, word [ecx]
    imul  eax, ebx
    add   esi, eax
    mov   ebx, [edx+8]
    movsx eax, word [ecx+2]
    imul  eax, ebx
    add   edi, eax
; проверка на завершение трека
    add   ecx, 4
    cmp   ecx, [edx+24]
    jb	  .notrackend
    mov   eax, [edx]
    cmp   eax, SSMIX_CHANMODE_SINGLE
    jne    @f
    mov   ebx, SSMIX_CHANMODE_FREE
    mov   [edx], ebx
    jmp   .notrackend
  @@:
    cmp   eax, SSMIX_CHANMODE_REPEAT
    jne    @f
    mov   ecx, [edx+20]
    jmp   .notrackend
  @@:
    cmp   eax, SSMIX_CHANMODE_SINGLE_WITHOUT_RESET
    jne    @f
    mov   ebx, SSMIX_CHANMODE_WAITRESET
    mov   [edx], ebx
    jmp   .notrackend
  @@:
  .notrackend:
    mov   [edx+16], ecx
    pop   ecx
  .channel_end:
    add   edx, 32
    dec   ecx
    jnz   .channel_loop
; нормализация стереоканалов
    sar   esi, 7
    cmp   esi, 0x7FFF
    jle   @f
    mov   esi, 0x7FFF
  @@:
    cmp   esi, -0x8000
    jge   @f
    mov   esi, -0x8000
  @@:
    sar   edi, 7
    cmp   edi, 0x7FFF
    jle   @f
    mov   edi, 0x7FFF
  @@:
    cmp   edi, -0x8000
    jge   @f
    mov   edi, -0x8000
  @@:
; сохранение результата в буффере микшера
    shl   edi, 16
    add   esi, edi
    pop   ecx ebx
    mov  [ebx], esi
    add  ebx, 4
    dec  ecx
    jnz  .fill_buffer_loop
; установка буффера драйвера
    stdcall _SetBuffer@16, [ssmix_hbuffer], [ssmix_pbuffer], [ssmix_buffer_offset], [ssmix_buffer_size]
    cmp  [ssmix_state], 1
    je	 .mixloop

; освободить память буффера воспроизведения
  .free_buffer_memory:
    mov  eax, 68
    mov  ebx, 13
    mov  ecx, [ssmix_pbuffer]
; уничтожить буффер драйвера
  .destroy_buffer:
    stdcall _DestroyBuffer@4, [ssmix_hbuffer]

; завершить работу потока
  .end:
    call  ssmix_free
    mov   eax, -1
    int   0x40

    ;ret


;=================================================================================

align 4
_InitSound@4:	    ;p_ver:dword

	   push ebx
	   push ecx

	   mov eax, 68
	   mov ebx, 16
	   mov ecx, ssmix_szInfinity
	   int 0x40
	   mov [ssmix_hSound], eax
	   test eax, eax
	   jz .fail

	   mov eax, 68
	   mov ebx, 16
	   mov ecx, ssmix_szSound
	   int 0x40
	   mov [ssmix_hrdwSound], eax

	   lea eax, [esp+12]   ;p_ver
	   xor ebx, ebx

	   push 4	       ;.out_size
	   push eax	       ;.output
	   push ebx	       ;.inp_size
	   push ebx	       ;.input
	   push SRV_GETVERSION ;.code
	   push [ssmix_hSound]	     ;.handle

	   mov eax, 68
	   mov ebx, 17
	   mov ecx, esp        ;[handle]
	   int 0x40
	   add esp, 24
	   pop ecx  
	   pop ebx
	   ret 4
.fail:
	   or eax, -1 
	   pop ecx  
	   pop ebx
	   ret 4


align 4
_CreateBuffer@12:   ;format:dword,size:dword,p_str:dword

	   push ebx
	   push ecx
	   lea eax, [esp+20]   ;p_str
	   lea ebx, [esp+12]   ;format

	   push 4	       ;.out_size
	   push eax	       ;.output
	   push 8	       ;.inp_size
	   push ebx	       ;.input
	   push SND_CREATE_BUFF;.code
	   push [ssmix_hSound]	     ;.handle

	   mov eax, 68
	   mov ebx, 17
	   mov ecx, esp
	   int 0x40
	   add esp, 24	     ;io_cintrol 
	   pop ecx
	   pop ebx
	   ret 12

align 4
_DestroyBuffer@4:     ;str:dword

	   push ebx
	   push ecx

	   xor eax, eax
	   lea ebx, [esp+12]   ;[stream]

	   push eax	       ;.out_size
	   push eax	       ;.output
	   push 4	       ;.inp_size
	   push ebx	       ;.input
	   push SND_DESTROY_BUFF;.code
	   push [ssmix_hSound]	     ;.handle

	   mov eax, 68
	   mov ebx, 17
	   mov ecx, esp        ;[handle]
	   int 0x40
	   add esp, 24
	   pop ecx 
	   pop ebx
	   ret 4

align 4
_GetBufferSize@8:      ;str:dword, p_size:dword
	   
	   push ebx
	   push ecx
	   
	   lea eax, [esp+16]
	   lea ebx, [esp+12]   ;[stream]

	   push 4	       ;.out_size
	   push eax	       ;.output
	   push 4	       ;.inp_size
	   push ebx	       ;.input
	   push SND_GETBUFFSIZE;.code
	   push dword [ssmix_hSound] ;.handle

	   mov eax, 68
	   mov ebx, 17
	   mov ecx, esp
	   int 0x40
	   add esp, 24
	   pop ecx 
	   pop ebx
	   ret 8

align 4
_SetBuffer@16:	 ;str:dword, src:dword, offs:dword, size:dword

	   push ebx
	   push ecx
	   
	   xor eax, eax
	   lea ebx, [esp+12]   ;[stream]

	   push eax	       ;.out_size
	   push eax	       ;.output
	   push 16	       ;.inp_size
	   push ebx	       ;.input
	   push SND_SETBUFF    ;.code
	   push dword [ssmix_hSound] ;.handle

	   mov eax, 68
	   mov ebx, 17
	   mov ecx, esp
	   int 0x40
	   add esp, 24
	   pop ecx 
	   pop ebx
	   ret 16
	   
align 4
_PlayBuffer@8:		;str:dword,flags:dword

	   push ebx
	   push ecx

	   xor eax, eax
	   lea ebx, [esp+12]   ;[stream]

	   push eax	       ;.out_size
	   push eax	       ;.output
	   push 8	       ;.inp_size
	   push ebx	       ;.input
	   push SND_PLAY       ;.code
	   push dword [ssmix_hSound] ;.handle

	   mov eax, 68
	   mov ebx, 17
	   mov ecx, esp
	   int 0x40
	   add esp, 24
	   pop ecx 
	   pop ebx
	   ret 8



; ========================= data  =====================================

ssmix_hSound	  dd ?
ssmix_hrdwSound   dd ? 

ssmix_szInfinity  db 'INFINITY',0
ssmix_szSound	  db 'SOUND',0

ssmix_channels dd 0
ssmix_thread_stack dd 0
ssmix_state dd 0
ssmix_volumeL dd ?
ssmix_volumeR dd ?
ssmix_driver_version dd ?
ssmix_hbuffer dd ?
ssmix_pbuffer dd 0
ssmix_buffer_size dd ?
ssmix_buffer_offset dd ?
ssmix_driver_notify_struct rd 6   ; сткуктура события от драйвера