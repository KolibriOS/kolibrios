;------------------------------------------
;-------------KEYS-------------------------
;------------------------------------------
   keys:

   mov eax,2
   int 0x40
   shr eax,8

   cmp eax,176
   jne key2
   sub [PosX],20
   jmp change_screen
 key2:
   cmp eax,179
   jne key3
   add [PosX],20
   jmp change_screen
 key3:
   cmp eax,177
   jne key4
   add [PosY],20
   jmp change_screen
 key4:
  cmp eax,178
   jne key5
   sub [PosY],20
   jmp change_screen
 key5:
   cmp eax,49
   jne key6
   mov [k],1
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],89
   call drawwin
   jmp change_screen
 key6:
   cmp eax,50
   jne key7
   mov [k],2
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],89
   call drawwin
   jmp change_screen
 key7:
   cmp eax,52
   jne key8
   mov [k],4
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],89
   call drawwin
   jmp change_screen
 key8:
   cmp eax,56
   jne key9
   mov [k],8
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],89
   call drawwin
   jmp change_screen
 key9:
   cmp eax,48
   jne key10
   mov [k],16
   mov [Scroll1CoordinatX],9
   mov [Scroll2CoordinatY],89
   call drawwin
   jmp change_screen
 key10:
   cmp eax,255
   jne key11
   call analizing_picture_to_palette
   call drawwin
   jmp change_screen
 key11:
   cmp eax,27
   jne still

    mov eax,CursorsID
    call delete_cursors

    mov eax,-1
    int 0x40

   jmp still

   change_screen:

   call CalculatePositionScreen
   call MovePictureToWorkScreen
   call draw_scrollers
   call PrintMousePos

   jmp still