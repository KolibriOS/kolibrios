;---------------------------------------------------------------------
convert_icons:
	xor	eax,eax
	mov	[return_code],eax

	push	image_file
	call	[cnv_png_import.Start]

	mov	ecx,[image_file]
	mcall	68,13,
	test	eax,eax
	jz	memory_free_error

	cmp	[return_code],dword 0
	je	@f
	mov	[N_error],6
	jmp	button.exit
@@:

;	mov	ebx,[raw_pointer]
;	mov	eax,[ebx+4]
; set of icon size x
;	mov	[file_browser_data_1.icon_size_x],ax
; mov eax,[ebx+8]
; set of icon size y
;	mov	[file_browser_data_1.icon_size_y],ax
;	inc	ax
;	mov	[file_browser_data_1.line_size_y],ax
;	mov	eax,[ebx+12]
; set of RAW resolution to pixel
;	mov	[file_browser_data_1.resolution_raw],eax

;	mov	eax,[ebx+20]
;	add	eax,ebx
; set RAW palette,use else resolution 8bit or less
;	mov	[file_browser_data_1.palette_raw],eax

;	mov	eax,[ebx+28]
;	add	eax,ebx
; set RAW area for icon
;	mov	[file_browser_data_1.icon_raw_area],eax
	ret
;---------------------------------------------------------------------
