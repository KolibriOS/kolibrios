lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         GUI
;           Libary
;
;   Ver 0.01
;

macro gui_text x,y,color,font_size,text {
   db 4 ; type of gui element 4 - text
   dw y, x
   dd (1 shl 31)+color+((font_size-1) shl 24)
   db text, 0
}

macro gui_button id,x,y,w,h,bcolor,tcolor,text {
   db 8 ; type of gui element 8 - button with text
   dw id, w, x, h, y
   dd bcolor
   dd (1 shl 31)+tcolor
   db text, 0
}

macro gui_end {
   db 255 ; type of gui element 255 - end of gui marker
}

macro draw_gui address
{
   mov esi,address
   call draw_gui_proc

if ~ defined draw_gui_used
draw_gui_used equ 1
   jmp end_draw_gui_proc
draw_gui_proc:
next_element:
   xor eax,eax
   mov al,byte [esi]
; eax =4 draw label
   cmp eax,4
   jne no_label
   mov ebx,[esi+1]     ; x,y
   mov ecx,[esi+1+4]   ; color
   lea edx,[esi+1+4+4] ; text offset
   add esi,9
   mcall
   jmp skip_string
no_label:
; eax = 8 draw button
   cmp eax,8
   jne no_draw_button
   xor edx,edx
   mov dx,[esi+1]        ; id
   mov ebx,[esi+3]     ; x,xs
   mov ecx,[esi+7]   ; y,ys
   push esi
   mov esi,[esi+11] ; button color
   mcall
   pop esi

   lea ebp,[esi+19]      ; start of text
   call get_size_of_string
   mov ebx,6
   mul ebx

   mov bx,[esi+3]
   sub bx,ax
   shr bx,1
   add bx,[esi+5]

   mov dx,[esi+7]
   sub dx,7
   shr dx,1
   add dx,[esi+9]

   shl ebx,16
   mov bx,dx

   mov ecx,[esi+15]       ; text color
   lea edx,[esi+19]       ; text offset
   mov eax,4
   add esi,19
   mcall
   jmp skip_string
no_draw_button:
   cmp eax,255
   je end_of_gui
; unknown gui element
   int3
end_of_gui:
   ret

get_size_of_string:
   xor eax,eax
next_bt:
   cmp byte [ebp],0
   jne no_en
   ret
no_en:
   inc ebp
   inc eax
   jmp next_bt
; function for skip string of text
next_byte:
   inc esi
skip_string:
   cmp byte [esi],0
   jne next_byte
   inc esi
   jmp next_element
end_draw_gui_proc:

end if

}