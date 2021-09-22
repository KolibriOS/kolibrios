;-----------------------;
; CPU - process manager ;
;-----------------------;

        format  binary as ""

        use32
        org     0x0
        
        db      "MENUET01"              ; 8 byte id
        dd      0x01             ; header version
        dd      START           ; start of code
        dd      IM_END          ; size of image
        dd      U_END           ; memory for app
        dd      stack_area              ; esp
        dd      0x0                     ; boot parameters
        dd      cur_dir_path     ; path
;-------------------------------------------------------------------------------
include "lang.inc"
include "../../../macros.inc"
include "../../../develop/libraries/box_lib/trunk/box_lib.mac"
include "../../../KOSfuncs.inc"
include "../../../load_lib.mac"
;-------------------------------------------------------------------------------
DISPLAY_PROCESSES = 20  ;number of processes to show
;-------------------------------------------------------------------------------
WINDOW.WIDTH = PROCESS_TABLE.WIDTH + 10*2
WINDOW.HEIGHT = WORK_AREA.HEIGHT + 30
WORK_AREA.HEIGHT = CHECKBOX.Y + BUTTON.HEIGHT + 10
PROCESS_TABLE:
        .X = 10
        .Y = 10
        .WIDTH = 640
        .HEIGHT = DISPLAY_PROCESSES * BUTTON.HEIGHT
UNDERTABLE:
        .X = PROCESS_TABLE.X
        .Y = PROCESS_TABLE.Y + PROCESS_TABLE.HEIGHT + 20
BUTTON:
        .WIDTH = 130
        .HEIGHT = 16 + 4
EDITBOX:
        .X = CHECKBOX.X + 100
        .Y = UNDERTABLE.Y + BUTTON.HEIGHT + 20
        .WIDTH = 465
        .HEIGHT = 23
        
CHECKBOX:
        .X = PROCESS_TABLE.X
        .Y = UNDERTABLE.Y + BUTTON.HEIGHT + 25
;-------------------------------------------------------------------------------
@use_library    ;use load lib macros
;-------------------------------------------------------------------------------
        struc   utf8z   string
{
        .       db      string, 0
        .size = $ - . - 1
}

;-------------------------------------------------------------------------------
START:                          ; start of execution
        mcall   SF_SYS_MISC,SSF_HEAP_INIT
        sys_load_library        library_name, library_path, system_path, myimport
        inc     eax
        jz      close
;-------------------------------------------------------------------------------
        mcall   SF_SET_EVENTS_MASK,0x80000027 ;set event
;-------------------------------------------------------------------------------
;set window size and position for 0 function
;to [winxpos] and [winypos] variables
;get screen size
        mcall   SF_GET_SCREEN_SIZE
        mov     ebx,eax
;calculate (x_screen-WINDOW.WIDTH)/2
        shr     ebx,16+1
        sub     ebx,WINDOW.WIDTH/2
        shl     ebx,16
        mov     bx,WINDOW.WIDTH
;winxpos=xcoord*65536+xsize
        mov     [winxpos],ebx
;calculate (y_screen-WINDOW.HEIGHT)/2
        and     eax,0xffff
        shr     eax,1
        sub     eax,WINDOW.HEIGHT/2
        shl     eax,16
        mov     ax,WINDOW.HEIGHT
;winypos=ycoord*65536+ysize
        mov     [winypos],eax
;-------------------------------------------------------------------------------
        init_checkboxes2 check1,check1_end
        mcall   SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,40
        edit_boxes_set_sys_color edit1,edit1_end,sc             ;set color
        ;check_boxes_set_sys_color2 check1,check1_end,sc ;set color
;-------------------------------------------------------------------------------
align 4
;main loop when process name isn"t edited.
red:
        call    draw_window             ; redraw all window
        mcall   71, 2, strings.window_caption, 3        ;set window caption
;-------------------------------------------------------------------------------
align 4
still:
        mcall   SF_WAIT_EVENT_TIMEOUT,100               ; wait here for event 1 sec.

        test    eax,eax
        jz      still_end

        dec     eax                     ; redraw request ?
        jz      red

        dec     eax                     ; key in buffer ?
        jz      key

        dec     eax                     ; button in buffer ?
        jz      button

        push    dword   edit1
        call    [edit_box_mouse]

        push    dword[check1.flags]

        push    dword   check1
        call    [check_box_mouse]

        pop     eax

        cmp     eax, dword[check1.flags]
        jz      still_end

        push    dword   check1
        call    [check_box_draw]
