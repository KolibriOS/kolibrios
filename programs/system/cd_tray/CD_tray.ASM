;
; ���������� ������� �������� �������� ATAPI
; 22.07.2007 Mario79 �������� �������
;---------------------------------------------------------------------

  use32 	 ; �������� 32-������ ����� ����������
  org	  0x0	      ; ��������� � ����

  db	 'MENUET01'  ; 8-������� ������������� MenuetOS
  dd	 0x01	      ; ������ ��������� (������ 1)
  dd	 START	       ; ����� ������ �������
  dd	 I_END	       ; ������ ���������
  dd	 0x11000      ; ���������� ������
  dd	 0x11000      ; ����� ������� �����
  dd	 0x0	     ; ����� ������ ��� ���������� (�� ������������)
  dd	 0x0	     ; ���������������

; Language support for locales: it_IT, en_US.

include 'macros.inc' ; ������� ��������� ����� ��������������!

;---------------------------------------------------------------------
;---  ������ ���������  ----------------------------------------------
;---------------------------------------------------------------------

START:

red:		; ������������ ����
    call draw_window	; �������� ��������� ��������� ����

;---------------------------------------------------------------------
;---  ���� ��������� �������  ----------------------------------------
;---------------------------------------------------------------------

still:
    mcall 10	    ; ������� 10 - ����� �������

    cmp  eax,1	      ; ������������ ���� ?
    je	   red	      ; ���� �� - �� ����� red
    cmp  eax,2	      ; ������ ������� ?
    je	   key	      ; ���� �� - �� key
    cmp  eax,3	      ; ������ ������ ?
    je	   button     ; ���� �� - �� button

    jmp  still	      ; ���� ������ ������� - � ������ �����


;---------------------------------------------------------------------


  key:		  ; ������ ������� �� ����������
    mcall 2	   ; ������� 2 - ������� ��� ������� (� ah)

    jmp  still	      ; ��������� � ������ �����

;---------------------------------------------------------------------

  button:
    mcall 17	    ; 17 - �������� ������������� ������� ������
; ������ 2,3,4,5 - load tray CD0/CD1/CD2/CD3
        movzx   ecx, ah
        sub     ecx, 2
        cmp     ecx, 3
        ja      b6
        mcall 24,5
        jmp     red
  b6:
; ������ 6,7,8,9 - eject tray CD0/CD1/CD2/CD3
        sub     ecx, 4
        cmp     ecx, 3
        ja      b1
        mcall 24,4
        jmp     red
  b1:
    cmp   ah, 1     ; ���� �� ������ ������ � ������� 1,
    jne   still     ;  ���������

  .exit:
    mcall -1	    ; ����� ����� ���������



;---------------------------------------------------------------------
;---  ����������� � ��������� ����  ----------------------------------
;---------------------------------------------------------------------

draw_window:
    mcall 12, 1 	   ; ������� 12: �������� �� �� ��������� ����
		   ; 1 - �������� ��������
		   ; ������� ����

    if lang eq it_IT
			mcall 0, <100,250>, <100,110>, 0x03AABBCC, 0x805080D0, 0x005080D0
	 else ; Default to en_US
			mcall 0, <100,230>, <100,110>, 0x03AABBCC, 0x805080D0, 0x005080D0
	 end if
    mcall 71, 1 ,header
    mcall 8,<15,42>,<40,20>,2,0xaaaaaa
    call  draw_buttons
    mov   edx,6
    add   ecx,30 shl 16
    mcall
    call  draw_buttons

	 if lang eq it_IT
			mcall 4, <16,25>, 0x80ffffff,text1
	 else ; Default to en_US
			mcall 4, <25,25>, 0x80ffffff,text1
	 end if
    mov   edx,text2

    add   ebx,3 shl 16+20
    mcall
    mov   edx,text3
    add   ebx,30
    sub   ebx,3 shl 16
    mcall

    mcall 12, 2 	   ; ������� 12: �������� �� �� ��������� ����
		   ; 2, ��������� ��������

    ret 	       ; ������� �� ���������

draw_buttons:
    pusha
    if lang eq it_IT
			add   ebx,60 shl 16
	 else ; Default to en_US
			add   ebx,50 shl 16
	 end if
    inc   edx
    mcall
    if lang eq it_IT
			add   ebx,60 shl 16
	 else ; Default to en_US
			add   ebx,50 shl 16
	 end if
    inc   edx
    mcall
    if lang eq it_IT
			add   ebx,60 shl 16
	 else ; Default to en_US
			add   ebx,50 shl 16
	 end if
    inc   edx
    mcall
    popa
    ret
;---------------------------------------------------------------------
;---  ������ ���������  ----------------------------------------------
;---------------------------------------------------------------------
header db ' ATAPI Device Tray Control',0
if lang eq it_IT
	text3 db 'espelli   espelli   espelli   espelli',0
	text2 db 'carica    carica    carica    carica',0
	text1 db '/cd0/     /cd1/     /cd2/     /cd3/',0
else ; Default to en_US
	text3 db 'eject   eject   eject   eject',0
	text2 db 'load    load    load    load',0
	text1 db '/cd0/   /cd1/   /cd2/   /cd3/',0
end if

;---------------------------------------------------------------------

I_END:			 ; ����� ����� ���������
