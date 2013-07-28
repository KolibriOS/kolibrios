left_sort_files:
;   mcall 26,9
;   mov	 [timer_tick],eax
   cmp   [left_folder_block], 0
   jz    .nothing
   mov	 eax,[left_folder_data]
   add	 eax,32+40
   cmp	 [eax],word '..'
   jne	 @f
   cmp	 [eax+2],byte 0
   jne	 @f
   add	 [left_folder_data],304
   dec	 [left_folder_block]
   mov	 [exit_to_dir],1
   jz    .return_exit_to_dir
@@:
; add memory for sorting
   mov	 ecx,[right_folder_data]
   sub	 ecx,[left_folder_data]
   mov	 ebx,[appl_memory]
   add	 ecx,ebx
   add   ecx,4 ; для страховки
   push  ebx
   mov	 [appl_memory],ecx
   mcall 64,1
   mov	 edi,[esp]
   mov	 ecx,[appl_memory]
   sub	 ecx,edi
   shr	 ecx,2
   mov	 eax,0xffffffff
   cld
   rep	 stosd
   cmp	 [left_sort_flag],0
   je	 left_name_sort
   cmp	 [left_sort_flag],1
   je	 left_type_sort
   jmp	  left_size_date_sort
.move_sorted_file:
; move sorted file area in folder area
   mov	 esi,[esp]
   mov	 edi,[left_folder_data]
   add	 edi,32
   mov	 ecx,[right_folder_data]
   sub	 ecx,edi
   shr	 ecx,2
   cld
   rep	 movsd
; sub memory for sorting
   pop	 ecx
   add   ecx,4 ; для страховки
   mov	 [appl_memory],ecx
   mcall 64,1
   cmp	 [exit_to_dir],0
   je	 @f
.return_exit_to_dir:
   sub	 [left_folder_data],304
   inc	 [left_folder_block]
   mov	 [exit_to_dir],0
@@:
.nothing:
;   mcall 26,9
;   sub	 eax,[timer_tick]
;   mov	 [timer_tick],eax
;   call  draw_window
   ret
;---------------------------------------------------------------------
left_name_sort:
; sorting of files
   call  prepare_data_1
   call  left_name_sort_1
   call  prepare_data_2
   call  left_name_sort_1
.1:
   call  prepare_data_4
   jmp	 left_sort_files.move_sorted_file
left_name_sort_1:
   xor	 ebp,ebp
.copy_folders:
   call  copy_folders_1
   je	 .file
   test  [esi],byte 0x10
   jz	 .continue
   jmp	 @f
.file:
   test  [esi],byte 0x10
   jnz	 .continue
@@:
   call  prepare_data_3
.start:
   push  esi edi
   call  sort_by_name.start
   jae	 .add_counter
.no_add_counter:
   call  move_area_for_bdvk
   pop	 edi esi
   jmp	.end
.add_counter:
   inc	 [sort_counter]
   pop	 edi esi
   call  add_counter
   jae	 .start
.end:
   call  move_bdvk
.continue:
   inc	 ebp
   cmp	 ebp,[left_folder_block]
   jb	 .copy_folders
   ret
;---------------------------------------------------------------------
left_type_sort:
; sorting of files
   call  left_prepare_extension_start_1
   call  prepare_data_1
   call  left_type_sort_1
   call  prepare_data_2
   call  left_type_sort_1
   jmp	 left_name_sort.1
left_type_sort_1:
   xor	 ebp,ebp
.copy_folders:
   call  copy_folders_1
   je	 .file
;   test  [esi],byte 0x10
   test  al,ah
   jz	 .continue
   jmp	 @f
.file:
;   test  [esi],byte 0x10
   test  al,ah
   jnz	 .continue
@@:
   call  prepare_data_3
.start:
   push esi edi
   cmp	 [dir_or_file],1
   je	 @f
   call  prepare_data_5
@@:
   call  sort_by_name.start
   je	 .1
   ja	 .add_counter
.no_add_counter:
;	pushad
;	pushfd
;	dps	'left_type_sort'
;	newline
;	popfd
;	popad
   call  move_area_for_bdvk
   pop	 edi esi
   jmp	.end
.1:
   cmp	 [dir_or_file],1
   je	 .add_counter
   call  sort_by_name
   jbe	 .no_add_counter
.add_counter:
   inc	 [sort_counter]
   pop	 edi esi
   call  add_counter
   jae	 .start
.end:
   call  move_bdvk
.continue:
   inc	 ebp
   cmp	 ebp,[left_folder_block]
   jb	 .copy_folders
   ret
;---------------------------------------------------------------------
left_size_date_sort:
; sorting of files
   call  prepare_data_1
   call  left_size_date_sort_1
   call  prepare_data_2
   call  left_size_date_sort_1
   jmp	 left_name_sort.1
left_size_date_sort_1:
   xor	 ebp,ebp
.copy_folders:
   call  copy_folders_1
   je	 .file
   test  [esi],byte 0x10
   jz	 .continue
   jmp	 @f
.file:
   test  [esi],byte 0x10
   jnz	 .continue
@@:
   call  prepare_data_3
.start:
   push  esi edi
   mov	 eax,[left_sort_flag]
   cmp	 eax,2
   je	 .size_sort
   call  sort_by_date
;   je	 .1
   jae	 .add_counter
   jmp	.no_add_counter
.size_sort:
   cmp	 [dir_or_file],0
   je	 .file_1
   call  sort_by_name.start
   jmp	 @f
.file_1:
   call  sort_by_size
@@:
   jae	 .add_counter
.no_add_counter:
   call  move_area_for_bdvk
   pop	 edi esi
   jmp	.end
;.1:
;   call  sort_by_name.start
;   jbe	 .no_add_counter
.add_counter:
   inc	 [sort_counter]
   pop	 edi esi
   call  add_counter
   jae	 .start
.end:
   call  move_bdvk
.continue:
   inc	 ebp
   cmp	 ebp,[left_folder_block]
   jb	 .copy_folders
   ret
;---------------------------------------------------------------------
left_prepare_extension_start:
   xor	 ebp,ebp
   mov	 esi,[left_folder_data]
   add	 esi,32+40
.start:
   call  prepare_extension_start_all
   cmp	 ebp,[left_folder_block]
   jb	 .start
   ret
;---------------------------------------------------------------------
prepare_extension_start_all:
   push  esi
   call  search_extension_start
   mov	 eax,esi
   pop	 esi
   sub	 eax,esi
   sub	 ebx,esi
   shl	 eax,16
   mov	 ax,bx
   mov	 [esi+300-40],eax
   mov	 [esi+299-40],byte 0
   add	 esi,304
   inc	 ebp
   ret
;---------------------------------------------------------------------
left_prepare_extension_start_1:
   xor	 ebp,ebp
   mov	 edi,[esp+4]
.start:
   xor	 eax,eax
   mov	 [edi+300],eax
   add	 edi,304
   inc	 ebp
   cmp	 ebp,[left_folder_block]
   jb	 .start
   ret
;---------------------------------------------------------------------
;left_files_name_normalize:
;   xor	 ebp,ebp
;   mov	 esi,[left_folder_data]
;   add	 esi,32+40
;.start:
;   push  esi
;   mov	 al,[esi]
;   call  char_toupper
;   mov	 [esi],al
;@@:
;   inc	 esi
;   mov	 al,[esi]
;   test  al,al
;   jz	@f
;   call  char_todown
;   mov	 [esi],al
;   jmp	 @b
;@@:
;   pop	 esi
;   add	 esi,304
;   inc	 ebp
;   cmp	 ebp,[left_folder_block]
;   jb	 .start
;   ret
;---------------------------------------------------------------------
;right_files_name_normalize:
;   xor	 ebp,ebp
;   mov	 esi,[right_folder_data]
;   add	 esi,32+40
;.start:
;   push  esi
;   mov	 al,[esi]
;   call  char_toupper
;   mov	 [esi],al
;@@:
;   inc	 esi
;   mov	 al,[esi]
;   test  al,al
;   jz	@f
;   call  char_todown
;   mov	 [esi],al
;   jmp	 @b
;@@:
;   pop	 esi
;   add	 esi,304
;   inc	 ebp
;   cmp	 ebp,[right_folder_block]
;   jb	 .start
;   ret
;---------------------------------------------------------------------
right_prepare_extension_start:
   xor	 ebp,ebp
   mov	 esi,[right_folder_data]
   add	 esi,32+40
.start:
   call  prepare_extension_start_all
   cmp	 ebp,[right_folder_block]
   jb	 .start
   ret
;---------------------------------------------------------------------
right_prepare_extension_start_1:
   xor	 ebp,ebp
   mov	 edi,[esp+4]
.start:
   xor	 eax,eax
   mov	 [edi+300],eax
   add	 edi,304
   inc	 ebp
   cmp	 ebp,[right_folder_block]
   jb	 .start
   ret
;---------------------------------------------------------------------
prepare_data_1:
   mov	 [dir_or_file],1
   mov	 [dir_temp_counter],0
   ret
