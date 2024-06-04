;   RDsave ��� Kolibri (0.6.5.0 � ����)
;   Save RAM-disk to hard or floppy drive
;---------------------------------------------------------------------
;   Mario79 2005
;   Heavyiron 12.02.2007
;   <Lrz>     11.05.2009 - ��� ࠡ��� �㦭� ��⥬��� ������⥪� box_lib.obj
;   Mario79   08.09.2010 - select path with OpenDialog,keys 1,2,3,4 for select options
;   Heavyiron 01.12.2013 - new logic
;---------------------------------------------------------------------
appname equ 'RDsave '
version equ '1.44'
debug	equ no

use32	     ; ������� 32-���� ०�� ��ᥬ����
org 0x0      ; ������ � ���

db 'MENUET01'	 ; 8-����� �����䨪��� MenuetOS
dd 0x01 	 ; ����� ��������� (�ᥣ�� 1)
dd START	 ; ���� ��ࢮ� �������
dd IM_END	 ; ࠧ��� �ணࠬ��
dd I_END	 ; ������⢮ �����
dd stacktop	 ; ���� ���設� �⥪�
dd PARAMS	 ; ���� ���� ��� ��ࠬ��஢
dd cur_dir_path


include 'lang.inc' ; Language support for locales: ru_RU (CP866), et_EE, it_IT, en_US.
include '../../../macros.inc'
if debug eq yes
include '../../../debug.inc'
end if
include '../../../proc32.inc'
include '../../../dll.inc'
include '../../../KOSfuncs.inc'
include '../../../load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'str.inc'

	@use_library
;---------------------------------------------------------------------
;---  ������ ���������  ----------------------------------------------
;---------------------------------------------------------------------
align 4
START:
;---------------------------------------------------------------------
	mcall  68,11

load_libraries l_libs_start,end_l_libs
	inc	eax
	test	eax,eax
	jz	close

	stdcall dll.Init,[init_lib]

	invoke	ini_get_int,ini_file,ini_section,aautoclose,0
	mov	[autoclose],eax
	invoke	ini_get_str,ini_file,ini_section,apath,fname_buf,4096,path
	stdcall _lstrcpy,ini_path,fname_buf
	stdcall _lstrcpy,filename_area,start_temp_file_name

	mov   eax,PARAMS
	cmp   byte[eax], 0
	je    no_params
	cmp   byte[eax], 'h'
	je    @f
	cmp   byte[eax], 'H'
	jne   .no_h
@@:
	mov   [hidden],1
	jmp   no_params
.no_h:
	mov   [param],1
	stdcall _lstrcpy,fname_buf,eax
	mov   ah,2
	jmp   noclose

;---------------------------------------------------------------------
no_params:
	stdcall _lstrcpy,check_dir,ini_path
	call	check_path
	test	eax,eax
	jz	path_ok
	cmp	eax,6
	je	path_ok
;---------------------------------------------------------------------
if debug eq yes
dps 'read_folder_error'
newline
end if
;---------------------------------------------------------------------
default_path:
	stdcall _lstrcpy,fname_buf,communication_area_default_path
	mov	[hidden],0

;OpenDialog     initialisation
	push	dword OpenDialog_data
	call	[OpenDialog_Init]

; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]

	mcall	40,0x00000027

	call	draw_window
	mov	ah,3
	jmp	noclose
;---------------------------------------------------------------------
path_ok:
;OpenDialog     initialisation
	push	dword OpenDialog_data
	call	[OpenDialog_Init]

; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]

	mcall	40,0x00000027

	cmp	[hidden],1
	jne	red
	mov	ah,2
	jmp	noclose
red:
	call	draw_window
;---------------------------------------------------------------------
still:
	mcall 10

	dec	eax	 ; ����ᮢ��� ����?
	jz	red	 ; �᫨ �� - �� ���� red
	dec	eax
	jz	key
	dec	eax
	jz	button
	jmp	still
