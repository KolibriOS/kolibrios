;На потом: добавит загрузку ico и возможность выбора иконки не из iconstrp

ICONS_DAT       equ '/rd/1/icons.dat'
ICON_STRIP      equ '/rd/1/iconstrp.png'
ICON_SIZE       equ 68
IMG_SIZE        equ 32
TEXT_BOTTOM_Y   equ 14
IMAGE_TOP_Y     equ 10    ;>=1
ALIGN_SIZE      equ 68
NAME_LENGTH     equ 11
MIN_NO_MOVING   equ 8
                           ;для диалога создания/редактирования
ICONSX          equ 20
ICONSY          equ 100
ICONS_DRAW_COUNTW equ 10  ;количество иконок в ширину
ICONS_DRAW_COUNTH equ 2   ;количество иконок в высоту
SPCW            equ 3     ;пробел между иконками по горизонтали
SPCH            equ 3
END_ICONS_AREAW equ ICONSX+(IMG_SIZE+SPCW)*ICONS_DRAW_COUNTW-SPCW
END_ICONS_AREAH equ ICONSY+(IMG_SIZE+SPCH)*ICONS_DRAW_COUNTH-SPCH



SizeData        equ bufStdIco+32
BegData         equ fiStdIco.point
;------------------------------------------------------------------------------
        use32
        org 0x0
        db 'MENUET01'   ; 8 byte id
        dd 0x01         ; header version
        dd START        ; start of code
        dd I_END       ; size of image
        dd ENDMEM        ; memory for app
        dd stack_area   ; esp
        dd 0            ; boot parameters
        dd 0            ; path
;------------------------------------------------------------------------------
include 'lang.inc'
include '../../macros.inc'
include '../../proc32.inc'
;include '../../API.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../dll.inc'
;include '../../debug.inc'

;------------------------------------------------------------------------------
START:          ; start of execution
        mcall   68,11
        stdcall dll.Load,IMPORTS
        test    eax,eax
        jnz     ErrLoadLibs

; unpack deflate
        mov     eax,[unpack_DeflateUnpack2]
        mov     [deflate_unpack],eax
;---------------------------------------------------------------------
; get size of file ICONSTRP.PNG
        mcall   70,fiStdIco
        test    eax,eax
        jnz     ErrorStrp
; get memory for ICONSTRP.PNG
        mov     ecx,dword[bufStdIco+32]
        mov     [fiStdIco.size],ecx
        mov     [img_size],ecx
        mcall   68,12
        mov     [fiStdIco.point],eax
        mov     [image_file],eax
; load ICONSTRP.PNG
        mov     dword[fiStdIco],0
        mcall   70,fiStdIco
        test    eax,eax
        jnz     close
; convert PNG to RAW
        xor     eax,eax
        mov     [return_code],eax
;int3

        push    image_file
        call    [cnv_png_import.Start]

        mov     eax,[raw_pointer]
        mov     ebx,[eax+32]
        mov     [strip_file_size],ebx
        mov     eax,[eax+28]
        add     eax,[raw_pointer]
        mov     [strip_file],eax
; back memory to system
        mcall   68,13,[fiStdIco.point]

        mov     eax,[raw_pointer]
        mov     eax,[eax+8]
        shr     eax,5
        mov     [icon_count],eax

        and     eax,0x7
        mov     [cur_band_compensation],eax

;########## загружаем данные иконок в память ##########################

        mcall   70,fiIni                     ;выделяем память, достаточной для хранения ini файла. Её точно хватит для хранения данных об иконках
        test    eax,eax
        jnz     ErrorIni

        cmp     dword[bufIni+32],0
        je      ErrorIni
        mcall   68,12,dword[bufIni+32]
        mov     dword[BegData],eax
        jmp     NoErrIni

  ErrorIni:
        mcall   70,fiRunProg
        mcall   -1

  NoErrIni:
        m2m     [PIcoDB],[BegData]

        stdcall [ini_enum_sections],IconIni,LoadIconsData
;int3
        mov     eax,dword[PIcoDB]
        sub     eax,[BegData]
        mov     dword[SizeData],eax
        mov     eax,[BegData]
        cmp     eax,[PIcoDB]
        jne     @f
        mov     dword[eax],0
        mov     dword[SizeData],0
   @@:
;######################################################################

        call    FillIconsOffs

        mcall   40,0110000b

        mov     eax,[icon_count]
        mov     bl,ICONS_DRAW_COUNTH
        div     bl
        test    ah,ah
        jz      @f
        inc     al
     @@:
        and     eax,0FFh
        mov     [sbIcons.max_area],eax

  ;    int3
;        mov     eax,1
;        mov     eax,[IconsOffs+eax*4]
;        stdcall [ini_del_section],IconIni,eax
;    ret
        jmp     MSGRedrawIcons

messages:
        mcall   10
        sub     eax,5
        jz      MSGRedrawIcons
        dec     eax
        jz      MSGMouse

        jmp     messages

MSGRedrawIcons:

        mcall   48,5
        mov     dx,ax
        shr     eax,16
        sub     dx,ax
        mov     [ScreenX],dx
        mov     ax,bx
        shr     ebx,16
        sub     ax,bx
        mov     [ScreenY],ax

        mov     ecx,[MaxNumIcon]
        test    ecx,ecx
        jz      .NoDraw
        xor     ebx,ebx
    @@: push    ecx
        stdcall DrawIcon,ebx,0
        inc     ebx
        pop     ecx
        loop    @b
   .NoDraw:
        jmp     messages

MSGMouse:
        mcall   37,0    ;GetMousePos
        xor     ebx,ebx
        mov     bx,ax
        shr     eax,16
        mov     ecx,ebx
        mov     ebx,eax

        mcall   34
        cmp     eax,1
        jne     messages

        cmp     [RButtonActiv],1
        je      messages

        mov     [MouseY],ecx
        mov     [MouseX],ebx


        mcall   37,2
        test    al,001b
        jnz     LButtonPress
        test    al,010b
        jnz     RButtonPress
        jmp     messages

ErrLoadLibs:
        ;dps     'Не удалось загрузить необходимые библиотеки'
        ;debug_newline
        jmp     close
ErrorStrp:
        ;dps     'Ошибка открытия iconstrp.png'
        ;debug_newline
close:
        mcall   -1

LButtonPress:
        stdcall GetNumIcon,[MouseX],[MouseY],-1
;int3
        cmp     eax,-1
        jnz     @f

    WaitLB1:
        mcall   37,2
        test    al,001b
        jz      messages
        ;Yield
        mcall   5,1
        jmp     WaitLB1


     @@:
        push    eax
        stdcall DrawIcon,eax,1
   WaitLB:
        mcall   37,2
        test    al,001b
        jz      endWaitLB

        mcall   37,0
        xor     ebx,ebx
        mov     bx,ax
        shr     eax,16
        sub     eax,[MouseX]
        jns     @f
        neg     eax
      @@:
        sub     ebx,[MouseY]
        jns     @f
        neg     ebx
      @@:
        cmp     [bFixIcons],0
        jne     @f
        cmp     eax,MIN_NO_MOVING
        ja      MovingIcon
        cmp     ebx,MIN_NO_MOVING
        ja      MovingIcon
      @@:
        ;Yield
        mcall   5,1     ;Sleep   1
        jmp     WaitLB
   endWaitLB:

        mcall   37,0
        xor     ebx,ebx
        mov     bx,ax
        shr     eax,16
        mov     ecx,ebx
        mov     ebx,eax
        mov     [MouseX],ebx
        mov     [MouseY],ecx

        stdcall GetNumIcon,[MouseX],[MouseY],-1
        cmp     eax,[esp]       ;[esp] = номер иконки
        jne     @f

        mov     edi,[IconsOffs+eax*4]
        or      ecx,-1
        xor     al,al
        repne   scasb
        mov     ebx,edi
        repne   scasb
                             ;run program
        mov     dword[fiRunProg+8],edi
        mov     dword[fiRunProg+21],ebx
        mcall   70,fiRunProg
        test    eax,80000000h
        jz      @f

        mov     dword[fiRunProg+8],ErrRunProg
        mov     dword[fiRunProg+21],pthNotify
        mcall   70,fiRunProg

     @@:
        pop     eax
        stdcall RestoreBackgrnd,eax
        jmp     messages

;-------------------------------------------------------------------------------
MovingIcon:
        stdcall GetNumIcon,[MouseX],[MouseY],-1
        mov     [SelIcon],eax
        stdcall RestoreBackgrnd,[SelIcon]

;        mov     ecx,[MaxNumIcon]
;        xor     ebx,ebx
;   .MI: push    ecx
;        cmp     ebx,[SelIcon]
;        je      @f
;        stdcall DrawIcon,ebx,0
;      @@:
;        inc     ebx
;        pop     ecx
;        loop    .MI



;dps 'Moving'
;debug_newline
;        mov     edi,[SelIcon]
;        mov     edi,[IconsOffs+edi*4]
;        or      ecx,-1
;        xor     al,al
;        repne scasb
;        repne scasb
;        repne scasb
;        repne scasb
;        xor     ebx,ebx
;        xor     esi,esi
;        mov     bx,word[edi+2]
;        mov     si,word[edi]
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;  dps 'Q'
;        stdcall DrawIcon,eax,0
;     @@:
;
;        add     ebx,ICON_SIZE-1
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;  dps 'Q'
;        stdcall DrawIcon,eax,0
;
;     @@:
;        add     esi,ICON_SIZE-1
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;  dps 'Q'
;        stdcall DrawIcon,eax,0
;
;     @@:
;        sub     ebx,ICON_SIZE-1
;        stdcall GetNumIcon,ebx,esi,[SelIcon]
;        cmp     eax,-1
;        je      @f
;   dps 'Q'
;        stdcall DrawIcon,eax,0
;     @@:


