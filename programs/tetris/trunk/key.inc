UP_KEY equ 178							; the ascii keycodes of some keys
DOWN_KEY equ 177
LEFT_KEY equ 176
RIGHT_KEY equ 179
ENTER_KEY equ 13
ESCAPE_KEY equ 27

key:
		mov  eax,2					; Read key ascii and store it into ah
		int  0x40

		cmp  ah,LEFT_KEY				; Check if left key is pressed
		jne  no_left

			cmp  byte[status],'1'
			jne  no_left
		     go_left:
			dec  dword [current_block_x]
			call check_crash
			jz   no_left
			inc  dword [current_block_x]

no_left:
		cmp  ah,RIGHT_KEY				; Check if right key is pressed
		jne  no_right

			cmp  byte[status],'1'
			jne  no_right
		    go_right:
			inc  dword [current_block_x]
			call check_crash
			jz   no_right
			dec  dword [current_block_x]

no_right:
		cmp  ah,UP_KEY					; Compare pressed key with up key
		jne  no_up					; Jump to nu_up if up key isnt pressed

			cmp byte[status],'0'			; Check if menu is running
			jne no_menu2				; Jump to no_up ifgame isnt running

				cmp  byte[menu],'0'		; Compare menu state with 0
				jne  no_menu0			; Jump to no_menu0 if menu state isnt zero

				mov  byte[menu],'3'		; Change menu state to 2
				call draw_window		; Redraw the window
				jmp  no_menu2			; Jump to no_menu2

			      no_menu0:
				dec  byte[menu] 		; menu state = menu state - 1
				call draw_window		; Redraw the window

	      no_menu2:
		cmp  byte[status],'1'				; Compare game state with 1
		jne  no_up					; Jump to no_up if game state isnt 1 (if game isnt running)

		call rotate_block

no_up:
		cmp  ah,DOWN_KEY				; Check if down key is pressed
		jne  no_down

			cmp  byte[status],'0'
			jne  no_menu3

				cmp  byte[menu],'3'
				jne  no_menu1

				mov  byte[menu],'0'
				call draw_window
				jmp  no_menu3

			      no_menu1:
				inc  byte[menu]
				call draw_window

	      no_menu3:
		cmp  byte[status],'1'
		jne  no_down

		inc  dword [current_block_y]
		call check_crash
		jne  block_crash
		jmp  still


no_down:
		cmp  ah,'n'					 ; Check if n key is pressed
		jne  no_n
		jmp  new_game

no_n:
		cmp  ah,'p'					 ; Check if p key is pressed
		jne  no_p
		cmp  byte[status],'2'
		je   unpause
		cmp  byte[status],'1'				 ; add this two line or p will work
		jne  no_p					 ; when your still in the menu
		mov  byte[status],'2'
		call draw_window
		jmp  attesa
	      unpause:
		mov  byte[status],'1'
		call draw_window
		jmp  still

no_p:
		cmp  byte[status],'0'
		jne  no_menu

no_menu:
		cmp  byte[status],'1'
		jne  no_game

no_game:
		cmp  byte[status],'2'
		jne  no_pause1

no_pause1:
		cmp  ah,ENTER_KEY				; Check if enter key is pressed
		jne  no_enter
		cmp  byte[status],'0'
		jne  no_enter
		call exemenu

no_enter:
		cmp  ah,ESCAPE_KEY				; Check if escape key is pressed
		jne  no_escape


			cmp  byte[status],'0'
			jne  no_instr1
			mov  eax,-1
			int  0x40

		      no_instr1:
			cmp  byte[status],'1'
			jne  no_menu4
			mov  byte[status],'0'
			call draw_window

		      no_menu4:
			cmp  byte[status],'3'
			jne  no_menu5
			mov  byte[status],'0'
			call draw_window

		      no_menu5:
			cmp  byte[status],'4'
			jne  no_escape
			mov  byte[status],'0'
			call draw_window

no_escape:

		cmp  byte[status],'1'
		jne  still
		jmp  scendi

ret

