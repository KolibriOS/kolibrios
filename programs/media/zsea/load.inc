;---------------------------------------------------------------------
load_plugins:

load_libraries l_libs_start,end_l_libs

	test  eax,eax
	jnz    START.exit_1
;---------------------------------------------------------------------
; Get ini values
	call  Get_ini
;	jmp  last_plugin
;---------------------------------------------------------------------
;	mov [cnv_plugin_name_pointer],plugin_0_name
load_convert_plugins:
	mov	ecx,10
.1:
	push	ecx
	mov esi,[cnv_plugin_name_pointer]
	cmp [esi],dword 'NoKe'
	je  clear_current_load_convert_plugins_area ;last_plugin
	mov edi,system_path.1
	mov ecx,128/4
	cld
	rep movsd
	
	mov esi,[cnv_plugin_name_pointer]
	mov edi,err_message_found_lib.1
	mov ecx,128/4
	rep movsd
	
	mov esi,[cnv_plugin_name_pointer]
	mov edi,err_message_import.1
	mov ecx,128/4
	rep movsd
	
load_library system_path.1, path, library_path, system_path, \
err_message_found_lib, head_f_l, \
[cnv_plugin_import_pointer], err_message_import, head_f_i, plugins_directory

	test  eax,eax
	jnz    START.exit_1
load_convert_plugins_next:	
	add  [cnv_plugin_import_pointer],dword 24
	add  [cnv_plugin_name_pointer],128
	pop	ecx
	dec	ecx
	jnz	load_convert_plugins.1
	
;	jmp  load_convert_plugins
	
last_plugin:
; initialize sort
	push dword 1
	call dword [sort_init]
; unpack deflate
	mov   eax,[unpack_DeflateUnpack2]
	mov  [deflate_unpack],eax
	ret
;-----------------------------------------
clear_current_load_convert_plugins_area:
	xor	eax,eax
	mov	edi,[cnv_plugin_import_pointer]
	cld
	push	ecx
	mov	ecx,24/4
	rep	stosd
	pop	ecx
	jmp	load_convert_plugins_next
;---------------------------------------------------------------------
copy_path_my:
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,edi
@@:
    std
    lodsb
    cmp   al,'/'
    jnz   @b
    mov   edi,esi
    add   edi,2
    mov   esi,ebx
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,ecx
    dec   edi
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    ret
;---------------------------------------------------------------------
load_image:
	
	call delete_RAW1
	mov	[fileinfo+0],dword 5
	mov	[fileinfo+12],dword 0
	mov	[fileinfo+16],dword process_info

    cmp  [soi],0
    je	  @f
    mov   ecx,[raw_pointer]
    mcall 68, 13,
    mov  [soi],0
@@:
;      cmp  [raw_pointer_2],dword 0
;      je	@f
;      mov   ecx,[raw_pointer_2]    ;[soi]
;      mcall 68, 13,
;      mov  [raw_pointer_2],0
;@@:
.1:
	mcall 70, fileinfo
	test  eax,eax
	jnz   .end
 
	mov	[fileinfo+0],dword 0

	mov	ecx,[process_info+32]

; for Diamond emulator, and broken files - size = 0
;	test  ecx,ecx
;	jnz    @f
	cmp  ecx,32
	jae   @f
	mov   eax,6
	jmp	.end
@@:
	mov	 [fileinfo+12],ecx
	mov  [img_size],ecx

	mcall 68, 12

	mov	[fileinfo+16],eax
	mov	[image_file],eax

	mcall 70, fileinfo
.end:
	mov  [error_fs],eax

	ret
;---------------------------------------------------------------------
delete_RAW1:
	cmp  [RAW1_pointer],0
	jne  @f
	ret
@@:
	mcall 68, 13,[previous_frame_pointer]
	xor  eax,eax
	mov  [Repeat_N],eax
	
@@:
	mov  edx,[Repeat_N]
	shl  edx,4
	add  edx,[RAW1_pointer]
	mov  ecx,[edx+16]
	call .1
	
	inc  [Repeat_N]
	mov  eax,[Repeat_N]
	cmp  [Total_Repeat_N],eax
	ja  @r
	
	mov  ecx,[RAW1_pointer]
	call .1
	
	xor   eax,eax
	mov   [RAW1_pointer],eax
	ret
.1:
	mcall 68, 13,
	ret
