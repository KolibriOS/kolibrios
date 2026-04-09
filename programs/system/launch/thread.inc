;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                    ;;
;;  This program is free software: you can redistribute it and/or modify it under the terms of    ;;
;;  the GNU General Public License as published by the Free Software Foundation, either version 3 ;;
;;  of the License, or (at your option) any later version.                                        ;;
;;                                                                                                ;;
;;  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     ;;
;;  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See ;;
;;  the GNU General Public License for more details.                                              ;;
;;                                                                                                ;;
;;  You should have received a copy of the GNU General Public License along with this program.    ;;
;;  If not, see <http://www.gnu.org/licenses/>.                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

thread_find_by_name:
	cld
	
	push ebp
	mov ebp, esp
	
	push ebx ecx
	
	push 1							;; initial slot
	
	mov ebx, dword [thread_find_buff]
	
	push 0

.cycle:
	mcall 9							;; Get thread info
	mov dword [esp], eax					;; number of slots
	
	cmp word [ebx+0x32], 9
	je .next
	
.cmpstr:
	mov ecx, 11
	
	mov esi, dword [thread_find_buff]
	add esi, 0xA						;; ESI = address of thread name in list
	
	mov edi, dword [ebp+8]					;; EDI = address of thread name to find
	
	repe cmpsb
	je .found
	
.next:
	mov ecx, dword [esp+4]
	inc ecx
	cmp ecx, dword [esp]
	jg .not_found
	mov dword [esp+4], ecx
	jmp .cycle
	
.found:
	mov eax, dword [ebx+30]					;; return TID
	
	jmp .exit
	
.not_found:
	xor eax, eax
	
.exit:
	pop ecx ebx
	
	leave
	
	ret 4

thread_find_buff:
	dd 0

event_add_mask:
	xchg eax, dword [esp+4]
	or dword [event_mask], eax
	mcall 40, dword [event_mask]
	xchg eax, dword [esp+4]
	ret 4

event_mask:
	dd 0
