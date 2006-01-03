;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                    ;
;;  Scroll.inc                                                        ;
;;                                                                    ;
;;  Vertical Scroll Bar Example for MenuetOS                          ;
;;                                                                    ;
;;  Version 0.2  27 February 2005                                     ;
;;                                                                    ;
;;                                                                    ;
;;          GNU GENERAL PUBLIC LICENSE                                ;
;;             Version 2, June 1991                                   ;
;;                                                                    ;
;;  Copyright 2005 Jason Delozier,                                    ;
;;   cordata51@hotmail.com                                            ;
;;                                                                    ;
;;  See file COPYING for details                                      ;
;;                                                                    ;
;;  Updates:                                                          ;
;;     - FPU not longer used for calculations -Feb 27, 2005           ;
;;                                                                    ;
;;                                                                    ;
;;   TODO:                                                            ;
;;     - Create Message Handler and only allow runtime arguments      ;
;;       to be processed through the message handler                  ;
;;     - Remove all local varibles and only allow working varible to  ;
;;       reside in scroll bar structure.                              ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;





;*****************************************************************************
;*
;*   VScroll Structure Equates
;*
;*****************************************************************************

VScroll_width	      equ 0
VScroll_x	      equ 2
VScroll_height	      equ 4
VScroll_y	      equ 6
VScroll_min	      equ 8
VScroll_max	      equ 12
VScroll_current       equ 16
VScroll_s_change      equ 20
VScroll_b_change      equ 24

;Example VScroll Structure
;VScroll_1:
;   dw  16  ;width         +0
;   dw  100 ;x             +2
;   dw  200 ;height        +4
;   dw  40  ;y             +6
;   dd  0   ;min           +8
;   dd  1000;max           +12
;   dd  1   ;current       +16
;   dd  1   ;small change  +20
;   dd  10  ;big change    +24

VScroll_Min_Height dw 16     ;scroll box
Vscroll_Fixed_Height dw 16   ;height of scroll buttons (up and down)

;**************************************
;*
;*   Mouse Over VScroll
;*
;**************************************
vsm1 dd 0
vstoff dd 0

VScroll_mouse_over:
   cmp	 [mouseb], 1
   je	 VScroll_Mouse_Has_Mouse
   cmp	 [mouseb], 0
   jne	 VScroll_mouse_over_done2
   mov	 [vsm1], 0
   jmp	 VScroll_mouse_over_done2
VScroll_Mouse_Has_Mouse:
   push ecx
   mov	ecx, [vsm1]
   shl	ecx,2
   call dword [VScroll_Mouse_Jumptable+ecx]
VScroll_mouse_over_done:
   pop ecx
VScroll_mouse_over_done2:
   ret

VScroll_Mouse_Jumptable:
   dd	   VScroll_Mouse_Find
   dd	   VScroll_Mouse_On_Up
   dd	   VScroll_Mouse_On_Down
   dd	   VScroll_Mouse_Above_Thumb
   dd	   VScroll_Mouse_Below_Thumb
   dd	   VScroll_Mouse_On_Thumb

VScroll_Mouse_Find:
   inc	 ecx
   call  dword [VScroll_Mouse_Jumptable+ecx*4]
   cmp	 [vsm1], 0
   jne	 VScroll_mouse_over_done2
   cmp	 ecx, 5
   je	 VScroll_mouse_over_done2
   jmp	 VScroll_Mouse_Find


;**************************************
;Is Mouse at Up button
VScroll_Mouse_On_Up:
   push  eax
   movzx eax, word [ebp+VScroll_y]
   add	  ax, word [Vscroll_Fixed_Height]
   cmp	 [mousey], ax
   pop	 eax
   ja	 VScroll_Mouse_On_Up2
   mov	 [vsm1], 1
   call   VScroll_Small_Up
VScroll_Mouse_On_Up2:
   ret

;**************************************
;Is Mouse at Down button
VScroll_Mouse_On_Down:
   push  eax
   movzx eax,word [ebp+VScroll_y]
   add	  ax, word [ebp+VScroll_height]
   sub	  ax, word [Vscroll_Fixed_Height]
   cmp	 [mousey], ax
   pop	 eax
   jb	 VScroll_Mouse_On_Down2
   mov	 [vsm1], 2
   call  VScroll_Small_Down
VScroll_Mouse_On_Down2:
   ret

;**************************************
;Is Mouse above Scroll Thumb
VScroll_Mouse_Above_Thumb:
    push  eax
    push  ecx
    push  edx
    call  Vscroll_SBox_Area
    add    cx, word [ebp+VScroll_y]
    add    cx, word [Vscroll_Fixed_Height] ;plus height of scroll button
    cmp    cx, [mousey]
    pop   edx
    pop   ecx
    pop   eax
    jb	  VScroll_Mouse_Above_Thumb2
    mov   [vsm1], 3
    call  VScroll_Big_Up
VScroll_Mouse_Above_Thumb2:
    ret

;**************************************
;Is Mouse below scroll thumb
VScroll_Mouse_Below_Thumb:
    push  eax
    push  ecx
    push  edx
    call  Vscroll_SBox_Area
    add   cx, word [ebp+VScroll_y]
    add   cx, word [Vscroll_Fixed_Height] ;plus height of scroll button
    add   cx, ax
    cmp   cx, [mousey]
    pop   edx
    pop   ecx
    pop   eax
    ja	  VScroll_Mouse_Below_Thumb2
    mov   [vsm1], 4
    call  VScroll_Big_Down
VScroll_Mouse_Below_Thumb2:
    ret

;**************************************
;Mouse is on Scroll Thumb
VScroll_Mouse_On_Thumb:

    pusha
    cmp [vsm1], 5		       ;first time moving since we clicked?
    je	VScroll_Mouse_On_Thumb1        ;if not jump.. we already have offset
    mov   [vsm1], 5  ;determine offset between mouse pointer and top of thumb
    call  Vscroll_SBox_Area	       ;get top of thumb coordinate
    add   cx, word [ebp+VScroll_y]     ;make screen coordinate
    add   cx, word [Vscroll_Fixed_Height] ;plus height of scroll button and top
    movzx esi,word  [mousey]	       ;get mouse position
    sub   esi, ecx		       ;make offset between top of thumb
    mov   [vstoff], esi 	       ;and mouse pointer then save it
VScroll_Mouse_On_Thumb1:	       ;
    movzx esi, [mousey] 	       ;get mouse y value
    sub    si, word [ebp+VScroll_y]    ;get y starting value of scroll bar
    sub    si, [Vscroll_Fixed_Height]
    sub   esi, [vstoff] 	   ;
    call  Vscroll_SBox_Area	       ;
VScroll_Mouse_On_Thumb2:	       ;
    cmp   [ebp+VScroll_current], edx   ;only redraw if change occured
    je	  VScroll_Mouse_On_Thumb3      ;
    mov   [ebp+VScroll_current], edx   ;new current value
    call  drawvscrolla		       ;redraw scroll background and thumb
VScroll_Mouse_On_Thumb3:	       ;
    popa			       ;
    ret
;**************************************




;*****************************************************************************
;*
;*   Vscroll_SBox_Area
;*
;*   Purpose: To determine the area of the Scroll Bar Thumb and the current
;*            value of scroll based on top pixel of thumb.
;*
;*   Inputs:
;*           EBP - Structure of scroll bar
;*           ESI - Top of Thumb - optional
;*
;*   Outputs:
;*           EAX - Thumb Height
;*           ECX - Top of thumb
;*           EDX - Current Value based on top pixel of thumb. Valid when ESI
;*                 is given as input.
;*
;*   This procedure is divided up into 5 specific functions.
;*     1.  ((Vmax-VMin)/Small Change)= Total Changes
;*     2.  Height - (2* Fixed Button Height) = Total Pixels
;*     3.  Total Pixels - Total Changes = Thumb Height (must be >= 16)
;*             Minimum Height of 16 is varible [VScroll_Min_Height]
;*     4.   (((Tot Pixels - Thumb Height)* Current)/Max-min)=Top Pixel of Thumb
;*     5.   ((Mouse Top * (max-Min))/(Total Pix - Thumb Height)) = Current
;*          Step 5 is only valid if ESI is supplied as an input.
;*
;*
;*****************************************************************************

Vscroll_SBox_Area:
    push  ebx
    push  esi
    push  edi
    ;Step 1. Determine Total Changes
    mov   eax, dword [ebp+VScroll_max]	    ;get max scroll
    sub   eax, dword [ebp+VScroll_min]	    ;sub minmum scroll for total scro
    xor   edx,edx
    push  eax
    div   dword [ebp+VScroll_s_change] ;
    ;eax = total changes ((max-min)/Small Change)

    ;Step 2. Determine Total Pixels
    movzx ecx,	word [ebp+VScroll_height]   ;height of entire scroll bar
    mov    bx,	word [Vscroll_Fixed_Height] ;minus height of both scroll button
    shl    bx, 1
    sub    cx, bx
    push  ecx
    ;ecx = total pixels (Total Height - (2*Fixed Button Height)

    ;Step 3. Determine Thumb Height
    cmp   ecx, eax
    jbe   vs1
    sub   ecx, eax
    cmp   cx, word [VScroll_Min_Height]
    jb	  vs1
    jmp   vs2
vs1:
    movzx ecx,	word [VScroll_Min_Height]
vs2:
    ;ecx = Thumb Height  (Total Pixels - Total Changes) >=16

    ;Step 4. Determine Top Pixel of Thumb
    pop   edi			       ;Total Pixels
    sub   edi, ecx		       ;
    ;edi = Total Pixels - thumb Height
    mov   eax, edi		       ;
    mul   dword [ebp+VScroll_current]  ;get current scroll value
    pop   ebx			       ;max scroll
    push  ecx			       ;
    div   ebx			       ;
    mov   ecx, eax		       ;
    ;ecx = top pixel of thumb

    ;Step 5. Determine Current Value based on Mouse Position
    cmp si,0
    jge vss4
    mov eax, dword [ebp+VScroll_min]
    jmp vsdone1
vss4:
    cmp esi, edi
    jbe vss3
    mov eax, dword [ebp+VScroll_max]
    jmp vsdone1
vss3:
    mov   eax,ebx		       ;max scroll
    mul   esi			       ;Top Pixel of mouse
    div   edi			       ;Total Pixels - Thumb Height
    ;eax = Current Value

vsdone1:
    mov   edx, eax		       ;Current Value
    pop   eax			       ;Thumb Height
    pop   edi
    pop   esi
    pop   ebx
ret

;*****************************************************************************
;*
;*****************************************************************************
;ebp is structure
drawvscroll:
pusha
    ;up button
    mov eax, 13 		       ;button system function
    mov edx,0x00777777  ;0x6677cc		      ;color
    mov ebx, [ebp+VScroll_width]      ;x-start/width
    mov  cx, word [ebp+VScroll_y]      ;get bottom of scroll bar
    shl ecx, 16
    mov  cx, [Vscroll_Fixed_Height]    ;fixed height
    int 0x40

    ;down button
    mov ebx, [ebp+VScroll_width]      ;x-start/width
    mov  cx, word [ebp+VScroll_y]      ;get bottom of scroll bar
    add  cx, word [ebp+VScroll_height] ;
    sub  cx, [Vscroll_Fixed_Height]    ;y-start is 12 pixel from bottom
    shl ecx, 16 		      ;
    mov  cx, [Vscroll_Fixed_Height]    ;fixed height
    int 0x40
;------- 'ASCL.INC' needs to be included to use this --
    putlabel 493,46,'',cl_Black
    putlabel 493,372,'',cl_Black
;------------------------------------------------------

popa
drawvscrolla:
pusha

    mov   eax, 13			 ;Draw Scroll Box Background
    mov   edx, 0x00999999;0			 ;color
    push  edx
    movzx edx, word [Vscroll_Fixed_Height]
    mov   ebx, [ebp+VScroll_width]    ;x-start/width
    movzx ecx,word [ebp+VScroll_y]    ;y-start
    add   ecx, edx		      ;add fixed height button
    shl   ecx, 16		      ;do height next
    mov    cx,word [ebp+VScroll_height]    ;y-start
    sub    cx, dx		      ;subtract fixed height of up button
    sub    cx, dx		      ;subtract fixed height of down button
    pop   edx
    int   0x40				 ;tell system to draw it

;determine height of scroll box (thumb)

    call  Vscroll_SBox_Area
    add    cx, [ebp+VScroll_y]
    add    cx, 16
    shl   ecx, 16
    mov    cx, ax

    mov   eax, 13		      ;draw scroll box
    mov   ebx, [ebp+VScroll_width]	 ;x-start/width
    mov   edx, 0x00bbbbbb ;0x999999
    int   0x40
;call Draw_Scroll_info
popa
ret
;*****************************************************************************

;*****************************************************************************
;* VScroll_Big_Up
;* VScroll_Small_Up
;*****************************************************************************
;need to update for signed values
VScroll_Big_Up:
   push   eax
   push   ebx
   mov	  eax, dword [ebp+VScroll_b_change]
   jmp	  VScroll_Up
VScroll_Small_Up:
   push   eax
   push   ebx
   mov	  eax, dword [ebp+VScroll_s_change]
VScroll_Up:
   mov	  ebx, dword [ebp+VScroll_min]	  ;get minimum value
   cmp	  [ebp+VScroll_current], ebx	  ;are we at minimum already?
   je	  VScroll_Up_exit		  ;if so leave
   sub	  [ebp+VScroll_current], eax	  ;
   jnc	  VScroll_Up_done
   mov	  [ebp+VScroll_current], ebx
VScroll_Up_done:
   cmp	  [ebp+VScroll_current], ebx	   ;if current is greater then max
   jae	  VScroll_Up_done2
   mov	  [ebp+VScroll_current], ebx
VScroll_Up_done2:
;redraw
   call   drawvscroll ;temp?
VScroll_Up_exit:
   pop	  ebx
   pop	  eax

ret
;*****************************************************************************


;*****************************************************************************
;* VScroll_Big_Down
;* VScroll_Small_Down
;*****************************************************************************
;need to update for signed values
VScroll_Big_Down:
   push   eax
   push   ebx
   mov	  eax, dword [ebp+VScroll_b_change];get big change value
   jmp	  VScroll_Down
VScroll_Small_Down:
   push   eax
   push   ebx
   mov	  eax, dword [ebp+VScroll_s_change];get small change value
VScroll_Down:
   mov	  ebx, dword [ebp+VScroll_max]	   ;get maximum scroll value
   cmp	  [ebp+VScroll_current], ebx	   ;are we at max already?
   je	  VScroll_Down_exit		   ;if so leave
   add	  [ebp+VScroll_current], eax	   ;add change to current
   jno	  VScroll_Down_done		   ;dont go beyond bounds of register
   mov	  [ebp+VScroll_current], ebx	   ;if we did then we at max
VScroll_Down_done:			   ;
   cmp	  [ebp+VScroll_current], ebx	   ;if current is greater then max
   jbe	  VScroll_Down_done2		   ;
   mov	  [ebp+VScroll_current], ebx	   ;then we at max
VScroll_Down_done2:			   ;
;redraw
   call   drawvscroll	    ;temp?
VScroll_Down_exit:
   pop	  ebx
   pop	  eax
ret
;*****************************************************************************




