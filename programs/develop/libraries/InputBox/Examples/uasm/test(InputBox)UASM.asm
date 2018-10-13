.586
.Model Flat, StdCall
Option CaseMap: None

Public @Main

FALSE = 0
TRUE  = Not FALSE

CStr Macro QuotedText:VarArg
Local LocalText
.Const
LocalText db QuotedText,0
.Code
ExitM <Offset LocalText>
EndM

RepArg Macro Arg
  Local NewStr
    Quot SubStr <Arg>, 1, 1
  IfIdn Quot, <">
    .Data
      NewStr db Arg,0
    .Code
    ExitM <ADDR NewStr>
  Else
    ExitM <Arg>
  EndIf
EndM

@ Macro Function:Req, Args:VarArg
  Arg equ <Invoke Function>
  For Var, <Args>
    Arg CatStr Arg, <, RepArg(Var)>
  EndM
  Arg
  ExitM <eax>
EndM

WINDOW_BORDER_SIZE = 5

; Events
REDRAW_EVENT      = 1
KEY_EVENT         = 2
BUTTON_EVENT      = 3
BACKGROUND_EVENT  = 5
MOUSE_EVENT       = 6
IPC_EVENT         = 7
NETWORK_EVENT     = 8
DEBUG_EVENT       = 9

; Window Style Constants
WS_SKINNED_FIXED  = 4000000H
WS_COORD_CLIENT   = 20000000H
WS_CAPTION        = 10000000H

; Caption Style Constants
CS_MOVABLE        = 0

; Event Mask Constants
EM_REDRAW         = 001H
EM_KEY            = 002H
EM_BUTTON         = 004H
EM_BACKGROUND     = 010H
EM_MOUSE          = 020H
EM_IPC            = 040H
EM_NETWORK        = 080H
EM_DEBUG          = 100H

; SharedMemoryOpen open\access flags
SHM_OPEN          = 000H
SHM_OPEN_ALWAYS   = 004H
SHM_CREATE        = 008H
SHM_READ          = 000H
SHM_WRITE         = 001H

; Draw zero terminated string for DrawText
DT_ZSTRING        = 080000000H

TRect Struct
  Left   SDword ?
  Top    SDword ?
  Right  SDword ?
  Bottom SDword ?
TRect EndS

TBox Struct
  Left  SDword ?
  Top   SDword ?
  SizeX Dword  ?
  SizeY Dword  ?
TBox EndS

TSize Struct
  SizeY Word ?
  SizeX Word ?
TSize EndS

TKeyboardInput Struct
  Flag Byte ?
  Code Byte ?
  Union
    Scan    Byte ?
    Control Word ?
  EndS
TKeyboardInput EndS

TButtonInput Struct
  MouseButton Byte ?
  ID          Word ?
  HiID        Byte ?
TButtonInput EndS

; Button identifiers
BUTTON_CLOSE       = 1
SET_LEFT_BUTTON    = 1111
SET_TOP_BUTTON     = 2222
SET_CAPTION_BUTTON = 3333

; Flags = [mouse|screen|parent][number|string]
IBF_STRING = 0      ; в буфер будет записана строка
IBF_NUMBER = 1      ; в буфер будет записано число
IBF_MOUSE_REL = 0   ; относительно положения указателя мыши
IBF_SCREEN_REL = 8  ; относительно экрана
IBF_PARENT_REL = 16 ; относительно родительского окна

; Errors
IBE_NO_ERROR = 0        ; успешно, нет ошибки
IBE_NUMBER_OVERFLOW = 1 ; переполнение при вводе числа
IBE_RESULT_TOO_LONG = 2 ; результат не умещается в буфер

GetButton          Proto
GetKey             Proto
WaitEvent          Proto
BeginDraw          Proto
EndDraw            Proto
DrawWindow         Proto Left:SDword, Top:SDword, Right:SDword, Bottom:SDword, Caption:Ptr, BackColor:Dword, Style:Dword, CapStyle:Dword
ThreadTerminate    Proto
LoadLibrary        Proto Path:Ptr
GetProcAddress     Proto hLib:Ptr, ProcName:Ptr
SetEventMask       Proto EventMask:Dword
GetScreenSize      Proto
GetSkinHeight      Proto
HeapAllocate       Proto Bytes:Dword
DrawButton         Proto Left:SDword, Top:SDword, Right:SDword, Bottom:SDword, BackColor:Dword, Style:Dword, ID:Dword
GetStandardColors  Proto ColorTable:Ptr, TableSize:Dword
DrawText           Proto X:SDword, Y:SDword, Text:Ptr, ForeColor:Dword, BackColor:Dword, Flags:Dword, Count:Dword
SetWindowCaption   Proto Caption:Ptr
RunFile            Proto Path:Ptr, CmdLine:Ptr
SetWindowPos       Proto Left:SDword, Top:SDword, Right:SDword, Bottom:SDword

InputBox_proto TypeDef Proto Buffer:Ptr, Caption:Ptr, Prompt:Ptr, Default:Ptr, Flags:Dword, BufferSize:Dword, RedrawProc:Ptr
InputBox_proc  TypeDef Ptr InputBox_proto

; -------------------------------------------------------- ;
.Data? ; ------------------------------------------------- ;
; -------------------------------------------------------- ;
Extern AppParams: Byte
Extern AppPath:   Byte

Window TBox <>
Screen TSize <>

Key TKeyboardInput <>
Button TButtonInput <>

Buf Byte 100 Dup (?)
NewPos Dword ?

InputBoxLib Dword ?
InputBox InputBox_proc ?

; -------------------------------------------------------- ;
.Code ; -------------------------------------------------- ;
; -------------------------------------------------------- ;
OnRedraw Proc
  Invoke BeginDraw
  Invoke DrawWindow, Window.Left, Window.Top, Window.SizeX, Window.SizeY, CStr("Test InputBox"), 0FFFFFFH, WS_SKINNED_FIXED + WS_COORD_CLIENT + WS_CAPTION, CS_MOVABLE
  
  Invoke DrawButton, 8, 60, 81, 33, 00FF0000H, 0, SET_LEFT_BUTTON
  Invoke DrawButton, 104, 60, 81, 33, 0000FF00H, 0, SET_TOP_BUTTON
  Invoke DrawButton, 200, 60, 93, 33, 00FFFF00H, 0, SET_CAPTION_BUTTON
  
  Invoke DrawText, 28, 72,  CStr("Set Left"), 0, 00FFFFFFH, DT_ZSTRING, 0
  Invoke DrawText, 122, 72, CStr("Set Top"), 0, 00FFFFFFH, DT_ZSTRING, 0
  Invoke DrawText, 214, 72, CStr("Set Caption"), 0, 00FFFFFFH, DT_ZSTRING, 0
  
  Invoke EndDraw
  ret
OnRedraw Endp
; -------------------------------------------------------- ;
@Main:
  mov InputBoxLib,  @(LoadLibrary, CStr("/sys/lib/InputBox.obj"))
  mov InputBox, @(GetProcAddress, InputBoxLib, CStr("InputBox"))  

  mov Window.Left, 100
  mov Window.Top, 70
  mov Window.SizeX, 315
  mov Window.SizeY, 200

  Invoke SetEventMask, EM_REDRAW + EM_KEY + EM_BUTTON + EM_MOUSE

  .Repeat
    Invoke WaitEvent
    .If eax == REDRAW_EVENT
      Invoke  OnRedraw
    .ElseIf eax == KEY_EVENT
      Invoke GetKey
    .ElseIf eax == BUTTON_EVENT
      mov Button, @(GetButton)
      .If Button.ID == BUTTON_CLOSE
        Invoke ThreadTerminate
      .ElseIf Button.ID == SET_LEFT_BUTTON
        Invoke InputBox, Offset NewPos, CStr("Input"), CStr("Enter new left"), CStr("100"), IBF_NUMBER + IBF_PARENT_REL, SizeOf(NewPos), Offset OnRedraw
        .If eax != 0
          Invoke RunFile, CStr("/sys/@notify"), CStr("Error")
        .EndIf
        Invoke SetWindowPos, NewPos, -1, -1, -1
      .ElseIf Button.ID == SET_TOP_BUTTON
        Invoke InputBox, Offset NewPos, CStr("Input"), CStr("Enter new top"), CStr("70"), IBF_NUMBER + IBF_SCREEN_REL, SizeOf(NewPos), Offset OnRedraw
        .If eax != 0
          Invoke RunFile, CStr("/sys/@notify"), CStr("Error")
        .EndIf
        Invoke SetWindowPos, -1, NewPos, -1, -1
      .ElseIf Button.ID == SET_CAPTION_BUTTON
        Invoke InputBox, Offset Buf, CStr("Input"), CStr("Enter new caption"), CStr("Test InputBox"), IBF_STRING + IBF_MOUSE_REL, SizeOf(Buf), Offset OnRedraw
        Invoke SetWindowCaption, Offset Buf
      .EndIf
    .ElseIf eax == MOUSE_EVENT
    .EndIf
  .Until 0
END
