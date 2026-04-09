;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                    ;;
;;  Kobra is free software: you can redistribute it and/or modify it under the terms of the GNU   ;;
;;  General Public License as published by the Free Software Foundation, either version 3         ;;
;;  of the License, or (at your option) any later version.                                        ;;
;;                                                                                                ;;
;;  Kobra is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without    ;;
;;  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU ;;
;;  General Public License for more details.                                                      ;;
;;                                                                                                ;;
;;  You should have received a copy of the GNU General Public License along with Kobra.           ;;
;;  If not, see <http://www.gnu.org/licenses/>.                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  This is set of functions to work with Kobra daemon                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

define KOBRA_MESSAGE_MAX_LEN 0x100

define KOBRA_CMD_REGISTER	'R'
define KOBRA_CMD_JOIN		'J'
define KOBRA_CMD_UNJOIN		'U'
define KOBRA_CMD_SEND		'S'

define KOBRA_MESSAGE_LAUNCH_STATE 1

kobra_register:
	mov eax, kobra_message_area
	mov byte [eax], KOBRA_CMD_REGISTER
	
	stdcall thread_find_by_name, kobra_thread_name
	
	test eax, eax
	je .error
	
	mov dword [kobra_tid], eax				;; Save tid
	
	mov ecx, eax
	
	mcall 60, 2, , kobra_message_area, 1
	
	test eax, eax
	je .return
	
.error:
	inc eax
	
.return:
	ret

;; void kobra_send_message(char *group, void *message, int length);
kobra_send_message:
	push ebp
	mov ebp, esp
	
	cld
	
	mov edi, kobra_message_area
	mov al, KOBRA_CMD_SEND
	stosb
	
	mov esi, dword [ebp+8]
	
.copy_group_cycle:
	mov al, byte [esi]
	test al, al
	jz .copy_group_end
	movsb
	jmp .copy_group_cycle
	
.copy_group_end:
	
; 	xor al, al
	stosb
	
	mov esi, dword [ebp+12]
	mov ecx, dword [ebp+16]
	rep movsb
	
	lea eax, [edi-kobra_message_area]
	
	mcall 60, 2, dword [kobra_tid], kobra_message_area, eax
	
	leave
	
	ret 12

kobra_tid:
	dd 0

kobra_message_area:
	rb KOBRA_MESSAGE_MAX_LEN

kobra_thread_name:
	db "kobra", 0, 0, 0, 0, 0, 0
