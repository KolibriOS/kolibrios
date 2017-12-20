;------------------------------------------------------------------------------
;///// COPYING ////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------

__func copy_file
        pushad

        cmp     [active_panel],0
        jne    ._00
        push    f_path0
        push    [f_plen0]
        mov     eax,[d_lpos]
        shl     eax,4
        add     eax,lstart
        push    eax
        mov     esi,f_path1
        mov     eax,[f_plen1]
        jmp     @f
   ._00:
        push    f_path1
        push    [f_plen1]
        mov     eax,[d_rpos]
        shl     eax,4
        add     eax,rstart
        push    eax
        mov     esi,f_path0
        mov     eax,[f_plen0]
    @@:
        mov     ebx,[esp]
        test    byte[ebx+11],FA_FOLDER
        jz      @f
        add     esp,3*4
        stc
        jmp    .close
    @@:
        mov     edi,fc_info.path
        call    get_normal_path
        mov     eax,[len]
        mov     byte[edi+eax-1],'/'
        mov     [fc_len2],eax

        pop     esi
        call    get_file_info
        mov     eax,[f_size]
        mov     [fc_size],eax

        pop     eax
        pop     esi
        mov     edi,f_info.path
        call    get_normal_path
        mov     eax,[len]
        mov     byte[edi+eax-1],'/'
        mov     [fc_len1],eax

        jmp     .redraw_copy

  .redraw:
        call    draw_window_full
  .redraw_copy:
        call    draw_window_copy_main
  .still:
        mcall   MF_WAITEVENT
        jmpe    al,.still,\
          EV_REDRAW,.redraw,\
          EV_KEY,.key,\
          EV_BUTTON,.button

  .key:
        mcall   MF_GETKEY

        jmpe    ah,.still,\
          VK_ESCAPE,._close,\
          VK_RETURN,.copy

  .button:
        jmp    .still

  .copy:
        mov     ecx,[fc_size]
        shr     ecx,9
        inc     ecx
        push    ecx
        shl     ecx,9
        add     ecx,MEM_USED
        mcall   MF_MEMRESIZE,MR_SETSIZE
        or      eax,eax
        jnz    .close

        mov     edi,file_info
        mov     al,' '
        mov     ecx,13
        repne   scasb
        sub     ecx,12
        neg     ecx
        push    ecx
        mov     esi,file_info
        mov     edi,f_info.path
        add     edi,[fc_len1]
        rep     movsb
        mov     byte[edi],0
        pop     ecx
        mov     esi,file_info
        mov     edi,fc_info.path
        add     edi,[fc_len2]
        rep     movsb
        mov     byte[edi],0

        xor     eax,eax                 ; FS_READ
        mov     [f_info],eax
        mov     [f_info+4],eax
        pop     eax
        mov     [f_info+8],eax
        mov     dword[f_info+12],MEM_USED
        mcall   MF_FSACCESS,f_info
        mov     eax,[fc_size]
        mov     [fc_info+8],eax
        mcall   MF_FSACCESS,fc_info

        mcall   MF_MEMRESIZE,MR_SETSIZE,MEM_USED
  ._close:
        clc
  .close:
        popad
        ret
endf

;------------------------------------------------------------------------------

__func draw_window_copy_main
        mcall2  MF_FILLRECT,oX+tW*2,tW*76,oY+tH*7,tH*8,0x00C0C0C0

        mmov    esi,oX+tW*4+2,oX+tW*75+3
        mmov    edi,oY+tH*8+3,oY+tH*13+5
        xor     edx,edx
        call    draw_frame
        mmov    esi,oX+tW*4+4,oX+tW*75+1
        mmov    edi,oY+tH*8+5,oY+tH*13+3
        call    draw_frame

        mcall2  MF_DRAWLINE,oX+tW*4+4,oX+tW*75+1,oY+tH*11+3,oY+tH*11+3,0

        mcall2  MF_FILLRECT,oX+tW*40-(sz_copy.size-2)*tW/2-tW,sz_copy.size*tW,\
                   oY+tH*8+3,3,0x00C0C0C0
        mcall1  MF_DRAWTEXT,oX+tW*40-(sz_copy.size-2)*tW/2,oY+tH*8,\
                   0x00000000,sz_copy,sz_copy.size-2

        mcall1  ,oX+tW*6,oY+tH*9,,,sz_copy.size

        mov     edi,file_info
        mov     al,' '
        mov     ecx,13
        repne   scasb

        madd    ebx,(sz_copy.size)*tW,0
        mov     esi,ecx
        sub     esi,12
        neg     esi
        mcall   MF_DRAWTEXT,,,file_info
        imul    esi,tW
        shl     esi,16
        add     ebx,esi
        mcall   ,,,sz_copyto,sz_copyto.size

        call    draw_copy_input

        ret
endf

;------------------------------------------------------------------------------

__func draw_copy_input
        mcall2  MF_FILLRECT,oX+tW*6,tW*68,oY+tH*10,tH,0x00008080

        mov     edx,fc_info.path
        mov     esi,[fc_len2]
        cmp     esi,68
        jb      @f
        sub     esi,68
        lea     edx,[edx+esi+1]
        mov     esi,67
    @@:
        mcall1  MF_DRAWTEXT,oX+tW*6,oY+tH*10+1,0x00000000
        ret
endf

;------------------------------------------------------------------------------

;fc_pst  db 0
;fc_cur  db 0

align 4

fc_size dd ?
fc_len1 dd 0
fc_len2 dd 0

fc_info:
 dd 1
 dd 0
 dd ?
 dd MEM_USED
 dd MEM_FOR_OS
.path:
 rb 200
