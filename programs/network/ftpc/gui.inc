include '../../load_img.inc'
include '../../develop/libraries/libs-dev/libimg/libimg.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../develop/libraries/box_lib/load_lib.mac'

@use_library_mem mem.Alloc, mem.Free, mem.ReAlloc, dll.Load
 
icon_tl_sys dd 0
icon_tl_nod dd 0

TLIST_SZ = 325+16 ;=341. 16 is x-size of scroll bar
TLIST1_X = 50
TLIST2_X = 409
BT_SZ_X  = 40
SYS_COL  = 0xe6e6e6
BT_COL   = 0xcccccc
STR_COL  = 0x595959 ;0x000000

; TODO: automatic resizing of GUI elements on window resizing

align 4
;;================================================================================================;;
gui: ;////////////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? GUI-specific functions                                                                         ;;
;;------------------------------------------------------------------------------------------------;;
;> none                                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

    dd .init
    dd .server_addr
    dd .get_username
    dd .get_cmd
    dd .print
    dd .set_flags
    dd .list
    dd .progress
    dd .error

    .init:
        ; load libraries
        stdcall dll.Load, @IMPORT_GUI_LIBS
        test    eax, eax
        jnz     .exit

        mcall   48, 3, sc, sizeof.system_colors
        mcall   40, 0xC0000027 ; set event mask
        ret

    .server_addr:
        mov     [auto_list], 1
        jmp     login_gui.server_addr

    .get_username:
        jmp     login_gui.get_username


;;================================================================================================;;
    .main: ;//////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Initialize tree list, editbox and textedit elements                                            ;;
;;------------------------------------------------------------------------------------------------;;
;> none                                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

        stdcall [ted_init], tedit0
        stdcall [edit_box_set_text], edit_volume, root_dir

        ; initialize tree list elements
        stdcall [tl_data_init], tree1
        stdcall [tl_data_init], tree2
		
		or dword[tree1.style], 8 ;tl_cursor_pos_limited - restrict cursor move to existing nodes
		or dword[tree2.style], 8 ;tl_cursor_pos_limited
		
        ; read icons
        load_image_file 'tl_sys_16.png', icon_tl_sys

        mov     [filestruct.ptr], eax
        mov     [filestruct.subfn], 0
        mov     [filestruct.offset], 54
        mov     [filestruct.size], ecx
        mcall   70, filestruct
        cmp     ebx, 0
        jl      @f
        m2m     dword[tree1.data_img_sys], dword[icon_tl_sys]
        m2m     dword[tree2.data_img_sys], dword[icon_tl_sys]

      @@:
        ; read nodes icon file
        load_image_file 'tl_nod_16.png', icon_tl_nod

        mov     [filestruct.ptr], eax
        ;mov    [filestruct.subfn], 0
        ;mov    [filestruct.offset], 54
        mov     [filestruct.size], ecx
        mcall   70, filestruct
        cmp     ebx, 0
        jl      @f
        m2m     dword[tree1.data_img], dword[icon_tl_nod]
        m2m     dword[tree2.data_img], dword[icon_tl_nod]
    
      @@:
        mcall   67, 35, 20, 830, 555 ; resize to main gui window's coordinates
        call    .draw
        ; create initial tree list for root_dir
        call    populate_local_tree_list
        jmp     arg_handler.connect

    .get_cmd:
        dec     [auto_list]
        jnz     .redraw
        call    .clear_remote_tree
        jmp     cmd_list

    .redraw:
        call     .draw

    align 4
    .still:
        mcall   10

        dec     eax
        jz      .redraw
        dec     eax
        jz      .key
        dec     eax
        jz      .button
        sub     eax, 3
        jz      .mouse

        jmp     .still

    .error:
        jmp     login_gui.error

    .exit:
    ;   stdcall mem.Free,[bmp_icon]
        stdcall [tl_data_clear], tree1
        stdcall [tl_data_clear], tree2
        stdcall [ted_delete], tedit0
        jmp     exit

    align 4
    .draw:
        mcall   12, 1
        ; main window
        mcall   0, <35,830>, <20,555>, 0x34000000+SYS_COL, 0x805080DD, str_title

        ;-------------------------
        ; textedit components
        ;-------------------------
        mcall   9, procinfo, -1
        mov     edi, tedit0
        call    EvSize

        movzx   ebx,word[procinfo.client_box.width]
        inc     bx
        mcall   13, , ted_wnd_t
        stdcall [ted_draw], tedit0

        ; draw "disconnect" button
        mcall   8, <50,95>, <25,25>, 3, BT_COL
        ; string "disconnect"
        mcall   4, <50+5,25+5>, 0xb0000000+STR_COL, str_disconnect
        ; draw "Copy" button
        mcall   8, <50,40>, <326,25>, 4, BT_COL
        ; string "copy"
        mcall   4, <50+5,326+5>, 0xb0000000+STR_COL, str_copy
        ; draw "Search" button
        mcall   8, <TLIST2_X+TLIST_SZ-55,55>, <20,25>, 5, BT_COL
        ; string "Search"
        mcall   4, <TLIST2_X+TLIST_SZ-55+5,20+5>, 0xb0000000+STR_COL, str_search

        ; draw "LIST" button
        mcall   8, <TLIST2_X+TLIST_SZ-4*BT_SZ_X-15,BT_SZ_X>, <50,25>, 12, BT_COL
        ; string "LIST"
        mcall   4, <TLIST2_X+TLIST_SZ-4*BT_SZ_X-15+5,50+5>, 0xb0000000+STR_COL, str_list
        ; draw "DELE" button
        mcall   8, <TLIST2_X+TLIST_SZ-3*BT_SZ_X-10,BT_SZ_X>, <50,25>, 7, BT_COL
        ; string "DELE"
        mcall   4, <TLIST2_X+TLIST_SZ-3*BT_SZ_X-10+5,50+5>, 0xb0000000+STR_COL, str_dele
        ; draw "RDIR" button
        mcall   8, <TLIST2_X+TLIST_SZ-2*BT_SZ_X-5,BT_SZ_X>, <50,25>, 8, BT_COL
        ; string "RDIR"
        mcall   4, <TLIST2_X+TLIST_SZ-2*BT_SZ_X-5+5,50+5>, 0xb0000000+STR_COL, str_rdir
        ; draw "RMD" button
        mcall   8, <TLIST2_X+TLIST_SZ-BT_SZ_X,BT_SZ_X>, <50,25>, 9, BT_COL
        ; string "RMD"
        mcall   4, <TLIST2_X+TLIST_SZ-BT_SZ_X+5,50+5>, 0xb0000000+STR_COL, str_rmd

        ; draw "mkd" button
        mcall   8, <TLIST2_X+TLIST_SZ-BT_SZ_X,BT_SZ_X>, <80,20>, 6, BT_COL
        ; string "mkd"
        mcall   4, <TLIST2_X+TLIST_SZ-BT_SZ_X+5,80+5>, 0xb0000000+STR_COL, str_mkd
        
        ; draw "cancel" button
        mcall   8, <555,55>, <308,25>, 10, BT_COL
        ; string "Cancel"
        mcall   4, <555+5,308+5>, 0xb0000000+STR_COL, str_abort
        ; draw "change volume" button
        mcall   8, <TLIST1_X+TLIST_SZ-55,55>, <100-20,22>, 11, BT_COL
        ; string "Change"
        mcall   4, <TLIST1_X+TLIST_SZ-55+5,100-20+5>, 0xb0000000+STR_COL, str_change

        ;--------------------------
        ; tree list components
        ;--------------------------
        stdcall [tl_draw], tree1
        stdcall [tl_draw], tree2
     
        mov     dword[wScr.all_redraw], 1
        stdcall [scrollbar_ver_draw], wScr

        mov     dword[wScr2.all_redraw],1
        stdcall [scrollbar_ver_draw], wScr2

        stdcall [edit_box_draw], edit_mkd
        stdcall [edit_box_draw], edit_volume
        stdcall [edit_box_draw], edit_search

        ; progress bar
        stdcall [progressbar_draw], pb

        mcall   12,2
        ret

    align 16
    .mouse:
        mcall   37, 1
        cmp     word[tedit0.wnd.top], ax
        jg      .no_edit

        shr     eax, 16
        cmp     word[tedit0.wnd.left], ax
        jg      .no_edit

        mcall   37, 3
        bt      eax, 24
        jnc     @f

        stdcall [ted_but_select_word], tedit0
        jmp     .still

      @@:
        stdcall [ted_mouse], tedit0
        jmp     .still

      .no_edit:
        stdcall [tl_mouse], tree1
        stdcall [tl_mouse], tree2
        stdcall [edit_box_mouse], edit_mkd
        stdcall [edit_box_mouse], edit_volume
        stdcall [edit_box_mouse], edit_search
        jmp     .still

    align 16
    .key:
        mcall   2
        stdcall [tl_key], tree1
        stdcall [tl_key], tree2
        stdcall [edit_box_key], edit_mkd
        stdcall [edit_box_key], edit_volume
        stdcall [edit_box_key], edit_search
        jmp     .still

    align 16
    .button:
        mcall   17

        cmp     ah, 3
        jne     @f
        ijmp    eax, interface_addr, interface.server_addr
        jmp     .still

      @@:
        cmp     ah, 4
        jne     @f
        stdcall [ted_but_copy], tedit0
        jmp     .still

      @@:
        cmp     ah, 5
        jne     @f
        je      filter_remote_list

      @@:
        cmp     ah, 6
        je      create_remote_dir

        cmp     ah, 7
        je      delete_file

        cmp     ah, 8
        je      cmd_rdir

        cmp     ah, 9
        je      remove_dir

        cmp     ah, 11
        jne     @f
        call    populate_local_tree_list

      @@:
        cmp     ah, 12
        jne     @f
        call    .clear_remote_tree
        je      cmd_list

      @@:
        cmp     ah, 1
        je      .exit
      
        jmp     .still

    .print:
        pushad
        mov     ebx, ted_opt_ed_change_time + ted_opt_ed_move_cursor
        mov     ecx, 2 ; 0a0d
        mov     esi, [esp+36]
        mov     word[tedit_buffer], 0x0a0d
        lea     edi, [tedit_buffer+2]

      @@:
        movsb
        inc     ecx
        cmp     byte[esi], 0
        jne     @b
        stdcall [ted_text_add], tedit0, tedit_buffer, ecx, ebx
        stdcall [ted_draw], tedit0
        ; write to log file
        mov     eax, [esp+36]
        sub     ecx, 2 ; 0a0d is not included
        call    write_to_file

      @@:
        popad
        ret     4

    .set_flags:
        ret     4


;;================================================================================================;;
    .list: ;//////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? update tree2 with remote directory listing                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> buf_buffer2 = received data buffer                                                             ;;
;> eax = size of data                                                                             ;;
;> remote_list_buf = buffer containing last incomplete entry (if any)                             ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

        push    edx
        push    eax
        ; check if any incomplete entry to be parsed
        cmp     byte[remote_list_buf], 0
        je      .no_backlog
        ; find end of remote_list_buf
        mov     edi, remote_list_buf
        mov     al, 0
        repne   scasb
        ; copy rest of the incomplete entry to remote_list_buf
        mov     esi, buf_buffer2
        dec     edi

      @@:
        movsb
        cmp     word[esi-2], 0x0a0d
        jne     @b
        mov     byte[edi], 0
        mov     ecx, edi
        sub     ecx, remote_list_buf ; ecx = length of remote_list_buf
        mov     ebx, esi ; ebx = address at which next parse should start
        mov     edi, remote_list_buf
        jmp     .start_parsing

      .no_backlog:
        mov     ecx, eax    ; ecx = number of bytes remaining to parse in buf_buffer2
        mov     edi, buf_buffer2

      .start_parsing:
        ; find pointer to name
        cmp     byte[edi], 'd' ; file is a directory
        je      @f
        cmp     byte[edi], 'l' ; link
        je      @f
        mov     word[node_entry2], 'fi'
        jmp     .parse

      @@:
        mov     word[node_entry2], 'fo'

      .parse:
        mov     edx, 8

      @@:
        mov     al, ' '
        mov     ecx, [esp]
        add     ecx, buf_buffer2
        sub     ecx, edi
        
        repne   scasb

        ; check if the (last) entry is incomplete
        test    ecx, ecx
        jz      .store_last_entry

        rep     scasb
        test    ecx, ecx
        jz      .store_last_entry
        
        dec     edx
        jnz     @b

        lea     esi, [edi-1]
        lea     edi, [node_entry2+4]
      @@:
        movsb
        cmp     word[esi], 0x0a0d ; end of entry
        jne     @b
        mov     byte[edi], 0

        ; add node to tree list
        cmp     word[node_entry2], 'fi'
        jne     @f
        stdcall [tl_node_add], tree2, 0, node_entry2
        jmp     .added_remote_node

      @@:
        stdcall [tl_node_add], tree2, 0x10000, node_entry2

      .added_remote_node:
        ; check whether we are parsing remote_list_buf
        cmp     byte[remote_list_buf], 0
        je      @f
        mov     byte[remote_list_buf], 0 ; done parsing remote_list_buf
        mov     edi, ebx
        mov     eax, [esp]
        mov     ecx, buf_buffer2
        add     ecx, eax
        sub     ecx, ebx
        jmp     .start_parsing

      @@:
        ; any more entries to parse?
        lea     edi, [esi+2] ; skip 0a0d
        cmp     byte[edi], 0
        jne     .start_parsing
        jmp     .done_parsing

      .store_last_entry:
        ; find index of the last incomplete entry
        mov     ecx, -1
        mov     eax, [esp]
        lea     edi, [buf_buffer2+eax-2]
        mov     al, 0x0a
        std
        repne   scasb
        cld

        ; copy last (incomplete) entry of buf_buffer2 to remote_list_buf
        lea     esi, [edi+2]
        mov     edi, remote_list_buf
      @@:
        movsb
        cmp     byte[esi-1], 0
        jne     @b

      .done_parsing:
        call    .draw ; to update tree list immediately in case of "auto_list"
        pop     eax
        pop     edx
        jmp     data_loop

        ; clear tree list and add ".." node before executing "LIST"
      .clear_remote_tree:
        stdcall [tl_info_clear], tree2
        mov     dword[node_entry2+4], 0x2E2E
        stdcall [tl_node_add], tree2, 0x10000, node_entry2
        ret


;;================================================================================================;;
    .progress: ;//////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Update progress bar every time new data is received from data_loop block                       ;;
;;------------------------------------------------------------------------------------------------;;
;> edx = no. of bytes downloaded                                                                  ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

        ; detect abort
        mcall   17
        cmp     ah, 10
        je      cmd_abor

        push    eax ebx ecx edx
        mov     ebx, [file_size]
        mov     ecx, 100
        mov     eax, edx
        mul     ecx
        div     ebx
        cmp     eax, 100
        jle     @f
        mov     eax, 100

      @@:
        mov     dword[pb], eax
        push    pb
        call    [progressbar_draw]
        pop     edx ecx ebx eax
        ret


remove_dir:

    stdcall [tl_node_get_data], tree2
    cmp     word[eax], 'fo'
    jne     gui.still
    lea     esi, [eax+4]
    lea     edi, [buf_cmd+4]
  @@:
    movsb
    cmp     byte[esi], 0
    jne     @b
    mov     word[edi], 0x000a
    jmp     cmd_rmd


delete_file:

    stdcall [tl_node_get_data], tree2
    cmp     word[eax], 'fi'
    jne     gui.still
    lea     esi, [eax+4]
    lea     edi, [buf_cmd+5]
  @@:
    movsb
    cmp     byte[esi], 0
    jne     @b
    mov     word[edi], 0x000a
    jmp     cmd_dele


create_remote_dir:

    lea     edi, [buf_cmd+4]
    mov     esi, new_dir_buf
  @@:
    movsb
    cmp     byte[esi], 0
    jne     @b
    mov     word[edi], 0x000a
    jmp     cmd_mkd


; Function to call when you press [Enter]
align 4
fun_on_enter:

    stdcall [tl_node_get_data], tree1
    cmp     word[eax], 'fi'
    je      @f
    lea     ecx, [eax+4] ; discard first 4 bytes
    call    populate_local_tree_list.read_new_dir
    ;stdcall [tl_draw], tree1
    ret

  @@:
    lea     esi, [eax+4]
    lea     edi, [buf_cmd+5]
  @@:
    movsb
    cmp     byte[esi], 0
    jne     @b
    mov     word[edi], 0x000a
    jmp     cmd_stor


; Function to call when you press [Enter]
; for treelist with remote directory listing
align 4
fun_on_enter2:

    stdcall [tl_node_get_data], tree2
    lea     esi, [eax+4]
    cmp     dword[esi], 0x2E2E
    jne     @f
    mov     [auto_list], 1
    jmp     cmd_cdup

  @@:
    cmp     word[eax], 'fo'
    je      @f
    lea     edi, [buf_cmd+5]
    jmp     .copy

  @@:
    lea     edi, [buf_cmd+4]

  .copy:
    movsb
    cmp     byte[esi], 0
    jne     .copy
    mov     word[edi], 0x000a ; extra character before 0 to compensate for
                                ; newline in console code
    cmp     word[eax], 'fo'
    je      @f
    jmp     cmd_retr

  @@:
    stdcall [tl_info_clear], tree2
    mov     [auto_list], 1
    jmp     cmd_cwd


