; Search Additional Partition for KolibriOS applications
;
; Copyright (c) 2013, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************
	use32
	org	0x0

	db	'MENUET01'
	dd 0x01
	dd START
	dd IM_END
	dd I_END
	dd stacktop
	dd 0x0
	dd 0x0
;---------------------------------------------------------------------
fileinfo:
.subfunction	dd 5
.Offset		dd 0
.Offset_1	dd 0
.size		dd 0
.return		dd folder_data
		db 0
.name:		dd basic_file_path
;---------------------------------------------------------------------
read_folder:
.subfunction	dd 1
.start		dd 0
.flags		dd 0
.size		dd 32
.return		dd folder_data
		db 0
.name:		dd read_folder_name
;---------------------------------------------------------------------
read_folder_1:
.subfunction	dd 1
.start		dd 0
.flags		dd 0
.size		dd 32
.return		dd folder_data_1
		db 0
.name:		dd read_folder_1_name
;---------------------------------------------------------------------
start_dir:
	db '/',0
;-------------------------------------------------------------------------------
basic_file_path:
	db '/rd/1/'
basic_file_name:
	db 'kolibri.lbl',0
additional_dir_name:
    db 'KolibriOS',0
real_additional_dir:
	db '/kolibrios',0
;-------------------------------------------------------------------------------
debug equ no	;yes

include	'../../macros.inc'

if debug eq yes
include	'../../debug.inc'
end if
;-------------------------------------------------------------------------------
START:
	mcall	5,500
	mov	ebx,start_dir
	mov	ax,[ebx]
	mov	ebx,read_folder_name
	mov	[ebx],ax
	mov	ebx,read_folder_1_name
	mov	[ebx],ax
	call	device_detect_f70
;--------------------------------------
if debug eq yes
	call	print_retrieved_devices_table
dps 'get basic file'
newline
end if
;--------------------------------------
	call	load_file	; download the master file
	xor	eax,eax
	cmp	[fs_error],eax
	jne	exit
	mov	eax,[fileinfo.size]
	mov	[basic_file_size],eax

	call	search_and_load_pointer_file_label
;---------------------------------------------------------------------
exit:
;--------------------------------------
if debug eq yes
dps 'just exit'
;newline
;	mov	edx,read_folder_name
;	call	debug_outstr
;newline
;	mov	edx,read_folder_1_name
;	call	debug_outstr
;newline
end if
;--------------------------------------
	mcall	-1
;---------------------------------------------------------------------
device_detect_f70:
;--------------------------------------
if debug eq yes
dps	'read_folder_name: '
	mov	edx,read_folder_name
	call	debug_outstr
newline
end if
;--------------------------------------
	mcall	70,read_folder
	test	eax,eax
	jz	@f
	cmp	eax,6
	je	@f
;--------------------------------------
if debug eq yes
dps 'read_folder_error'
newline
;	mov	edx,read_folder_name
;	call	debug_outstr
;newline
end if
;--------------------------------------
	jmp	exit
@@:
;--------------------------------------
if debug eq yes
	call	print_root_dir
end if
;--------------------------------------
	mov	[left_folder_block],ebx
	xor	eax,eax
	mov	[temp_counter_1],eax
	mov	[retrieved_devices_table_counter],eax
.start_temp_counter_1:
	imul	esi,[temp_counter_1],304
	add	esi,[read_folder.return]
	add	esi,32+40
	call	copy_folder_name_1
;--------------------------------------
if debug eq yes
;dps	'read_folder_1_name: '
;	mov	edx,read_folder_1_name
;	call	debug_outstr
;newline
end if
;--------------------------------------
	mcall	70,read_folder_1
	test	eax,eax
	jz	@f
	cmp	eax,6
	je	@f
;--------------------------------------
if debug eq yes
dps 'read_folder_error_1'
newline
;	mov	edx,read_folder_1_name
;	call	debug_outstr
;newline
end if
;--------------------------------------
	jmp	exit
@@:
	mov	eax,[read_folder_1.return]
	cmp	[eax+4],dword 0
	je	.continue
	mov	[right_folder_block],ebx
	xor	ebp,ebp
.start_copy_device_patch:
	imul	edi,[retrieved_devices_table_counter],10
	add	edi,retrieved_devices_table
	mov	[edi],byte '/'
	inc	edi
	imul	esi,[temp_counter_1],304
	add	esi,[read_folder.return]
	add	esi,32+40
	call	proc_copy_patch
	imul	esi,ebp,304
	add	esi,[read_folder_1.return]
	add	esi,32+40
	mov	[edi-1],byte '/'
	call	proc_copy_patch
	inc	[retrieved_devices_table_counter]
	inc	ebp
	cmp	ebp,[right_folder_block]
	jb	.start_copy_device_patch
.continue:
	inc	[temp_counter_1]
	mov	eax,[temp_counter_1]
	cmp	eax,[left_folder_block]
	jb	.start_temp_counter_1
	mov	esi,retrieved_devices_table+1
	call	copy_folder_name
	mov	esi,retrieved_devices_table+3
	xor	ecx,ecx
@@:
	add	esi,8
	cld
	lodsw
	inc	ecx
	cmp	ecx,[retrieved_devices_table_counter]
	ja	@f
	cmp	ax,'hd'
	jne	@r
	sub	esi,2
	call	copy_folder_name_1
	ret
@@:
	mov	esi,retrieved_devices_table+1
	call	copy_folder_name_1
	ret
;---------------------------------------------------------------------
load_file:
	mov	[fileinfo.subfunction],dword 5
	xor	eax,eax
	mov	[fileinfo.size],eax
	mov	[fs_error],eax
;--------------------------------------
if debug eq yes
dps 'get file info'
newline
end if
;--------------------------------------
	mcall	68,1
	mcall	70,fileinfo
	mov	[fs_error],eax
	test	eax,eax
	jnz	.file_error
;--------------------------------------
if debug eq yes
dps 'file info ok'
newline
end if
;--------------------------------------
	xor	eax,eax
	mov	[fileinfo.subfunction],eax	;dword 0
	mov	eax,[fileinfo.return]
	mov	ecx,[eax+32]
;--------------------------------------
if debug eq yes
dps 'real file size: '
dpd ecx
newline
end if
;--------------------------------------
	test	ecx,ecx
	jz	.file_error
	mov	eax,304*32+32 ; 9 Kb
	cmp	ecx,eax
	jbe	@f
	mov	ecx,eax
;-----------------------------------
@@:
	mov	[fileinfo.size],ecx
;--------------------------------------
if debug eq yes
dps 'get file'
newline
end if
;--------------------------------------
	mcall	68,1
	mcall	70,fileinfo
	mov	[fs_error],eax
	test	eax,eax
	jz	@f
;	cmp	eax,6
;	jne	.file_error
;	xor	eax,eax
;	mov	[fs_error],eax
;	jmp	@f
;-----------------------------------
.file_error:
;--------------------------------------
if debug eq yes
dps 'read file - error!'
newline
end if
;--------------------------------------
	ret
;-----------------------------------
@@:
;--------------------------------------
if debug eq yes
dps 'read file corrected size: '
dpd dword[fileinfo.size]
newline
end if
;--------------------------------------
	ret
;---------------------------------------------------------------------
search_and_load_pointer_file_label:
	mov	[fileinfo.return],dword folder_data_1
	mov	ecx,[retrieved_devices_table_counter]
	dec	ecx	; /rd/1/ no need to check
	mov	[fileinfo.name],dword read_folder_name
	mov	esi,retrieved_devices_table
;	sub	esi,10	; deleted because /rd/1/ no need to check
.next_entry:
;--------------------------------------
if debug eq yes
newline
dps 'copy next entry'
newline
end if
;--------------------------------------
	add	esi,10
	push	esi
	add	esi,1
	call	copy_folder_name
	mov	esi,basic_file_name-1
	dec	edi
	call	copy_folder_name.1
	pop	esi
;--------------------------------------
if debug eq yes
	mov	edx,[fileinfo.name]
	push	ecx
	call	debug_outstr
	pop	ecx
newline
end if
;--------------------------------------
;	mcall	5,10
	push	ecx
	call	load_file
	pop	ecx

	xor	eax,eax
	cmp	[fs_error],eax
	jne	@f
	mov	eax,[fileinfo.size]
	cmp	eax,[basic_file_size]
	jae	.sucess
@@:
	dec	ecx
	jnz	.next_entry
;--------------------------------------
if debug eq yes
dps 'additional parttition is not found!'
newline
end if
;--------------------------------------
	ret
.sucess:
	call	compare_files_and_mount
	cmp	[compare_flag],byte 0
	jne	@b
	ret
;---------------------------------------------------------------------
compare_files_and_mount:
	push	ecx esi
	mov	ecx,[basic_file_size]
	mov	esi,folder_data
	mov	edi,folder_data_1
.next_char:
	cld
	lodsb
	mov	ah,[edi]
	inc	edi
	cmp	al,ah
	jne	.not_match
	dec	ecx
	jnz	.next_char
	mov	[compare_flag],byte 0
	pop	esi ecx
;--------------------------------------
if debug eq yes
dps 'compare files sucess!'
newline
dps 'mount directory:'
newline
	mov	edx,esi
	push	esi
	call	debug_outstr
	pop	esi
newline
end if
;--------------------------------------
; prepare real directory path for mounting
	inc	esi
	mov	edi,f30_3_work_area+64
	call	proc_copy_patch
	dec	edi
	mov	esi,real_additional_dir
	call	proc_copy_patch
; prepare fake directory name
	mov	esi,additional_dir_name
	mov	edi,f30_3_work_area
	call	proc_copy_patch
; here is call kernel function to mount the found partition
; as "/KolibriOS" directory to root directory "/"
	mcall	30,3,f30_3_work_area
	ret
;--------------------------------------
.not_match:
	mov	[compare_flag],byte 1
	pop	esi ecx
;--------------------------------------
if debug eq yes
dps 'compare files is not match!'
newline
end if
;--------------------------------------
	ret
;---------------------------------------------------------------------
copy_folder_name:
	mov	edi,read_folder_name+1
.1:
proc_copy_patch:
	cld
@@:
	lodsb
	stosb
	test	al,al
	jnz	@r
	ret
;---------------------------------------------------------------------
copy_folder_name_1:
	mov	edi,read_folder_1_name+1
	jmp	proc_copy_patch
;---------------------------------------------------------------------
if debug eq yes
print_retrieved_devices_table:
	mov	ecx,[retrieved_devices_table_counter]
	mov	edx,retrieved_devices_table
dps 'retrieved_devices_table:'
newline
dps '----------'
newline
@@:
	push	ecx edx
	call	debug_outstr
newline
	pop	edx ecx
	add	edx,10
	dec	ecx
	jnz	@b
newline
dps '----------'
newline
	ret
;---------------------------------------------------------------------
print_root_dir:
dps '----------'
dps 'root dir:'
dps '----------'
newline
	pusha
	mov	ecx,ebx
	mov	edx,folder_data+32+40
@@:
	push	ecx edx
	call	debug_outstr
newline
	pop	edx ecx
	add	edx,304
	dec	ecx
	jnz	@b
	popa
newline
dps '----------'
newline
	ret
end if
;-------------------------------------------------------------------------------
IM_END:
;-------------------------------------------------------------------------------
align 4
left_folder_block	rd 1
right_folder_block	rd 1
temp_counter_1		rd 1
retrieved_devices_table_counter	rd 1
basic_file_size		rd 1
fs_error		rd 1
compare_flag		rb 1
;-------------------------------------------------------------------------------
align 4
f30_3_work_area:
	rb 128
;-------------------------------------------------------------------------------
align 4
retrieved_devices_table:
	rb 10*100
;-------------------------------------------------------------------------------
align 4
read_folder_name:
	rb 256
;-------------------------------------------------------------------------------
align 4
read_folder_1_name:
	rb 256
;-------------------------------------------------------------------------------
align 4
folder_data:
	rb 304*32+32 ; 9 Kb
;-------------------------------------------------------------------------------
align 4
folder_data_1:
	rb 304*32+32 ; 9 Kb
;-------------------------------------------------------------------------------
align 4
	rb 512
stacktop:
;-------------------------------------------------------------------------------
I_END:
;-------------------------------------------------------------------------------