; Sleep 40

;qweqwe:

        mov    [MovingActiv],1
        mcall   51,1,MovingWnd,stack_dlg        ;CreateThread MovingWnd,stack_dlg
   .WaitLB:
        mcall   37,2    ;GetMouseKey
        test    al,001b
        jz      .endWaitLB

        ;Yield
        mcall   5,1     ;Sleep 1
        jmp     .WaitLB
   .endWaitLB:
        mov    [MovingActiv],0

        mcall   37,0            ;GetMousePos
        xor     ebx,ebx
        mov     bx,ax
        shr     eax,16

        add     ebx,7
        dec     eax

        sub     eax,ICON_SIZE/2
        jns     @f
        xor     eax,eax
      @@:
        sub     ebx,ICON_SIZE/2
        jns     @f
        xor     ebx,ebx
      @@:

        movzx   edx,[ScreenX]
        sub     edx,ICON_SIZE
        cmp     eax,edx
        jbe     @f
        mov     eax,edx
      @@:

        movzx   edx,[ScreenY]
        sub     edx,ICON_SIZE
        cmp     ebx,edx
        jbe     @f
        mov     ebx,edx
      @@:

        xor     edx,edx
        mov     dx,[ScreenX]
        shr     edx,1
        cmp     eax,edx
        jbe     @f
        sub     ax,[ScreenX]
        inc     ax
      @@:

        xor     edx,edx
        mov     dx,[ScreenY]
        shr     edx,1
        cmp     ebx,edx
        jbe     @f
        sub     bx,[ScreenY]
        inc     bx
      @@:

        stdcall SetPosition,[SelIcon],eax,ebx
        mcall   15,3

        m2m     [PIcoDB],[BegData]
        stdcall [ini_enum_sections],IconIni,Ini_SavePos  ;in RButton.inc

;        mov     dword[fInfo],2
;        mcall   70,fInfo

        mov     [bNotSave],1

        jmp     messages

;-------------------------------------------------------------------------------

RButtonPress:
        cmp     [RButtonActiv],1
        je      messages
        mov     [RButtonActiv],1

     @@:
        mcall   37,2    ;GetMouseKey
        test    al,010b
        jz      @f
        mcall   5,1     ;Yield
        jmp     @b
     @@:

        mcall   51,1,RButtonWin,stack_dlg       ;CreateThread RButtonWin,stack_dlg

        jmp     messages


;###############################################################################
;ret eax = 1/0 = удача/ошибка
proc DrawIcon NumIcon:DWORD,Activ:DWORD ;NumIcon = 0..n
local   IconData:DWORD
   ; int3
        push    ebx edi esi

        mov     ecx,ICON_SIZE*ICON_SIZE
        xor     eax,eax
        ;mov     eax,0FFFFFFFh
        mov     edi,IconArea
        rep     stosd

        mov     eax,[NumIcon]
        cmp     eax,[MaxNumIcon]
        jb      @f
        xor     eax,eax
        pop     esi edi ebx
        ret
      @@:

        mov     esi,[IconsOffs+eax*4]
        mov     [IconData],esi

                              ;рисуем текст
        mov     edi,esi
        xor     al,al
        or      ecx,-1
        repne scasb
        mov     edx,esi
        mov     eax,edi
        sub     eax,esi
        dec     eax
        shl     eax,1            ;*6
        lea     eax,[eax*2+eax]
        mov     ebx,ICON_SIZE
        sub     ebx,eax
        shr     ebx,1        ;ebx = x текста
        shl     ebx,16
        mov     bx,ICON_SIZE
        sub     bx,TEXT_BOTTOM_Y
        mov     ecx,88000000h
        mov     edi,IconAreaH
        add     ebx,-1*10000h+0
        mcall   4
        add     ebx,2*10000h+0
        mcall   4
        add     ebx,-1*10000h-1
        mcall   4
        add     ebx,0*10000h+2
        mcall   4
        add     ebx,1*10000h+0
        mcall   4
        add     ebx,0*10000h+1
        mcall   4
        add     ebx,-1*10000h+0
        mcall   4
        add     ebx,0*10000h-2
        mov     ecx,88FFFFFFh
        mcall   4
                                    ;рисуем картинку

        mov     edi,esi
        xor     al,al
        or      ecx,-1
        repne   scasb
        repne   scasb
        repne   scasb
        mov     al,[edi]
        cmp     al,'9'
        ja      PathToIcon
        cmp     al,'/'
        jne     GetIconInd

   PathToIcon:
        ;stdcall LoadIcon,edi
        ;mov     esi,eax
        mov     al,30h           ;заглушка!!!!!!!!!!!!!
        mov     byte[edi+1],0

        jmp     CopyToMem
   GetIconInd:

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
        je      DI1
        mov     ecx,eax
        xor     eax,eax
      @@:
        add     eax,IMG_SIZE*IMG_SIZE*4
        loop    @b
     DI1:
        mov     esi,eax
        add     esi,[raw_pointer]
        add     esi,0+4*11

  CopyToMem:
        mov     edi,IconArea+((IMAGE_TOP_Y*ICON_SIZE)+((ICON_SIZE-IMG_SIZE)/2))*4

        mov     eax,IMG_SIZE
        mov     edx,eax
      @@:
        mov     ecx,eax
        rep     movsd
        add     edi,(ICON_SIZE-IMG_SIZE)*4
        dec     edx
        jnz     @b

;-----------------
                                ;если надо, то рисуем обводку
        cmp     [Activ],0
        je      .NoSelect

        mov     edi,IconArea
        mov     eax,0FF000000h
        mov     ecx,ICON_SIZE
        rep     stosd
        ;mov     edi,IconArea+ICON_SIZE*1

        mov     ecx,ICON_SIZE-1
    @@: mov     dword[edi],eax
        add     edi,(ICON_SIZE)*4
        loop    @b

        mov     edi,IconArea+ICON_SIZE*2*4-4
        mov     ecx,ICON_SIZE-1
    @@: mov     dword[edi],eax
        add     edi,(ICON_SIZE)*4
        loop    @b

        mov     edi,IconArea+ICON_SIZE*(ICON_SIZE-1)*4+4
        mov     ecx,ICON_SIZE-2
        rep     stosd


        mov     edi,IconArea+ICON_SIZE*4+4
        mov     eax,0FFFFFFFFh
        mov     ecx,ICON_SIZE-2
        rep     stosd

        mov     edi,IconArea+ICON_SIZE*4+4
        mov     ecx,ICON_SIZE-2
    @@: mov     dword[edi],eax
        add     edi,(ICON_SIZE)*4
        loop    @b

        mov     edi,IconArea+ICON_SIZE*2*4-4*2
        mov     ecx,ICON_SIZE-3
    @@: mov     dword[edi],eax
        add     edi,(ICON_SIZE)*4
        loop    @b

        mov     edi,IconArea+ICON_SIZE*(ICON_SIZE-2)*4+4*2
        mov     ecx,ICON_SIZE-3
        rep     stosd
;--------------

   .NoSelect:
        mov     edi,[IconData]
        xor     al,al
        or      ecx,-1
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb
        mov     edx,[edi]

        test    edx,00008000h
        jz      @f
        add     dx,[ScreenY]
    @@:
        test    edx,80000000h
        jz      @f
        rol     edx,16
        add     dx,[ScreenX]
        rol     edx,16
    @@:

        mov     ebx,IconArea
        mov     ecx,ICON_SIZE*10000h+ICON_SIZE
        mcall   25

;        mov     eax,1
        pop     esi edi ebx
        ret
endp



proc RestoreBackgrnd,NumIcon:DWORD
        push    ebx edi
        mov     eax,[NumIcon]
        cmp     eax,[MaxNumIcon]
        jb      @f
        xor     eax,eax
        pop     edi ebx
        ret
      @@:

        mov     edi,[IconsOffs+eax*4]
        xor     al,al
        or      ecx,-1
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb

        mov     ax,[edi+2]
        test    ax,8000h
        jz      @f
        add     ax,[ScreenX]
      @@:

        mov     bx,[edi]
        test    bx,8000h
        jz      @f
        add     bx,[ScreenY]
      @@:

        mov     cx,ax
        shl     ecx,16
        add     ax,ICON_SIZE
        mov     cx,ax
        mov     dx,bx
        shl     edx,16
        add     bx,ICON_SIZE
        mov     dx,bx
        dec     ecx
        dec     edx
        mcall   15,9
        pop     edi ebx
        ret
endp

;ret eax=numIcon
proc AddIcon stdcall,x,y,lpIcon,lpName,lpExeFile,lpParams
        push    ebx edi esi

        mov     eax,[x]
        mov     ebx,[y]

        xor     edx,edx
        test    eax,8000h
        jnz     @f
        mov     dx,[ScreenX]    ;если надо, то преобразовываем в
        shr     edx,1           ;отрицательные координаты
        cmp     eax,edx
        jbe     @f
        sub     ax,[ScreenX]
        inc     eax
      @@:

        test    ebx,8000h
        jnz     @f
        mov     dx,[ScreenY]
        shr     edx,1
        cmp     ebx,edx
        jbe     @f
        sub     bx,[ScreenY]
        inc     ebx
      @@:
                                ;поправка на случай выхода за край экрана

        test    eax,8000h
        jnz     @f
        mov     dx,[ScreenX]
        sub     edx,ICON_SIZE
        cmp     eax,edx
        jbe     @f
        mov     eax,edx
     @@:

        test    ebx,8000h
        jnz     @f
        mov     dx,[ScreenY]
        sub     edx,ICON_SIZE
        cmp     ebx,edx
        jbe     @f
        mov     ebx,edx
     @@:

        mov     [x],eax
        mov     [y],ebx

        xor     al,al
        or      ecx,-1
        mov     edi,[lpName]
        repne   scasb
        sub     edi,[lpName]
        mov     ebx,edi
        mov     edi,[lpExeFile]
        repne   scasb
        sub     edi,[lpExeFile]
        add     ebx,edi
        mov     edi,[lpParams]
        repne   scasb
        sub     edi,[lpParams]
        add     ebx,edi
        mov     edi,[lpIcon]
        repne   scasb
        sub     edi,[lpIcon]
        add     ebx,edi
        add     ebx,4
        mov     ecx,dword[SizeData]
        add     ecx,ebx
        mov     edx,[IconsOffs]
        mcall   68,20

        mov     edx,dword[SizeData]
        mov     dword[SizeData],ecx
        mov     dword[BegData],eax
        mov     edi,eax
        add     edi,edx
        mov     esi,[lpName]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

        mov     esi,[lpExeFile]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

        mov     esi,[lpParams]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

        mov     esi,[lpIcon]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

        mov     eax,[x]
        mov     ebx,[y]
        shl     eax,16
        mov     ax,bx
        mov     dword[edi],eax

        stdcall FillIconsOffs

        pop     esi edi ebx
        ret
endp


proc EditIcon stdcall,NumIcon,lpIcon,lpName,lpExeFile,lpParams
        push    edi
        mov     eax,[NumIcon]
        mov     edi,[IconsOffs+eax*4]
        xor     al,al
        or      ecx,-1
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb
        push    dword[edi]
        stdcall DelIcon,[NumIcon]
        pop     edx
        xor     eax,eax
        mov     ax,dx
        shr     edx,16
        stdcall AddIcon,edx,eax,DAreaIcon,DAreaName,DAreaPath,DAreaParams
        pop     edi
        ret
endp

proc SetPosition stdcall,NumIcon,x,y
        push    edi
        mov     eax,[NumIcon]
        mov     edi,[IconsOffs+eax*4]
        xor     al,al
        or      ecx,-1
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb
        mov     eax,[x]
        shl     eax,16
        mov     ax,word[y]
        mov     dword[edi],eax
        pop     edi
        ret
endp

proc GetNumIcon stdcall,x,y,NumIconI  ;номер иконки, который проверять не надо или -1
local posX:WORD,\
      posY:WORD
        push    ebx edi
        mov     ecx,[MaxNumIcon]
        test    ecx,ecx
        jnz     @f
        or      eax,-1
        pop     edi ebx
        ret
     @@:

        xor     ebx,ebx
   .TestIcon:
        cmp     dword[NumIconI],-1
        je      @f

        cmp     ebx,[NumIconI]
        jne     @f
        inc     ebx
        dec     ecx
        test    ecx,ecx
        jz      .NoIcon
     @@:

        push    ecx

        or      ecx,-1
        xor     al,al
        mov     edi,[IconsOffs+ebx*4]
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb

        mov     ax,[edi+2]
        test    ax,8000h
        jz      @f
        add     ax,[ScreenX]
      @@:
        mov     [posX],ax

        mov     ax,[edi]
        test    ax,8000h
        jz      @f
        add     ax,[ScreenY]
      @@:
        mov     [posY],ax

        mov     eax,[x]
        mov     edx,[y]

        cmp     ax,[posX]
        jb      @f
        add     word[posX],ICON_SIZE
        cmp     ax,[posX]
        ja      @f

        cmp     dx,[posY]
        jb      @f
        add     word[posY],ICON_SIZE
        cmp     dx,[posY]
        ja      @f

        jmp     .OkIcon
     @@:

        inc     ebx
        pop     ecx

        ;loop    .TestIcon
        dec     ecx
        jnz     .TestIcon
        jmp     .NoIcon
   .OkIcon:
        mov     eax,ebx
        pop     edi ebx
        ret
   .NoIcon:
        or      eax,-1
        pop     edi ebx
        ret
endp

proc DelIcon stdcall,NumIcon
        push    ebx edi esi

        cmp     [MaxNumIcon],0
        je      .end

        cmp     [MaxNumIcon],1
        je      .OhneIco

        mov     ebx,[NumIcon]

        mov     eax,[MaxNumIcon]
        dec     eax
        mov     [MaxNumIcon],eax
        cmp     ebx,eax
        je      @f

        shl     ebx,2

        mov     ecx,dword[BegData]
        add     ecx,dword[SizeData]
        sub     ecx,[IconsOffs+4+ebx]

        mov     edx,[IconsOffs+4+ebx]
        sub     edx,[IconsOffs+ebx]
        sub     dword[SizeData],edx

        mov     esi,[IconsOffs+4+ebx]
        mov     edi,[IconsOffs+ebx]
        rep     movsb
        jmp     .endDel

     @@:
        mov     ecx,dword[BegData]
        add     ecx,dword[SizeData]
        sub     ecx,[IconsOffs+ebx*4]
        sub     dword[SizeData],ecx

  .endDel:
        stdcall FillIconsOffs
        jmp     .end

.OhneIco:
        mov     edi,[BegData]
        mov     [SizeData],0
        mov     dword[edi],0
        mov     [MaxNumIcon],0
        mov     dword[IconsOffs],0
   .end:
        mcall   15,3
        pop     esi edi ebx
        ret
endp
                                 ;заполняет MaxNumIcon,IconsOffs
proc FillIconsOffs
        push    ebx edi
        mov     edi,[BegData]
        mov     dword[MaxNumIcon],0
        cmp     dword[edi],0
        jne     @f
        mov     dword[IconsOffs],0
        pop     edi ebx
        ret
    @@:

        mov     [IconsOffs],edi
        xor     al,al
        xor     edx,edx
        mov     ebx,dword[SizeData]
        add     ebx,dword[BegData]
        or      ecx,-1
 .CalcNumIc:
        repne   scasb
        repne   scasb
        repne   scasb
        repne   scasb
        add     edi,4
        mov     dword[IconsOffs+edx+4],edi
        inc     dword[MaxNumIcon]
        add     edx,4

        cmp     edi,ebx
        jae     @f
        jmp     .CalcNumIc
   @@:

        mov     dword[IconsOffs+edx],0
        pop     edi ebx
        ret
endp

proc LoadIconsData stdcall,f_name,sec_name
        push    ebx esi edi

        mov     edi,secRButt
        mov     esi,[sec_name]
    @@: lodsb
        scasb
        jnz     .lid1
        test    al,al
        jnz     @b

        mov     eax,1
        pop     edi esi ebx
        ret
     .lid1:

        mov     edi,[PIcoDB]
        mov     esi,[sec_name]
    @@: lodsb
        stosb
        test    al,al
        jnz     @b

        stdcall [ini_get_str],[f_name],[sec_name],keyPath,edi,4096,0
        test    eax,eax
        jz      @f
        xor     eax,eax
        pop     edi esi ebx
        ret
     @@:
        xor     al,al
        or      ecx,-1
        repne   scasb

        stdcall [ini_get_str],[f_name],[sec_name],keyParams,edi,4096,0
        test    eax,eax
        jz      @f
        xor     eax,eax
        pop     edi esi ebx
        ret
     @@:
        xor     al,al
        or      ecx,-1
        repne   scasb

        stdcall [ini_get_str],[f_name],[sec_name],keyIco,edi,4096,0
        test    eax,eax
        jz      @f
        xor     eax,eax
        pop     edi esi ebx
        ret
     @@:
        xor     al,al
        or      ecx,-1
        repne   scasb

        stdcall [ini_get_int],[f_name],[sec_name],keyX,80000000h
        cmp     eax,80000000h
        jne     @f
        xor     eax,eax
        pop     edi esi ebx
        ret
     @@:
        mov     word[edi+2],ax

        stdcall [ini_get_int],[f_name],[sec_name],keyY,80000000h
        cmp     eax,80000000h
        jne     @f
        xor     eax,eax
        pop     edi esi ebx
        ret
     @@:
        mov     word[edi],ax
        add     edi,4
        mov     [PIcoDB],edi

        mov     eax,1
        pop     edi esi ebx
        ret
endp

include 'RButton.inc'
include 'DlgAdd.inc'
include 'Moving.inc'
;include 'Ico.inc'

;'Eolite',0,'/sys/File managers/eolite',0,'/hd0/3/Muzik',0,'1',0,00010001h
;-------------------------------------------------------------------------------
;##### DATA ####################################################################
;-------------------------------------------------------------------------------
; not change this section!!!
; start section
;------------------------------------------------------------------------------
align 4
image_file     dd 0 ;+0
raw_pointer    dd 0 ;+4
return_code    dd 0 ;+8
img_size       dd 0 ;+12
deflate_unpack dd 0 ;+16        ; not use for scaling
raw_pointer_2  dd 0 ;+20        ; not use for scaling
;------------------------------------------------------------------------------
; end section
;------------------------------------------------------------------------------


align 4
fiStdIco:
        dd 5
        dd 0
        dd 0
.size   dd 0
.point  dd bufStdIco
        db ICON_STRIP,0


align 4
fiRunProg:            ;для запуска программ
        dd 7
        dd 0
        dd 0
        dd 0
        dd ErrNotFoundIni
        db 0
        dd pthNotify

fiIni   dd 5           ;для ini файла
        dd 0
        dd 0
        dd 0
        dd bufIni
        db '/rd/1/icon.ini',0


IconsFile       db ICON_STRIP,0
NameIconsDat    db ICONS_DAT,0
align 4
MaxNumIcon      dd 0           ;количество иконок

bFixIcons       dd 0
bNotSave        dd 0

LButtonActiv    dd 0
RButtonActiv    dd 0
MovingActiv     dd 0

IconIni         db '/rd/1/icon.ini',0


;keyName         db 'name',0
keyPath         db 'path',0
keyParams       db 'param',0
keyIco          db 'ico',0
keyX            db 'x',0
keyY            db 'y',0

;-------------------------------------------------------------------------------
IMPORTS:
library cnv_png,'cnv_png.obj',\
        archiver,'archiver.obj',\
        box_lib,'box_lib.obj',\
        proc_lib,'proc_lib.obj',\
        libini,'libini.obj'

import  cnv_png,\
        cnv_png_import.Start,'START',\
        cnv_png_import.Version,'version',\
        cnv_png_import.Check,'Check_Header',\
        cnv_png_import.Assoc,'Associations'

import  archiver,\
        unpack_DeflateUnpack2,'deflate_unpack2'

import  box_lib,\
        edit_box_draw,'edit_box',\
        edit_box_key,'edit_box_key',\
        edit_box_mouse,'edit_box_mouse',\
        scrollbar_h_draw,'scrollbar_h_draw',\
        scrollbar_h_mouse,'scrollbar_h_mouse'

import  proc_lib,\
        OpenDialog_Init,'OpenDialog_init',\
        OpenDialog_Start,'OpenDialog_start'

import  libini,\
        ini_enum_sections,'ini_enum_sections',\
        ini_enum_keys,'ini_enum_keys',\
        ini_get_str,'ini_get_str',\
        ini_set_str,'ini_set_str',\
        ini_get_color,'ini_get_color',\
        ini_get_int,'ini_get_int',\
        ini_set_int,'ini_set_int',\
        ini_del_section,'ini_del_section'


;ini.get_str (f_name, sec_name, key_name, buffer, buf_len, def_val)
;ini.set_str (f_name, sec_name, key_name, buffer, buf_len)


;-------------------------------------------------------------------------------
;----- RButton.inc -------------------------------------------------------------
;-------------------------------------------------------------------------------
secRButt        db 'rbmenu',0

if lang eq ru
 RMenuRedrawFon db 'Перерисовать',0
 RMenuAlign     db 'Выровнять по сетке',0
 RMenuAdd       db 'Добавить',0
 RMenuDel       db 'Удалить',0
 RMenuProp      db 'Свойства',0
 RMenuOffMoving db 'Закрепить иконки',0
 RMenuOnMoving  db 'Открепить иконки',0

else

 RMenuRedrawFon db 'Redraw',0
 RMenuAlign     db 'Snap to Grid',0
 RMenuAdd       db 'Add',0
 RMenuDel       db 'Delete',0
 RMenuProp      db 'Properties',0
 RMenuOffMoving db 'Fix the icons',0
 RMenuOnMoving  db 'Unfix the icons',0

end if

if lang eq ru

 ErrRunProg     db 'Ошибка запуска программы',0
 WarningSave    db 'Не забудьте сохранить изменения, запустить RDSave',0
 ErrNotFoundIni db 'Не найден icon.ini',0

else

 ErrRunProg     db 'Error runing program',0
 WarningSave    db 'Do not forget to save the changes, run the RDSave',0
 ErrNotFoundIni db 'icon.ini not found',0

end if


pthNotify       db '/rd/1/@notify',0

;-------------------------------------------------------------------------------
;------- AddDlg.inc ---------------------------------------------------------------
;-------------------------------------------------------------------------------
if lang eq ru
DTitle          db 'Добавить иконку',0

DCaptName       db 'Имя',0
DCaptPath       db 'Путь',0
DCaptParams     db 'Параметры',0
DCaptIcon       db 'Иконка',0
;DCaptChange     db '.',0
DCaptCreate     db 'Создать',0
DCaptProperties db 'Изменить',0
DCaptCancel     db 'Отменить',0

else
DTitle          db 'Add icon',0

DCaptName       db 'Name',0
DCaptPath       db 'Path',0
DCaptParams     db 'Parameters',0
DCaptIcon       db 'Icon',0
;DCaptChange     db '.',0
DCaptCreate     db 'Create',0
DCaptProperties db 'Change',0
DCaptCancel     db 'Cancel',0
end if

;/не менять положение
edtName    edit_box NAME_LENGTH*6+4,70+20+IMG_SIZE,6,0FFFFFFh,06F9480h,0FFh,0h,0,NAME_LENGTH,\
                DAreaName,mouse_dd,0,0,0
edtExePath edit_box 281-3-20-IMG_SIZE,70+20+IMG_SIZE,26,0FFFFFFh,06F9480h,0FFh,0h,0,256,\
                DAreaPath,mouse_dd,0,0,0
edtParams  edit_box 295-20-IMG_SIZE,70+20+IMG_SIZE,46,0FFFFFFh,06F9480h,0FFh,0h,0,256,\
                DAreaParams,mouse_dd,0,0,0
edtIcon    edit_box 295-20-IMG_SIZE,70+20+IMG_SIZE,66,0FFFFFFh,06F9480h,0FFh,0h,0,256,\
                DAreaIcon,mouse_dd,0,0,0
endEdits:
;\

sbIcons:
             dw END_ICONS_AREAW-ICONSX
             dw ICONSX
             dw 15
             dw END_ICONS_AREAH+3
             dd 0
             dd 1
 .max_area   dd 0
 .cur_area   dd ICONS_DRAW_COUNTW
 .position   dd 0
 .bckg_col   dd 0
 .frnt_col   dd 0
 .line_col   dd 0
 .redraw     dd 0
 .delta      dd 0
 .delta2     dd 0
 .r_size_x   dw 0
 .r_start_x  dw 0
 .r_size_y   dw 0
 .r_start_y  dw 0
 .m_pos      dd 0
 .m_pos2     dd 0
 .m_keys     dd 0
 .run_size   dd 0
 .position2  dd 0
 .work_size  dd 0
 .all_redraw dd 0
 .ar_offset  dd 0

;-------------------------------------------------------------------------------
OpenDialog_data:
.type                   dd 0
.procinfo               dd RBProcInfo       ;+4
.com_area_name          dd communication_area_name      ;+8
.com_area               dd 0    ;+12
.opendir_pach           dd temp_dir_pach        ;+16
.dir_default_pach       dd communication_area_default_pach      ;+20
.start_path             dd open_dialog_path     ;+24
.draw_window            dd DRedraw;draw_window_for_OD   ;+28
.status                 dd 0    ;+32
.openfile_pach          dd DAreaPath;fname_Info   ;+36
.filename_area          dd 0;DAreaPath        ;+40
.filter_area            dd Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 100 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 100 ;+54 ; Window Y position

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
open_dialog_path:
        db '/sys/File managers/opendial',0

communication_area_default_pach:
        db '/sys',0

Filter  dd 0

;open_dialog     db 0
;-------------------------------------------------------------------------------

;/не разделять
align 4
IconAreaH       dd ICON_SIZE,ICON_SIZE
I_END:
;##### UDATA ###################################################################
IconArea        rb 4*ICON_SIZE*ICON_SIZE
;\

ScreenX         rw 1
ScreenY         rw 1

MouseX          rd 1
MouseY          rd 1

RBSlot          rd 1

AddX            rd 1
AddY            rd 1

SelIcon         rd 1

sc              system_colors

align 4
bufStdIco       rb 40
IconsOffs       rd 100
PIcoDB          rd 1

align 4
icon_count      rd 1
strip_file      rd 1
strip_file_size rd 1

cur_band_compensation rd 1

;---- RButton -----------------------------------------------------------------------
bufIni          rb 40
NumUserButt     rd 1
RBUser          rd 16*2+1
RMenuW          rw 1
RMenuH          rw 1
RMenuHsb        rw 1

MaxPage         rd 1

mouse_dd        rd 1

DAreaName       rb NAME_LENGTH+1
DAreaPath       rb 256+1
DAreaParams     rb 256+1
DAreaIcon       rb 256+1

align 4
RBProcInfo      rb 1024
align 4

; OpenDialog
temp_dir_pach   rb 1024
fname_Info      rb 1024
;-------------------------------------------------------------------------------
                rb 1024
stack_dlg:
align 4
                rb 1024
stack_area:
;------------------------------------------------------------------------------
ENDMEM: