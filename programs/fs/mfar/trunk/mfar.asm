
;   MENUET FAR [MFAR] PRE-ALPHA-8.1
;   2003-2004 (C) Mike Semenyako aka mike.dld
;   Compile with FASM for Menuet

use32
org 0

  db 'MENUET01' ; 8 byte id
  dd $01        ; header version
  dd START      ; start of code
  dd I_END      ; size of image
  dd MEM_USED   ; memory for app
  dd $007FF0    ; esp
  dd $00,$00    ; I_Param , I_Icon

;
;     START:SIZE
;
; +00000000:00007FF0 - image
; +00007FF0:00007FF0 - stack
; +00008000:00000300 - path strings buffer
; +00008300:0003FD00 - files data
; +00048000:00000300 - temporary file read area
; +00048300:00004000 - memory for OS (read/write buffer)
; +0004C300:FFFB3CFF - dinamically allocated for copy, view, edit etc.
;

include 'lang.inc'
include 'macros.inc'
include 'menuet.inc'
include 'mfar.inc'

START:

        mcall   MF_RDREADFILE,fcfile,0,-1,fc ; read user colors

        mov     esi,p_rd                ; left panel = RD
        mov     edi,f_path0
        mov     ecx,p_rd.size
        rep     movsb
        mov     esi,p_hd                ; right panel = HD
        mov     edi,f_path1
        mov     ecx,p_hd.size
        rep     movsb

redraw_all:
        call    draw_window_full
        jmp     still
redraw_files:
        call    draw_files
still:
        mcall   MF_WAITEVWTO,100        ; waiting 1 sec
        jmpe    al,,\
          EV_REDRAW,redraw_all,\
          EV_KEY,key,\
          EV_BUTTON,button

;  after every 1 second [when no other events occur] updating files info
;  eg. requesting ACTIVE panel files data and drawing it
;; timed redraw {
        mov     dl,[active_panel]
        cmp     dl,0
        jne    ._00
        push    [d_ltsz]
        jmp     @f
   ._00:
        push    [d_rtsz]
    @@:
        call    get_files_data
        pop     eax
        cmp     dl,0
        jne    ._01
        cmp     eax,[d_ltsz]
        jmp     @f
   ._01:
        cmp     eax,[d_rtsz]
    @@:
        je      still                   ; nothing happened, waiting again
        mov     al,dl
        jmp     redraw_files
;; }

; key pressed event
 key:
        mcall   MF_GETKEY               ; get key-code

        movzx   ecx,[active_panel]

        jmpe    ah,k_directional,\
          VK_RETURN,k_return,\          ; execute something
          VK_TAB,k_tab,\                ; change active panel
          VK_F5,k_f5,\                  ; copy
          VK_F10,mfar_exit,\            ; terminate
          VK_CTRLF3,k_ctrl_f3,\         ; sort by name
          VK_CTRLF4,k_ctrl_f4           ; sort by extension
;------------------------------------------------------------------------------
  k_return:
        call    execute_current_file
        jmp     still
;------------------------------------------------------------------------------
  k_tab:
        mov     edi,[fc.background]
        call    draw_sel                ; hide current file selection
        xor     [active_panel],1        ; changing active panel
        call    draw_path               ; drawing path of new panel
        mov     dl,[active_panel]
        call    get_files_data          ; requesting panel files info
        mov     edi,sel_color
        call    draw_sel                ; show current file
        mov     al,[active_panel]
        call    draw_file_info          ; and its info (size,date)
        jmp     still
;------------------------------------------------------------------------------
  k_f5:
        call    copy_file
        jc      still
        jmp     redraw_all
;------------------------------------------------------------------------------
  k_ctrl_f3:
        jecxz  ._00
        cmp     [r_sort],0
        je      still
        mov     [r_sort],0
        jmp     @f
   ._00:
        cmp     [l_sort],0
        je      still
        mov     [l_sort],0
    @@:
        mov     al,cl
        call    mfar_sort
        jmp     redraw_files
;------------------------------------------------------------------------------
  k_ctrl_f4:
        jecxz  ._00
        cmp     [r_sort],1
        je      still
        mov     [r_sort],1
        jmp     @f
   ._00:
        cmp     [l_sort],1
        je      still
        mov     [l_sort],1
    @@:
        mov     al,al
        call    mfar_sort
        jmp     redraw_files
;------------------------------------------------------------------------------
  k_directional:
        cmp     ah,VK_LEFT              ; non-directional keys ?
        jb      still                   ; ignore them
        cmp     ah,VK_END
        ja      still
        jecxz  ._00
        mov     edx,[d_rpos]
        mov     ebx,[d_rtop]
        mov     esi,[d_rcnt]
        jmp     @f
   ._00:
        mov     edx,[d_lpos]
        mov     ebx,[d_ltop]
        mov     esi,[d_lcnt]
    @@:
        xor     ebp,ebp                 ; redraw whole panel [0-no, 1-yes]
;------------------------------------------------------------------------------
        jmpe    ah,show_cur,\
          VK_LEFT,k_left,\
          VK_DOWN,k_down,\
          VK_UP,k_up,\
          VK_RIGHT,k_right,\
          VK_HOME,k_home,\
          VK_END,k_end
  k_left:
        cmp     edx,0
        je      still
        sub     edx,FPC
        jge     @f
        xor     edx,edx
    @@:
        mov     eax,edx
        sub     eax,ebx
        jge     show_cur
        inc     ebp
        sub     ebx,FPC
        jge     show_cur
        xor     ebx,ebx
        jmp     show_cur
;------------------------------------------------------------------------------
  k_down:
        lea     eax,[esi-1]
        cmp     edx,eax
        je      still
        inc     edx
        mov     eax,edx
        sub     eax,ebx
        cmp     eax,FPC*3-1
        jle     show_cur
        inc     ebp
        inc     ebx
        jmp     show_cur
;------------------------------------------------------------------------------
  k_up:
        cmp     edx,0
        je      still
        dec     edx
        mov     eax,ebx
        cmp     edx,eax
        jge     show_cur
        inc     ebp
        dec     ebx
        jmp     show_cur
;------------------------------------------------------------------------------
  k_right:
        lea     eax,[esi-1]
        cmp     edx,eax
        je      still
        add     edx,FPC
        cmp     edx,eax
        jle     @f
        mov     edx,eax
    @@:
        mov     eax,edx
        sub     eax,ebx
        cmp     eax,FPC*3-1
        jle     show_cur
        inc     ebp
        add     ebx,FPC
        jmp     show_cur
;------------------------------------------------------------------------------
  k_home:
        cmp     edx,0
        je      still
        inc     ebp
        xor     edx,edx
        xor     ebx,ebx
        jmp     show_cur
;------------------------------------------------------------------------------
  k_end:
        lea     eax,[esi-1]
        cmp     edx,eax
        je      still
        mov     edx,eax
        mov     edi,eax
        sub     edi,ebx
        cmp     edi,FPC*3-1
        jle     show_cur
        inc     ebp
        sub     eax,FPC*3-1
        mov     ebx,eax
;------------------------------------------------------------------------------

  show_cur:

        mov     edi,[fc.background]
        call    draw_sel

        jecxz  ._00
        mov     [d_rpos],edx
        mov     [d_rtop],ebx
        jmp     @f
   ._00:
        mov     [d_lpos],edx
        mov     [d_ltop],ebx
    @@:

        cmp     ebp,0
        mov     al,cl
        jne     redraw_files

        mov     edi,[fc.selection]
        call    draw_sel
        call    draw_file_info
        jmp     still

  button:
        mcall   MF_GETPRSDBTNID
        cmp     ah,1
        jne     noclose

  mfar_exit:
        mcall   MF_RDDELFILE,fcfile             ; delete existing mfar.dat
        mcall   MF_RDWRITEFILE,fcfile,fc,12*4,0 ; create new mfar.dat

        mcall   MF_TERMINATE                    ; close program
  noclose:
        jmp     still

;------------------------------------------------------------------------------
;///// DRAW WINDOW ////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func draw_window
        mcall   MF_WINPROPS,WP_GETSYSCLRS,sc,sizeof.system_colors

        mcall   MF_WNDDRAW,WD_BEGINDRAW
        mov     edx,[fc.background]
        or      edx,WS_SKINNED
        mcall2  MF_DEFWINDOW,90,oX+tW*80+4,45,oY+tH*25+4
        mcall1  MF_DRAWTEXT,8,8,[sc.grab_text],caption,caption.size

        mmov    esi,oX+2,oX+tW*39+2
        mmov    edi,oY+3,oY+tH*22+3
        mov     edx,[fc.default]
        call    draw_frame
        mmov    esi,oX+tW*40+2,oX+tW*79+2
        call    draw_frame

        mcall2  MF_DRAWLINE,oX+tW*13+2,oX+tW*13+2,oY+tH,oY+tH*20+3
        madd    ebx,tW*13,tW*13
        mcall
        mcall2  ,oX+3,oX+tW*39+1,oY+tH*20+3,oY+tH*20+3
        mcall2  ,oX+tW*53+2,oX+tW*53+2,oY+tH,oY+tH*20+3
        madd    ebx,tW*13,tW*13
        mcall
        mcall2  ,oX+tW*40+3,oX+tW*79+1,oY+tH*20+3,oY+tH*20+3

        mcall1  MF_DRAWTEXT,oX+tW*5,oY+tH,$00ffff00,lname,lname.size
        mcall1  ,oX+tW*18,oY+tH
        mcall1  ,oX+tW*31,oY+tH
        mcall1  ,oX+tW*45,oY+tH
        mcall1  ,oX+tW*58,oY+tH
        mcall1  ,oX+tW*71,oY+tH

        movzx   edi,[l_sort]
        lea     edx,[edi+sort_kind]
        mcall1  ,oX+tW,oY+tH,$00ffff00,,1
        movzx   edi,[r_sort]
        lea     edx,[edi+sort_kind]
        mcall1  ,oX+tW*41,oY+tH

        call    draw_path
        call    draw_bottom_keys

        mcall   MF_WNDDRAW,WD_ENDDRAW
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func draw_window_full
        call    draw_window
        mov     edx,1
        call    get_files_data
        mov     al,1
        call    draw_files
        mov     edx,0
        call    get_files_data
        mov     al,0
        call    draw_files
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

align 4
len dd ?

func get_normal_path
        pusha
        mov     ecx,5
        rep     movsb
        dec     esi
        dec     edi
        mov     ecx,5
        push    eax
    @@:
        dec     dword[esp]
        jz      @f
        lodsb
        cmp     al,0
        je      @b
        cmp     al,' '
        je      @b
        stosb
        inc     ecx
        jmp     @b
    @@:
        pop     eax
        mov     byte[edi],'>'
        mov     [len],ecx
        popa
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; EAX = length needed
; ECX = current length
; EDI = path string
func get_path_ellipses
        cmp     ecx,eax
        jbe     @f
        pushad
        lea     esi,[edi+ecx+3+6]       ; ... + /??/?/
        sub     esi,eax
        add     ecx,edi
        sub     ecx,esi
        mov     dword[edi+6],'... '
        add     edi,9
        cld
        rep     movsb
        mov     [len],eax
        popad
    @@:
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func draw_path
        pushad
        cmp     [active_panel],0
        jne    ._00
        mov     esi,f_path0
        mov     eax,[f_plen0]
        jmp     @f
   ._00:
        mov     esi,f_path1
        mov     eax,[f_plen1]
    @@:
        mov     edi,f_pathn
        call    get_normal_path
        mov     eax,39
        mov     ecx,[len]
        mov     edi,f_pathn
        call    get_path_ellipses
        mcall2  MF_FILLRECT,oX-1,tW*80+1,oY+tH*23-1,tH,[fc.pathbg]
        mcall1  MF_DRAWTEXT,oX,oY+tH*23,[fc.path],f_pathn,[len]
        popad
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; AL = panel
func draw_files
        push    eax
        mmov    ecx,oY+tH*2-1,tH*FPC
        mov     edx,[fc.background]
        cmp     al,0
        mov     eax,MF_FILLRECT
        jne    ._00
        mcall1  ,oX+tW-2,tW*12+3
        mcall1  ,oX+tW*14-2,tW*12+3
        mcall1  ,oX+tW*27-2,tW*12+3
        mmov    ebx,oX+tW,oY+tH*2
        mov     edx,[d_ltop]
        shl     edx,4
        add     edx,lstart
        mov     edi,[d_lcnt]
        mov     esi,edi
        sub     edi,[d_ltop]
        jmp     @f
   ._00:
        mcall1  ,oX+tW*41-2,tW*12+3
        mcall1  ,oX+tW*54-2,tW*12+3
        mcall1  ,oX+tW*67-2,tW*12+3
        mmov    ebx,oX+tW*41,oY+tH*2
        mov     edx,[d_rtop]
        shl     edx,4
        add     edx,rstart
        mov     edi,[d_rcnt]
        mov     esi,edi
        sub     edi,[d_rtop]
    @@:
        cmp     esi,0
        je     .exit_nok                ; no files

        mov     ecx,3
   .next_col:
        push    ecx
        mov     ecx,FPC
   .next_row:
        dec     edi
        jge     @f
        pop     eax
        jmp    .exit_ok
    @@:
        push    ecx
        call    get_file_color
        mov     esi,edx
        call    get_file_name
        push    edx
        mcall   4,,,f_name,12           ; draw file name
        pop     edx ecx
        add     ebx,tH

        add     edx,16
        dec     ecx
        jnz    .next_row
        pop     ecx
        dec     ecx
        jz     .exit_ok
        madd    ebx,tW*13,0
        mov     bx,oY+tH*2
        jmp    .next_col

   .exit_ok:
        mov     edi,sel_color
        call    draw_sel
   .exit_nok:
        pop     eax
        call    draw_file_info
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func draw_bottom_keys
        pushad
        mcall2  MF_FILLRECT,oX-1,tW*80+1,oY+tH*24-1,tH+1,[fc.pathbg]
        dec     ecx
        mcall1  ,oX+tW-1,tW*6+1,,[fc.keysbg]
        mov     esi,7
    @@:
        madd    ebx,tW*8,0
        mcall
        dec     esi
        jge     @b
        mcall1  ,oX+tW*73-1,tW*7+1
        mcall   MF_DRAWNUMBER,$00010100,1,oX*65536+(oY+tH*24),[fc.path]
        sub     edx,$00010000
    @@:
        inc     cl
        madd    edx,tW*8,0
        mcall
        cmp     cl,10
        jb      @b
        mcall1  MF_DRAWTEXT,oX+tW,oY+tH*24,[fc.keys],btmkeys,btmkeys.size
        popad
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; ESI = X1*65536+X2
; EDI = Y1*65536+Y2
; EDX = color
func draw_frame
        mov     ecx,edi
        mov     ebx,edi
        shr     ebx,16
        mov     cx,bx
        mcall   MF_DRAWLINE,esi
        mov     ecx,edi
        shl     ecx,16
        mov     cx,di
        mcall
        mov     ebx,esi
        mov     ecx,esi
        shr     ecx,16
        mov     bx,cx
        mcall   ,,edi
        mov     ebx,esi
        shl     ebx,16
        mov     bx,si
        mcall
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; EDX = pointer to file data
func get_file_color
        push    esi
        mov     cl,[edx+11]
        test    cl,(FA_HIDDEN or FA_SYSTEM)
        jz      @f
        mov     ecx,[fc.system]
        jmp    .exit
    @@:
        test    cl,FA_FOLDER
        jz      @f
        mov     ecx,[fc.folder]
        jmp    .exit
    @@:
        mov     ecx,[edx+7]
        and     ecx,$ffffff00
        or      ecx,$00000020

        mov     esi,ext0-4
    @@:
        lodsd
        or      eax,eax
        jz      @f
        cmp     ecx,eax
        jne     @b
        mov     ecx,[fc.executable]
        jmp    .exit
    @@:
        lodsd
        or      eax,eax
        jz      @f
        cmp     ecx,eax
        jne     @b
        mov     ecx,[fc.bitmap]
        jmp    .exit
    @@:
        lodsd
        or      eax,eax
        je      @f
        cmp     ecx,eax
        jne     @b
        mov     ecx,[fc.source]
        jmp    .exit
    @@:
        mov     ecx,[fc.default]
   .exit:
        pop     esi
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; EDI = color
func draw_sel
        pushad
        cmp     [active_panel],0
        jne    ._00
        mov     eax,[d_lpos]
        sub     eax,[d_ltop]
        mov     esi,[d_lcnt]
        jmp     @f
   ._00:
        mov     eax,[d_rpos]
        sub     eax,[d_rtop]
        mov     esi,[d_rcnt]
    @@:
        cmp     esi,0
        je     .exit
        mov     cl,FPC
        div     cl
        mov     bp,ax
        and     eax,$000000FF
        mov     cl,tW*13
        mul     cl
        add     ax,oX+tW-2
        push    eax
        shl     eax,16
        mov     ax,tW*12+3
        mov     ebx,eax
        mov     ax,bp
        shr     eax,8
        and     eax,$000000FF
        mov     cl,tH
        mul     cl
        add     ax,oY+tH*2-1
        push    eax
        shl     eax,16
        mov     ax,tH
        mov     ecx,eax
        mov     edx,edi
        cmp     [active_panel],0
        je      @f
        madd    ebx,(40*6),0
    @@:
        mcall   MF_FILLRECT

        pop     eax
        movzx   ebx,ax
        inc     bx
        pop     eax
        add     ax,2
        shl     eax,16
        or      ebx,eax
        cmp     [active_panel],0
        jne    ._01
        mov     edx,[d_lpos]
        shl     edx,4
        add     edx,lstart
        jmp     @f
   ._01:
        mov     edx,[d_rpos]
        shl     edx,4
        add     edx,rstart
    @@:
        mov     esi,edx
        call    get_file_name
        call    get_file_color
        cmp     ecx,edi
        jne     @f
        xor     ecx,ecx
    @@:
        cmp     [active_panel],0
        je      @f
        madd    ebx,tW*40,0
    @@:
        mcall   MF_DRAWTEXT,,,f_name,12
   .exit:
        popad
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; AL = panel
func draw_file_info
        push    eax
        mmov    ecx,oY+tH*21,tH
        mov     edx,[fc.background]
        cmp     al,0
        mov     eax,MF_FILLRECT
        jne    ._00
        mcall1  ,oX+tW,tW*38+1
        mov     esi,[d_lpos]
        shl     esi,4
        add     esi,lstart
        mov     edi,[d_lcnt]
        jmp     @f
   ._00:
        mcall1  ,oX+tW*41,tW*38+1
        mov     esi,[d_rpos]
        shl     esi,4
        add     esi,rstart
        mov     edi,[d_rcnt]
    @@:
        cmp     edi,0
        jne     @f
        pop     eax
        ret
    @@:
        call    get_file_info
        mov     dword[file_info+17],'    '
        mov     word[file_info+15],'  '
        test    byte[esi+11],FA_FOLDER
        jz      @f
        mov     dword[file_info+21],' DIR'
        jmp    .no_size
    @@:
        mov     dword[file_info+21],'    '
        mov     eax,[f_size]
        mov     esi,file_info+24
        mov     ebx,10
        mov     ecx,ebx
    @@:
        xor     edx,edx
        div     ebx
        add     dl,'0'
        mov     [esi],dl
        or      eax,eax
        jz     .no_size
        dec     esi
        loop    @b
   .no_size:
        pop     eax
        mmov    edx,oX+tW*35,oY+tH*21
        mmov    ebx,oX+tW,oY+tH*21
        cmp     al,0
        je      @f
        madd    edx,tW*40,0
        madd    ebx,tW*40,0
    @@:
        push    edx
        mcall   MF_DRAWTEXT,,[fc.default],file_info,34
        pop     edx
        mov     esi,ecx
        mov     cx,[f_year]
        mcall   MF_DRAWNUMBER,$00040000
        sub     edx,$00240000 ; 6*6 = $24
        movzx   ecx,[f_day]
        mcall   ,$00020000
        add     edx,$00120000
        mov     cl,[f_mnth]
        mcall
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func get_file_name
        pushad
        mov     eax,[esi+0]
        mov     [f_name+0],eax
        mov     eax,[esi+4]
        mov     [f_name+4],eax
        mov     eax,[esi+8]
        shl     eax,8
        and     eax,$FFFFFF00
        or      al,$00000020
        mov     [f_name+8],eax
        popad
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; ESI = pointer to file data
func get_file_info
        pushad
        mov     eax,[esi+12]
        mov     dword[f_info],FS_READ
        mov     [f_info+4],eax
        mov     dword[f_info+12],read_area
        mcall   MF_FSACCESS,f_info
        cmp     eax,ERR_SUCCESS
        je      @f
        cmp     eax,ERR_FS_LAST
        ja      @f
        popad
        stc
        ret
    @@:
        mov     eax,read_area
        mov     ecx,16
   .next_file:
        mov     edx,[esi+0]
        cmp     [eax+0],edx
        jne     @f
        mov     edx,[esi+4]
        cmp     [eax+4],edx
        jne     @f
        mov     edx,[esi+8]
        cmp     [eax+8],edx
        jne     @f
        mov     esi,eax
        jmp     .found
    @@:
        add     eax,32
        loop   .next_file
   .found:
        push    esi
        mov     eax,'    '
        mov     ecx,3
        mov     edi,file_info
        rep     stosd
        mov     edi,file_info
        mov     ecx,2
        rep     movsd
        mov     edi,file_info
    @@:
        cmp     byte[edi],' '
        jbe     @f
        inc     edi
        jmp     @b
    @@:
        mov     eax,[esi]
        shl     eax,8
        mov     al,'.'
        cmp     eax,'.   '
        je      @f
        mov     [edi],eax
    @@:
        pop     esi
        mov     eax,[esi+28]
        mov     [f_size],eax
        mov     eax,[esi+24]
        mov     [f_day],al
        and     [f_day],00011111b
        shr     eax,5
        mov     [f_mnth],al
        and     [f_mnth],00001111b
        shr     eax,4
        mov     [f_year],ax
        and     [f_year],01111111b
        add     [f_year],1980
        popad
        clc
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

; DL = panel
func get_files_data
        pushad
        mov     [d_tcnt],0
        mov     [d_ttsz],0
        cmp     dl,0
        jne    ._00
        mov     esi,f_path0
        mov     ecx,[f_plen0]
        jmp     @f
   ._00:
        mov     esi,f_path1
        mov     ecx,[f_plen1]
    @@:
        push    edi
        mov     edi,f_info.path
        mov     byte[edi+ecx],0
        rep     movsb
        pop     edi
        cmp     dl,0
        jne    ._01
        mov     edi,lstart
        jmp     @f
   ._01:
        mov     edi,rstart
    @@:
        push    edx
        xor     ebp,ebp
        mov     dword[f_info],FS_READ          ; read
        mov     dword[f_info+8],1              ; 1 block
        mov     dword[f_info+12],read_area     ; to read_area
   .next_block:
        mov     dword[f_info+4],ebp            ; starting from block #ebp
        mcall   MF_FSACCESS,f_info
        cmp     eax,ERR_SUCCESS
        je      @f
        cmp     eax,ERR_FS_LAST
        ja      @f
        jmp    .exit
    @@:
        mov     esi,read_area
        mov     ecx,16
   .next_file:
        cmp     word[esi],0             ; end of entries
        je     .exit
        cmp     word[esi],'. '          ; self-pointer
        je     .skip
        test    byte[esi+11],FA_LABEL   ; disk label
        jnz    .skip
        cmp     byte[esi+11],$0F        ; fat32
        je     .skip
        cmp     byte[esi],$E5           ; erased
        je     .skip
        mov     eax,[esi+28]            ; add file size
        add     [d_ttsz],eax            ; to folder size
        push    ecx
        mov     ecx,3
        rep     movsd                   ; copy name, attriputes
        mov     [edi],ebp               ; and block number
        add     edi,4
        add     esi,20
        inc     [d_tcnt]                ; increase folder files counter
;       cmp     [d_tcnt],max_cnt
;       je     .exit
        pop     ecx
        jmp     @f
   .skip:
        add     esi,32
    @@:
        loop   .next_file

        cmp     [d_tcnt],max_cnt
        je     .exit
        inc     ebp
        jmp    .next_block
   .exit:
        pop     edx
        push    [d_ttsz]
        mov     eax,[d_tcnt]
        cmp     dl,0
        jne    ._02
        mov     [d_lcnt],eax
        pop     [d_ltsz]
        jmp     @f
   ._02:
        mov     [d_rcnt],eax
        pop     [d_rtsz]
    @@:
        popad
        mov     al,dl
        call    mfar_sort
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func execute_current_file
        pushad
        cmp     [active_panel],0
        jne    ._00
        mov     esi,[d_lpos]
        shl     esi,4
        add     esi,lstart
        mov     edi,f_path0
        mov     ecx,f_plen0
        jmp     @f
   ._00:
        mov     esi,[d_rpos]
        shl     esi,4
        add     esi,rstart
        mov     edi,f_path1
        mov     ecx,f_plen1
    @@:
        call    get_file_info
        test    byte[esi+11],FA_FOLDER
        jz     .file
        cmp     word[esi],'..'
        jne    .enter_dir
        add     edi,[ecx]
        dec     edi
    @@:
        dec     [f_plen1]
        cmp     byte[edi],'/'
        je      @f
        dec     edi
        jmp     @b
    @@:
        mov     dword[edi],0
        jmp     @f
   .enter_dir:
        add     edi,[ecx]
        push    ecx edi
        mov     byte[edi],'/'
        inc     edi
        mov     esi,file_info
        mov     ecx,3
        cld
        rep     movsd
        mov     dword[edi],0
        pop     esi ecx
        sub     edi,esi
        add     dword[ecx],edi
        mov     [d_rpos],0
        mov     [d_rtop],0
    @@:
        mov     dl,[active_panel]
        call    get_files_data
        call    draw_path
        mov     al,dl
        call    draw_files
        jmp    .exit
   .file:
        mov     eax,[esi+8]
        shl     eax,8
        and     eax,$FFFFFF00
        or      eax,$00000020
        cmp     eax,'    '
        jne    .exit

        mov     esi,edi
        mov     eax,[ecx]
        mov     edi,f_pathn
        call    get_normal_path
        mov     esi,f_pathn
        mov     ecx,[len]
        dec     ecx
        mov     edi,f_info.path
        rep     movsb
        mov     byte[edi],'/'
        inc     edi
        mov     esi,file_info
    @@:
        movsb
        cmp     byte[esi],0
        je      @f
        cmp     byte[esi],' '
        jne     @b
    @@:
        mov     byte[edi],0
        mov     dword[f_info],FS_EXECUTE
        mov     dword[f_info+12],0
        mcall   MF_FSACCESS,f_info
   .exit:
        popad
        ret
endf

;------------------------------------------------------------------------------
;//////////////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

func delete_current_file
        pushad
        popad
        ret
endf

;------------------------------------------------------------------------------
;///// INCLUDES ///////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

include 'mf-copy.inc'
include 'mf-sort.inc'

;------------------------------------------------------------------------------
;///// DATA ///////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

align 4

f_size dd ?
f_day  db ?
f_mnth db ?
f_year dw ?

d_lpos dd 0
d_ltop dd 0
d_lcnt dd 0
d_ltsz dd 0
d_rpos dd 0
d_rtop dd 0
d_rcnt dd 0
d_rtsz dd 0
d_tpos dd ?
d_ttop dd ?
d_tcnt dd ?
d_ttsz dd ?

f_info:
 dd 0
 dd 0
 dd ?
 dd read_area
 dd MEM_FOR_OS
.path:
 rb 200

f_plen0 dd 5
f_plen1 dd 5

sz caption,'MFAR : PRE-ALPHA-8.1'
sz fcfile,'MFAR    DAT'
sz p_hd,'/HD/1',0
sz p_rd,'/RD/1',0
ext0:
  db '     EXE COM BAT CMD'
  dd 0
ext1:
  db ' BMP GIF JPG PNG WMF'
  dd 0
ext2:
  db ' ASM INC'
  dd 0

;----- LANGUAGE-SPECIFIC STRINGS -----
include 'mf-lang.inc'
;-------------------------------------

f_name: rb 12
file_info: db '                              .  .'
active_panel db 0
;---------------------------------

sel_color  = $00008080

FPC        = 18

MEM_FOR_OS = $48300
MEM_USED   = $4C300
read_area  = $48000
fdata_size = $40000-$300
max_cnt    = fdata_size/32
lstart     = $8000+$300
rstart     = lstart+max_cnt*16

f_path0    = $8000
f_path1    = $8000+$100
f_pathn    = $8000+$200

align 4

sc system_colors
fc mfar_colors

I_END:
; 16 bytes per entry:
;  -> 8 bytes - name
;  -> 3 bytes - extension
;  -> 1 byte  - attributes
;  -> 4 bytes - block number
