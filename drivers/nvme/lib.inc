;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc memsetdz stdcall, dest:dword, sz:dword

		push 	edi
		mov 	edi, [dest]
		mov 	ecx, [sz]
		xor 	eax, eax
		rep 	stosd
		pop 	edi
		ret

endp

proc memcpyd stdcall, dest:dword, src:dword, sz:dword

		push 	esi edi
		mov 	esi, [src]
		mov 	edi, [dest]
		mov 	ecx, [sz]
		rep 	movsd
		pop 	edi esi
		ret

endp

