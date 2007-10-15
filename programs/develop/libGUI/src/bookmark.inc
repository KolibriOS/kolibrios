
zk_redraw_all_Bookmark       = 1000000b
zk_redraw_all_Bookmark_off   = 10111111b
zk_init_Bookmark             = 10000000b
zk_init_Bookmark_off         = 01111111b
zk_child_button_type         = 10001001b

;****************************************************
;******************craete Bookmark********************
;****************************************************
;IN
;pointer to parend
;pointer to Bookmark's structure
;OUT
;pointer to initialized Bookmark's structure
align 4

craete_Bookmark:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForBookmark],eax
      mov [ParendForBookmark],ebx

      mov ebx,eax

      ;calculate size of memory for control

      ;number of strings
      mov ecx,[ebx+30]
      add ebx,30+4
      xor edx,edx
      ;calculate total number of Bookmarks
      next_string_add:

        add edx,[ebx]
        add ebx,4

      dec ecx
      jnz next_string_add
      ;edx=chislo zakladok
      mov [ChisloZakladok],edx

      ;save addres
      mov edi,ebx

      xor esi,esi
      next_Bookmark_calculate:

        ;total number of controls on Bookmark
        mov ecx,[ebx+4]
        shl ecx,3
        add esi,ecx

        add ebx,8
        add ebx,ecx

      dec edx
      jnz next_Bookmark_calculate

      mov ecx,ebx
      sub ebx,eax
      mov eax,ebx
      add eax,control_header_size+(44*4)

      ;eax= size of memory for control
      push ebx
      push edi

      mov ebx,[ParendForBookmark]
      call craete_control

      pop edi
      pop ebx


      ;set all button's parameters in control
      mov [eax],dword Bookmark_

      push edi

      mov ecx,ebx
      mov esi,[PointerToStructureForBookmark]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      pop edi

      mov [eax+control_header_size+22],ebx

      ;craete child controls for parend(Bookmark)

      call get_skin_height

      ;set coordinats and size of control
      mov ebx,[PointerToStructureForBookmark]
      mov ecx,[ebx+2]   ;x
      mov edx,[ebx+6]   ;y
      mov esi,[ebx+10]  ;size x
      mov edi,[ebx+14]  ;size y
      add ecx,border_width
      add edx,[skin_height]
      ;mov [ebx+2],ecx
      ;mov [ebx+6],edx
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],edi

      ;load addres
      mov ebx,eax
      add ebx,control_header_size
      mov edx,[ebx+30]
      mov ecx,edx
      mov esi,[ChisloZakladok]
      mov [Bookmark.counter1],esi
      ;sizex*17
      shl ecx,4
      add ecx,edx
      mov [Bookmark.sizey],ecx

      mov ecx,edx
      shl ecx,2

      add ebx,30+4
      add ebx,ecx       ;pointer to information for first Bookmark

      next_Bookmark_craete_childs:

        ;total number of controls on Bookmark
         mov ecx,[ebx+4]
         mov [Bookmark.counter2],ecx
         add ebx,8

         test ecx,ecx
         jz no_controls_for_current_Bookmark

         next_child_for_Bookmark:

           ;is child button ?
           cmp [ebx],dword 1
           jnz no_child_button_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_button

              mov esi,eax

             pop ebx
             pop eax

              ;save addres of new control
              mov [ebx+4],esi

              mov edi,[eax+24]
              sub edi,border_width
              add [esi+24],edi
              mov edi,[eax+28]
              sub edi,[skin_height]
              add edi,[Bookmark.sizey]
              add [esi+28],edi
            jmp exit_child_for_Bookmark
           no_child_button_for_Bookmark:

           ;is child scroler ?
           cmp [ebx],dword 2
           jnz no_child_scroler_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_scroller

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark
           no_child_scroler_for_Bookmark:


           ;is child image ?
           cmp [ebx],dword 4
           jnz no_child_image_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_image

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark

           no_child_image_for_Bookmark:

           ;is child text ?
           cmp [ebx],dword 5
           jnz no_child_text_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_text

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark

           no_child_text_for_Bookmark:

           ;is child number ?
           cmp [ebx],dword 6
           jnz no_child_number_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_number

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark

           no_child_number_for_Bookmark:

           ;is child CheckBox ?
           cmp [ebx],dword 7
           jnz no_child_check_box_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_check_box

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark

           no_child_check_box_for_Bookmark:

           ;is child EditBox ?
           cmp [ebx],dword 8
           jnz no_child_edit_box_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_edit_box

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark

           no_child_edit_box_for_Bookmark:

           ;is child ProgresBar ?
           cmp [ebx],dword 9
           jnz no_child_progres_bar_for_Bookmark

             mov esi,eax      ;parend
             mov edi,[ebx+4]  ;information for child

             push eax
             push ebx

              push edi
              push esi

              call craete_progress_bar

              mov esi,eax

             pop ebx
             pop eax

             mov [ebx+4],esi

             mov edi,[eax+24]
             sub edi,border_width
             add [esi+24],edi
             mov edi,[eax+28]
             sub edi,[skin_height]
             add edi,[Bookmark.sizey]
             add [esi+28],edi
            jmp exit_child_for_Bookmark

           no_child_progres_bar_for_Bookmark:

          exit_child_for_Bookmark:

          add ebx,8

         dec [Bookmark.counter2]
         jnz next_child_for_Bookmark

        no_controls_for_current_Bookmark:

      dec [Bookmark.counter1]
      jnz next_Bookmark_craete_childs

      ret 8

;****************************************************
;******************Draw Bookmark*********************
;****************************************************
;IN
;pointer to control of Bookmark
;message
;OUT
;not returned value
align 4

Bookmark_:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of Bookmark
      mov esi,[esp+4]
      mov [PointerForBookmark],esi

      push esi
       mov ebx,[esi+control_header_size+22]  ;size of data's structure
       mov edi,dword Bookmark
       mov ecx,(43*4)+2+4
       add esi,control_header_size
       add esi,ebx
       rep movsb
      pop esi

      cmp [eax],dword 1
      jne no_redraw_all_Bookmark

       or [esi+control_header_size],byte zk_redraw_all_Bookmark
      no_redraw_all_Bookmark:

      cmp [eax],dword 6
      jne no_mouse_Bookmark

        mov esi,[eax+4]
        mov edi,[eax+8]
        mov ecx,[eax+12]
        mov [Bookmark.MouseX],esi
        mov [Bookmark.MouseY],edi
        mov [ButtonsOfMouse],ecx
      no_mouse_Bookmark:

      ;save message for child buttons
      mov esi,eax
      mov edi,dword Bookmark.MessageForChildButton
      mov ecx,4
      rep movsd

      ;no_mouse_Bookmark:


      ;load coordinats and size from control
      mov eax,[PointerForBookmark]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size x
      mov esi,[eax+36]           ;size y
      mov edi,[eax+control_header_size+18]        ;color1

      ;set current coordinats and sizes in Bookmark
      mov [Bookmark.x],ebx
      mov [Bookmark.y],ecx
      mov [Bookmark.sizex],edx
      mov [Bookmark.sizey],esi
      mov [Bookmark.color1],edi

      mov ebx,[PointerForBookmark]
      add ebx,control_header_size
      xor eax,eax
      mov al,[ebx]
      and al,zk_init_Bookmark
      test al,al
      jz  no_redraw_Bookmark

      and [ebx],byte zk_init_Bookmark_off

      ;generate colors tables

      mov eax,[Bookmark.color1]
      mov ebx,0xffffff
      mov ecx,20
      mov edx,dword colors_table1
      call gradient

      xor eax,eax
      mov ebx,[Bookmark.color1]
      mov ecx,20
      mov edx,dword colors_table2
      call gradient

      ;get memory for child buttons of Bookmark
      mov eax,50*4
      call malloc
      mov [Bookmark.ChildButtonsForBookmark],eax

      mov ebx,[PointerForBookmark]
      add ebx,control_header_size
      mov ecx,[ebx+30]
      mov edx,ecx
      add ebx,30+4
      shl edx,2
      mov esi,ebx
      add esi,edx ;esi= pointer to information for first child
      mov [Bookmark.PointerToTextForBookmark],esi

      mov edi,[Bookmark.y]
      mov [Bookmark.BookmarkY],edi

      mov edi,[Bookmark.sizex]
      sub edi,3
      mov [Bookmark.FullBookmarkSizeX],edi
      mov [Bookmark.ChisloStrokeZakladok],ecx

      and [Bookmark.CounterChildButtons],0

      ;craete child buttons for Bookmarks

      next_string_of_Bookmarks:

        mov edi,[Bookmark.x]
        add edi,2
        mov [Bookmark.BookmarkX],edi
        mov [Bookmark.WorkPlace_windowx],edi

        ;total number of Bookmarks in string

        mov edx,[ebx]
        mov edi,edx
        mov [Bookmark.NumberBookmarksInEndString],edx

        push eax
        mov eax,[Bookmark.ChildButtonsForBookmark]
        add eax,[Bookmark.CounterChildButtons]
        mov [Bookmark.PointerToEndBookmarks],eax
        pop eax

        ;calculate size of Bookmark
        ;size=(full_size/number_zakladok)
        push eax
        push edx

        mov eax,[Bookmark.FullBookmarkSizeX]
        cdq
        idiv edi

        mov [Bookmark.BookmarkSizeX],eax
        mov [Bookmark.WorkPlace_windowsizex],eax

        imul eax,edi
        mov edx,[Bookmark.FullBookmarkSizeX]
        sub edx,eax
        mov [Bookmark.DefectSizeX],edx

        pop edx
        pop eax

        next_child_button_craete_for_string:

          cmp edx,1
          jne no_disactivate_defect_size_x

          mov esi,[Bookmark.DefectSizeX]
          add [Bookmark.BookmarkSizeX],esi
          no_disactivate_defect_size_x:


          ;craete control button for Bookmark
          ;load info to structure
          mov esi,[PointerForBookmark]
          add esi,control_header_size+26
          mov edi,[esi]
          mov [ChildButton.type],zk_child_button_type
          mov [ChildButton.textcolor],edi

          mov esi,[Bookmark.BookmarkX]
          mov edi,[Bookmark.BookmarkY]
          sub esi,border_width
          sub edi,[skin_height]
          mov [ChildButton.x],si
          mov [ChildButton.y],di
          mov esi,[Bookmark.BookmarkSizeX]
          mov [ChildButton.width],si
          mov [ChildButton.height],word 16

          mov esi,[Bookmark.PointerToTextForBookmark]
          mov edi,[esi]
          mov [ChildButton.text],edi

          ;save child controls for this child button
          mov edi,[Bookmark.ChildButtonsForBookmark]
          add edi,[Bookmark.CounterChildButtons]
          add edi,4
          mov [edi],esi

          ;calculate new pointer for text of zakldka
          ;get total number of controls for current Bookmark
          mov edi,[esi+4]
          shl edi,3
          add esi,edi
          add esi,8
          mov [Bookmark.PointerToTextForBookmark],esi

          pushad

           push ChildButton
           push [PointerForBookmark]
           call craete_button

           ;save control of child button
           mov esi,eax
           mov edi,[Bookmark.ChildButtonsForBookmark]
           add edi,[Bookmark.CounterChildButtons]
           mov [edi],esi

           call DrawBookmark

           ;load obrabotchik for child button
           mov edi,[Bookmark.ChildButtonsForBookmark]
           add edi,[Bookmark.CounterChildButtons]
           mov esi,[edi]

           mov [Bookmark.MessageForChildButton],dword 1
           push Bookmark.MessageForChildButton
           push esi
           call button

           add [Bookmark.CounterChildButtons],8
          popad

          mov esi,[Bookmark.BookmarkSizeX]
          add [Bookmark.BookmarkX],esi

        dec edx
        jnz  next_child_button_craete_for_string

        add ebx,4
        add [Bookmark.BookmarkY],17  ;Bookmark size y

      dec ecx
      jnz next_string_of_Bookmarks

      ;calculate chislo zakladok
      mov eax,[Bookmark.CounterChildButtons]
      shr eax,3
      mov [Bookmark.ChisloZakladok],eax

      ;draw work place for Bookmark
      mov ebx,[Bookmark.ChisloStrokeZakladok]  ;chislo strok
      mov ecx,[Bookmark.x]   ;x for Bookmark work place
      shl ebx,4
      mov edx,ebx
      add ebx,[Bookmark.y]   ;y for Bookmark work place
      add ebx,[Bookmark.ChisloStrokeZakladok]
      mov esi,[Bookmark.sizey]  ;size y
      mov edi,[Bookmark.sizex]  ;size x
      sub esi,edx       ;size x Bookmark work place
      sub esi,1

      mov [Bookmark.WorkPlace_x],ecx
      mov [Bookmark.WorkPlace_y],ebx
      mov [Bookmark.WorkPlace_sizex],edi
      mov [Bookmark.WorkPlace_sizey],esi

      call DrawBookmarkWorkPlace

      mov edi,1
      mov [Bookmark.NumberActiveControl],edi

      call SendMessageChildControlsOfBookmark

      jmp exit_Bookmark
      no_redraw_Bookmark:



      ;check crosing arrea of child buttons
      mov eax,[Bookmark.x]
      mov ebx,[Bookmark.y]
      mov ecx,[Bookmark.sizex]
      mov edx,[Bookmark.ChisloStrokeZakladok]
      shl edx,4
      mov esi,[Bookmark.MouseX]
      mov edi,[Bookmark.MouseY]

      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crosing_child_buttons_Bookmark

        mov eax,[PointerForBookmark]
        mov ecx,[eax+control_header_size+30]           ;chislo zakladok in stroke
        add eax,control_header_size+30+4

        and [Bookmark.CounterChildButtons],0
        mov [Bookmark.BookmarkFlag],0

        next_stroke_Bookmark_check_crossing:

           mov edx,[eax]              ;number zakladok in string
           mov [Bookmark.NumberBookmarksInActiveString],edx

           mov esi,[Bookmark.ChildButtonsForBookmark]
           add esi,[Bookmark.CounterChildButtons]
           mov [Bookmark.PointerToActiveBookmarks],esi

           next_Bookmark_in_string_check:

             pushad

               mov eax,[Bookmark.ChildButtonsForBookmark]
               add eax,[Bookmark.CounterChildButtons]
               mov ebx,[eax]
               mov [Bookmark.ChildControlForBookmark],ebx

               push Bookmark.MessageForChildButton
               push ebx
               call button

               add [Bookmark.CounterChildButtons],8
             popad

               mov esi,[Bookmark.ChildControlForBookmark]
               xor ebx,ebx
               mov bl,byte[esi+control_header_size+1]
               and bl,1b
               cmp bl,1b
               jne no_crossing_Bookmark___

                cmp [ButtonsOfMouse],1
                jne no_crossing_Bookmark___

                mov [Bookmark.BookmarkFlag],byte 1
                jmp exit_check_crossing_with_Bookmark

               no_crossing_Bookmark___:

           dec edx
           jnz next_Bookmark_in_string_check

           add eax,4

        dec ecx
        jnz next_stroke_Bookmark_check_crossing

        exit_check_crossing_with_Bookmark:


        cmp [Bookmark.BookmarkFlag],1
        jne no_crosing_with_Bookmarks

          mov [Bookmark.BookmarkFlag],byte 0

          mov eax,[Bookmark.PointerToEndBookmarks]
          mov ebx,[eax]
          mov ecx,[ebx+28]      ;y for string of end Bookmarks

          mov edx,[Bookmark.PointerToActiveBookmarks]
          mov esi,[edx]
          mov edi,[esi+28]      ;y for string of active Bookmarks

          cmp ecx,edi
          je no_change_Y_coordinats_in_Bookmarks

             mov [Bookmark.y_end_stroke],ecx
             mov [y],edi        ;save y coordinate of active Bookmark

             mov eax,[Bookmark.PointerToActiveBookmarks]
             mov ecx,[Bookmark.NumberBookmarksInActiveString]
             ;y_active_string=y_end_string
             next_Bookmark_change_y_active:

                mov edx,[Bookmark.y_end_stroke]
                mov ebx,[eax]   ;ebx = pointer to Bookmark
                mov [ebx+28],edx

                add eax,8

             dec ecx
             jnz next_Bookmark_change_y_active

            mov eax,[Bookmark.PointerToEndBookmarks]
            mov ecx,[Bookmark.NumberBookmarksInEndString]
            ;y_end_string=y_active_string
            next_Bookmark_change_y_end:

                mov edx,[y]
                mov ebx,[eax]
                mov [ebx+28],edx

                add eax,8

            dec ecx
            jnz next_Bookmark_change_y_end

          mov eax,[Bookmark.PointerToActiveBookmarks]
          mov ebx,[Bookmark.PointerToEndBookmarks]
          mov ecx,[Bookmark.NumberBookmarksInActiveString]
          mov edx,[Bookmark.NumberBookmarksInEndString]

          mov esi,eax
          mov edi,ecx

          mov [Bookmark.PointerToActiveBookmarks],ebx
          mov [Bookmark.NumberBookmarksInActiveString],edx

          mov [Bookmark.PointerToEndBookmarks],esi
          mov [Bookmark.NumberBookmarksInEndString],edi

          no_change_Y_coordinats_in_Bookmarks:

          mov eax,[Bookmark.ChildButtonsForBookmark]
          mov ecx,[Bookmark.ChisloZakladok]
          mov [Bookmark.MessageForChildButton],dword 1
          next_Bookmark_redraw:

             mov ebx,[eax]
             mov edx,[ebx+24] ;x
             mov esi,[ebx+28] ;y
             mov edi,[ebx+32] ;size x
             mov [Bookmark.BookmarkX],edx
             mov [Bookmark.BookmarkY],esi
             mov [Bookmark.BookmarkSizeX],edi
             mov [Bookmark.ControlAddres],ebx

             pushad

                call DrawBookmark

                ;mov [Bookmark.MessageForChildButton],dword 1
                push Bookmark.MessageForChildButton
                push [Bookmark.ControlAddres]
                call button

             popad

             add eax,8
          dec ecx
          jnz next_Bookmark_redraw


          mov eax,[Bookmark.ChildControlForBookmark]
          mov ebx,[eax+24]    ;x coordinat of Bookmark
          mov ecx,[eax+32]    ;size x of Bookmark
          mov [Bookmark.WorkPlace_windowx],ebx
          mov [Bookmark.WorkPlace_windowsizex],ecx

          ;draw work place for Bookmark
          mov ebx,[Bookmark.ChisloStrokeZakladok]  ;chislo strok
          mov ecx,[Bookmark.x]   ;x for Bookmark work place
          shl ebx,4
          mov edx,ebx
          add ebx,[Bookmark.y]   ;y for Bookmark work place
          add ebx,[Bookmark.ChisloStrokeZakladok]
          mov esi,[Bookmark.sizey]  ;size y
          mov edi,[Bookmark.sizex]  ;size x
          sub esi,edx       ;size x Bookmark work place
          sub esi,1

          mov [Bookmark.WorkPlace_x],ecx
          mov [Bookmark.WorkPlace_y],ebx
          mov [Bookmark.WorkPlace_sizex],edi
          mov [Bookmark.WorkPlace_sizey],esi

          call DrawBookmarkWorkPlace

          mov edi,[Bookmark.CounterChildButtons]
          shr edi,3
          mov [Bookmark.NumberActiveControl],edi

          call SendMessageChildControlsOfBookmark

          ;delay mouse click
        mcall   5,7

        no_crosing_with_Bookmarks:

        jmp exit_Bookmark
      no_crosing_child_buttons_Bookmark:


       ;check for redraw all Bookmark
       mov ebx,[PointerForBookmark]
       add ebx,control_header_size
       xor eax,eax
       mov al,[ebx]
       and al,zk_redraw_all_Bookmark
       test al,al
       jz  no_redraw_all_Bookmark__

          and [ebx],byte zk_redraw_all_Bookmark_off

          mov ebx,[PointerForBookmark]
          add ebx,control_header_size
          mov ecx,[ebx+30]
          mov edx,ecx
          add ebx,30+4
          shl edx,2
          mov esi,ebx
          add esi,edx ;esi= pointer to information for first child
          mov [Bookmark.PointerToTextForBookmark],esi

          mov edi,[Bookmark.y]
          mov [Bookmark.BookmarkY],edi

          mov edi,[Bookmark.sizex]
          sub edi,3
          mov [Bookmark.FullBookmarkSizeX],edi
          mov [Bookmark.ChisloStrokeZakladok],ecx

          and [Bookmark.CounterChildButtons],0
          mov [Bookmark.MessageForChildButton],dword 1
          ;craete child buttons for Bookmarks

          next_string_of_Bookmarks_resize:

            mov edi,[Bookmark.x]
            add edi,2
            mov [Bookmark.BookmarkX],edi
            mov [Bookmark.WorkPlace_windowx],edi

            ;total number of Bookmarks in string

            mov edx,[ebx]
            mov edi,edx
            mov [Bookmark.NumberBookmarksInEndString],edx

            push eax
            mov eax,[Bookmark.ChildButtonsForBookmark]
            add eax,[Bookmark.CounterChildButtons]
            mov [Bookmark.PointerToEndBookmarks],eax
            pop eax

            ;calculate size of Bookmark
            ;size=(full_size/number_zakladok)
            push eax
            push edx

            mov eax,[Bookmark.FullBookmarkSizeX]
            cdq
            idiv edi

            mov [Bookmark.BookmarkSizeX],eax
            mov [Bookmark.WorkPlace_windowsizex],eax

            imul eax,edi
            mov edx,[Bookmark.FullBookmarkSizeX]
            sub edx,eax
            mov [Bookmark.DefectSizeX],edx

            pop edx
            pop eax

            next_child_button_resize:

               cmp edx,1
               jne no_disactivate_defect_size_x_resize

               mov esi,[Bookmark.DefectSizeX]
               add [Bookmark.BookmarkSizeX],esi
               no_disactivate_defect_size_x_resize:

                 pushad

                 mov edi,[Bookmark.ChildButtonsForBookmark]
                 add edi,[Bookmark.CounterChildButtons]
                 mov esi,[edi]

                 mov eax,[Bookmark.BookmarkX]
                 mov ebx,[Bookmark.BookmarkY]
                 mov ecx,[Bookmark.BookmarkSizeX]
                 mov [Bookmark.ControlAddres],esi
                 mov [esi+24],eax  ;x
                 mov [esi+28],ebx  ;y
                 mov [esi+32],ecx  ;size x

                 call DrawBookmark

                 push Bookmark.MessageForChildButton
                 push [Bookmark.ControlAddres]
                 call button

                 add [Bookmark.CounterChildButtons],8

                 popad

                 mov esi,[Bookmark.BookmarkSizeX]
                 add [Bookmark.BookmarkX],esi

              dec edx
              jnz  next_child_button_resize

            add ebx,4
            add [Bookmark.BookmarkY],17  ;Bookmark size y

          dec ecx
          jnz next_string_of_Bookmarks_resize

           ;calculate chislo zakladok
          mov eax,[Bookmark.CounterChildButtons]
          shr eax,3
          mov [Bookmark.ChisloZakladok],eax

          ;draw work place for Bookmark
          mov ebx,[Bookmark.ChisloStrokeZakladok]  ;chislo strok
          mov ecx,[Bookmark.x]   ;x for Bookmark work place
          shl ebx,4
          mov edx,ebx
          add ebx,[Bookmark.y]   ;y for Bookmark work place
          add ebx,[Bookmark.ChisloStrokeZakladok]
          mov esi,[Bookmark.sizey]  ;size y
          mov edi,[Bookmark.sizex]  ;size x
          sub esi,edx       ;size x Bookmark work place
          sub esi,1

          mov [Bookmark.WorkPlace_x],ecx
          mov [Bookmark.WorkPlace_y],ebx
          mov [Bookmark.WorkPlace_sizex],edi
          mov [Bookmark.WorkPlace_sizey],esi

          call DrawBookmarkWorkPlace

          mov edi,[Bookmark.NumberActiveControl]
          call SendMessageChildControlsOfBookmark

          jmp exit_Bookmark

       no_redraw_all_Bookmark__:


       ;send events to controls of active Bookmark
       mov edi,[Bookmark.NumberActiveControl]
       call SendMessageChildControlsOfBookmark

       exit_Bookmark:

       ;save work data in control
       mov esi,[PointerForBookmark]
       mov ebx,[esi+control_header_size+22]  ;size of data's structure
       mov esi,dword Bookmark
       mov ecx,(43*4)+2+4
       mov edi,[PointerForBookmark]
       add edi,control_header_size
       add edi,ebx
       rep movsb

      ret 8

;edi - number child Bookmark
;SendMessageForChildButton    - message

SendMessageChildControlsOfBookmark:

        ;edi - number child Bookmark

       mov eax,[PointerForBookmark]
       add eax,control_header_size+30
       mov ecx,[eax]   ;number Bookmarks
       shl ecx,2
       add eax,4
       add eax,ecx     ;pointer to first child Bookmark

       dec edi
       jz addres_find_

         next_child_control_find:

            mov ebx,[eax+4]   ;number controls
            shl ebx,3
            add eax,8
            add eax,ebx       ;pointer to next control
         dec edi
         jnz next_child_control_find

       addres_find_:

       mov ecx,[eax+4]   ;number child controls for active Bookmark
       mov [Bookmark.CounterChildControls],ecx
       add eax,4+4+4       ;eax=pointer to child controls for Bookmark
       mov [Bookmark.AddresOfActiveChildControl],eax

       send_message_to_next_child_control_of_active_Bookmark:

         mov eax,[Bookmark.AddresOfActiveChildControl]

         ;redraw control ?

         cmp [Bookmark.MessageForChildButton],1
         jne no_redraw_child_control_Bookmark


           mov ebx,[eax]
           mov ecx,[ebx]
           push Bookmark.MessageForChildButton
           push ebx
           call ecx

         jmp exit_send_message_to_child_control_Bookmark
         no_redraw_child_control_Bookmark:

         ;send message of keys ?

         cmp [Bookmark.MessageForChildButton],2
         jne no_keys_child_control_Bookmark

           ;mov esi,[Bookmark.ActiveChildControl]
           ;test esi,esi
           ;jz exit_send_message_to_child_control_Bookmark

             ;mov eax,[Bookmark.ActiveChildControl]
             mov ebx,[eax]
             mov ecx,[ebx]
             push Bookmark.MessageForChildButton
             push ebx
             call ecx

           jmp exit_send_message_to_child_control_Bookmark
         no_keys_child_control_Bookmark:

         ;send special message ?

         cmp [Bookmark.MessageForChildButton],3
         jne no_send_special_message_child_control_Bookmark


           mov ebx,[eax]
           mov ecx,[ebx]
           push Bookmark.MessageForChildButton
           push ebx
           call ecx

           jmp exit_send_message_to_child_control_Bookmark
         no_send_special_message_child_control_Bookmark:

         ;Check mouse state ?
         ;------------------------------------------------------
         cmp [Bookmark.MessageForChildButton],6
         jne no_mouse_child_control_Bookmark

           mov ebx,[eax]
           mov eax,ebx
	
           mov ebx,[Bookmark.MessageForChildButton+12]
           test ebx,ebx
           jz bookmark_left_button_of_mouse_not_pressed

              mov ebx,[Bookmark.ActiveChildControl]
              test ebx,ebx
              jz bookmark_havent_active_control

                 mov eax,[Bookmark.ActiveChildControl]
                 mov ebx,[eax]
                 push Bookmark.MessageForChildButton
                 push eax
                 call ebx

                 jmp exit_send_message_to_child_control_Bookmark
              bookmark_havent_active_control:

           jmp bookmark_exit_if_left_button_of_mouse_pressed
           bookmark_left_button_of_mouse_not_pressed:

               mov ebx,[Bookmark.ActiveChildControl]
               test ebx,ebx
               jz bookmark_havent_active_control_

                 mov [Bookmark.ActiveChildControl],dword 0

               bookmark_havent_active_control_:

           bookmark_exit_if_left_button_of_mouse_pressed:

          ;send message to all child controls of active Bookmark
           mov esi,[eax+24]      ;x
           mov edi,[eax+28]      ;y
           mov ecx,[eax+32]      ;size x
           mov edx,[eax+36]      ;size y
           mov [x],esi
           mov [y],edi
           mov eax,[x]
           mov ebx,[y]
           mov esi,[Bookmark.MouseX]
           mov edi,[Bookmark.MouseY]

           call CheckCrossingBox

           cmp eax,0xffffff
           jne no_crossing_123

              cmp [Bookmark.MessageForChildButton+12],1
              jne button_not_pressed_123

                 ;save addres of activated control
                 mov eax,[Bookmark.AddresOfActiveChildControl]
                 mov ebx,[eax]
                 mov [Bookmark.ActiveChildControl],ebx

              button_not_pressed_123:
            no_crossing_123:

           ;send message to control
           mov eax,[Bookmark.AddresOfActiveChildControl]
           mov ebx,[eax]
           mov ecx,[ebx]
           push Bookmark.MessageForChildButton
           push ebx
           call ecx

           no_mouse_child_control_Bookmark:
        ;------------------------------------------------------

       ;destroy child controls ?
       ;--------------------------------------------------------
       cmp [Bookmark.MessageForChildButton],dword -1
       jne no_delete_Bookmark_

          and [Bookmark.CounterChildButtons],0

         next_Bookmark_delete:

          mov eax,[Bookmark.ChildButtonsForBookmark]
          add eax,[Bookmark.CounterChildButtons]

          ;destroy child button
          mov ebx,[eax]   ;control child button

          ;pushad

          push ebx
          call destroy_control

          ;popad

          ;destroy child controls for CURRENT Bookmark
          mov edi,[Bookmark.CounterChildButtons]
          shr edi,3
          inc edi

          mov eax,[PointerForBookmark]
          add eax,control_header_size+30
          mov ecx,[eax]   ;number Bookmarks
          shl ecx,2
          add eax,4
          add eax,ecx     ;pointer to first child Bookmark

          dec edi
          jz addres_find___

            next_child_control_find__:

               mov ebx,[eax+4]   ;number controls
               shl ebx,3
               add eax,8
               add eax,ebx       ;pointer to next control
            dec edi
            jnz next_child_control_find__

          addres_find___:

          mov ecx,[eax+4]   ;number child controls for active Bookmark
          mov [Bookmark.CounterChildControls],ecx
          add eax,4+4+4       ;eax=pointer to child controls for Bookmark
          mov [Bookmark.AddresOfActiveChildControl],eax

             next_child_control_of_Bookmark_delete:

               mov eax,[Bookmark.AddresOfActiveChildControl]
               mov ebx,[eax]

               push ebx
               call destroy_control

               add [Bookmark.AddresOfActiveChildControl],8

             dec [Bookmark.CounterChildControls]
             jnz next_child_control_of_Bookmark_delete

          add [Bookmark.CounterChildButtons],8

         dec [Bookmark.ChisloZakladok]
         jnz next_Bookmark_delete

         ;free memory from array
         mov eax,[Bookmark.ChildButtonsForBookmark]
         call free

       no_delete_Bookmark_:
       ;----------------------------------------------------------------

       exit_send_message_to_child_control_Bookmark:

       add [Bookmark.AddresOfActiveChildControl],4+4

      dec [Bookmark.CounterChildControls]
      jnz  send_message_to_next_child_control_of_active_Bookmark

      ret

;BookmarkX
;BookmarkY
;BookmarkSizeX
;BookmarkSizeY
DrawBookmark:

      mov eax,[Bookmark.BookmarkX]
      mov ebx,[Bookmark.BookmarkY]
      mov [x],eax
      mov [y],ebx

      ;draw rectangle(x+1,y+1,sizex-2,sizey-2)
      mov edx,[colors_table1]
      mov esi,[Bookmark.BookmarkSizeX]
      sub esi,2

      mov eax,13
      mov ebx,[x]
      mov ecx,[y]
      add ebx,1
      add ecx,1
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,(16)
      and edx,0xffffff
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax
      mov [Pixel.color],eax
      ;draw white line(x+2,y,x+sizex-3,y)

      mov eax,[x]
      mov ebx,[y]
      mov ecx,eax
      add eax,2
      add ecx,[Bookmark.BookmarkSizeX]
      sub ecx,3
      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine

      ;draw white line(x,y+2,x,y+sizey)
      mov eax,[x]
      mov ebx,[y]
      mov ecx,ebx
      add ebx,2
      add ecx,(16)
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ecx
      call DrawLine

      mov eax,[x]
      mov ebx,[y]
      add eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[colors_table1+6*3]
      and eax,0xffffff
      mov [Line.color],eax
      ;draw light line(x+2,y+1,x+sizex-2,y+1)

      mov eax,[x]
      mov ebx,[y]
      mov ecx,eax
      add eax,2
      add ebx,1
      add ecx,[Bookmark.BookmarkSizeX]
      sub ecx,2
      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine

      ;draw light line(x+1,y+2,x+1,y+sizey-1)
      mov eax,[x]
      mov ebx,[y]
      mov ecx,ebx
      add eax,1
      add ebx,2
      add ecx,(16)
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ecx
      call DrawLine

      mov eax,[colors_table2+9*3]
      and eax,0xffffff
      mov [Line.color],eax
      mov [Pixel.color],eax

      ;draw dark line(x+sizex-1,y+2,x+sizex-1,y+sizey)
      mov eax,[x]
      mov ebx,[y]
      add eax,[Bookmark.BookmarkSizeX]
      mov edx,ebx
      add ebx,2
      add edx,(16)
      sub eax,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],edx
      call DrawLine

      mov eax,[x]
      mov ebx,[y]
      add eax,[Bookmark.BookmarkSizeX]
      add ebx,1
      sub eax,2
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[colors_table2+16*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw light dark line(x+sizex-2,y+2,x+sizex-2,y+sizey)
      mov eax,[x]
      mov ebx,[y]
      add eax,[Bookmark.BookmarkSizeX]
      sub eax,2
      mov edx,ebx
      add ebx,2
      add edx,(16)
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],edx
      call DrawLine


      ret

DrawBookmarkWorkPlace:

      mov esi,[Bookmark.WorkPlace_sizex]
      sub esi,1

      mov eax,13
      mov ebx,[Bookmark.WorkPlace_x]
      mov ecx,[Bookmark.WorkPlace_y]
      add ebx,1
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,[Bookmark.WorkPlace_sizey]
      mov edx,[colors_table1]
      and edx,0xffffff
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical light line(x,y,x,y+sizey)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,ebx
      add ecx,[Bookmark.WorkPlace_sizey]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ecx
      call DrawLine

      ;draw first horizontal light line
      ;(x,y,x+windowx,y)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,[Bookmark.WorkPlace_windowx]

      cmp eax,ecx
      je no_draw_line_00

      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine

      no_draw_line_00:

      ;draw second horizontal light line
      ;(x,y,x+windowx,y)
      mov eax,[Bookmark.WorkPlace_windowx]
      add eax,[Bookmark.WorkPlace_windowsizex]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,[Bookmark.WorkPlace_x]
      add ecx,[Bookmark.WorkPlace_sizex]
      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine

      ;draw next light lines

      mov eax,[colors_table1+4*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical light line(x+1,y+1,x+1,y+sizey-1)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,ebx
      add ecx,[Bookmark.WorkPlace_sizey]
      add eax,1
      add ebx,1
      sub ecx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ecx
      call DrawLine

      ;draw first horizontal light line
      ;(x+1,y+1,x+windowx,y+1)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,[Bookmark.WorkPlace_windowx]
      add ebx,1

      cmp eax,ecx
      je no_draw_line_01

      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      ;call DrawLine

      no_draw_line_01:

      ;draw second horizontal light line
      ;(x+1,y+1,x+windowx-2,y+1)
      mov eax,[Bookmark.WorkPlace_windowx]
      mov ecx,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      add eax,[Bookmark.WorkPlace_windowsizex]
      add ecx,[Bookmark.WorkPlace_sizex]
      add eax,1
      add ebx,1
      sub ecx,2
      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine


      mov eax,[colors_table2+9*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical dark line(x+sizex,y,x+sizex,y+sizey)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,ebx
      add eax,[Bookmark.WorkPlace_sizex]
      add ecx,[Bookmark.WorkPlace_sizey]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ecx
      call DrawLine

      ;draw horizontal dark line(x+1,y+sizey,x+sizex,y+sizey)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,eax
      add ecx,[Bookmark.WorkPlace_sizex]
      add ebx,[Bookmark.WorkPlace_sizey]
      add eax,1
      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine

      mov eax,[colors_table2+16*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical dark line(x+sizex-1,y+1,x+sizex-1,y+sizey-1)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,ebx
      add eax,[Bookmark.WorkPlace_sizex]
      add ecx,[Bookmark.WorkPlace_sizey]
      sub eax,1
      sub ecx,1
      ;add ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ecx
      call DrawLine

      ;draw horizontal dark line(x+1,y+sizey-1,x+sizex-1,y+sizey-1)
      mov eax,[Bookmark.WorkPlace_x]
      mov ebx,[Bookmark.WorkPlace_y]
      mov ecx,eax
      add ecx,[Bookmark.WorkPlace_sizex]
      add ebx,[Bookmark.WorkPlace_sizey]
      add eax,1
      sub ecx,1
      sub ebx,1
      mov [Line.x1],eax
      mov [Line.x2],ecx
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      call DrawLine

      ret
