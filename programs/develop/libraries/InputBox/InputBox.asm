.586
.Model Flat, StdCall
Option CaseMap: None

Public EXPORTS

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

TMem Struct
  State   Dword ?
  Caption Dword ?
  Prompt  Dword ?
  Default Dword ?
  Flags   Dword ?
  Error   Dword ?
  Result  Dword ?
TMem EndS

TEditBox Struct
  x_size             Dword  ?
  left               SDword ?
  top                SDword ?
  color              Dword  ?
  shift_color        Dword  ?
  focus_border_color Dword  ?
  blur_border_color  Dword  ?
  text_color         Dword  ?
  max                Dword  ?
  text               Dword  ?
  mouse_variable     Dword  ?
  flags              Dword  ?
  count              Dword  ?
  pos                Dword  ?
  offs               Dword  ?
  cl_curs_x          SWord  ?
  cl_curs_y          SWord  ?
  shift              Word   ?
  shift_old          Word   ?
  height             Dword  ?
  char_width         Dword  ?
TEditBox EndS

TStandartColors Struct
  frames           Dword ?
  grab             Dword ?
  grab_button      Dword ?
  grab_button_text Dword ?
  grab_text        Dword ?
  work             Dword ?
  work_button      Dword ?
  work_button_text Dword ?
  work_text        Dword ?
  work_graph       Dword ?
TStandartColors EndS

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

TLongPoint Struct
  Y SDword ?
  X SDword ?
TLongPoint EndS

TPoint Struct
  Y SWord ?
  X SWord ?
TPoint EndS

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

TThreadInfo Struct
  CpuUsage     Dword ?
  WinStackPos  Word ?
  reserved0    Word ?
  reserved1    Word ?
  ThreadName   Byte 11 Dup (?)
  reserved2    Byte ?
  MemAddress   Dword ?
  MemUsage     Dword ?
  Identifier   Dword ?
  Window       TBox <>
  ThreadState  Word ?
  reserved3    Word ?
  Client       TBox <>
  WindowState  Byte ?
  EventMask    Dword ?
  KeyboardMode Byte ?
  reserved4    Byte 948 Dup (?)
TThreadInfo EndS

IfDef     LANG_IT
  szCancel equ <CStr("Annulla ")>
ElseIfDef LANG_SP
  szCancel equ <CStr("Cancelar")>
ElseIfDef LANG_RU
  szCancel equ <CStr(" Отмена ")>
ElseIfDef LANG_EN
  szCancel equ <CStr(" Cancel ")>
Else
  .Err LANG_??
EndIf

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

; Draw zero terminated string for DrawText
DT_ZSTRING        = 080000000H

; Charset specifiers for DrawText
DT_CP866_8X16     = 10000000H
DT_CP866_6X9      = 00000000H

; Button identifiers
BUTTON_CLOSE      = 1
BUTTON_OK         = 10
BUTTON_CANCEL     = 20

KEY_CODE_ENTER   = 13
KEY_CODE_ESCAPE  = 27

; Flags = [mouse|screen|parent][number|string]
IBF_STRING = 0      ; в буфер будет записана строка         ; string will be written to the buffer
IBF_NUMBER = 1      ; в буфер будет записано число          ; number will be written to the buffer
IBF_MOUSE_REL = 0   ; относительно положения указателя мыши ; relative to the mouse pointer
IBF_SCREEN_REL = 8  ; относительно экрана                   ; relative to the screen
IBF_PARENT_REL = 16 ; относительно родительского окна       ; relative to the parent window

; Errors
IBE_NO_ERROR = 0        ; успешно, нет ошибки            ; success
IBE_NUMBER_OVERFLOW = 1 ; переполнение при вводе числа   ; number greater than 0xFFFFFFFFFFFFFFFF
IBE_RESULT_TOO_LONG = 2 ; результат не умещается в буфер ; result does not fit into buffer

EDIT1_TEXT_BUFFER_SIZE = 1000

