
; Import Section

    dd      0, 0, 0, RVA kernel32, RVA kernel32tbl
    dd      0, 0, 0, RVA user32, RVA user32tbl
    dd      0, 0, 0, RVA gdi32, RVA gdi32tbl
    dd      0, 0, 0, 0, 0

kernel32tbl:
;{W32}
    GetLocalTime dd RVA _GetLocalTime
;{W32}
    SetFilePointer dd RVA _SetFilePointer
;{W32}
    GetFileAttributes dd RVA _GetFileAttributes
;{W32}
    SetCurrentDir dd RVA _SetCurrentDir
;{W32}
    GetConsoleScrBufInfo dd RVA _GetConsoleScrBufInfo
;{W32}
    GetConsoleMode dd RVA _GetConsoleMode
;{W32}
    GetStdHandle dd RVA _GetStdHandle
;{W32}
    FreeConsole dd RVA _FreeConsole
;{W32}
    AllocConsole dd RVA _AllocConsole
;{W32}
    CreateThread dd RVA _CreateThread
;{W32}
    Sleep dd RVA _Sleep
;{W32}
    MulDiv dd RVA _MulDiv
;{W32}
    VirtualFree dd RVA _VirtualFree
;{W32}
    VirtualAlloc dd RVA _VirtualAlloc
;{W32}
    GetFileSize dd RVA _GetFileSize
;{W32}
    WriteFile dd RVA _WriteFile
;{W32}
    ReadFile dd RVA _ReadFile
;{W32}
    CloseHandle dd RVA _CloseHandle
;{W32}
    CreateFile dd RVA _CreateFile
;{W32}
    CreateFileAnsi dd RVA _CreateFileAnsi
;{W32}
    GetTickCount dd RVA _GetTickCount
;{W32}
    FindClose dd RVA _FindClose
;{W32}
    FindNextFile dd RVA _FindNextFile
;{W32}
    FindFirstFile dd RVA _FindFirstFile
;{W32}
    GetProcessHeap dd RVA _GetProcessHeap
;{W32}
    GetModuleFileName dd RVA _GetModuleFileName
;{W32}
    OutputDebugString dd RVA _OutputDebugString
;{W32}
    HeapCreate dd RVA _HeapCreate
;{W32}
    HeapAlloc dd RVA _HeapAlloc
;{W32}
    HeapReAlloc dd RVA _HeapReAlloc
;{W32}
    HeapFree dd RVA _HeapFree
;{W32}
    HeapDestroy dd RVA _HeapDestroy
;{W32}
    GetModuleHandle dd RVA _GetModuleHandle
;{W32}
    ExitProcess dd RVA _ExitProcess
    dd      0

user32tbl:
;{W32}
    OffsetRect dd RVA _OffsetRect
;{W32}
    AdjustWindowRect dd RVA _AdjustWindowRect
;{W32}
    MessageBeep dd RVA _MessageBeep
;{W32}
    GetCursorPos dd RVA _GetCursorPos
;{W32}
    TrackPopupMenu dd RVA _TrackPopupMenu
;{W32}
    AppendMenu dd RVA _AppendMenu
;{W32}
    DestroyMenu dd RVA _DestroyMenu
;{W32}
    CreatePopupMenu dd RVA _CreatePopupMenu
;{W32}
    GetFocus dd RVA _GetFocus
;{W32}
    GetWindowTextLen dd RVA _GetWindowTextLen
;{W32}
    GetWindowText dd RVA _GetWindowText
;{W32}
    GetDlgItem dd RVA _GetDlgItem
;{W32}
    PostMessage dd RVA _PostMessage
;{W32}
    SendMessage dd RVA _SendMessage
;{W32}
    InflateRect dd RVA _InflateRect
;{W32}
    EndDialog dd RVA _EndDialog
;{W32}
    DlgBoxParam dd RVA _DlgBoxParam
;{W32}
    GetSystemMetrics dd RVA _GetSystemMetrics
;{W32}
    DrawIcon dd RVA _DrawIcon
;{W32}
    DrawText dd RVA _DrawText
;{W32}
    ReleaseCapture dd RVA _ReleaseCapture
;{W32}
    SetCapture dd RVA _SetCapture
;{W32}
    SetWindowText dd RVA _SetWindowText
;{W32}
    SetCursor dd RVA _SetCursor
;{W32}
    ReleaseDC dd RVA _ReleaseDC
;{W32}
    GetDC dd RVA _GetDC
;{W32}
    FrameRect dd RVA _FrameRect
;{W32}
    FillRect dd RVA _FillRect
;{W32}
    EndPaint dd RVA _EndPaint
;{W32}
    BeginPaint dd RVA _BeginPaint
;{W32}
    UpdateWindow dd RVA _UpdateWindow
;{W32}
    PostQuitMessage dd RVA _PostQuitMessage
;{W32}
    LoadBitmap dd RVA _LoadBitmap
;{W32}
    LoadImage dd RVA _LoadImage
;{W32}
    LoadIcon dd RVA _LoadIcon
;{W32}
    LoadCursor dd RVA _LoadCursor
;{W32}
    GetWindowRect dd RVA _GetWindowRect
;{W32}
    GetClientRect dd RVA _GetClientRect
;{W32}
    ShowWindow dd RVA _ShowWindow
;{W32}
    CreateWindowEx dd RVA _CreateWindowEx
;{W32}
    DefWindowProc dd RVA _DefWindowProc
;{W32}
    RegisterClass dd RVA _RegisterClass
;{W32}
    GetClassInfo dd RVA _GetClassInfo
;{W32}
    SetWindowLong dd RVA _SetWindowLong
;{W32}
    GetWindowLong dd RVA _GetWindowLong
;{W32}
    GetMessage dd RVA _GetMessage
;{W32}
    DispatchMessage dd RVA _DispatchMessage
;{W32}
    TranslateMessage dd RVA _TranslateMessage
;{W32}
    TranslateAccelerator dd RVA _TranslateAccelerator
;{W32}
    CreateAcceleratorTable dd RVA _CreateAcceleratorTable
;{W32}
    InvalidateRect dd RVA _InvalidateRect
;{W32}
    MoveWindow dd RVA _MoveWindow
;{W32}
    IsWindowVisible dd RVA _IsWindowVisible
;{W32}
    MessageBox dd RVA _MessageBox
    dd      0

gdi32tbl:
;{W32}
    TextOut dd RVA _TextOut
;{W32}
    MoveToEx dd RVA _MoveToEx
;{W32}
    SetTextAlign dd RVA _SetTextAlign
;{W32}
    BitBlt dd RVA _BitBlt
;{W32}
    CreateCompatibleBitmap dd RVA _CreateCompatibleBitmap
;{W32}
    DeleteDC dd RVA _DeleteDC
;{W32}
    SetTextColor dd RVA _SetTextColor
;{W32}
    CreatePatternBrush dd RVA _CreatePatternBrush
;{W32}
    GetStockObject dd RVA _GetStockObject
;{W32}
    CreatePen dd RVA _CreatePen
;{W32}
    Polyline dd RVA _Polyline
;{W32}
    CreateSolidBrush dd RVA _CreateSolidBrush
;{W32}
    DeleteObject dd RVA _DeleteObject
;{W32}
    CreateFontIndirect dd RVA _CreateFontIndirect
;{W32}
    GetDeviceCaps dd RVA _GetDeviceCaps
;{W32}
    GetTextExtentPoint32 dd RVA _GetTextExtentPoint32
;{W32}
    SetBkMode dd RVA _SetBkMode
;{W32}
    SelectObject dd RVA _SelectObject
;{W32}
    CreateCompatibleDC dd RVA _CreateCompatibleDC
    dd      0

; --- Modules

kernel32    db 'KERNEL32.DLL', 0
user32      db 'USER32.DLL', 0
gdi32       db 'GDI32.DLL', 0

; --- Table

align 2
_GetLocalTime dw 0
db 'GetLocalTime',0

align 2
_SetFilePointer dw 0
db 'SetFilePointer',0

align 2
_GetFileAttributes dw 0
db 'GetFileAttributesA',0

align 2
_SetCurrentDir dw 0
db 'SetCurrentDirectoryW',0

align 2
_OffsetRect dw 0
db 'OffsetRect',0

align 2
_GetConsoleScrBufInfo dw 0
db 'GetConsoleScreenBufferInfo',0

align 2
_GetConsoleMode dw 0
db 'GetConsoleMode',0

align 2
_FreeConsole dw 0
db 'FreeConsole',0

align 2
_GetStdHandle dw 0
db 'GetStdHandle',0

align 2
_AllocConsole dw 0
db 'AllocConsole',0

align 2
_CreateThread dw 0
db 'CreateThread',0

align 2
_Sleep dw 0
db 'Sleep',0

align 2
_AdjustWindowRect dw 0
db 'AdjustWindowRect',0

align 2
_MessageBeep dw 0
db 'MessageBeep',0

align 2
_TextOut dw 0
db 'TextOutW',0

align 2
_MoveToEx dw 0
db 'MoveToEx',0

align 2
_SetTextAlign dw 0
db 'SetTextAlign',0

align 2
_BitBlt dw 0
db 'BitBlt',0

align 2
_CreateCompatibleBitmap dw 0
db 'CreateCompatibleBitmap',0

align 2
_DeleteDC dw 0
db 'DeleteDC',0

align 2
_MulDiv dw 0
db 'MulDiv',0

align 2
_GetCursorPos dw 0
db 'GetCursorPos',0

align 2
_TrackPopupMenu dw 0
db 'TrackPopupMenu',0

align 2
_AppendMenu dw 0
db 'AppendMenuW',0

align 2
_DestroyMenu dw 0
db 'DestroyMenu',0

align 2
_CreatePopupMenu dw 0
db 'CreatePopupMenu',0

align 2
_GetFocus dw 0
db 'GetFocus',0

align 2
_SetTextColor dw 0
db 'SetTextColor',0

align 2
_GetWindowTextLen dw 0
db 'GetWindowTextLengthW',0

align 2
_GetWindowText dw 0
db 'GetWindowTextW',0

align 2
_CreatePatternBrush dw 0
db 'CreatePatternBrush',0

align 2
_GetDlgItem dw 0
db 'GetDlgItem',0

align 2
_VirtualFree dw 0
db 'VirtualFree',0

align 2
_VirtualAlloc dw 0
db 'VirtualAlloc',0

align 2
_ImageList_AddMasked dw 0
db 'ImageList_AddMasked',0

align 2
_ImageList_Create dw 0
db 'ImageList_Create',0

align 2
_PostMessage dw 0
db 'PostMessageW',0

align 2
_SendMessage dw 0
db 'SendMessageW',0

align 2
_GetFileSize dw 0
db 'GetFileSize',0

align 2
_WriteFile dw 0
db 'WriteFile',0

align 2
_ReadFile dw 0
db 'ReadFile',0

align 2
_CloseHandle dw 0
db 'CloseHandle',0

align 2
_CreateFile dw 0
db 'CreateFileW',0

align 2
_CreateFileAnsi dw 0
db 'CreateFileA',0

align 2
_InflateRect dw 0
db 'InflateRect',0

align 2
_EndDialog dw 0
db 'EndDialog',0

align 2
_DlgBoxParam dw 0
db 'DialogBoxIndirectParamW',0

align 2
_GetSystemMetrics dw 0
db 'GetSystemMetrics',0

align 2
_GetStockObject dw 0
db 'GetStockObject',0

align 2
_GetTickCount dw 0
db 'GetTickCount',0

align 2
_FindClose dw 0
db 'FindClose',0

align 2
_FindNextFile dw 0
db 'FindNextFileW',0

align 2
_FindFirstFile dw 0
db 'FindFirstFileW',0

align 2
_DrawText dw 0
db 'DrawTextW',0

align 2
_DrawIcon dw 0
db 'DrawIcon',0

align 2
_CreatePen dw 0
db 'CreatePen',0

align 2
_Polyline dw 0
db 'Polyline',0

