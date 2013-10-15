if lang eq ru
MinRMenuW equ 18*6+10
else
MinRMenuW equ 15*6+10
end if

idbRRedrawFon equ 100
idbRAlign    equ 101
idbRFixIcons equ 102
idbRAdd      equ 103
idbRDel      equ 104
idbRProp     equ 105

DELTA_COLOR  equ 70

;---- thread for rbutton -----------------------------------------------------------------------
RButtonWin:
        mcall   40,100101b      ;SetMaskMessage 100101b
        mcall   48,3,sc,40

        mov     eax,[sc.work]     ;вычисляем цвет светлее фонового
        rol     eax,16
        add     al,DELTA_COLOR
        jnc     @f
        mov     al,0FFh
      @@:
        rol     eax,16
        add     al,DELTA_COLOR
        jnc     @f
        mov     al,0FFh
      @@:
        add     ah,DELTA_COLOR
        jnc     @f
        mov     ah,0FFh
      @@:
        mov     [sc.workH],eax

        mov     eax,[sc.work]     ;вычисляем цвет темнее фонового
        rol     eax,16
        sub     al,DELTA_COLOR
        jnc     @f
        mov     al,0FFh
      @@:
        rol     eax,16
        sub     al,DELTA_COLOR
        jnc     @f
        mov     al,0FFh
      @@:
        sub     ah,DELTA_COLOR
        jnc     @f
        mov     ah,0FFh
      @@:
        mov     [sc.workL],eax



        stdcall GetNumIcon,[MouseX],[MouseY],-1
        mov     [SelIcon],eax

        mov     [NumUserButt],0
        mov     [RMenuW],0
        mcall   70,fiIni
        mcall   68,12,dword[bufIni+32]
        mov     [RBUser],eax
        stdcall [ini_enum_keys],IconIni,secRButt,GetUserBut

        cmp     [SelIcon],-1
        jne     @f
        mov     [RMenuH],16*4
        mov     eax,[NumUserButt]
        shl     eax,4
        add     [RMenuH],ax
        jmp     .rbw1
      @@:
        mov     [RMenuH],16*5
    .rbw1:

RBWRedraw:
        mcall   12,1    ;RedrawWinBeg
        mov     ebx,[MouseX]
        shl     ebx,16
        mov     bx,[RMenuW]
        mov     ecx,[MouseY]
        shl     ecx,16
        mov     cx,[RMenuH]
        mov     edx,01000000h
        mov     esi,01000000h
        xor     eax,eax
        int     40h

        and     ebx,0FFFFh      ;фоновый прямоугольник
        inc     bx
        and     ecx,0FFFFh
        inc     cx
        mcall   13,,,[sc.work]

        cmp     [SelIcon],-1       ;если клик по иконке, то пункты пользователя не нужны
        jne     .NoDrawUserBut
       ;-----------
        mov     ecx,[NumUserButt]
  .DrawUB:
        push    ecx

        mov     eax,[NumUserButt]
        sub     eax,ecx
        mov     edx,eax      ;button ID
        or      edx,60000000h
        shl     eax,4+16          ;*16*10000h
        mov     ecx,eax
        mov     cx,16

        xor     ebx,ebx
        mov     bx,[RMenuW]
        mov     esi,[sc.work]
        mcall   8                       ;определили

        mov     ebx,50000h
        shr     ecx,16
        mov     bx,cx
        add     bx,5

        mov     ecx,80000000h

        mov     eax,[NumUserButt]
        sub     eax,[esp]
        mov     edx,[RBUser+eax*8]
        mcall   4                      ;подписали

        pop     ecx
        loop    .DrawUB

       ;-----------
.NoDrawUserBut:
                                        ;определение кнопок для иконок
        xor     ebx,ebx
        mov     bx,[RMenuW]

        xor     eax,eax
        cmp     [SelIcon],-1
        jne     @f
        mov     eax,[NumUserButt]
     @@:
        shl     eax,4
        mov     cx,ax
        shl     ecx,16
        mov     cx,16
        push    ecx                     ;отсюда рисовать кнопки (по высоте)


        mcall   8,,,60000000h or idbRRedrawFon,[sc.work]
        add     ecx,16*10000h
        mcall    ,,,60000000h or idbRAlign
        add     ecx,16*10000h
        mcall    ,,,60000000h or idbRFixIcons
        add     ecx,16*10000h

        cmp     dword[SelIcon],-1
        je      @f

        mcall    ,,,60000000h or idbRDel,
        add     ecx,16*10000h
        mcall    ,,,60000000h or idbRProp,
        jmp     .endRM1
      @@:
        mcall    ,,,60000000h or idbRAdd,
      .endRM1:

                                        ;подписи к кнопкам
        xor     eax,eax
        cmp     [SelIcon],-1
        jne     @f
        mov     eax,[NumUserButt]
      @@:
        shl     eax,4
        mov     ebx,50000h
        mov     bx,ax

        add     bx,5
        mcall   4,,80000000h,RMenuRedrawFon
        add     ebx,16
        mcall    ,,,RMenuAlign

        add     ebx,16
        cmp     [bFixIcons],0
        jne     @f
        mov     edx,RMenuOffMoving
        jmp     .RBWR1
     @@:
        mov     edx,RMenuOnMoving
     .RBWR1:
        mcall
        add     ebx,16

        cmp     dword[SelIcon],-1
        je      @f

        mcall    ,,,RMenuDel
        add     bx,16
        mcall    ,,,RMenuProp
        jmp     .endRM2
      @@:

        mcall    ,,,RMenuAdd
      .endRM2:

        pop     ecx
        mcall   12,2    ;RedrawWinEnd

        mcall   9,RBProcInfo,-1         ;GetThreadInfo RBProcInfo,-1
        mcall   18,21,dword[RBProcInfo+30]
        m2m     dword[RBSlot],eax

RBWMessages:
        mcall   10              ;WaitMessage
        dec     eax
        jz      RBWRedraw
        sub     eax,2
        jz      RBWButton
        sub     eax,3
        jz      RBWMouse
        jmp     RBWMessages

RBWButton:
        mcall   17              ;GetButtonPressed
        shr     eax,8
        cmp     eax,idbRRedrawFon
        jz      RBRedrawFon
        cmp     eax,idbRAlign
        jz      RBAlign
        cmp     eax,idbRFixIcons
        jz      RBFixIcons
        cmp     eax,idbRAdd
        jz      RBAdd
        cmp     eax,idbRDel
        jz      RBDelete
        cmp     eax,idbRProp
        jz      RBPropeties

        cmp     eax,[NumUserButt]
        ja      @f

        mov     eax,[RBUser+eax*8+4]
        mov     dword[fiRunProg+8],0
        mov     dword[fiRunProg+21],eax
        mcall   70,fiRunProg
    @@:
        jmp     RBWExit

RBWMouse:
        cmp     [PredItem],-1
        je      @f
        mov     eax,[PredItem]
        xor     ebx,ebx
        mov     bx,[RMenuW]
        mov     cx,ax
        shl     ecx,16
        mov     cx,ax
        mcall   38,,,[sc.work]
        add     ecx,00100010h
        mcall    ,,,[sc.work]
      @@:
        call    ClickInWin
        jnc     @f
        mcall   37,1          ;положение мыши относительно окна
        and     eax,0FFF0h
        mov     [PredItem],eax
        xor     ebx,ebx
        mov     bx,[RMenuW]
        mov     cx,ax
        shl     ecx,16
        mov     cx,ax
        mcall   38,,,[sc.workL]
        add     ecx,00100010h
        mcall    ,,,[sc.workH]
      @@:


        mcall   37,2            ;GetMouseKey
        test    eax,111b
        jz      RBWMessages

        call    ClickInWin
        jc      RBWMessages

RBWExit:
        mcall   68,13,[RBUser]
        mov     [RButtonActiv],0
        mcall   -1      ;ExitProcess


RBRedrawFon:
        mcall   15,3
        jmp     RBWExit

RBAlign:
        call    AlignIcons

        m2m     [PIcoDB],[BegData]
        stdcall [ini_enum_sections],IconIni,Ini_SavePos

        mov     dword[fiRunProg+8],WarningSave
        mov     dword[fiRunProg+21],pthNotify
        mcall   70,fiRunProg
        mov     [bNotSave],0

        jmp     RBRedrawFon

RBFixIcons:
        xor     dword[bFixIcons],1

        cmp     [bNotSave],0
        je      @f

        mov     dword[fiRunProg+8],WarningSave
        mov     dword[fiRunProg+21],pthNotify
        mcall   70,fiRunProg
        mov     [bNotSave],0

      @@:
        jmp     RBWExit

RBAdd:
        cmp     [DlgAddActiv],1
        je      RBWSetActivWin
        mov     [DlgAddActiv],1

        mcall   51,1,DlgAdd,stack_dlg           ;CreateThread DlgAdd,stack_dlg
        jmp     RBWExit

RBDelete:

        mov     eax,[SelIcon]
        mov     eax,[IconsOffs+eax*4]
        stdcall [ini_del_section],IconIni,eax

        stdcall DelIcon,[SelIcon]
    ;    mov     dword[fiStdIco],2
    ;    m2m     dword[fInfo+12],dword[SizeData]
    ;    mcall   70,fInfo

        mov     dword[fiRunProg+8],WarningSave
        mov     dword[fiRunProg+21],pthNotify
        mcall   70,fiRunProg
        mov     [bNotSave],0

        jmp     RBRedrawFon

RBPropeties:
        cmp     [DlgAddActiv],1
        je      RBWSetActivWin
        mov     [DlgAddActiv],1

        mcall   51,1,DlgProp,stack_dlg          ;CreateThread DlgProp,stack_dlg
        jmp     RBWExit

RBWSetActivWin:
        mcall   18,3,[slotDlgAdd]
        jmp     RBWExit

proc ClickInWin        ;Carry flag = 1, если клик в окне
        mcall   37,0            ;GetMousePos
        xor     ebx,ebx
        mov     bx,ax           ;y
        shr     eax,16          ;x

        xor     ecx,ecx
        mov     cx,bx
        mov     bx,ax

        mcall   34
        cmp     eax,[RBSlot]
        jne     @f
        stc
        ret
      @@:
        clc
        ret
endp


proc AlignIcons
local posX:WORD,\
      posY:WORD,\
      negatX:WORD,\
      negatY:WORD

        xor     ebx,ebx

        mov     ecx,[MaxNumIcon]
        test    ecx,ecx
        jnz     .AlignIco
        ret
 .AlignIco:
        push    ecx
        or      ecx,-1
        mov     edi,dword[IconsOffs+ebx]
        xor     eax,eax
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb

        mov     dword[negatX],0

     ;-------- преобразуем координаты в положительные
        mov     ax,[edi+2]
        test    ax,8000h
        jz      @f
        neg     ax
        mov     word[negatX],1
      @@:
        mov     [posX],ax

        mov     ax,[edi]
        test    ax,8000h
        jz      @f
        neg     ax
        mov     word[negatY],1
      @@:
        mov     [posY],ax

      ;-----
        mov     ax,[posY]
        mov     dx,ALIGN_SIZE

        cmp     ax,dx
        jb      .AI0
    @@: sub     ax,dx
        cmp     ax,dx
        jae     @b
    .AI0:

        mov     dx,ax
        cmp     dx,ALIGN_SIZE/2
        jb      .AI1

        mov     ax,ALIGN_SIZE
        sub     ax,dx
        add     [posY],ax

        jmp     .AI2
     .AI1:
        sub     [posY],dx
     .AI2:
     ;-----
        mov     ax,[posX]
        mov     dx,ALIGN_SIZE

        cmp     ax,dx
        jb      .AI3
    @@: sub     ax,dx
        cmp     ax,dx
        jae     @b
    .AI3:

        mov     dx,ax
        cmp     dx,ALIGN_SIZE/2
        jb      .AI4

        mov     ax,ALIGN_SIZE
        sub     ax,dx
        add     [posX],ax

        jmp     .AI5
     .AI4:
        sub     [posX],dx
     .AI5:

      ;----- преобразование назад и поправка в отрицательных координат
        mov     ax,[posX]

        cmp     [negatX],1
        jne     @f
        neg     ax
        inc     ax
        jmp     .AI6
      @@:

        mov     dx,[ScreenX]
        shr     dx,1
        cmp     ax,dx
        jbe     .AI6
        sub     ax,[ScreenX]
      .AI6:
        mov     word[edi+2],ax


        mov     ax,[posY]
        cmp     [negatY],1
        jne     @f
        neg     ax
        inc     ax
        jmp     .AI7
      @@:

        mov     dx,[ScreenY]
        shr     dx,1
        cmp     ax,dx
        jbe     .AI7
        sub     ax,[ScreenY]
      .AI7:
         mov     word[edi],ax

       ;--------



        pop     ecx
        add     ebx,4
        dec     ecx
        jnz     .AlignIco
;        loop    .AlignIco


;        mov     dword[fInfo],2
;        mcall   70,fInfo
        ret
endp

;-------------------------------------------------------------------------------
proc GetUserBut stdcall,f_name,sec_name,key_name,key_value
        cmp     [NumUserButt],16
        jne     @f
        ret
     @@:

        push    ebx esi edi

        mov     edi,[key_name]           ;определяем ширину меню
        xor     eax,eax
        or      ecx,-1
        repne   scasb
        sub     edi,[key_name]
        mov     eax,edi
        shl     eax,1
        lea     eax,[eax*2+eax+5*2]
        cmp     ax,[RMenuW]
        jbe     @f
        mov     [RMenuW],ax
     @@:
        cmp     [RMenuW],MinRMenuW
        jae     @f
        mov     [RMenuW],MinRMenuW
     @@:

        mov     ebx,[NumUserButt]

        mov     esi,[key_name]
        mov     edi,[RBUser+ebx*8]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

;debug_newline


        mov     [RBUser+ebx*8+4],edi
        mov     esi,[key_value]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

        mov     [RBUser+ebx*8+8],edi

        inc     [NumUserButt]

        pop     edi esi ebx
        mov     eax,1
        ret
endp

proc Ini_SavePos stdcall,f_name,sec_name
        push    ebx edi esi

        xor     ebx,ebx
  .SearchSect:
        mov     edi,[IconsOffs+ebx*4]
        mov     esi,[sec_name]
      @@:
        lodsb
        scasb
        jne     @f
        test    al,al
        jnz     @b

        jmp     .IcoDataFound
      @@:

        inc     ebx
        cmp     ebx,[MaxNumIcon]
        jb      .SearchSect

        mov     eax,1
        pop     esi edi ebx
        ret

.IcoDataFound:
        or      ecx,-1
        repne   scasb
        repne   scasb
        repne   scasb

;int3
        movzx   eax,word[edi+2]
        test    eax,8000h
        jz      @f
        or      eax,0FFFF0000h
     @@:

        stdcall [ini_set_int],[f_name],[sec_name],keyX,eax

        movzx   eax,word[edi]
        test    eax,8000h
        jz      @f
        or      eax,0FFFF0000h
     @@:

        stdcall [ini_set_int],[f_name],[sec_name],keyY,eax

        mov     eax,1
        pop     esi edi ebx
        ret
endp



