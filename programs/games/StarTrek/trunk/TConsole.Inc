; --------------------------------------------------------------------------
; FILE: TConsole.Inc
; DATE: September 21, 2008
; --------------------------------------------------------------------------

CONSOLE_ROWS = 40
CONSOLE_COLS = 80
CONSOLE_ATTRS = 24
CONSOLE_LAST_COL = (CONSOLE_COLS - 1)

; --------------------------------------------------------------------------
ATTR_GAME_MSG = 0
ATTR_CREW_MSG = 1
ATTR_KLINGON = 2
ATTR_ROMULAN = 3
ATTR_COSMOS = 4
ATTR_SHIP = 5
ATTR_STAR = 6
ATTR_BASE = 7
ATTR_THOLIAN = 8
ATTR_BLACK_HOLE = 9
ATTR_PLANET = 10
ATTR_REPORT_TEXT = 11
ATTR_REPORT_VALUE = 12
ATTR_GALAXY_EDGE = 13
ATTR_SHIP_QUAD = 14
ATTR_SCANNED_QUAD = 15
ATTR_SCAN_SCALE = 16
ATTR_HIT_DAMAGE = 17
ATTR_WEB = 18
ATTR_COND_GREEN = 19
ATTR_COND_YELLOW = 20
ATTR_COND_RED = 21
ATTR_PROMPT_TEXT = 22
ATTR_PROMPT_VALUE = 23

; --------------------------------------------------------------------------
virtual at 0
TCharacter:
    .char_Symbol db ?
    .char_Attribute db ?
    .size = $
end virtual

; --------------------------------------------------------------------------
virtual at 0
TLine:
    .cons_Symbols rb (TCharacter.size * CONSOLE_COLS)
    .size = $
end virtual

; --------------------------------------------------------------------------
virtual at 0
TConsole:
    .cons_Rows rb (TLine.size * (CONSOLE_ROWS - 1))
    .cons_LastRow rb TLine.size
    .cons_CaretColumn INDEX ?
    .cons_ActiveColor INDEX ?
    .cons_Attributes COLORREFS CONSOLE_ATTRS
    .size = $
end virtual

; --- EOF ---