align 2
_CreateSolidBrush dw 0
db 'CreateSolidBrush',0

align 2
_DeleteObject dw 0
db 'DeleteObject',0

align 2
_SetCapture dw 0
db 'SetCapture',0

align 2
_ReleaseCapture dw 0
db 'ReleaseCapture',0

align 2
_SetWindowText dw 0
db 'SetWindowTextW',0

align 2
_InitCommonControlsEx dw 0
db 'InitCommonControlsEx',0

align 2
_SetCursor dw 0
db 'SetCursor',0

align 2
_GetProcessHeap dw 0
db 'GetProcessHeap',0

align 2
_CreateFontIndirect dw 0
db 'CreateFontIndirectW',0

align 2
_GetDeviceCaps dw 0
db 'GetDeviceCaps',0

align 2
_ReleaseDC dw 0
db 'ReleaseDC',0

align 2
_GetDC dw 0
db 'GetDC',0

align 2
_GetTextExtentPoint32 dw 0
db 'GetTextExtentPoint32W',0

align 2
_GetModuleFileName dw 0
db 'GetModuleFileNameW',0

align 2
_FrameRect dw 0
db 'FrameRect',0

align 2
_FillRect dw 0
db 'FillRect',0

align 2
_SetBkMode dw 0
db 'SetBkMode',0

align 2
_EndPaint dw 0
db 'EndPaint',0

align 2
_BeginPaint dw 0
db 'BeginPaint',0

align 2
_SelectObject dw 0
db 'SelectObject',0

align 2
_OutputDebugString dw 0
db 'OutputDebugStringW',0

align 2
_HeapCreate dw 0
db 'HeapCreate',0

align 2
_HeapAlloc dw 0
db 'HeapAlloc',0

align 2
_HeapReAlloc dw 0
db 'HeapReAlloc',0

align 2
_HeapFree dw 0
db 'HeapFree',0

align 2
_HeapDestroy dw 0
db 'HeapDestroy',0

align 2
_IsWindowVisible dw 0
db 'IsWindowVisible',0

align 2
_MoveWindow dw 0
db 'MoveWindow',0

align 2
_InvalidateRect dw 0
db 'InvalidateRect',0

align 2
_UpdateWindow dw 0
db 'UpdateWindow',0

align 2
_CreateAcceleratorTable dw 0
db 'CreateAcceleratorTableW',0

align 2
_TranslateAccelerator dw 0
db 'TranslateAccelerator',0

align 2
_TranslateMessage dw 0
db 'TranslateMessage',0

align 2
_DispatchMessage dw 0
db 'DispatchMessageW',0

align 2
_GetMessage dw 0
db 'GetMessageW',0

align 2
_PostQuitMessage dw 0
db 'PostQuitMessage',0

align 2
_LoadBitmap dw 0
db 'LoadBitmapW',0

align 2
_LoadImage dw 0
db 'LoadImageW',0

align 2
_LoadIcon dw 0
db 'LoadIconW',0

align 2
_LoadCursor dw 0
db 'LoadCursorW',0

align 2
_GetWindowRect dw 0
db 'GetWindowRect',0

align 2
_GetClientRect dw 0
db 'GetClientRect',0

align 2
_ShowWindow dw 0
db 'ShowWindow',0

align 2
_CreateWindowEx dw 0
db 'CreateWindowExW',0

align 2
_DefWindowProc dw 0
db 'DefWindowProcW',0

align 2
_RegisterClass dw 0
db 'RegisterClassW',0

align 2
_GetClassInfo dw 0
db 'GetClassInfoW',0

align 2
_SetWindowLong dw 0
db 'SetWindowLongW',0

align 2
_GetWindowLong dw 0
db 'GetWindowLongW',0

align 2
_GetModuleHandle dw 0
db 'GetModuleHandleW',0

align 2
_CreateCompatibleDC dw 0
db 'CreateCompatibleDC',0

align 2
_ExitProcess dw 0
db 'ExitProcess',0

align 2
_MessageBox dw 0
db 'MessageBoxW',0

;<EOF>