;---------------------------------------------------------------------
button:
	mcall	17	; ������� �����䨪��� ����⮩ ������
	cmp	ah,1		 ; ������ � id=1("�������")?
	jne	noclose
close:
	mcall	-1	    ; �㭪�� -1: �������� �ணࠬ��

;---------------------------------------------------------------------
key:
	mcall	2
	cmp	ah,0x1b
	je	close
	cmp	ah,0x0D
	jne	@f
	mov	ah,2
	jmp	noclose
@@:
	cmp	ah,9h
	jne	still
;---------------------------------------------------------------------
noclose:
	mov	ecx,fname_buf
	push  16
	mov   ebx,1
	cmp   byte[ecx+1],'f'
	je    @f
	cmp   byte[ecx+1],'F'
	jne   not_fdd
@@:
	cmp   byte[ecx+4],'1'
	jne   @f
	cmp   ah,2
	je    doit
@@:
	inc   ebx
	cmp   ah,2
	je    doit
not_fdd:
	push  18
	mov   ebx,6	; 18.6 = save to specified folder
	cmp   ah,2
	je    doit

; invoke OpenDialog
	push	dword OpenDialog_data
	call	[OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	still

; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]
	call	draw_window
	mov	ah,2
	jmp	noclose

doit:
	cmp	[param],0
	jne	 @f
	call	save_ini
@@:
	cmp	byte[ecx+1],'r'
	je	@f
	cmp	byte[ecx+1],'R'
	je	@f
        cmp     byte[ecx],'/'
	je	not_rd
@@:
	mov	edx,rdError
	call	print_err
	jmp	still
not_rd:
	cmp	 [hidden],0
	jne	 @f
	pusha
	stdcall  _lstrcpy,msg,label2
	mov	 eax,[sc.work_text]
	or	 eax,0xc0000088
	mov	 [color],eax
	call	print_msg
	popa
@@:
	pop	eax
	mcall
	call	check_for_error
	jmp	still

;---------------------------------------------------------------------
check_for_error:		      ;��ࠡ��稪 �訡��
	test	eax,eax
	jz	print_ok
	cmp	ebx,6
	je	@f
	mov	edx,error11
	jmp	print_err
@@:
	cmp	eax, 11
	ja	.unknown
	mov	edx, [errors+eax*4]
	stdcall _lstrcpy,msg,error
	stdcall _lstrcat,msg,edx
	mov	edx,msg
	jmp	print_err
.unknown:
	mov	edx, aUnknownError

print_err:
	stdcall _lstrlen,ini_path
	pusha
	invoke	ini_set_str,ini_file,ini_section,apath,ini_path,eax
	popa
	stdcall _lstrcpy,msg,edx
	cmp	[hidden],1
	je	@f
	cmp	[param],1
	je	@f
	mov	 ecx,[sc.work_text]
	or	 ecx,0xc0880000
	mov	 [color],ecx
	call	print_msg
	ret
@@:
	stdcall _lstrcpy, ntf_msg, ntf_start
	stdcall _lstrcat, ntf_msg, edx
	stdcall _lstrcat, ntf_msg, ntf_end_e
	mov	dword [is_notify + 8], ntf_msg
	mcall	70, is_notify
	mov	[param],0
	mov	[hidden],0
	stdcall _lstrcpy,fname_buf,ini_path
	jmp	no_params

print_ok:
	cmp	 [hidden],1
	je	 @f
	cmp	 [param],1
	je	 @f
	stdcall  _lstrcpy,msg,ok
	mov	 ecx,[sc.work_text]
	or	 ecx,0xc0008800
	mov	 [color],ecx
	call	 print_msg
	mcall	 5,100
	cmp	 [autoclose],1
	je	 close
	ret
@@:
	stdcall  _lstrcpy,msg,ok
	stdcall  _lstrcat,msg,fname_buf
	stdcall _lstrcpy, ntf_msg, ntf_start
	stdcall _lstrcat, ntf_msg, msg
	stdcall _lstrcat, ntf_msg, ntf_end_o
	mov	 edx,ntf_msg
	mov	 dword [is_notify + 8], edx
	mcall	 70, is_notify
	mcall	 5,100
	jmp	 close
;---------------------------------------------------------------------
print_msg:
	mcall	13,<5,380>,<96,10>,[sc.work]
	stdcall _lstrlen,msg
	lea	eax,[eax+eax*2]
	add	eax,eax
	mov	ebx,390
	sub	ebx,eax
	shl	ebx,15
	add	ebx,96
	mcall	4, ,[color],msg, ,[sc.work]
	ret
;---------------------------------------------------------------------
draw_PathShow:
	pusha
	mcall	13,<15,280>,<32,16>,0xffffff
	push	dword PathShow_data_1
	call	[PathShow_draw]
	popa
	ret
;---------------------------------------------------------------------
save_ini:
	pusha
	stdcall _lstrlen,fname_buf
	invoke	ini_set_str,ini_file,ini_section,apath,fname_buf,eax
	invoke	ini_set_int,ini_file,ini_section,aautoclose,[autoclose]
	popa
	ret
;---------------------------------------------------------------------
check_path:
stdcall _lstrlen,check_dir
	add	eax,check_dir
@@:
	dec	eax
	cmp	byte [eax],'/'
	jne	@b
	mov	byte [eax+1],0

	mcall	70,read_folder
	ret
;---------------------------------------------------------------------
;---  Draw window  ---------------------------------------------------
;---------------------------------------------------------------------
draw_window:
	mcall  48,3,sc,sizeof.system_colors
	mcall	12,1

	mcall  48,4
	mov ecx,200*65536+111
	add ecx,eax

	mov edx,[sc.work]
	or  edx,0x34000000
	mcall	0,<200,400>,, , ,title

;buttons
	mcall	8,<198,70>,<68,20>,1,[sc.work_button]
	inc	edx
	mcall	 ,<125,70>,
	inc	edx
	mcall	 ,<300,75>,<30,20>

;labels
	mov	ecx,[sc.work_button_text]
	or	ecx,0x80000000
	mcall	4,<134,75>, ,save
	mcall	 ,<215,75>, ,cancel
	mcall	 ,<315,36>, ,select

	m2m	dword [frame_data.font_backgr_color],[sc.work]
	m2m dword [frame_data.font_color],[sc.work_text]
	m2m dword [frame_data.ext_fr_col],[sc.work_graph]
	m2m dword [frame_data.int_fr_col],[sc.work_light]

	push	dword frame_data
	call	[Frame_draw]

	call	draw_PathShow
	call	print_msg

	mcall	12,2
	ret

;---------------------------------------------------------------------
;---  Data  ----------------------------------------------------------
;---------------------------------------------------------------------
if lang eq ru_RU
save		db '���࠭���',0
cancel		db '�⬥��',0
select		db '��������',0
label1		db ' ��ࠧ �㤥� ��࠭�� �: ',0
label2		db '���࠭���� ��ࠧ�...',0
ok		db 'RAM-��� ��࠭�� �ᯥ譮 ',0
error1		db '�� ��।����� ���� �/��� ࠧ��� ���⪮�� ��᪠',0
error2		db '�㭪�� �� �����ন������ ��� ������ 䠩����� ��⥬�',0
error3		db '�������⭠� 䠩����� ��⥬�',0
error4		db '��࠭��... �訡�� 4',0
error5		db '���������騩 ����',0
error6		db '䠩� �����稫��',0
error7		db '㪠��⥫� ��� ����� �ਫ������',0
error8		db '��� ��������',0
error9		db '䠩����� ������� ࠧ��襭�',0
error10 	db '����� ������',0
error11 	db '�訡�� ���ன�⢠',0
aUnknownError	db '�������⭠� �訡��',0
rdError 	db '����� ��࠭��� ��ࠧ � ᠬ��� ᥡ�',0
error		db '�訡��: ',0
;---------------------------------------------------------------------
else if lang eq et_EE
save		db 'Salvesta',0
cancel		db 'Cancel',0
select		db ' Valige',0
label1		db ' RAM-drive will be saved as: ',0
label2		db 'Saving in progress...',0
ok		db 'RAM-ketas salvestatud edukalt ',0
error1		db 'hard disk base and/or partition not defined',0
error2		db 'the file system does not support this function',0
error3		db 'tundmatu failis�steem',0
error4		db 'strange... Error 4',0
error5		db 'vigane teekond',0
error6		db 'end of file',0
error7		db 'pointer is outside of application memory',0
error8		db 'ketas t�is',0
error9		db 'FAT tabel vigane',0
error10 	db 'juurdep��s keelatud',0
error11 	db 'Seadme viga',0
aUnknownError	db 'Tundmatu viga',0
rdError 	db "You can't save image on itself",0
error		db 'Viga: ',0
;---------------------------------------------------------------------
else if lang eq it_IT
save		db '  Salva',0
cancel		db 'Cancel',0
select		db 'Seleziona',0
label1		db ' RAM-drive will be saved as: ',0
label2		db 'Saving in progress...',0
ok		db 'Il RAM-drivet e stato salvato ',0
error1		db 'hard disk base and/or partition not defined',0
error2		db 'the file system does not support this function',0
error3		db 'filesystem sconosciuto',0
error4		db 'strange... Error 4',0
error5		db 'percorso non valido',0
error6		db 'end of file',0
error7		db 'pointer is outside of application memory',0
error8		db 'disco pieno',0
error9		db 'tabella FAT corrotta',0
error10 	db 'accesso negato',0
error11 	db 'Errore di device',0
aUnknownError	db 'Errore sconosciuto',0
rdError 	db "You can't save image on itself",0
error		db 'Errore: ',0
;---------------------------------------------------------------------
else ; Default to en_US
save		db '  Save',0
cancel		db 'Cancel',0
select		db ' Select',0
label1		db ' RAM-drive will be saved as: ',0
label2		db 'Saving in progress...',0
ok		db 'RAM-drive was saved successfully ',0
error1		db 'hard disk base and/or partition not defined',0
error2		db 'the file system does not support this function',0
error3		db 'unknown file system',0
error4		db 'strange... Error 4',0
error5		db 'incorrect path',0
error6		db 'end of file',0
error7		db 'pointer is outside of application memory',0
error8		db 'disk is full',0
error9		db 'file structure is destroyed',0
error10 	db 'access denied',0
error11 	db 'Device error',0
aUnknownError	db 'Unknown error',0
rdError 	db "You can't save image on itself",0
error		db 'Error: ',0
end if
;---------------------------------------------------------------------
ntf_start	db '"RDSave\n', 0
ntf_end_o	db '" -tO', 0
ntf_end_e	db '" -tE', 0
;---------------------------------------------------------------------
errors:
	dd	ok
	dd	error1
	dd	error2
	dd	error3
	dd	error4
	dd	error5
	dd	error6
	dd	error7
	dd	error8
	dd	error9
	dd	error10
	dd	error11
;---------------------------------------------------------------------

title	db appname,version,0

;Lib_DATA
;�ᥣ�� ᮡ��� ��᫥����⥫쭮��� � �����.
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0
system_dir_libini	db '/sys/lib/libini.obj',0
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, library_path, system_dir_Boxlib, \
 Box_lib_import

library02  l_libs system_dir_ProcLib+9, library_path, system_dir_ProcLib, \
 ProcLib_import

library03  l_libs system_dir_libini+9, library_path, system_dir_libini, \
 libini_import

end_l_libs:
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 1	; Save
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd temp_dir_path	;+16
.dir_default_path	dd communication_area_default_path	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_pach		dd fname_buf	;+36
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 200 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 120 ;+54 ; Window Y position

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
communication_area_default_path:
	db '/',0

Filter:
dd	Filter.end - Filter
.1:
db	'IMG',0
db	'IMA',0
.end:
db	0

start_temp_file_name:	db 'kolibri.img',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init 	dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
	dd	0,0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw 36	;+4
.start_x		dw 20	;+6
.font_size_x		dw 6	;+8     ; 6 - for font 0, 8 - for font 1
.area_size_x		dw 270	;+10
.font_number		dd 0	;+12    ; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0	;+20
.background_color	dd 0	;+24
.text_pointer		dd fname_buf	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
align 4
Box_lib_import:
;edit_box_draw           dd aEdit_box_draw
;edit_box_key            dd aEdit_box_key
;edit_box_mouse          dd aEdit_box_mouse
;version_ed              dd aVersion_ed

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
Frame_draw		dd sz_Frame_draw
			dd 0,0

;aEdit_box_draw          db 'edit_box_draw',0
;aEdit_box_key           db 'edit_box_key',0
;aEdit_box_mouse         db 'edit_box_mouse',0
;aVersion_ed             db 'version_ed',0

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0

sz_Frame_draw		db 'frame_draw',0
;szVersion_frame        db 'version_frame',0
;---------------------------------------------------------------------
frame_data:
.type			dd 0 ;+0
.x:
.x_size 		dw 374 ;+4
.x_start		dw 8 ;+6
.y:
.y_size 		dw 45 ;+8
.y_start		dw 17 ;+10
.ext_fr_col		dd 0x888888 ;+12
.int_fr_col		dd 0xffffff ;+16
.draw_text_flag 	dd 1 ;+20
.text_pointer		dd label1 ;+24
.text_position		dd 0 ;+28
.font_number		dd 0 ;+32
.font_size_y		dd 9 ;+36
.font_color		dd 0x0 ;+40
.font_backgr_color	dd 0xdddddd ;+44
;---------------------------------------------------------------------
align 4
libini_import:
init_lib     dd a_init
ini_get_str  dd aini_get_str
ini_get_int  dd aini_get_int
ini_set_str  dd aini_set_str
ini_set_int  dd aini_set_int
	     dd 0,0
a_init	     db 'lib_init',0
aini_get_str db 'ini_get_str',0
aini_get_int db 'ini_get_int',0
aini_set_str db 'ini_set_str',0
aini_set_int db 'ini_set_int',0
;---------------------------------------------------------------------

ini_file db  '/sys/settings/app.ini',0
ini_section db 'RDSave',0
apath db 'path',0
aautoclose db 'autoclose',0
path	db '/hd2/1/kolibri.img',0
;---------------------------------------------------------------------
is_notify:
    dd	  7, 0, ok, 0, 0
    db	  "/sys/@notify", 0

read_folder:
.subfunction	dd 1
.start		dd 0
.flags		dd 0
.size		dd 1
.return 	dd folder_data
		db 0
.name:		dd check_dir

param dd 0
hidden dd 0
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
align 4
PARAMS:
       rb 256
ini_path:
	rb 4096
check_dir:
	rb 4096

sc     system_colors

autoclose rd 1

color	rd 1

msg:
	rb 1024

ntf_msg:
	rb 1024

folder_data:
	rb 304*32+32 ; 9 Kb
;---------------------------------------------------------------------
cur_dir_path:
	rb 4096
;---------------------------------------------------------------------
library_path:
	rb 4096
;---------------------------------------------------------------------
temp_dir_path:
	rb 4096
;---------------------------------------------------------------------
fname_buf:
	rb 4096
;---------------------------------------------------------------------
procinfo:
	rb 1024
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
text_work_area:
	rb 1024
;---------------------------------------------------------------------
align 32
	rb 4096
stacktop:
I_END:
