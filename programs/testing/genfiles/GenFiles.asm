;             program to generate files            ;
;              path to folder in edit1             ;
;              count of files in edit2             ;
; to compile: nasm -f bin GenFiles.asm -o GenFiles ;
ORG 0
BITS 32
; ------------------------------------- ;
STACK_SIZE             equ 256
; ------------------------------------- ;
EM_REDRAW              equ 0b1
EM_KEY                 equ 0b10
EM_BUTTON              equ 0b100
EM_MOUSE               equ 0b100000
; ------------------------------------- ;
BUTTON_START           equ 2
; ------------------------------------- ;
ED_DISABLED            equ 0b100000000000
; ------------------------------------- ;
EDIT1_MAX_LENGTH       equ 1024
EDIT2_MAX_LENGTH       equ 10
FILE_NAME_LENGTH       equ 256
; ------------------------------------- ;
text_buffer1           equ END + STACK_SIZE
text_buffer2           equ END + STACK_SIZE + (EDIT1_MAX_LENGTH + 2)
file_name              equ END + STACK_SIZE + (EDIT1_MAX_LENGTH + 2) + (EDIT2_MAX_LENGTH + 2)
; ------------------------------------- ;
MENUET01                db 'MENUET01'
version                 dd 1
program.start           dd START
program.end             dd END
program.memory          dd END + STACK_SIZE + (EDIT1_MAX_LENGTH + 2) + (EDIT2_MAX_LENGTH + 2) + FILE_NAME_LENGTH
program.stack           dd END + STACK_SIZE
program.params          dd 0
program.path            dd 0
; ------------------------------------- ;
align 4
Events:
.0:                     dd  On_Idle
.1:                     dd  On_Redraw
.2:                     dd  On_Key
.3:                     dd  On_Button
.4:                     dd  0
.5:                     dd  0
.6:                     dd  On_Mouse
; ------------------------------------- ;
ButtonEvents:
.0:                     dd  0
.1:                     dd  On_ButtonClose
.2:                     dd  On_ButtonStart
; ------------------------------------- ;
count                   dd  0
; ------------------------------------- ;
pb:
.value                  dd 0
.left                   dd 8
.top                    dd 38
.width                  dd 269
.height                 dd 15
.style                  dd 0
.min                    dd 0
.max                    dd 0
.back_color             dd 0x00C8D0D4
.progress_color         dd 0x8072B7EB
.frame_color            dd 0x00406175
; ------------------------------------- ;
edit1:
.width                  dd 100
.left                   dd 48
.top                    dd 8
.color                  dd 0X00FFFFFF
.shift_color            dd 0x94AECE
.focus_border_color     dd 0
.blur_border_color      dd 0
.text_color             dd 0x10000000
.max                    dd EDIT1_MAX_LENGTH
.text                   dd text_buffer1
.mouse_variable         dd 0
.flags                  dd 10b ; active
.size                   dd 0
.pos                    dd 0
.offset                 dd 0
.cl_curs_x              dd 0
.cl_curs_y              dd 0
.shift                  dd 0
.shift_old              dd 0
; ------------------------------------- ;
edit2:
.width                  dd 60
.left                   dd 216
.top                    dd 8
.color                  dd 0X00FFFFFF
.shift_color            dd 0x94AECE
.focus_border_color     dd 0
.blur_border_color      dd 0
.text_color             dd 0x10000000
.max                    dd EDIT2_MAX_LENGTH
.text                   dd text_buffer2
.mouse_variable         dd 0
.flags                  dd 1000000000000000b ; only numbers
.size                   dd 0
.pos                    dd 0
.offset                 dd 0
.cl_curs_x              dd 0
.cl_curs_y              dd 0
.shift                  dd 0
.shift_old              dd 0
; ------------------------------------- ;
progressbar_progress    dd 0
progressbar_draw        dd 0
; ------------------------------------- ;
edit_box_draw           dd 0
edit_box_key            dd 0
edit_box_mouse          dd 0
edit_box_set_text       dd 0
; ------------------------------------- ;
status_string           dd sz_empty
; ------------------------------------- ;
box_lib                 dd 0
; ------------------------------------- ;
sz_box_lib              db "/sys/lib/box_lib.obj",0
; ------------------------------------- ;
sz_progressbar_draw     db 'progressbar_draw', 0
sz_progressbar_progress db 'progressbar_progress', 0
; ------------------------------------- ;
sz_edit_box             db "edit_box",0
sz_edit_box_key         db "edit_box_key",0
sz_edit_box_mouse       db "edit_box_mouse",0
sz_edit_box_set_text    db "edit_box_set_text",0
; ------------------------------------- ;
sz_path                 db "Path:",0
sz_count                db "Count:",0
sz_start                db "Start",0
sz_caption              db "Generate files",0
; ------------------------------------- ;
sz_empty                db "",0
sz_doing                db "doing",0
sz_done                 db "done ",0
sz_error                db "error",0
; ------------------------------------- ;
digits                  db "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
; ------------------------------------- ;
START:
; LoadLibrary:
        mov    eax, 68
        mov    ebx, 19
        mov    ecx, sz_box_lib
        int    64
        mov    [box_lib], eax

        push   dword[box_lib]
        push   sz_edit_box
        call   GetProcAddress
        mov    [edit_box_draw], eax

        push   dword[box_lib]
        push   sz_edit_box_key
        call   GetProcAddress
        mov    [edit_box_key], eax

        push   dword[box_lib]
        push   sz_edit_box_mouse
        call   GetProcAddress
        mov    [edit_box_mouse], eax

        push   dword[box_lib]
        push   sz_edit_box_set_text
        call   GetProcAddress
        mov    [edit_box_set_text], eax

        push   dword[box_lib]
        push   sz_progressbar_draw
        call   GetProcAddress
        mov    [progressbar_draw], eax

        push   dword[box_lib]
        push   sz_progressbar_progress
        call   GetProcAddress
        mov    [progressbar_progress], eax
; ------------------------------------- ;
; SetEventMask
        mov    eax, 40
        mov    ebx, EM_REDRAW | EM_KEY | EM_BUTTON | EM_MOUSE
        int    64
; ------------------------------------- ;
align 4
WaitEvent:
        mov    eax, 10
        int    64
        call   [eax * 4 + Events]
        jmp    WaitEvent
; ------------------------------------- ;
%macro CreateNextFile 0
; Base36(count)
        mov    eax, [count]
        mov    ecx, 36        ; because our base is 36
        mov    esi, file_name
        mov    edi, esi
        add    esi, 7         ; base36(0xFFFFFFFF) = 1Z141Z3 : 7 simbols
        mov    [esi], byte 0
%%next:
        xor    edx, edx
        div    ecx
        dec    esi
        mov    dl, [edx + digits]
        mov    [esi], dl
        test   eax, eax
        jnz    %%next
        mov    eax, esi
        sub    eax, edi
        mov    ecx, 7 + 1
        sub    ecx, eax
        rep    movsb
; ------------------------------------- ;
        push   dword file_name
        dec    esp
        mov    [esp], byte 0
        push   dword 0
        push   dword 0
        push   dword 0
        push   dword 0
        push   dword 2  ; SubFunction #2 Create/Rewrite file
        mov    ebx, esp
        mov    eax, 70  ; Function #70
        int    64
        add    esp, 25  ; restore stack
%endmacro
; ------------------------------------- ;
align 4
DoAction:
; convert text in edit2 to number
        mov    edx, [edit2.text]
        xor    eax, eax
        xor    ecx, ecx
.convert:
        mov    al, [edx]
        test   al, al
        jz     .converted
        lea    ecx, [ecx + ecx * 4]
        lea    ecx, [eax + ecx * 2 - 48]
        inc    edx
        jmp    .convert
.converted:
        mov    [count], ecx
; ------------------------------------- ;
        mov    [pb.max], ecx
        mov    [pb.value], dword 0
; draw progressbar
        push   pb
        call   [progressbar_draw]
; ------------------------------------- ;
        cmp    [count], dword 0
        jz     .done
; SetCurrentDirectory
        mov    eax, 30
        mov    ebx, 1
        mov    ecx, [edit1.text]
        int    64
;
        mov    [status_string], dword sz_doing
        call   DrawStatus
.do:
        CreateNextFile
        test   eax, eax
        jnz    .error
; increase progress
        push   pb
        call   [progressbar_progress]
; CheckEvent
        mov    eax, 11
        int    64
        call   [eax * 4 + Events]
        dec    dword [count]
        jnz   .do
.done:
        mov    [status_string], dword sz_done
        call   DrawStatus
								ret
.error:
        mov    [status_string], dword sz_error
        call   DrawStatus
								ret
; ------------------------------------- ;
DrawStatus:
        mov    eax, 4
        mov    ecx, 0xD0000000
        mov    ebx, (297 << 16) | 38
        mov    edx, [status_string]
        mov    edi, 0x00FFFFFF
        int    64
        ret
; ------------------------------------- ;
align 4
On_Idle:
        ret
; ------------------------------------- ;
align 4
On_Redraw:
; RedrawStart
        mov    eax, 12
        mov    ebx, 1
        int    64
; DrawWindow
        xor    eax, eax
        mov    ebx, (50 << 16) | 360
        mov    ecx, (50 << 16) | 88
        mov    edx, 0x34FFFFFF
        mov    edi, sz_caption
        xor    esi, esi
        int    64
; draw progressbar
        push   pb
        call   [progressbar_draw]
; draw edit1
        push   edit1
        call   [edit_box_draw]
; draw edit2
        push   edit2
        call   [edit_box_draw]
; DrawButton
        mov    eax, 8
        mov    ecx, (8   << 16) | 26
        mov    ebx, (288 << 16) | 53
        mov    edx, BUTTON_START
        mov    esi, 0x00DDDDDD
        int    64
; DrawTexts
        mov    eax, 4
        mov    ecx, 0x90000000
;   Path:
        mov    ebx, (8   << 16) | 11
        mov    edx, sz_path
        int    64
;   Count:
        mov    ebx, (168 << 16) | 11
        mov    edx, sz_count
        int    64
;   Start:
        mov    ebx, (297 << 16) | 15
        mov    edx, sz_start
        int    64
; draw status
        call   DrawStatus
; RedrawFinish
        mov    eax, 12
        mov    ebx, 2
        int    64
        ret
; ------------------------------------- ;
align 4
On_Key:
; GetKeyCode
        mov    eax, 2
        int    64
; notify edit1 about key event
        push   edit1
        call   [edit_box_key]
; notify edit2 about key event
        push   edit2
        call   [edit_box_key]
        ret
; ------------------------------------- ;
align 4
On_Button:
; GetButtonNumber
        mov    eax, 17
        int    64
        movzx  eax, ah
        call   [eax * 4 + ButtonEvents]
        ret
; ------------------------------------- ;
align 4
On_ButtonClose:
; Terminate
        or     eax, -1
        int    64
        ; ret is not needed here because we are not back after terminate
; ------------------------------------- ;
align 4
On_ButtonStart:
        mov    [ButtonEvents.2], dword On_Idle        ; disable ButtonStart | because
        or     [edit1.flags], dword ED_DISABLED       ; disable edit1       |   we will
        or     [edit2.flags], dword ED_DISABLED       ; disable edit2       |     in Action
; redraw edit1 after change flag
        push   edit1
        call   [edit_box_draw]
; redraw edit2 after change flag
        push   edit2
        call   [edit_box_draw]
        call   DoAction
        mov    [ButtonEvents.2], dword On_ButtonStart ; enable ButtonStart
        and    [edit1.flags], dword ~ED_DISABLED      ; enable edit1
        and    [edit2.flags], dword ~ED_DISABLED      ; enable edit2
; redraw edit1 after change flag
        push   edit1
        call   [edit_box_draw]
; redraw edit2 after change flag
        push   edit2
        call   [edit_box_draw]
        ret
; ------------------------------------- ;
align 4
On_Mouse:
; notify edit1 about mouse event
        push   edit1
        call   [edit_box_mouse]
; notify edit2 about mouse event
        push   edit2
        call   [edit_box_mouse]
        ret
; ------------------------------------- ;
align 4
GetProcAddress:
        mov    edx, [esp + 8]
        xor    eax, eax
        test   edx, edx
        jz     .end
.next:
        xor    eax, eax
        cmp    [edx], dword 0
        jz     .end
        mov    esi, [edx]
        mov    edi, [esp + 4]
.next_:
        lodsb
        scasb
        jne    .fail
        test   al, al
        jnz    .next_
        jmp    .ok
.fail:
        add    edx, 8
        jmp    .next
.ok:
        mov    eax, [edx + 4]
.end:
        ret    8
; ------------------------------------- ;
align 4
END: