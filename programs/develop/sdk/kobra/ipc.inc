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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  This is set of functions to work with IPC in KolibriOS                                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IPC_init:
	push eax
	mov eax, dword [IPC_area]
	mov dword [eax], 1
	mov dword [eax+4], 8
	
	stdcall event_add_mask, 0x40
	
	pop eax
	
	ret

IPC_area:
	dd 0