;---------------------------------------------------------------------
prepare_data_2:
   mov	 eax,[dir_temp_counter]
   mov	 [dir_temp_counter_1],eax
   imul  eax,304
   add	 [esp+4],eax
   mov	 [dir_or_file],0
   ret
;---------------------------------------------------------------------
prepare_data_3:
   mov	 edi,[esp+4+4]
   add	 esi,40
   add	 edi,40
   mov	 [sort_counter],0
   ret
;---------------------------------------------------------------------
prepare_data_4:
   mov	 eax,[dir_temp_counter_1]
   imul  eax,304
   sub	 [esp+4],eax
   ret
;---------------------------------------------------------------------
prepare_data_5:
   xor	eax,eax
   mov	 ax,[esi+302-40]
   add	 esi,eax
   mov	 ax,[edi+302-40]
   add	 edi,eax
   ret
;---------------------------------------------------------------------
sort_by_name:
   mov	 esi,[esp+8]
   mov	 edi,[esp+4]
.start:
   cld
   lodsd
   mov	 ebx,[edi]  ; bufer
   add	 edi,4
   cmp	 al,bl
   jnz	 @f
   test  bl, bl
   jz	 @f
   cmp	 ah,bh
   jnz	 @f
   test  bh, bh
   jz	 @f
   ror	 eax,16
   ror	 ebx,16
   cmp	 al,bl
   jnz	 @f
   test  bl, bl
   jz	 @f
   cmp	 ah, bh
   jnz	 @f
   test  bh, bh
   jnz	 .start
@@:
   ret
;---------------------------------------------------------------------
sort_by_name_3:
   mov	 esi,[esp+8]
   mov	 edi,[esp+4]
.start:
   cld
   lodsd
   mov	ecx,4
   rep scasb
   jz  .start
   ret
;---------------------------------------------------------------------
sort_by_name_1:
   mov	 esi,[esp+8]
   mov	 edi,[esp+4]
   cmp	 [edi],byte 255
   je	 @f
.start:
   cld
   lodsb
   mov	 ah,[edi]  ; bufer
   inc	 edi
   cmp	 al,ah
   jnz	 @f
   test  ah, ah
   jnz	 .start
@@:
   ret
;---------------------------------------------------------------------
align 4
sort_by_date:
   mov	eax,[esi-12]  ;+28-40]
   cmp	eax,[edi-12]  ;+28-40]
   jnz	.end
   mov	eax,[esi-16]  ;+24-40]
   cmp	eax,[edi-16]  ;+24-40]
.end:
   ret

;mov eax,[esi-12]
;cmp eax,[edi-12]
;jz @f
;ret
;@@:
;mov eax,[esi-16]
;cmp eax,[edi-16]
;ret

;push ecx ; можно опустить, если ecx не жалко
;mov eax,[esi-12]
;cmp eax,[edi-12]
;setnz ch
;adc ch,-2
;mov eax,[esi-16]
;cmp eax,[edi-16]
;setnz cl
;adc cl,-2
;cmp cx,0xFEFE
;pop ecx
;ret
;---------------------------------------------------------------------
align 4
sort_by_size:
   mov	eax,[esi-4]   ;+36-40]
   cmp	eax,[edi-4]   ;+36-40]
   jnz	.end
   mov	eax,[esi-8]   ;+32-40]
   cmp	eax,[edi-8]   ;+32-40]
.end:
   ret

;mov eax,[esi-4]
;cmp eax,[edi-4]
;jz @f
;ret
;@@:
;mov eax,[esi-8]
;cmp eax,[edi-8]
;ret

;push ecx ; можно опустить, если ecx не жалко
;mov eax,[esi-4]
;cmp eax,[edi-4]
;setnz ch
;adc ch,-2
;mov eax,[esi-8]
;cmp eax,[edi-8]
;setnz cl
;adc cl,-2
;cmp cx,0xFEFE
;pop ecx
;ret
;---------------------------------------------------------------------
add_counter:
   add	 edi,304
   mov	 eax,[appl_memory]
   sub	 eax,304-40
   cmp	 eax,edi
   ret
;---------------------------------------------------------------------
copy_folders_1:
   mov	 edi,ebp
   call  multiple_304
;   imul  edi,304
   mov	 esi,edi
   add	 esi,[left_folder_data]
   add	 esi,32
   mov	 ah,0x10
   mov	 al,[esi]
   cmp	 [dir_or_file],0
   ret
;---------------------------------------------------------------------
copy_folders_2:
   mov	 edi,ebp
   call  multiple_304
;   imul  edi,304
   mov	 esi,edi
   add	 esi,[right_folder_data]
   add	 esi,32
   cmp	 [dir_or_file],0
   ret
;---------------------------------------------------------------------
move_bdvk:
   sub	 esi,40
   mov	 edi,[sort_counter]
   call  multiple_304
   add	 edi,[esp+4+4]
   call  move_bdvk_1
   inc	 [dir_temp_counter]
   ret
;---------------------------------------------------------------------
move_bdvk_1:
   mov	 eax,[esi+300]
   mov	 [edi+300],eax
   mov	 ecx,0
   mov	 [edi+299],cl
   mov	 cx,[esi+300]
   add	 ecx,41
   ror	 ecx,2
   mov	 ax,cx
   xor	 cx,cx
   rol	 ecx,2
   mov	 bx,cx
   mov	 cx,ax
   cld
   rep	 movsd
   mov	 cx,bx
   rep	 movsb
   ret
;---------------------------------------------------------------------
multiple_304:
   shl	 edi,4
   mov	 eax,edi
   mov	 ebx,eax
   shl	 edi,5
   shl	 eax,3
   sub	 edi,eax
   mov	 eax,ebx
   shl	 ebx,2
   sub	 edi,ebx
   sub	 edi,eax
   ret
;---------------------------------------------------------------------
move_area_for_bdvk:
   mov	 eax,[dir_temp_counter]
   mov	 edi,[sort_counter]
   test  eax,eax
   je	 .end
   cmp	 eax,edi
   jbe	 .end
   call  multiple_304
;   imul  edi,304
   mov	 ecx,edi
   mov	 edi,[dir_temp_counter]
   add	 edi,2
   call  multiple_304
;   imul  edi,304
   mov	 eax,ecx
   mov	 ebx,[esp+12+4]
   add	 edi,ebx
   mov	 edx,[appl_memory]
   cmp	 edi,edx
   jbe	 @f
   mov	 edi,edx
@@:
   mov	 esi,edi
   sub	 esi,304
   mov	 ecx,edi
   sub	 ecx,ebx
   sub	 ecx,eax
   shr	 ecx,2
;	pushad
;	pushfd
;	dph	[appl_memory]
;	newline
;	dps	'ECX:'
;	dpd	ecx
;	newline
;	dps	'ESI:'
;	dpd	esi
;	newline
;	dps	'EDI:'
;	dpd	edi
;	newline
;	popfd
;	popad
   std
   rep	 movsd
;	pushad
;	pushfd
;	dps	'move_area_for_bdvk END'
;	newline
;	popfd
;	popad
;   mov	 eax,ecx
;   mov	 ebx,304
;   mov	 edx,0
;   div	 ebx
;   mov	 ecx,eax
;   sub	 ecx,2
;   mov	 eax,[appl_memory]
;   mov	 [temp_ecx],edi
;   mov	 [temp_ebx],esi
;   mov	 [temp_eax],eax
;   pusha
;   call  draw_window
;   mcall 5,300
;   popa
;   sub   edi,303
;   sub   esi,303
;@@:
;   push  ecx esi edi
;   call  move_bdvk_1
;   pop	 edi esi ecx
;   sub	 edi,304
;   sub	 esi,304
;   dec	 ecx
;   jnz	 @b
.end:
   ret
;---------------------------------------------------------------------
search_extension_start:
   mov	 edx,esi
   xor	 eax,eax
@@:
   cld
   lodsb
   test  eax,eax
   jnz	 @b
   dec	 esi
   dec	 edx
   push  esi
@@:
   std
   lodsb
   cmp	 esi,edx
   je	 .end
   cmp	 al,'.'
   jnz	 @b
   add	 esi,2
   pop	 ebx
   ret
.end:
   pop	esi
   mov	ebx,esi
   ret
;---------------------------------------------------------------------
right_sort_files:
   cmp   [right_folder_block], 0
   jz    .nothing
   mov	 eax,[right_folder_data]
   add	 eax,32+40
   cmp	 [eax],word '..'
   jne	 @f
   cmp	 [eax+2],byte 0
   jne	 @f
   add	 [right_folder_data],304
   dec	 [right_folder_block]
   mov	 [exit_to_dir],1
   jz    .return_exit_to_dir
@@:
; add memory for sorting
   mov	 ecx,[appl_memory]
   push  ecx
   sub	 ecx,[right_folder_data]
   add	 ecx,[esp]
   add   ecx,4 ; для страховки
   mov	 [appl_memory],ecx
   mcall 64,1
   mov	 edi,[esp]
   mov	 ecx,[appl_memory]
   sub	 ecx,edi
   shr	 ecx,2
   mov	 eax,0xffffffff
   cld
   rep	 stosd
   cmp	 [right_sort_flag],0
   je	 right_name_sort
   cmp	 [right_sort_flag],1
   je	 right_type_sort
   jmp	  right_size_date_sort
.move_sorted_file:
; move sorted file area in folder area
   mov	 esi,[esp]
   mov	 ecx,esi
   mov	 edi,[right_folder_data]
   add	 edi,32
   sub	 ecx,edi
   shr	 ecx,2
   cld
   rep	 movsd
; sub memory for sorting
   pop	 ecx
   add   ecx,4 ; для страховки
   mov	 [appl_memory],ecx
   mcall 64,1
   cmp	 [exit_to_dir],0
   je	 @f
.return_exit_to_dir:
   sub	 [right_folder_data],304
   inc	 [right_folder_block]
   mov	 [exit_to_dir],0
@@:
.nothing:
   ret
;---------------------------------------------------------------------
right_name_sort:
; sorting of files
   call  prepare_data_1
   call  right_name_sort_1
   call  prepare_data_2
   call  right_name_sort_1
.1:
   call  prepare_data_4
   jmp	 right_sort_files.move_sorted_file
right_name_sort_1:
   xor	 ebp,ebp
.copy_folders:
   call  copy_folders_2
   je	 .file
   test  [esi],byte 0x10
   jz	 .continue
   jmp	 @f
.file:
   test  [esi],byte 0x10
   jnz	 .continue
@@:
   call  prepare_data_3
.start:
   push esi edi
   call  sort_by_name.start
   jae	 .add_counter
.no_add_counter:
   call  move_area_for_bdvk
   pop	 edi esi
   jmp	.end
.add_counter:
   inc	 [sort_counter]
   pop	 edi esi
   call  add_counter
   jae	 .start
.end:
   call  move_bdvk
.continue:
   inc	 ebp
   cmp	 ebp,[right_folder_block]
   jb	 .copy_folders
   ret
;---------------------------------------------------------------------
right_type_sort:
; sorting of files
   call  right_prepare_extension_start_1
   call  prepare_data_1
   call  right_type_sort_1
   call  prepare_data_2
   call  right_type_sort_1
   jmp	 right_name_sort.1
right_type_sort_1:
   xor	 ebp,ebp
.copy_folders:
   call  copy_folders_2
   je	 .file
   test  [esi],byte 0x10
   jz	 .continue
   jmp	 @f
.file:
   test  [esi],byte 0x10
   jnz	 .continue
@@:
   call  prepare_data_3
.start:
   push esi edi
   cmp	 [dir_or_file],1
   je	 @f
   call  prepare_data_5
@@:
   call  sort_by_name.start
   je	 .1
   ja	 .add_counter
.no_add_counter:
;	pushad
;	pushfd
;	dps	'left_type_sort'
;	newline
;	popfd
;	popad
   call  move_area_for_bdvk
   pop	 edi esi
   jmp	.end
.1:
   cmp	 [dir_or_file],1
   je	 .add_counter
   call  sort_by_name
   jbe	 .no_add_counter
.add_counter:
   inc	 [sort_counter]
   pop	 edi esi
   call  add_counter
   jae	 .start
.end:
   call  move_bdvk
.continue:
   inc	 ebp
   cmp	 ebp,[right_folder_block]
   jb	 .copy_folders
   ret
;---------------------------------------------------------------------
right_size_date_sort:
; sorting of files
   call  prepare_data_1
   call  right_size_date_sort_1
   call  prepare_data_2
   call  right_size_date_sort_1
   jmp	 right_name_sort.1
right_size_date_sort_1:
   xor	 ebp,ebp
.copy_folders:
   call  copy_folders_2
   je	 .file
   test  [esi],byte 0x10
   jz	 .continue
   jmp	 @f
.file:
   test  [esi],byte 0x10
   jnz	 .continue
@@:
   call  prepare_data_3
.start:
   push  esi edi
   cmp	 [right_sort_flag],2
   je	 .size_sort
   call  sort_by_date
   je	 .1
   ja	 .add_counter
   jmp	.no_add_counter
.size_sort:
   cmp	 [dir_or_file],0
   je	 .file_1
   call  sort_by_name.start
   jmp	 @f
.file_1:
   call  sort_by_size
@@:
   jae	 .add_counter
.no_add_counter:
   call  move_area_for_bdvk
   pop	 edi esi
   jmp	.end
.1:
   call  sort_by_name.start
   jbe	 .no_add_counter
.add_counter:
   inc	 [sort_counter]
   pop	 edi esi
   call  add_counter
   jae	 .start
.end:
   call  move_bdvk
.continue:
   inc	 ebp
   cmp	 ebp,[right_folder_block]
   jb	 .copy_folders
   ret
;---------------------------------------------------------------------