EditBoxDraw_proto     TypeDef Proto :Ptr
EditBoxKey_proto      TypeDef Proto :Ptr
EditBoxMouse_proto    TypeDef Proto :Ptr
EditBoxSetText_proto  TypeDef Proto :Ptr, :Ptr

EditBoxDraw_proc      TypeDef Ptr EditBoxDraw_proto
EditBoxKey_proc       TypeDef Ptr EditBoxKey_proto
EditBoxMouse_proc     TypeDef Ptr EditBoxMouse_proto
EditBoxSetText_proc   TypeDef Ptr EditBoxSetText_proto

InputBox Proto Buffer:Ptr, Caption:Ptr, Prompt:Ptr, Default:Ptr, Flags:Dword, BufferSize:Dword, RedrawProc:Ptr

; -------------------------------------------------------- ;
.Data; --------------------------------------------------- ;
; -------------------------------------------------------- ;
Edit1 TEditBox <\
      0,\
      5,\
      26,\
      00FFFFFFH,\
      00A4C4E4H,\
      0,\
      0,\
      DT_CP866_8X16,\
      EDIT1_TEXT_BUFFER_SIZE,\
      Offset Edit1TextBuffer,\
      Offset Edit1MouseVariable,\
      4002H,\
      0,\
      0,\
      0,\
      0,\
      0,\
      0,\
      0\
>

Mem Dword Offset InpuBox_Mem

FontHeight Dword 9

Initialized db FALSE

; -------------------------------------------------------- ;
.Data? ; ------------------------------------------------- ;
; -------------------------------------------------------- ;
BoxLib         Dword ?

EditBoxDraw     EditBoxDraw_proc    ?
EditBoxKey      EditBoxKey_proc     ?
EditBoxMouse    EditBoxMouse_proc   ?
EditBoxSetText  EditBoxSetText_proc ?

SC TStandartColors <>
Window TBox <>
Screen TSize <>

Key TKeyboardInput <>
Button TButtonInput <>

Align 4
ThreadInfo TThreadInfo <>

Align 4
ParentThreadInfo TThreadInfo <>

Align 4
Margin Dword ?
PromptPos TLongPoint <>
ButtonCancel TBox <>
ButtonOK TBox <>

WinCaption Dword ?
TextPrompt Dword ?
TextDefault Dword ?
TextOK Dword ?
TextCancel Dword ?

TextOKLeft     Dword ?
TextOKTop      Dword ?
TextCancelLeft Dword ?
TextCancelTop  Dword ?

TextFlags Dword ?

FontWidth Dword ?

MousePos TPoint <>

ParentID Dword ?
ParentSlot Dword ?

Edit1MouseVariable Dword ?

Align 4
Edit1TextBuffer Byte EDIT1_TEXT_BUFFER_SIZE + 2 Dup (?) ; buffer for Edit1.text

Align 4
InpuBox_Mem Byte SizeOf(TMem) + SizeOf(Qword) Dup (?)

Align 4
Byte 256 Dup (?)
InputBox_Thread_StackTop Label Dword
; -------------------------------------------------------- ;
.Code ; -------------------------------------------------- ;
; -------------------------------------------------------- ;
; convert a string, which represents an signed integer to a number
; if a number too large(greater than 0xFFFFFFFFFFFFFFFF) then return 0xFFFFFFFFFFFFFFFF and the CARRY flag
a2i Proc Uses ebx esi edi S:Ptr
Local LoDword:Dword, HiDword:Dword, Tmp:Dword
   mov    edi, 10
   mov    esi, S
   xor    eax, eax
   xor    edx, edx
   xor    ebx, ebx
   mov    LoDword, eax
   mov    HiDword, eax
   movzx  ecx, Byte Ptr [esi]
   inc    esi
   cmp    cl, '-'
   jne    @no_sign
   dec    ebx
@next:
   movzx  ecx, Byte Ptr [esi]
   inc    esi
@no_sign:
   sub    cl, 48
   cmp    cl, 9
   jnbe   @done
   mov    eax, HiDword
   mul    edi           ; multiply by 10
   jc     @overflow     ; CF flag are set if edx is not 0
   mov    Tmp, eax
   mov    eax, LoDword
   mul    edi           ; multiply by 10
   add    edx, Tmp
   jc     @overflow
   add    eax, ecx
   adc    edx, 0
   mov    LoDword, eax
   mov    HiDword, edx
   jnc    @next
@overflow:
   mov    eax, -1       ; RETURN 0xFFFFFFFFFFFFFFFF
   mov    edx, eax
   jmp    @exit
@done:
   add    eax, ebx
   adc    edx, ebx
   xor    eax, ebx
   xor    edx, ebx
@exit:
  ret
a2i EndP
; -------------------------------------------------------- ;
Align 4
MemCpy Proc Uses esi edi Src:Ptr, Dst:Ptr, Count:Dword
  mov    edi, Dst
  mov    esi, Src
  mov    ecx, Count
  mov    edx, ecx
  shr    ecx, 2
  and    edx, 3
  rep movsd
  mov    ecx, edx
  rep movsb
  ret
MemCpy EndP
; -------------------------------------------------------- ;
Align 4
StrLen Proc Uses edi Src:Ptr
  mov    edi, Src
  mov    ecx, 0FFFFFFFFH
  xor    al, al
  repne scasb
  mov    eax, 0FFFFFFFEH
  sub    eax, ecx
  ret
StrLen EndP
; -------------------------------------------------------- ;
Align 4
StrCpy Proc Uses esi edi Src:Ptr, Dst:Ptr
  mov    edi, Src
  mov    ecx, 0FFFFFFFFH
  mov    esi, edi
  xor    eax, eax
  repne scasb
  not    ecx
  mov    edi, Dst
  mov    edx, ecx
  shr    ecx, 2
  and    edx, 3
  rep movsd
  mov    ecx, edx
  rep movsb
  mov    eax, edi
  ret
StrCpy EndP
; -------------------------------------------------------- ;
Align 4
GetThreadInfo Proc Uses ebx Slot:Dword, Buffer: Ptr TThreadInfo
  mov    eax, 9
  mov    ebx, Buffer
  mov    ecx, Slot
  int    64
  ret
GetThreadInfo EndP
; -------------------------------------------------------- ;
Align 4
ActivateWindow Proc Uses ebx Slot:Dword
  mov    eax, 18
  mov    ebx, 3
  mov    ecx, Slot
  int    64
  ret
ActivateWindow  EndP
; -------------------------------------------------------- ;
Align 4
GetSlotById Proc Uses ebx ID:Dword
  mov    eax, 18
  mov    ebx, 21
  mov    ecx, ID
  int    64
  ret
GetSlotById  EndP
; -------------------------------------------------------- ;
Align 4
SetEventMask Proc Uses ebx EventMask:Dword
  mov    eax, 40
  mov    ebx, EventMask
  int    64
  ret
SetEventMask EndP
; -------------------------------------------------------- ;
Align 4
WaitEvent Proc
  mov    eax, 10
  int    64
  ret
WaitEvent EndP
; -------------------------------------------------------- ;
Align 4
GetButton Proc
  mov    eax, 17
  int    64
  ret
GetButton EndP
; -------------------------------------------------------- ;
Align 4
GetKey Proc
  mov    eax, 2
  int    64
  ret
GetKey EndP
; -------------------------------------------------------- ;
Align 4
BeginDraw Proc
  push   ebx
  mov    eax, 12
  mov    ebx, 1
  int    64
  pop    ebx
  ret
BeginDraw EndP
; -------------------------------------------------------- ;
Align 4
EndDraw Proc
  push   ebx
  mov    eax, 12
  mov    ebx, 2
  int    64
  pop    ebx
  ret
