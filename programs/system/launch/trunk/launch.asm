;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                    ;;
;;  Launch is free software: you can redistribute it and/or modify it under the terms of the GNU  ;;
;;  General Public License as published by the Free Software Foundation, either version 3         ;;
;;  of the License, or (at your option) any later version.                                        ;;
;;                                                                                                ;;
;;  Launch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without   ;;
;;  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU ;;
;;  General Public License for more details.                                                      ;;
;;                                                                                                ;;
;;  You should have received a copy of the GNU General Public License along with Launch.          ;;
;;  If not, see <http://www.gnu.org/licenses/>.                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Launch finds program in search dirictories and runs it.                                       ;;
;;  For more details see readme.txt                                                               ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary

APP_NAME fix 'Launch'
APP_VERSION fix '0.1.80.1'

use32
org 0x0

db 'MENUET01'
dd 0x01
dd START
dd APP_END
dd MEM_END
dd APP_STACK
dd args
dd path

define DEBUG_NO 0
define DEBUG_CONSOLE 1
define DEBUG_BOARD 2			;; Not used now

define PATH_MAX_LEN 1024
define DEBUG_MAX_LEN 8
define DEBUG_DEFAULT 0
define BUFF_SIZE 1024

include 'proc32.inc'
include 'macros.inc'
include 'libio.inc'
include 'mem.inc'
include 'dll.inc'

purge mov

include 'thread.inc'
include 'ipc.inc'
include 'kobra.inc'

;;--------------------------------------------------------------------------------------------------
;; Basic initialization
START:
	;; Initialize process heap
	mcall 68,11
	test eax, eax
	jz exit

	;; Import modules
	stdcall dll.Load,importTable
	test eax, eax
	jnz exit

;;--------------------------------------------------------------------------------------------------
;; Reading config
read_ini_path:													;; Read search path
	;; First read config in /sys/etc
	invoke ini.get_str, etc_cfg, cfg_main, cfg_path, search_path, PATH_MAX_LEN, empty_str

	;; Next, read config from current directory
	;; Find end of path string
.find_path_eol:
	mov edi, path
	mov ecx, PATH_MAX_LEN
	xor al, al
	cld
	repne scasb

	;; Append ext to run path (NOTE: this work only when config file has name <launch-file-name>.cfg and ext size is dword)
	mov eax, dword [cfg_ext]
	dec edi
	mov dword [edi], eax
	mov byte [edi+5], 0

	;; Currently there is no checking for repeating pathes, so we should only concatenate two strings
	;; So we need to find end of current search_path string
.find_search_eol:
	mov edi, search_path
	mov ecx, PATH_MAX_LEN
	xor al, al
	;cld
	repne scasb
	dec edi

	;; Now we need to correct buffer length
	mov eax, path+PATH_MAX_LEN
	sub eax, edi
	;; Read ini
	invoke ini.get_str, path, cfg_main, cfg_path, edi, PATH_MAX_LEN, empty_str

read_ini_debug:												;; Read debug options
	;; Read debug options from config files
	invoke ini.get_option_str, etc_cfg, cfg_debug, cfg_debug, debug_strings, DEBUG_MAX_LEN, DEBUG_DEFAULT
	invoke ini.get_option_str, path, cfg_debug, cfg_debug, debug_strings, DEBUG_MAX_LEN, eax
	mov [debug_option], eax
	
	test eax, eax				;; No console
	je .ok
	
	jmp .con_init

.console_err:
	mov byte [debug_option], 0
	jmp .ok

.con_init:
	stdcall dll.Load, consoleImport
	test eax, eax
	jnz .console_err
	invoke con.init, -1, -1, -1, -1, window_title

.read_level:
	invoke ini.get_int, etc_cfg, cfg_debug, cfg_level, 0
	invoke ini.get_int, path, cfg_debug, cfg_level, eax
	mov byte [debug_level], al
.ok:

read_ini_kobra:
	invoke ini.get_bool, etc_cfg, cfg_kobra, cfg_use, 0
	invoke ini.get_bool, path, cfg_kobra, cfg_use, eax
	
	mov byte [kobra_use], al

;;--------------------------------------------------------------------------------------------------
;; Parse command line options
parse_args:
	;; Now parse command line arguments
	;; TODO: use optparse library
	;; Currently the only argument to parse is program name with its' arguments

.skip_spaces:
	mov ecx, -1
	mov edi, args
	mov al, ' '
	xor bl, bl
	;cld
	repe scasb

	push edi

	mov ecx, -1
@@:
	scasb
	je @f
	xchg al, bl
	dec edi
	scasb
	jne @b
@@:
	mov dword [prog_args], edi

	pop edi
	dec edi
	;; Now edi = program name

;;--------------------------------------------------------------------------------------------------
;; Finding file
search_file:
	push edi
	mov esi, search_path
	xchg esi, [esp]
.loop:
	or dl, dl
	je .prn_dbg
	xor dl, dl
	jmp .prn_end
.prn_dbg:
	push eax
	mov al, byte [debug_option]
;	dec al
	test al, al
	je .prn_stp
	mov al, byte [debug_level]
	or al, al
	je .prn_stp
	dec al
	or al, al
	je .prn_1
.prn_1:
	cinvoke con.printf, message_dbg_not_found, buff

.prn_stp:
	pop eax
.prn_end:
	xor eax, eax					;; When we check is proramme launched we are checking for eax
	xchg esi, [esp]
	mov edi, buff
.copy_path:
	lodsb
	cmp al, ';'
	je .copy_file
	or al, al
	je exit
	stosb
	jmp .copy_path

.copy_file:
	xchg esi, [esp]
	push esi
.cp_file_loop:
	lodsb
	or al, al
	je .copy_end
	cmp al, ' '
	je .copy_end
	stosb
	jmp .cp_file_loop

.copy_end:
	pop esi
	xor al, al
	stosb

	;; Try to launch

	mov dword [LaunchStruct.Function], 7
	push dword [prog_args]
	pop dword [LaunchStruct.Arguments]
	mov dword [LaunchStruct.Flags], 0
	mov dword [LaunchStruct.Zero], 0
	mov dword [LaunchStruct.FileNameP], buff
	mcall 70, LaunchStruct
	cmp eax, 0
	jl .loop

;;--------------------------------------------------------------------------------------------------
;; Exit
exit:
	mov dword [tid], eax
	;; If console is present we should write some info
	mov al, byte [debug_option]
	cmp al, DEBUG_CONSOLE
	jne .kobra

.write_console:
	mov eax, dword [tid]
	test eax, eax
	jz .write_error
.write_launched:
	cinvoke con.printf, message_ok, buff, eax, eax
	jmp .wr_end
.write_error:
	pop edi
	cinvoke con.printf, message_error, edi
.wr_end:
	invoke con.exit, 0

.kobra:
	mov al, byte [kobra_use]
	test al, al
	je .close
	
.register:
	mov dword [IPC_area], buff
	call IPC_init
; 	jnz .close
	
	mov dword [thread_find_buff], another_buff
	
	call kobra_register
	
	test eax, eax
	jnz .close
	
	;; Prepare message
	mov dword [kobra_message], KOBRA_MESSAGE_LAUNCH_STATE
	
	mov eax, dword [tid]
	mov dword [kobra_message+4], eax
	
.kobra_send:
	stdcall kobra_send_message, kobra_group_launch_reactive, kobra_message, 8

.close:
	mcall -1

;; End of code
;;--------------------------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------------------------
;; Imports
align 16
importTable:

library libini, 'libconfig.obj' ;,                      \
;        libio, 'libio.obj',                            \

import	libini, \
	ini.get_str        ,'ini_get_str', \
\;        ini.set_str      ,'ini_set_str', \
	ini.get_int        ,'ini_get_int', \
\;        ini.set_int      ,'ini_set_int', \
\;        ini.get_color    ,'ini_get_color', \
\;        ini.set_color    ,'ini_set_color', \
	ini.get_option_str ,'ini_get_option_str', \
	ini.get_bool       ,'ini_get_bool';,\

;import  libio, \
;        file_find_first,'file_find_first', \
;        file_find_next ,'file_find_next', \
;        file_find_close,'file_find_close', \
;        file_size      ,'file_size', \
;        file_open      ,'file_open', \
;        file_read      ,'file_read', \
;        file_write     ,'file_write', \
;        file_seek      ,'file_seek', \
;        file_tell      ,'file_tell', \
;        file_eof?      ,'file_eof?', \
;        file_truncate  ,'file_truncate', \
;        file_close     ,'file_close'

consoleImport:
library 						\
	conlib, 'console.obj'

import conlib,\
	con.init,	  'con_init',\
	con.exit,	  'con_exit',\
	con.printf,	  'con_printf' ;,\
;        con.write_asciiz, 'con_write_asciiz'

;;--------------------------------------------------------------------------------------------------
;; Data
align 16
APP_DATA:

;; Window title
window_title:
	db APP_NAME, ' ', APP_VERSION, 0

;; Messages
message_dbg_not_found:
	db '%s not found', 10, 0

message_error:
	db 'File (%s) not found!', 0

message_ok:
	db '%s loaded succesfully. PID: %d (0x%X)', 0

;; Configuration path
etc_cfg:
	db '/sys/etc/'
cfg_name:
	db 'launch'
cfg_ext:
	db '.cfg', 0

;; Strings in config file
cfg_main:
	db 'main', 0
cfg_path:
	db 'path', 0
cfg_debug:
	db 'debug', 0
cfg_level:
	db 'level', 0
cfg_kobra:
	db 'kobra', 0
cfg_use:
	db 'use', 0

;; List of debug modes for parsing debug option
debug_strings:
	dd debug_no
	dd debug_console
	dd 0

debug_no:
	db 'no', 0
debug_console:
	db 'console', 0

;; Empty string
empty_str:
	db 0

kobra_group_launch_reactive:
	db 'launch_reactive', 0

;;--------------------------------------------------------------------------------------------------
;; Configuration options
debug_level:
	db 0

; debug_kobra:
; 	db 0

;; debug option (bool)
debug_option:
	db 0

kobra_use:
	db 0

;;--------------------------------------------------------------------------------------------------
tid:
	dd 0

LaunchStruct FileInfoRun

args: db 0
APP_END:
rb 255

prog_args:
	rd 1
path:
	rb 1024

;; Search path will be here
search_path:
	rb PATH_MAX_LEN

;; Buffer
buff:
	rb BUFF_SIZE

another_buff:
	rb 0x1000

kobra_message:
	rb 0x100

rb 0x1000	;; 4 Kb stack
APP_STACK:
MEM_END:
