			;;      CPU latency test -- Art J    ;;


  use32 	     ;
  org	 0x0	     ;

  db	 'MENUET01'  ;
  dd	 0x01	     ;
  dd	 START	     ;
  dd	 I_END	     ;
  dd	 0x4000      ;
  dd	 0x4000      ;
  dd	 0x0	     ;
  dd	 0x0	     ;

include '..\..\..\macros.inc' ;


START:


    call draw_win


	rdtsc

main_loop:
	mov   ecx, eax		; ecx = previous timestamp
	rdtsc			; new timestamp
	cmp   eax, ecx
	jb    main_loop 	; tsc overflow?

	mov   ebx, eax		;
	sub   ebx, ecx		; net clocks

	cmp   ebx, [max]
	jb    @f
	mov   [max], ebx
;       mcall 13, 200*65536+60, 500*65536+20, 0x00EEEEEEEE
;       mcall 47, 0x800A0001, max, 200*65536+501
@@:
	push  ebx
	fldlg2			; lg(2)
	fild  dword[esp]
	fyl2x
	fimul [yscale]
	fistp dword[esp]
	pop   ebx		; ebx = lg(time/100) * yscale
	sub   ebx, [yscale]
	sub   ebx, [yscale]
	jb    main_loop

	cmp   ebx, 512
	ja    main_loop

	mov   edx, [ebx*4+data_]

	inc   edx
	mov   [ebx*4+data_], edx
	push  edx
	fld   qword[xscale]
	fild  dword[esp]
	fyl2x
	fistp dword[esp]
	pop   edx	      ; edx = log2(counts) * xscale
	cmp   edx, 300
	jge   main_loop

	shl   ebx, 12		 ; 1 line = 1024 pixels
	add   ebx, 1024*4*30 + 17*4
	mov   dword[ebx + edx*4 + 0xFE000000], 0x00703030
	jmp   main_loop




;------------------------------------------------
draw_win:
;------------------------------------------------


    mcall 12, 1
    mcall 0, 300, 590, 0x14BAA6A0,,title
; -----------------------------------------------------------------
    mov   ecx, [yscale]
    shl   ecx, 16
    mov   cx,  word[yscale]
    mov   [ytick], ecx
    mcall 38, 16*65536+16, 30*65536+530, 0
    mov   cx, 30
@@:
    mcall , 14*65536+16
    add   ecx, [ytick]
    cmp   cx, 512
    jb	  @b

    mcall 4, 0x00100230, 0x80000000, advise
    mov   eax, 47
    mov   ebx, 0x00010000
    mov   ecx, 2
    mov   edx, 7*65536+30
    mov   esi, 0
@@:
    mcall
    add   dx, word[yscale]
    inc   ecx
    cmp   dx, 512
    jb	  @b

    mcall 40, 0xC0000000	; ignore sys messages

    mcall 12, 2


ret


align 4
;-------------------------------------------------
data_  dd 512 dup 1	 ; hysto
max    dd  0
xscale dq  7.2
yscale dd  80
ytick  dd  0


    title db '   Latency test ',0
    advise db 'Use CPU process manager to close this window)',0

I_END:		; end of program

	rd 256

align 256
st_0:
