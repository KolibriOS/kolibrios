; --------------------------------------------------------------------------
; FILE: TCmdBuf.Inc
; DATE: September 27, 2008
; --------------------------------------------------------------------------

CMDBUF_ROOM_CHARS = 64
CMDBUF_MAX_CHARS = (CMDBUF_ROOM_CHARS - 1)
CMDBUF_MAX_TOKENS = 16

; --------------------------------------------------------------------------
CMD_TOKEN_EOL = 0
CMD_TOKEN_REAL = 1
CMD_TOKEN_ALPHA = 2
CMD_TOKEN_ERROR = 3

; --------------------------------------------------------------------------
virtual at 0
TCmdBuf:
    .cmdbuf_FullBuffer WCHARS CMDBUF_ROOM_CHARS
    .cmdbuf_CmdColumn INDEX ?
    .cmdbuf_CmdTokens STRINGS CMDBUF_MAX_TOKENS
    .cmdbuf_NumTokens COUNT ?
    .cmdbuf_TokenIterator INDEX ?
    .cmdbuf_AnsiTokenBuf CHARS CMDBUF_ROOM_CHARS
    ;
    ; If scanned token is of type CMD_TOKEN_REAL
    ; then following section is filled, otherwise
    ; all following members are set to zero
    ;
    .cmdbuf_INUM INT32 ?
    .cmdbuf_FNUM DOUBLE ?
    .cmdbuf_KEY BYTE ?
    .cmdbuf_Reserved BYTE ?
    .size = $
end virtual

; --- EOF ---
