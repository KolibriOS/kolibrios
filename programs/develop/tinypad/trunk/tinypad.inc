
struct POINT
  X dd ?
  Y dd ?
ends

struct RECT
  Left	 dd ?
  Top	 dd ?
  Right  dd ?
  Bottom dd ?
ends

struct SCROLLBAR
  Top  dd ?
  Size dd ?
ends

struct EDITOR
  FilePath	 db PATHL dup(?)
  FileName	 dd ?
  Bounds	 RECT
  Lines 	 dd ?
  Lines.Size	 dd ?
  Lines.Count	 dd ?
  Columns.Count  dd ?
  Caret 	 POINT
  SelStart	 POINT
  TopLeft	 POINT
  VScroll	 SCROLLBAR
  HScroll	 SCROLLBAR
  Gutter.Width	 dd ?
  Gutter.Visible db ?
  AsmMode	 db ?
  Modified	 db ?
		 db ?
ends

struct TABITEM
  Editor EDITOR
ends

struct TABCTL
  Bounds	RECT
  Items 	dd ?
  Items.Count	dd ?
  Items.Left	dd ?
  Current	TABITEM
  Current.Ptr	dd ?
  Default.Ptr	dd ?
  Buttons.First dd ?
  Buttons.Last	dd ?
  Style 	db ?
		db 3 dup(?)
ends



virtual at -20
 POPUP:
  .actions dd ?
  .data    dd ?
  .acc_ofs dd ?
 .size:
  .height  dw ?
  .width   dw ?
 .pos:
  .y	   dw ?
  .x	   dw ?
end virtual

macro popup_res _name,[_lang,_title,_accel,_action]
{
  common
    local s1,s2,m1,m2,c1,c2
    m1 = 0
    m2 = 0
    c1 = 0
    c2 = 0
    if used _name
      dd _name#.actions
      dd _name#.data
      dd ?
      dw ?		; height
      dw ?		; width
      dw ?		; y
      dw ?		; x
      _name:
  forward
      if (lang eq _lang) | (_lang eq @!)
	db 1
      end if
  common
      .data:
  forward
      if (lang eq _lang) | (_lang eq @!)
	if _title eq '-'
	  db 1,'-'
	  c2 = c2+1
	else
	  virtual at 0
	    db _title
	    s1 = $
	  end virtual
	  if m1 < s1
	    m1 = s1
	  end if
	  virtual at 0
	    db _accel
	    s2 = $
	  end virtual
	  if m2 < s2
	    m2 = s2
	  end if
	  db s1,_title,s2,_accel
	  c1 = c1+1
	end if
      end if
  common
      db 0
      align 4
      .actions:
  forward
      if (lang eq _lang) | (_lang eq @!)
	if (_action eq )
	  dd 0
	else
	  dd _name#.#_action
	end if
      end if
  common
      .size	 = $-_name+20
      .max_title = m1
      .max_accel = m2
      .cnt_item  = c1
      .cnt_sep	 = c2
      if (m2 = 0)
	m2 = -2
      end if
      store word (m1*6+m2*6+6*8) at _name+POPUP.width
      store word (c1*POP_IHEIGHT+c2*4+4) at _name+POPUP.height
      store dword (((m1+2)*6-1)*65536) at _name+POPUP.acc_ofs
    end if
}

macro menubar_res _name,[_lang,_title,_popup,_onshow]
{
  common
    local s,m,c,x
    m = 0
    c = 0
    x = 2
    if used _name
      _name:
  forward
      if lang eq _lang
	virtual at 0
	  db _title
	  s = $
	end virtual
	if m < s
	  m = s
	end if
	dw (s+2)*6,x,ATOPH-1,0
	db s,_title
	c = c+1
	x = x + (s+2)*6
      end if
  common
      db 0
      align 4
      .popups:
  forward
      if lang eq _lang
	dd _popup
      end if
  common
      align 4
      .onshow:
  forward
      if lang eq _lang
	dd _onshow
      end if
  common
      .size	 = $-_name
      .max_title = m
      .cnt_item  = c
      .width	 = x-7
    end if
}

;// OPTIONS

OPTS_SECURESEL	= 00000001b
OPTS_AUTOBRACES = 00000010b
OPTS_AUTOINDENT = 00000100b
OPTS_SMARTTAB	= 00001000b
OPTS_OPTIMSAVE	= 00010000b
OPTS_LINENUMS	= 00100000b

;// MOUSE EVENTS

MEV_LDOWN = 1
MEV_LUP   = 2
MEV_RDOWN = 3
MEV_RUP   = 4
MEV_MOVE  = 5

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