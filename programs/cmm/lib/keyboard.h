#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H
#print "[include <keyboard.h>]\n"

//ASCII KEYS
#define ASCII_KEY_BS    008
#define ASCII_KEY_TAB   009
#define ASCII_KEY_ENTER 013
#define ASCII_KEY_ESC   027
#define ASCII_KEY_DEL   182
#define ASCII_KEY_INS   185
#define ASCII_KEY_SPACE 032

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

#define SCAN_CODE_LEFT  075
#define SCAN_CODE_RIGHT 077
#define SCAN_CODE_DOWN  080
#define SCAN_CODE_UP    072
#define SCAN_CODE_HOME  071
#define SCAN_CODE_END   079
#define SCAN_CODE_PGDN  081
#define SCAN_CODE_PGUP  073

#define SCAN_CODE_KEY_C 046
#define SCAN_CODE_KEY_M 050
#define SCAN_CODE_KEY_O 024
#define SCAN_CODE_KEY_P 025

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

inline fastcall word GetKey()  //+Gluk fix
{
		$push edx
GETKEY:
		$mov  eax,2
		$int  0x40
		$cmp eax,1
		$jne GETKEYI
		$mov ah,dh
		$jmp GETKEYII //jz?
GETKEYI:
		$mov dh,ah
		$jmp GETKEY
GETKEYII:
		$pop edx
		$shr eax,8
}

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
	//get alt/shift/ctrl key status
	$mov eax,66
	$mov ebx,3
	$int 0x40
	key_modifier = EAX;
	EAX = key_editbox;
}

#endif