;-------------------------------------------------------------------------------
align 4
show_process_info_1:
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        add     eax, 100
        mov     [time_counter],eax

        call    show_process_info       ; draw new state of processes
        jmp     still
;-------------------------------------------------------------------------------
align 4
still_end:
        mcall   SF_SYSTEM_GET,SSF_TIME_COUNT
        cmp     [time_counter],eax
        ja      still

        add     eax,100
        mov     [time_counter],eax

        call    show_process_info       ; draw new state of processes
        jmp     still
;-------------------------------------------------------------------------------
align 4
key:                            ; key
        mcall   SF_GET_KEY

        cmp     ah,184          ; PageUp
        jz      pgdn

        cmp     ah,183
        jz      pgup                    ; PageDown

        cmp     ah,27
        jz      close                   ; Esc

        push    dword   edit1
        call    [edit_box_key]
                                ; Check ENTER with ed_focus edit_box
        lea     edi,[edit1]
        test    word    ed_flags,ed_focus
        jz      still_end

        sub     ah,13                   ; ENTER?
        jz      program_start           ; RUN a program

        jmp     still
;-------------------------------------------------------------------------------
align 4
button:
; get button id
        mcall   SF_GET_BUTTON
        mov     bl, al ; save mouse button to bl
        shr     eax,8
;id in [10,50] corresponds to terminate buttons.
        cmp     eax,10
        jb      noterm

        cmp     eax,50
        jg      noterm
;calculate button index
        sub     eax,11
;calculate process slot
        mov     ecx,[tasklist+4*eax]
;ignore empty buttons
        test    ecx,ecx
        jle     still_end
        test    bl, bl ; check mouse button
        jz      .terminate
        mov     eax, ecx
        mov     edi, tinfo.params_buf
;; number in eax
;; buffer in edi
; int2str:
        push    0
        mov     ecx, 10
.push:
        xor     edx, edx
        div     ecx
        add     edx, 48
        push    edx
        test    eax, eax
        jnz     .push
.pop:
        pop     eax
        stosb
        test    eax, eax
        jnz     .pop
; launch tinfo app
        mov     ebx, tinfo
        mov     eax, SF_FILE
        int     64
        jmp     show_process_info_1
.terminate:
;terminate application
        mcall   SF_SYSTEM,SSF_TERMINATE_THREAD
        jmp     show_process_info_1
;-------------------------------------------------------------------------------
align 4
noterm:
;special buttons
        dec     eax
        jz      close

        sub     eax,50
        jz      pgdn    ;51

        dec     eax
        jz      pgup    ;52

        dec     eax
        jz      reboot  ;53

        dec     eax
        jz      program_start   ;54

        jmp     still_end
;buttons handlers
;-------------------------------------------------------------------------------
align 4
pgdn:
        sub     [list_start],DISPLAY_PROCESSES
        jge     show_process_info_1
        mov     [list_start],0
        jmp     show_process_info_1
;-------------------------------------------------------------------------------
align 4
pgup:
        mov     eax,[list_add]  ;maximal displayed process slot
        mov     [list_start],eax
        jmp     show_process_info_1
;-------------------------------------------------------------------------------
align 4
program_start:
        mcall   SF_FILE,file_start
        jmp     show_process_info_1
;-------------------------------------------------------------------------------
align 4
reboot:
        mcall   SF_FILE,sys_reboot
;close program if we going to reboot
;-------------------------------------------------------------------------------
align 4
close:
        or      eax,SF_TERMINATE_PROCESS ; close this program
        mcall
;-------------------------------------------------------------------------------
align 4
draw_empty_slot:
        cmp     [draw_window_flag],1
        je      @f
        mov     ecx,[curposy]
        shl     ecx,16
        mov     cx, BUTTON.HEIGHT
        mcall   SF_DRAW_RECT, <141, PROCESS_TABLE.WIDTH-141>, , [bar_bacground_color]
@@:
        ret
;-------------------------------------------------------------------------------
align 4
draw_next_process:
;input:
;       edi - current slot
;       [curposy] - y position
;output:
;       edi - next slot (or -1 if no next slot)
;registers corrupted!

;putting 2 pixels to make the list of buttons visually solid
        mov ecx,[curposy]
        mcall SF_PUT_PIXEL, PROCESS_TABLE.X, , 0x586468
        add ebx, BUTTON.WIDTH
        mcall

;create terminate process button        
        ;mov    ecx,[curposy]
        shl     ecx,16
        mov     cx, BUTTON.HEIGHT
        mov     edx,[index]
        add     edx,11
        mov     esi,0xaabbcc
        test    dword   [index],1
        jz      @f
        mov     esi,0xccddee
@@:
                add     edx,0x80000000 ; delete a button
                mcall   SF_DEFINE_BUTTON ; before create
                sub     edx,0x80000000 ; a new one below
        mcall   SF_DEFINE_BUTTON,<PROCESS_TABLE.X,BUTTON.WIDTH>
        mov     [btn_bacground_color],esi
;draw background for proccess information
        mov     edx,0xDDDddf
        test    dword   [index],1
        jz      @f
        mov     edx,0xFFFfff
@@:
        ;inc    cx

        mcall   SF_DRAW_RECT, <141, PROCESS_TABLE.WIDTH-141>

        mov     [bar_bacground_color],edx
;nothing else should be done if there is no process for this button
        cmp     edi,-1
        jne     .return_1

        call    draw_empty_slot
        or      edi,-1
        jmp     .ret
;-------------------------------------------------------------------------------
align 4
.return_1:
;find process
        inc     edi
;more comfortable register for next loop
        mov     ecx,edi
;precacluate pointer to process buffer
        mov     ebx,process_info_buffer
;-------------------------------------------------------------------------------
align 4
.find_loop:
        cmp     ecx,256
        jge     .no_processes
;load process information in buffer
        mcall   SF_THREAD_INFO
;if current slot greater than maximal slot,
;there is no more proccesses.
        cmp     ecx,eax
        jg      .no_processes
;if slot state is equal to 9, it is empty.
        cmp     [process_info_buffer+process_information.slot_state],9
        jnz     .process_found

        inc     ecx
        jmp     .find_loop
;-------------------------------------------------------------------------------
align 4
.no_processes:
        call    draw_empty_slot
        or      edi,-1
        ret
;-------------------------------------------------------------------------------
align 4
.process_found:
;check on/off check box
        test    dword   [check1.flags], ch_flag_en
        jnz     .no_filter

        cmp     dword   [process_info_buffer+10],"ICON"
        jnz     @f
        cmp     dword   [process_info_buffer+10+4],0
        jz      .return_1
@@:
        cmp     dword   [process_info_buffer+10],"IDLE"
        jnz     @f
        cmp     dword   [process_info_buffer+10+4],0
        jz      .return_1
@@:
        cmp     word    [process_info_buffer+10],"OS"
        jnz     @f
        cmp     dword   [process_info_buffer+10+2],0
        jz      .return_1
@@:
        cmp     byte [process_info_buffer+10],"@"
        jz      .return_1
;-------------------------------------------------------------------------------
align 4
.no_filter:
        mov     edi,ecx
        mov     [list_add],ecx
;get processor cpeed
;for percent calculating
        mcall   SF_SYSTEM,SSF_GET_CPU_FREQUENCY
        xor     edx,edx
        mov     ebx,100
        div     ebx
;eax = number of operation for 1% now
;calculate process cpu usage percent
        mov     ebx,eax
        mov     eax,[process_info_buffer+process_information.cpu_usage]
;       cdq
        xor     edx,edx ; for CPU more 2 GHz - mike.dld
        div     ebx
        mov     [cpu_percent],eax
;set text color to display process information
;0%     : black
;1-80%  : green
;81-100% : red
        test    eax,eax
        jnz     .no_black

        mov     esi, 0x10000000
        jmp     .color_set
;-------------------------------------------------------------------------------
align 4
.no_black:
        cmp     eax,80
        ja      .no_green

        mov     esi, 0x10107A30
        jmp     .color_set
;-------------------------------------------------------------------------------
align 4
.no_green:
        mov     esi,0x10AC0000
;-------------------------------------------------------------------------------
align 4
.color_set:
;show slot number
;ecx haven"t changed since .process_found
        push    edi
        mov     eax, ecx
        mov     ebx, [curposy]
        add     ebx, 40 shl 16 + 3
        mov     ecx, esi
        xor     edx, edx
        call    draw_ra_dec_number
        push    ecx
;show process name
        mov     ebx,[curposy]
        add     ebx,50*65536+3
        mov     ecx, esi
        or      ecx, 0x80000000
        mcall   SF_DRAW_TEXT,,,process_info_buffer.process_name,11
        pop     ecx
;show PTID
        mov     eax, [process_info_buffer.PID]
        add     ebx, 160 shl 16
        xor     edx, edx
        call    draw_ra_dec_number
;show cpu usage
        mov     eax, [process_info_buffer.cpu_usage]
        add     ebx, 100 shl 16
        call    draw_ra_dec_number
;show cpu percent
        mov     eax, [cpu_percent]
        add     ebx, 55 shl 16
        call    draw_ra_dec_number
;show memory usage
        mov     eax, [process_info_buffer.used_memory]
        add     ebx, 60 shl 16
        call    draw_ra_data_size
;show window stack position
        movzx   eax, word [process_info_buffer.window_stack_position]
        add     ebx, 70 shl 16
        call    draw_ra_dec_number
;show window x size
        movzx   eax, word [process_info_buffer.box.left]
        add     ebx, 70 shl 16
        call    draw_ra_dec_number
;show window y size
        movzx   eax, word [process_info_buffer.box.top]
        add     ebx, 70 shl 16
        call    draw_ra_dec_number      
        pop     edi
;-------------------------------------------------------------------------------
align 4
.ret:
;build index->slot map for terminating processes.
        mov     eax,[index]
        mov     [tasklist+4*eax],edi
        ret
;-------------------------------------------------------------------------------
align 4
f11:
;full update
        push    edi
        call    draw_window
        pop     edi
;-------------------------------------------------------------------------------
;       *********************************************
;       ******* WINDOW DEFINITIONS AND DRAW ********
;       *********************************************
align 4
draw_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
; DRAW WINDOW
        xor     eax,eax                  ; function 0 : define and draw window
        xor     esi,esi
        mcall   ,[winxpos],[winypos], 0x24FFFFFF

        mcall   SF_THREAD_INFO,process_info_buffer,-1

        mov     eax,[ebx+70]
        mov     [window_status],eax
        test    [window_status],100b            ; window is rolled up
        jnz     .exit

        test    [window_status],10b             ; window is minimized to panel
        jnz     .exit
        
        mov     eax, strings.process_name
        mov     ebx, 130 shl 16 + 5
        xor     ecx, ecx
        call    draw_ra_text
        
        mov     eax, strings.ptid
        add     ebx, 80 shl 16
        call    draw_ra_text
        
        mov     eax, strings.cpu_usage_cycles
        add     ebx, 100 shl 16
        call    draw_ra_text
        
        mov     eax, strings.cpu_usage_percent
        add     ebx, 55 shl 16
        call    draw_ra_text
        
        mov     eax, strings.memory_usage
        add     ebx, 60 shl 16
        call    draw_ra_text
        
        mov     eax, strings.window_stack_pos
        add     ebx, 70 shl 16
        call    draw_ra_text
        
        mov     eax, strings.window_position.x
        add     ebx, 70 shl 16
        call    draw_ra_text
        
        mov     eax, strings.window_position.y
        add     ebx, 70 shl 16
        call    draw_ra_text

        mcall   SF_SYSTEM_GET,SSF_TIME_COUNT
        add     eax,100
        mov     [time_counter],eax

        mov     [draw_window_flag],1
        call    show_process_info
        mov     [draw_window_flag],0

        push    dword   edit1
        call    [edit_box_draw]

        push    dword   check1
        call    [check_box_draw]

;previous page button, ID = 51:
        mov     eax, strings.previous_page
        mov     ebx, UNDERTABLE.X shl 16 + UNDERTABLE.Y
        mov     ecx, 51
        mov     edx, 0xCCDDEE
        xor     esi, esi
        call    draw_button_with_caption
;next page button, ID = 52:
        mov     eax, strings.next_page
        add     ebx, 10 shl 16
        inc     ecx
        call    draw_button_with_caption
;reboot button, ID = 53:
        mov     eax, strings.reboot
        add     ebx, 345 shl 16
        inc     ecx
        call    draw_button_with_caption
;run button, ID = 54
        mov     eax, strings.run
        mov     ebx, (EDITBOX.X + EDITBOX.WIDTH + 10) shl 16 + (EDITBOX.Y + EDITBOX.HEIGHT/2 - BUTTON.HEIGHT/2)
        inc     ecx
        call    draw_button_with_caption
;-------------------------------------------------------------------------------
align 4
.exit:
        mcall   SF_REDRAW, SSF_END_DRAW
        ret
;-------------------------------------------------------------------------------
align 4
show_process_info:
        test    [window_status], 100b           ; window is rolled up
        jnz     .exit

        test    [window_status], 10b            ; window is minimized to panel
        jnz     .exit

        mov     ecx,DISPLAY_PROCESSES
        mov     edi,tasklist
        xor     eax,eax
        cld
        rep     stosd

        mov     edi,[list_start]
        mov     [list_add],edi
        mov     dword   [index],0
        mov     dword   [curposy],20
;-------------------------------------------------------------------------------
align 4
.loop_draw:
        call    draw_next_process
        inc     dword   [index]
        add     dword   [curposy],16+4
        cmp     [index],DISPLAY_PROCESSES
        jl      .loop_draw
;-------------------------------------------------------------------------------
align 4
.exit:
        ret
        
;-------------------------------------------------------------------------------

draw_ra_dec_number:
;-------------------------------------------------------------------------------
;draws (posfixed) number with flush right alignment in decimal form
;8x16 number + 8x16 UTF8Z text
;in:
;eax = number
;ebx = right margin coordinates (x shl 16 + y)
;ecx = 0x00RRGGBB
;edx = pointer to postfix string or 0 - no postfix
;-------------------------------------------------------------------------------
        pusha

        ror     ebx, 16
        mov     ebp, eax
        
        test    edx, edx
        jz      .no_postfix
        
        mov     eax, edx
        call    count_utf8z_chars
        
        test    eax, eax
        jz      .no_postfix
        push    ecx
        lea     eax, [eax*8]
        sub     bx, ax
        rol     ebx, 16
        or      ecx, 0xB0000000
        mcall   SF_DRAW_TEXT
        ror     ebx, 16
        pop     ecx
        
.no_postfix:
        mov     eax, ebp
        push    edx

        xor     edi, edi
        
        mov     esi, 10
@@:
        xor     edx, edx
        div     esi
        inc     edi
        test    eax, eax
        jz      @f
        jmp     @b

@@:
        pop     edx
        mov     esi, ecx
        or      esi, 0x10000000
        mov     ecx, ebp
        mov     edx, ebx
        lea     eax, [edi*8]
        sub     dx, ax
        rol     edx, 16
        mcall   SF_DRAW_NUMBER, (11 shl 16) or 0x80000000
        
        popa
        ret
;-------------------------------------------------------------------------------

draw_ra_data_size:
;-------------------------------------------------------------------------------
;draws data size with flush right alignment in following form:
;n (for <1024 bytes) or n xB (KB/MB/GB)
;8x16 font
;in:
;eax = number
;ebx = right margin coordinates (x shl 16 + y)
;ecx = 0x00RRGGBB
;-------------------------------------------------------------------------------
        pusha

        xor     edx, edx
        cmp     eax, 1024
        ja      @f
        jmp     .draw_text
        
@@:
        cmp     eax, 1024*1024
        jae     @f
        mov     esi,  1024
        div     esi
        mov     edx, strings.KB
        jmp     .draw_text
        
@@:
        cmp     eax, 1024*1024*1024
        jae     @f
        mov     esi,  1024*1024
        div     esi
        mov     edx, strings.MB
        jmp     .draw_text
        
@@:
        mov     esi,  1024*1024*1024
        div     esi
        mov     edx, strings.GB
        
