;load cursors from memory
;IN
; eax- pointer to memory for ID of cursors
;OUT
; not returned value

load_cursors:

      mov [PointerToCursorsID],eax

      mov eax, 37
      mov ebx, 4
      mov ecx,brush_cursor
      mov edx,1
      mcall

      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      mov eax, 37
      mov ebx, 4
      mov ecx,flood_fill_cursor
      mov edx,1
      mcall

      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      mov eax, 37
      mov ebx, 4
      mov ecx,lastik_cursor
      mov edx,1
      mcall

      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4


      mov eax, 37
      mov ebx, 4
      mov ecx,other_cursor
      mov edx,1
      mcall


      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      mov eax, 37
      mov ebx, 4
      mov ecx,pencil_cursor
      mov edx,1
      mcall


      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      mov eax, 37
      mov ebx, 4
      mov ecx,pipette_cursor
      mov edx,1
      mcall

      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      mov eax, 37
      mov ebx, 4
      mov ecx,spray_cursor
      mov edx,1
      mcall

      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      mov eax, 37
      mov ebx, 4
      mov ecx,zoom_cursor
      mov edx,1
      mcall

      mov ebx,[PointerToCursorsID]
      mov [ebx],eax
      add [PointerToCursorsID],4

      ret

;set cirsor
;IN
;eax- pointer to IDs of cursors
;ebx- number(ID) of current instrument

set_cursor:

      mov [PointerToCursorsID],eax

      cmp ebx,10
      jne no_1_l
       mov eax,5
       jmp che
      no_1_l:

      cmp ebx,11
      jne no_2_l
       mov eax,1
       jmp che
      no_2_l:

      cmp ebx,12
      jne no_3_l
       mov eax,7
       jmp che
      no_3_l:

      cmp ebx,13
      jne no_4_l
       mov eax,3
       jmp che
      no_4_l:

      cmp ebx,14
      jne no_5_l
       mov eax,2
       jmp che
      no_5_l:

      cmp ebx,15
      jne no_6_l
       mov eax,6
       jmp che
      no_6_l:

       cmp ebx,16
      jne no_7_l
       mov eax,4
       jmp che
      no_7_l:

      cmp ebx,17
      jne no_8_l
       mov eax,4
       jmp che
      no_8_l:

      cmp ebx,18
      jne no_9_l
       mov eax,4
       jmp che
      no_9_l:

       cmp ebx,19
      jne no_10_l
       mov eax,4
       jmp che
      no_10_l:

      cmp ebx,20
      jne no_11_l
       mov eax,4
       jmp che
      no_11_l:

      cmp ebx,21
      jne no_12_l
       mov eax,4
       jmp che
      no_12_l:

      cmp ebx,22
      jne no_13_l
       mov eax,4
       jmp che
      no_13_l:

      cmp ebx,23
      jne no_14_l
       mov eax,4
       jmp che
      no_14_l:

      cmp ebx,24
      jne no_15_l
       mov eax,4
       jmp che
      no_15_l:

      cmp ebx,25
      jne no_16_l
       mov eax,4
       jmp che
      no_16_l:

      cmp ebx,26
      jne no_17_l
       mov eax,4
       jmp che
      no_17_l:

      cmp ebx,27
      jne no_18_l
       mov eax,4
       jmp che
      no_18_l:

      cmp ebx,28
      jne no_19_l
       mov eax,4
       jmp che
      no_19_l:


      cmp ebx,45
      jne no_25_l
       mov eax,8
       jmp che
      no_25_l:

      cmp ebx,46
      jne no_26_l
       mov eax,8
       jmp che
      no_26_l:

       cmp ebx,47
      jne no_27_l
       mov eax,8
       jmp che
      no_27_l:

      cmp ebx,48
      jne no_28_l
       mov eax,8
       jmp che
      no_28_l:

       cmp ebx,49
      jne no_29_l
       mov eax,8
       jmp che
      no_29_l:


      cmp ebx,50
      jne no_30_l
       mov eax,8
       jmp che
      no_30_l:

      cmp ebx,40
      je no_change_cursor

      cmp ebx,41
      je no_change_cursor

      cmp ebx,42
      je no_change_cursor

      cmp ebx,43
      je no_change_cursor

      cmp ebx,44
      je no_change_cursor

      cmp ebx,52
      je no_change_cursor

      cmp ebx,51
      je no_change_cursor

      cmp ebx,52
      je no_change_cursor

      che:

      dec eax
      shl eax,2
      mov ebx,[PointerToCursorsID]
      add eax,ebx
      mov ecx,[eax]

      mov eax, 37
      mov ebx, 5
      mcall

      no_change_cursor:

      ret

;delete cursors
;IN
;eax- pointer to IDs of cursors
;OUT
;not returned value

delete_cursors:

     mov [PointerToCursorsID],eax
     mov edx,8

  next_cursor_delete:

     mov eax,[PointerToCursorsID]
     mov ecx,[eax]

     mov eax,37
     mov ebx,6
     mcall

     add [PointerToCursorsID],4

     dec edx
     jnz next_cursor_delete

     ret

PointerToCursorsID  dd 0
