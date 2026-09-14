; in:
;  sort_type - 0=им€, 2=расширение, 4=врем€ последней модификации, 6=размер,
;              10=врем€ создани€, 12=врем€ последнего доступа
align 4
proc sort_directory uses eax ebx, tmem:dword, sort_type:dword
	mov	eax,[tmem]
	mov	ebx,[eax+4]	; number of files
	add	eax,32
	cmp	[eax+40],word '..'
	jne	@f
	cmp	[eax+40+2],byte 0
	jne	@f
	dec	ebx
	add	eax,304
@@:
	stdcall	[sort_dir], eax, ebx, [sort_type]
	ret
endp
;---------------------------------------------------------------------
align 4
left_sort_files:
   cmp   [left_folder_block], 0
   jz    @f
   stdcall sort_directory, [left_folder_data], [left_sort_flag]
@@:
   ret
;---------------------------------------------------------------------
align 4
right_sort_files:
   cmp   [right_folder_block], 0
   jz    @f
   stdcall sort_directory, [right_folder_data], [right_sort_flag]
@@:
   ret
;---------------------------------------------------------------------
