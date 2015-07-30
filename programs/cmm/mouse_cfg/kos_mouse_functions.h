
inline fastcall int GetMouseSpeed() {
	$mov eax,18
	$mov ebx,19
	$mov ecx,0
	$int 0x40
}

inline fastcall void SetMouseSpeed(EDX) {
	$mov eax,18
	$mov ebx,19
	$mov ecx,1
	$int 0x40
}

inline fastcall int GetMouseDelay() {
	$mov eax,18
	$mov ebx,19
	$mov ecx,2
	$int 0x40
}

inline fastcall void SetMouseDelay(EDX) {
	$mov eax,18
	$mov ebx,19
	$mov ecx,3
	$int 0x40
}