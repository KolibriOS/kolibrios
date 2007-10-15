;
;
; This is example of using GUI component EditBox from libGUI.
;
;

include 'macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      1800
        dd      1800
        dd      0
        dd      path

start:
        ;init hepe of memory
        mcall   68,11

        ;set current dir as ./
        call GetPath

        ;load dll
        mcall   68,19,path

        test    eax,eax
        jnz    libGUI_loaded

        mcall 68,19,sys_libGUI_path

           test eax,eax
           jnz libGUI_loaded

             mcall -1

        libGUI_loaded:

        mov [myexport],eax

        ;load dll functions

        push fnDestroyControl
        push [myexport]
        call _ksys_cofflib_getproc
        mov [destroy_control],eax

        push fnSendMessage
        push [myexport]
        call _ksys_cofflib_getproc
        mov [send_message],eax

        push fnCraeteButton
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_button],eax

        push fnCraeteEditbox
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_edit_box],eax

        ;set events mask
        mcall   40,1100111b

        ;get standart colors table
        mcall   48,3,ColorsTable,40

        ;*********************************************
        ;****************Init Butttons****************
        ;*********************************************

        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        mov [Button1.type],byte 10010001b
        mov [Button1.x],20
        mov [Button1.y],50
        mov [Button1.width],word 70
        mov [Button1.height],word 20
        mov [Button1.color1],dword ecx
        mov [Button1.color2],dword 0xffffff
        mov [Button1.text],text1

        push Button1
        push Parend
        call [craete_button]
        mov [PointerToControlForButtonExit],eax

        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        ;********************************************
        ;***************Init EditBox*****************
        ;********************************************

        mov ecx,[ColorsTable+8]
        mov [EditBox.ed_flags],0b;1000000000000000b
        mov [EditBox.ed_left],5   ;x
        mov [EditBox.ed_top],10   ;y
        mov [EditBox.ed_width],100
        mov [EditBox.ed_height],14
        mov [EditBox.ed_max],100
        mov [EditBox.ed_text],buffer_for_text
        mov [EditBox.ed_color],dword 0xffffff
        mov [EditBox.shift_color],dword 0xaabbcc
        mov [EditBox.ed_focus_border_color],0
        mov [EditBox.ed_blur_border_color],ecx
        mov [EditBox.ed_text_color],0

        push EditBox
        push Parend
        call [craete_edit_box]
        mov [PointerToControlForEditBox],eax

        call draw_window


        ;send message 1 for redrawing ALL controls
        mov [Message],dword 1

        push Message
        push Parend
        call [send_message]

still:
        mcall   10

        ;check for redraw window
        cmp eax,1
        jne no_window

          call draw_window

          mov [Message],dword 1
          push Message
          push Parend
          call [send_message]

        jmp still
        no_window:

        ;check for keys events
        cmp eax,2
        jne no_keys

          mcall   2
          shr eax,8

          mov [Message],dword 2
          mov [Message+4],eax

          push Message
          push Parend
          call [send_message]

          mov eax,[Message+4]

          cmp al,27
          je exit

        jmp still
        no_keys:

        ;check for pressed butons
        cmp eax,3
        jne no_buttons

          mcall   17
          shr eax,8

        jmp still
        no_buttons:

        ;check for mouse events
        cmp eax,6
        jne no_mouse

         mov [Message],dword 6

         mcall   37,1
         mov ebx,eax
         shr eax,16 ;x
         and ebx,0xffff ;y

         mov [Message+4],eax
         mov [Message+8],ebx

         mcall   37,2
         mov [Message+12],eax

         ;send system events to control
         push Message
         push Parend
         call [send_message]

         mov eax,[PointerToControlForButtonExit]

         xor ebx,ebx
         mov bl,byte[eax+45]
         cmp bl,11b
         jne no_crossing_pressing_button

           mov [button_pressed],1

         no_crossing_pressing_button:

         xor ebx,ebx
         mov bl,byte[eax+45]
         cmp bl,1b
         jne no_crossing_button

           cmp [button_pressed],1
           jne no_crossing_button

              jmp exit

         no_crossing_button:

         jmp still
         no_mouse:

        jmp still

exit:

        push [PointerToControlForButtonExit]
        call [destroy_control]

        push [PointerToControlForEditBox]
        call [destroy_control]

        mcall   -1



;**********************************************
;*******************Draw window****************
;**********************************************

draw_window:

        mcall 12,1

        xor eax,eax
        mov ebx,50
        mov ecx,50
        shl ebx,16
        shl ecx,16
        add ebx,120
        add ecx,120
        mov edx,0x03aabbcc
        mov esi,0x805080d0
        mov edi,0x005080d0
        mcall

        mcall   12,2
        ret


GetPath:

        mov ebx,255
        mov ecx,path

        next_symvol:
        mov edx,ecx
        add edx,ebx

        xor eax,eax
        mov al,[edx]
        cmp eax,'/'
        je exit_path

        dec ebx
        jnz next_symvol

        exit_path:

        inc edx
        mov esi,dll_name
        mov edi,edx
        mov ecx,10
        rep movsb

        ret


include 'getproc.asm'

;************************************************************
;***************************DATA*****************************
;************************************************************

align 4

dll_name        db 'libGUI.obj',0
sys_libGUI_path db '/sys/lib/libGUI.obj',0

text1    db 'Exit',0

text_for_text db 'Hello world from zakladka',0

fnDestroyControl                     db 'DestroyControl',0
fnSendMessage                        db 'SendMessage',0
fnCraeteButton                       db 'CraeteButton',0
fnCraeteEditbox                      db 'CraeteEditbox',0

myexport                             dd 0

destroy_control                      dd 0
send_message                         dd 0
craete_button                         dd 0
craete_edit_box                       dd 0

PointerToControlForButtonExit        dd 0

PointerToControlForEditBox           dd 0

button_pressed                       dd 0

path                                 rb 256

Parend:        dd 0,0,0,0,0,0,0,0,0,0,0,0   ;44 bytes
Message                               rd 4

ColorsTable  rd 10
buffer_for_text rb 100+2

struc BUTTON
{
 .type             db 1
 .flag             db 1
 .x                dw 1
 .y                dw 1
 .width            dw 1
 .height           dw 1
 .image            dd 1
 .imageX           dw 1
 .imageY           dw 1
 .imageSizeX       dw 1
 .imageSizeY       dw 1
 .transparentColor dd 1
 .text             dd 1
 .textX            dw 1
 .textY            dw 1
 .textcolor        dd 1
 .color1           dd 1
 .color2           dd 1
 .mouseX           dw 1
 .mouseY           dw 1
}

struc EDITBOX
{
 .ed_width                      rd 1
 .ed_left                       rd 1
 .ed_top                        rd 1
 .ed_color                      rd 1
 .shift_color                   rd 1
 .ed_focus_border_color         rd 1
 .ed_blur_border_color          rd 1
 .ed_text_color                 rd 1
 .ed_max                        rd 1
 .ed_text                       rd 1
 .ed_flags                      rw 1
 .ed_size                       rd 1
 .ed_pos                        rd 1
 .ed_offset                     rd 1
 .cl_curs_x                     rd 1
 .cl_curs_y                     rd 1
 .ed_shift_pos                  rd 1
 .ed_shift_pos_old              rd 1
 .ed_height                     rd 1
 .mouseX                        rd 1
 .mouseY                        rd 1
}

Button1             BUTTON
EditBox             EDITBOX

i_end:
