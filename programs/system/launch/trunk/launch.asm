;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                    ;;
;;  Launch is free software: you can redistribute it and/or modify it under the terms of the GNU  ;;
;;  General Public License as published by the Free Software Foundation, either version 2         ;;
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

APP_NAME fix 'Launch'
APP_VERSION fix '0.1.4'

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

define PATH_MAX_LEN 1024
define DEBUG_MAX_LEN 8
define DEBUG_DEFAULT debug_no_num
define BUFF_SIZE 1024

include 'macros.inc'
include 'proc32.inc'
include 'libio.inc'
include 'mem.inc'
include 'dll.inc'

START:
	;; Initialize process heap
	mcall 68,11
	test eax, eax
	jz exit

	;; Import modules
	stdcall dll.Load,importTable
	test eax, eax
	jnz exit

read_ini:
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

read_ini_debug:
	;; Read debug options from config files
	invoke ini.get_str, etc_cfg, cfg_debug, cfg_debug, debug_option, DEBUG_MAX_LEN, DEBUG_DEFAULT
	invoke ini.get_str, path, cfg_debug, cfg_debug, debug_option, DEBUG_MAX_LEN, debug_option

	;; Now convert debug_option from string to number of debug mode
	mov ebx, modes-4
.convert:
	mov al, byte [debug_option]
	cmp al, 10
	jbe .converted

.convert_nxt:
	add ebx, 4
	mov esi, dword [ebx]
	test esi, esi
	je .set_default 			;; String is incorrect, so set default
	mov edi, debug_option
.conv_loop:
	mov al, byte [esi]
	cmp al, byte [edi]
	jne .convert_nxt			;; Not equal, so try next
	test al, al
	je .found				;; Equal, end of loop
	inc esi
	inc edi
	jmp .conv_loop

.set_default:
	mov al, byte [DEBUG_DEFAULT]
	mov byte [debug_option], al
	jmp .converted

.found:
	sub ebx, modes
	shr ebx, 2
	add ebx, modes_nums
	mov al, byte [ebx]
	mov byte [debug_option], al

.converted:
	dec al
	test al, al
	je .ok
	dec al
	test al, al
	je .con_init

.noconsole:
	mov al, 1
	mov byte [debug_option], al
	jmp .ok

.con_init:
	stdcall dll.Load, consoleImport
	test eax, eax
	jnz .noconsole
	invoke con.init, -1, -1, -1, -1, WinTitle

.read_level:
	invoke ini.get_int, etc_cfg, cfg_debug, cfg_level, 0
	invoke ini.get_int, path, cfg_debug, cfg_level, eax
	mov dword [debug_level], eax
.ok:

parse_args:
	;; Now parse command line arguments
	;; TODO: use optparse library
	;; Currently the only argument to parse is program name

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

	;; End of preparations! Now we can find file and launch it.
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
	dec al
	test al, al
	je .prn_stp
	mov eax, dword [debug_level]
	or eax, eax
	je .prn_stp
	dec eax
	or eax, eax
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

exit:
	push eax
	;; If console is present we should write some info
	mov al, byte [debug_option]
	cmp al, 2
	je .write_console

.close:
	mcall -1

.write_console:
	pop eax
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
	jmp .close

align 16
importTable:
library 						\
	libini, 'libini.obj';,                           \
;        libio, 'libio.obj',                            \

import	libini, \
	ini.get_str  ,'ini.get_str', \
\;        ini.set_str  ,'ini.set_str', \
	ini.get_int  ,'ini.get_int';, \
\;        ini.set_int  ,'ini.set_int', \
;        ini.get_color,'ini.get_color', \
;        ini.set_color,'ini.set_color'

;import  libio, \
;        file.find_first,'file.find_first', \
;        file.find_next ,'file.find_next', \
;        file.find_close,'file.find_close', \
;        file.size      ,'file.size', \
;        file.open      ,'file.open', \
;        file.read      ,'file.read', \
;        file.write     ,'file.write', \
;        file.seek      ,'file.seek', \
;        file.tell      ,'file.tell', \
;        file.eof?      ,'file.eof?', \
;        file.truncate  ,'file.truncate', \
;        file.close     ,'file.close'

consoleImport:
library 						\
	conlib, 'console.obj'

import conlib, \
	con.init,	  'con.init',\
	con.exit,	  'con.exit',\
	con.printf,	  'con.printf';,\
;        con.write_asciiz, 'con.write_asciiz'

align 16
APP_DATA:

WinTitle:
	db APP_NAME, ' ', APP_VERSION, 0

message_dbg_not_found:
	db '%s not found', 10, 0

message_error:
	db 'File (%s) not found!', 0

message_ok:
	db '%s loaded succesfully. PID: %d (0x%X)'

empty_str:
	db 0
etc_cfg:
	db '/sys/etc/'
cfg_name:
	db 'launch'
cfg_ext:
	db '.cfg', 0

cfg_main:
	db 'main', 0
cfg_path:
	db 'path', 0
cfg_debug:
	db 'debug', 0
cfg_level:
	db 'level', 0

modes:
	dd debug_no
	dd debug_console
	dd 0

debug_no:
	db 'no', 0
debug_console:
	db 'console', 0

modes_nums:
debug_no_num:
	db 1

debug_console_num:
	db 2

debug_level:
	dd 0

LaunchStruct FileInfoRun

args: db 0
APP_END:
rb 255

prog_args:
	rd 1
path:
	rb 1024

search_path:
	rb PATH_MAX_LEN

debug_option:
	rb DEBUG_MAX_LEN

buff:
	rb BUFF_SIZE

rb 0x1000	;; 4 Kb stack
APP_STACK:
MEM_END:
