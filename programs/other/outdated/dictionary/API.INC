;-------------------------------------------------------------------------------
;=========== Constans ==========================================================
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
MEV_LDOWN = 1
MEV_LUP   = 2
MEV_RDOWN = 3
MEV_RUP   = 4
MEV_WHEEL = 5
MEV_MOVE  = 6

;//Mouse buttons state
MBS_LEFT   = 1
MBS_RIGHT  = 2
MBS_CENTER = 4
MBS_4	   = 8
MBS_5	   = 16

;// KEY MODIFIERS

KM_SHIFT	= 0x00010000
KM_CTRL 	= 0x00020000
KM_ALT		= 0x00040000

KM_CTRLSHIFT	= 0x00030000
KM_ALTSHIFT	= 0x00050000
KM_CTRLALT	= 0x00060000

KM_CTRLALTSHIFT = 0x00070000

;// KEY CODES

KEY_ESCAPE	= 0x00000001
KEY_SNAPSHOT	= 0x00000137
KEY_PAUSE	= 0x0000021D

KEY_F1		= 0x0000003B
KEY_F2		= 0x0000003C
KEY_F3		= 0x0000003D
KEY_F4		= 0x0000003E
KEY_F5		= 0x0000003F
KEY_F6		= 0x00000040
KEY_F7		= 0x00000041
KEY_F8		= 0x00000042
KEY_F9		= 0x00000043
KEY_F10 	= 0x00000044
KEY_F11 	= 0x00000057
KEY_F12 	= 0x00000058

KEY_TILDE	= 0x00000029
KEY_MINUS	= 0x0000000C
KEY_EQUAL	= 0x0000000D
KEY_BACKSPACE	= 0x0000000E
KEY_TAB 	= 0x0000000F
KEY_SQBRACE1	= 0x0000001A
KEY_SQBRACE2	= 0x0000001B
KEY_SLASH1	= 0x00000035
KEY_SLASH2	= 0x0000002B
KEY_SEMICOLON	= 0x00000027
KEY_QUOTE	= 0x00000028
KEY_COMMA	= 0x00000033
KEY_DOT 	= 0x00000034
KEY_SPACE	= 0x00000039

KEY_1		= 0x00000002
KEY_2		= 0x00000003
KEY_3		= 0x00000004
KEY_4		= 0x00000005
KEY_5		= 0x00000006
KEY_6		= 0x00000007
KEY_7		= 0x00000008
KEY_8		= 0x00000009
KEY_9		= 0x0000000A
KEY_0		= 0x0000000B

KEY_A		= 0x0000001E
KEY_B		= 0x00000030
KEY_C		= 0x0000002E
KEY_D		= 0x00000020
KEY_E		= 0x00000012
KEY_F		= 0x00000021
KEY_G		= 0x00000022
KEY_H		= 0x00000023
KEY_I		= 0x00000017
KEY_J		= 0x00000024
KEY_K		= 0x00000025
KEY_L		= 0x00000026
KEY_M		= 0x00000032
KEY_N		= 0x00000031
KEY_O		= 0x00000018
KEY_P		= 0x00000019
KEY_Q		= 0x00000010
KEY_R		= 0x00000013
KEY_S		= 0x0000001F
KEY_T		= 0x00000014
KEY_U		= 0x00000016
KEY_V		= 0x0000002F
KEY_W		= 0x00000011
KEY_X		= 0x0000002D
KEY_Y		= 0x00000015
KEY_Z		= 0x0000002C

KEY_CAPSLK	= 0x0000003A
KEY_SCRLK	= 0x00000046
KEY_NUMLK	= 0x00000069

KEY_RETURN	= 0x0000001C
KEY_NUMRETURN	= 0x0000011C

KEY_LCONTROL	= 0x0000001D
KEY_RCONTROL	= 0x0000011D
KEY_LSHIFT	= 0x0000002A
KEY_RSHIFT	= 0x00000036
KEY_LALT	= 0x00000038
KEY_RALT	= 0x00000138

KEY_LWIN	= 0x0000015B
KEY_RWIN	= 0x0000015C
KEY_APPS	= 0x0000015D

KEY_INSERT	= 0x00000152
KEY_DELETE	= 0x00000153
KEY_HOME	= 0x00000147
KEY_END 	= 0x0000014F
KEY_PAGEUP	= 0x00000149
KEY_PAGEDOWN	= 0x00000151
KEY_UP		= 0x00000148
KEY_LEFT	= 0x0000014B
KEY_RIGHT	= 0x0000014D
KEY_DOWN	= 0x00000150

KEY_DIVIDE	= 0x00000135
KEY_MULTIPLY	= 0x00000037
KEY_SUBTRACT	= 0x0000004A
KEY_ADD 	= 0x0000004E
KEY_SEPARATOR	= 0x00000053

KEY_NUM1	= 0x0000004F
KEY_NUM2	= 0x00000050
KEY_NUM3	= 0x00000051
KEY_NUM4	= 0x0000004B
KEY_NUM5	= 0x0000004C
KEY_NUM6	= 0x0000004D
KEY_NUM7	= 0x00000047
KEY_NUM8	= 0x00000048
KEY_NUM9	= 0x00000049
KEY_NUM0	= 0x00000052

;// EXTENDED KEY CODES (MULTIMEDIA KEYBOARDS)

KEY_MYDOCUMENTS = 0x0000014C
KEY_MYPICTURES	= 0x00000164
KEY_MYMUSIC	= 0x0000013C
KEY_MUTE	= 0x00000120
KEY_PLAYPAUSE	= 0x00000122
KEY_STOP	= 0x00000124
KEY_VOLUMEUP	= 0x00000130
KEY_VOLUMEDOWN	= 0x0000013E
KEY_PREVSONG	= 0x00000110
KEY_NEXTSONG	= 0x00000119
KEY_MEDIA	= 0x0000016D
KEY_MAIL	= 0x0000016C
KEY_WEBHOME	= 0x00000132
KEY_MESSENGER	= 0x00000105

KEY_CALCULATOR	= 0x00000121
KEY_LOGOFF	= 0x00000116
KEY_SLEEP	= 0x0000015F

;Keyboard modes (func 66,2)

KBM_ASCII = 0
KBM_SCAN  = 1

;-------------------------------------------------------------------------------
;Colors-------------------------------------------------------------------------

clWhite = 0FFFFFFh
clBlack = 0
clRed	= 0FF0000h
clBlue	= 0FF00h
clGreen = 0FFh
clGrey	= 0888888h


;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;========== APIs macros ========================================================
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------

macro DrawWindow x,y,w,h,nameW	 ;void
{
    if ~(x eq bx)
       mov bx,x
    end if
    rol ebx,16
    mov bx,w
    if ~(y eq cx)
       mov cx,y
    end if
    rol ecx,16
    mov cx,h
    mov edx,0x33FFFFFF
    mov esi,0x80333333
    mov edi,nameW
    xor eax,eax
    int 40h
}

macro DrawPanel x,y,w,h,color	 ;void    ;рисует неперемещаемое прозрачное(или цвета color) окно
{
    xor eax,eax
    if ~(x eq bx)
       mov bx,x
    end if
    rol ebx,16
    mov bx,w
    if ~(y eq cx)
       mov cx,y
    end if
    rol ecx,16
    mov cx,h
    mov edx,21000000h
   ; add edx,color
    mov esi,01000000h
   ; mov edi,color
    int 40h

    if ~(color eq)
       mov eax,13
       xor bx,bx
       rol ebx,16
       mov bx,w
       xor cx,cx
       rol ecx,16
       mov cx,h
       mov edx,color
       int 40h
    end if
}


;WinClass
;       .Caption  (offset Name)
;       .Color (color dword)
;       .Transparent (0/1)
;       .WorkTransp  (0/1)
;       .Fix         (0/1)
;       .Skin        (0/1)
;       .CaptionColor (color dword)
;       .FrameColor   (color dword)

macro DrawWindowEx x,y,w,h,WinClass   ;void может рисовать любое окно
{
    if ~(x eq bx)
       mov bx,x
    end if
    rol ebx,16
    mov bx,w
    if ~(y eq cx)
       mov cx,y
    end if
    rol ecx,16
    mov cx,h

    if WinClass#.WorkTransp==1
      WinClass#.WorkTransp=40000000h
    end if

    mov edx,3000000h+WinClass#.WorkTransp+WinClass#.Color
    mov esi,0x80333333
    mov edi,WinClass#.Name
    xor eax,eax
    int 40h
}

macro DrawPix x,y,color     ;void
{
    mov ebx,x
    mov ecx,y
    mov edx,color
    mov eax,1
    int 40h
}

macro GetKeyPressed   ;eax=1-пусто либо al=0, ah=код нажатой клавиши либо
{		      ;al=2, ah=сканкод нажатой ГОРЯЧЕЙ клавиши,HI eax=сост упр клавиш
    mov eax,2
    int 40h
}


macro OutText x,y,Text,colorFon    ;void
{
  if colorFon eq
    mov bx,x
    rol ebx,16
    mov bx,y
    mov ecx,80000000h
    mov edx,Text
    mov eax,4
    int 40h
  else
    mov bx,x
    rol ebx,16
    mov bx,y
    mov ecx,0C0000000h
    mov edx,Text
    mov edi,colorFon
    mov eax,4
    int 40h
  end if
}

macro Sleep time      ;void
{
    mov ebx,time
    mov eax,5
    int 40h
}


macro DrawImage x,y,w,h,Image	    ;void
{
    mov ebx,Image
    mov cx,w
    rol ecx,16
    mov cx,h
    mov dx,x
    rol edx,16
    mov dx,y
    mov eax,7
    int 40h
}

macro DrawButton x,y,w,h,ID,color,Caption     ;void
{
    if ID<1000000h
	mov bx,x
	rol ebx,16
	mov bx,w
	mov cx,y
	rol ecx,16
	mov cx,h
	mov edx,ID
	mov esi,color
	mov eax,8
	int 40h

	sub edx,80000000h
    else
	mov eax,-1
    end if

    if ~(Caption eq)
	mov bx,x
	add bx,5
	rol ebx,16
	mov bx,y
	add bx,5
	mov ecx,80000000h
	mov edx,Caption
	mov eax,4
	int 40h
    end if
}

macro DeleteButton ID	     ;void
{
    if ID<1000000h
	mov eax,8
	mov edx,80000000h
	add edx,ID
	int 40h
    ;else

    end if

}

macro GetThreadInfo Buffer,N ;1KB  eax=max номер слота потока
{
    mov ebx,Buffer
    if N eq
      mov ecx,-1
    else
      mov ecx,N
    end if
    mov eax,9
    int 40h
}
; +34 = X window
; +38 = Y window
; +42 = W window
; +46 = H window


macro WaitMessage	 ;eax=event(1=redraw,2=key,3=button,5=redrawBackgroud,
{			 ;6=mouse,7=IPC,8=ethernet,9=debug,16..31=IRQ0..IRQ15)
    mov eax,10
    int 40h
}

macro ReadMessage      ;eax=0-пусто иначе см WaitMessage
{
   mov eax,11
   int 40h
}

macro RedrawWinBeg    ;void
{
    mov eax,12
    mov ebx,1
    int 40h
}

macro RedrawWinEnd	;void
{
    mov eax,12
    mov ebx,2
    int 40h
}

macro DrawRect x,y,w,h,color	  ;void
{
   mov bx,x
   rol ebx,16
   mov bx,w
   mov cx,y
   rol ecx,16
   mov cx,h
   mov edx,color
   mov eax,13
   int 40h
}

