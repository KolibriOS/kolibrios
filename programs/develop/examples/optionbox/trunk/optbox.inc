;
;OptionBox for MenuetOS
;
;by Harald Weber
;kitemaster@gmx.at
;
;16.02.07 Heavyiorn - optimizations for kolibri system functions
;
;######### USAGE ############
;include "optionbox.inc"
;
;//in end of draw_window procedure
;draw_optionboxes:      
; mov eax, [pos_x] shl 16 + [pos_y]
; mov cl, buttonid
; mov edx, Text		;pointer to text beginning
;call optionbox
;
;
;Text db 'Optionbox Text',0
;
;//for more info see optxampl.asm
;##########################

optionbox:

mov [optionbox_pos_y],ax
shr eax,16
mov [optionbox_pos_x],ax
mov [optionbox_text],edx
mov [optionbox_id],cl
;######### Draw Checkbox ########
optionbox_draw:

 mov eax, 13
 mov bx, [optionbox_pos_x]
 shl ebx, 16
 add bx, 11
 mov cx, [optionbox_pos_y]
 shl ecx, 16
 add cx, 11
 mov edx,[sc.work_text]
 int 0x40

 mov edx,[sc.work]
 add ebx, 1 shl 16 - 2
 add ecx, 1 shl 16 -2
 int 0x40


;######### Draw Hidden Button ######
optionbox_hid_button:
mov eax,8
mov bx,[optionbox_pos_x]
sub bx,1
shl ebx,16
mov bx,13
mov cx,[optionbox_pos_y]
sub cx,1
shl ecx,16
mov cx,13
mov dl,[optionbox_id]                      ;buttonid
or  edx,0x60000000                         ;button in invisible without frame
int 0x40
;######### Draw X #######
optionbox_x_draw:
mov al,[optionbox_id]
cmp [optionbox_checked],al
jne optionbox_dis
 
 mov eax, 13
 mov bx, [optionbox_pos_x]
 add bx, 3
 shl ebx, 16
 add bx, 5
 mov cx, [optionbox_pos_y]
 add cx, 3
 shl ecx, 16
 add cx, 5
 mov edx,[sc.work_text]
 int 0x40
 
optionbox_dis:

;######### Checkbox Text #####
optionbox_draw_text:
      mov  eax,4                   ; function 4 : write text to window
      mov  bx,[optionbox_pos_x]
      add  bx, 14
      shl  ebx,16
      mov  bx,[optionbox_pos_y] 
      add  bx, 2	                 ; [x start] *65536 + [y start]
      mov  ecx,[sc.work_text]      ; color of text RRGGBB
      or   ecx,0x80000000
      mov  edx,[optionbox_text]      ; pointer to text beginning
      int  0x40

ret

;######### Set OptionBox #####
set_optionbox:
 mov [optionbox_checked],ah
 call draw_optionboxes
ret

;########## Data Area ########

optionbox_pos_x dw ?
optionbox_pos_y dw ?
optionbox_text dd ?
optionbox_checked db ?
optionbox_id db ?