EndDraw EndP
; -------------------------------------------------------- ;
Align 4
DrawWindow Proc Left:SDword, Top:SDword, Right:SDword, Bottom:SDword, Caption:Ptr, BackColor:Dword, Style:Dword, CapStyle:Dword
  push   ebx
  push   edi
  push   esi
  xor    eax, eax
  mov    ebx, Left
  mov    ecx, Top
  shl    ebx, 16
  shl    ecx, 16
  or     ebx, Right
  or     ecx, Bottom
  mov    edx, Style
  or     edx, BackColor
  mov    edi, Caption
  mov    esi, CapStyle
  int    64
  pop    esi
  pop    edi
  pop    ebx
  ret
DrawWindow EndP
; -------------------------------------------------------- ;
Align 4
ThreadTerminate Proc
  mov    eax, 0FFFFFFFFH
  int    64
  ret
ThreadTerminate EndP
; -------------------------------------------------------- ;
Align 4
LoadLibrary Proc Path:Ptr
  push   ebx
  mov    eax, 68
  mov    ebx, 19
  mov    ecx, Path
  int    64
  pop    ebx
  ret
LoadLibrary EndP
; -------------------------------------------------------- ;
Align 4
GetProcAddress Proc hLib:Ptr, ProcName:Ptr
  push   esi
  push   edi
  push   ebx
  mov    edx, hLib
  xor    eax, eax
  test   edx, edx
  jz     @end
  mov    edi, ProcName
  mov    ecx, 0FFFFFFFFH
  repne scasb
  mov    ebx, ecx
  not    ebx
@next:
  mov    esi, [edx]
  test   esi, esi
  jz     @end
  mov    ecx, ebx
  mov    edi, ProcName
  add    edx, 8
  repe cmpsb
  jne    @next
  mov    eax, [edx - 4]
@end:
  pop    ebx
  pop    edi
  pop    esi
  ret
GetProcAddress EndP
; -------------------------------------------------------- ;
Align 4
GetScreenSize Proc
  push   ebx
  mov    eax, 61
  mov    ebx, 1
  int    64
  pop    ebx
  ret
GetScreenSize EndP
; -------------------------------------------------------- ;
Align 4
GetSkinHeight Proc
  push   ebx
  mov    eax, 48
  mov    ebx, 4
  int    64
  pop    ebx
  ret
GetSkinHeight EndP
; -------------------------------------------------------- ;
Align 4
GetFontHeight Proc
  push   ebx
  mov    eax, 48
  mov    ebx, 11
  int    64
  pop    ebx
  ret
GetFontHeight EndP
; -------------------------------------------------------- ;
Align 4
DrawButton Proc Left:SDword, Top:SDword, Right:SDword, Bottom:SDword, BackColor:Dword, Style:Dword, ID:Dword
  push   ebx
  push   esi
  mov    eax, 8
  mov    ebx, Left
  mov    ecx, Top
  shl    ebx, 16
  shl    ecx, 16
  or     ebx, Right
  or     ecx, Bottom
  mov    edx, ID
  or     edx, Style
  mov    esi, BackColor
  int    64
  pop    esi
  pop    ebx
  ret
DrawButton EndP
; -------------------------------------------------------- ;
Align 4
GetStandardColors Proc ColorTable:Ptr, TableSize:Dword
  push   ebx
  mov    eax, 48
  mov    ebx, 3
  mov    ecx, ColorTable
  mov    edx, TableSize
  int    64
  pop    ebx
  ret
GetStandardColors EndP
; -------------------------------------------------------- ;
Align 4
DrawText Proc X:SDword, Y:SDword, Text:Ptr, ForeColor:Dword, BackColor:Dword, Flags:Dword, Count:Dword
  push   ebx
  push   edi
  push   esi
  mov    eax, 4
  mov    ebx, X
  shl    ebx, 16
  or     ebx, Y
  mov    ecx, Flags
  or     ecx, ForeColor
  mov    edx, Text
  mov    edi, BackColor
  mov    esi, Count
  int    64
  pop    esi
  pop    edi
  pop    ebx
  ret
DrawText EndP
; -------------------------------------------------------- ;
Align 4
GetMousePos Proc
  push   ebx
  mov    eax, 37
  mov    ebx, 0
  int    64
  pop    ebx
  ret
GetMousePos EndP
; -------------------------------------------------------- ;
Align 4
ThreadCreate Proc Uses ebx Entry:Ptr, Stack:Ptr
  mov    eax, 51
  mov    ebx, 1
  mov    ecx, Entry
  mov    edx, Stack
  int    64
  ret
ThreadCreate EndP
; -------------------------------------------------------- ;
Align 4
InputBox Proc Uses ebx esi edi Buffer:Ptr, Caption:Ptr, Prompt:Ptr, Default:Ptr, Flags:Dword, BufferSize:Dword, RedrawProc:Ptr
Local InputBoxID:Dword, InputBoxSlot:Dword, SharedMemorySize:Dword
Local EventMask:Dword, Result:Dword
  Invoke GetThreadInfo, 0FFFFFFFFH, Offset ParentThreadInfo

  mov eax, ParentThreadInfo.Identifier
  mov ParentID, eax
  mov ParentSlot, @(GetSlotById, ParentID)

  mov ebx, Mem
  mov [ebx][TMem.State], FALSE

  mov eax, Caption
  mov [ebx][TMem.Caption], eax

  mov eax, Prompt
  mov [ebx][TMem.Prompt], eax

  mov eax, Default
  mov [ebx][TMem.Default], eax

  mov eax, Flags
  mov [ebx][TMem.Flags], eax

  mov EventMask, @(SetEventMask, EM_REDRAW)

  mov InputBoxID, @(ThreadCreate, Offset InputBox_Thread, Offset InputBox_Thread_StackTop)
  mov InputBoxSlot, @(GetSlotById, InputBoxID)
  .Repeat
    Invoke WaitEvent
    Invoke ActivateWindow, InputBoxSlot
    .If RedrawProc != 0
      call RedrawProc
    .EndIf
  .Until Dword Ptr [ebx][TMem.State] == TRUE

  .If Flags & IBF_NUMBER
    .If [ebx][TMem.Error] != IBE_NUMBER_OVERFLOW
      mov edi, [ebx][TMem.Result]
      add edi, SizeOf(Qword) - 1
      mov edx, edi
      mov ecx, SizeOf(Qword)
      std
      xor al, al
      repe scasb
      cld
      lea eax, [ecx + 1]
      .If eax > BufferSize
        mov [ebx][TMem.Error], IBE_RESULT_TOO_LONG
      .EndIf
      Invoke MemCpy, [ebx][TMem.Result], Buffer, BufferSize
    .EndIf
  .Else
    .If @(StrLen, [ebx][TMem.Result]) <= BufferSize
      Invoke StrCpy, [ebx][TMem.Result], Buffer
    .Else
      mov [ebx][TMem.Error], IBE_RESULT_TOO_LONG
      mov eax, BufferSize
      dec eax
      mov edi, Buffer
      add edi, eax
      mov Byte Ptr [edi], 0
      Invoke MemCpy, [ebx][TMem.Result], Buffer, eax
    .EndIf
  .EndIf

  mov eax, [ebx][TMem.Error]
  mov Result, eax

  Invoke SetEventMask, EventMask
  Invoke ActivateWindow, @(GetSlotById, ParentThreadInfo.Identifier)

  mov eax, Result
  ret
InputBox EndP
; -------------------------------------------------------- ;
Align 4
OnRedraw Proc
        Invoke BeginDraw
        Invoke GetStandardColors, Offset SC, SizeOf(SC)
        mov MousePos, @(GetMousePos)
        mov FontHeight, @(GetFontHeight)
        .If FontHeight > 9
          mov FontHeight, 16
        .EndIf
;
        .If FontHeight > 9
          mov eax, DT_ZSTRING + DT_CP866_8X16
          mov Edit1.text_color, DT_CP866_8X16
          mov FontWidth, 8
        .Else
          mov eax, DT_ZSTRING + DT_CP866_6X9
          mov Edit1.text_color, DT_CP866_6X9
          mov FontWidth, 6
        .EndIf
        mov TextFlags, eax
;
        mov ebx, Mem

        mov eax, [ebx][TMem.Caption]
        mov WinCaption, eax
        mov eax, [ebx][TMem.Prompt]
        mov TextPrompt, eax

        mov TextOK, CStr("   OK   ")
        mov TextCancel, szCancel

        mov eax, SC.work_graph
        mov Edit1.focus_border_color, eax
; Margin = FontHeight / 2
        mov eax, FontHeight
        shr eax, 1
        mov Margin, eax
; Window.SizeY =
;   GetSkinHeight + Margin + FontHeight + Margin + 1 + FontHeight + 1 + Margin + FontHeight + Margin + FontWidth + Margin + WINDOW_BORDER_SIZE
        Invoke GetSkinHeight
        add eax, Margin
        add eax, FontHeight
        add eax, Margin
        add eax, 1
        add eax, FontHeight
        add eax, 1
        add eax, Margin
        add eax, FontHeight
        add eax, Margin
        add eax, FontWidth
        add eax, Margin
        add eax, WINDOW_BORDER_SIZE
        mov Window.SizeY, eax
; Window.SizeX = Screen.SizeX / 4
        mov Screen, @(GetScreenSize)
        movzx  eax, Screen.SizeX
        shr eax, 2
        mov Window.SizeX, eax
        .If (ebx != 0) && ([ebx][TMem.Flags] & IBF_SCREEN_REL)
; Window.Left = (Screen.SizeX - Window.SizeX) / 2
          movzx eax, Screen.SizeX
          sub eax, Window.SizeX
          shr eax, 1
          mov Window.Left, eax
; Window.Top = (Screen.SizeY - Window.SizeY) / 2
          movzx eax, Screen.SizeY
          sub eax, Window.SizeX
          shr eax, 1
          mov Window.Top, eax
        .ElseIf (ebx != 0) && ([ebx][TMem.Flags] & IBF_PARENT_REL)
          Invoke GetThreadInfo, ParentSlot, Offset ThreadInfo
; Window.Left = (Parent.SizeX - Window.SizeX) / 2 + Parent.Left
          mov    eax, ThreadInfo.Window.SizeX
          sub    eax, Window.SizeX
          shr    eax, 1
          cdq             ;
          xor    eax, edx ; eax = abs(eax)
          sub    eax, edx ;
          mov    edx, eax
          mov    eax, ThreadInfo.Window.Left
          add    eax, edx
          mov    Window.Left, eax
; Window.Left = (Parent.SizeY - Window.SizeY) / 2 + Parent.Top
          mov    eax, ThreadInfo.Window.SizeY
          sub    eax, Window.SizeY
          shr    eax, 1
          cdq             ;
          xor    eax, edx ; eax = abs(eax)
          sub    eax, edx ;
          mov    edx, eax
          mov    eax, ThreadInfo.Window.Top
          add    eax, edx
          mov    Window.Top, eax
        .Else ;------------------------------------------------
; Window.Left = MousePos.X - Window.SizeX / 2
          movzx eax, MousePos.X
          mov ecx, Window.SizeX
          shr ecx, 1
          sub eax, ecx
          .If SIGN?
            xor eax, eax
          .EndIf
          mov Window.Left, eax
; Window.Top = MousePos.Y - Window.SizeY / 2
          movzx eax, MousePos.Y
          mov ecx, Window.SizeY
          shr ecx, 1
          sub eax, ecx
          .If SIGN?
            xor eax, eax
          .EndIf
          mov Window.Top, eax
        .EndIf ;-----------------------------------------------
; PromptPos.X = Margin
; PromptPos.Y = Margin
; Edit1.left = Margin
        mov eax, Margin
        mov PromptPos.X, eax
        mov PromptPos.Y, eax
        mov Edit1.left, eax
; Edit1.x_size = Window.SizeX - WINDOW_BORDER_SIZE * 2
        mov eax, Window.SizeX
        sub eax, WINDOW_BORDER_SIZE * 2
        sub eax, Margin
        sub eax, Margin
        mov Edit1.x_size, eax
; Edit1.top = Margin + FontHeight + Margin
        mov eax, Margin
        add eax, FontHeight
        add eax, Margin
        mov Edit1.top, eax
; ButtonCancel.SizeX = StrLen(szCancel) * FontWidth + FontWidth
; ButtonOK.SizeX = ButtonCancel.SizeX
        mov eax, FontWidth
        imul eax, 8
        mov ecx, FontWidth
        add eax, ecx
        mov ButtonCancel.SizeX, eax
        mov ButtonOK.SizeX, eax
; ButtonCancel.SizeY = FontHeight + FontWidth
; ButtonOK.SizeY = ButtonCancel.SizeY
        mov eax, FontHeight
        mov ecx, FontWidth
        add eax, ecx
        mov ButtonCancel.SizeY, eax
        mov ButtonOK.SizeY, eax
; ButtonCancel.Top = Edit1.top + 1 + FontHeight + 1 + Margin + Margin
; ButtonOK.Top = ButtonCancel.Top
        mov eax, Edit1.top
        add eax, 1
        add eax, FontHeight
        add eax, 1
        add eax, Margin
        add eax, Margin
        mov ButtonCancel.Top, eax
        mov ButtonOK.Top, eax
; ButtonCancel.Left = Window.SizeX - WINDOW_BORDER_SIZE * 2 - Margin - ButtonCancel.SizeX
; ButtonOK.Left = ButtonCancel.Left - Margin - ButtonOK.SizeX
        mov eax, Window.SizeX
        sub eax, WINDOW_BORDER_SIZE * 2
        sub eax, Margin
        sub eax, ButtonCancel.SizeX
        mov ButtonCancel.Left, eax
        sub eax, Margin
        sub eax, ButtonOK.SizeX
        mov ButtonOK.Left, eax
; TextOKLeft = ButtonOK.Left + FontWidth / 2 + 1
; TextOKTop = ButtonOK.Top + FontWidth / 2 + 1
        mov eax, ButtonOK.Left
        mov edx, ButtonOK.Top
        mov ecx, FontWidth
        shr ecx, 1
        add ecx, 1
        add eax, ecx
        add edx, ecx
        mov TextOKLeft, eax
        mov TextOKTop, edx
; TextCancelLeft = ButtonCancel.Left + FontWidth / 2 + 1
; TextCancelTop = ButtonCancel.Top + FontWidth / 2 + 1
        mov eax, ButtonCancel.Left
        mov edx, ButtonCancel.Top
        mov ecx, FontWidth
        shr ecx, 1
        add ecx, 1
        add eax, ecx
        add edx, ecx
        mov TextCancelLeft, eax
        mov TextCancelTop, edx
;
        Invoke DrawWindow, Window.Left, Window.Top, Window.SizeX, Window.SizeY, WinCaption, SC.work, WS_SKINNED_FIXED + WS_COORD_CLIENT + WS_CAPTION, CS_MOVABLE
        Invoke DrawText, PromptPos.X, PromptPos.Y, TextPrompt, SC.work_text, 0, TextFlags, 0
        Invoke DrawButton, ButtonOK.Left, ButtonOK.Top, ButtonOK.SizeX, ButtonOK.SizeY, SC.work_button, 0, BUTTON_OK
        Invoke DrawText, TextOKLeft, TextOKTop, TextOK, SC.work_button_text, 0, TextFlags, 0
        Invoke DrawButton, ButtonCancel.Left, ButtonCancel.Top, ButtonCancel.SizeX, ButtonCancel.SizeY, SC.work_button, 0, BUTTON_CANCEL
        Invoke DrawText, TextCancelLeft, TextCancelTop, TextCancel, SC.work_button_text, 0, TextFlags, 0
        Invoke EditBoxDraw, Offset Edit1
        Invoke EndDraw
        ret
OnRedraw Endp
; -------------------------------------------------------- ;
Align 4
OK Proc
  mov ebx, Mem
  .If [ebx][TMem.Flags] & IBF_NUMBER

    lea edi, [ebx + SizeOf(TMem)]
    mov [ebx][TMem.Result], edi

    mov [edi], @(a2i, Edit1.text)
    mov [edi + SizeOf(Dword)], edx
    .If CARRY?
      mov [ebx][TMem.Error], IBE_NUMBER_OVERFLOW
    .Else
      mov [ebx][TMem.Error], IBE_NO_ERROR
    .EndIf
  .Else
    mov eax, Edit1.text
    mov [ebx][TMem.Result], eax
    mov [ebx][TMem.Error], IBE_NO_ERROR
  .EndIf
  mov [ebx][TMem.State], TRUE
  Invoke ActivateWindow, ParentSlot
  Invoke ThreadTerminate
  ret
OK Endp
; -------------------------------------------------------- ;
Align 4
Cancel Proc
  mov ebx, Mem
  .If [ebx][TMem.Flags] & IBF_NUMBER

     lea edi, [ebx + SizeOf(TMem)]

     mov [ebx][TMem.Result], edi
     mov [edi], @(a2i, [ebx][TMem.Default])
     mov [edi + SizeOf(Dword)], edx
    .If CARRY?
      mov [ebx][TMem.Error], IBE_NUMBER_OVERFLOW
    .Else
      mov [ebx][TMem.Error], IBE_NO_ERROR
    .EndIf
  .Else
    mov eax, [ebx][TMem.Default]
    mov [ebx][TMem.Result], eax
    mov [ebx][TMem.Error], IBE_NO_ERROR
  .EndIf
  mov [ebx][TMem.State], TRUE
  Invoke ActivateWindow, ParentSlot
  Invoke ThreadTerminate
  ret
Cancel Endp
; -------------------------------------------------------- ;
Align 4
InputBox_Thread:
 .If Initialized == FALSE
   mov BoxLib,         @(LoadLibrary, CStr("/sys/lib/box_lib.obj"))
   mov EditBoxDraw,    @(GetProcAddress, BoxLib, CStr("edit_box"))
   mov EditBoxKey,     @(GetProcAddress, BoxLib, CStr("edit_box_key"))
   mov EditBoxMouse,   @(GetProcAddress, BoxLib, CStr("edit_box_mouse"))
   mov EditBoxSetText, @(GetProcAddress, BoxLib, CStr("edit_box_set_text"))

   mov Initialized, TRUE
 .EndIf

  mov ebx, Mem

  Invoke EditBoxSetText, Offset Edit1, [ebx][TMem.Default]

  Invoke SetEventMask, EM_REDRAW + EM_KEY + EM_BUTTON + EM_MOUSE

  .Repeat
    Invoke WaitEvent
    .If eax == REDRAW_EVENT
      Invoke  OnRedraw
    .ElseIf eax == KEY_EVENT
      mov Key, @(GetKey)
      .If Key.Code == KEY_CODE_ENTER
        Invoke OK
      .ElseIf Key.Code == KEY_CODE_ESCAPE
        Invoke Cancel
      .Else
        Invoke EditBoxKey, Offset Edit1
      .EndIf
    .ElseIf eax == BUTTON_EVENT
      mov Button, @(GetButton)
      .If Button.ID == BUTTON_CLOSE
        Invoke Cancel
      .ElseIf Button.ID == BUTTON_OK
        Invoke OK
      .ElseIf Button.ID == BUTTON_CANCEL
        Invoke Cancel
      .EndIf
    .ElseIf eax == MOUSE_EVENT
      Invoke EditBoxMouse, Offset Edit1
    .EndIf
  .Until 0
; -------------------------------------------------------- ;
Align 16
EXPORTS Label Dword
Dword	CStr("InputBox"),	Offset InputBox
Dword	0, 0
; -------------------------------------------------------- ;
END