macro DrawClearRect x,y,w,h,color ;(рамки. Рисует просто рамку. Без закрашивания внутри)
{
    mov eax,38
    mov bx,x
    rol ebx,16
    mov bx,x
    add bx,w
    mov cx,y
    rol ecx,16
    mov cx,y
    mov edx,color
    int 40h

    mov eax,38
    mov bx,x
    add bx,w
    rol ebx,16
    mov bx,x
    add bx,w
    mov cx,y
    rol ecx,16
    mov cx,y
    add cx,h
    mov edx,color
    int 40h

    mov eax,38
    mov bx,x
    rol ebx,16
    mov bx,x
    add bx,w
    mov cx,y
    add cx,h
    rol ecx,16
    mov cx,y
    add cx,h
    mov edx,color
    int 40h

    mov eax,38
    mov bx,x
    rol ebx,16
    mov bx,x
    mov cx,y
    rol ecx,16
    mov cx,y
    add cx,h
    mov edx,color
    int 40h
}



macro GetScreenSize	     ;eax=[xsize]*0x10000+[ysize]. Значения уже увеличены на 1,т.е.
{			     ;xsize и ysize могут быть равны 1024 и 768 (например)
    mov eax,14
    int 40h
    add eax,00010001h
}

macro RedrawDesktop
{
    mov eax,15
    mov ebx,3
    int 40h
}

macro GetButtonPressed	   ;eax=1(пусто) иначе al=0(LButton)или код другой кнопки мыши,
{			   ; HI eax и ah содержат номер кнопки
   mov eax,17
   int 40h
}

macro Minimize		  ;void
{
    mov eax,18
    mov ebx,10
    int 40h
}

macro SetMousePos x,y
{
     mov ecx,4
     mov dx,x
     rol edx,16
     mov dx,y
     mov eax,18
     mov ebx,19
     int 40h
}

macro WaitMessageT time
{
     mov ebx,time
     mov eax,23
     int 40h
}

macro SetDefaultFolder folder
{
    mov ecx,folder
    mov eax,30
    mov ebx,1
    int 40h
}

macro GetDefaultFolder folderBuf,sizeBuf
{
    mov ecx,folderBuf
    moc edx,sizeBuf
    mov eax,30
    mov ebx,2
    int 40h
}

macro GetMousePos
{
     mov eax,37
     xor ebx,ebx
     int 40h
}

macro GetMouseWinPos
{
     mov eax,37
     mov ebx,1
     int 40h
}


macro GetMouseKey
{
    mov eax,37
    mov ebx,2
    int 40h
}

macro LoadCursorF path
{
    mov ecx,path
    mov eax,37
    mov ebx,4
    mov dx,0
    int 40h
}

macro LoadCursorM cur
{
    mov ecx,cur
    mov eax,37
    mov ebx,4
    mov dx,1
    int 40h
}

macro LoadCursorStruct cur,x,y
{
 ;   mov edx,x*1000000h+y*10000h+0002
    mov edx,x
    shl edx,24
    xor ecx,ecx
    mov cx,y
    shl ecx,16
    add edx,ecx
    add edx,2
    mov ecx,path
    mov eax,37
    mov ebx,4
    int 40h
}

macro SetCursor Handle
{
    mov ecx,Handle
    mov eax,37
    mov ebx,5
    int 40h
}


macro DrawLine x1,y1,x2,y2,color
{
    mov bx,x1
    rol ebx,16
    mov bx,x2
    mov cx,y1
    rol ecx,16
    mov cx,y2
    mov edx,color
    mov eax,38
    int 40h
}

macro SetMaskMessage Mask
{
    mov ebx,Mask
    mov eax,40
    int 40h
}

macro DrawInt Int,Leng,x,y,color
{
    xor ecx,ecx
    mov cx,Leng
    shl ecx,16
    add ebx,ecx

    mov ecx,Int
    mov dx,x
    rol edx,16
    mov dx,y
    mov esi,color
    mov eax,47
    mov ebx,080100000h	 ;+Leng*10000h+0000
    int 40h
}

macro GetSkinH
{
    mov eax,48
    mov ebx,4
    int 40h
}

macro CreateThread start,stack
{
   mov ecx,start
   mov edx,stack
   mov eax,51
   mov ebx,1
   int 40h
}



macro SetKeyboardMode mode
{
   mov ecx,mode
   mov eax,66
   mov ebx,2
   int 40h
}

macro ResizeWindow x,y,w,h
{
   mov ebx,x
   mov ecx,y
   mov edx,w
   mov esi,h
   mov eax,67
   int 40h
}

macro InitMemory	;eax=0(неуспех) или размер кучи
{
    mov eax,68
    mov ebx,11
    int 40h
}

macro GetMemory size  ;eax=Pointer
{
    mov ecx,size
    mov ebx,12
    mov eax,68
    int 40h
}

macro FreeMemory pointer   ;eax = 0/1 = false/true
{
    mov ecx,pointer
    mov ebx,13
    mov eax,68
    int 40h
}

macro ReallocMemory pointer,newsize
{
    mov edx,pointer
    mov ecx,newsize
    mov ebx,20
    mov eax,68h
    int 40h
}

macro ReadFile Struc,path,n,Buffer,posL;,posH
{
    mov dword [Struc],0
    mov dword [Struc+4],posL
    mov dword [Struc+8],0;posH
    mov dword [Struc+12],n
    mov dword [Struc+16],Buffer
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    mov ebx,Struc
    mov eax,70
    int 40h
}

macro CreateFile Struc,path,n,Buffer
{
    mov dword [Struc],2
    mov dword [Struc+4],0
    mov dword [Struc+8],0
    mov dword [Struc+12],n
    mov dword [Struc+16],Buffer
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    mov ebx,Struc
    mov eax,70
    int 40h
}

macro WriteFile Struc,path,n,Buffer,posL;,posH
{
    mov dword [Struc],3
    mov dword [Struc+4],posL
    mov dword [Struc+8],0;posH
    mov dword [Struc+12],n
    mov dword [Struc+16],Buffer
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    mov ebx,Struc
    mov eax,70
    int 40h

}

macro SetFileSize Struc,path,sizeL;,sizeH
{
    mov dword [Struc],4
    mov dword [Struc+4],sizeL
    mov dword [Struc+8],0;sizeH
    mov dword [Struc+12],0
    mov dword [Struc+16],0
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    mov ebx,Struc
    mov eax,70
    int 40h
}

macro GetFileInfo Struc,path,BufferInfo ;(40байт)
{
    mov dword [Struc],5
    mov dword [Struc+4],0
    mov dword [Struc+8],0
    mov dword [Struc+12],0
    mov dword [Struc+16],BufferInfo
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    mov ebx,Struc
    mov eax,70
    int 40h
}

macro RunProgram Struc,path,params
{
    mov eax,70
    mov ebx,Struc
    mov dword [Struc],7
    mov dword [Struc+4],0
    mov dword [Struc+8],params
    mov dword [Struc+12],0
    mov dword [Struc+16],0
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    int 40h
}

macro Remove Struc,path
{
    mov eax,70
    mov ebx,Struc
    mov dword [Struc],8
    mov dword [Struc+4],0
    mov dword [Struc+8],0
    mov dword [Struc+12],0
    mov dword [Struc+16],0
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    int 40h
}

macro CreateFolder Struc,path
{
    mov eax,70
    mov ebx,Struc
    mov dword [Struc],9
    mov dword [Struc+4],0
    mov dword [Struc+8],0
    mov dword [Struc+12],0
    mov dword [Struc+16],0
    mov byte [Struc+20],0
    mov dword [Struc+21],path
    int 40h
}

macro SetWindowCaption caption
{
    mov eax,71
    mov ebx,1
    mov ecx,name
    int 40h
}

macro ExitThread
{
    mov eax,-1
    int 40h
}

ExitProcess fix ExitThread