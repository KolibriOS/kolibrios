;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                    ;;
;;  This program is free software: you can redistribute it and/or modify it under the terms of    ;;
;;  the GNU General Public License as published by the Free Software Foundation, either version 3 ;;
;;  of the License, or (at your option) any later version.                                        ;;
;;                                                                                                ;;
;;  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     ;;
;;  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     ;;
;;  See the GNU General Public License for more details.                                          ;;
;;                                                                                                ;;
;;  You should have received a copy of the GNU General Public License along with this program.    ;;
;;  If not, see <http://www.gnu.org/licenses/>.                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Now there is only one function now: find string in array of strings                           ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; int str_find_in_array(char **options, char *str)
;; NOTE: options should be zero-ended array
str_find_in_array:
	push ebx
	
	mov ebx, [esp+8]
	cld
.convert_nxt:
	add ebx, 4
	mov esi, dword [ebx-4]
	test esi, esi
	je .set_default 			;; String is incorrect, so set default
	mov edi, [esp+0xc]
.conv_loop:
	cmpsb
	jne .convert_nxt			;; Not equal, so try next
	dec esi
	lodsb
	test al, al
	jne .conv_loop
.found:
	sub ebx, [esp+8]
	shr ebx, 2
	mov eax, ebx
.exit:
	dec eax
	pop ebx
	ret
.set_default:
	xor eax, eax
	jmp .exit
