;             program to generate files            ;
;              path to folder in edit1             ;
;              count of files in edit2             ;
; to compile: nasm -f bin GenFiles.asm -o GenFiles ;
use32
	org 0
	db 'MENUET01'
version dd 1
	dd program.start
	dd program.end
	dd program.memory
	dd program.stack
	dd 0,0

include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'

; ------------------------------------- ;
BUTTON_START           = 2
; ------------------------------------- ;
ED_DISABLED            = 100000000000b
; ------------------------------------- ;
EDIT1_MAX_LENGTH       = 1024
EDIT2_MAX_LENGTH       = 10
FILE_NAME_LENGTH       = 256
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
.color                  dd 0x00FFFFFF
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
.color                  dd 0x00FFFFFF
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
program.start:
; LoadLibrary:
        mcall SF_SYS_MISC,SSF_LOAD_DLL, sz_box_lib
        mov    [box_lib], eax

        stdcall   GetProcAddress,sz_edit_box,[box_lib]
        mov    [edit_box_draw], eax

        stdcall   GetProcAddress,sz_edit_box_key,[box_lib]
        mov    [edit_box_key], eax

        stdcall   GetProcAddress,sz_edit_box_mouse,[box_lib]
        mov    [edit_box_mouse], eax

        stdcall   GetProcAddress,sz_edit_box_set_text,[box_lib]
        mov    [edit_box_set_text], eax

        stdcall   GetProcAddress,sz_progressbar_draw,[box_lib]
        mov    [progressbar_draw], eax

        stdcall   GetProcAddress,sz_progressbar_progress,[box_lib]
        mov    [progressbar_progress], eax
; ------------------------------------- ;
; SetEventMask
        mcall SF_SET_EVENTS_MASK, EVM_REDRAW or EVM_KEY or EVM_BUTTON or EVM_MOUSE
; ------------------------------------- ;
align 4
WaitEvent:
        mcall SF_WAIT_EVENT
        call   dword[eax * 4 + Events]
        jmp    WaitEvent
; ------------------------------------- ;
macro CreateNextFile {
; Base36(count)
        mov    eax, [count]
        mov    ecx, 36        ; because our base is 36
        mov    esi, file_name
        mov    edi, esi
        add    esi, 7         ; base36(0xFFFFFFFF) = 1Z141Z3 : 7 simbols
        mov    [esi], byte 0
@@:
        xor    edx, edx
        div    ecx
        dec    esi
        mov    dl, [edx + digits]
        mov    [esi], dl
        test   eax, eax
        jnz    @b
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
        push   dword SSF_CREATE_FILE  ; SubFunction #2 Create/Rewrite file
        mov    ebx, esp
        mcall SF_FILE
        add    esp, 25  ; restore stack
}
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
        stdcall   [progressbar_draw], pb
; ------------------------------------- ;
        cmp    [count], dword 0
        jz     .done
; SetCurrentDirectory
        mcall SF_CURRENT_FOLDER,SSF_SET_CF, [edit1.text]
;
        mov    [status_string], dword sz_doing
        call   DrawStatus
.do:
        CreateNextFile
        test   eax, eax
        jnz    .error
; increase progress
        stdcall   [progressbar_progress], pb
; CheckEvent
        mcall SF_CHECK_EVENT
        call   dword[eax * 4 + Events]
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
        mcall SF_DRAW_TEXT, (297 shl 16) or 38, 0xD0000000, [status_string],, 0x00FFFFFF
        ret
; ------------------------------------- ;
align 4
On_Idle:
        ret
; ------------------------------------- ;
align 4
On_Redraw:
; RedrawStart
        mcall SF_REDRAW,SSF_BEGIN_DRAW
; DrawWindow
        mov    edi, sz_caption
        xor    esi, esi
        mcall SF_CREATE_WINDOW, (50 shl 16) or 360, (50 shl 16) or 88, 0x34FFFFFF
; draw progressbar
        stdcall   [progressbar_draw], pb
; draw edit1
        stdcall   [edit_box_draw], edit1
; draw edit2
        stdcall   [edit_box_draw], edit2
; DrawButton
        mcall SF_DEFINE_BUTTON, (288 shl 16) or 53, (8   shl 16) or 26, BUTTON_START, 0x00DDDDDD
; DrawTexts
;   Path:
        mcall SF_DRAW_TEXT, (8   shl 16) or 11, 0x90000000, sz_path
;   Count:
        mcall , (168 shl 16) or 11,, sz_count
;   Start:
        mcall , (297 shl 16) or 15,, sz_start
; draw status
        call   DrawStatus
; RedrawFinish
        mcall SF_REDRAW,SSF_END_DRAW
        ret
; ------------------------------------- ;
align 4
On_Key:
; GetKeyCode
        mcall SF_GET_KEY
; notify edit1 about key event
        stdcall   [edit_box_key], edit1
; notify edit2 about key event
        stdcall   [edit_box_key], edit2
        ret
; ------------------------------------- ;
align 4
On_Button:
; GetButtonNumber
        mcall SF_GET_BUTTON
        movzx  eax, ah
        call   dword[eax * 4 + ButtonEvents]
        ret
; ------------------------------------- ;
align 4
On_ButtonClose:
; Terminate
        mcall SF_TERMINATE_PROCESS
        ; ret is not needed here because we are not back after terminate
; ------------------------------------- ;
align 4
On_ButtonStart:
        mov    [ButtonEvents.2], dword On_Idle        ; disable ButtonStart | because
        or     [edit1.flags], dword ED_DISABLED       ; disable edit1       |   we will
        or     [edit2.flags], dword ED_DISABLED       ; disable edit2       |     in Action
; redraw edit1 after change flag
        stdcall   [edit_box_draw], edit1
; redraw edit2 after change flag
        stdcall   [edit_box_draw], edit2
        call   DoAction
        mov    [ButtonEvents.2], dword On_ButtonStart ; enable ButtonStart
        and    [edit1.flags], not ED_DISABLED      ; enable edit1
        and    [edit2.flags], not ED_DISABLED      ; enable edit2
; redraw edit1 after change flag
        stdcall   [edit_box_draw], edit1
; redraw edit2 after change flag
        stdcall   [edit_box_draw], edit2
        ret
; ------------------------------------- ;
align 4
On_Mouse:
; notify edit1 about mouse event
        stdcall   [edit_box_mouse], edit1
; notify edit2 about mouse event
        stdcall   [edit_box_mouse], edit2
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
program.end:
; ------------------------------------- ;
text_buffer1 rb EDIT1_MAX_LENGTH+2
text_buffer2 rb EDIT2_MAX_LENGTH+2
file_name    rb FILE_NAME_LENGTH
	rb 256
align 16
program.stack:
program.memory: