;
;
;  This is a example of using GUI component Button from libGUI
;
;

control_hader_size           =  44
control_button_data_size     =  50

include 'macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      4*1024
        dd      4*1024
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
        mov [crate_button],eax

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
        mov [Button1.y],10
        mov [Button1.width],word 70
        mov [Button1.height],word 20
        mov [Button1.color1],0xaabbccff;dword ecx
        ;mov [Button1.color2],dword 0xffffff
        mov [Button1.text],text1

        push Button1
	push Parend
        call [crate_button]
        mov [PointerToControlForButton1],eax

        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        mov [Button2.type],byte 10010010b
        mov [Button2.x],20
        mov [Button2.y],40
        mov [Button2.width],word 70
        mov [Button2.height],word 20
        mov [Button2.color1],dword ecx
        mov [Button2.color2],dword 0xffffff
        mov [Button2.image],ButtonPicture
        mov [Button2.imageSizeX],16
        mov [Button2.imageSizeY],16
        mov [Button2.transparentColor],0xffffff

        push Button2
	push Parend
        call [crate_button]
        mov [PointerToControlForButton2],eax


        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        mov [Button3.type],byte 10000100b
        mov [Button3.x],20
        mov [Button3.y],70
        mov [Button3.width],word 70
        mov [Button3.height],word 20
        mov [Button3.color1],dword ecx
        mov [Button3.color2],dword 0xffffff
        mov [Button3.text],text1
        mov [Button3.textX],40
        mov [Button3.textY],8
        mov [Button3.textcolor],0
        mov [Button3.image],ButtonPicture
        mov [Button3.imageX],3
        mov [Button3.imageY],3
        mov [Button3.imageSizeX],16
        mov [Button3.imageSizeY],16
        mov [Button3.transparentColor],0xffffff

        push Button3
	push Parend
        call [crate_button]
        mov [PointerToControlForButton3],eax

        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        mov [Button4.type],byte 10010001b
        mov [Button4.x],20
        mov [Button4.y],100
        mov [Button4.width],word 70
        mov [Button4.height],word 20
        mov [Button4.color1],dword ecx
        mov [Button4.color2],dword 0xffffff
        mov [Button4.text],text4
        mov [Button4.textX],10
        mov [Button4.textY],10
        mov [Button4.textcolor],0
        mov [Button4.image],ButtonPicture
        mov [Button4.imageX],5
        mov [Button4.imageY],5
        mov [Button4.imageSizeX],16
        mov [Button4.imageSizeY],16
        mov [Button4.transparentColor],0xffffff

        push Button4
	push Parend
        call [crate_button]
        mov [PointerToControlForButton4],eax


        call draw_window

        ;send message 1 for redrawing ALL controls
        mov [Message],dword 1

        push Message
        push Parend
        call [send_message]


still:
        mcall   10

        mov [SystemEvent],eax

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


         ;button 1
         mov esi,[PointerToControlForButton1]
         add esi,control_hader_size ;size control's structure
         mov edi,Button1
         mov ecx,control_button_data_size
         rep movsb

         ;button 2
         mov esi,[PointerToControlForButton2]
         add esi,control_hader_size ;size control's structure
         mov edi,Button2
         mov ecx,control_button_data_size
         rep movsb

         ;button 3
         mov esi,[PointerToControlForButton3]
         add esi,control_hader_size ;size control's structure
         mov edi,Button3
         mov ecx,control_button_data_size
         rep movsb

         ;button 4
         mov esi,[PointerToControlForButton4]
         add esi,control_hader_size ;size control's structure
         mov edi,Button4
         mov ecx,control_button_data_size
         rep movsb

         xor eax,eax
         mov al,[Button4.flag]
         cmp al,11b
         jne no_exit_demo

            mov [button_pressed],1

         no_exit_demo:

         cmp al,1b
         jne no_only_crossing_button

          cmp [button_pressed],1
          jne no_only_crossing_button

           jmp exit

         no_only_crossing_button:

         jmp still
         no_mouse:

        jmp still

exit:

        ;free resourses
        push [PointerToControlForButton1]
        call [destroy_control]

        push [PointerToControlForButton2]
        call [destroy_control]

        push [PointerToControlForButton3]
        call [destroy_control]

        push [PointerToControlForButton4]
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
        add ebx,200
        add ecx,200
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

dll_name      db 'libGUI.obj',0
sys_dll_path  db '/sys/lib/libGUi.obj',0


text1          db 'text',0
text4          db 'Exit',0

fnDestroyControl                     db 'DestroyControl',0
fnSendMessage                        db 'SendMessage',0
fnCraeteButton                       db 'CraeteButton',0

myexport                             dd 0

destroy_control                      dd 0
send_message                         dd 0
crate_button                         dd 0

PointerToControlForButton1           dd 0
PointerToControlForButton2           dd 0
PointerToControlForButton3           dd 0
PointerToControlForButton4           dd 0

SystemEvent                          dd 0
button_pressed                       rb 1

path                                 rb 256

Parend:        dd 0,0,0,0,0,0,0,0,0,0,0,0   ;44 bytes
Message        dd 0,0,0,0
ColorsTable    dd 0,0,0,0,0,0,0,0,0,0,0,0

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


Button1            BUTTON
Button2            BUTTON
Button3            BUTTON
Button4            BUTTON

antibug rb 460

ButtonPicture:
file 'test.raw'

i_end:
