idbChange       equ 2
idbCreate       equ 3
idbLeft         equ 4
idbRight        equ 5
idbCancel       equ 1

DlgAdd:
DlgProp:

        mov     eax,[SelIcon]
        mov     [DlgSelIcon],eax
        cmp     eax,-1
        jne     SetProp

        m2m     dword[AddX],dword[MouseX]
        m2m     dword[AddY],dword[MouseY]

        mov     ecx,NAME_LENGTH
        mov     edi,DAreaName
        xor     al,al
        rep     stosb
        mov     dword[edtName.size],0
        mov     dword[edtName.pos],0

        mov     ecx,256
        mov     edi,DAreaPath
        rep     stosb
        mov     dword[edtExePath.size],0
        mov     dword[edtExePath.pos],0

        mov     ecx,256
        mov     edi,DAreaParams
        rep     stosb
        mov     dword[edtParams.size],0
        mov     dword[edtParams.pos],0

        mov     ecx,256
        mov     edi,DAreaIcon
        rep     stosb
        mov     dword[edtIcon.size],0
        mov     dword[edtIcon.pos],0

        jmp     startDlg
    ;--------
SetProp:
        mov     esi,[IconsOffs+eax*4]
        mov     edi,DAreaName
        xor     ecx,ecx
     @@:
        lodsb
        stosb
        inc     ecx
        test    al,al
        jnz     @b
        dec     ecx
        mov     dword[edtName.size],ecx
        mov     dword[edtName.pos],ecx

        xor     ecx,ecx
        mov     edi,DAreaPath
     @@:
        lodsb
        stosb
        inc     ecx
        test    al,al
        jnz     @b
        dec     ecx
        mov     dword[edtExePath.size],ecx
        mov     dword[edtExePath.pos],ecx

        xor     ecx,ecx
        mov     edi,DAreaParams
     @@:
        lodsb
        stosb
        inc     ecx
        test    al,al
        jnz     @b
        dec     ecx
        mov     dword[edtParams.size],ecx
        mov     dword[edtParams.pos],ecx

        xor     ecx,ecx
        mov     edi,DAreaIcon
     @@:
        lodsb
        stosb
        inc     ecx
        test    al,al
        jnz     @b
        dec     ecx
        mov     dword[edtIcon.size],ecx
        mov     dword[edtIcon.pos],ecx


startDlg:
        mcall   9,RBProcInfo,-1
        mcall   18,21,dword[RBProcInfo+30]
        mov     [slotDlgAdd],eax

        or      [edtName.flags],ed_focus

        mcall   40,100111b      ;SetMaskMessage 100111b
        xor     eax,eax
        mov     dword[MaxPage],0
      @@:
        add     eax,ICONS_DRAW_COUNTW
        inc     dword[MaxPage]
        cmp     eax,[icon_count]
        jb      @b
        dec     dword[MaxPage]
        cmp     dword[MaxPage],0
        jne     @f
        mov     dword[MaxPage],1
      @@:

DReDraw:
        mcall   48,3,sc,40

        mov     eax,[sc.work]
        rol     eax,16
        add     al,9
        jnc     @f
        mov     al,0FFh
      @@:
        rol     eax,16
        add     al,9
        jnc     @f
        mov     al,0FFh
      @@:
        add     ah,9
        jnc     @f
        mov     ah,0FFh
      @@:

        mov     [sbIcons.bckg_col],eax
        m2m     [sbIcons.frnt_col],[sc.work]
        m2m     [sbIcons.line_col],[sc.work_graph]

        edit_boxes_set_sys_color edtName,edtExePath,sc
        edit_boxes_set_sys_color edtExePath,edtParams,sc
        edit_boxes_set_sys_color edtParams,edtIcon,sc
        edit_boxes_set_sys_color edtIcon,endEdits,sc

        call    DRedraw

DMessages:
        mcall   10              ;WaitMessage

        dec     eax
        jz      DReDraw
        dec     eax
        jz      DKey
        dec     eax
        jz      DButton
        sub     eax,3
        jz      DMouse

        jmp  DMessages


;---------------------------------------------------------------------
DKey:
        mcall   2               ;GetKeyPressed
        cmp     ah,1Bh
        je      DExit
        cmp     ah,9  ;Tab
        je      DNextEdit
        stdcall [edit_box_key],edtName
        stdcall [edit_box_key],edtExePath
        stdcall [edit_box_key],edtParams
        stdcall [edit_box_key],edtIcon

        jmp     DMessages


DNextEdit:
        test    [edtName.flags],ed_focus
        jne      .DNE1
        test    [edtExePath.flags],ed_focus
        jne      .DNE2
        test    [edtParams.flags],ed_focus
        jne      .DNE3
        test    [edtIcon.flags],ed_focus
        jne      .DNE4
        jmp     DMessages

    .DNE1:
        and     [edtName.flags],not ed_focus
        or      [edtExePath.flags],ed_focus
        stdcall [edit_box_draw],edtName
        stdcall [edit_box_draw],edtExePath
        jmp     DMessages

    .DNE2:
        and     [edtExePath.flags],not ed_focus
        or      [edtParams.flags],ed_focus
        stdcall [edit_box_draw],edtExePath
        stdcall [edit_box_draw],edtParams
        jmp     DMessages

    .DNE3:
        and     [edtParams.flags],not ed_focus
        or      [edtIcon.flags],ed_focus
        stdcall [edit_box_draw],edtParams
        stdcall [edit_box_draw],edtIcon
        jmp     DMessages

    .DNE4:
        and     [edtIcon.flags],not ed_focus
        or      [edtName.flags],ed_focus
        stdcall [edit_box_draw],edtName
        stdcall [edit_box_draw],edtIcon
        jmp     DMessages
;---------------------------------------------------------------------
DButton:
        mcall   17              ;GetButtonPressed
        cmp     ah, idbCancel
        je      DExit
        cmp     ah,idbChange
        je      DSetExePath
        cmp     ah,idbCreate
        je      DSaveIcon

        jmp     DMessages
;-------------------------------------------------------------------------------
DMouse:
        stdcall [edit_box_mouse],edtName
        stdcall [edit_box_mouse],edtExePath
        stdcall [edit_box_mouse],edtParams
        stdcall [edit_box_mouse],edtIcon

        push    [sbIcons.position]
        stdcall [scrollbar_h_mouse],sbIcons
        pop     eax
        cmp     eax,[sbIcons.position]
        je      @f
        call    DrawStdIcons
      @@:

        mcall   37,2            ;GetMouseKey
        test    eax,1
        jz      DMessages
        mcall   37,1            ;GetMouseWinPos

        test    eax,10001000h
        jne     DMessages
        xor     edx,edx
        mov     dx,ax     ;y
        shr     eax,16    ;x

        sub     eax,ICONSX
        js      DMessages
        sub     edx,ICONSY
        js      DMessages

        cmp     eax,(IMG_SIZE+SPCW)*ICONS_DRAW_COUNTW-SPCW
        ja      DMessages
        cmp     edx,(IMG_SIZE+SPCH)*ICONS_DRAW_COUNTH-SPCH
        ja      DMessages

        xor     ebx,ebx
        mov     ecx,[sbIcons.position]
        test    ecx,ecx
        jz      .DM
     @@:
        add     ebx,ICONS_DRAW_COUNTH
        loop    @b
     .DM:

        sub     eax,IMG_SIZE+SPCW
        js     .DM1
     @@:
        add     ebx,ICONS_DRAW_COUNTH
        sub     eax,IMG_SIZE+SPCW
        jns     @b
     .DM1:


        sub     edx,IMG_SIZE+SPCH
        js     .DM2
     @@:
        inc     ebx
        sub     edx,IMG_SIZE+SPCH
        jns     @b
     .DM2:

        mov     eax,ebx

        cmp     eax,[icon_count]
        jae     DMessages

        mov     bl,10
        div     bl

        add     ah,30h
        test    al,al
        jz      @f

        add     al,30h
        mov     byte[DAreaIcon],al
        mov     byte[DAreaIcon+1],ah
        mov     byte[DAreaIcon+2],0
        mov     dword[edtIcon.size],2
        jmp     .RedrawNum

      @@:
        mov     byte[DAreaIcon],ah
        mov     byte[DAreaIcon+1],0
        mov     dword[edtIcon.size],1
        jmp     .RedrawNum
    .RedrawNum:


        stdcall [edit_box_draw],edtIcon


        call    DrawSelIcon
        jmp     DReDraw;DMessages;
;-------------------------------------------------------------------------------
DSetExePath:
        stdcall [OpenDialog_Start],OpenDialog_data
        mov     edi,DAreaPath
        xor     al,al
        or      ecx,-1
        repne   scasb
        sub     edi,DAreaPath
        dec     edi
        mov     dword[edtExePath+12*4],edi

        jmp     DMessages;DReDraw;

DSaveIcon:
        mov     dword[fiRunProg+8],WarningSave
        mov     dword[fiRunProg+21],pthNotify
        mcall   70,fiRunProg


        cmp     [DlgSelIcon],-1
        je      @f
                       ;удаляем из ini старую иконку
        mov     ebx,[DlgSelIcon]
        mov     eax,[IconsOffs+ebx*4]
        stdcall [ini_del_section],IconIni,eax

        stdcall EditIcon,[DlgSelIcon],DAreaIcon,DAreaName,DAreaPath,DAreaParams
        jmp     DExitAndSave
     @@:
        stdcall AddIcon,[AddX],[AddY],DAreaIcon,DAreaName,DAreaPath,DAreaParams
        ;jmp     DExitAndSave
;-------------------------------------------------------------------------------
DExitAndSave:

         stdcall [ini_set_str],IconIni,DAreaName,keyPath,DAreaPath,dword[edtExePath.size]
         stdcall [ini_set_str],IconIni,DAreaName,keyParams,DAreaParams,dword[edtParams.size]
         stdcall [ini_set_str],IconIni,DAreaName,keyIco,DAreaIcon,dword[edtIcon.size]
         stdcall [ini_set_int],IconIni,DAreaName,keyX,[MouseX]
         stdcall [ini_set_int],IconIni,DAreaName,keyY,[MouseY]

DExit:
        mov     [DlgAddActiv],0
        mov     [slotDlgAdd],0
        mcall   -1              ;ExitThread

;##################################################################
proc DRedraw
        mcall   12,1            ;RedrawWinBeg
        mov     edx,[sc.work]
        or      edx,33000000h
        mcall   0,<100,400>,<100,(IMG_SIZE+SPCH)*ICONS_DRAW_COUNTH+185>,,,DTitle

        stdcall [edit_box_draw],edtName
        stdcall [edit_box_draw],edtExePath
        stdcall [edit_box_draw],edtParams
        stdcall [edit_box_draw],edtIcon

        call    DrawStdIcons


        stdcall [scrollbar_h_draw],sbIcons
        mcall   38,<ICONSX+1,END_ICONS_AREAW-2>,\
                   <END_ICONS_AREAH+3,END_ICONS_AREAH+3>,\
                [sc.work_graph]
        mcall     ,<ICONSX+1,END_ICONS_AREAW-2>,\
                   <END_ICONS_AREAH+3+15,END_ICONS_AREAH+3+15>,\
                [sc.work_graph]

        mcall     ,<ICONSX,ICONSX>,\
                   <END_ICONS_AREAH+3+1,END_ICONS_AREAH+3+15-1>,\
                [sc.work_graph]
        mcall     ,<END_ICONS_AREAW-1,END_ICONS_AREAW-1>,\
                   <END_ICONS_AREAH+3+1,END_ICONS_AREAH+3+15-1>,\
                [sc.work_graph]

                                  ;BUTTONS
        mcall   8,<250,49>,<30+END_ICONS_AREAH,16>,idbCreate,[sc.work_button]
        mcall    ,<305,54>,                       ,idbCancel
        mcall    ,<351,14>,<26,14>             ,idbChange

                                  ;CAPTIONS
        mov     ecx,[sc.work_text]
        or      ecx,80000000h
        mcall   4,<30+IMG_SIZE,10>,,DCaptName
        mcall    ,<30+IMG_SIZE,30>,,DCaptPath
        mcall    ,<30+IMG_SIZE,50>,,DCaptParams
        mcall    ,<30+IMG_SIZE,70>,,DCaptIcon

if lang eq ru
        mov     ecx,[sc.work_button_text]
        or      ecx,80000000h
        mcall    ,<309,35+END_ICONS_AREAH>,,DCaptCancel

        cmp     [DlgSelIcon],-1
        jne     @f
        mpack   ebx,255,35+END_ICONS_AREAH
        mov     edx,DCaptCreate
        jmp     .DRD1
      @@:
        mpack   ebx,252,35+END_ICONS_AREAH
        mov     edx,DCaptProperties
     .DRD1:
else
        mov     ecx,[sc.work_button_text]
        or      ecx,80000000h
        mcall    ,<315,35+END_ICONS_AREAH>,,DCaptCancel


        cmp     [DlgSelIcon],-1
        jne     @f
        mpack   ebx,257,35+END_ICONS_AREAH
        mov     edx,DCaptCreate
        jmp     .DRD1
      @@:
        mpack   ebx,257,35+END_ICONS_AREAH
        mov     edx,DCaptProperties
     .DRD1:
end if
        mcall

        mcall   1,351+4  ,26+10,[sc.work_button_text]
        mcall    ,351+4+3,26+10,
        mcall    ,351+4+6,26+10,

        call DrawSelIcon

        mcall   12,2            ;RedrawWinEnd
        ret
endp


proc DrawStdIcons
local IcoX:DWORD,\
      IcoY:DWORD,\
      iIcon:DWORD

        mov     ecx,[sbIcons.position]
        xor     eax,eax
        m2m     dword[iIcon],0

        test    ecx,ecx
        jz      .DST1
     @@:
        add     [iIcon],ICONS_DRAW_COUNTH
        add     eax,IMG_SIZE*IMG_SIZE*4*ICONS_DRAW_COUNTH
        loop    @b
   .DST1:

        lea     esi,[eax+44]
        add     esi,[raw_pointer]

        mov     [IcoX],ICONSX


        mov     ecx,ICONS_DRAW_COUNTW
  .DrawIcons:
        push    ecx

        mov     [IcoY],ICONSY

        mov     ecx,ICONS_DRAW_COUNTH
  .DrawIcoStolb:
        push    ecx

        mov     edi,IconArea
        mov     edx,IMG_SIZE

    .DrawLine:


        mov     ecx,IMG_SIZE
     .DrawPix:

        lodsd
        test    eax,0FF000000h
        jnz     @f
        mov     eax,[sc.work]
     @@:

        mov     word[edi],ax
        shr     eax,16
        mov     [edi+2],al
        add     edi,3

        loop    .DrawPix

        dec     edx
        jnz    .DrawLine

        mov     edx,[IcoX]
        shl     edx,16
        mov     dx,word[IcoY]
        mcall   7,IconArea,<IMG_SIZE,IMG_SIZE>

        inc     [iIcon]
        mov     eax,[iIcon]
        cmp     eax,[icon_count]
        jae     .endDrawIcon

        add     dword[IcoY],IMG_SIZE+SPCH
        pop     ecx
        loop    .DrawIcoStolb

        add     dword[IcoX],IMG_SIZE+SPCW
        pop     ecx
        loop    .DrawIcons
        jmp     .endProc

    .endDrawIcon:

        mov     ecx,IMG_SIZE*IMG_SIZE
        mov     edi,IconArea
        mov     eax,[sc.work]
        mov     ebx,eax
        shr     ebx,16
     @@:
        mov     word[edi],ax
        mov     [edi+2],bl
        add     edi,3
        loop    @b

                                     ;затирание не использованых мест
        jmp     .Dalee

  .DrawIcons2:
        push    ecx

        mov     [IcoY],ICONSY
        mov     ecx,ICONS_DRAW_COUNTH
  .DrawIcoStolb2:
        push    ecx

        mov     edx,[IcoX]
        shl     edx,16
        mov     dx,word[IcoY]
        mcall   7,IconArea,<IMG_SIZE,IMG_SIZE>

     .Dalee:
        add     dword[IcoY],IMG_SIZE+SPCH
        pop     ecx
        loop    .DrawIcoStolb2

        add     dword[IcoX],IMG_SIZE+SPCW
        pop     ecx
        loop    .DrawIcons2

   .endProc:
        ret
endp

proc DrawSelIcon
        mov     edi,DAreaIcon
        cmp     byte[edi],0
        jne     @f
        ret
      @@:

        mov     al,[edi]
        cmp     al,'9'
        ja      .PathToIcon
        cmp     al,'/'
        jne     .GetIconInd
   .PathToIcon:
        mov     al,30h           ;заглушка!!!!!!!!!!!!!
        mov     byte[edi+1],0
   .GetIconInd:
;int3
        sub     al,30h
        cmp     byte[edi+1],0
        je      @f
        shl     eax,1
        lea     eax,[eax*4+eax]
        xor     edx,edx
        mov     dl,[edi+1]
        sub     dl,30h
        add     eax,edx
     @@:             ;eax=num icon
        cmp     eax,[icon_count]
        jb      @f
        xor     eax,eax
     @@:
        test    eax,eax
        je      .DI1
        mov     ecx,eax
        xor     eax,eax
      @@:
        add     eax,IMG_SIZE*IMG_SIZE*4
        loop    @b
     .DI1:
        mov     esi,eax
        add     esi,[raw_pointer]
        add     esi,44

        mov     edi,IconArea
        mov     edx,IMG_SIZE
    .DrawLine:

        mov     ecx,IMG_SIZE
     .DrawPix:

        lodsd
        test    eax,0FF000000h
        jnz     @f
        mov     eax,[sc.work]
     @@:

        mov     word[edi],ax
        shr     eax,16
        mov     [edi+2],al
        add     edi,3

        loop    .DrawPix

        dec     edx
        jnz     .DrawLine

        mcall   7,IconArea,<IMG_SIZE,IMG_SIZE>,<15,25>

        ret
endp