;---------------------------------------------------------------------
load_buttons:

    mov   esi,path
    mov   edi,plugin_pach
    mov   ebx,buttons_directory
    mov   ecx,button_name ;buttons_passive_name
    call  copy_path_my
	
	mov   [fileinfo.name], dword plugin_pach

	xor  eax,eax
    mov  [raw_pointer_2],eax

	call  load_image
    mov   eax,[error_fs]
	test  eax,eax
	jnz   START.exit_1

	call  convert_plugin_invoke
    mov   eax,[return_code]
	test  eax,eax
    jnz   START.exit_1

	
    mov  ebx,[raw_pointer]
    mov  eax,[ebx+4]
    mov  [dinamic_button_data_1.size_x],ax
    mov  [dinamic_button_data_1.size_y],ax
    mov  [dinamic_button_data_2.size_x],ax
    mov  [dinamic_button_data_2.size_y],ax
    mov  [dinamic_button_data_3.size_x],ax
    mov  [dinamic_button_data_3.size_y],ax
    mov  [dinamic_button_data_4.size_x],ax
    mov  [dinamic_button_data_4.size_y],ax
    mov  [dinamic_button_data_5.size_x],ax
    mov  [dinamic_button_data_5.size_y],ax
    mov  [dinamic_button_data_6.size_x],ax
    mov  [dinamic_button_data_6.size_y],ax
    mov  [dinamic_button_data_7.size_x],ax
    mov  [dinamic_button_data_7.size_y],ax
    mov  [dinamic_button_data_8.size_x],ax
    mov  [dinamic_button_data_8.size_y],ax
    mov  [dinamic_button_data_9.size_x],ax
    mov  [dinamic_button_data_9.size_y],ax
    mov  [dinamic_button_data_10.size_x],ax
    mov  [dinamic_button_data_10.size_y],ax
    mov  [dinamic_button_data_11.size_x],ax
    mov  [dinamic_button_data_11.size_y],ax
    mov  [dinamic_button_data_12.size_x],ax
    mov  [dinamic_button_data_12.size_y],ax
    mov  [dinamic_button_data_13.size_x],ax
    mov  [dinamic_button_data_13.size_y],ax
    mov  [dinamic_button_data_14.size_x],ax
    mov  [dinamic_button_data_14.size_y],ax
    mov  [dinamic_button_data_15.size_x],ax
    mov  [dinamic_button_data_15.size_y],ax
    mov  eax,[ebx+12]
    mov  [dinamic_button_data_1.resolution_raw],eax
    mov  [dinamic_button_data_2.resolution_raw],eax
    mov  [dinamic_button_data_3.resolution_raw],eax
    mov  [dinamic_button_data_4.resolution_raw],eax
    mov  [dinamic_button_data_5.resolution_raw],eax
    mov  [dinamic_button_data_6.resolution_raw],eax
    mov  [dinamic_button_data_7.resolution_raw],eax
    mov  [dinamic_button_data_8.resolution_raw],eax
    mov  [dinamic_button_data_9.resolution_raw],eax
    mov  [dinamic_button_data_10.resolution_raw],eax
    mov  [dinamic_button_data_11.resolution_raw],eax
    mov  [dinamic_button_data_12.resolution_raw],eax
    mov  [dinamic_button_data_13.resolution_raw],eax
    mov  [dinamic_button_data_14.resolution_raw],eax
    mov  [dinamic_button_data_15.resolution_raw],eax
    mov  eax,[ebx+20]
    add  eax,ebx
    mov  [dinamic_button_data_1.palette_raw],eax
    mov  [dinamic_button_data_2.palette_raw],eax
    mov  [dinamic_button_data_3.palette_raw],eax
    mov  [dinamic_button_data_4.palette_raw],eax
    mov  [dinamic_button_data_5.palette_raw],eax
    mov  [dinamic_button_data_6.palette_raw],eax
    mov  [dinamic_button_data_7.palette_raw],eax
    mov  [dinamic_button_data_8.palette_raw],eax
    mov  [dinamic_button_data_9.palette_raw],eax
    mov  [dinamic_button_data_10.palette_raw],eax
    mov  [dinamic_button_data_11.palette_raw],eax
    mov  [dinamic_button_data_12.palette_raw],eax
    mov  [dinamic_button_data_13.palette_raw],eax
    mov  [dinamic_button_data_14.palette_raw],eax
    mov  [dinamic_button_data_15.palette_raw],eax
    mov  eax,[ebx+28]
    add  eax,ebx
	push	eax
    mov  [dinamic_button_data_1.passive_raw],eax
	mov  ecx,[ebx+4]
	imul ecx,[ebx+4]
	imul ecx,[ebx+12]
	shr  ecx,3
	add  eax,ecx
    mov  [dinamic_button_data_2.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_3.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_4.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_5.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_6.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_7.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_8.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_9.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_10.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_11.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_12.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_13.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_14.passive_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_15.passive_raw],eax
;------------------------------------
	movzx	ebx,word [dinamic_button_data_1.size_x]
	movzx	eax,word [dinamic_button_data_1.size_y]
	imul	eax,ebx
	imul	eax,15
	mov	ebx,[dinamic_button_data_1.resolution_raw]
	shr	ebx,3
	imul	ebx,eax
	
	pop	eax
	add	eax,ebx
	push	eax
    mov  [dinamic_button_data_1.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_2.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_3.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_4.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_5.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_6.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_7.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_8.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_9.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_10.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_11.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_12.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_13.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_14.active_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_15.active_raw],eax
;------------------------------------
	pop	eax
	add	eax,ebx
    mov  [dinamic_button_data_1.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_2.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_3.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_4.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_5.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_6.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_7.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_8.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_9.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_10.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_11.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_12.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_13.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_14.click_raw],eax
	add  eax,ecx
    mov  [dinamic_button_data_15.click_raw],eax
;------------------------------------
	mov   [fileinfo.name], dword string
	xor  eax,eax
	mov  [img_size],eax
    ret
;---------------------------------------------------------------------
load_directory:
    cmp   [load_directory_pointer],dir_header ;dword 0
	je    @f
	mcall 68, 13, [load_directory_pointer]	
@@:
	call  .copy_dir_path
	mov   [dirinfo.size],dword 0
	mov	 [dirinfo.return],dir_header  ;procinfo	
	mcall 70, dirinfo
	test  eax,eax
	jnz   .end_err
	
	mov   ecx,[dir_header.totl_blocks]  ;[procinfo+8]
	mov   [dirinfo.size],ecx
	imul  ecx,304
	add   ecx,32
	mcall 68, 12
	mov	 [dirinfo.return],eax
	mov  [load_directory_pointer],eax

	mcall 70, dirinfo
	test  eax,eax
	jnz   .end_err
	call  delete_unsupported_BDFE
.sort:
	call  sort_directory
;	add [load_directory_pointer],2*304+32+40
    mov   ebp,[load_directory_pointer]
	mov   ebp,[ebp+4]
; check ebp = 0
	test  ebp,ebp 
	jz   .end_1  ;.start
	dec   ebp
	jz   .end_1

.start:
    mov   esi,file_name
    mov   ecx,[file_name_length]
	imul  edi,ebp,304
	add   edi,32+40
	add   edi,[load_directory_pointer]
@@:
    cld
	lodsb
	call  char_todown
	xchg  ah,al
	xchg  edi,esi
	cld
	lodsb
	call  char_todown
;	xchg  ah,al
	xchg  edi,esi
	dec  ecx
	jz   .end
	cmp  ah,al
    je   @r
	
    dec  ebp
	jnz  .start
.end_1:
	xor  eax,eax
    mov  [load_file_number],eax
	ret
.end:
    mov  [load_file_number],ebp
	xor  eax,eax
.end_err:
	ret
;-------------------------------------------
.copy_dir_path:
    mov   esi,string  ;path
    mov   edi,plugin_pach
	mov   ebx,file_name
	call  copy_dir_path
	mov  [file_name_length],ebx
	ret
;---------------------------------------------------------------------
sort_directory:
;	pusha
	push  dword [sort_type]  ;2	; режим сортировки
	mov   eax,[load_directory_pointer]
	push  dword [eax+4] ;[dirdata+4]	; число файлов
				; можно писать push ebx, если
				; дело происходит непосредственно после int 0x40
	add   eax,32
	push  eax ;dirdata+32	; данные файлов
;	call  [plugin_sort]
	call  [sort_dir]
;	popa
	ret
;---------------------------------------------------------------------
delete_unsupported_BDFE:

	mov	 [fileinfo.subfunction],dword 0
	mov	 [fileinfo.size],dword 32
	mov  [fileinfo.name], dword plugin_pach
	
	mov  eax,process_info
	mov	 [fileinfo.return],eax
	mov	[image_file],eax

	mov   ebx,[load_directory_pointer]
	add   ebx,4
	xor   ecx,ecx
	dec   ecx
	
	mov   eax,[load_directory_pointer]
	add   eax,32+40
	sub   eax,304
.start:
    inc   ecx
	add   eax,304
.1:
    cmp   [ebx],ecx
	je    .end
	cmp   [eax],byte '.'
	jne   @f
	cmp   [eax+1],byte 0
	je    .delete
@@:
	cmp   [eax],word '..'
	jne   @f
	cmp   [eax+2],byte 0
	je    .delete
@@:
	test   [eax-40],byte 0x10
	jnz   .delete

	push  eax ebx
	mov   esi,eax
	call  search_expansion
	test   eax,eax
	pop   ebx eax
	jnz   .delete
	
