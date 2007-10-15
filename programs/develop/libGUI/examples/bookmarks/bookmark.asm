;
;
;  This is example using GUI component Bookmark from libGUI.
;
;

include 'macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      126*1024
        dd      126*1024
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

        push fnResizeComponent
        push [myexport]
        call _ksys_cofflib_getproc
        mov [resize_component],eax

        push fnRemoveComponent
        push [myexport]
        call _ksys_cofflib_getproc
        mov [remove_component],eax

        push fnCraeteButton
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_button],eax

        push fnCraeteScroller
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_scroller],eax

        push fnCraeteBookmark
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_bookmark],eax

        push fnCraeteImage
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_image],eax

        push fnCraeteText
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_text],eax

        push fnCraeteNumber
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_number],eax

        push fnCraeteCheckbox
        push [myexport]
        call _ksys_cofflib_getproc
        mov [craete_check_box],eax

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
        mov [Button1.x],120
        mov [Button1.y],120
        mov [Button1.width],word 70
        mov [Button1.height],word 20
        mov [Button1.color1],dword ecx
        mov [Button1.color2],dword 0xffffff
        mov [Button1.text],text1



        mov [Button3.type],byte 10010001b
        mov [Button3.x],165
        mov [Button3.y],320
        mov [Button3.width],word 70
        mov [Button3.height],word 20
        mov [Button3.color1],dword ecx
        mov [Button3.color2],dword 0xffffff
        mov [Button3.text],text3


        push Button3
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
        mov [VerticalScroller.type],byte 11110001b
        mov [VerticalScroller.x],150
        mov [VerticalScroller.y],10
        mov [VerticalScroller.pos],0.2
        mov [VerticalScroller.length],200
        mov [VerticalScroller.size],0.3
        mov [VerticalScroller.color1],ecx

        ;********************************************
        ;******************Init Image****************
        ;********************************************

        mov [Image.type],byte 10000000b
        mov [Image.x],30
        mov [Image.y],10
        mov [Image.sizex],200
        mov [Image.sizey],200
        mov [Image.pointer],Picture

        ;********************************************
        ;******************Init Text*****************
        ;********************************************

        mov [Text.type],byte 10000000b
        mov [Text.color],0xffffff
        mov [Text.x],100
        mov [Text.y],100
        mov [Text.length],25
        mov [Text.pointer],text_for_text

        ;********************************************
        ;*****************Init Number****************
        ;********************************************

        mov [Number.type],byte 10000010b
        mov [Number.color],0xffffff
        mov [Number.x],150
        mov [Number.y],100
        mov [Number.parameters],5*65536+4
        mov [Number.number],-1234.5678

        ;********************************************
        ;***************Init CheckBox****************
        ;********************************************

        mov ecx,[ColorsTable+8]
        and ecx,0xffffff

        ;mov [CheckBox.ch_flags],word 10b
        mov [CheckBox.ch_left],150
        mov [CheckBox.ch_top],330
        mov [CheckBox.ch_text_margin],4
        mov [CheckBox.ch_size],11
        mov [CheckBox.ch_color],0xffffff
        mov [CheckBox.ch_border_color],ecx
        mov [CheckBox.ch_text_color],0
        mov [CheckBox.ch_text_ptr],check_box_text
        mov [CheckBox.ch_text_length],9

        ;********************************************
        ;***************Init EditBox*****************
        ;********************************************

        mov ecx,[ColorsTable+8]
        mov [EditBox.ed_flags],0b;1000000000000000b
        mov [EditBox.ed_left],100   ;x
        mov [EditBox.ed_top],100   ;y
        mov [EditBox.ed_width],150
        mov [EditBox.ed_height],14
        mov [EditBox.ed_max],100
        mov [EditBox.ed_text],buffer_for_text
        mov [EditBox.ed_color],dword 0xffffff
        mov [EditBox.shift_color],dword 0xaabbcc
        mov [EditBox.ed_focus_border_color],0
        mov [EditBox.ed_blur_border_color],ecx
        mov [EditBox.ed_text_color],0



        ;********************************************
        ;***************Init bookmark****************
        ;********************************************


        ;init bookmark
        mov ecx,[ColorsTable+8]
        mov [Bookmark.type],byte 10000001b
        mov [Bookmark.x],10
        mov [Bookmark.y],10
        mov [Bookmark.sizex],350
        mov [Bookmark.sizey],270
        mov [Bookmark.color_1],dword ecx
        mov [Bookmark.color2],dword 0xffffff

        mov [Bookmark.number_bookmarks],3
        mov [Bookmark.number_zak_in_1],2
        mov [Bookmark.number_zak_in_2],3
        mov [Bookmark.number_zak_in_3],1

        mov [Bookmark.text_for_1],text4
        mov [Bookmark.number_contrl_1],1
        mov [Bookmark.type_control_1],1
        mov [Bookmark.info_1_control],dword Button1

        mov [Bookmark.text_for_2],text5
        mov [Bookmark.number_contrl_2],1
        mov [Bookmark.type_control_2],6
        mov [Bookmark.info_2_control],dword Number

        mov [Bookmark.text_for_3],text6
        mov [Bookmark.number_contrl_3],1
        mov [Bookmark.type_control_3],8
        mov [Bookmark.info_3_control],dword EditBox

        mov [Bookmark.text_for_4],text7
        mov [Bookmark.number_contrl_4],1
        mov [Bookmark.type_control_4],4
        mov [Bookmark.info_4_control],dword Image

        mov [Bookmark.text_for_5],text8
        mov [Bookmark.number_contrl_5],1
        mov [Bookmark.type_control_5],2
        mov [Bookmark.info_5_control],dword VerticalScroller

        mov [Bookmark.text_for_6],text9
        mov [Bookmark.number_contrl_6],1
        mov [Bookmark.type_control_6],5
        mov [Bookmark.info_6_control],dword Text


        push Bookmark
        push Parend
        call [craete_bookmark]
        mov [PointerToControlForBookmark],eax

        call draw_window

        ;*****************************************************
        ;**********Example of resize component****************
        ;*****************************************************
        mov ebx,[PointerToControlForButtonExit]
        push 30;270
        push 150;250
        push ebx
        call [resize_component]

        ;*****************************************************
        ;**********Example of remove component****************
        ;*****************************************************
        mov ebx,[PointerToControlForButtonExit]
        push 340;270
        push 120;250
        push ebx
        call [remove_component]

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

          mov eax,[PointerToControlForBookmark]
          mov ebx,[Window_SizeX]
          mov ecx,[Window_SizeY]
          sub ebx,50
          sub ecx,130
          ;or [eax+44],byte 1000000b
          mov [eax+32],ebx
          mov [eax+36],ecx

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

        push [PointerToControlForBookmark]
        call [destroy_control]

        mcall   -1



;**********************************************
;*******************Draw window****************
;**********************************************

draw_window:
        mcall   9,IPC_table,-1
        mcall   12,1

        mov eax,[IPC_table+34]
        mov ebx,[IPC_table+38]
        mov ecx,[IPC_table+42]
        mov edx,[IPC_table+46]
        mov [Window_CordinatX],eax
        mov [Window_CordinatY],ebx
        mov [Window_SizeX],ecx
        mov [Window_SizeY],edx

        cmp [Window_SizeX],400
        jae no_minimum_size_x

        mov [Window_SizeX],400
        mov ecx,[Window_SizeX]

        no_minimum_size_x:

        cmp [Window_SizeY],400
        jae no_minimum_size_y

        mov [Window_SizeY],400
        mov edx,[Window_SizeY]

        no_minimum_size_y:

        xor eax,eax
        mov ebx,[Window_CordinatX]
        mov ecx,[Window_CordinatY]
        shl ebx,16
        shl ecx,16
        add ebx,[Window_SizeX]
        add ecx,[Window_SizeY]
        mov edx,0x03aabbcc
        mov esi,0x805080d0
        mov edi,0x005080d0
        mcall

        ;call print_controls_information

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

check_box_text db 'Check box',0

text1    db 'text',0
text3    db 'Exit',0

text4    db 'Bookmark_1',0
text5    db 'Bookmark_2',0
text6    db 'Bookmark_3',0
text7    db 'Bookmark_4',0

text8    db 'Bookmark_5',0
text9    db 'Bookmark_6',0

text_for_text db 'Hello world from bookmark',0

fnDestroyControl                     db 'DestroyControl',0
fnSendMessage                        db 'SendMessage',0
fnResizeComponent                    db 'ResizeComponent',0
fnRemoveComponent                    db 'RemoveComponent',0
fnCraeteButton                       db 'CraeteButton',0
fnCraeteScroller                     db 'CraeteScroller',0
fnCraeteBookmark                     db 'CraeteBookmark',0
fnCraeteImage                        db 'CraeteImage',0
fnCraeteText                         db 'CraeteText',0
fnCraeteNumber                       db 'CraeteNumber',0
fnCraeteCheckbox                     db 'CraeteCheckbox',0
fnCraeteEditbox                      db 'CraeteEditbox',0
fnCraeteProgressbar                  db 'CraeteProgressbar',0

myexport                             dd 0

destroy_control                       dd 0
send_message                          dd 0
resize_component                      dd 0
remove_component                      dd 0
craete_button                         dd 0
craete_scroller                       dd 0
craete_bookmark                       dd 0
craete_image                          dd 0
craete_text                           dd 0
craete_number                         dd 0
craete_check_box                      dd 0
craete_edit_box                       dd 0
craete_progres_bar                    dd 0

PointerToControlForButton1           dd 0
PointerToControlForButtonExit        dd 0

PointerToControlForHorizontalScroller dd 0
PointerToControlForVerticalScroller   dd 0

PointerToControlForBookmark          dd 0

PointerToControlForImage             dd 0

PointerToControlForText              dd 0

PointerToControlForCheckBox          dd 0

PointerToControlForEditBox           dd 0


Window_CordinatX                     dd 0
Window_CordinatY                     dd 0

Window_SizeX                         dd 0
Window_SizeY                         dd 0

button_pressed                       dd 0

IPC_table                            rd 256
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

struc BOOKMARK
{
 .type             rb 1
 .flag             rb 1
 .x                rd 1
 .y                rd 1
 .sizex            rd 1
 .sizey            rd 1
 .color_1          rd 1
 .reserved         rd 1
 .color2           rd 1

 .number_bookmarks rd 1
 .number_zak_in_1  rd 1
 .number_zak_in_2  rd 1
 .number_zak_in_3  rd 1

 .text_for_1       rd 1
 .number_contrl_1  rd 1
 .type_control_1   rd 1
 .info_1_control   rd 1

 .text_for_2       rd 1
 .number_contrl_2  rd 1
 .type_control_2   rd 1
 .info_2_control   rd 1

 .text_for_3       rd 1
 .number_contrl_3  rd 1
 .type_control_3   rd 1
 .info_3_control   rd 1

 .text_for_4       rd 1
 .number_contrl_4  rd 1
 .type_control_4   rd 1
 .info_4_control   rd 1

 .text_for_5       rd 1
 .number_contrl_5  rd 1
 .type_control_5   rd 1
 .info_5_control   rd 1

 .text_for_6       rd 1
 .number_contrl_6  rd 1
 .type_control_6   rd 1
 .info_6_control   rd 1

}

struc IMAGE
{
 .type             rb 1
 .flag             rb 1
 .color            rd 1
 .x                rd 1
 .y                rd 1
 .sizex            rd 1
 .sizey            rd 1
 .pointer          rd 1
}

struc TEXT
{
 .type             rb 1
 .flag             rb 1
 .color            rd 1
 .x                rd 1
 .y                rd 1
 .length           rd 1
 .pointer          rd 1
}

struc NUMBER
{
 .type             rb 1
 .flag             rb 1
 .color            rd 1
 .x                rd 1
 .y                rd 1
 .number           rd 1
 .parameters       rd 1
}

struc CHECKBOX
{
 .ch_flags         rw 1
 .ch_left          rw 1
 .ch_top           rw 1
 .ch_text_margin   rd 1
 .ch_size          rd 1
 .ch_size_2        rw 1
 .ch_size_3        rw 1
 .ch_color         rd 1
 .ch_border_color  rd 1
 .ch_text_color    rd 1
 .ch_text_ptr      rd 1
 .ch_text_length   rw 1
 .mouseX           rd 1
 .mouseY           rd 1
}
struc EDITBOX
{
 .ed_width                      rd 1     ;? ?
 .ed_left                       rd 1     ;  ??
 .ed_top                        rd 1     ;  ??
 .ed_color                      rd 1     ;??? ?
 .shift_color                   rd 1     ;?????. ??
 .ed_focus_border_color         rd 1     ;??? ?
 .ed_blur_border_color          rd 1     ;?? ? ?
 .ed_text_color                 rd 1     ;????
 .ed_max                        rd 1     ;- ? ?? ?? ?
 .ed_text                       rd 1     ;??? ??
 .ed_flags                      rw 1     ;?
 .ed_size                       rd 1     ;- ?
 .ed_pos                        rd 1     ;? ??
 .ed_offset                     rd 1     ;??
 .cl_curs_x                     rd 1     ;??? ????  ?
 .cl_curs_y                     rd 1     ;??? ????  ?
 .ed_shift_pos                  rd 1     ; ??
 .ed_shift_pos_old              rd 1     ;??  ??
 .ed_height                     rd 1
 .mouseX                        rd 1
 .mouseY                        rd 1
}

struc PROGRESSBAR
{
 .type                          rb 1
 .flag                          rb 1
 .color1                        rd 1
 .color2                        rd 1
 .x                             rd 1
 .y                             rd 1
 .sizex                         rd 1
 .sizey                         rd 1
 .progres                       rd 1
 .color3                        rd 1
}

Button1             BUTTON
Button3             BUTTON
VerticalScroller    SCROLLER
Bookmark            BOOKMARK
Image               IMAGE
Text                TEXT
Number              NUMBER
CheckBox            CHECKBOX
EditBox             EDITBOX
EditBox2            EDITBOX
ProgressBar         PROGRESSBAR

antibug: rb 2000

Picture:
file 'image.raw'

i_end:
