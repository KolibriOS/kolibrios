;------------------------------------------
;-------------KEYS-------------------------
;------------------------------------------
align 4
keys:

   mcall SF_GET_KEY
   shr eax,8

   cmp al,176
   jne key2
   sub [PosX],20
   jmp change_screen
 key2:
   cmp al,179
   jne key3
   add [PosX],20
   jmp change_screen
 key3:
   cmp al,177
   jne key4
   add [PosY],20
   jmp change_screen
 key4:
  cmp al,178
   jne key5
   sub [PosY],20
   jmp change_screen
 key5:
   cmp al,49 ;1
   jne key6
   mov [k],1
   call scrollbar_move_00
   call drawwin
   jmp change_screen
 key6:
   cmp al,50 ;2
   jne key7
   mov [k],2
   call scrollbar_move_00
   call drawwin
   jmp change_screen
 key7:
   cmp al,52 ;4
   jne key8
   mov [k],4
   call scrollbar_move_00
   call drawwin
   jmp change_screen
 key8:
   cmp al,56 ;8
   jne key9
   mov [k],8
   call scrollbar_move_00
   call drawwin
   jmp change_screen
 key9:
   cmp al,48 ;0
   jne key10
   mov [k],16
   call scrollbar_move_00
   call drawwin
   jmp change_screen
 key10:
   cmp al,255
   jne key11
   call analizing_picture_to_palette
   call drawwin
   jmp change_screen
 key11:
   ;cmp al,27 ;Esc
   ;jne still
   jmp still

    mov eax,CursorsID
    call delete_cursors
close:
    mcall SF_TERMINATE_PROCESS

   jmp still

   change_screen:

   call CalculatePositionScreen
   call MovePictureToWorkScreen
   call draw_scrollers
   call PrintMousePos

   jmp still