;;================================================================================================;;
populate_local_tree_list: ;///////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Update tree1 with file names from current working directory                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> none                                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

    mov     [filestruct.subfn], 1
    mov     [filestruct.size], 32
    mov     [filestruct.ptr], folder_data
    mov     ecx, root_dir

  .read_new_dir: ; ecx = pointer to folder name
    ; parse ".."
    cmp     word[ecx], '..'
    jne     @f
    cmp     byte[ecx+2], 0
    jne     @f

    ; removes last file name from filestruct.name
    mcall   30, 2, filestruct.name,1024 ; get absolute path for cwd
    lea     edi, [filestruct.name+eax] ; edi = pointer to the tail of file name
    mov     al, '/'
    mov     ecx, -1
    std
    repne   scasb
    cld
    mov     byte[edi+1], 0
    mov     ecx, filestruct.name

  @@:
    mcall   30, 1, ; set cwd
    mcall   30, 2, filestruct.name, 1024 ; get absolute path for cwd
    ; clear all nodes in tree list
    stdcall [tl_info_clear], tree1
    mov     [filestruct.offset], 0

  ; read 32 blocks
  .outer:
    mcall   70, filestruct
    ;int3
    test    eax, eax
    jz      @f
    cmp     eax, 6 ; EOF
    je      @f
    call    error_fs
    jmp     .done
  @@:
    test    ebx, ebx
    je      .done ; when no. of blocks is multiple of 32
    add     [filestruct.offset], ebx
    mov     ecx, ebx
    mov     edx, folder_data+32+40 ; pointer to first file's name in BDFE

  .inner:   ; ebx = total number of BDFEs
            ; ecx = number of BDFEs remaining
            ; edx = pointer to file's name in BDFE
    ; ignore "." entry
    cmp     word[edx], 0x002E
    je      .added
    ; copy file/folder name to node_entry
    mov     esi, edx
    mov     edi, node_entry+4
  @@:
    movsb
    cmp     byte [esi-1], 0
    jne     @b
    ; check whether this is a file or a folder
    mov     al, byte[edx-40]
    and     al, 0x10
    jz      @f
    ; add a folder to tree
    mov     word[node_entry], 'fo'
    stdcall [tl_node_add], tree1, 0x10000, node_entry
    jmp     .added

  @@:
    ; add a file to tree
    mov     word[node_entry], 'fi'
    stdcall [tl_node_add], tree1, 0, node_entry

  .added:
    add     edx, 304 ; 304 = length of BDFE. jump to next file's name
    loop    .inner
    stdcall [tl_draw], tree1
    cmp     ebx, 32
    je      .outer ; more BDFEs to come

  .done:
    ret    


MIN_M_WND_H equ 500 ;
;input:
; edi = pointer to tedit struct
align 4
EvSize:
    pushad
    mov     ebx, ted_scr_h
    mov     esi, ted_scr_w

    m2m     ted_wnd_w, [procinfo.client_box.width]
    mov     eax, ted_wnd_l
    sub     ted_wnd_w, eax
    sub     ted_wnd_w, 70
    movzx   eax, word[esi+sb_offs_size_x]
    sub     ted_wnd_w, eax

    ;m2m    ted_wnd_h,[procinfo.client_box.height] ;
    ;cmp    ted_wnd_h,MIN_M_WND_H
    ;jg     @f
    mov     ted_wnd_h, MIN_M_WND_H
    ;@@:

    movzx   eax, word[ebx+sb_offs_size_y]
    sub     ted_wnd_h, eax
    mov     eax, ted_wnd_t
    sub     ted_wnd_h, eax

    stdcall [ted_init_scroll_bars], tedit0,2

    popad
    ret


;;================================================================================================;;
filter_remote_list: ;/////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Filter tree list according to pattern specified in 'filter'                                    ;;
;;------------------------------------------------------------------------------------------------;;
;> none                                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;
        stdcall [tl_cur_beg], tree2

    .next_node:
        stdcall [tl_node_get_data], tree2
        ; check whether any nodes left
        test    eax, eax
        jnz     @f
        stdcall [tl_draw], tree2
        stdcall dword[edit_box_set_text], edit_search, str_null
        je      gui.still

    @@:
        cmp     byte[filter], 0
        je      gui.still

        ; calculate size of node entry
        lea     esi, [eax+4]
        mov     edi, esi
        mov     ecx, -1
    @@:
        lodsb
        inc     ecx
        test    al, al
        jnz     @b

        ; calculate size of pattern
        mov     esi, filter
        mov     edx, -1
    @@:
        lodsb
        inc     edx
        test    al, al
        jnz     @b

        mov     esi, filter

        call    search
        pop     esi
        test    al, al

        jnz     .get_next
        stdcall [tl_node_delete], tree2
        jmp     @f

    .get_next:
        stdcall [tl_cur_next], tree2

    @@:
        jmp     .next_node


;;================================================================================================;;
search: ;/////////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Search for a substring within a string                                                         ;;
;;------------------------------------------------------------------------------------------------;;
;> edi = pointer to haystack                                                                      ;;
;> ecx = size of haystack                                                                         ;;
;> esi = pointer to needle                                                                        ;;
;> edx = size of needle                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< al = 0 (not found) / 1 (found)                                                                 ;;
;;================================================================================================;;
    
    push    ebx ecx edx esi edi
    ; if length of haystack < length of needle
    cmp     ecx, edx
    jge     @f
    mov     bh, 0
    jmp     .done
  @@:
    mov     bh, 1 ; flag (0 = not found, 1 = found)
    sub     ecx, edx
    dec     edi

  .outer:
    cmp     ecx, 0
    jl      .done
    dec     ecx
    inc     edi
    mov     bh, 1 ; flag (0 = not found, 1 = found)
    mov     edx, [esp+8]
    mov     esi, [esp+4]

  .inner:
    test    edx, edx
    jz      .done
    dec     edx

    mov     eax, esi
    sub     eax, [esp+4]
    mov     al, byte[edi+eax]
    mov     bl, byte[esi]

    inc     esi
    cmp     al, bl
    je      .inner
    mov     bh, 0
    jmp     .outer


  .done:
    mov     al, bh
    pop     edi esi edx ecx ebx
    ret

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

;hed             db 'FTP Client for KolibriOS',0
str_change      db 'Change',0
str_disconnect  db 'Disconnect',0
str_copy        db 'Copy',0
str_mkd         db 'MKD',0
str_dele        db 'DELE',0
str_rdir        db 'RDIR',0
str_rmd         db 'RMD',0
str_abort       db 'Cancel',0
str_search      db 'Search',0
str_list        db 'LIST',0
str_null        db 0

;----------------------
; tree list elements
;----------------------
tree1 tree_list 64, 500, tl_list_box_mode, 16, 16, \
    0xffffff, BT_COL, 0x000000, TLIST1_X, 105, TLIST_SZ-16, 185, 14, 4,\
    0, el_focus, wScr, fun_on_enter

tree2 tree_list 64, 500, tl_list_box_mode, 16, 16, \
    0xffffff, BT_COL, 0x000000, TLIST2_X, 105, TLIST_SZ-16, 185, 14, 4, \
    0, el_focus, wScr2, fun_on_enter2

; editbox for mkd
edit_mkd edit_box TLIST_SZ-45, TLIST2_X, 82, 0xffffff, 0x6a9480, 0, \
    0xAABBCC, 0, 99, new_dir_buf, mouse_dd, 0

; editbox for default volume
edit_volume edit_box TLIST_SZ-60, TLIST1_X, 105-20, 0xffffff, 0x6a9480, 0, \
    0xAABBCC, 0, 99, root_dir, mouse_dd, 0

; editbox for search
edit_search edit_box TLIST_SZ-60, TLIST2_X, 25, 0xffffff, 0x6a9480, 0, \
    0xAABBCC, 0, 99, filter,mouse_dd, 0
 
; A subsidiary structure for scrolling
align 4
wScr:
    .x:
    .size_x     dw 16 ;+0
    .start_x    dw 250+50-16 ;+2
    .y:
    .size_y     dw 185 ;+4
    .start_y    dw 105 ;+6
    .btn_high   dd 15 ;+8
    .type       dd 1  ;+12
    .max_area   dd 100  ;+16
    .cur_area   dd 30  ;+20
    .position   dd 0  ;+24
    .bckg_col   dd 0xeeeeee ;+28
    .frnt_col   dd 0xbbddff ;+32
    .line_col   dd 0  ;+36
    .redraw     dd 0  ;+40
    .delta      dw 0  ;+44
    .delta2     dw 0  ;+46
    .run_x:
    .r_size_x   dw 0  ;+48
    .r_start_x  dw 0  ;+50
    .run_y:
    .r_size_y   dw 0 ;+52
    .r_start_y  dw 0 ;+54
    .m_pos      dd 0 ;+56
    .m_pos_2    dd 0 ;+60
    .m_keys     dd 0 ;+64
    .run_size   dd 0 ;+68
    .position2  dd 0 ;+72
    .work_size  dd 0 ;+76
    .all_redraw dd 0 ;+80
    .ar_offset  dd 1 ;+84

align 4
wScr2:
    .x:
    .size_x     dw 16 ;+0
    .start_x    dw 675+50-16 ;+2
    .y:
    .size_y     dw 185 ;+4
    .start_y    dw 105 ;+6
    .btn_high   dd 15 ;+8
    .type       dd 1  ;+12
    .max_area   dd 100  ;+16
    .cur_area   dd 30  ;+20
    .position   dd 0  ;+24
    .bckg_col   dd 0xeeeeee ;+28
    .frnt_col   dd 0xbbddff ;+32
    .line_col   dd 0  ;+36
    .redraw     dd 0  ;+40
    .delta      dw 0  ;+44
    .delta2     dw 0  ;+46
    .run_x:
    .r_size_x   dw 0  ;+48
    .r_start_x  dw 0  ;+50
    .run_y:
    .r_size_y   dw 0 ;+52
    .r_start_y  dw 0 ;+54
    .m_pos      dd 0 ;+56
    .m_pos_2    dd 0 ;+60
    .m_keys     dd 0 ;+64
    .run_size   dd 0 ;+68
    .position2  dd 0 ;+72
    .work_size  dd 0 ;+76
    .all_redraw dd 0 ;+80
    .ar_offset  dd 1 ;+84
;----------------------
; end tree list element
;----------------------

;----------------------
; textedit elements
;----------------------
align 4
tedit0:
    .wnd BOX 50,355,325,240 ;+ 0 195,5+40,325,240
    .rec BOX 30,13,6,10   ;+16
    .drag_m db 0 ;+32
    .drag_k db 0 ;+33
    .sel rb 16 ;+34
    .seln rb 16 ;+50
    .tex      dd 0 ;+66 text memory pointer
    .tex_1    dd 0 ;+70 text first symbol pointer
    .tex_end  dd 0 ;+74 text end memory pointer
    .cur_x    dd 0 ;+78
    .cur_y    dd 0 ;+82
    .max_chars dd 4096 ;+86
    .count_colors_text dd 1 ;+90
    .count_key_words   dd 0 ;+94
    .color_cursor      dd SYS_COL  ;+98
    .color_wnd_capt    dd BT_COL   ;+102
    .color_wnd_work    dd 0xffffff ;+106
    .color_wnd_bord    dd 0x737373 ;+110
    .color_select      dd 0xA4C4E4 ;+114
    .color_cur_text    dd 0xffff00 ;+118
    .color_wnd_text    dd 0x000000 ;+122
    .syntax_file       dd 0 ;+126
    .syntax_file_size  dd 500 ;+130
    .text_colors       dd 0 ;+134
    .help_text_f1      dd 0 ;+138
    .help_id       dd -1 ;+142
    .key_words_data    dd 0 ;+146
    .tim_ch      dd ? ;+150
    .tim_undo    dd ? ;+154
    .tim_ls      dd ? ;+158
    .tim_co      dd ? ;+162
    .el_focus    dd el_focus ;+166
    .err_save    db 0 ;+170
    .panel_id    db 0 ;+171
    .key_new     db 0 ;+172
    .symbol_new_line db 20 ;+173
    .scr_w       dd scrol_w1 ;+174
    .scr_h       dd scrol_h1 ;+178
    .arr_key_pos dd 0 ;+182
    .buffer      dd ed_buffer ;+186
    .buffer_find dd 0 ;+190
    .cur_ins     db 1 ;+194
    .mode_color  db 1 ;+195
    .mode_invis  db 0 ;+196
    .gp_opt      db 0 ;+197
    dd 0 ;.fun_on_key_ctrl_all dd but_ctrl_all ;+198 
    dd 0,0,0 ;reserved
    .buffer_size       dd 4096 ;+214
    .fun_find_err      dd 0 ;+218
    .fun_init_synt_err dd 0 ;+222
    .fun_draw_panel_buttons dd 0 ;+226
    .fun_draw_panels    dd 0 ;+230
    dd 0 ;+234 reserved
    .fun_save_err       dd 0 ;+238
    .increase_size dd 1000 ;+242
    .ptr_free_symb dd ? ;+246
    .font_size     dd 16 shl 16 ; +250

align 4
scrol_w1:
    .x:
    .size_x   dw 16 ;+0
    .start_x  dw 85 ;+2
    .y:
    .size_y   dw 100 ; +4
    .start_y  dw  15 ; +6
    .btn_high dd  15 ; +8
    .type     dd   1 ;+12
    .max_area dd 100 ;+16
    rb 4+4
    .bckg_col dd 0xeeeeee ;+28
    .frnt_col dd 0xbbddff ;+32
    .line_col dd 0x808080 ;+36
    .redraw   dd   0 ;+40
    .delta    dw   0 ;+44
    .delta2   dw   0 ;+46
    .run_x:
    rb 2*4+4*6
    .all_redraw dd 0 ;+80
    .ar_offset  dd 1 ;+84
;---------------------------------------------------------------------
align 4
scrol_h1:
    .x:
    .size_x     dw 85 ;+0
    .start_x    dw 30 ;+2
    .y:
    .size_y     dw 16 ;+4
    .start_y    dw 100 ;+6
    .btn_high   dd 15 ;+8
    .type       dd 1  ;+12
    .max_area   dd 100 ;+16
    rb 4+4
    .bckg_col   dd 0xeeeeee ;+28
    .frnt_col   dd 0xbbddff ;+32
    .line_col   dd 0x808080 ;+36
    .redraw     dd 0  ;+40
    .delta      dw 0  ;+44
    .delta2     dw 0  ;+46
    .run_x:
    rb 2*4+4*6
    .all_redraw dd 0 ;+80
    .ar_offset  dd 1 ;+84

;------------
;Progress bar
;------------
struc pb
{
    dd 0  ; PB_VALUE          equ +0
    dd (TLIST2_X+TLIST1_X+TLIST_SZ)/2-150 ; PB_LEFT equ +4. 150 = PB_WIDTH/2
    dd 315; PB_TOP            equ +8
    dd 300; PB_WIDTH          equ +12
    dd 15 ; PB_HEIGHT         equ +16
    dd ?  ; PB_STYLE          equ +20
    dd 0  ; PB_MIN            equ +24
    dd 100; PB_MAX            equ +28
    dd 0xffffff; PB_BACK_COLOR     equ +32
    dd 0x00ff40; PB_PROGRESS_COLOR equ +36
    dd 0x808080; PB_FRAME_COLOR    equ +40
}
pb pb


align 4
@IMPORT_GUI_LIBS:

library box_lib, 'box_lib.obj', libimg, 'libimg.obj'

import  box_lib, \
        edit_box_draw,      'edit_box', \
        edit_box_key,       'edit_box_key', \
        edit_box_mouse,     'edit_box_mouse', \
        edit_box_set_text,  'edit_box_set_text', \
        version_ed,         'version_ed', \
        scrollbar_ver_draw ,'scrollbar_v_draw', \
        scrollbar_ver_mouse,'scrollbar_v_mouse', \
        scrollbar_hor_draw, 'scrollbar_h_draw', \
        scrollbar_hor_mouse,'scrollbar_h_mouse', \
        version_scrollbar,  'version_scrollbar', \
        tl_data_init,       'tl_data_init', \
        tl_data_clear,      'tl_data_clear', \
        tl_info_clear,      'tl_info_clear', \
        tl_key,             'tl_key', \
        tl_mouse,           'tl_mouse', \
        tl_draw,            'tl_draw', \
        tl_node_add,        'tl_node_add', \
        tl_node_set_data,   'tl_node_set_data', \
        tl_node_get_data,   'tl_node_get_data', \
        tl_node_delete,     'tl_node_delete', \
        tl_cur_beg,         'tl_cur_beg', \
        tl_cur_next,        'tl_cur_next', \
        tl_cur_perv,        'tl_cur_perv', \
        tl_node_close_open, 'tl_node_close_open', \
        ted_but_sumb_upper, 'ted_but_sumb_upper', \
        ted_but_sumb_lover, 'ted_but_sumb_lover', \
        ted_can_save,       'ted_can_save', \
        ted_clear,          'ted_clear', \
        ted_delete,         'ted_delete', \
        ted_draw,           'ted_draw', \
        ted_init,           'ted_init', \
        ted_init_scroll_bars, 'ted_init_scroll_bars', \
        ted_init_syntax_file, 'ted_init_syntax_file', \
        ted_is_select,      'ted_is_select', \
        ted_key,            'ted_key', \
        ted_mouse,          'ted_mouse', \
        ted_open_file,      'ted_open_file', \
        ted_save_file,      'ted_save_file', \
        ted_text_add,       'ted_text_add', \
        ted_but_select_word,'ted_but_select_word', \
        ted_but_cut,        'ted_but_cut', \
        ted_but_copy,       'ted_but_copy', \
        ted_but_paste,      'ted_but_paste', \
        ted_but_undo,       'ted_but_undo', \
        ted_but_redo,       'ted_but_redo', \
        ted_but_reverse,    'ted_but_reverse', \
        ted_but_find,       'ted_but_find', \
        ted_but_replace,    'ted_but_replace', \
        ted_text_colored,   'ted_text_colored', \
        ted_go_to_position, 'ted_go_to_position', \
        progressbar_draw,   'progressbar_draw'

import  libimg, \
        lib_init1         , 'lib_init', \
        img_is_img        , 'img_is_img', \;определяет по данным, может ли библиотека сделать из них изображение \
        img_info          , 'img_info', \
        img_from_file     , 'img_from_file', \
        img_to_file       , 'img_to_file', \
        img_from_rgb      , 'img_from_rgb', \
        img_to_rgb        , 'img_to_rgb', \;преобразование изображения в данные RGB \
        img_to_rgb2       , 'img_to_rgb2', \
        img_decode        , 'img_decode', \ ;автоматически определяет формат графических данных \
        img_encode        , 'img_encode', \
        img_create        , 'img_create', \
        img_destroy       , 'img_destroy', \
        img_destroy_layer , 'img_destroy_layer', \
        img_count         , 'img_count', \
        img_lock_bits     , 'img_lock_bits', \
        img_unlock_bits   , 'img_unlock_bits', \
        img_flip          , 'img_flip', \
        img_flip_layer    , 'img_flip_layer', \
        img_rotate        , 'img_rotate', \
        img_rotate_layer  , 'img_rotate_layer', \
        img_draw          , 'img_draw'


run_file_70 FileInfoBlock ; required for libimg

auto_list   db 1 ; to run LIST immediately after CWD
filter      rb 512
root_dir    db '/rd/1'
            rb 507
new_dir_buf rb 512
folder_data rb 32+32*304
node_entry  db '1111'
            rb 512
node_entry2 db '1111'
            rb 512
remote_list_buf rb 1024
file_name   rb 4096 ; required for libimg
conv_tabl   rb 128
ed_buffer   rb 100
tedit_buffer rb 1024
el_focus    dd 0
;-----------------------

procinfo        process_information