.draw_text:
        call    draw_ra_dec_number
        
        popa
        ret
;-------------------------------------------------------------------------------

draw_ra_text:
;-------------------------------------------------------------------------------
;draws 8x16 UTF8Z text with flush right alignment in decimal form
;in:
;eax = pointer to text string
;ebx = right margin coordinates (x shl 16 + y)
;ecx = 0x00RRGGBB
;-------------------------------------------------------------------------------
        pusha

        ror     ebx, 16
        mov     edx, eax
        
        call    count_utf8z_chars
        
        test    eax, eax
        jz      .ret
        lea     eax, [eax*8]
        sub     bx, ax
        rol     ebx, 16
        or      ecx, 0xB0000000
        mcall   SF_DRAW_TEXT
        
.ret:
        popa
        ret
;-------------------------------------------------------------------------------

draw_button_with_caption:
;-------------------------------------------------------------------------------
;draws button with 8x16 UTF8Z caption in center
;in:
;eax = pointer to button caption or 0 - no caption
;ebx = x shl 16 + y
;ecx = 0x00XXXXXX, where XXXXXX - button ID
;edx = 0x00RRGGBB - button color
;esi = 0x00RRGGBB - text color
;out:
;eax = width of button
;ebx = x+width shl 16 + y
;-------------------------------------------------------------------------------
        pusha
        
        xor     ebp, ebp
        mov     edi, eax
        test    eax, eax
        jz      .no_caption_0

        call    count_utf8z_chars
        mov     ebp, eax

.no_caption_0:  
        push    ebx esi
        lea     eax, [ebp*8]
        mov     esi, edx
        mov     edx, ecx
        mov     ecx, ebx
        shl     ecx, 16
        mov     bx, ax
        add     bx, 3*2
        movzx   eax, bx
        mov     dword [esp+4*2+4*7], eax        ;out eax = width
        add     word [esp+4*2+4*4+2], ax        ;out ebx = x+width shl 16 + y
        mov     cx, BUTTON.HEIGHT
        mcall   SF_DEFINE_BUTTON
        pop     esi ebx
        test    edi, edi
        jz      .no_caption_1
        mov     edx, edi
        add     ebx, 3 shl 16 + 3
        mov     ecx, esi
        or      ecx, 0xB0000000
        mcall   SF_DRAW_TEXT
        
.no_caption_1:
        popa
        ret
;-------------------------------------------------------------------------------

count_utf8z_chars:
;-------------------------------------------------------------------------------
;in:
;eax = pointer to UTF8Z string
;out:
;eax = count of chars (excluding finishing zero) (0 if string is empty or invalid)
;-------------------------------------------------------------------------------
        push    esi ebx
        mov     esi, eax
        xor     ebx, ebx
        
.0:
        lodsb
        test    al, al
        jz      .ok
        inc     ebx
        cmp     al, 0x7F
        ja      @f
        jmp     .0
@@:
        cmp     al, 0xC0
        jb      .err
        cmp     al, 0xDF
        ja      @f
        inc     esi
        jmp     .0
        
@@:
        cmp     al, 0xEF
        ja      @f
        inc     esi
        inc     esi
        jmp     .0
        
@@:
        cmp     al, 0xF7
        ja      .err
        add     esi, 3
        jmp     .0
        
.ok:
        mov     eax, ebx
        pop     ebx esi
        ret
        
.err:
        xor     eax, eax
        pop     ebx esi
        ret
;-------------------------------------------------------------------------------

; DATA AREA
;-------------------------------------------------------------------------------
system_path     db      "/sys/lib/"
library_name    db      "box_lib.obj", 0

;-------------------------------------------------------------------------------
align 4
myimport:
edit_box_draw           dd      aEdit_box_draw
edit_box_key            dd      aEdit_box_key
edit_box_mouse          dd      aEdit_box_mouse
;version_ed             dd      aVersion_ed

init_checkbox           dd      aInit_checkbox
check_box_draw          dd      aCheck_box_draw
check_box_mouse  dd     aCheck_box_mouse
;version_ch             dd      aVersion_ch

;option_box_draw        dd      aOption_box_draw
;option_box_mouse       dd      aOption_box_mouse
;version_op             dd      aVersion_op

                dd      0
                dd      0

aEdit_box_draw          db      "edit_box",0
aEdit_box_key           db      "edit_box_key",0
aEdit_box_mouse  db     "edit_box_mouse",0
;aVersion_ed            db      "version_ed",0

aInit_checkbox          db      "init_checkbox2",0
aCheck_box_draw  db     "check_box_draw2",0
aCheck_box_mouse        db      "check_box_mouse2",0
;aVersion_ch            db      "version_ch",0

;aOption_box_draw       db      "option_box_draw",0
;aOption_box_mouse      db      "option_box_mouse",0
;aVersion_op            db      "version_op",0
;-------------------------------------------------------------------------------
align 4
check1 check_box2 CHECKBOX.X shl 16 + 12, CHECKBOX.Y shl 16 + 12, 6, 0x80D6DEE7, 0x4C5258, 0xB0000000, strings.checkbox_caption, ch_flag_top
check1_end:
edit1 edit_box EDITBOX.WIDTH, EDITBOX.X, EDITBOX.Y, 0xffffff, 0x6f9480, 0, 0xAABBCC, 0x10000000, start_application_c,\
        start_application,mouse_dd,ed_focus,start_application_e,start_application_e
edit1_end:
list_start      dd      0
;-------------------------------------------------------------------------------
align 4
sys_reboot:
                dd      SSF_START_APP
                dd      0
                dd      0
                dd      0
                dd      0
                db      "/sys/end",0
;-------------------------------------------------------------------------------
strings:
if lang eq de
        .window_caption         utf8z   "Prozesse v0.2.3 - [Ctrl+Alt+Del]"
        
        .process_name           utf8z   "NAME/BEENDEN"
        .ptid                   utf8z   "PID/TID"
        .cpu_usage_cycles       utf8z   "CPU(ZYKLEN)"
        .cpu_usage_percent      utf8z   "CPU(%)"
        .memory_usage           utf8z   "SPEICHER"
        .window_stack_pos       utf8z   "W-STACK"
        .window_position.x      utf8z   "  WIN-X"
        .window_position.y      utf8z   "  WIN-Y"
        
        .previous_page          utf8z   "SEITE ZURUECK"
        .next_page              utf8z   "SEITE VOR"
        .reboot                 utf8z   "REBOOT SYSTEM"
        .run                    utf8z   "START"
        
        .checkbox_caption       utf8z   "System"
        
        .KB                     utf8z   " KB"
        .MB                     utf8z   " MB"
        .GB                     utf8z   " GB"
;-------------------------------------------------------------------------------
else if lang eq et
        .window_caption         utf8z   "Protsessid v0.2.3 - [Ctrl+Alt+Del]"
        
        .process_name           utf8z   "NIMI/LÕPETA"
        .ptid                   utf8z   "PID/TID"
        .cpu_usage_cycles       utf8z   "CPU(TSÜKLID)"
        .cpu_usage_percent      utf8z   "CPU(%)"
        .memory_usage           utf8z   "MÄLU"
        .window_stack_pos       utf8z   "W-PUHVER"
        .window_position.x      utf8z   "  WIN-X"
        .window_position.y      utf8z   "  WIN-Y"
        
        .previous_page          utf8z   "EELMINE LEHT"
        .next_page              utf8z   "JÄRGMINE LEHT"
        .reboot                 utf8z   "REBOODI SÜSTEEM"
        .run                    utf8z   "START"
        
        .checkbox_caption       utf8z   "System"
                
        .KB                     utf8z   " KB"
        .MB                     utf8z   " MB"
        .GB                     utf8z   " GB"
;-------------------------------------------------------------------------------
else if lang eq ru
        .window_caption         utf8z   "Диспетчер процессов v0.2.3 - [Ctrl+Alt+Del]"
        
        .process_name           utf8z   "ИМЯ/ЗАВЕРШИТЬ"
        .ptid                   utf8z   "PID/TID"
        .cpu_usage_cycles       utf8z   "CPU(ТАКТЫ)"
        .cpu_usage_percent      utf8z   "CPU(%)"
        .memory_usage           utf8z   "ПАМЯТЬ"
        .window_stack_pos       utf8z   "W-STACK"
        .window_position.x      utf8z   "  WIN-X"
        .window_position.y      utf8z   "  WIN-Y"
        
        .previous_page          utf8z   "ПРЕД. СТР."
        .next_page              utf8z   "СЛЕД. СТР."
        .reboot                 utf8z   "ПЕРЕЗАГРУЗКА"
        .run                    utf8z   "ЗАПУСК"
        
        .checkbox_caption       utf8z   "Системные"
                
        .KB                     utf8z   " КБ"
        .MB                     utf8z   " МБ"
        .GB                     utf8z   " ГБ"
;-------------------------------------------------------------------------------
else if lang eq it
        .window_caption         utf8z   "Gestore processi v0.2.3 - [Ctrl+Alt+Del]"
        
        .process_name           utf8z   "NOME-PROGRAMMA"
        .ptid                   utf8z   "PID/TID"
        .cpu_usage_cycles       utf8z   "CPU(CICLI)"
        .cpu_usage_percent      utf8z   "CPU(%)"
        .memory_usage           utf8z   "MEMORY"
        .window_stack_pos       utf8z   "W-STACK"
        .window_position.x      utf8z   "  WIN-X"
        .window_position.y      utf8z   "  WIN-Y"
        
        .previous_page          utf8z   "INDIETRO"
        .next_page              utf8z   "AVANTI"
        .reboot                 utf8z   "RIAVVIA SISTEMA"
        .run                    utf8z   "START"
        
        .checkbox_caption       utf8z   "System"
        
        .KB                     utf8z   " KB"
        .MB                     utf8z   " MB"
        .GB                     utf8z   " GB"
;-------------------------------------------------------------------------------
else
        .window_caption         utf8z   "Process manager v0.2.3 - [Ctrl+Alt+Del]"
        
        .process_name           utf8z   "NAME/TERMINATE"
        .ptid                   utf8z   "PID/TID"
        .cpu_usage_cycles       utf8z   "CPU(CYCLES)"
        .cpu_usage_percent      utf8z   "CPU(%)"
        .memory_usage           utf8z   "MEMORY"
        .window_stack_pos       utf8z   "W-STACK"
        .window_position.x      utf8z   "  WIN-X"
        .window_position.y      utf8z   "  WIN-Y"
        
        
        .previous_page          utf8z   "PREV PAGE"
        .next_page              utf8z   "NEXT PAGE"
        .reboot                 utf8z   "REBOOT SYSTEM"
        .run                    utf8z   "RUN"
        
        .checkbox_caption       utf8z   "System"
        
        .KB                     utf8z   " KB"
        .MB                     utf8z   " MB"
        .GB                     utf8z   " GB"
end if
;-------------------------------------------------------------------------------
align 4
tinfo:
                        dd      SSF_START_APP
                        dd      0
.params         dd      .params_buf
                        dd      0
                        dd      0
                        db      0
.file_path              dd      sz_tinfo_file_path
align 4
.params_buf:
times 11 db     0 ; at now 4 bytes will be enough, but may be in the future not
align 4
sz_tinfo_file_path      db      "/sys/tinfo",0
;-------------------------------------------------------------------------------
align 4
file_start:
        dd      SSF_START_APP
        dd      0
        dd      0
        dd      0
        dd      0
start_application: db   "/sys/LAUNCHER",0
start_application_e=$-start_application-1
;                       times 60 db     0
        rb      60
start_application_c=$-start_application-1
;-------------------------------------------------------------------------------
IM_END:
;-------------------------------------------------------------------------------
align 4
sc system_colors
winxpos  rd     1
winypos  rd     1
mouse_dd        rd      1
cpu_percent     rd      1
list_add        rd      1
curposy  rd     1
index           rd      1
tasklist        rd      DISPLAY_PROCESSES
time_counter    rd      1

window_status           rd      1
client_area_x_size      rd      1
client_area_y_size      rd      1
bar_bacground_color     rd      1
btn_bacground_color     rd      1
draw_window_flag        rd      1
;-------------------------------------------------------------------------------
align 4
library_path:
process_info_buffer process_information
;-------------------------------------------------------------------------------
align 4
cur_dir_path:
        rb      1024
        rb      1024
stack_area:
U_END:
