;unz - распаковщик, использующий archiver.obj. Поддерживает zip и 7z.

;unz [-o output path] [-f file for unpack] [-f ...] [-h] file.zip
;-h - hide GUI. If params is empty - do exit.

;unz -o /hd0/1/arci -f text1.txt file.zip -unpack in folder only file text1.txt
;or
;unz -o "/hd0/1/arci" -f "text1.txt" text2.txt "file.zip" -unpack in folder only file text1.txt and text2.txt


;НЕ РЕАЛИЗОВАННО
;unz -n "namezone" "file.zip"  - open packed file, write list files of main folder in namezone
;namezone:
;dd 0 - если именнованая зона занята, то 1
;dd cmd - 0 нет команды
;         1 получить список файлов
;         2 получить файл
;         3 установить текуший каталог в архиве
;         4 завершить работу
;data - данные, зависит от комманды
; 1 получить список файлов
; input
;   none
; output
;   dd numfiles - количество файлов и папок в текущем каталоге
;   strz file1...fileN - список строк, разделённых 0
; 2 получить файл
; input
;   dd num bytes
;   strz filename (file1.txt of /fold1/file1.txt)
; output
;   dd num bytes
;   data

use32
org    0
db     'MENUET01'
dd     1, start, init_end, end_mem, stack_top, params,	0


include 'lang.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../dll.inc'
;include '../../debug.inc'
include 'debug.inc'

version equ '0.70'
version_dword equ 0*10000h + 70

WIN_W = 400
SIZE_COPY_BUF = 1024*1024*2
MM_MAX_BLOCKS equ 1024


virtual at 0
kfar_info_struc:
.lStructSize	dd	?
.kfar_ver	dd	?
.open		dd	?
.open2		dd	?
.read		dd	?
.write		dd	?
.seek		dd	?
.tell		dd	?
.flush		dd	?
.filesize	dd	?
.close		dd	?
.pgalloc	dd	?
.pgrealloc	dd	?
.pgfree 	dd	?
.getfreemem	dd	?
.pgalloc2	dd	?
.pgrealloc2	dd	?
.pgfree2	dd	?
.menu		dd	?
.menu_centered_in dd	?
.DialogBox	dd	?
.SayErr 	dd	?
.Message	dd	?
.cur_console_size dd	?
end virtual



;--  CODE  -------------------------------------------------------------------

include 'parse.inc'
include 'fs.inc'
include 'file_tree.inc'
include 'memory_manager.inc'
include 'dialogs.inc'


start:
;dnl
;dpsP params
;dnl
	mcall	68, 11
	mcall	40, 100111b + 0C0000000h
	stdcall dll.Load, IMPORTS
	test	eax, eax
	jnz	exit
	mov	[pathOut],0

;----------------------------
	;1. find input file, clear
	;2. find -o, copy data, clear
	;3. find -f, add pointer, copy data, clear
	;4. find -c, check variable, clear
;1.
	call	getLastParam
	test	eax, eax
	je	wm_redraw
	dec	eax
	je	.arbeit
	jmp	errorParsing

.arbeit:
;2.
	call	getParamOutPath
	cmp	eax, 2
	je	errorParsing

;3.

   @@:
	mov	eax, [iFiles]
	shl	eax, 2
	add	eax, files
	m2m	dword[eax], dword[endPointer]
	stdcall getParam, '-f'
	cmp	eax, 2
	je	errorParsing
	inc	[iFiles]
	cmp	eax, 1
	je	@b

;4.
	mov	edi, params
	mov	ax,'-h'
@@:	cmp	word [edi], ax
	je	.check
	inc	edi
	cmp	edi, params+1024
	je	@f
	cmp	byte[edi],0
	je	@f
	jmp	@b
.check:
	call  startUnpack
	mcall -1
@@:

	stdcall [OpenDialog_Init],OpenDialog_data

;init edit fields  --------------
	xor	al,al
	mov	edi,fInp
	mov	ecx,1024
	repne	scasb
	inc	ecx
	mov	eax,1024
	sub	eax,ecx
	mov	dword[edtPack.size],eax
	mov	dword[edtPack.pos],eax

	xor	al, al
	mov	edi, pathOut
	mov	ecx, 1024
	repne	scasb
	inc	ecx
	mov	eax, 1024
	sub	eax, ecx
	mov	dword[edtUnpPath.size], eax
	mov	dword[edtUnpPath.pos], eax

;main loop --------------------
wm_redraw:
	call winRedraw

still:
	mcall	10
	cmp	eax, 1
	je	wm_redraw
	cmp	eax, 2
	je	wm_key
	cmp	eax, 3
	je	wm_button
	cmp	eax, 6
	je	wm_mouse

	jmp	still

wm_key:
	mcall	2
	cmp	[bWinChild],0
	jne	still

	stdcall [edit_box_key],edtPack
	stdcall [edit_box_key],edtUnpPath

	jmp	still


wm_button:
	mcall	17
	cmp	[bWinChild],0
	jne	still

	cmp	ah, 3
	jne	@f
	call selectInput
	jmp	wm_redraw
    @@:
	cmp	ah, 4
	jne	@f
	call selectOutFold
	jmp	wm_redraw
    @@:

	cmp	ah, 2
	jne	@f
	mcall	51,1,startUnpack,stackUnpack
	mov	[bWinChild],1
	;call   startUnpack
	jmp	wm_redraw
    @@:

	cmp	ah, 1
	je	exit
	jmp	still

wm_mouse:
	cmp	[bWinChild],0
	jne	still
	stdcall [edit_box_mouse],edtPack
	stdcall [edit_box_mouse],edtUnpPath
	jmp	still

exit:
	mcall	-1

errorParsing:
dph edx

	dps ' errorParsing'
	mcall	-1

;--- functions ------------------------------------------------------------------

proc winRedraw
	mcall 12, 1
	mcall 48, 3, sc, sizeof.system_colors
	mov   edx, [sc.work]
	or	  edx, 0x34000000
	mcall 0, <200,WIN_W>, <200,130>, , , title
	mcall 8, <100,100>,<65,25>,2,[sc.work_button]
	mcall 8, <(WIN_W-52),33>,<10,20>,3,[sc.work_button]
	mcall 8, <(WIN_W-52),33>,<35,20>,4,[sc.work_button]

	edit_boxes_set_sys_color edtPack,endEdits,sc
	stdcall [edit_box_draw],edtPack
	stdcall [edit_box_draw],edtUnpPath


	cmp	[redInput],0
	jne	@f
	mov	ecx,[sc.work_text]
	or	ecx,90000000h
	jmp	.l1
      @@:
	mov	 ecx,90FF0000h
     .l1:
	mcall 4, <15,16>, , strInp
	mov	ecx,[sc.work_text]
	or	ecx,90000000h
	mcall 4, <15,37>, , strPath
	mov	ecx,[sc.work_button_text]
	or	ecx,90000000h
if lang eq ru
	mcall 4, <107,70>, , strGo
else
	mcall 4, <127,70>, , strGo
end if
	mcall 4, <(WIN_W-47),12>, , strDots
	mcall 4, <(WIN_W-47),37>, , strDots	

	mcall 12, 2
	ret
endp

;region
selectInput:
	mov	[OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	mov	edi,ODAreaPath
	xor	al,al
	or	ecx,-1
	repne	scasb
	sub	edi,ODAreaPath
	dec	edi
	mov	dword[edtPack+12*4],edi
	mov	ecx,edi
	inc	ecx
	mov	edi,fInp
	mov	esi,ODAreaPath
	rep	movsb
	mov	[redInput],0
	ret
;endregion

;region
selectOutFold:
	mov	[OpenDialog_data.type],2
	stdcall [OpenDialog_Start],OpenDialog_data
	mov	edi,ODAreaPath
	xor	al,al
	or	ecx,-1
	repne	scasb
	sub	edi,ODAreaPath
	dec	edi
	mov	dword[edtUnpPath+12*4],edi
	mov	ecx,edi
	inc	ecx
	mov	edi,pathOut
	mov	esi,ODAreaPath
	rep	movsb
	ret
;endregion


;-------------------------------------------------------------------------------

allfiles dd 0
succfiles dd 0
numbytes dd 0

proc startUnpack
locals
  paramUnp	rd 1
  sizeUnpack	rd 1
  hFile rd 1
  hFileZip rd 1
  hPlugin rd 1
  pathFold rb 256
endl
;if input not corrected
	cmp	[fInp],byte 0
	je	.errNullInp

    ;    mcall   68, 24, Exception, 0FFFFFFFFh ;??
;init plugin
	push	ebp
	stdcall [aPluginLoad],kfar_info
	pop	ebp


;set current directory, create folder
	cmp	[pathOut],0
	jne	@f
	lea	eax,[pathFold]
	stdcall cpLastName, fInp, eax
	lea	eax,[pathFold]
	mov	[fsNewDir.path],eax
	mcall	70, fsNewDir
	mov	ecx, [fsNewDir.path]

	mcall	30,4,,1
	jmp	.n
@@:
	mcall	30,4,pathOut,1
.n:

;open and read first 1KB
	stdcall open, fInp, O_READ
	mov	[hFileZip], eax
	mcall	70,fsZip
	test	eax,eax
	jnz	.errNotFound
	mcall	70,fsZipRead


;open pack
	push	ebp
	stdcall [aOpenFilePlugin],[hFileZip],bdvkPack,filedata_buffer,filedata_buffer_size ,0 ,0 , fInp
	pop	ebp

	test	eax,eax
	jnz	@f
	cmp	ebx,0		;;/ КОСТЫЛЬ!!!!
	je	 .errNotFound	;;значение ebx получено опытным путём.
	cmp	ebx,400h
	je	 .errNotSupp
      @@:
	mov	[hPlugin],eax

;get num of all files
       ; stdcall calcSizeArch,[hPlugin]
	push	ebp
	stdcall [aSetFolder],[hPlugin], .strRoot,0
	pop	ebp
;unpack
;       void __stdcall GetFiles(HANDLE hPlugin, int NumItems, void* items[], void* addfile, void* adddir);
	push	ebp
	stdcall [aGetFiles], [hPlugin], -1, 0, myAddFile, myAddDir
	pop	ebp

jmp @f
   .str1 db '/LICENSE.txt',0
   .strRoot db '.svn',0
@@:

;HANDLE __stdcall open(HANDLE hPlugin, const char* filename, int mode);
;Открыть файл filename. Параметр mode зарезервирован и в текущей версии kfar всегда равен 1.
 ;       push    ebp
 ;       stdcall [aOpen], [hPlugin], .str1, O_READ
 ;       pop     ebp

 ;       mov     [hFile],eax
;unsigned __stdcall read(HANDLE hFile, void* buf, unsigned size);
;Чтение size байт в буфер buf из файла hFile, ранее открытого через open.
;size кратен 512 байт
;Возвращаемое значение: число прочитанных байт, -1 при ошибке.
 ;       push    ebp
 ;       stdcall [aRead], [hFile], copy_buf, SIZE_COPY_BUF
 ;       pop     ebp
 ;
;        mcall   70, fsWrite
;void __stdcall close(HANDLE hFile);
	push	ebp
	stdcall [aClose], [hFile]
	mov	[bWinChild],0
	pop	ebp

	push	ebp
	stdcall [aClosePlugin], [hPlugin]
	mov	[bWinChild],0

	mov	[fsRunNotifyOK.param],strUnpackOk
	mcall	70,fsRunNotifyOK
	pop	ebp
	ret		;SUCCESS


.errNotFound:
;        stdcall SimpleSayErr,strNotFound
	mov	[bWinChild],0
	mov	[fsRunNotifyOK.param],strUnpackFault
	mcall	70,fsRunNotifyOK
	ret

.errNotSupp:
	mov	eax,[fsNewDir.path]
	mov	[fsDelDir.path],eax
	mcall	70, fsDelDir

	mov	[bWinChild],0
	mov	[fsRunNotifyOK.param],strUnpackFault
	mcall	70,fsRunNotifyOK
	ret

.errNullInp:
	mov	[redInput],1
	mov	[bWinChild],0
	ret
endp


proc Exception param1:dword
	stdcall SimpleSayErr,strErrorExc
	ret
endp

proc debugInt3
	dps 'Паника!!!!!!!!!!!!!!!!!!!!!!!!!'
	dnl
	int3
	ret
endp


allnumbytes dd 0
strBackFold db '../',0

proc calcSizeArch hPlugin:dword
locals
  bdwk rb 560
  num	rd 1
endl
;int __stdcall ReadFolder(HANDLE hPlugin, unsigned dirinfo_start,
;        unsigned dirinfo_size, void* dirdata);
	mov	[num],0
    ;    int3
.mainloop:
	push	ebp
	lea	eax, [bdwk]
	stdcall [aReadFolder], [hPlugin],[num],1,eax
	pop	ebp

	cmp	eax,6
	je	.lastFile
;??????????????????????????????????????????????????????????????????????????????????????????????????
	lea	ebx,[bdwk+0x20]      ;почему либа пишет в смещение +0x20 - неизестно
	test	[ebx],dword 10h
	jz	@f
;bool __stdcall SetFolder(HANDLE hPlugin, const char* relative_path,
;                         const char* absolute_path);

	push	ebp
	lea	eax,[ebx+40]
dps 'Folder: '
dpsP eax
dnl
	stdcall [aSetFolder],[hPlugin], eax,0

	pop	ebp
	stdcall calcSizeArch, [hPlugin]
	inc	[num]
	jmp	.mainloop
   @@:

	lea	ebx,[bdwk+0x20]
lea	eax,[ebx+40]
dps 'File: '
dpsP eax
dnl
	mov	eax,[ebx+32]
	add	[allnumbytes],eax
	inc	[num]
	jmp	.mainloop

.lastFile:
;        lea     ebx,[bdwk+0x20]
;        test    [ebx],dword 10h
;        jz      @f
;
;        push    ebp
;        lea     eax,[ebx+40]
;        stdcall [aSetFolder],[hPlugin], eax,0
;        pop     ebp
;        stdcall calcSizeArch, [hPlugin]
;    @@:


	push	ebp
	stdcall [aSetFolder],[hPlugin], strBackFold,0
	pop	ebp
	ret
endp


proc rec_calcSize hPlugin:dword
locals
  bdwk rb 560
endl
;int __stdcall ReadFolder(HANDLE hPlugin, unsigned dirinfo_start,
;        unsigned dirinfo_size, void* dirdata);
;bool __stdcall SetFolder(HANDLE hPlugin, const char* relative_path, const char* absolute_path);
	push	ebp
	lea	eax,[bdwk]
	stdcall [aReadFolder], [hPlugin],1,560,eax
	pop	ebp

	ret
endp
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------

hTrPlugin dd 0

;eax - file struct for sys70

proc rdFoldforTree
locals
  fi rd 0
endl
	cmp	[hTrPlugin],0
	je	.exit
	push	ebx edi esi

	mov	[fi],eax
	push	ebp
	stdcall [aSetFolder],[hTrPlugin], [eax+20],0
	mov	ebp,[esp]
			  ;hPlug,startBlock,numBlocks,buffer
	stdcall [aReadFolder], [hTrPlugin],dword[eax+4],\
			dword[eax+12],dword[eax+16]
	pop	ebp

	;cmp     eax,6
	;je      .lastFile

	;lea     ebx,[bdwk+0x20]      ;почему либа пишет в смещение +0x20 - неизестно

	pop	esi edi ebx
.exit:
	ret
endp

;--  DATA  -------------------------------------------------------------------



sc system_colors


bWinChild db 0	;1 - дочернее окно есть, главное окно не должно реагировать
redInput  db 0	;1 - подсветить красным надпись

if lang eq ru
 title db 'uNZ v0.2 - Распаковщик Zip и 7z',0
 strGo db 'Распаковать',0
 strInp db  '    Архив',0
 strPath db 'Извлечь в',0
 strError db 'Ошибка',0
 strErrorExc db 'Неопознанная ошибка',0
 strOk	db 'OK',0
 strGetPass db 'Пароль',0
 strCancel  db 'Отмена',0
 strUnpackOk  db "'Успешно распаковано' -O",0
 strUnpackFault  db "'Ошибка распаковки' -E",0
 strNotSupport db "'Неподдерживаемый формат архива' -E",0
 strNotFound db "'Файл не найден' -E",0
else if lang eq es
 title db 'uNZ v0.2 - Desarchivador para Zip y 7z',0
 strGo db 'Desarchivar',0
 strInp db 'Archivar',0
 strPath db 'Extraer en',0
 strError db 'Error',0
 strErrorExc db 'Error desconocido',0
 strOk db 'OK',0
 strGetPass db 'Contrasena',0
 strCancel db 'Cancelar',0
 strUnpackOk db "'Extracion exitosa' -O",0
 strUnpackFault db "'Fallo al extraer' -E",0
 strNotSupport db "'El formato del archivo no es soportado' -E",0
 strNotFound db "'Archivo no encontrado' -E",0
else
 title db 'uNZ v0.2 - Unarchiver of Zip and 7z',0
 strGo db   'Unpack',0
 strInp db  'Archive',0
 strPath db 'Extract to',0
 strError db 'Error',0
 strErrorExc db 'Unrecognized error',0
 strOk	db 'OK',0
 strGetPass db 'Password',0
 strCancel  db 'Cancel',0
 strUnpackOk  db "'Unpacked successfuly' -O",0
 strUnpackFault  db "'Unprack failed' -E",0
 strNotSupport db "'Archive format is not supported' -E",0
 strNotFound db "'File not found' -E",0
end if



strDots db '...', 0

;--------
; int __stdcall SayErr(int num_strings, const char* strings[],
;                      int num_buttons, const char* buttons[]);
; int __stdcall DialogBox(DLGDATA* dlg);

stateDlg dd 0 ;0 - in process, 1 - button ok, 2 - button cancel
errmess0 dd strErrorExc


kfar_info:
	dd	.size
	dd	version_dword
	dd	open
	dd	open2
	dd	read
	dd	-1	; write: to be implemented
	dd	seek
	dd	tell
	dd	-1	; flush: to be implemented
	dd	filesize
	dd	close
	dd	xpgalloc
	dd	xpgrealloc
	dd	pgfree
	dd	getfreemem
	dd	debugInt3;libini_alloc
	dd	debugInt3;libini_realloc
	dd	debugInt3;libini_free
	dd	debugInt3;menu
	dd	debugInt3;menu_centered_in
	dd	DialogBox;DialogBox
	dd	SayErr	 ;SayErr
	dd	debugInt3;Message
	dd	0	;cur_width
.size = $ - kfar_info
;--------


iFiles dd 0	;количество распаковываемых файлов
endPointer dd buffer


fsZip:
.cmd	dd 5
	dd 0
	dd 0
.size	dd 0
.buf	dd bdvkPack
	db 0
	dd fInp

fsZipRead:
.cmd	dd 0
	dd 0
	dd 0
.size	dd 1024
.buf	dd filedata_buffer
	db 0
	dd fInp


fsWrite:
.cmd	dd 2	;2 rewrite, 3 - write
.pos	dd 0
.hpos	dd 0
.size	dd SIZE_COPY_BUF
.buf	dd copy_buf
	db 0
.path	dd 0


fsNewDir:
.cmd	dd 9
	dd 0
	dd 0
	dd 0
	dd 0
	db 0
.path	dd 0

fsDelDir:
.cmd	dd 8
	dd 0
	dd 0
	dd 0
	dd 0
	db 0
.path	dd 0



fsRunNotifyOK:
.cmd	dd 7
	dd 0
.param	dd strUnpackOk
.size	dd 0
.buf	dd 0
	db '/sys/@notify',0



edtPack     edit_box (WIN_W-100-60),100,10,0FFFFFFh,0xff,0x80ff,0h,0x90000000,\
            1024, fInp, 0,0,0,0
edtUnpPath  edit_box (WIN_W-100-60),100,35,0FFFFFFh,0xff,0x80ff,0h,0x90000000,\
            1024, pathOut, 0,0,0,0
edtPassword edit_box 200, 56, 40, 0FFFFFFh,0xff,0x80ff,0h,0x90000000,\
            255, 0, 0, ed_focus+ed_always_focus ;+ed_pass

endEdits:



;-------------------------------------------------------------------------------
OpenDialog_data:
.type			dd 0	;0-open, 1-save, 2-select folder
.procinfo		dd RBProcInfo	    ;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd winRedraw		   ;+28
.status 		dd 0	;+32
.openfile_pach		dd ODAreaPath;   ;+36
.filename_area		dd 0;        ;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 100 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 100 ;+54 ; Window Y position

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
	db '/sys/File managers/opendial',0

communication_area_default_pach:
	db '/sys',0

Filter	dd 0


; int __stdcall         ReadFolder(HANDLE hPlugin,
;       unsigned dirinfo_start, unsigned dirinfo_size, void* dirdata);
; void __stdcall        ClosePlugin(HANDLE hPlugin);
; bool __stdcall        SetFolder(HANDLE hPlugin,
;       const char* relative_path, const char* absolute_path);
; void __stdcall        GetOpenPluginInfo(HANDLE hPlugin, OpenPluginInfo* info);
; void __stdcall        GetFiles(HANDLE hPlugin, int NumItems, void* items[],
;       void* addfile, void* adddir);
;       bool __stdcall addfile(const char* name, void* bdfe_info, HANDLE hFile);
;       bool __stdcall adddir(const char* name, void* bdfe_info);
; int __stdcall         getattr(HANDLE hPlugin, const char* filename, void* info);
; HANDLE __stdcall      open(HANDLE hPlugin, const char* filename, int mode);
; void __stdcall        setpos(HANDLE hFile, __int64 pos);
; unsigned __stdcall    read(HANDLE hFile, void* buf, unsigned size);
; void __stdcall        close(HANDLE hFile);
IMPORTS:
library archiver, 'archiver.obj',\
	box_lib ,'box_lib.obj',\
	proc_lib,'proc_lib.obj'

import	archiver,\
	aPluginLoad	,      'plugin_load',\
	aOpenFilePlugin ,      'OpenFilePlugin',\
	aClosePlugin	,      'ClosePlugin',\
	aReadFolder	,      'ReadFolder',\
	aSetFolder	,      'SetFolder',\
	aGetFiles	,      'GetFiles',\
	aGetOpenPluginInfo ,   'GetOpenPluginInfo',\
	aGetattr	,      'getattr',\
	aOpen		,      'open',\
	aRead		,      'read',\
	aSetpos 	,      'setpos',\
	aClose		,      'close',\
	aDeflateUnpack	,      'deflate_unpack',\
	aDeflateUnpack2 ,      'deflate_unpack2'

import	proc_lib,\
	OpenDialog_Init 	,'OpenDialog_init',\
	OpenDialog_Start	,'OpenDialog_start'
import	box_lib,\
	edit_box_draw		,'edit_box_draw',\
	edit_box_key		,'edit_box_key',\
	edit_box_mouse		,'edit_box_mouse'


IncludeIGlobals

params1 db '-o "/hd0/1/unz/pig" -h "/hd0/1/unz/abc1"',0
;--  UDATA  -----------------------------------------------------------------------------
init_end:
align 16
IncludeUGlobals

path rb 512

;params db 'unz -o "fil epar1" -f "arch1.txt" -f "ar ch2.txt" file1',0
;params db 'unz -o "fil epar1" -f arch1.txt -f "ar ch2.txt" file1',0

fInp	rb 1024
pathOut rb 1024 	;путь, куда распакуется всё
files	rd 256

fZipInfo	 rb 40

RBProcInfo	rb 1024
temp_dir_pach	rb 1024
ODAreaPath	rb 1024



;--------

copy_buf rb SIZE_COPY_BUF

execdata rb	1024
execdataend:

filedata_buffer_size = 1024
filedata_buffer rb	filedata_buffer_size

CopyDestEditBuf 	rb	12+512+1
.length = $ - CopyDestEditBuf - 13

bdvkPack rb 560



;------------ memory_manager.inc
align 4
MM_NBlocks	rd 1  ;количество выделенных блоков памяти
MM_BlocksInfo	rd 2*MM_MAX_BLOCKS  ;begin,size


;--------

buffer	rb 4096 ;for string of file name for extract
params rb 4096

	rb 1024
stackUnpack:

	rb 1024
stackDlg:

	rb 1024
stack_top:

end_mem:


