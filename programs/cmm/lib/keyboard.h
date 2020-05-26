#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

//ASCII KEYS
#define ASCII_KEY_BS    008
#define ASCII_KEY_TAB   009
#define ASCII_KEY_ENTER 013
#define ASCII_KEY_ESC   027
#define ASCII_KEY_DEL   182
#define ASCII_KEY_INS   185
#define ASCII_KEY_SPACE 032

#define ASCII_KEY_PLUS  043
#define ASCII_KEY_MINUS 045
#define ASCII_KEY_EQU   061

#define ASCII_KEY_LEFT  176
#define ASCII_KEY_RIGHT 179
#define ASCII_KEY_DOWN  177
#define ASCII_KEY_UP    178
#define ASCII_KEY_HOME  180
#define ASCII_KEY_END   181
#define ASCII_KEY_PGDN  183
#define ASCII_KEY_PGUP  184

//SCAN CODE KEYS
#define SCAN_CODE_BS    014
#define SCAN_CODE_TAB   015
#define SCAN_CODE_ENTER 028
#define SCAN_CODE_ESC   001
#define SCAN_CODE_DEL   083
#define SCAN_CODE_INS   082
#define SCAN_CODE_SPACE 057
#define SCAN_CODE_MENU  093

#define SCAN_CODE_LEFT  075
#define SCAN_CODE_RIGHT 077
#define SCAN_CODE_DOWN  080
#define SCAN_CODE_UP    072
#define SCAN_CODE_HOME  071
#define SCAN_CODE_END   079
#define SCAN_CODE_PGDN  081
#define SCAN_CODE_PGUP  073

#define SCAN_CODE_MINUS 012
#define SCAN_CODE_PLUS  013

#define SCAN_CODE_F1    059
#define SCAN_CODE_F2    060
#define SCAN_CODE_F3    061
#define SCAN_CODE_F4    062
#define SCAN_CODE_F5    063
#define SCAN_CODE_F6    064
#define SCAN_CODE_F7    065
#define SCAN_CODE_F8    066
#define SCAN_CODE_F9    067
#define SCAN_CODE_F10   068
#define SCAN_CODE_F11   087
#define SCAN_CODE_F12   088

#define SCAN_CODE_1     002
#define SCAN_CODE_2     003
#define SCAN_CODE_3     004
#define SCAN_CODE_4     005
#define SCAN_CODE_5     006
#define SCAN_CODE_6     007
#define SCAN_CODE_7     008
#define SCAN_CODE_8     009
#define SCAN_CODE_9     010
#define SCAN_CODE_10    011

#define SCAN_CODE_KEY_A 030
#define SCAN_CODE_KEY_B 048
#define SCAN_CODE_KEY_C 046
#define SCAN_CODE_KEY_D 032
#define SCAN_CODE_KEY_E 018
#define SCAN_CODE_KEY_F 033
#define SCAN_CODE_KEY_G 034
#define SCAN_CODE_KEY_J 036
#define SCAN_CODE_KEY_H 035
#define SCAN_CODE_KEY_I 023
#define SCAN_CODE_KEY_L 038
#define SCAN_CODE_KEY_M 050
#define SCAN_CODE_KEY_N 049
#define SCAN_CODE_KEY_O 024
#define SCAN_CODE_KEY_P 025
#define SCAN_CODE_KEY_Q 016
#define SCAN_CODE_KEY_R 019
#define SCAN_CODE_KEY_S 031
#define SCAN_CODE_KEY_T 020
#define SCAN_CODE_KEY_U 022
#define SCAN_CODE_KEY_V 047
#define SCAN_CODE_KEY_W 017
#define SCAN_CODE_KEY_X 045 
#define SCAN_CODE_KEY_Y 021 
#define SCAN_CODE_KEY_Z 044 


#define KEY_LSHIFT     00000000001b
#define KEY_RSHIFT     00000000010b
#define KEY_LCTRL      00000000100b
#define KEY_RCTRL      00000001000b
#define KEY_LALT       00000010000b
#define KEY_RALT       00000100000b
#define KEY_CAPSLOCK   00001000000b
#define KEY_NUMLOCK    00010000000b
#define KEY_SCROLLLOCK 00100000000b
#define KEY_LWIN       01000000000b
#define KEY_RWIN       10000000000b

:unsigned char key_ascii;
:dword key_scancode, key_modifier, key_editbox;
:int GetKeys()
{
		$push edx
GETKEY:
		$mov  eax,2
		$int  0x40
		$cmp eax,1
		$jne GETKEYI
		$mov eax,edx
		$jmp GETKEYII
GETKEYI:
		$mov edx,eax
		$jmp GETKEY
GETKEYII:
		$pop edx
	key_editbox = EAX;
	key_ascii = AH;
	$shr  eax,16
	key_scancode = AL;
	key_modifier = GetKeyModifier();
	EAX = key_editbox;
}

inline fastcall byte GetKeyScancode()
{
	$mov  eax,2
	$int  0x40
	$shr  eax,16
	return AL;
}

inline fastcall GetKey()
{
	$mov  eax,2
	$int  0x40
}

// ECX is a mode: 1 - scancodes, 0 - ascii
inline fastcall SetKeyboardMode(ECX) 
{
	$mov  eax,66
	$mov  ebx,1 
	$int 0x40	
}

//get alt/shift/ctrl key status
inline fastcall dword GetKeyModifier()
{
	$mov eax,66
	$mov ebx,3
	$int 0x40
	key_modifier = EAX;
}

#endif