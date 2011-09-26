; Text CodePage = cp1251

; <--- include all MeOS stuff --->

include "../../../../../macros.inc"


; <--- start of MenuetOS application --->
MEOS_APP_START

include  "key_read.inc"

; <--- start of code --->
CODE

   mov	eax,48			    ; get system colors
   mov	ebx,3
   mov	ecx,sc
   mov	edx,sizeof.system_colors
   mcall

	 xor	  ecx, ecx
	 inc ecx
	 mcall 66,1		       ; установка режима ввода сканкодов



  redraw:				 ; redraw event handler
   call    draw_window		  ; at first create and draw the window

  wait_event:			   ; main cycle

      mcall   10		   ; ожидать событие

;    mcall    23, 2                  ; вариант постоянный опрос
;    or      eax, eax
;    jz     key.1

    dec   eax			 ;   if event = 1
    jz	    redraw		 ;   jump to redraw handler
    dec   eax			 ;   else if event = 2
    jz	    key 		   ;   jump to key handler


  button:			  ; button event handler
    mov     al, 17		 ;   get button identifier
    mcall

    cmp     ah, 1
    jne     wait_event	      ;   return if button id != 1

    or	    eax, -1		  ;   exit application
    mcall

  key:				    ; key event handler
		     ;   get key code
    call      load_keys
				       ; преобразование
.1:
    mov     ah, 0xfe
    mov     edx, txt_data.fe
    call     load_ports

    mov     ah, 0xfd
    mov     edx, txt_data.fd
    call     load_ports

    mov     ah, 0xfb
    mov     edx, txt_data.fb
    call     load_ports

    mov     ah, 0xf7
    mov     edx, txt_data.f7
    call     load_ports

    mov     ah, 0xef
    mov     edx, txt_data.ef
    call     load_ports

    mov     ah, 0xdf
    mov     edx, txt_data.df
    call     load_ports

    mov     ah, 0xbf
    mov     edx, txt_data.bf
    call     load_ports

    mov     ah, 0x7f
    mov     edx, txt_data.7f
    call     load_ports

    mov     ah, 0x00
    mov     edx, txt_data.00
    call     load_ports



    jmp     redraw

  draw_window:
    mov     eax, 12		   ; start drawing
    mov     ebx, 1
    mcall

    xor       eax, eax			    ; create and draw the window
    mov     ebx, 40*65536+560 ; (window_cx)*65536+(window_sx)
    mov     ecx, 40*65536+240  ; (window_cy)*65536+(window_sy)
    mov     edx, [sc.work]		; work area color 
    or	       edx, 0x33000000	      ; & window type 3
    mov     edi, title			  ; window title
    int        0x40

    mov     ecx, [sc.work_text]
    or	       ecx, 0x80000000
    mov     edx, txt_data
    mcall   4, 10*65536+40
    mov     edx, txt_data.2s
    mcall   4, 10*65536+60
    mov     edx, txt_data.3s
    mcall   4, 10*65536+80


    mov     eax, 12		   ; finish drawing
    mov     ebx, 2
    mcall

  ret


load_ports:			 ; вход  ah - старший байт порта
					  ;           edx - адрес текста
     mov     al, 0xFE
     call      load_port_FE

rept  8
{
     mov     bl, '1'
     shl      al,1
     jc   @f
     mov     bl, '0'
@@:
     mov     byte [edx], bl
     inc      edx
}

  ret




; <--- initialised data --->
DATA


title db 'Key ZX-Spectrum TEST',0

txt_data   db '#FE: '
.fe db '********   #FD: '
.fd db '********   #FB: '
.fb db '********   #F7: '
.f7 db '********   ', 0
.2s db '#EF: '
.ef db '********   #DF: '
.df db '********   #BF: '
.bf db '********   #7F: '
.7f db '********   ', 0
.3s db '#00: '
.00 db '********   ', 0


; <--- uninitialised data --->
UDATA
sc   system_colors

MEOS_APP_END
; <--- end of MenuetOS application --->