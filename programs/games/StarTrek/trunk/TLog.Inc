; --------------------------------------------------------------------------
; FILE: TLog.Inc
; DATE: February 21, 2009
; --------------------------------------------------------------------------

MAX_LOG_ROOM = 40000h

; --------------------------------------------------------------------------
PLAQUE_INSERT_KLRATE = 0F8h
PLAQUE_INSERT_SCORE = 0F9h
PLAQUE_INSERT_DATE = 0FAh
PLAQUE_INSERT_LEVEL = 0FBh
PLAQUE_INSERT_NAME = 0FCh
PLAQUE_STRING = 0FDh
PLAQUE_END_OF_LINE = 0FEh
PLAQUE_END_OF_ALL = 0FFh

; --------------------------------------------------------------------------
virtual at 0
TLog:
    .pBuffer PCHAR ?
    .pCurrentBufPos PCHAR ?
    .nStoredChars COUNT ?
    .nRemainChars COUNT ?
    .size = $
end virtual

; --- EOF ---
