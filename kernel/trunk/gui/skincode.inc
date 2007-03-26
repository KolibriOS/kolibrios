$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "skindata.inc"

;skin_data = 0x00778000

load_skin_file:
; eax = filename
; edx = destination
    mov   ebx,1
    or    ecx,-1
    mov   esi,12
;    call  fileread

    pushad
    push eax
    sub edx,std_application_base_address
    mov [skin_to_load.adr],edx

    ;sub eax,std_application_base_address
    ;mov [skin_to_load.stradr],eax
    mov [skin_to_load.stradr],_skin_file_default - std_application_base_address
    mov eax,skin_to_load - std_application_base_address
    call    file_system_lfn
    ;call  fileread
    pop eax
    popad
    ret


    ret


skin_to_load:
   			dd 0,0,0
   			dd 64*1024
 .adr  		dd 0
   			db 0
 .stradr	dd 0


struct SKIN_HEADER
  .ident   dd ?
  .version dd ?
  .params  dd ?
  .buttons dd ?
  .bitmaps dd ?
ends

struct SKIN_PARAMS
  .skin_height    dd ?
  .margin.right   dw ?
  .margin.left    dw ?
  .margin.bottom  dw ?
  .margin.top     dw ?
  .colors.inner   dd ?
  .colors.outer   dd ?
  .colors.frame   dd ?
  .colors_1.inner dd ?
  .colors_1.outer dd ?
  .colors_1.frame dd ?
  .dtp.size       dd ?
  .dtp.data       db 40 dup (?)
ends

struct SKIN_BUTTONS
  .type     dd ?
  .pos:
    .left   dw ?
    .top    dw ?
  .size:
    .width  dw ?
    .height dw ?
ends

struct SKIN_BITMAPS
  .kind  dw ?
  .type  dw ?
  .data  dd ?
ends

load_skin:
        pushad
        mov     [_skinh],22
        mov     eax,_skin_file
        mov     edx,skin_data
        mov     [edx+SKIN_HEADER.ident],'????'
        call    load_skin_file
        cmp     eax,ERROR_SUCCESS
        je      @f
        cmp     eax,ERROR_END_OF_FILE
        jne     .exit
    @@: call    parse_skin_data
  .exit:
        popad
        ret

parse_skin_data:
        mov     ebp,skin_data
        cmp     [ebp+SKIN_HEADER.ident],'SKIN'
        jne     .exit

        mov     edi,skin_udata
        mov     ecx,(skin_udata.end-skin_udata)/4
        xor     eax,eax
        cld
        rep	stosd

        mov     ebx,[ebp+SKIN_HEADER.params]
        add     ebx,skin_data
        mov     eax,[ebx+SKIN_PARAMS.skin_height]
        mov     [_skinh],eax
        mov     eax,[ebx+SKIN_PARAMS.colors.inner]
        mov     [skin_active.colors.inner],eax
        mov     eax,[ebx+SKIN_PARAMS.colors.outer]
        mov     [skin_active.colors.outer],eax
        mov     eax,[ebx+SKIN_PARAMS.colors.frame]
        mov     [skin_active.colors.frame],eax
        mov     eax,[ebx+SKIN_PARAMS.colors_1.inner]
        mov     [skin_inactive.colors.inner],eax
        mov     eax,[ebx+SKIN_PARAMS.colors_1.outer]
        mov     [skin_inactive.colors.outer],eax
        mov     eax,[ebx+SKIN_PARAMS.colors_1.frame]
        mov     [skin_inactive.colors.frame],eax
        lea     esi,[ebx+SKIN_PARAMS.dtp.data]
        mov     edi,common_colours
        mov     ecx,[ebx+SKIN_PARAMS.dtp.size]
        and     ecx,127
        rep     movsb
        mov     eax,dword[ebx+SKIN_PARAMS.margin.right]
        mov     dword[_skinmargins+0],eax
        mov     eax,dword[ebx+SKIN_PARAMS.margin.bottom]
        mov     dword[_skinmargins+4],eax

        mov     ebx,[ebp+SKIN_HEADER.bitmaps]
        add     ebx,skin_data
  .lp1: cmp     dword[ebx],0
        je      .end_bitmaps
        movzx   eax,[ebx+SKIN_BITMAPS.kind]
        movzx   ecx,[ebx+SKIN_BITMAPS.type]
        dec     eax
        jnz     .not_left
        xor     eax,eax
        mov     edx,skin_active.left.data
        or      ecx,ecx
        jnz     @f
        mov     edx,skin_inactive.left.data
    @@: jmp     .next_bitmap
  .not_left:
        dec     eax
        jnz     .not_oper
        mov     esi,[ebx+SKIN_BITMAPS.data]
        add     esi,skin_data
        mov     eax,[esi+0]
        neg     eax
        mov     edx,skin_active.oper.data
        or      ecx,ecx
        jnz     @f
        mov     edx,skin_inactive.oper.data
    @@: jmp     .next_bitmap
  .not_oper:
        dec     eax
        jnz     .not_base
        mov     eax,[skin_active.left.width]
        mov     edx,skin_active.base.data
        or      ecx,ecx
        jnz     @f
        mov     eax,[skin_inactive.left.width]
        mov     edx,skin_inactive.base.data
    @@: jmp     .next_bitmap
  .not_base:
        add     ebx,8
        jmp     .lp1
  .next_bitmap:
        mov     ecx,[ebx+SKIN_BITMAPS.data]
        add     ecx,skin_data
        mov     [edx+4],eax
        mov     eax,[ecx+0]
        mov     [edx+8],eax
        add     ecx,8
        mov     [edx+0],ecx
        add     ebx,8
        jmp     .lp1
  .end_bitmaps:

        mov     ebx,[ebp+SKIN_HEADER.buttons]
        add     ebx,skin_data
  .lp2: cmp     dword[ebx],0
        je      .end_buttons
        mov     eax,[ebx+SKIN_BUTTONS.type]
        dec     eax
        jnz     .not_close
        mov     edx,skin_btn_close
        jmp     .next_button
  .not_close:
        dec     eax
        jnz     .not_minimize
        mov     edx,skin_btn_minimize
        jmp     .next_button
  .not_minimize:
        add     ebx,12
        jmp     .lp2
  .next_button:
        movsx   eax,[ebx+SKIN_BUTTONS.left]
        mov     [edx+SKIN_BUTTON.left],eax
        movsx   eax,[ebx+SKIN_BUTTONS.top]
        mov     [edx+SKIN_BUTTON.top],eax
        movsx   eax,[ebx+SKIN_BUTTONS.width]
        mov     [edx+SKIN_BUTTON.width],eax
        movsx   eax,[ebx+SKIN_BUTTONS.height]
        mov     [edx+SKIN_BUTTON.height],eax
        add     ebx,12
        jmp     .lp2
  .end_buttons:

  .exit:
        ret

sys_putimage_with_check:
	or	ebx,ebx
	jz	@f
        call    sys_putimage.forced
    @@: ret

drawwindow_IV_caption:

        mov     ebp,skin_active
        or      al,al
        jnz     @f
        mov     ebp,skin_inactive
    @@:

        mov     esi,[esp+4]
        mov     eax,[esi+WDATA.box.width]    ; window width
        mov     edx,[ebp+SKIN_DATA.left.left]
        shl     edx,16
        mov     ecx,[ebp+SKIN_DATA.left.width]
        shl     ecx,16
        add     ecx,[_skinh]

        mov     ebx, [ebp+SKIN_DATA.left.data]
        call    sys_putimage_with_check

        mov     esi,[esp+4]
        mov     eax,[esi+WDATA.box.width]
        sub     eax,[ebp+SKIN_DATA.left.width]
        sub     eax,[ebp+SKIN_DATA.oper.width]
        cmp     eax,[ebp+SKIN_DATA.base.left]
        jng     .non_base
        xor     edx,edx
        mov     ecx,[ebp+SKIN_DATA.base.width]
        jecxz   .non_base
        div     ecx

        inc     eax

        mov     ebx,[ebp+SKIN_DATA.base.data]
        mov     ecx,[ebp+SKIN_DATA.base.width]
        shl     ecx,16
        add     ecx,[_skinh]
        mov     edx,[ebp+SKIN_DATA.base.left]
        sub     edx,[ebp+SKIN_DATA.base.width]
        shl     edx,16
  .baseskinloop:
        shr     edx,16
        add     edx,[ebp+SKIN_DATA.base.width]
        shl     edx,16

        push    eax ebx ecx edx
        call    sys_putimage_with_check
        pop     edx ecx ebx eax

        dec     eax
        jnz     .baseskinloop
  .non_base:

        mov     esi,[esp+4]
        mov     edx,[esi+WDATA.box.width]
        sub     edx,[ebp+SKIN_DATA.oper.width]
        inc     edx
        shl     edx,16
        mov     ebx,[ebp+SKIN_DATA.oper.data]

        mov     ecx,[ebp+SKIN_DATA.oper.width]
        shl     ecx,16
        add     ecx,[_skinh]
        call    sys_putimage_with_check

        ret

;//mike.dld, 2006-08-02 ]


drawwindow_IV:
;param1 - aw_yes

        pusha

        push  edx

        mov   edi,edx

        mov     ebp,skin_active
        cmp     byte [esp+32+4+4],0
        jne     @f
        mov     ebp,skin_inactive
     @@:

        mov   eax,[edi+WDATA.box.left]
        shl   eax,16
		mov   ax,word [edi+WDATA.box.left]
		add   ax,word [edi+WDATA.box.width]
        mov   ebx,[edi+WDATA.box.top]
        shl   ebx,16
		mov   bx,word [edi+WDATA.box.top]
		add   bx,word [edi+WDATA.box.height]
;        mov   esi,[edi+24]
;        shr   esi,1
;        and   esi,0x007f7f7f
        mov   esi,[ebp+SKIN_DATA.colors.outer]
        or      [edi+WDATA.fl_wdrawn], 4
        call  draw_rectangle
        mov   ecx,3
      _dw3l:
        add   eax,1*65536-1
        add   ebx,1*65536-1
        test  ax,ax
        js    no_skin_add_button
        test  bx,bx
        js    no_skin_add_button
        mov   esi,[ebp+SKIN_DATA.colors.frame] ;[edi+24]
        call  draw_rectangle
        dec   ecx
        jnz   _dw3l
        mov   esi,[ebp+SKIN_DATA.colors.inner]
        add   eax,1*65536-1
        add   ebx,1*65536-1
        test  ax,ax
        js    no_skin_add_button
        test  bx,bx
        js    no_skin_add_button
        call  draw_rectangle

        cmp   dword[skin_data],'SKIN'
        je    @f
        xor   eax,eax
        xor   ebx,ebx
        mov   esi,[esp]
        mov   ecx,[esi+WDATA.box.width]
        inc   ecx
        mov   edx,[_skinh]
        mov   edi,[common_colours+4] ; standard grab color
        call  [drawbar]
        jmp   draw_clientbar
    @@:

        mov     al,[esp+32+4+4]
        call    drawwindow_IV_caption

    draw_clientbar:

        mov   esi,[esp]

        mov   edx,[esi+WDATA.box.top]                       ; WORK AREA
        add   edx,21+5
        mov   ebx,[esi+WDATA.box.top]
        add   ebx,[esi+WDATA.box.height]
        cmp   edx,ebx
        jg    _noinside2
        mov   eax,5
        mov   ebx,[_skinh]
        mov   ecx,[esi+WDATA.box.width]
        mov   edx,[esi+WDATA.box.height]
        sub   ecx,4
        sub   edx,4
        mov   edi,[esi+WDATA.cl_workarea]
	test  edi,0x40000000
	jnz   _noinside2
        call  [drawbar]
      _noinside2:

        cmp   dword[skin_data],'SKIN'
        jne   no_skin_add_button

;* close button
        mov   edi,[BTN_ADDR]
        movzx eax,word [edi]
        cmp   eax,1000
        jge   no_skin_add_button
        inc   eax
        mov   [edi],ax

        shl   eax,4
        add   eax,edi

        mov   bx,[CURRENT_TASK]
        mov   [eax],bx

        add   eax,2         ; save button id number
        mov   bx,1
        mov   [eax],bx
        add   eax,2         ; x start
        xor   ebx,ebx
        cmp   [skin_btn_close.left],0
        jge   _bCx_at_right
        mov   ebx,[esp]
        mov   ebx,[ebx+WDATA.box.width]
        inc   ebx
      _bCx_at_right:
        add   ebx,[skin_btn_close.left]
        mov   [eax],bx
        add   eax,2         ; x size
        mov   ebx,[skin_btn_close.width]
        dec   ebx
        mov   [eax],bx
        add   eax,2         ; y start
        mov   ebx,[skin_btn_close.top]
        mov   [eax],bx
        add   eax,2         ; y size
        mov   ebx,[skin_btn_close.height]
        dec   ebx
        mov   [eax],bx

;* minimize button
        mov   edi,[BTN_ADDR]
        movzx eax,word [edi]
        cmp   eax,1000
        jge   no_skin_add_button
        inc   eax
        mov   [edi],ax

        shl   eax,4
        add   eax,edi

        mov   bx,[CURRENT_TASK]
        mov   [eax],bx

        add   eax,2         ; save button id number
        mov   bx,65535 ;999
        mov   [eax],bx
        add   eax,2         ; x start
        xor   ebx,ebx
        cmp   [skin_btn_minimize.left],0
        jge   _bMx_at_right
        mov   ebx,[esp]
        mov   ebx,[ebx+WDATA.box.width]
        inc   ebx
      _bMx_at_right:
        add   ebx,[skin_btn_minimize.left]
        mov   [eax],bx
        add   eax,2         ; x size
        mov   ebx,[skin_btn_minimize.width]
        dec   ebx
        mov   [eax],bx
        add   eax,2         ; y start
        mov   ebx,[skin_btn_minimize.top]
        mov   [eax],bx
        add   eax,2         ; y size
        mov   ebx,[skin_btn_minimize.height]
        dec   ebx
        mov   [eax],bx

      no_skin_add_button:
        pop     edi
        and     [edi+WDATA.fl_wdrawn], not 4
        test    [edi+WDATA.fl_wdrawn], 2
        jz      @f
        call    drawwindowframes2
@@:

        popa

        ret  4

