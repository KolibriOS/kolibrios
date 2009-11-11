;
;   PROCESS MANAGEMENT
;
;   VTurjanmaa
;   additions by M.Lisovin lisovin@26.ru
;   integrated with load_lib.obj by <Lrz>
;   Compile with FASM for Menuet
;

  use32
  org    0x0
	STACK_SIZE=1024
	offset_y=22		; Correction for skin
	offset_x=5
  db     'MENUET01'              ; 8 byte id
  dd     0x01                    ; header version
  dd     START                   ; start of code
  dd     I_END                   ; size of image
  dd     U_END+STACK_SIZE        ; memory for app
  dd     U_END+STACK_SIZE        ; esp
  dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include '../../../macros.inc'
include '../../../develop/libraries/box_lib/asm/trunk/editbox_ex.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'
display_processes=32            ; number of processes to show
@use_library	;use load lib macros
START:                          ; start of execution

sys_load_library  library_name, cur_dir_path, library_path, system_path, \
err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i
        inc     eax
        jz      close
; calculate window position
; at the center of the screen
    mcall 40,0x27	;set event
    call calculate_window_pos
    
;main loop when process name isn't edited.    
red:    
        mcall   48,3,sc,40
        edit_boxes_set_sys_color edit1,edit1_end,sc	;set color
        check_boxes_set_sys_color check1,check1_end,sc  ;set color
	xor	ebp,ebp
	inc	ebp
;    mov  ebp,1
    call draw_window            ; redraw all window
still:
    mov  eax,23                 ; wait here for event
    mov  ebx,100                ; 1 sec.
    mcall

    dec  eax                  ; redraw request ?
    jz   red
    dec  eax                  ; key in buffer ?
    jz   key
    dec  eax                  ; button in buffer ?
    jz   button

    sub eax,3                 ; If not use mouse - show 
    jnz still_end
        push    dword edit1
        call    [edit_box_mouse]
	push	dword check1
	call	[check_box_mouse]
    jmp still	

still_end:    
    xor  ebp,ebp                ; draw new state of processes
    call draw_window
    jmp  still


  key:                          ; key
    mov  eax,2                  
    mcall

    cmp  ah,184                 ; PageUp
    jz   pgdn
    cmp  ah,183
    jz   pgup                   ; PageDown
    cmp  ah,27
    jz   close                  ; Esc

        push    dword edit1
        call    [edit_box_key]
				; Check ENTER with ed_focus edit_box
    lea  edi,[edit1]
    test word ed_flags,ed_focus
    jz   still_end 
    sub  ah,13                  ; ENTER?
    jz   program_start          ; RUN a program

    jmp  still

  button:                       
; get button id  
    mov  eax,17                 
    mcall
    shr  eax,8                  

;id in [10,50] corresponds to terminate buttons.
    cmp  eax,10
    jb   noterm                 
    cmp  eax,50
    jg   noterm

;calculate button index        
    sub  eax,11
    
;calculate process slot    
    mov  ecx,[tasklist+4*eax]
    
;ignore empty buttons
    test ecx,ecx
    jle  still_end
;terminate application    
    mov  eax,18
    mov  ebx,2
    mcall
    jmp  still_end 

  noterm:

;special buttons
    dec  eax
    jz   close

    sub  eax,50
    jz   pgdn     ;51
    dec  eax
    jz   pgup     ;52
;    dec  eax
;    jz   read_string
    dec  eax
    jz   program_start  ;53
    dec  eax
    jz   reboot         ;54
    jmp  still_end
    
;buttons handlers    

  pgdn:
    sub  [list_start],display_processes
;    cmp  [list_start],0
    jge  still_end  
    mov  [list_start],0
    jmp  still_end  

  pgup:
    mov  eax,[list_add]  ;maximal displayed process slot
    mov  [list_start],eax
    jmp  still_end  
    
  program_start:    
    mov  eax,70
    mov  ebx,file_start
    mcall
    jmp  still_end
    
  reboot:    
    mov  eax,70
    mov  ebx,sys_reboot
    mcall
;close program if we going to reboot

  close:
    or   eax,-1                 ; close this program
    mcall

draw_next_process:
;input:
;  edi - current slot
;  [curposy] - y position
;output:
;  edi - next slot (or -1 if no next slot)
;registers corrupted!
    
;create button
    test  ebp,ebp
    jnz   .nodelete
;delete old button
    mov   eax,8
    mov   edx,[index]
    add   edx,(1 shl 31)+11
    mcall

.nodelete:
;create terminate process button
    mov   eax,8
    mov   ebx,(15-offset_x)*65536+100-offset_y
    mov   ecx,[curposy]
    shl   ecx,16
    mov   cx,10
    mov   edx,[index]
    add   edx,11
    mov   esi,0xaabbcc
;contrast    
    test  dword [index],1
    jz    .change_color_button
    mov   esi,0x8899aa

.change_color_button:
    mcall
    
;draw background for proccess information
    mov   eax,13
    mov   ebx,(115-offset_x)*65536+395
    ;ecx was already set
    mov   edx,0x88ff88
;contrast
    test  dword [index],1
    jz    .change_color_info
    mov   edx,0xddffdd

.change_color_info:
    mcall
    
;nothing else should be done
;if there is no process for this button    
    test  edi,edi
    jl    .ret
    
;find process
.return_1:
    inc   edi
;more comfortable register for next loop    
    mov   ecx,edi
;precacluate pointer to process buffer    
    mov   ebx,process_info_buffer
    
;find process loop

.find_loop:
    cmp   ecx,256
    jge   .no_processes
    
;load process information in buffer
    mov   eax,9
;    mov   ebx,process_info_buffer
    mcall
    
;if current slot greater than maximal slot,
;there is no more proccesses.    
    cmp   ecx,eax
    jg    .no_processes
    
;if slot state is equal to 9, it is empty.    
    cmp   [process_info_buffer+process_information.slot_state],9
    jnz   .process_found
    
    inc   ecx
    jmp   .find_loop
    
.no_processes:
    or   edi,-1
    ret
    
.process_found:
;check on/off check box
    push edi
    lea  edi,[check1]
    test dword ch_flags,ch_flag_en
    pop  edi
    jnz   @f
    cmp   dword [process_info_buffer+10],'ICON'
    jz    .return_1 
    cmp   dword [process_info_buffer+10],'OS/I'
    jz    .return_1
    cmp   byte [process_info_buffer+10],'@'
    jz    .return_1


@@: mov  edi,ecx
    mov  [list_add],ecx
    
;get processor cpeed    
;for percent calculating
    mov  eax,18
    mov  ebx,5
    mcall
    
    xor  edx,edx
    mov  ebx,100
    div  ebx
    
;eax = number of operation for 1% now
;calculate process cpu usage percent
    mov  ebx,eax
    mov  eax,[process_info_buffer+process_information.cpu_usage]
;    cdq
    xor edx,edx ; for CPU more 2 GHz - mike.dld 
 
    div  ebx
    mov  [cpu_percent],eax
    
;set text color to display process information
;([tcolor] variable)
;0%      : black    
;1-80%   : green
;81-100% : red
    test eax,eax
    jnz  .no_black
    mov  [tcolor],eax
    jmp  .color_set

.no_black:   
    cmp  eax,80
    ja   .no_green
    mov  dword [tcolor],0x107a30
    jmp  .color_set

.no_green:
    mov  dword [tcolor],0xac0000
.color_set:

;show slot number
    mov  eax,47                
    mov  ebx,2*65536+1*256
;ecx haven't changed since .process_found    
;    mov  ecx,edi
    mov  edx,[curposy]
    add  edx,(20-offset_x)*65536+1
    mov  esi,[tcolor]
    mcall
    
;show process name
    mov  eax,4
    mov  ebx,[curposy]
    add  ebx,(50-offset_x)*65536+1
    mov  ecx,[tcolor]
    mov  edx,process_info_buffer.process_name
    mov  esi,11
    mcall
    
;show pid
    mov  eax,47
    mov  ebx,8*65536+1*256
    mov  ecx,[process_info_buffer.PID]
    mov  edx,[curposy]
    add  edx,(130-offset_x)*65536+1
    mov  esi,[tcolor]
    mcall
    
;show cpu usage
    mov  ecx,[process_info_buffer.cpu_usage]
    add  edx,60*65536
    mcall
    
;show cpu percent
    mov  ebx,3*65536+0*256
    mov  ecx,[cpu_percent]
    add  edx,60*65536
    mcall
    
;show memory start - obsolete
    mov  ebx,8*65536+1*256
    mov  ecx,[process_info_buffer.memory_start]
    add  edx,30*65536
    mcall
    
;show memory usage
    mov  ecx,[process_info_buffer.used_memory]
    inc  ecx
    add  edx,60*65536
    mcall
    
;show window stack and value
    mov  ecx,dword [process_info_buffer.window_stack_position]
    add  edx,60*65536
    mcall
    
;show window xy size
    mov  ecx,[process_info_buffer.box.left]
    shl  ecx,16
    add  ecx,[process_info_buffer.box.top]
    add  edx,60*65536
    mcall    
            
.ret:
;build index->slot map for terminating processes.
    mov  eax,[index]
    mov  [tasklist+4*eax],edi        
    ret

;read_string:
;clean string
;    mov  edi,start_application
;    xor  eax,eax
;    mov  ecx,60
;    cld
;    rep  stosb
;    call print_text

;    mov  edi,start_application
;edi now contains pointer to last symbol       
;    jmp  still1

;read string main loop

  f11:
;full update  
    push edi
	xor	ebp,ebp
	inc	ebp
;    mov  ebp,1
    call draw_window
    pop  edi
;
;  still1:  
;wait for message  
;    mov  eax,23
;    mov  ebx,100
;    mcall
;    cmp  eax,1
;    je   f11
;if no message - update process information    
;    cmp  eax,0
;    jnz  .message_received
;    push edi                ;edi should be saved since draw_window
;    xor  ebp,ebp            ;corrupt registers
;    call draw_window
;    pop  edi
;    jmp  still1
;    
;.message_received:
;    cmp  eax,2
;    jne  read_done          ;buttons message
;read char    
;    mov  eax,2
;    mcall
;    shr  eax,8
    
;if enter pressed, exit read string loop    
;    cmp  eax,13
;    je   read_done
;if backslash pressed?    
;    cmp  eax,8
;    jnz  nobsl
;decrease pointer to last symbol    
;    cmp  edi,start_application
;    jz   still1
;    dec  edi
;fill last symbol with space because
;print_text show all symbols    
;    mov  [edi],byte 32
;    call print_text
;    jmp  still1
;    
;  nobsl:
;write new symbol  
;    mov  [edi],al
;display new text
;    call print_text
;increment pointer to last symbol
;    inc  edi
;compare with end of string    
;    mov  esi,start_application
;    add  esi,60
;    cmp  esi,edi
;    jnz  still1

;exiting from read string loop
;
;  read_done:
;terminate string for file functions
;    mov  [edi],byte 0

;    call print_text
;    jmp  still

;
;print_text:
;display start_application string

;    pushad
    
;display text background
;    mov  eax,13
;    mov  ebx,64*65536+62*6
;    mov  ecx,400*65536+12
;    mov  edx,0xffffcc  ;0xeeeeee
;    mcall
    
;display text    
;    mov  eax,4                  
;    mov  edx,start_application  ;from start_application string
;    mov  ebx,70*65536+402       ;text center-aligned
;    xor  ecx,ecx                ;black text
;    mov  esi,60                 ;60 symbols
;    mcall

;    popad
;    ret

window_x_size=524
window_y_size=430

calculate_window_pos:
;set window size and position for 0 function
;to [winxpos] and [winypos] variables

;get screen size
    mov  eax,14
    mcall
    mov  ebx,eax
    
;calculate (x_screen-window_x_size)/2    
    shr  ebx,16+1
    sub  ebx,window_x_size/2
    shl  ebx,16
    mov  bx,window_x_size
;winxpos=xcoord*65536+xsize    
    mov  [winxpos],ebx
    
;calculate (y_screen-window_y_size)/2    
    and  eax,0xffff
    shr  eax,1
    sub  eax,window_y_size/2
    shl  eax,16
    mov  ax,window_y_size
;winypos=ycoord*65536+ysize    
    mov  [winypos],eax
    
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

align 16
draw_window:
;ebp=1 - redraw all
;ebp=0 - redraw only process information

    test ebp,ebp
    jz   .show_process_info
    
    mov  eax,12                    ; function 12:tell os about windowdraw
;    mov  ebx,1                     ; 1, start of draw
    xor	 ebx,ebx
    inc  ebx
    mcall                      

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,[winxpos]             ; [x start] *65536 + [x size]
    mov  ecx,[winypos]             ; [y start] *65536 + [y size]
    mov  edx,0x34ddffdd  ;ffffff   ; color of work area RRGGBB,8->color
    mov  edi,title                ; WINDOW CAPTION;
    mcall

                                   
    add  eax,4                     ; function 4 : write text to window
    mov  ebx,(22-offset_x)*65536+35-offset_y           ; draw info text with function 4
    xor  ecx,ecx
    mov  edx,text
    mov  esi,text_len
    mcall

        push    dword edit1
        call    [edit_box_draw]
        push    dword check1
	call	[check_box_draw]

align 16
.show_process_info:
    mov  edi,[list_start]
    mov  [list_add],edi
    mov  dword [index],0
    mov  dword [curposy],54-offset_y

.loop_draw:
    call draw_next_process
    inc  dword [index]
    add  dword [curposy],10
    cmp  [index],display_processes
    jl   .loop_draw
    
    test ebp,ebp
    jz   .end_redraw
    mov  eax,8
    mov  esi,0xaabbcc
                                    
; previous page button
    mov  ebx,(30-offset_x)*65536+96
    mov  ecx,(380-offset_y)*65536+10
    mov  edx,51
    mcall
                                    
; next page button  52
    mov  ebx,(130-offset_x)*65536+96
    inc  edx
    mcall
                              
; ">" (text enter) button
;    mov  ebx,30*65536+20
    add  ecx,20 shl 16
;    inc  edx
;    mcall
                                    
; run button 53
    mov  ebx,(456-offset_x)*65536+50
    inc  edx
    mcall

; reboot button    
    sub  ebx,120*65536              
    add  ebx,60
    sub  ecx,20 shl 16
    inc  edx
    mcall
    
;"PREV PAGE", "NEXT PAGE" and "REBOOT" labels
    mov  eax,4
    mov  ebx,(50-offset_x)*65536+382-offset_y
    xor  ecx,ecx
    mov  edx,tbts
    mov  esi,tbte-tbts
    mcall

;">" labels
;    mov  eax,4
;    mov  ebx,40*65536+402
;    xor  ecx,ecx
;    mov  edx,tbts_2
;    mov  esi,1
;    mcall

;"RUN" labels
;    mov  eax,4
    mov  ebx,(475-offset_x)*65536+402-offset_y
    xor  ecx,ecx
    mov  edx,tbts_3
    mov  esi,tbte_2-tbts_3
    mcall

;print application name in text box
;    call print_text

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall
    
.end_redraw:
    ret


; DATA AREA
system_path      db '/sys/lib/'
library_name     db 'box_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

err_message_found_lib   db 'Sorry I cannot load library box_lib.obj',0
head_f_i:
head_f_l        db 'System error',0
err_message_import      db 'Error on load import library box_lib.obj',0

myimport:   

edit_box_draw   dd      aEdit_box_draw
edit_box_key    dd      aEdit_box_key
edit_box_mouse  dd      aEdit_box_mouse
;version_ed      dd      aVersion_ed

check_box_draw  dd      aCheck_box_draw
check_box_mouse dd      aCheck_box_mouse
;version_ch      dd      aVersion_ch

;option_box_draw  dd      aOption_box_draw
;option_box_mouse dd      aOption_box_mouse
;version_op       dd      aVersion_op

                dd      0
                dd      0

aEdit_box_draw  db 'edit_box',0
aEdit_box_key   db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
;aVersion_ed     db 'version_ed',0

aCheck_box_draw  db 'check_box_draw',0
aCheck_box_mouse db 'check_box_mouse',0
;aVersion_ch      db 'version_ch',0

;aOption_box_draw  db 'option_box_draw',0
;aOption_box_mouse db 'option_box_mouse',0
;aVersion_op       db 'version_op',0
check1 check_box 10,(400-offset_y),6,11,0x80AABBCC,0,0,check_text,check_t_e,ch_flag_en
check1_end:
edit1 edit_box 350,(100-offset_x),(398-offset_y),0xffffff,0x6f9480,0,0xAABBCC,0,start_application_c,start_application,mouse_dd,ed_focus,start_application_e,start_application_e
edit1_end:
list_start  dd 0

sys_reboot:
            dd 7
            dd 0
            dd 0
            dd 0
            dd 0
            db '/sys/end',0

if lang eq de
text:
  db 'NAME/BEENDEN        PID     CPU-LAST   % '
  db 'SPEICHER START/NUTZUNG  W-STACK  W-SIZE'
text_len = $-text

tbts:   db  'SEITE ZURUECK       SEITE VOR                      REBOOT SYSTEM'
tbte:
;tbts_2  db  '>'
tbts_3  db  'START'
tbte_2:
check_text db '@ gehoren/aus'
check_t_e=$-check_text
title  db   'Prozesse  - Ctrl/Alt/Del',0

else if lang eq et
text:
  db 'NIMI/LÕPETA         PID    CPU-KASUTUS %   '
  db 'MÄLU ALGUS/KASUTUS  W-PUHVER  W-SUURUS'
text_len = $-text

tbts:	db  'EELMINE LEHT   JÄRGMINE LEHT                     REBOODI SÜSTEEM'
tbte:
;tbts_2	db  '>'
tbts_3	db  'START'
tbte_2:
check_text db '@ on/off'
check_t_e=$-check_text
title  db   'Protsessid - Ctrl/Alt/Del'

else
text:
  db 'NAME/TERMINATE      PID     CPU-USAGE  %   '
  db 'MEMORY START/USAGE  W-STACK   W-SIZE'
text_len = $-text

tbts:   db  'PREV PAGE       NEXT PAGE                         REBOOT SYSTEM'
tbte:
;tbts_2  db  '>'
tbts_3  db  'RUN'
tbte_2:
check_text db '@ on/off'
check_t_e=$-check_text
title  db   'Processes - Ctrl/Alt/Del',0

end if
file_start: dd 7
            dd 0,0,0,0
start_application: db '/sys/LAUNCHER',0
start_application_e=$-start_application-1
;                   times 60 db 0
rb	60
start_application_c=$-start_application-1

I_END:
sc system_colors
winxpos  rd 1
winypos  rd 1
mouse_dd	rd 1
cpu_percent rd 1
tcolor      rd 1
list_add    rd 1
curposy     rd 1
index       rd 1
tasklist    rd display_processes
process_info_buffer process_information
cur_dir_path    rb 1024
library_path    rb 1024

U_END:
