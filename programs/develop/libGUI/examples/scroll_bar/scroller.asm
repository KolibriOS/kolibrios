;
;
;  This is example of using GUI component Scroller from libGUI
;
;

control_hader_size           =  44
control_scroller_data_size   =  29
control_button_data_size     =  50

first_child_button_pressed   =  1b
second_child_button_pressed  =  10000b


include 'macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      4000
        dd      4000
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
        jnz     libGUI_loaded

          ;load dll from system directory
          mcall 68,19,sys_dll_path

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

        push fnCraeteScroller
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_scroller],eax


        ;set events mask
        mcall   40,1100111b

        ;get standart colors table
        mcall   48,3,ColorsTable,40

        ;*********************************************
        ;****************Init Butttons****************
        ;*********************************************

        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        mov [ButtonExit.type],byte 10010001b
        mov [ButtonExit.x],90
        mov [ButtonExit.y],160
        mov [ButtonExit.width],word 70
        mov [ButtonExit.height],word 20
        mov [ButtonExit.color1],dword ecx
        mov [ButtonExit.color2],dword 0xffffff
        mov [ButtonExit.text],text


        push ButtonExit
        push Parend
        call [craete_button]
        mov [PointerToControlForButtonExit],eax

         mov ecx,[ColorsTable+8]
         and ecx,0xffffff

        ;********************************************
        ;***************Init scrollers****************
        ;********************************************

        ;init vertical scroller
        mov ecx,[ColorsTable+8]
        mov [VerticalScroller.type],byte 11100001b
        mov [VerticalScroller.x],10
        mov [VerticalScroller.y],30
        mov [VerticalScroller.pos],0.2
        mov [VerticalScroller.length],200
        mov [VerticalScroller.size],0.9
        mov [VerticalScroller.color1],ecx


        push VerticalScroller
        push Parend
        call [craete_scroller]
        mov [PointerToControlForVerticalScroller],eax

        ;init horizontal scroller
        mov ecx,[ColorsTable+8]
        mov [HorizontalScroller.type],byte 11110010b
        mov [HorizontalScroller.x],30
        mov [HorizontalScroller.y],30
        mov [HorizontalScroller.pos],0.7
        mov [HorizontalScroller.length],200
        mov [HorizontalScroller.size],0.3
        mov [HorizontalScroller.color1],0xaabbccff;ecx

        push HorizontalScroller
        push Parend
        call [craete_scroller]
        mov [PointerToControlForHorizontalScroller],eax

        call draw_window

        ;send message 1 for redrawing ALL controls
        mov [Message],dword 1

        push Message
        push Parend
        call [send_message]

still:
        mcall   10

        mov [SystemEvent],eax

        ;-----------------------
        ;check for redraw window
        ;-----------------------

        cmp eax,1
        jne no_window

          call draw_window

          mov [Message],dword 1

          push Message
          push Parend
          call [send_message]

          jmp still
        no_window:

        ;---------------------
        ;check for keys events
        ;---------------------

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

        ;-------------------------
        ;check for events of mouse
        ;-------------------------

        cmp eax,3
        jne no_button_close_window


        mcall   17
        shr eax,8

        jmp still
        no_button_close_window:

        ;check for mouse events
        cmp eax,6
        jne no_mouse

         ;craete message of mouse for controls
         mov [Message],dword 6

         mcall   37,1
         mov ebx,eax
         shr eax,16 ;x
         and ebx,0xffff ;y

         mov [Message+4],eax
         mov [Message+8],ebx

         mcall   37,2
         mov [Message+12],eax

         ;send message to controls
         push Message
         push Parend
         call [send_message]

         ;interraction with button exit
         ;copy data of scroller of button from control to structure

         mov esi,[PointerToControlForButtonExit]
         add esi,control_hader_size
         mov edi,ButtonExit
         mov ecx,control_button_data_size
         rep movsb

         xor eax,eax
         mov al,[ButtonExit.flag]

         ;check button for pressing
         and al,10b
         test al,al
         jz button_3_not_pressed

           mov [button_pressed],1

         jmp no_pressed_button

         button_3_not_pressed:

         cmp [button_pressed],1
         jne no_pressed_button

             jmp exit
         no_pressed_button:

         ;interraction with vertical scroller

         ;copy data of vertical scroller from control to structure

         mov esi,[PointerToControlForVerticalScroller]
         add esi,control_hader_size
         mov edi,VerticalScroller
         mov ecx,control_scroller_data_size
         rep movsb

         mov eax,[VerticalScroller.pos]
         mov [PosY_float],eax  ;position of scroll bar  from 0...1

         xor edx,edx
         call DrawRectangle

         xor eax,eax
         mov ax,[VerticalScroller.buttons_flags]
         and ax,first_child_button_pressed
         test ax,ax
         jz vertical_first_child_button_not_pressed

           mov edx,0xff00
           call DrawRectangle

         vertical_first_child_button_not_pressed:


         xor eax,eax
         mov ax,[VerticalScroller.buttons_flags]
         and ax,second_child_button_pressed
         test ax,ax
         jz vertical_second_child_button_not_pressed

           mov edx,0xff
           call DrawRectangle

         vertical_second_child_button_not_pressed:

         ;interraction with horizontal scroller

         ;copy data of horizontal scroller from control to structure

         mov esi,[PointerToControlForHorizontalScroller]
         add esi,control_hader_size
         mov edi,HorizontalScroller
         mov ecx,control_scroller_data_size
         rep movsb

         xor eax,eax
         mov ax,[HorizontalScroller.buttons_flags]
         and ax,first_child_button_pressed
         test ax,ax
         jz horizontal_first_child_button_not_pressed

           mov edx,0xffffff
           call DrawRectangle

         horizontal_first_child_button_not_pressed:

         xor eax,eax
         mov ax,[HorizontalScroller.buttons_flags]
         and ax,second_child_button_pressed
         test ax,ax
         jz horizontal_second_child_button_not_pressed

           mov edx,0xff0000
           call DrawRectangle

         horizontal_second_child_button_not_pressed:

         jmp still
         no_mouse:

        jmp still

exit:

        ;free resourses
        push [PointerToControlForVerticalScroller]
        call [destroy_control]

        push [PointerToControlForHorizontalScroller]
        call [destroy_control]

        push [PointerToControlForButtonExit]
        call [destroy_control]

        mcall   -1

;**********************************************
;*******************Draw window****************
;**********************************************

draw_window:

        mcall   12,1

        xor eax,eax
        mov ebx,50
        mov ecx,50
        shl ebx,16
        shl ecx,16
        add ebx,280
        add ecx,280
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

DrawRectangle:

        mov eax,13
        mov ebx,100*65536+50
        mov ecx,100*65536+50
        int 0x40

        ret

include 'getproc.asm'

;************************************************************
;***************************DATA*****************************
;************************************************************

align 4

dll_name      db 'libGUI.obj',0
sys_dll_path  db '/sys/lib/libGUI.obj',0

text          db 'Exit',0

fnDestroyControl                     db 'DestroyControl',0
fnSendMessage                        db 'SendMessage',0
fnCraeteButton                       db 'CraeteButton',0
fnCraeteScroller                     db 'CraeteScroller',0

myexport                             dd 0

destroy_control                      dd 0
send_message                         dd 0
craete_button                         dd 0
craete_scroller                        dd 0

PointerToControlForButtonExit        dd 0

PointerToControlForHorizontalScroller dd 0
PointerToControlForVerticalScroller   dd 0


SystemEvent                          dd 0

PosX_float                           dd 0
PosY_float                           dd 0


button_pressed                       dd 0

IPC_table                            rd 256
path                                 rb 256
ColorsTable                          rd 10

Parend:    dd 0,0,0,0,0,0,0,0,0,0,0,0   ;44 bytes
Message                              rd 4
x                                    dd 0
y                                    dd 0
number                               dd 0

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

struc SCROLLER
{
 .type             rb 1
 .x                rw 1
 .y                rw 1
 .length           rw 1
 .color1           rd 1
 .size             rd 1
 .pos              rd 1
 .buttons_flags    rw 1
 .ChildButton1     rd 1
 .ChildButton2     rd 1
 .mouseX           rw 1
 .mouseY           rw 1
}

ButtonExit          BUTTON
VerticalScroller     SCROLLER
HorizontalScroller   SCROLLER


i_end: