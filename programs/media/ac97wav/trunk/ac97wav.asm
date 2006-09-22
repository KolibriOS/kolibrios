;---------------------------------------------------------------------
;
;   MenuetOS AC97 WAV Player
;
;    0.03  November 10, 2004  doesn't halt if file not found
;    0.04  November 11, 2004  better positioning (with mouse)
;    0.05  November 14, 2004  internals clean up
;                             fixed cutting sound at the edges
;    0.06  November 17, 2004  fixed many bugs
;    0.07  Nov      20, 2004  deactivates text box when 'play' pressed
;                             stops playing before closing a window
;    0.08  Nov      24, 2004  added support for 8bit and mono modes
;                             +variable rate for some chipsets
;    0.09  August   26, 2006  modified to use function 70
;
;   Use [flat assembler 1.64] to compile.
;
;---------------------------------------------------------------------

  use32 	 ; turn on 32 bit mode
  org	  0x0	      ; the program is placed at 0 offset

  db	 'MENUET01'	; 8-byte identifier of MenuetOS application
  dd	 0x01	      ; header version (always 1)
  dd	 START	       ; address of the beginning of the code
  dd	 IMAGE_END     ; size of the program's image
  dd	 MEMORY_END	; how much memory does it need
  dd	 STACK_P     ; a pointer to the top of the stack
  dd	 textbox_string
;  dd     0x0             ; address of buffer for parameters (not used)
  dd	 0x0	     ; reserved

;---------------------------------------------------------------------

include "MACROS.INC"	 ; standart macros & constants
include "MEOSFUNC.INC"	   ; MenuetOS API functions names
include "DEBUG.INC"	; printing to debug board
include "CONSTANT.INC"	   ; BIT?? constants
include "AC97.INC"     ; AC'97 constants
include "PCI.INC"     ; PCI interface
include "CODEC.INC"	; functions for configuring codec
include "FRONTEND.INC"	   ; main window

;---------------------------------------------------------------------

;  Uncomment these strings if you don't want to receive debug messages:

;  macro dps str   {}    ; dps prints a string without CRLF
;  macro dpd num   {}    ; prints unsigned decimal number
;  macro pregs     {}    ; outputs EAX, EBX, ECX, EDX
;  macro newline   {}    ; CRLF
;  macro print str {}    ; output a string with CRLF
;  macro dph arg   {}    ; print hex number

;---------------------------------------------------------------------

;macro device id, addr { dd id, addr }
macro devices [id, str]
{
  common
    label supported_devices dword
  forward
    local string
    dd id
    dd string
  forward
    string db str
    db 0
}


devices \
  (ICH_DID shl 16) + INTEL_VID,      "ICH"    ,\
  (ICH0_DID shl 16) + INTEL_VID,     "ICH0"   ,\
  (ICH2_DID shl 16) + INTEL_VID,     "ICH2"   ,\
  (ICH3_DID shl 16) + INTEL_VID,     "ICH2"   ,\
  (ICH4_DID shl 16) + INTEL_VID,     "ICH4"   ,\
  (ICH5_DID shl 16) + INTEL_VID,     "ICH5"   ,\
  (MX440_DID shl 16) + INTEL_VID,    "440MX"  ,\
  (SI7012_DID shl 16) + SIS_VID,     "SI7012" ,\
  (NFORCE_DID shl 16) + NVIDIA_VID,  "NForce" ,\
  (NFORCE2_DID shl 16) + NVIDIA_VID, "NForce2",\
  (AMD8111_DID shl 16) + AMD_VID,    "AMD8111",\
  (AMD768_DID shl 16) + AMD_VID,     "AMD768"
dd    0


;---------------------------------------------------------------------
;---  MAIN PROGRAM  --------------------------------------------------
;---------------------------------------------------------------------

START:

; Print PCI version (for example, 2.16)
;        mcall   MF_PCI, 0
;        mov     bl, al
;        movzx   eax, ah
;        dps    "PCI version: "
;        dpd     eax
;        movzx   eax, bl
;        dpd     eax
;        newline

; Check PCI access mechanism (must be 1 or 2)
    mcall    MF_PCI, 2
    dec    al
    cmp    al, 1
    jna    @f
    print    "Error: cannot access PCI bus."
    jmp    exit
;        dps     "PCI returned "
;        movzx   eax, al
;        dpd     eax
;        newline
     @@:


; Get last bus & then check all buses & devices
    mcall    MF_PCI, 1
    mov    [lastbus], al

     ; looking for a compatible device
    mov    [bus], -1
  .next_bus:
    inc    [bus]

    mov    al, [lastbus]
    cmp    al, [bus]
    jb	  .device_not_found

    mov    [devfn], 0
  .next_devfn:

    mov    cl, 0
    call    pciRegRead32

    mov    edi, supported_devices
      @@:
    mov    ebx, [edi]
    test    ebx, ebx
    jz	  @f
    cmp    eax, ebx
    jnz    .skip
    add    edi, 4
    mov    [device_id], eax
    mov    edx, [edi]
    call    debug_outstr
    jmp    proceed
       .skip:
    add    edi, 8
    jmp    @b
      @@:

    inc    [devfn]
    cmp    [devfn], 255
    jb	  .next_devfn

    jmp    .next_bus


  .device_not_found:
    print    "Could not find Intel AC'97 compatible codec!"
    print    "1) Check if it's enabled in BIOS."
    print    "2) Check if your device is included in the device list."
    jmp    exit


 proceed:
    print    " integrated AC97 audio codec detected."
    mov    eax, [device_id]
    cmp    eax, (ICH4_DID shl 16) + INTEL_VID
    je	  .newich
    cmp    eax, (ICH5_DID shl 16) + INTEL_VID
    jne    .nonewich
      .newich:
    mov    [AC97ICH4], 1
      .nonewich:

    cmp    eax, (SI7012_DID shl 16) + SIS_VID
    jne    @f
    mov    [SI7012], 1
      @@:

;---------------------------------------------------------------------

; Get NAMBAR register base port address & save it
    mov    cl, NAMBAR_REG
    call    pciRegRead16

    and    eax, 0xFFFE
    mov    [NAMBAR], ax
    test    eax, eax
    jnz    .mixer_base_ok

    print    "Error: Intel ICH based AC97 audio codec disabled in BIOS!"
    jmp    exit

 .mixer_base_ok:
    dps    "NAMBAR: "
    dph    eax

; Get NABMBAR & save it
    mov    cl, NABMBAR_REG
    call    pciRegRead16
    and    eax, 0xFFC0
    mov    [NABMBAR], ax
    test    eax, eax
    jnz    .bm_base_ok

    print    "Error: Intel ICH based AC97 audio codec disabled in BIOS!"
    jmp    exit

 .bm_base_ok:
    dps    " NABMBAR: "
    dph    eax
    newline

;---------------------------------------------------------------------

; Get IRQ (not used)
    mov    cl, IRQ_REG
    call    pciRegRead8
    mov    [AC97IRQ], al

; Get Interrupt pin (not used)
    mov    cl, INT_REG
    call    pciRegRead8
    mov    [AC97INT], al

; AC97ICH4 should work then...
    cmp    [AC97ICH4], 1
    jne    .skip_ich4_init

    mov    cl, ICH4_CFG_REG  ; 0x41
    call    pciRegRead8
    or	  al, 0x1
    mov    dl, al
    call    pciRegWrite8

    mov    cl, 0x54
    call    pciRegRead16
    and    eax, 0xFFFF
    dps    "Power Control & Status: "
    dph    eax
    newline
 .skip_ich4_init:

;---------------------------------------------------------------------

    mov    cl, PCI_CMD_REG
    call    pciRegRead16	   ; read PCI command register
    mov    dx, ax
    or	  dx, IO_ENA+BM_ENA+BIT10    ; enable IO and bus master + disable
		       ;  interrupts
    call    pciRegWrite16

;---------------------------------------------------------------------

    print    "Enabling access to ports..."

    movzx    ecx, [NAMBAR]
    mov    edx, ecx
    add    edx, NAM_SIZE
    mcall    MF_PORTS, PRT_RESERVE
    test    eax, eax
    jz	  @f
    print    "Error: couldn't enable access to ports"
    jmp    exit
     @@:

    movzx    ecx, [NABMBAR]
    mov    edx, ecx
    add    edx, NABM_SIZE
    mcall    MF_PORTS, PRT_RESERVE
    test    eax, eax
    jz	  @f
    print    "Error: couldn't enable access to ports"
    jmp    exit
     @@:

;---------------------------------------------------------------------

; setup the Codec
    mov    eax, 48000
    call    codecConfig 	 ; unmute codec, set rates.
    test    eax, eax
    jnz    @f
    print    "Error: cannot initialize AC97 device."
    jmp    fpexit
       @@:

    print    "Congrutalations! Your device has been initialized properly!"
    call    print_info

;---------------------------------------------------------------------

; register reset the DMA engine.
    mov    edx, PO_CR_REG    ; PCM out control register
    mov    al, RR	 ; reset
    call    NABMBAR_write_byte
    
;start fix for MM (1)
    mcall  MF_INTERNAL_SERVICES,ALLOC_PHYS_MEM,120*1024
    test   eax,eax
    jz     no_phys_buffers             ;not enough memory
    mov    [phys_wav_buffer1],eax
    add    eax,60*1024
    mov    [phys_wav_buffer2],eax
    mcall  MF_INTERNAL_SERVICES,ALLOC_PHYS_MEM,32*8
    test   eax,eax
    jnz    @f
    mcall  MF_INTERNAL_SERVICES,FREE_PHYS_MEM,[phys_wav_buffer1]
    jmp    no_phys_buffers
@@:
    mov    [phys_bdl_buffer],eax
;end fix for MM (1)

; create Buffer Descriptors List
    call    prepare_BDL

; open player's window
    mcall    MF_THREAD, THR_CREATE, thread, thread_stack

; wait for command
  .new_check:
    cmp    [status], ST_PLAY
    jne    @f
    call    play
      @@:
    cmp    [status], ST_STOP
    jne    @f
    call    stop
      @@:
    cmp    [status], ST_EXIT
    je	  stopexit

    mcall    MF_DELAY, 10
    jmp    .new_check

stopexit:
    call    stop

fpexit:

; free ports
    movzx    ecx, [NAMBAR]
    mov    edx, ecx
    add    edx, NAM_SIZE
    mcall    MF_PORTS, PRT_FREE

    movzx    ecx, [NABMBAR]
    mov    edx, ecx
    add    edx, NABM_SIZE
    mcall    MF_PORTS, PRT_FREE

;---------------------------------------------------------------------
;start fix for MM (2)
    mcall  MF_INTERNAL_SERVICES,FREE_PHYS_MEM,[phys_bdl_buffer]
    mcall  MF_INTERNAL_SERVICES,FREE_PHYS_MEM,[phys_wav_buffer1]
;end fix for MM (2)
exit:
    mcall    MF_EXIT
no_phys_buffers:
    print  "allocation of physical buffers failed"
    jmp    exit

;---------------------------------------------------------------------
;---  FUNCTIONS  -----------------------------------------------------
;---------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; prepare_BDL - initializes BUFFER DESCRIPTORS LIST
prepare_BDL:
    mov    ecx, 32 / 2	  ; make 32 entries in BDL
    mov    edi, BDL_BUFFER
;    call   get_my_address
    mov    ebx, 30*1024
    cmp    [SI7012], 1
    jne    @f
    add    ebx, ebx
@@:
    ; set buf. desc. 0 to start of data file in memory
    push   eax
;    add    eax, WAV_BUFFER1
;start fix for MM (6)
    mov    eax,[phys_wav_buffer1]
;end fix for MM (6)    
    stosd
    ; set length to 60k samples. 1 sample is 16 bit or 2 bytes.
    mov    eax, ebx ;60*1024   ; number of samples
    or	   eax, BUP
    stosd

    mov    eax, [esp]
;    add    eax, WAV_BUFFER2
;start fix for MM (7)
    mov    eax,[phys_wav_buffer2]
;end fix for MM (7)
    stosd
    mov    eax, ebx ;60*1024
    or	   eax, BUP
    stosd

    pop    eax
    loop   @b
        

    ; tell the DMA engine where to find our list of Buffer Descriptors.
    ; eax = base addr!
;start fix for MM (3)
  ;copy to physical memory
    mcall  MF_INTERNAL_SERVICES,SET_PHYS_BUFFER,[phys_bdl_buffer],BDL_BUFFER,32*8 
  ;physical address of bdl    
    mov    eax,[phys_bdl_buffer]                                
;end fix for MM (3)    
    mov    edx, PO_BDBAR_REG
;    add    eax, BDL_BUFFER
    call   NABMBAR_write_dword

ret


;---------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; stop - stops current music
;; in:  nothing
;; out: nothing
stop:
;        print   "STOP!"
    push    eax edx

    mcall    MF_DELAY, 10
    mov    edx, PO_CR_REG
    mov    al, 0
    call    NABMBAR_write_byte
    cmp    [status], ST_STOP
    jne    .exit
    mov    [status], ST_DONE
      .exit:

    pop    edx eax
ret

;---------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; play - plays wav file!
;; in:  nothing
;; out: nothing (but sound :)   !corrupts registers!
play:
    ; at first, reset file and get file size
        mcall   MF_SYSTREE, attrinfo
        test    eax, eax
        jnz     .notfound
        mov     eax, [fileattr+32]
        mov     [file_size], eax
    mov    [fileinfo.first_byte], 0
    mcall    MF_SYSTREE, fileinfo ; load a block, returns error code in eax
		     ;   and size of the file in ebx
    test    eax, eax	     ; 0 - successful
    jz	  @f
        cmp     eax, 6          ; 6 = eof - successful too
        jz      @f
.notfound:
    print    "AC97: File not found!"
    mov    [status], ST_STOP
    jmp    .exit
     @@:

    mov    al, [LOAD_BUFFER+32] ; bytes per sample
    dec    al
    jz	  @f
    cmp    al, 3
    je	  @f
    sub    al, [LOAD_BUFFER+22] ; channels
    add    al, 2
      @@:
    mov    [wav_mode], al

    pusha
    movzx    ebx,word [LOAD_BUFFER+24]
    mov   eax,48000
    xor   edx,edx
    div   ebx
    mov   [difference_of_frequency],al
;        dph   eax
    mov   ecx,edx
    imul  eax,ecx,10
    xor   edx,edx
    div   ebx
    mov   ecx,edx
    imul  ecx,10
    push  eax
    mov   eax,ecx
    xor   edx,edx
    div   ebx
;        dph   eax
    cmp   eax,5
    jl	  .temp_15
    pop   eax
;        dph   eax

    inc   eax
    jmp   .temp_16
     .temp_15:
    pop   eax
     .temp_16:
    mov   [difference_of_frequency_1],al
;        dph   eax
    xor   edx,edx
    movzx  ebx,[difference_of_frequency]
    imul   ebx,10
    add    bl,[difference_of_frequency_1]
    mov   [difference_of_frequency_2],bl
;        dph   ebx
    popa

    movzx    eax, word [LOAD_BUFFER+24]
    ;dps "Freq: "
    ;dpd eax
    ;newline
    call    set_sample_rate


    ; change the last_valid_index to the (current_index-1)
    ; the LVI register tells the DMA engine where to stop playing
    call    updateLVI

    ; if current index is odd, load buffer 1 then 0, jump to tuneLoop
    ; if it is even, buffers 0 then 1; tuneLoop1
    call    getCurrentIndex
    and    eax, BIT0

    mov    esi, eax
    push    eax
    call    update_next_buffer
    pop    eax
    xor    eax, 1
    call    update_next_buffer

    ; start playing!
    mov    edx, PO_CR_REG
    mov    al, RPBM
    call    NABMBAR_write_byte

    jmp    [jumpto+esi*4]


   .tuneLoop:
    ; wait while the current_index is even
    @@:
;        dps     "a"
    mcall    MF_DELAY, 7
    call    getCurrentIndex
    test    al, BIT0
    jz	  @b	   ; loop if not ready yet
;        print   "fa"

    call    updateLVI

    mov    eax, 0
    call    update_next_buffer
    test    al, al
    jnz    .exit_wait

    cmp    [status], ST_PLAY
    jne    .exit

    test    [volume], 0x10000000 ; test volume_changed bit
    je	  @f
    mov    al, byte [volume]
    call    setVolume
    and    [volume], 0x0FFFFFFF ; clear vloume_changed bit
     @@:

     .tuneLoop1:
     @@:
;        dps     "b"
    mcall    MF_DELAY, 7
    call    getCurrentIndex
    test    al, BIT0
    jnz    @b	    ; loop if not ready yet
;        print   "fb"

    cmp    [status], ST_PLAY
    jne    .exit

    call    updateLVI

    mov    eax, 1
    call    update_next_buffer
    test    al, al
    jnz    .exit_wait

    jmp    .tuneLoop
   .exit_wait:
    mcall    MF_DELAY, 30 ; a little pause - let the player finish
   .exit:
ret
attempts db 0

buffers dd WAV_BUFFER1, WAV_BUFFER2
jumpto	  dd play.tuneLoop, play.tuneLoop1


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; update_first_buffer - load a chunk into the first buffer, increments offset
;; in:  eax = number - 0 or 1
;; out: error code, 0 - successful
update_next_buffer:
    push   esi edi

    movzx  edx, byte [wav_mode]
    mov    ecx, [blocks + edx * 4]
    mov    [fileinfo.bytes], ecx

    mov    esi, LOAD_BUFFER
    mov    edi, [buffers+eax*4]
    push   eax              ;save buffer index
  start_attempts:
    mcall  MF_SYSTREE, fileinfo
    test eax, eax
    jz @f
        cmp     eax, 6
        jz      @f
    cmp   [attempts],100
    je	 @f
    inc   [attempts]
    jmp   start_attempts
;    dpd eax
;    newline
;    dpd [fileinfo.first_block]
;    newline
       @@:
;        print  " loaded!"

    push   eax ebx edx
    mov    eax,ecx
    xor    edx,edx
    imul   eax,10
    movzx     ebx,[difference_of_frequency_2]

    div    ebx
    mov    ecx,eax

;    mov   ebx,10
;    mov   eax,edx
;    xor   edx,edx
;    div   ebx
;   cmp   edx,5
;    jb   temp_12_7
;    inc   ecx
;  temp_12_7:
;   cmp	 edx,0
;    je	 temp_12_7
;    inc   ecx
;  temp_12_7:

    pop    edx ebx 
    mov    eax,[esp+4]            ;restore buffer index
    and    ecx, not 511
    add    [fileinfo.first_byte], ecx ; +60Kb
    call   [convert + edx * 4]
;start fix for MM (4)    
    mov    eax,[esp+4]            ;restore buffer index
    test   eax,not 1
    jz     .ok
    print  "buffer index out of range"
    dpd    eax
    jmp    .ret
  .ok:
    push   ebp
    mov    ebp,[phys_wav_buffer1+eax*4]
    mov    edi,[buffers+eax*4]
    mcall  MF_INTERNAL_SERVICES,SET_PHYS_BUFFER,ebp,edi,60*1024
    pop    ebp
.ret:
    pop    eax
    add    esp,4                  ;pop buffer index    
;end fix for MM (4)    

    pop    edi esi
ret

c8mono:
    mov   [type_of_conversion],1
    jmp   for_all_type

c8mono_1:
    lodsb
    call  c8mono_2
    push  ax
    shl   eax,16
    pop   ax
    push  eax
    mov al,[esi]
    call  c8mono_2
    push  ax
    shl   eax,16
    pop   ax
    mov  ebx,eax
    pop  eax
    jmp  for_all_type_1

c8mono_2:
    sub    al, 0x80
    cbw
    imul   ax, 255
    ret

c8stereo:
    mov   [type_of_conversion],2
    jmp   for_all_type

c8stereo_1:
    lodsb
    call  c8stereo_2
    shl eax,16
    lodsb
    call  c8stereo_2
    push  eax
    mov al,[esi]
    call  c8stereo_2
    shl eax,16
    mov al,[esi+1]
    call  c8stereo_2
    mov   ebx,eax
    pop  eax
    jmp   for_all_type_1

c8stereo_2:
    sub    al, 0x80
    cbw
    imul   ax, 255
    ret

c16mono:
    mov   [type_of_conversion],3
    jmp   for_all_type

c16mono_1:
	lodsw
	push  ax
	shl eax,16
	pop   ax
	mov bx,[esi]
	shl ebx,16
	mov bx,[esi]
	jmp  for_all_type_1

c16stereo:
 for_all_type:
    xor   edx,edx
    mov    eax, 15*1024*10
    movzx     ebx,[difference_of_frequency_2]
    xor    edx,edx
    div    ebx
    mov    ecx,eax

;    mov   ebx,10
;    mov   eax,edx
;    xor   edx,edx
;    div   ebx
;    cmp   edx,5
;    jb   temp_12_6
;    inc   ecx
;  temp_12_6:
   cmp	 edx,0
    je	 temp_12_6
    inc   ecx
  temp_12_6:

  c16stereo_1:
    mov  [znak],0

    cmp  [type_of_conversion],1
    je	 c8mono_1
    cmp  [type_of_conversion],2
    je	 c8stereo_1
    cmp  [type_of_conversion],3
    je	 c16mono_1
    lodsd

    mov ebx,[esi]
for_all_type_1:
    cmp eax,ebx
    jne  c16stereo_2
    inc  [znak]
    c16stereo_2:
    push eax
    push ecx
    sub  eax,ebx
    push eax
    shl  eax,16
    movzx  ebx,[difference_of_frequency]
    inc   ebx
    xor  edx,edx
    div  ebx
    shr  eax,16
    mov  ecx,eax
    pop  eax
    xor  ax,ax
    xor  edx,edx
    div  ebx
    shl  eax,16
    mov  cx,ax
    mov  ebx,ecx
    pop  ecx
    pop   eax
    mov    dl,[difference_of_frequency]
    inc   dl
    @@:
temp_12:
    cmp   [difference_of_frequency_1],0
    je	  temp_12_3
    cmp   [difference_of_frequency_1],5
    jne   temp_12_4
    cmp  [difference_of_frequency_4],2
    jne  temp_12_3
    jmp  temp_12_5
 temp_12_4:
    cmp  [difference_of_frequency_4],10
    jne  temp_12_3

 temp_12_5:

    cmp  [znak],0
    jne   temp_12_5_1
    sub  eax,ebx
    jmp  temp_12_5_2
 temp_12_5_1:
    add  eax,ebx
 temp_12_5_2:


    stosd
    inc  [schetchik]
    mov    [difference_of_frequency_4],0
 temp_12_3:
    cmp  [znak],0
    jne   temp_13
    sub  eax,ebx
    jmp  temp_14
 temp_13:
    add  eax,ebx

 temp_14:
    cld
    dec    dl
    jz	  temp_14_1
    stosd
    inc   [schetchik]
    inc   [difference_of_frequency_4]
    jmp   temp_12
 temp_14_1:
    dec    ecx
    cmp   ecx,0
;    jnz    c16stereo_1
    jg	  c16stereo_1
    newline
    dph [schetchik]
 temp_14_2:
    cmp   [schetchik],15360
    jge   temp_14_3
    stosd
    inc  [schetchik]
    jmp  temp_14_2

  temp_14_3:
    newline
    dph [schetchik]
    cmp   [schetchik],15360
    je	  temp_14_4
;    mov  [edi-4],dword 0
    sub   edi,4
;    sub   esi,4
 temp_14_4:
    mov   [schetchik],0
    ret


difference_of_frequency db 0
difference_of_frequency_1 db 0
difference_of_frequency_2 db 0
difference_of_frequency_4 db 0
schetchik dd 0
znak db 0
type_of_conversion db 0

convert dd c8mono, c8stereo, c16mono, c16stereo
blocks	  dd 30*512, 60*512, 60*512, 120*512


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; get_my_address - get base address of the program in physical memory
;;   in:  nothing
;;   out: eax = address
;start fix for MM (8)
;function shouldn't used.
;get_my_address:
;    pushad
;    mcall    MF_PROCINFO, procinfo, PN_MYSELF
;    popad
;    mov    eax, [procinfo.memory_start]
;ret
;end fix for MM (8)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; set the last valid index to something other than we're currently playing
;; so that we never end
;;
;; this routine just sets the last valid index to 1 less than the index
;; that we're currently playing, thus keeping it in and endless loop
;; input:  none
;; output: none
updateLVI:
    push    eax
    call    getCurrentIndex
     ; dps "index "
     ; dpd eax
     ; newline
    dec    al
    and    al, INDEX_MASK
    call    setLastValidIndex
    pop    eax
ret

;---------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; returns AL = current index value
getCurrentIndex:
    push    edx
    mov    edx, PO_CIV_REG
    call    NABMBAR_read_byte
    pop    edx
ret

;---------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; input AL = index # to stop on
setLastValidIndex:
    push    edx
    mov    edx, PO_LVI_REG
    call    NABMBAR_write_byte
    pop    edx
ret

;---------------------------------------------------------------------

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; print_info - outputs debug information
;;  in:  nothing
;;  out: nothing
print_info:
    dps    "BUS: "
    movzx    eax, [bus]
    dph    eax

    dps    " DEVFN: "
    movzx    eax, [devfn]
    dph    eax

    dps    " IRQ: "
    movzx    eax, [AC97IRQ]
    dpd    eax
    newline


    dps    "CODEC_POWER_CTRL: "
    mov    edx, CODEC_POWER_CTRL_REG
    call    NAMBAR_read_word
    dph    eax
    dps    " (bits 0-3 should be set)"
    newline

    mov    edx, 0x28
    call    NAMBAR_read_word
    dph    eax
    dps    " - supported features"
    newline
    mov    edx, 0x2A
    call    NAMBAR_read_word
    dph    eax
    dps    " - config"
    newline
    mov    edx, 0x2C
    call    NAMBAR_read_word
    dph    eax
    dps    " - PCM rate"

    newline
ret


;---------------------------------------------------------------------
;---  DATA OF PROGRAM  -----------------------------------------------
;---------------------------------------------------------------------
volume		 dd  15

attrinfo:
        dd      5
        dd      0
        dd      0
        dd      0
        dd      fileattr
        db      0
        dd      textbox_string

fileinfo:
 .mode          dd      0		 ; READ
 .first_byte    dd      0
                dd      0
 .bytes         dd      60*1024		 ; 60 Kb
 .dest		 dd  LOAD_BUFFER ;file_data
	 ;  db  "/HD/1/WINDOWS/MEDIA/WICEB7~1.WAV",0
;sz textbox_string, "/hd/1/testmuz/menuet11.wav",0
textbox_string:
;---------------------------------------------------------------------

IMAGE_END:		 ; end of program's image
   rb 257
;   rb 257-textbox_string.size
;     textbox_string.size

;---------------------------------------------------------------------

device_id dd ?	  ; (device_id << 16) + vendor_id
lastbus   db ?	  ; pci coordinates
bus	 db ?
devfn	   db ?

AC97ICH4  db ?	  ; Intel ICH4 codec flag
SI7012	    db ?    ; SiS SI7012 codec flag
NAMBAR	    dw ?    ; Audio Mixers Registers (base)
NABMBAR   dw ?	  ; Bus Master Registers   (base)

AC97IRQ   db ?	  ; Interrupt request
AC97INT   db ?	  ; Interrupt pin

wav_mode  db ?	  ; bits per sample & channels

;---------------------------------------------------------------------

ST_DONE = 0x0	 ; for interacting with player's window
ST_PLAY = 0x1
ST_EXIT = 0x2
ST_STOP = 0x4

status	    db ?

fileattr: rb 40

;---------------------------------------------------------------------
phys_bdl_buffer rd 1
phys_wav_buffer1 rd 1
phys_wav_buffer2 rd 1
align 32


; Buffer Descriptors List
;  ___________________________
;  |     physical address    | dword
;  |_________________________|
;  | attr       |   length   | dword     max. length = 65535 samples
;  |_________________________|

BDL_BUFFER:
 rb 32*8     ; 32 descriptors, 8 bytes each


;---------------------------------------------------------------------

file_data:

WAV_BUFFER1:
 rb 60 * 1024  ; 60 Kb

WAV_BUFFER2:
 rb 60 * 1024

LOAD_BUFFER:
 rb 60 * 1024

;---------------------------------------------------------------------

procinfo process_information

work_area:
 rb 0x10000

;---------------------------------------------------------------------

rb 0x800
thread_stack:

rb 0x1000 ; for stack
STACK_P:

MEMORY_END:
