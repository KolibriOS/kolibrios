Comment *--------------------+
|                            |
|    Run with OpenDialog     |
|    UASM + KolibriOS.lib    |
|                            |
+----------------------------*
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

IfDef     LANG_IT
  szProgramRunSuccessfully equ <CStr("Programma eseguito correttamente")>
  szFileNotFound           equ <CStr("File non trovato")>
  szFileIsNotExecutable    equ <CStr("File non eseguibile")>
  szTooManyProcesses       equ <CStr("Troppi processi")>
  szAccessDenied           equ <CStr("Accesso negato")>
  szOutOfMemory            equ <CStr("Memoria esaurita")>
  szUnknownError           equ <CStr("Errore sconosciuto")>
  szTypeNameOfProgram      equ <CStr("Digita il nome del programma per eseguirlo")>
  szRun                    equ <CStr("Esegui")>
  szTitle                  equ <CStr("Esegui")>
  szPressF1                equ <CStr("Premere F1 per visualizzare le informazioni sui tasti di scelta rapida")>
  szHelpInfo               equ <CStr("Insert                - il percorso da aprire la finestra di dialogo casella di modifica\n",\
                                     "Ctrl + Insert         - aggiungi percorso alla posizione corrente del cursore\n",\
                                     "Ctrl + Shift + Insert - aggiungi sempre un percorso senza virgolette")>
ElseIfDef LANG_SP
  szProgramRunSuccessfully equ <CStr("Programa ejecutado con exito")>
  szFileNotFound           equ <CStr("Archivo no encontrado")>
  szFileIsNotExecutable    equ <CStr("Archivo no es ejecutable")>
  szTooManyProcesses       equ <CStr("Muchos procesos")>
  szAccessDenied           equ <CStr("Acceso denegado")>
  szOutOfMemory            equ <CStr("Memoria insuficiente")>
  szUnknownError           equ <CStr("Error desconocido")>
  szTypeNameOfProgram      equ <CStr("Tipo nombre del programa a ejecutar")>
  szRun                    equ <CStr("Ejecutar")>
  szTitle                  equ <CStr("Ejecutar")>
  szPressF1                equ <CStr("Presione F1 para ver la informacion acerca de las teclas de acceso rapido")>
  szHelpInfo               equ <CStr("Insert                - establecer la ruta de recuadro de dialogo en el cuadro de edicion\n",\
                                     "Ctrl + Insert         - agregar ruta a la posicion actual del cursor\n",\
                                     "Ctrl + Shift + Insert - siempre agregar la ruta sin las comillas")>
ElseIfDef LANG_RU
  szProgramRunSuccessfully equ <CStr("Программа запущена успешно")>
  szFileNotFound           equ <CStr("Файл не найден")>
  szFileIsNotExecutable    equ <CStr("Файл не является исполняемым")>
  szTooManyProcesses       equ <CStr("Слишком много процессов")>
  szAccessDenied           equ <CStr("Доступ запрещен")>
  szOutOfMemory            equ <CStr("Недостаточно памяти")>
  szUnknownError           equ <CStr("Неизвестная ошибка")>
  szTypeNameOfProgram      equ <CStr("Введите название программы")>
  szRun                    equ <CStr("Запустить")>
  szTitle                  equ <CStr("Запуск программы")>
  szPressF1                equ <CStr("Нажмите клавишу F1, чтобы просмотреть информацию о горячих клавишах")>
  szHelpInfo               equ <CStr("Insert                - установить путь из диалога открытия в поле ввода\n",\
                                     "Ctrl + Insert         - добавить путь из диалога открытия в поле ввода в текущую позицию курсора\n",\
                                     "Ctrl + Shift + Insert - всегда добавлять путь без кавычек")>
ElseIfDef LANG_EN
  szProgramRunSuccessfully equ <CStr("Program run successfully")>
  szFileNotFound           equ <CStr("File not found")>
  szFileIsNotExecutable    equ <CStr("File is not executable")>
  szTooManyProcesses       equ <CStr("Too many processes")>
  szAccessDenied           equ <CStr("Access denied")>
  szOutOfMemory            equ <CStr("Out of memory")>
  szUnknownError           equ <CStr("Unknown error")>
  szTypeNameOfProgram      equ <CStr("Type name of program to run")>
  szRun                    equ <CStr("  Run ")>
  szTitle                  equ <CStr("Run")>
  szPressF1                equ <CStr("Press F1 to view information about the hot keys")>
  szHelpInfo               equ <CStr("Insert                - set path from open dialog to edit box\n",\
                                     "Ctrl + Insert         - add path from open dialog into edit box at the current cursor position\n",\
                                     "Ctrl + Shift + Insert - always add path without quotes")>
Else
  .Err LANG_??
EndIf

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

; bits for the GetControlKeyState
LEFT_SHIFT_PRESSED  = 1
RIGHT_SHIFT_PRESSED = 2
LEFT_CTRL_PRESSED   = 4
RIGHT_CTRL_PRESSED  = 8

TOpenDialog Struct
  mode             Dword ?
  procinfo         Dword ?
  com_area_name    Dword ?
  com_area         Dword ?
  opendir_path     Dword ?
  dir_default_path Dword ?
  start_path       Dword ?
  draw_Window      Dword ?
  status           Dword ?
  openfile_path    Dword ?
  filename_area    Dword ?
  filter_area      Dword ?
  x_size           Word  ?
  x_start          SWord ?
  y_size           Word  ?
  y_start          SWord ?
TOpenDialog EndS

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
BUTTON_RUN        = 10
BUTTON_BROWSE     = 20

KEY_SCAN_ENTER   = 01CH
KEY_SCAN_INSERT  = 052H
KEY_SCAN_F1      = 03BH

KEY_CODE_0 = 030H

PROC_INFO_BUFFER_SIZE      = 1024
OPEN_FILE_PATH_BUFFER_SIZE = 4096
FILE_NAME_AREA_BUFFER_SIZE = 1024
OPEN_DIR_PATH_BUFFER_SIZE  = OPEN_FILE_PATH_BUFFER_SIZE - FILE_NAME_AREA_BUFFER_SIZE

EDIT1_TEXT_BUFFER_SIZE = 4096
TMP_BUFFER_SIZE = 4096

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
RunFile            Proto Path:Ptr, CmdLine:Ptr
GetStandardColors  Proto ColorTable:Ptr, TableSize:Dword
DrawText           Proto X:SDword, Y:SDword, Text:Ptr, ForeColor:Dword, BackColor:Dword, Flags:Dword, Count:Dword
GetControlKeyState Proto

EditBoxDraw_proto     TypeDef Proto :Ptr
EditBoxKey_proto      TypeDef Proto :Ptr
EditBoxMouse_proto    TypeDef Proto :Ptr
EditBoxSetText_proto  TypeDef Proto :Ptr, :Ptr

OpenDialogInit_proto  TypeDef Proto :Ptr
OpenDialogStart_proto TypeDef Proto :Ptr

OpenDialogInit_proc   TypeDef Ptr OpenDialogInit_proto
OpenDialogStart_proc  TypeDef Ptr OpenDialogStart_proto

EditBoxDraw_proc      TypeDef Ptr EditBoxDraw_proto
EditBoxKey_proc       TypeDef Ptr EditBoxKey_proto
EditBoxMouse_proc     TypeDef Ptr EditBoxMouse_proto
EditBoxSetText_proc   TypeDef Ptr EditBoxSetText_proto
; -------------------------------------------------------- ;
.Const; -------------------------------------------------- ;
; -------------------------------------------------------- ;
szFFFFFFFFOpenDialog db "FFFFFFFF_open_dialog",0
szSYS                db "/sys",0
szOpenDialPath       db "/sys/File managers/opendial",0
szEmpty              db 0
; -------------------------------------------------------- ;
.Data; --------------------------------------------------- ;
; -------------------------------------------------------- ;
FileFilter Dword 0

OD TOpenDialog <\
      0,\
      Offset ProcInfoBuffer,\
      Offset szFFFFFFFFOpenDialog,\
      0,\
      Offset OpenDirPathBuffer,\
      Offset szSYS,\
      Offset szOpenDialPath,\
      Offset OnRedraw,\
      0,\
      0,\
      Offset FileNameAreaBuffer,\
      Offset FileFilter,\
      414,\
      0,\
      414,\
      0\
>

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
; -------------------------------------------------------- ;
.Data? ; ------------------------------------------------- ;
; -------------------------------------------------------- ;
Extern AppParams: Byte
Extern AppPath:   Byte 

BoxLib         Dword ?
ProcLib        Dword ?

OpenDialogInit  OpenDialogInit_proc  ?
OpenDialogStart OpenDialogStart_proc ?

EditBoxDraw     EditBoxDraw_proc     ?
EditBoxKey      EditBoxKey_proc      ?
EditBoxMouse    EditBoxMouse_proc    ?
EditBoxSetText  EditBoxSetText_proc  ?

SC TStandartColors <>
Window TRect <>
Screen TSize <>

RunResult Dword ?
RunParams Dword ?
RunPath   Dword ?

AlwaysWithoutQuote Dword ?
ClearBeforeInsert Dword ?

Key TKeyboardInput <>
Button TButtonInput <>

FileNameAreaBuffer Byte FILE_NAME_AREA_BUFFER_SIZE Dup (?)
OpenFilePathBuffer Byte OPEN_FILE_PATH_BUFFER_SIZE Dup (?)
OpenDirPathBuffer  Byte OPEN_DIR_PATH_BUFFER_SIZE Dup (?)
ProcInfoBuffer     Byte PROC_INFO_BUFFER_SIZE Dup (?)

Edit1MouseVariable Dword ?
Edit1TextBuffer Byte EDIT1_TEXT_BUFFER_SIZE + 2 Dup (?) ; buffer for Edit1.text
TmpBuffer Byte TMP_BUFFER_SIZE Dup (?) ; this buffer uses in StrInsert 
; -------------------------------------------------------- ;
.Code ; -------------------------------------------------- ;
; -------------------------------------------------------- ;
OnRedraw Proc
        Invoke BeginDraw
        Invoke GetStandardColors, Offset SC, SizeOf(SC)

        mov    eax, SC.work_graph
        mov    Edit1.focus_border_color, eax

        Invoke DrawWindow, Window.Left, Window.Top, Window.Right, Window.Bottom, szTitle, SC.work, WS_SKINNED_FIXED + WS_COORD_CLIENT + WS_CAPTION, CS_MOVABLE
        Invoke DrawText, 5, 5, szTypeNameOfProgram, SC.work_text, 0, DT_ZSTRING + DT_CP866_8X16, 0
; Browse button
        mov    eax, Window.Right
        sub    eax, 70
        Invoke DrawButton, eax, 26, 52, 21, SC.work_button, 0, 20
        mov    eax, Window.Right
        sub    eax, 56
        Invoke DrawText, eax, 29, CStr("..."), SC.work_button_text, 0, DT_ZSTRING + DT_CP866_8X16, 0
; Run button
        mov    eax, Window.Right
        sub    eax, 22
        Invoke DrawButton, 5, 52, eax, 21, SC.work_button, 0, 10
        mov    eax, Window.Right
        shr    eax, 1
        sub    eax, 40
        Invoke DrawText, eax, 55, szRun, SC.work_button_text, 0, DT_ZSTRING + DT_CP866_8X16, 0
; Result text
        Invoke DrawText, 5, 80, RunResult, SC.work_text, 0, DT_ZSTRING + DT_CP866_8X16, 0
        Invoke EditBoxDraw, Offset Edit1
        Invoke EndDraw
        ret
OnRedraw Endp
; -------------------------------------------------------- ;
StrInsert Proc Uses esi edi Src:Ptr, Dst:Ptr, Pos:Dword
Local TotalLength:Dword

        mov    TotalLength, 0

        mov    esi, Dst
        mov    edi, Offset TmpBuffer
        mov    ecx, Pos
        add    TotalLength, ecx
        rep    movsb

        mov    edi, Src
        mov    ecx, 0FFFFFFFFH
        xor    eax, eax
        repne scasb
        mov    eax, 0FFFFFFFEH
        sub    eax, ecx
        add    TotalLength, eax

        mov    esi, Src
        mov    edi, Offset TmpBuffer
        add    edi, Pos
        mov    ecx, eax
        rep    movsb

        push   edi

        mov    edi, Dst
        add    edi, Pos
        mov    ecx, 0FFFFFFFFH
        xor    eax, eax
        repne scasb
        mov    eax, 0FFFFFFFEH
        sub    eax, ecx
        inc    eax
        add    TotalLength, eax

        pop    edi

        mov    esi, Dst
        add    esi, Pos
        mov    ecx, eax
        rep movsb

        mov    esi, Offset TmpBuffer
        mov    edi, Dst
        mov    ecx, TotalLength
        rep movsb

        ret
StrInsert Endp
; -------------------------------------------------------- ;
OnButtonRun Proc
Local TmpChar: Byte
Local StartWithQuote: Dword
; need to split path & params
        mov    eax, Edit1.text
; Skip Spaces
        .While Byte Ptr [eax] == ' '
          inc eax
        .EndW
        mov StartWithQuote, 0
; Skip Possible Quotation Mark
        .If Byte Ptr [eax] == '"'
          mov StartWithQuote, TRUE
          inc eax
        .EndIf
        mov    RunPath, eax
; Find Space Or Quote Or EndOfString
        .Repeat
          .If Byte Ptr [eax] == ' '
            .If !StartWithQuote
              mov TmpChar, ' '
              mov Byte Ptr [eax], 0
              push eax ; save pointer
              inc eax
; Skip Spaces
              .While Byte Ptr [eax] == ' '
                inc eax
              .EndW
              .Break
            .EndIf
          .ElseIf Byte Ptr [eax] == '"'
            mov TmpChar, '"'
            mov Byte Ptr [eax], 0
            push eax ; save pointer
            inc eax
; Skip Spaces
            .While Byte Ptr [eax] == ' '
              inc eax
            .EndW
; params can be written immediately after quotation mark
; without any spaces
; like this way: "path"params
            .Break
          .ElseIf Byte Ptr [eax] == 0
            push eax ; save pointer
            mov TmpChar, 0
            .Break
          .EndIf
          inc eax
        .Until 0
        mov RunParams, eax
        Invoke RunFile, RunPath, RunParams
        test   eax, eax
        .If SIGN?
          .If eax == -5
            mov RunResult, szFileNotFound
          .ElseIf eax == -31
            mov RunResult, szFileIsNotExecutable
          .ElseIf eax == -32
            mov RunResult, szTooManyProcesses
          .ElseIf eax == -10
            mov RunResult, szAccessDenied
          .ElseIf eax == -30
            mov RunResult, szOutOfMemory
          .Else
            mov RunResult, szUnknownError
          .EndIf
        .Else
          mov RunResult, szProgramRunSuccessfully
        .EndIf

        pop eax ; restore char changed by 0
        mov cl, TmpChar
        mov Byte Ptr [eax], cl

        Invoke  OnRedraw
        ret
OnButtonRun Endp
; -------------------------------------------------------- ;
OnButtonBrowse Proc
Local NeedQuote:Dword

        Invoke OpenDialogStart, Offset OD
        .If OD.status != 0

; need to check if spaces are in path and if so then quote path
          mov eax, OD.openfile_path
; by default assume quote not needed
          mov NeedQuote, 0
          .While Byte Ptr [eax]
            .If Byte Ptr [eax] == ' '
; need quote
              mov NeedQuote, TRUE
            .EndIf
            inc eax
          .EndW
; eax -> zero byte
          .If NeedQuote && !AlwaysWithoutQuote
            mov Byte Ptr [eax], '"'
            mov Byte Ptr [eax + 1], 0
            push eax ; save pointer
            mov eax, OD.openfile_path
            dec eax
          .Else
            mov eax, OD.openfile_path
          .EndIf

          .If ClearBeforeInsert
            Invoke EditBoxSetText, Offset Edit1, Offset szEmpty
          .EndIf

          Invoke StrInsert, eax, Edit1.text, Edit1.pos

          .If NeedQuote && !AlwaysWithoutQuote
            pop eax ; restore zero changed by quote
            mov Byte Ptr [eax], 0
          .EndIf

          Invoke EditBoxSetText, Offset Edit1, Edit1.text
        .EndIf
        ret
OnButtonBrowse Endp
; -------------------------------------------------------- ;
@Main:
        Invoke SetEventMask, EM_REDRAW + EM_KEY + EM_BUTTON + EM_MOUSE
        Invoke LoadLibrary, CStr("/sys/lib/box_lib.obj")
        mov    BoxLib, eax
        Invoke LoadLibrary, CStr("/sys/lib/proc_lib.obj")
        mov    ProcLib, eax
        Invoke GetProcAddress, BoxLib, CStr("edit_box")
        mov    EditBoxDraw, eax
        Invoke GetProcAddress, BoxLib, CStr("edit_box_key")
        mov    EditBoxKey, eax
        Invoke GetProcAddress, BoxLib, CStr("edit_box_mouse")
        mov    EditBoxMouse, eax
        Invoke GetProcAddress, BoxLib, CStr("edit_box_set_text")
        mov    EditBoxSetText, eax
        Invoke GetProcAddress, ProcLib, CStr("OpenDialog_init")
        mov    OpenDialogInit, eax
        Invoke GetProcAddress, ProcLib, CStr("OpenDialog_start")
        mov    OpenDialogStart, eax

; Copy command line parameters to EditBox
; Assume EDIT1_TEXT_BUFFER_SIZE > PARAMS_SIZE(defined in linker-script)    
        Invoke EditBoxSetText, Offset Edit1, Offset AppParams                         
        
; it need for case if spaces are present in filepath
        lea    eax, [OpenFilePathBuffer + 1]
        mov    OD.openfile_path, eax
        mov    OpenFilePathBuffer, '"'

        Invoke OpenDialogInit, Offset OD

        Invoke GetScreenSize
        mov    Screen, eax
        shr    eax, 17
        add    eax, 90
        mov    Window.Right, eax

        Invoke GetSkinHeight
        add    eax, 100
        mov    Window.Bottom, eax

        movzx  eax, Screen.SizeX
        sub    eax, Window.Right
        shr    eax, 1
        mov    Window.Left, eax

        movzx  eax, Screen.SizeY
        sub    eax, Window.Bottom
        sub    eax, 100
        mov    Window.Top, eax

        mov    eax, Window.Right
        sub    eax, 80
        mov    Edit1.x_size, eax

        mov    RunResult, szPressF1

        .Repeat
          Invoke WaitEvent
          .If eax == REDRAW_EVENT
            Invoke  OnRedraw
          .ElseIf eax == KEY_EVENT
            Invoke GetControlKeyState
            .If eax & (RIGHT_SHIFT_PRESSED Or LEFT_SHIFT_PRESSED)
              mov AlwaysWithoutQuote, TRUE
            .Else
              mov AlwaysWithoutQuote, FALSE
            .EndIf
            .If eax & (RIGHT_CTRL_PRESSED Or LEFT_CTRL_PRESSED)
              mov ClearBeforeInsert, FALSE
            .Else
              mov ClearBeforeInsert, TRUE
            .EndIf
            Invoke GetKey
            mov Key, eax
            .If Key.Scan == KEY_SCAN_ENTER
              Invoke OnButtonRun
            ; need check Code cause numpad0.Scan = Insert.Scan
            .ElseIf (Key.Scan == KEY_SCAN_INSERT) && \
                    (Key.Code != KEY_CODE_0)
              Invoke OnButtonBrowse
            .ElseIf Key.Scan == KEY_SCAN_F1
              Invoke RunFile, CStr("/SYS/@NOTIFY"), szHelpInfo
            .Else
              Invoke EditBoxKey, Offset Edit1
            .EndIf
          .ElseIf eax == BUTTON_EVENT
            Invoke GetButton
            mov Button, eax
            .If Button.ID == 1
              Invoke ThreadTerminate
            .ElseIf Button.ID == BUTTON_RUN
              Invoke OnButtonRun
            .ElseIf Button.ID == BUTTON_BROWSE
              Invoke OnButtonBrowse
            .EndIf
          .ElseIf eax == MOUSE_EVENT
            Invoke EditBoxMouse, Offset Edit1
          .EndIf
        .Until 0
END