;	mov  edi,[test_area+4]
;	cmp  [esi],edi
;	je   .start

	mov edi,plugin_0_name
	cmp [edi],dword 'NoKe'
	je  .delete

	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_0.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_1_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_1.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start

	mov edi,plugin_2_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_2.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start

	mov edi,plugin_3_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_3.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_4_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_4.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_5_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_5.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_6_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_6.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_7_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_7.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_8_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_8.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
	
	mov edi,plugin_9_name
	cmp [edi],dword 'NoKe'
	je  .delete
	
	push  eax ebx ecx esi
	mov   edi,[Convert_plugin_9.Assoc]
	test	edi,edi
	jnz	@f
	pop   esi ecx ebx eax
	jmp	.delete
@@:
	call  compare_expansion
	test   eax,eax
	pop   esi ecx ebx eax
	jz   .start
;-------------------------------------------
.delete:
	dec   dword [ebx]
	mov   esi,[ebx]
	sub   esi,ecx

	push  ecx
	mov   ecx,esi
	imul  ecx,304/4
	mov   edi,eax
	sub   edi,40
	mov   esi,edi
	add   esi,304
	cld
	rep  movsd
	pop   ecx
	
	jmp  .1
.end:
	mov   [fileinfo.name], dword string
	ret
;---------------------------------------------------------------------
image_editor:
	mov   [start_appl.parameter],dword string
	mov   [start_appl.name],dword  Editor_path
	mcall 70, start_appl
    jmp   still
;---------------------------------------------------------------------
open_help:
	mov		ebx,key_help_name
    mov		esi,path
    mov		edi,library_path
    call	copy_file_path
	mov   [start_appl.parameter],dword library_path
	mov   [start_appl.name],dword Help_path
	mcall 70, start_appl	
    jmp   still
;---------------------------------------------------------------------
char_todown:
; convert character to uppercase, using cp866 encoding
; in: al=symbol
; out: al=converted symbol
	cmp	al, 'A'
	jb	.ret
	cmp	al, 'Z'
	jbe	.az
	cmp	al, 'А'
	jb	.ret
	cmp	al, 'Р'
	jb	.rus1
	cmp	al, 'Я'
	ja	.ret
; 0x90-0x9F -> 0xE0-0xEF
	add	al, 'а'-'Р'
.ret:
	ret
.rus1:
; 0x80-0x8F -> 0xA0-0xAF
.az:
	add	al, 0x20
	ret
;---------------------------------------------------------------------
copy_dir_path:
    xor   eax,eax
	cmp  [esi],al
	jne  @f
	mov  [edi],al
	mov  [ebx],al
	xor  ebx,ebx
	inc  ebx
	ret
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,edi
	dec   esi
@@:
    std
    lodsb
    cmp   al,'/'
    jnz   @b
	
	push  esi
	add  esi,2
	mov  edi,ebx  ;file_name
	xor  ebx,ebx
@@:
    cld
    lodsb
    stosb
	inc   ebx
    test  eax,eax
    jnz   @b
	pop   esi
	
    mov  [esi+1], byte 0
    cld
	ret
;---------------------------------------------------------------------
copy_file_path:
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,edi
    dec   esi
@@:
    std
    lodsb
    cmp   al,'/'
    jnz   @b
    mov   edi,esi
    add   edi,2
    mov   esi,ebx
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    ret
;---------------------------------------------------------------------
search_expansion:
    mov   edi,esi
    xor   eax,eax
@@:
    cld
    lodsb
    test  eax,eax
    jnz   @b
	mov   ebx,esi
	dec   esi
@@:
    std
    lodsb
    cmp  esi,edi
	jb   .end_err
    cmp   al,'.'
    jne   @b
	
	add   esi,2
	sub   ebx,esi
	dec   ebx
	mov   [expansion_length],ebx
	cld
	xor   eax,eax
	ret
	
.end_err:
    cld
    xor   eax,eax
	inc   eax
	ret
;---------------------------------------------------------------------
compare_expansion:
	mov	ebx,[edi]
	add	ebx,edi
	add	edi,3
.start:
	cmp	ebx,edi
	jb	.end_err
	mov	ecx,[expansion_length]
	inc	edi
	
	push	esi edi
@@:
	cld
	lodsb
	xchg	esi,edi
	shl	eax,8
	lodsb
	xchg	esi,edi
	call	char_todown
	xchg	al,ah
	call	char_todown
	cmp	al,ah
	jne	@f
	dec	ecx
	jnz	@b
	jmp	.end
@@:
	pop	edi esi
	jmp	.start
	
;	jnz   .start
.end:
	pop	edi esi
	xor	eax,eax
	ret
	
.end_err:
	xor	eax,eax
	inc	eax
	ret
;---------------------------------------------------------------------