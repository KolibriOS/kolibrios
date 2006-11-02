; <--- description --->
; compiler:     FASM 1.50
; name:         FreeCell for MeOS
; version:      1.00
; last update:  21/07/2004
; written by:   Alexandr Gorbovets
; e-mail:       gorsash@mail.ru


include "macros.inc"
meos_app_start

code
   call    randomize
   call    draw_window

  wait_event:
    mov     eax, 10
    int     0x40

    cmp     eax, 1	     ;   if event == 1
    je	    redraw	     ;     jump to redraw handler
    cmp     eax, 2	     ;   else if event == 2
    je	    key 	     ;     jump to key handler
    cmp     eax, 3	     ;   else if event == 3
    je	    button	     ;     jump to button handler


    jmp     wait_event	;else return to the start of main cycle


  redraw:		     ; redraw event handler
    call    draw_window
    jmp     wait_event


  key:			     ; key event handler
    mov     eax, 2	     ;   get key code
    int     0x40

    jmp     wait_event

  button:		     ; button event handler
    mov     eax, 17	     ;   get button identifier
    int     0x40

    cmp     ah, 1
    je	    exit_app	     ;   return if button id != 1

    cmp     ah, 1 + 8
    jbe     common_card      ;   if 1 < ah <= 9

    cmp     ah, 1 + 8 + 4    ;   if 9 < ah <= 13
    jbe     temp_cell

    cmp     ah, 1 + 8 + 8
    jbe     home_cell

    cmp     ah, 1 + 8 + 4 + 4 + 1
    je	    new_game_button

    cmp     ah, 1 + 8 + 4 + 4 + 2
    je	    exit_app


    jmp     wait_event


  exit_app:
    mov     eax, -1	     ;   exit application
    int     0x40

  common_card:
    sub     ah, 2	     ;going from number of card to number of column
    mov     [columnclicked], 0
    mov     byte [columnclicked], ah
    call    common_card_click
    jmp     wait_event

  temp_cell:
    sub     ah, 2 + 8
    mov     [columnclicked], 0
    mov     byte [columnclicked], ah
    call    temp_cell_click
    jmp     wait_event


  home_cell:
    sub    ah, 2 + 8 + 4
    mov    [columnclicked], 0
    mov    byte [columnclicked], ah
    call   home_cell_click
    jmp    wait_event

  new_game_button:
    call   new_game_click
    jmp    wait_event


;******************************************************************************
;                            common_card_click(columnclicked)
  common_card_click:

			     ; counting code of card, that has been clicked
    mov    eax, [columnclicked]
    mov    [ncolumn], eax
    call   get_row_of_top_card_in_column
    mov    eax, [topcardrow]  ; eax = topcardrow * 8 + columnofselcard
    mov    bl, 8
    mul    bl
    add    eax, [columnclicked]
    add    eax, cards

    mov    ebx, 0
    mov    bl, byte [eax]
    mov    [cardclicked], ebx


    call   get_sel_card_code_and_addr

    cmp    [selcardcode], 52
    jb	    .something_selected


    cmp    [cardclicked], 52
    je	    .end

    mov    [whereisselcard], scCommonCells
    mov    eax, [columnclicked]
    mov    [columnofselcard], eax
    call   draw_window
    jmp    .end


    .something_selected:


	     ; checking if selected and clicked cards are equivalent
      mov     eax, [selcardcode]
      cmp     [cardclicked], eax
      jne     .not_same_card

      mov     [whereisselcard], scNotSelected
      call    draw_window
      jmp     .end

    .not_same_card:

      cmp     [cardclicked], 52
      jae     .put_in_blank_cell


      mov     eax, [selcardcode]
      mov     bl, 4
      div     bl

      mov     ebx, 0
      mov     bl, ah
      mov     [cardfamily], ebx

      mov     ecx, 0
      mov     cl, al
      mov     [cardrange], ecx


      mov     eax, [cardclicked]
      mov     bl, 4
      div     bl		     ; reminder in ah, quotient in al

      mov     ebx, 0
      mov     bl, ah
      mov     [clickedcardfamily], ebx

      mov     ecx, 0
      mov     cl, al
      mov     [clickedcardrange], ecx

			     ; clickedcardrange must be = cardrange + 1
      mov     eax, [cardrange]
      inc     eax

      cmp     [clickedcardrange], eax ; eax is such as needed
      jne     .end


      cmp     [cardfamily], 1
      ja	     .black_card

			     ; if selected red card
      cmp     [clickedcardfamily], 1
      jbe     .end	       ; if clicked red card (range <= 1) then exit

      jmp     .valid_cards

    .black_card:
			     ; if selected black card
      cmp     [clickedcardfamily], 1
      ja      .end	       ; if clicked black card then exit

      jmp     .valid_cards

    .valid_cards:
		      ; moving card from its place on clicked card

      mov     eax, [columnclicked]
      mov     [ncolumn], eax
      call    get_row_of_top_card_in_column
      mov     eax, [topcardrow]
      inc     eax

      mov     bl, 8
      mul     bl

      and     eax, $0000FFFF
      add     eax, [columnclicked]
      add     eax, cards

      mov     bl, byte [selcardcode]
      mov     byte [eax], bl

      mov     eax, [selcardaddr]
      mov     byte [eax], 52

      mov     [whereisselcard], scNotSelected

      call    draw_window

      jmp     .end

      .put_in_blank_cell:

      mov     eax, cards
      add     eax, [columnclicked]
      mov     bl,  byte [selcardcode]
      mov     byte [eax], bl

      mov     eax, [selcardaddr]
      mov     byte [eax], 52

      mov     [whereisselcard], scNotSelected

      call    draw_window

    .end:

  ret


;******************************************************************************
;                            temp_cell_click(columnclicked)
  temp_cell_click:
    call   get_sel_card_code_and_addr
    cmp    [selcardcode], 52
    jb	   .something_selected


    mov    [whereisselcard], scTempCells
    mov    eax, [columnclicked]
    mov    [columnofselcard], eax
    call   draw_window
    jmp    .end

    .something_selected:
			     ; checking if selected and clicked cards equivalent
    mov     eax, [columnclicked]
    add     eax, tempcells

    mov     ebx, 0
    mov     bl, byte [eax]
    mov     [cardclicked], ebx

    mov     eax, [selcardcode]
    cmp     [cardclicked], eax
    jne     .not_same_card

    mov     [whereisselcard], scNotSelected
    call    draw_window

    .not_same_card:

			     ;putting cards in temp cells

    mov     eax, [columnclicked]
    add     eax, tempcells

    mov     ebx, 0
    mov     bl, byte [eax]
    mov     [cardclicked], ebx


    cmp     [cardclicked], 52
    jb	   .end
			     ; if nothing lay in this cell
			     ; move selected card to temp cell
    mov     eax, [columnclicked]
    add     eax, tempcells
    mov     bl, byte [selcardcode]
    mov     byte [eax], bl

    mov     eax, [selcardaddr]
    mov     byte [eax], 52

    mov     [whereisselcard], scNotSelected

    call    draw_window


    jmp     .end


    .end:

  ret

;******************************************************************************
;                            home_cell_click(column_clicked)
  home_cell_click:
    call    get_sel_card_code_and_addr

    mov     eax, [columnclicked]
    add     eax, homecells


    mov     ebx, 0
    mov     bl, byte [eax]
    mov     [cardclicked], ebx

    mov     eax, [selcardcode]
    mov     bl, 4
    div     bl		     ; reminder in ah, quotient in al

    mov     ebx, 0
    mov     bl, ah
    mov     [cardfamily], ebx

    mov     ecx, 0
    mov     cl, al
    mov     [cardrange], ecx


    cmp     [cardclicked], 52
    jb	   .not_blank
			     ; if nothing lay in this cell
    cmp     [cardrange], 0
    jne     .end
			     ; move ace to home
    mov     eax, [columnclicked]
    add     eax, homecells
    mov     bl, byte [selcardcode]
    mov     byte [eax], bl

    mov     eax, [selcardaddr]
    mov     byte [eax], 52

    mov     [whereisselcard], scNotSelected

    call    draw_window


    jmp     .end

    .not_blank:

    mov     eax, [cardclicked]
    mov     bl, 4
    div     bl		     ; reminder in ah, quotient in al

    mov     ebx, 0
    mov     bl, ah
    mov     [clickedcardfamily], ebx

    mov     ecx, 0
    mov     cl, al
    mov     [clickedcardrange], ecx

    cmp     [cardfamily], ebx
    jne     .end

    inc     ecx
    cmp     [cardrange], ecx
    jne     .end

		      ; moving card from its place to home with replacing
		      ; of old card in home
    mov     eax, [columnclicked]
    add     eax, homecells
    mov     bl, byte [selcardcode]
    mov     byte [eax], bl

    mov     eax, [selcardaddr]
    mov     byte [eax], 52

    mov     [whereisselcard], scNotSelected

    call    draw_window



    .end:

  ret


;******************************************************************************
  new_game_click:

      mov   [i], 0
    .deleting_cards_from_common_cells:
      mov   eax, cards
      add   eax, [i]
      mov   byte [eax], 52


      inc   [i]
      cmp   [i], 19*8
      jb    .deleting_cards_from_common_cells


    mov     [i], 0
    .filling_pack:
      mov   eax, pack
      add   eax, [i]
      mov   bl, byte [i]
      mov   byte [eax], bl

      inc   [i]
      cmp   [i], 52
      jb    .filling_pack

      mov     [i], 0

    .putting_cards:

      mov   [range], 52
      call  random
      mov   eax, [random_value]
      add   eax, pack

      mov   ebx, 0
      mov   bl, byte [eax]
      mov   [randomcard], ebx

      mov   eax, [random_value]
      mov   [j], eax

      cmp   [randomcard], 52
      jb    .found_card


      mov   [range], 52
      call  random
      cmp   [random_value], 26
      jae    .decreasing_j

    .increasing_j:
      inc   [j]
			     ; j mod 52
      mov   eax, [j]
      mov   edx, 0
      mov   ebx, 52
      div   ebx
      mov   [j], edx


      mov   eax, [j]
      add   eax, pack
      mov   ebx, 0
      mov   bl, byte [eax]
      mov   [randomcard], ebx
      cmp   [randomcard], 52
      jb    .found_card

      jmp  .increasing_j


    .decreasing_j:
      dec   [j]
			     ; i mod 32
      mov   eax, [j]
      mov   edx, 0
      mov   ebx, 52
      div   ebx
      mov   [j], edx

      mov   eax, [j]
      add   eax, pack
      mov   ebx, 0
      mov   bl, byte [eax]
      mov   [randomcard], ebx
      cmp   [randomcard], 52
      jb    .found_card

      jmp  .decreasing_j

    .found_card:
			     ; putting card from pack
      mov   eax, cards
      add   eax, [i]
      mov   bl, byte [randomcard]
      mov   byte [eax], bl
			     ; deleting card from pack
      mov   eax, pack
      add   eax, [j]
      mov   byte [eax], 52


      inc   [i]
      cmp   [i], 52
      jb    .putting_cards




      mov   [i], 0
    .deleting_cards_from_temp_cells:
      mov   eax, tempcells
      add   eax, [i]
      mov   byte [eax], 52


      inc   [i]
      cmp   [i], 4
      jb    .deleting_cards_from_temp_cells

      mov   [i], 0
    .deleting_cards_from_home_cells:
      mov   eax, homecells
      add   eax, [i]
      mov   byte [eax], 52


      inc   [i]
      cmp   [i], 4
      jb    .deleting_cards_from_home_cells


    mov     [whereisselcard], scNotSelected
    call    draw_window


  ret


;******************************************************************************
;                       get_sel_card_code_and_addr(): selcardcode, selcardaddr
;                       if nothing selected, then selcardcode is 52
  get_sel_card_code_and_addr:
    cmp     [whereisselcard], scNotSelected
    jne     .something_selected

    mov     [selcardcode], 52
    jmp     .end

    .something_selected:
    cmp     [whereisselcard], scTempCells
    je	    .temp_cells_selected

			     ; common cells selected
    mov     eax, [columnofselcard]
    mov     [ncolumn], eax
    call    get_row_of_top_card_in_column


    mov     eax, [topcardrow]; eax = topcardrow * 8  + columnofselcard
    mov     bl, 8
    mul     bl			     ; result of multiplication in ax
    add     eax, [columnofselcard]
    add     eax, cards


    mov     [selcardaddr], eax
    xor     ebx, ebx
    mov     bl, byte [eax]
    mov     [selcardcode], ebx

    jmp     .end

    .temp_cells_selected:

    mov     eax, tempcells
    add     eax, [columnofselcard]
    mov     [selcardaddr], eax
    mov     ebx, 0
    mov     bl, byte [eax]
    mov     [selcardcode], ebx

    .end:

  ret

;******************************************************************************
;                            draw_window()

  draw_window:
    mov  eax,48  ; get system colors
    mov  ebx,3
    mov  ecx,syscolors
    mov  edx,sizeof.system_colors
    int  0x40


    mov     eax, 12	     ; start drawing
    mov     ebx, 1
    int     0x40

    mov     eax, 0	     ; create and draw the window
    mov     ebx, 100 * 65536 + 8 * cardwidth + 10 + 7 * columnspace
    mov     ecx, 100 * 65536 + 500
    mov     edx, 0x13008000
    mov     edi, header
    int     0x40

    mov     eax, 9	     ; getting window info
    mov     ebx, process_info
    mov     ecx, -1	     ; we want to know info of our window
    int     0x40


    mov     eax, [process_info.y_size]
    mov     [WindowHeight], ax

    mov     eax, [process_info.x_size]
    mov     [WindowWidth], ax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; draw top panel

    mov     eax, 13
    mov     ebx, 5
    shl     ebx, 16
    add     bx, word [process_info.x_size]
    sub     bx, 9
    mov     ecx, 22 shl 16 + topbuttonsbarheight - 1
    mov     edx, [syscolors.work_graph]
    int     $40

			     ; draw button "new game"

    mov     eax, 8
    mov     ebx, 5 shl 16 + 80
    mov     ecx, 22 shl 16 + topbuttonsbarheight - 2
    mov     edx, 1 + 8 + 4 + 4 + 1 ;button id
    mov     esi, [syscolors.work_button]
    int     $40

    mov     eax, 4
    mov     ebx, 20 shl 16 + 22 + topbuttonsbarheight/2 - 4
    mov     ecx, [syscolors.work_button_text]
    mov     edx, new_game
    mov     esi, new_game_len
    int     $40


       ; draw button "exit"
    mov     eax, 8
    mov     ebx, (5 + 85) shl 16 + 80 + 5
    mov     ecx, 22 shl 16 + topbuttonsbarheight - 2
    mov     edx, 1 + 8 + 4 + 4 + 2 ;button id
    mov     esi, [syscolors.work_button]
    int     $40

    mov     eax, 4
    mov     ebx, (40 + 80) shl 16 + 22 + topbuttonsbarheight/2 - 4
    mov     ecx, [syscolors.work_button_text]
    mov     edx, exit
    mov     esi, exit_len
    int     $40
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                        draw separators between home, temp and common cells
    mov     eax, 13
                   ; horizontal line
    mov     ebx, 5
    shl     ebx, 16
    add     bx,  word [process_info.x_size]
    sub     bx,  9
    mov     ecx, (21 + topbuttonsbarheight + cardheight + columnspace) shl 16+1

    mov     edx, [syscolors.work_graph]
    int     $40
                  ; verical line
    mov     eax, [process_info.x_size]
    mov     edx, 0
    mov     ecx, 2
    div     ecx

    mov     ebx, eax

    ;
    shl     ebx, 16
    add     bx,  1
    mov     ecx, (21 + topbuttonsbarheight) shl 16 + cardheight + columnspace
    mov     edx, [syscolors.work_graph]
    mov     eax, 13
    int     $40

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                            draw temp buttons

    mov     [j], 0	     ;counter that loops from 0 to 51

    draw_a_temp_card:

			     ; code of card must be in ecx
    mov     eax, tempcells
    add     eax, [j]
    xor     ecx, ecx
    mov     cl, byte [eax]   ; placing in cl value from memory
			     ;  with address [tempcells + j] or
			     ;  j-th element of array "tempcells"

    mov     [cardcode], ecx

    mov     eax, [j]
    xor     edx, edx
    mov     ebx, 8
    div     ebx 	     ; divsion by 8 (8 columns),
			     ;   so in eax quotient - number of row
			     ;   and in edx remainder -
			     ;   number of column where lay card

    mov     [row], eax
    mov     [column], edx

    mov     eax, [process_info.x_size]	; width of window
    sub     eax, 10
    sub     eax, cardwidth
    mov     ebx, 7
    mov     edx, 0
    div     ebx
    mov     ebx, [column]
    mul     ebx
    add     eax, 5

    mov     [xpos], eax


    mov     eax, [row]
    mov     bl, rowsize
    mul     bl
    add     eax, 24 + topbuttonsbarheight
    mov     [ypos], eax

			     ; checking, if this card selected

    mov     [negativedraw], 0

    cmp     [whereisselcard], scTempCells
    jne     .this_temp_cell_isnt_selected

    mov     eax, [column]
    cmp     [columnofselcard], eax
    jne     .this_temp_cell_isnt_selected

    mov     [negativedraw], 1

    .this_temp_cell_isnt_selected:

    call    draw_card

			     ; define button on place of card
    mov     eax, 8
    mov     ebx, [xpos]
    shl     ebx, 16
    add     bx, cardwidth - 1
    mov     ecx, [ypos]
    shl     ecx, 16
    add     cx, cardheight - 1
    mov     edx, [column]
    add     edx, 01000000000000000000000000000000b + 2 + 8;  button id = column
					   ; id = 1 reserved as close button
    int     $40


    inc     [j]
    cmp     [j], 4
    jb	    draw_a_temp_card


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                            draw home buttons
 mov	 [j], 0 	     ;counter that loops from 0 to 51

    draw_a_home_card:


			     ; code of card must be in ecx
    mov     eax, homecells
    add     eax, [j]
    xor     ecx, ecx
    mov     cl, byte [eax]   ; placing in cl value from memory
			     ;  with address [tempcells + j] or
			     ;  j-th element of array "tempcells"

    mov     [cardcode], ecx

    mov     eax, [j]
    xor     edx, edx
    mov     ebx, 8
    div     ebx 	     ; divsion by 8 (8 columns),
			     ;  so in eax quotient - number of row
			     ;  and in edx remainder -
			     ;  number of column where lay card

    mov     [row], eax
    mov     [column], edx

    mov     eax, [process_info.x_size]	; width of window
    sub     eax, 10
    sub     eax, cardwidth
    mov     ebx, 7
    mov     edx, 0
    div     ebx
    mov     ebx, [column]
    add     ebx, 4
    mul     ebx
    add     eax, 5

    mov     [xpos], eax

    mov     eax, [row]
    mov     bl, rowsize
    mul     bl
    add     eax, 24 + topbuttonsbarheight
    mov     [ypos], eax

    mov     [negativedraw], 0

    call    draw_card

			     ; define button on place of card

    mov     eax, 8
    mov     ebx, [xpos]
    shl     ebx, 16
    add     bx, cardwidth - 1
    mov     ecx, [ypos]
    shl     ecx, 16
    add     cx, cardheight - 1
    mov     edx, [column]
    add     edx, 01000000000000000000000000000000b + 2 + 8 + 4 ; button id

			     ; id = 1 reserved as close button
    int     $40


    inc     [j]
    cmp     [j], 4
    jb	     draw_a_home_card


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                            draw common cards

    mov     [j], 0	     ;counter that loops from 0 to 8 * 19

    draw_a_card:


			     ; code of card must be in ecx
    mov     eax, cards
    add     eax, [j]
    xor     ecx, ecx
    mov     cl, byte [eax]   ; placing in cl value from memory
			     ;  with address [cards + j] or
			     ;  j-th element of array "cards"
;    cmp     ecx, 52          ; if code of card >= 52 then there is no card
;    jae     no_draw
;
;    cmp     ecx, 0           ; if code of card  < 0 then there is no card
;    jb      no_draw

    mov     [cardcode], ecx



    mov     eax, [j]
    xor     edx, edx
    mov     ebx, 8
    div     ebx 	    ; divsion by 8 (8 columns),
			    ;  so in eax quotient - number of row
			    ;  and in edx remainder -
			    ;  number of column where lay card

    mov     [row], eax
    mov     [column], edx

    mov     eax, [process_info.x_size]	; width of window
    sub     eax, 10
    sub     eax, cardwidth
    mov     ebx, 7
    mov     edx, 0
    div     ebx
    mov     ebx, [column]
    mul     ebx
    add     eax, 5

    mov     [xpos], eax

    mov     eax, [row]
    mov     bl, rowsize
    mul     bl
    add     eax, cardheight + 24 + topbuttonsbarheight + columnspace
    mov     [ypos], eax


    mov     [negativedraw], 0 ;checking, if this is selected card

    cmp     [whereisselcard], scCommonCells
    jne     .this_card_isnt_selected

    mov     eax, [column]
    cmp     [columnofselcard], eax
    jne     .this_card_isnt_selected


    mov     eax, [column]
    mov     [ncolumn], eax
    call    get_row_of_top_card_in_column
    mov     eax, [row]
    cmp     [topcardrow], eax
    jne     .this_card_isnt_selected

    mov     [negativedraw], 1

    .this_card_isnt_selected:

    call    draw_card



			     ; now checking if it is top card in its column
			     ; if it does, we'll define button on its place
    mov     eax, [column]
    mov     [ncolumn], eax
    call    get_row_of_top_card_in_column
    mov     eax, [row]
    cmp     [topcardrow], eax
    je	     .define_button

    cmp     [topcardrow], 0
    jne     .no_define_button

    cmp     [row], 0
    jne     .no_define_button


    .define_button:
    mov     eax, 8
    mov     ebx, [xpos]
    shl     ebx, 16
    add     bx, cardwidth - 1
    mov     ecx, [ypos]
    shl     ecx, 16
    add     cx, cardheight - 1
    mov     edx, [column]
    add     edx, 01000000000000000000000000000000b + 2; button id = column + 2,
			     ; id = 1 reserved as close button
    int     $40


    .no_define_button:

    inc     [j]
    cmp     [j], 8 * 19
    jb	     draw_a_card



 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


    mov     eax, 12	     ; finish drawing
    mov     ebx, 2
    int     $40

  ret


;******************************************************************************
;            get_row_of_top_card_in_column(ncolumn): topcardrow

  get_row_of_top_card_in_column:
			     ; number of column in ncolumn
			     ; returns in topcardrow

    mov [i], 0		     ; i loops from 0 to 1, ... while card i * 8 + ncolumn
			     ; is valid card (0 <= its code < 52)

    .cycle:
       xor  eax, eax
       mov  al, 8
       mov  ebx, [i]
       mul  bl
       add  eax, [ncolumn]
       add  eax, cards
       xor  ecx, ecx
       mov  cl, byte [eax]

       cmp  ecx, 52
       jae  .endcycle


       cmp  [i], 18
       ja   .endcycle


       inc  [i]

       jmp  .cycle

    .endcycle:

      cmp   [i], 0
      je    .dont_dec

      dec   [i]

    .dont_dec:

      mov   eax, [i]
      mov   [topcardrow], eax
  ret


;******************************************************************************
;                      invert_image_colors(imagetoinvert, sizeofimagetoinvert)
  invert_image_colors:
    mov     [i], 0

    .inverting:
    mov     eax, [imagetoinvert]
    add     eax, [i]

    mov     bl, byte [eax]
    ;xor     ebx, ebx
    ;add     ebx, 10
    not     ebx

    mov     byte [eax], bl


    inc     [i]

    mov     ecx, [sizeofimagetoinvert]
    cmp     [i], ecx
    jb	    .inverting

    jmp   .later


    .exit:
      mov  eax, -1
      int  $40

    .later:


  ret



;******************************************************************************
;            draw_card(xpos, ypos, cardcode, negativedraw)
; if negativedraw = 1 then card drawn in inverted colors

  draw_card: ; draws card with left top corner
		    ; in point ([xpos],[ypos]),
		    ; type of card in [cardcode]

    cmp     [cardcode], 52   ; if code of card >= 52 then there is no card
    jae     .no_draw_card


    cmp     [negativedraw], 1
    jne     .no_invert1
			     ;doing if negativedraw
    mov     [bgcolor], $00000000
    mov     [blackcolor], $00FFFFFF
    mov     [redcolor], $0000FFFF

	     ;inverting all images
    call invert_all_images

    jmp     .colors_selection_done

    .no_invert1:
			     ;doing if not negativedraw
    mov     [bgcolor], $00FFFFFF
    mov     [blackcolor], $00000000
    mov     [redcolor], $00FF0000


    .colors_selection_done:

    mov     eax, 13

    mov     ebx, [xpos]      ; filling card with bgcolor
			     ; (big background rectangle)
    mov     edx, [bgcolor]
    add     ebx, 2
    shl     ebx, 16
    mov     bx, cardwidth - 4

    mov     ecx, [ypos]
    add     ecx, 2
    shl     ecx, 16
    mov     cx, cardheight - 4
    int     $40

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov     ebx, [xpos]      ; left black line
    shl     ebx, 16
    mov     bx, 1

    mov     ecx, [ypos]
    add     ecx, 5
    shl     ecx, 16
    xor     cx, cx
    mov     cx, cardheight - 2 * radius - 2
    mov     edx, [blackcolor]
    int     $40

    mov     ebx, [xpos]      ; left white line
    inc     ebx
    shl     ebx, 16
    mov     bx, 1
    mov     edx, [bgcolor]
    int     $40

    mov     ebx, [xpos]      ; right black line
    add     ebx, cardwidth - 1
    shl     ebx, 16
    mov     bx,  1
    mov     edx, [blackcolor]
    int     $40

    mov     ebx, [xpos]      ; right white line
    add     ebx, cardwidth - 2
    shl     ebx, 16
    mov     bx, 1
    mov     edx, [bgcolor]
    int     $40

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov     ecx, [ypos]      ; top black line
    shl     ecx, 16
    mov     cx, 1

    mov     ebx, [xpos]
    add     ebx, 5
    shl     ebx, 16
    mov     bx, cardwidth - 2 * radius - 2
    mov     edx, [blackcolor]
    int     $40

    mov     ecx, [ypos]      ; top white line
    inc     ecx
    shl     ecx, 16
    mov     cx, 1
    mov     edx, [bgcolor]
    int     $40

    mov     ecx, [ypos]      ; bottom black line
    add     ecx, cardheight - 1
    shl     ecx, 16
    mov     cx,  1
    mov     edx, [blackcolor]
    int     $40

    mov     ecx, [ypos]      ; bottom white line
    add     ecx, cardheight - 2
    shl     ecx, 16
    mov     cx, 1
    mov     edx, [bgcolor]
    int     $40

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov    eax, 1	     ; drawing points
    mov    edx, [blackcolor] ; black color for all pixels

    mov    ebx, [xpos]	     ; draw top left corner
    mov    ecx, [ypos]
    inc    ebx
    add    ecx, 4
    int    $40

    dec    ecx
    int    $40

    dec    ecx
    inc    ebx
    int    $40

    dec    ecx
    inc    ebx
    int    $40

    inc    ebx
    int    $40

    mov    ebx, [xpos]	     ;drawing top right corner
    mov    ecx, [ypos]
    add    ebx, cardwidth - 2
    add    ecx, 4
    int    $40

    dec    ecx
    int    $40

    dec    ebx
    dec    ecx
    int    $40

    dec    ebx
    dec    ecx
    int    $40

    dec    ebx
    int    $40
			     ;drawing bottom left corner
    mov    ebx, [xpos]
    mov    ecx, [ypos]
    inc    ebx
    add    ecx, cardheight - 5
    int    $40

    inc    ecx
    int    $40

    inc    ebx
    inc    ecx
    int    $40

    inc    ebx
    inc    ecx
    int    $40

    inc    ebx
    int    $40
			     ;drawing bottom right corner
    mov    ebx, [xpos]
    mov    ecx, [ypos]
    add    ebx, cardwidth - 2
    add    ecx, cardheight - 5
    int    $40

    inc    ecx
    int    $40

    dec    ebx
    inc    ecx
    int    $40

    dec    ebx
    inc    ecx
    int    $40

    dec    ebx
    int    $40


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   drawing text and images

    mov    eax, [cardcode]
    mov    edx, 0
    mov    ebx, 4
    div    ebx

    mov    [cardfamily], edx
    mov    [cardrange], eax

		     ; counting position of small card image
    mov eax, 7
    mov ecx, 8*65536+8
    mov edx, [xpos]
    add edx, radius
    shl edx, 16
    mov dx, word [ypos]
    add dx, radius + 8



    cmp    [cardfamily], 0
    je	   .heart

    cmp    [cardfamily], 1
    je	   .diamond

    cmp    [cardfamily], 2
    je	   .club

    cmp    [cardfamily], 3
    je	   .spade

    .heart:
       mov esi, [redcolor]
       mov [color], esi
       mov [imageaddr], heart
       mov [imageflipaddr], heart_updown

       mov ebx, heart_small
       int $40

       jmp .selnumber

    .diamond:
       mov esi, [redcolor]
       mov [color], esi
       mov [imageaddr], diamond
       mov [imageflipaddr], diamond_updown

       mov ebx, diamond_small
       int $40

       jmp .selnumber

    .club:
      mov  esi, [blackcolor]
      mov  [color], esi
      mov  [imageaddr], club
      mov  [imageflipaddr], club_updown

      mov ebx, club_small
      int $40

      jmp  .selnumber

    .spade:
      mov  esi, [blackcolor]
      mov  [color], esi
      mov  [imageaddr], spade
      mov  [imageflipaddr], spade_updown

      mov ebx, spade_small
      int $40



    .selnumber:

    mov    ebx, [xpos]	     ; counting position of text
			     ; in ebx, same for all cards
    add    ebx, radius
    shl    ebx, 16
    mov    bx,	word [ypos]
    add    bx,	radius


    mov    ecx, [color]


    cmp    [cardrange], 0
    je	   .ace

    cmp    [cardrange], 1
    je	   .two

    cmp    [cardrange], 2
    je	   .three

    cmp    [cardrange], 3
    je	   .four

    cmp    [cardrange], 4
    je	   .five

    cmp    [cardrange], 5
    je	   .six

    cmp    [cardrange], 6
    je	   .seven

    cmp    [cardrange], 7
    je	   .eight

    cmp    [cardrange], 8
    je	   .nine

    cmp    [cardrange], 9
    je	   .ten

    cmp    [cardrange], 10
    je	   .jack

    cmp    [cardrange], 11
    je	   .queen

    cmp    [cardrange], 12
    je	   .king

    ;      +-------+-------+-------+
    ;      |   3   |   2   |   3   |   ace   = 1
    ;      +-------+-------+-------+   two   = 2
    ;      |       |       |       |   three = 2 + 1
    ;      +-------+-------+-------+   four  = 3
    ;      |       |   6   |       |   five  = 3 + 1
    ;      +-------+-------+-------+   six   = 3 + 4
    ;      |   5   |       |   5   |   seven = 3 + 4 + 6
    ;      +-------+-------+-------+   eight = 3 + 5
    ;      |   4   |   1   |   4   |   nine  = 3 + 5
    ;      +-------+-------+-------+   ten   = 3 + 5 + 6 + 7
    ;      |   5   |       |   5   |
    ;      +-------+-------+-------+
    ;      |       |   7   |       |   1 means draw_1
    ;      +-------+-------+-------+
    ;      |       |       |       |
    ;      +-------+-------+-------+
    ;      |   3   |   2   |   3   |
    ;      +-------+-------+-------+



    .ace:
      mov  eax, 4
      mov  [s], byte 'A'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_1
      jmp .end

    .two:
      mov  eax, 4
      mov  [s], byte '2'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_2
      jmp .end


    .three:
      mov  eax, 4
      mov  [s], byte '3'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_1
      call draw_2

      jmp  .end

    .four:
      mov  eax, 4
      mov  [s], byte '4'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_3
      jmp  .end

    .five:
      mov  eax, 4
      mov  [s], byte '5'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_1
      call draw_3

      jmp  .end

    .six:
      mov  eax, 4
      mov  [s], byte '6'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_3
      call draw_4

      jmp  .end

    .seven:
      mov  eax, 4
      mov  [s], byte '7'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_3
      call draw_4
      call draw_6

      jmp  .end

    .eight:
      mov  eax, 4
      mov  [s], byte '8'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_3
      call draw_5

      jmp  .end

    .nine:
      mov  eax, 4
      mov  [s], byte '9'
      mov  edx, s
      mov  esi, 1
      int  $40

      call draw_3
      call draw_5
      call draw_1

      jmp  .end

    .ten:
      mov  eax, 4
      mov  [s], word '10'
      mov  edx, s
      mov  esi, 2
      int  $40

      call draw_3
      call draw_5
      call draw_6
      call draw_7

      jmp  .end

    .jack:
      mov  eax, 4
      mov  [s], byte 'J'
      mov  edx, s
      mov  esi, 1
      int  $40

      jmp  .end

    .queen:
      mov  eax, 4
      mov  [s], byte 'Q'
      mov  edx, s
      mov  esi, 1
      int  $40

      jmp  .end

    .king:
      mov  eax, 4
      mov  [s], byte 'K'
      mov  edx,s
      mov  esi, 1
      int  $40

    .end:


    cmp  [negativedraw], 1
    jne  .no_invert2

    call invert_all_images


    .no_invert2:
    .no_draw_card:

  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                            invert_all_images()
  invert_all_images:
    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], heart
    call invert_image_colors

    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], diamond
    call invert_image_colors

    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], spade
    call invert_image_colors

    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], club
    call invert_image_colors


    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], heart_updown
    call invert_image_colors

    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], diamond_updown
    call invert_image_colors

    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], spade_updown
    call invert_image_colors

    mov  [sizeofimagetoinvert], 16 * 16 * 3
    mov  [imagetoinvert], club_updown
    call invert_image_colors


    mov  [sizeofimagetoinvert], 8 * 8 * 3
    mov  [imagetoinvert], heart_small
    call invert_image_colors

    mov  [sizeofimagetoinvert], 8 * 8 * 3
    mov  [imagetoinvert], diamond_small
    call invert_image_colors

    mov  [sizeofimagetoinvert], 8 * 8 * 3
    mov  [imagetoinvert], spade_small
    call invert_image_colors

    mov  [sizeofimagetoinvert], 8 * 8 * 3
    mov  [imagetoinvert], club_small
    call invert_image_colors



  ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  draw_1:
			     ;draw center image
      mov     ebx, [imageaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, cardwidth/2 - 8
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight/2 - 8
      mov      eax, 7
      int     $40
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


  draw_2:
			     ;draw top image
      mov     ebx, [imageaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, 40 - 8
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, margin
      mov     eax, 7
      int     $40
			     ;draw bottom image
      mov     ebx, [imageflipaddr]
      mov     edx, [xpos]
      add     edx, cardwidth/2 - 8
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight - 16 - margin
      mov     eax, 7
      int     $40
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  draw_3:
			     ;draw top left image
      mov     ebx, [imageaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, margin
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, margin
      mov     eax, 7
      int     $40
			     ;draw bottom left image
      mov     ebx, [imageflipaddr]
      mov     edx, [xpos]
      add     edx, margin
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight - margin - 16
      mov     eax, 7
      int     $40
			     ;draw top right image
      mov     ebx, [imageaddr]
      mov     edx, [xpos]
      add     edx, cardwidth - margin - 16
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, margin
      mov     eax, 7
      int     $40
			     ;draw bottom right image
      mov     ebx, [imageflipaddr]
      mov     edx, [xpos]
      add     edx, cardwidth - margin - 16
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight - margin - 16
      mov     eax, 7
      int     $40
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  draw_4:
			     ;draw center left image
      mov     ebx, [imageaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, margin
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight/2 - 8
      mov     eax, 7
      int     $40
			     ;draw center right image
      mov     edx, [xpos]
      add     edx, cardwidth - margin - 16
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight/2 - 8
      mov     eax, 7
      int     $40
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  draw_5:
			     ;draw top left image
      mov     ebx, [imageaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, margin
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight * 3 / 9
      mov     eax, 7
      int     $40
			     ;draw bottom left image
      mov     ebx, [imageflipaddr]
      mov     edx, [xpos]
      add     edx, 16
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight * 5 / 9
      mov     eax, 7
      int     $40
			     ;draw top right image
      mov     ebx, [imageaddr]
      mov     edx, [xpos]
      add     edx, cardwidth - margin - 16
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight * 3 / 9
      mov     eax, 7
      int     $40
			     ;draw bottom right image
      mov     ebx, [imageflipaddr]
      mov     edx, [xpos]
      add     edx, cardwidth - margin - 16
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight * 5 / 9
      mov     eax, 7
      int     $40
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  draw_6:
      mov     ebx, [imageaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, cardwidth/2 - 8
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight * 2 / 9
      mov     eax, 7
      int     $40
  ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  draw_7:
      mov     ebx, [imageflipaddr]
      mov     ecx, 16 * 65536 + 16
      mov     edx, [xpos]
      add     edx, cardwidth/2 - 8
      shl     edx, 16
      mov     dx, word [ypos]
      add     dx, cardheight * 6 / 9
      mov     eax, 7
      int     $40
  ret


;******************************************************************************
  randomize:
    push eax

    mov  eax, 3
    int  0x40

    mov  ebx, $A59E3F1C
    mul  ebx
    mov  dword [randseed], eax
    pop  eax
  ret



;******************************************************************************
;          function Random(Range): RandomValue
  random:
    push ebx

    mov  eax, [randseed]
    mov  edx, 0
    mov  ebx, 7
    div  ebx

    cmp  edx, 0
    je	 _0

    cmp  edx, 1
    je	 _1

    cmp  edx, 2
    je	 _2

    cmp  edx, 3
    je	 _3

    cmp  edx, 4
    je	 _4

    cmp  edx, 5
    je	 _5

    cmp  edx, 6
    je	 _6

    jmp  _end


    _0:
      ;base := base + 58 + a[8];
      mov  eax, [randseed]
      add  eax, 58
      add  eax, dword [a + 8 * 4]
      mov  [randseed], eax
      jmp  _end;

    _1:
      ;base := base + 1 + a[9];
      mov  eax, [randseed]
      add  eax, 1
      add  eax, dword [a + 9 * 4]
      mov  [randseed], eax
      jmp _end;

    _2:
      ;base := base + 4 + a[88];
      mov  eax, [randseed]
      add  eax, 4
      add  eax, dword [a + 88 * 4]
      mov  [randseed], eax
      jmp _end;

    _3:
      ;randseed := randseed + 79 + a[43];
      mov  eax, [randseed]
      add  eax, 79
      add  eax, dword [a + 43 * 4]
      mov  [randseed], eax
      jmp _end;

    _4:
      ;randseed := randseed + 3 + a[12];
      mov  eax, [randseed]
      add  eax, 3
      add  eax, dword [a + 12 * 4]
      mov  [randseed], eax
      jmp _end;

    _5:
      ;randseed := randseed + 2 + a[63];
      mov  eax, [randseed]
      add  eax, 2
      add  eax, dword [a + 63 * 4]
      mov  [randseed], eax
      jmp _end;

    _6:
      ;randseed := randseed + 151 + a[24];
      mov  eax, [randseed]
      add  eax, 151
      add  eax, dword [a + 24 * 4]
      mov  [randseed], eax

      _end:

    mov  eax, [randseed]
    mov  edx, eax
    shl  edx, 16
    mov  bx, 100
    div  bx		      ; dx = randseed mod 100

    mov  ax, dx 	      ; ax = randseed mod 100
    mov  bx, 4
    mul  bx		      ; dx:ax = (randseed mod 100) * 4
    and  eax, $0000FFFF
    shr  edx, 16
    and  edx, $FFFF0000
    or	 eax, edx

    mov  eax, dword [a + eax] ; eax = dword[a + (randseed mod 100) * 4]
			    ; ~ a[randseed mod 100]
    mov  ebx, dword [a + 47 * 4]
    mul  ebx		      ; eax = low(a[randseed mod 100] * a[47])

    add  eax, [randseed]
    add  eax, $4AE783A
    mov  [randseed], eax

    mov  eax, dword [a + 6 * 4]
    mov  edx, 0
    mov  ebx,  100
    div  ebx
    mov  eax, edx
    mov  ebx, 4
    mul  ebx		      ; eax = (dword [a + 6 * 4] mod 100) * 4 ~ a[6] mod 100


    mov  eax, dword [a + eax] ; eax = dword [a + (dword [a + 6 * 4] mod 100) * 4

			    ; ~ a[a[6] mod 100]
    add  eax, [randseed]
    mov  [random_value], eax

    mov  edx, 0

    mov  ebx, [range]
    div  ebx
    mov  [random_value], edx

    mov  al, [TimesCalled]
    xor  ah, ah
    inc  al
    mov  bl, 100
    div  bl
    mov  [TimesCalled], ah   ; TimesCalled = (TimesCalled + 1 ) mod 100

    mov  al, ah
    mov  bl, 4
    mul  bl
    and  eax, $0000FFFF

    mov  ebx, [randseed]
    mov  dword [a + eax], ebx ; a[TimesCalled] = randseed

    pop  ebx
  ret

;******************************************************************************

; <--- initialised data --->
data
  header db 'Freecell',0

  new_game: db "New game"
  new_game_len = $ - new_game

  exit: db "Exit"
  exit_len = $ - exit

  s: db "10"


  negativedraw db 0	     ; for procedure draw_card


  spade 	 file 'SPADE.BMP': 54
  spade_updown	 file 'SPADEUD.BMP': 54
  spade_small	 file 'SPADESML.BMP': 54

  club		 file 'CLUB.BMP': 54
  club_updown	 file 'CLUBUD.BMP': 54
  club_small	 file 'CLUBSML.BMP': 54

  diamond	 file 'DIAM.BMP': 54
  diamond_updown file 'DIAMUD.BMP': 54
  diamond_small  file 'DIAMSML.BMP': 54

  heart 	 file 'HEART.BMP': 54
  heart_updown	 file 'HEARTUD.BMP': 54
  heart_small	 file 'HEARTSML.BMP': 54


  scNotSelected = 0
  scCommonCells = 1
  scTempCells = 2


  whereisselcard  dd scNotSelected
  columnofselcard dd 0	     ; if WhereIsSelCard = scGeneralCells
			     ;    then this can be 0 .. 7,
			     ; if scTempCells then - 0 .. 3
			     ; if scNotSelected - no matter

  tempcells: times 4 db 52;
  homecells: times 4 db 52 ; maximal card code is 51
  cards:     times 8 * 19 db 52; - %
  pack:      times 52 db ?



udata
  process_info process_information
  syscolors system_colors

  WindowHeight rw 1
  WindowWidth rw 1

  xpos rd 1
  ypos rd 1
  bgcolor rd 1
  blackcolor rd 1
  redcolor rd 1


  lastparam rd 1		  ;

  randomcard rd 1		 ; for new_game_click

  columnclicked rd 1		 ; used in common_card_click, temp_cell_click,
  cardclicked rd 1		 ;    home_cell_click
  clickedcardrange rd 1 	 ;
  clickedcardfamily rd 1	 ;


  selcardcode rd 1		 ; for procedure get_sel_card_code_and_addr
  selcardaddr rd 1		 ;

  column rd 1			 ; for procedure draw_window
  row rd 1			    ;

  imagetoinvert rd 1		 ; for procedure invert_image_colors
  sizeofimagetoinvert rd 1	 ;

  ncolumn rd 1			 ; for procedure get_row_of_top_card_in_column
  topcardrow rd 1		 ;


  color rd 1			 ; for procedue draw_card
  imageaddr rd 1		 ;
  imageflipaddr rd 1		 ;

  cardcode rd 1 		 ; used in differrent procedures
  cardrange rd 1		 ; cardcode = cardrange * 4 + cardfamily
  cardfamily rd 1		 ;

  a: times 100 rd 1		 ; for function Random
  range rd 1			 ;
  random_value rd 1		 ;
  randseed rd 1 		 ;
  TimesCalled rb 1		 ;

  j rd 1			 ; number of card (in array cards) drawn now
  i rd 1			 ; used in many procedures of 1-st level
  k rd 1

  cardwidth = 80
  cardheight = 120
  radius = 4			 ; not recommended to change
  rowsize = 30			 ; distance between top poins
				 ;of cards in neighboring rows
  columnspace = 5		 ; minimal space between cards
  margin = 14			    ; margin of every card

  topbuttonsbarheight = 20


meos_app_end