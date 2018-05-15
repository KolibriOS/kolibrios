
; pngwrite.asm - general routines to write a PNG file

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc

; Write out all the unknown chunks for the current given location
;void (png_structrp png_ptr, png_const_inforp info_ptr, unsigned int where)
align 4
proc write_unknown_chunks, png_ptr:dword, info_ptr:dword, where:dword
pushad
	mov esi,[info_ptr]
	cmp dword[esi+png_info_def.unknown_chunks_num],0
	je .end_f ;if(..!=0)
		mov edi,[png_ptr]
		;ecx = up

		png_debug 5, 'writing extra chunks'

		mov ecx,[esi+png_info_def.unknown_chunks]
		mov edx,[esi+png_info_def.unknown_chunks_num]
		imul edx,sizeof.png_unknown_chunk
		add edx,ecx
		.cycle0: ;for (..;..<..;..)
			cmp ecx,edx
			jge .end_f
			movzx eax,byte[ecx+png_unknown_chunk.location]
			and eax,[where]
			jz .end0 ;if (..!=0)
			; If per-chunk unknown chunk handling is enabled use it, otherwise
			; just write the chunks the application has set.

if PNG_SET_UNKNOWN_CHUNKS_SUPPORTED eq 1
			mov eax,ecx
			add eax,png_unknown_chunk.name
			stdcall png_handle_as_unknown, edi, eax

			; NOTE: this code is radically different from the read side in the
			; matter of handling an ancillary unknown chunk.  In the read side
			; the default behavior is to discard it, in the code below the default
			; behavior is to write it.  Critical chunks are, however, only
			; written if explicitly listed or if the default is set to write all
			; unknown chunks.

			; The default handling is also slightly weird - it is not possible to
			; stop the writing of all unsafe-to-copy chunks!

			; TODO: REVIEW: this would seem to be a bug.

			cmp eax,PNG_HANDLE_CHUNK_NEVER
			je .end0
			mov bl,byte[ecx+png_unknown_chunk.name+3]
			and bl,0x20
			cmp bl,0
			jne .beg0
			cmp eax,PNG_HANDLE_CHUNK_ALWAYS
			je .beg0
			cmp eax,PNG_HANDLE_CHUNK_AS_DEFAULT
			jne .end0
			cmp dword[edi+png_struct.unknown_default],PNG_HANDLE_CHUNK_ALWAYS
			jne .end0
end if
			.beg0: ;if (..!=.. && (.. safe-to-copy overrides everything || ..==.. || (..==.. && ..==..)))
				; TODO: review, what is wrong with a zero length unknown chunk?
				cmp dword[ecx+png_unknown_chunk.size],0
				jne @f ;if (..==0)
					png_warning [png_ptr], 'Writing zero-length unknown chunk'
				@@:
				mov eax,dword[ecx+png_unknown_chunk.name]
				stdcall png_write_chunk, edi, eax, [ecx+png_unknown_chunk.podata], [ecx+png_unknown_chunk.size]
			.end0:
			add ecx,sizeof.png_unknown_chunk
			jmp .cycle0
		;.cycle0end:
.end_f:
popad
	ret
endp

; Writes all the PNG information.  This is the suggested way to use the
; library.  If you have a new chunk to add, make a function to write it,
; and put it in the correct location here.  If you want the chunk written
; after the image data, put it in png_write_end().  I strongly encourage
; you to supply a PNG_INFO_ flag, and check info_ptr->valid before writing
; the chunk, as that will keep the code from breaking if you want to just
; write a plain PNG file.  If you have long comments, I suggest writing
; them in png_write_end(), and compressing them.

;void (png_structrp png_ptr, png_const_inforp info_ptr)
align 4
proc png_write_info_before_PLTE, png_ptr:dword, info_ptr:dword
	png_debug 1, 'in png_write_info_before_PLTE'

pushad
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f
	mov esi,[info_ptr]
	cmp esi,0
	je .end_f ;if(..==0 || ..==0) return

	mov eax,[edi+png_struct.mode]
	and eax,PNG_WROTE_INFO_BEFORE_PLTE
	jnz .end_f ;if (..==0)

		; Write PNG signature
		stdcall png_write_sig, edi

if PNG_MNG_FEATURES_SUPPORTED eq 1
		mov eax,[edi+png_struct.mode]
		and eax,PNG_HAVE_PNG_SIGNATURE
		jz @f
		cmp dword[edi+png_struct.mng_features_permitted],0
		je @f ;if(..!=0 && ..!=0)
			png_warning edi, 'MNG features are not allowed in a PNG datastream'
			mov dword[edi+png_struct.mng_features_permitted],0
		@@:
end if

	; Write IHDR information.
if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	movzx eax,byte[esi+png_info_def.interlace_type]
	push eax
else
	push dword 0
end if
	movzx eax,byte[esi+png_info_def.filter_type]
	push eax
	movzx eax,byte[esi+png_info_def.compression_type]
	push eax
	movzx eax,byte[esi+png_info_def.color_type]
	push eax
	movzx eax,byte[esi+png_info_def.bit_depth]
	stdcall png_write_IHDR, edi,\
		dword[esi+png_info_def.width], dword[esi+png_info_def.height], eax

; The rest of these check to see if the valid field has the appropriate
; flag set, and if it does, writes the chunk.

; 1.6.0: COLORSPACE support controls the writing of these chunks too, and
; the chunks will be written if the WRITE routine is there and
; information * is available in the COLORSPACE. (See
; png_colorspace_sync_info in png.c for where the valid flags get set.)

; Under certain circumstances the colorspace can be invalidated without
; syncing the info_struct 'valid' flags; this happens if libpng detects
; an error and calls png_error while the color space is being set, yet
; the application continues writing the PNG.  So check the 'invalid'
; flag here too.

if PNG_GAMMA_SUPPORTED eq 1
if PNG_WRITE_gAMA_SUPPORTED eq 1
	movzx eax,word[esi+png_info_def.colorspace.flags]
	and eax,PNG_COLORSPACE_INVALID
	jnz @f
	movzx eax,word[esi+png_info_def.colorspace.flags]
	and eax,PNG_COLORSPACE_FROM_gAMA
	jz @f
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_gAMA
	jz @f ;if (..==0 && ..!=0 && ..!=0)
		stdcall png_write_gAMA_fixed, edi, [esi+png_info_def.colorspace.gamma]
	@@:
end if
end if

if PNG_COLORSPACE_SUPPORTED eq 1
	; Write only one of sRGB or an ICC profile.  If a profile was supplied
	; and it matches one of the known sRGB ones issue a warning.

if PNG_WRITE_iCCP_SUPPORTED eq 1
	movzx eax,word[esi+png_info_def.colorspace.flags]
	and eax,PNG_COLORSPACE_INVALID
	jnz .end0
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_iCCP
	jz .end0 ;if (..==0 && ..!=0)
if PNG_WRITE_sRGB_SUPPORTED eq 1
		mov eax,[esi+png_info_def.valid]
		and eax,PNG_INFO_sRGB
		jz @f ;if (..!=0)
			png_app_warning edi, 'profile matches sRGB but writing iCCP instead'
		@@:
end if
		stdcall png_write_iCCP, edi, [esi+png_info_def.iccp_name],\
			[esi+png_info_def.iccp_profile]
if PNG_WRITE_sRGB_SUPPORTED eq 1
		jmp .end1
end if
	.end0: ;else
end if

if PNG_WRITE_sRGB_SUPPORTED eq 1
	movzx eax,word[esi+png_info_def.colorspace.flags]
	and eax,PNG_COLORSPACE_INVALID
	jnz .end1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_sRGB
	jz .end1 ;if (..==0 && ..!=0)
		movzx eax,word[esi+png_info_def.colorspace.rendering_intent]
		stdcall png_write_sRGB, edi, eax
	.end1:
end if ;sRGB
end if ;COLORSPACE

if PNG_WRITE_sBIT_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_sBIT
	jz @f ;if (..!=0)
		movzx eax,byte[esi+png_info_def.color_type]
		push eax
		mov eax,esi
		add eax,png_info_def.sig_bit
		stdcall png_write_sBIT, edi, eax ;, ...color_type
	@@:
end if

if PNG_COLORSPACE_SUPPORTED eq 1
if PNG_WRITE_cHRM_SUPPORTED eq 1
	movzx eax,word[esi+png_info_def.colorspace.flags]
	and eax,PNG_COLORSPACE_INVALID
	jnz @f
	movzx eax,word[esi+png_info_def.colorspace.flags]
	and eax,PNG_COLORSPACE_FROM_cHRM
	jz @f
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_cHRM
	jz @f ;if (..==0 && ..!=0 && ..!=0)
		stdcall png_write_cHRM_fixed, edi, [esi+png_info_def.colorspace.end_points_xy]
	@@:
end if
end if

if PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED eq 1
		stdcall write_unknown_chunks, edi, esi, PNG_HAVE_IHDR
end if
		or dword[edi+png_struct.mode],PNG_WROTE_INFO_BEFORE_PLTE
	.end_f:
popad
	ret
endp

;void (png_structrp png_ptr, png_const_inforp info_ptr)
align 4
proc png_write_info, png_ptr:dword, info_ptr:dword
if (PNG_WRITE_TEXT_SUPPORTED eq 1) | (PNG_WRITE_sPLT_SUPPORTED eq 1)
;   int i;
end if
pushad
	png_debug 1, 'in png_write_info'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f
	mov esi,[info_ptr]
	cmp esi,0
	je .end_f ;if (..==0 || ..==0) return

	stdcall png_write_info_before_PLTE, edi, esi

	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_PLTE
	jz @f ;if (..!=0)
		movzx eax,word[esi+png_info_def.num_palette]
		stdcall png_write_PLTE, edi, [esi+png_info_def.palette], eax
		jmp .end_0
	@@:
	mov al,byte[esi+png_info_def.color_type]
	cmp al,PNG_COLOR_TYPE_PALETTE
	jne .end_0 ;else if (..==..)
		png_error edi, 'Valid palette required for paletted images'
	.end_0:

if PNG_WRITE_tRNS_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_tRNS
	jz .end_1 ;if (..!=0)
if PNG_WRITE_INVERT_ALPHA_SUPPORTED eq 1
	; Invert the alpha channel (in tRNS)
;      if ((png_ptr->transformations & PNG_INVERT_ALPHA) != 0 &&
;          info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
;      {
;         int j, jend;

;         jend = info_ptr->num_trans;
;         if (jend > PNG_MAX_PALETTE_LENGTH)
;            jend = PNG_MAX_PALETTE_LENGTH;

;         for (j = 0; j<jend; ++j)
;            info_ptr->trans_alpha[j] =
;               (byte)(255 - info_ptr->trans_alpha[j]);
;      }
end if
		mov eax,esi
		add eax,png_info_def.trans_color
		movzx ebx,word[esi+png_info_def.num_trans]
		movzx ecx,byte[esi+png_info_def.color_type]
		stdcall png_write_tRNS, edi, dword[esi+png_info_def.trans_alpha], eax, ebx, ecx
	.end_1:
end if
if PNG_WRITE_bKGD_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_bKGD
	jz @f ;if (..!=0)
		mov eax,esi
		add eax,png_info_def.background
		movzx ebx,byte[esi+png_info_def.color_type]
		stdcall png_write_bKGD, edi, eax, ebx
	@@:
end if

if PNG_WRITE_hIST_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_hIST
	jz @f ;if (..!=0)
		movzx ebx,word[esi+png_info_def.num_palette]
		stdcall png_write_hIST, edi, [esi+png_info_def.hist], ebx
	@@:
end if

if PNG_WRITE_oFFs_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_oFFs
	jz @f ;if (..!=0)
		movzx ebx,byte[esi+png_info_def.offset_unit_type]
		stdcall png_write_oFFs, edi, [esi+png_info_def.x_offset], [esi+png_info_def.y_offset], ebx
	@@:
end if

if PNG_WRITE_pCAL_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_pCAL
	jz @f ;if (..!=0)
		movzx ebx,byte[esi+png_info_def.pcal_type]
		movzx ecx,byte[esi+png_info_def.pcal_nparams]
		stdcall png_write_pCAL, edi, [esi+png_info_def.pcal_purpose], [esi+png_info_def.pcal_X0], [esi+png_info_def.pcal_X1], ebx, ecx, [esi+png_info_def.pcal_units], [esi+png_info_def.pcal_params]
	@@:
end if

if PNG_WRITE_sCAL_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_sCAL
	jz @f ;if (..!=0)
		movzx ebx,byte[esi+png_info_def.scal_unit]
		stdcall png_write_sCAL_s, edi, ebx, [esi+png_info_def.scal_s_width], [esi+png_info_def.scal_s_height]
	@@:
end if ;sCAL

if PNG_WRITE_pHYs_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_pHYs
	jz @f ;if (..!=0)
		movzx ebx,byte[esi+png_info_def.phys_unit_type]
		stdcall png_write_pHYs, edi, [esi+png_info_def.x_pixels_per_unit], [esi+png_info_def.y_pixels_per_unit], ebx
	@@:
end if ;pHYs

if PNG_WRITE_tIME_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_tIME
	jz @f ;if (..!=0)
		mov eax,esi
		add eax,png_info_def.mod_time
		stdcall png_write_tIME, edi, eax
		or [edi+png_struct.mode],PNG_WROTE_tIME
	@@:
end if ;tIME

if PNG_WRITE_sPLT_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_sPLT
	jz @f ;if (..!=0)
		mov eax,[esi+png_info_def.splt_palettes]
		mov ecx,[esi+png_info_def.splt_palettes_num]
		cmp ecx,1
		jl @f
		.cycle0:
			stdcall png_write_sPLT, edi, eax
			add eax,4
			loop .cycle0
	@@:
end if ;sPLT

if PNG_WRITE_TEXT_SUPPORTED eq 1
	; Check to see if we need to write text chunks
;   for (i = 0; i < info_ptr->num_text; i++)
;   {
;      png_debug2(2, "Writing header text chunk %d, type %d", i,
;          info_ptr->text[i].compression);
	; An internationalized chunk?
;      if (info_ptr->text[i].compression > 0)
;      {
if PNG_WRITE_iTXt_SUPPORTED eq 1
	; Write international chunk
;         png_write_iTXt(png_ptr,
;             info_ptr->text[i].compression,
;             info_ptr->text[i].key,
;             info_ptr->text[i].lang,
;             info_ptr->text[i].lang_key,
;             info_ptr->text[i].text);
	; Mark this chunk as written
;         if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_NONE)
;            info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
;         else
;            info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_zTXt_WR;
else
	png_warning edi, 'Unable to write international text'
end if
;      }

	; If we want a compressed text chunk
;      else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_zTXt)
;      {
if PNG_WRITE_zTXt_SUPPORTED eq 1
	; Write compressed chunk
;         png_write_zTXt(png_ptr, info_ptr->text[i].key,
;             info_ptr->text[i].text, info_ptr->text[i].compression);
	; Mark this chunk as written
;         info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_zTXt_WR;
else
	png_warning edi, 'Unable to write compressed text'
end if
;      }

;      else if (info_ptr->text[i].compression == PNG_TEXT_COMPRESSION_NONE)
;      {
if PNG_WRITE_tEXt_SUPPORTED eq 1
	; Write uncompressed chunk
;         png_write_tEXt(png_ptr, info_ptr->text[i].key,
;             info_ptr->text[i].text,
;             0);
	; Mark this chunk as written
;         info_ptr->text[i].compression = PNG_TEXT_COMPRESSION_NONE_WR;
else
	; Can't get here
	png_warning edi, 'Unable to write uncompressed text'
end if
;      }
;   }
end if ;tEXt

if PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED eq 1
	stdcall write_unknown_chunks, edi, esi, PNG_HAVE_PLTE
end if
	.end_f:
popad
	ret
endp

; Writes the end of the PNG file.  If you don't want to write comments or
; time information, you can pass NULL for info.  If you already wrote these
; in png_write_info(), do not write them again here.  If you have long
; comments, I suggest writing them here, and compressing them.

;void (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_write_end, png_ptr:dword, info_ptr:dword
pushad
	png_debug 1, 'in png_write_end'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	mov eax,[edi+png_struct.mode]
	and eax,PNG_HAVE_IDAT
	jnz @f ;if (..==0)
		png_error edi, 'No IDATs written into file'
	@@:

if PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED eq 1
	mov eax,[edi+png_struct.num_palette_max]
	cmp ax,[edi+png_struct.num_palette]
	jle @f ;if (..>..)
		png_benign_error edi, 'Wrote palette index exceeding num_palette'
	@@:
end if

	; See if user wants us to write information chunks
	mov esi,[info_ptr]
	cmp esi,0 
	je .end0 ;if (..!=0)
if PNG_WRITE_TEXT_SUPPORTED eq 1
;      int i; /* local index variable */
end if
if PNG_WRITE_tIME_SUPPORTED eq 1
		; Check to see if user has supplied a time chunk
		mov eax,[esi+png_info_def.valid]
		and eax,PNG_INFO_tIME
		jz @f
		mov eax,[edi+png_struct.mode]
		and eax,PNG_WROTE_tIME
		jnz @f ;if (..!=0 && ..==0)
			mov eax,esi
			add eax,png_info_def.mod_time
			stdcall png_write_tIME, edi, eax
		@@:

end if
if PNG_WRITE_TEXT_SUPPORTED eq 1
		; Loop through comment chunks
		cmp dword[esi+png_info_def.num_text],0
		jle .cycle0end
		xor ecx,ecx
align 4
		.cycle0: ;for (i = 0; i < info_ptr->num_text; i++)

;         png_debug2(2, "Writing trailer text chunk %d, type %d", i,
;             info_ptr->text[i].compression);
			; An internationalized chunk?
			mov eax,ecx
			shl eax,2
			add eax,[esi+png_info_def.text] ;eax = info_ptr.text[i]
			cmp dword[eax+png_text.compression],0
			jle .end1 ;if (info_ptr.text[i].compression > 0)
if PNG_WRITE_iTXt_SUPPORTED eq 1
				; Write international chunk
				stdcall png_write_iTXt, edi,\
					[eax+png_text.compression],\
					[eax+png_text.key],\
					[eax+png_text.lang],\
					[eax+png_text.lang_key],\
					[eax+png_text.text]
				; Mark this chunk as written
				mov ebx,PNG_TEXT_COMPRESSION_zTXt_WR
				cmp dword[eax+png_text.compression],PNG_TEXT_COMPRESSION_NONE
				jne @f
					mov ebx,PNG_TEXT_COMPRESSION_NONE_WR
				@@:
				mov dword[eax+png_text.compression],ebx
else
				png_warning edi, 'Unable to write international text'
end if
				jmp .end3
			.end1:
			cmp dword[eax+png_text.compression],PNG_TEXT_COMPRESSION_zTXt
			jl .end2 ;else if (info_ptr.text[i].compression >= ..)
if PNG_WRITE_zTXt_SUPPORTED eq 1
				; Write compressed chunk
				stdcall png_write_zTXt, edi, [eax+png_text.key],\
					[eax+png_text.text], [eax+png_text.compression]
				; Mark this chunk as written
				mov dword[eax+png_text.compression],PNG_TEXT_COMPRESSION_zTXt_WR
else
				png_warning edi, 'Unable to write compressed text'
end if
				jmp .end3
			.end2:
			cmp dword[eax+png_text.compression],PNG_TEXT_COMPRESSION_NONE
			jl .end3 ;else if (info_ptr.text[i].compression == ..)
if PNG_WRITE_tEXt_SUPPORTED eq 1
				; Write uncompressed chunk
				stdcall png_write_tEXt, edi, [eax+png_text.key],\
					[eax+png_text.text], 0
				; Mark this chunk as written
				mov dword[eax+png_text.compression],PNG_TEXT_COMPRESSION_NONE_WR
else
				png_warning edi, 'Unable to write uncompressed text'
end if
			.end3:

			inc ecx
			cmp ecx,[esi+png_info_def.num_text]
			jl .cycle0
		.cycle0end:
end if
if PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED eq 1
		stdcall write_unknown_chunks, edi, esi, PNG_AFTER_IDAT
end if
	.end0:

	or dword[edi+png_struct.mode], PNG_AFTER_IDAT

	; Write end of PNG file
	stdcall png_write_IEND, edi

; This flush, added in libpng-1.0.8, removed from libpng-1.0.9beta03,
; and restored again in libpng-1.2.30, may cause some applications that
; do not set png_ptr->output_flush_fn to crash.  If your application
; experiences a problem, please try building libpng with
; PNG_WRITE_FLUSH_AFTER_IEND_SUPPORTED defined, and report the event to
; png-mng-implement at lists.sf.net .

if PNG_WRITE_FLUSH_SUPPORTED eq 1
if PNG_WRITE_FLUSH_AFTER_IEND_SUPPORTED eq 1
	stdcall png_flush, edi
end if
end if
.end_f:
popad
	ret
endp

;void (png_timep ptime, struct tm * ttime)
align 4
proc png_convert_from_struct_tm, ptime:dword, ttime:dword
	png_debug 1, 'in png_convert_from_struct_tm'

;   ptime->year = (uint_16)(1900 + ttime->tm_year);
;   ptime->month = (byte)(ttime->tm_mon + 1);
;   ptime->day = (byte)ttime->tm_mday;
;   ptime->hour = (byte)ttime->tm_hour;
;   ptime->minute = (byte)ttime->tm_min;
;   ptime->second = (byte)ttime->tm_sec;
	ret
endp

;void (png_timep ptime, time_t ttime)
align 4
proc png_convert_from_time_t, ptime:dword, ttime:dword
;   struct tm *tbuf;

	png_debug 1, 'in png_convert_from_time_t'

;   tbuf = gmtime(&ttime);
;   png_convert_from_struct_tm(ptime, tbuf);
	ret
endp

; Initialize png_ptr structure, and allocate any memory needed
;png_structp (charp user_png_ver, voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn)
align 4
proc png_create_write_struct, user_png_ver:dword, error_ptr:dword, error_fn:dword, warn_fn:dword
if PNG_USER_MEM_SUPPORTED eq 1
	stdcall png_create_png_struct, [user_png_ver], [error_ptr], [error_fn], [warn_fn], 0, 0, 0
	;eax = png_ptr
end if ;USER_MEM
	test eax,eax
	jz .end0 ;if (..!=0)
		; Set the zlib control values to defaults; they can be overridden by the
		; application after the struct has been created.

		mov dword[eax+png_struct.zbuffer_size], PNG_ZBUF_SIZE

		; The 'zlib_strategy' setting is irrelevant because png_default_claim in
		; pngwutil.asm defaults it according to whether or not filters will be
		; used, and ignores this setting.

		mov dword[eax+png_struct.zlib_strategy], PNG_Z_DEFAULT_STRATEGY
		mov dword[eax+png_struct.zlib_level], PNG_Z_DEFAULT_COMPRESSION
		mov dword[eax+png_struct.zlib_mem_level], 8
		mov dword[eax+png_struct.zlib_window_bits], 15
		mov dword[eax+png_struct.zlib_method], 8

if PNG_WRITE_COMPRESSED_TEXT_SUPPORTED eq 1
		mov dword[eax+png_struct.zlib_text_strategy], PNG_TEXT_Z_DEFAULT_STRATEGY
		mov dword[eax+png_struct.zlib_text_level], PNG_TEXT_Z_DEFAULT_COMPRESSION
		mov dword[eax+png_struct.zlib_text_mem_level], 8
		mov dword[eax+png_struct.zlib_text_window_bits], 15
		mov dword[eax+png_struct.zlib_text_method], 8
end if
		; This is a highly dubious configuration option; by default it is off,
		; but it may be appropriate for private builds that are testing
		; extensions not conformant to the current specification, or of
		; applications that must not fail to write at all costs!

if PNG_BENIGN_WRITE_ERRORS_SUPPORTED eq 1
		; In stable builds only warn if an application error can be completely
		; handled.

		or dword[eax+png_struct.flags], PNG_FLAG_BENIGN_ERRORS_WARN
end if
		; App warnings are warnings in release (or release candidate) builds but
		; are errors during development.

if PNG_RELEASE_BUILD eq 1
		or dword[eax+png_struct.flags], PNG_FLAG_APP_WARNINGS_WARN
end if
		; TODO: delay this, it can be done in png_init_io() (if the app doesn't
		; do it itself) avoiding setting the default function if it is not
		; required.

		stdcall png_set_write_fn, eax, 0, 0, 0
	.end0:
	ret
endp


; Write a few rows of image data.  If the image is interlaced,
; either you will have to write the 7 sub images, or, if you
; have called png_set_interlace_handling(), you will have to
; "write" the image seven times.

;void (png_structrp png_ptr, bytepp row, uint_32 num_rows)
align 4
proc png_write_rows uses ebx ecx edi, png_ptr:dword, row:dword, num_rows:dword
;locals
	;i dd ? ;uint_32 ;row counter
	;rp dd ? ;bytepp ;row pointer
;endl
	png_debug 1, 'in png_write_rows'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if(..==0) return

	; Loop through the rows
	mov ecx,[num_rows]
	cmp ecx,1
	jl .end_f
	mov ebx,[row]
	@@: ;for (i = 0, rp = row; i < num_rows; i++, rp++)
		stdcall png_write_row, edi, [ebx]
		add ebx,4
		loop @b
.end_f:
	ret
endp

; Write the image.  You only need to call this function once, even
; if you are writing an interlaced image.

;void (png_structrp png_ptr, bytepp image)
align 4
proc png_write_image, png_ptr:dword, image:dword
pushad
;ebx ;bytepp ;points to current row
;ecx ;uint_32 ;row index
;edx ;int ;pass
;esi ;int ;num_pass
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	png_debug 1, 'in png_write_image'

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	; Initialize interlace handling.  If image is not interlaced,
	; this will set pass to 1

	stdcall png_set_interlace_handling, edi
	mov esi,eax
else
	xor esi,esi
	inc esi
end if
	; Loop through passes
	xor edx,edx
	.cycle0: ;for (edx = 0; edx < esi; edx++)
		cmp edx,esi
		jge .cycle0end
		; Loop through image
		mov ebx,[image]
		xor ecx,ecx
		.cycle1: ;for (ecx = 0, ebx = image; ecx < png_ptr.height; ecx++, ebx++)
			stdcall png_write_row, edi,[ebx]
			inc ecx
			add ebx,4
			cmp ecx,[edi+png_struct.height]
			jl .cycle1
		;.cycle1end:
		inc edx
		jmp .cycle0
	.cycle0end:
.end_f:
popad
	ret
endp

; Performs intrapixel differencing
;void (png_row_infop row_info, bytep row)
align 4
proc png_do_write_intrapixel uses eax ebx ecx edx edi, row_info:dword, row:dword
	png_debug 1, 'in png_do_write_intrapixel'

	mov ebx,[row_info]
	movzx eax,byte[ebx+png_row_info.color_type]
	and eax,PNG_COLOR_MASK_COLOR
	jz .end_f ;if (..!=0)
		;edx = bytes_per_pixel
		mov ecx,[ebx+png_row_info.width] ;ecx = row_width
		cmp byte[ebx+png_row_info.bit_depth],8 ;if (..==8)
		jne .end0
;         bytep rp;
;         uint_32 i;

			cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_RGB
			jne @f ;if (..==..)
				mov edx,3-1 ;(-1) for stosb
				jmp .end2
			@@:
			cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_RGB_ALPHA
			jne @f ;else if (..==..)
				mov edx,4-1 ;(-1) for stosb
				jmp .end2
			@@:
				jmp .end_f ;else return
			.end2:

			mov edi,[row]
align 4
			.cycle0: ;for (i = 0, rp = row; i < row_width; i++, rp += bytes_per_pixel)
				mov ax,word[edi]
				sub al,ah
				stosb ;*(rp) = (byte)(*rp - *(rp + 1))
				mov ax,word[edi]
				sub ah,al
				mov byte[edi+1],ah ;*(rp + 2) = (byte)(*(rp + 2) - *(rp + 1))
				add edi,edx
				loop .cycle0
			.cycle0end:
			jmp .end_f
		.end0:

if PNG_WRITE_16BIT_SUPPORTED eq 1
		cmp byte[ebx+png_row_info.bit_depth],16 ;else if (..==16)
		jne .end1
;         bytep rp;
;         uint_32 i;

			cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_RGB
			jne @f ;if (..==..)
				mov edx,6
				jmp .end3
			@@:
			cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_RGB_ALPHA
			jne @f ;else if (..==..)
				mov edx,8
				jmp .end3
			@@:
				jmp .end_f ;else return
			.end3:

			mov edi,[row]
align 4
			.cycle1: ;for (i = 0, rp = row; i < row_width; i++, rp += bytes_per_pixel)
;            uint_32 s0   = (*(rp    ) << 8) | *(rp + 1);
;            uint_32 s1   = (*(rp + 2) << 8) | *(rp + 3);
;            uint_32 s2   = (*(rp + 4) << 8) | *(rp + 5);
;            uint_32 red  = (uint_32)((s0 - s1) & 0xffffL);
;            uint_32 blue = (uint_32)((s2 - s1) & 0xffffL);
;            *(rp    ) = (byte)(red >> 8);
;            *(rp + 1) = (byte)red;
;            *(rp + 4) = (byte)(blue >> 8);
;            *(rp + 5) = (byte)blue;
				add edi,edx
				loop .cycle1
			.cycle1end:
		.end1:
end if ;WRITE_16BIT
.end_f:
	ret
endp

; Called by user to write a row of image data
;void (png_structrp png_ptr, bytep row)
align 4
proc png_write_row, png_ptr:dword, row:dword
locals
	; 1.5.6: moved from png_struct to be a local structure:
	row_info png_row_info
endl
pushad
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if(..==0) return

;   png_debug2(1, "in png_write_row (row %u, pass %d)",
;       png_ptr->row_number, png_ptr->pass);
png_debug1 1, 'in png_write_row (row %u)',[edi+png_struct.row_number]

	; Initialize transformations and other stuff if first time
	cmp dword[edi+png_struct.row_number],0
	jne .end0
	cmp byte[edi+png_struct.pass],0
	jne .end0 ;if(..==0 && ..==0)

	; Make sure we wrote the header info
	mov eax,[edi+png_struct.mode]
	and eax,PNG_WROTE_INFO_BEFORE_PLTE
	jnz @f ;if(..==0)
		png_error edi, 'png_write_info was never called before png_write_row'
	@@:

	; Check for transforms that have been set but were defined out
if (PNG_WRITE_INVERT_SUPPORTED eq 0) & (PNG_READ_INVERT_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_INVERT_MONO
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_INVERT_SUPPORTED is not defined'
	@@:
end if

if (PNG_WRITE_FILLER_SUPPORTED eq 0) & (PNG_READ_FILLER_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_FILLER
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_FILLER_SUPPORTED is not defined'
	@@:
end if

if (PNG_WRITE_PACKSWAP_SUPPORTED eq 0) & (PNG_READ_PACKSWAP_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_PACKSWAP
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_PACKSWAP_SUPPORTED is not defined'
	@@:
end if

if (PNG_WRITE_PACK_SUPPORTED eq 0) & (PNG_READ_PACK_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_PACK
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_PACK_SUPPORTED is not defined'
	@@:
end if

if (PNG_WRITE_SHIFT_SUPPORTED eq 0) & (PNG_READ_SHIFT_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_SHIFT
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_SHIFT_SUPPORTED is not defined'
	@@:
end if

if (PNG_WRITE_BGR_SUPPORTED eq 0) & (PNG_READ_BGR_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_BGR
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_BGR_SUPPORTED is not defined'
	@@:
end if

if (PNG_WRITE_SWAP_SUPPORTED eq 0) & (PNG_READ_SWAP_SUPPORTED eq 1)
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_SWAP_BYTES
	jz @f ;if(..!=0)
		png_warning edi, 'PNG_WRITE_SWAP_SUPPORTED is not defined'
	@@:
end if

	stdcall png_write_start_row, edi
	.end0:

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	; If interlaced and not interested in row, return
	cmp byte[edi+png_struct.interlaced],0
	je .end1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_INTERLACE
	jz .end1 ;if(..!=0 && ..!=0)
		cmp byte[edi+png_struct.pass],0
		jne @f
			mov eax,[edi+png_struct.row_number]
			and eax,0x07
			jz .end1 ;if (..!=0)
				stdcall png_write_finish_row, edi
				jmp .end_f
		@@:
		cmp byte[edi+png_struct.pass],1
		jne @f
			mov eax,[edi+png_struct.row_number]
			and eax,0x07
			jnz .end2
			cmp dword[edi+png_struct.width],5
			jge .end1 ;if (..!=0 || ..<..)
			.end2:
				stdcall png_write_finish_row, edi
				jmp .end_f
		@@:
		cmp byte[edi+png_struct.pass],2
		jne @f
			mov eax,[edi+png_struct.row_number]
			and eax,0x07
			cmp eax,4
			je .end1 ;if (..!=..)
				stdcall png_write_finish_row, edi
				jmp .end_f
		@@:
		cmp byte[edi+png_struct.pass],3
		jne @f
			mov eax,[edi+png_struct.row_number]
			and eax,0x03
			jnz .end3
			cmp dword[edi+png_struct.width],3
			jge .end1 ;if (..!=0 || ..<..)
			.end3:
				stdcall png_write_finish_row, edi
				jmp .end_f
		@@:
		cmp byte[edi+png_struct.pass],4
		jne @f
			mov eax,[edi+png_struct.row_number]
			and eax,0x03
			cmp eax,2
			je .end1 ;if (..!=..)
				stdcall png_write_finish_row, edi
				jmp .end_f
		@@:
		cmp byte[edi+png_struct.pass],5
		jne @f
			mov eax,[edi+png_struct.row_number]
			and eax,0x01
			jnz .end4
			cmp dword[edi+png_struct.width],2
			jge .end1 ;if (..!=0 || ..<..)
			.end4:
				stdcall png_write_finish_row, edi
				jmp .end_f
		@@:
		cmp byte[edi+png_struct.pass],6
		jne .end1
			mov eax,[edi+png_struct.row_number]
			and eax,0x01
			jnz .end1 ;if (..==0)
				stdcall png_write_finish_row, edi
				jmp .end_f
	.end1:
end if

	; Set up row info for transformations
	mov ebx,ebp
	sub ebx,sizeof.png_row_info
	mov al,byte[edi+png_struct.color_type]
	mov byte[ebx+png_row_info.color_type],al
	mov eax,[edi+png_struct.usr_width]
	mov [ebx+png_row_info.width],eax
	movzx eax,byte[edi+png_struct.usr_channels]
	mov byte[ebx+png_row_info.channels],al
	movzx ecx,byte[edi+png_struct.usr_bit_depth]
	mov byte[ebx+png_row_info.bit_depth],cl
	imul eax,ecx ;.bit_depth * .channels
	mov byte[ebx+png_row_info.pixel_depth],al
	PNG_ROWBYTES eax, [ebx+png_row_info.width]
	mov [ebx+png_row_info.rowbytes], eax

	push eax
	movzx eax,byte[ebx+png_row_info.color_type]
	png_debug1 3, 'row_info->color_type = %d', eax
	png_debug1 3, 'row_info->width = %u', [ebx+png_row_info.width]
	movzx eax,byte[ebx+png_row_info.channels]
	png_debug1 3, 'row_info->channels = %d', eax
	movzx eax,byte[ebx+png_row_info.bit_depth]
	png_debug1 3, 'row_info->bit_depth = %d', eax
	movzx eax,byte[ebx+png_row_info.pixel_depth]
	png_debug1 3, 'row_info->pixel_depth = %d', eax
	png_debug1 3, 'row_info->rowbytes = %lu', [ebx+png_row_info.rowbytes]
	pop eax

	; Copy user's row into buffer, leaving room for filter byte.
	push edi
	mov edi,[edi+png_struct.row_buf]
	inc edi
	mov esi,[row]
	mov ecx,eax
	rep movsb ;memcpy(...
	pop edi

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	; Handle interlacing
	cmp byte[edi+png_struct.interlaced],0
	je @f
	cmp byte[edi+png_struct.pass],6
	jge @f
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_INTERLACE
	jz @f ;if (.. && ..<.. && ..!=0)
		movzx eax,byte[edi+png_struct.pass]
		push eax
		mov eax,[edi+png_struct.row_buf]
		inc eax
		stdcall png_do_write_interlace, ebx, eax ;, ...pass
		; This should always get caught above, but still ...
		cmp dword[ebx+png_row_info.width],0
		jne @f ;if (..==0)
			stdcall png_write_finish_row, edi
			jmp .end_f
	@@:
end if

if PNG_WRITE_TRANSFORMS_SUPPORTED eq 1
	; Handle other transformations
	cmp dword[edi+png_struct.transformations],0
	je @f ;if (..!=0)
		stdcall png_do_write_transformations, edi, ebx
	@@:
end if

	; At this point the row_info pixel depth must match the 'transformed' depth,
	; which is also the output depth.

	mov al,[ebx+png_row_info.pixel_depth]
	cmp al,[edi+png_struct.pixel_depth]
	jne @f
	cmp al,[edi+png_struct.transformed_pixel_depth]
	je .end5
	@@: ;if (..!=.. || ..!=..)
		png_error edi, 'internal write transform logic error'
	.end5:

if PNG_MNG_FEATURES_SUPPORTED eq 1
	; Write filter_method 64 (intrapixel differencing) only if
	; 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
	; 2. Libpng did not write a PNG signature (this filter_method is only
	;    used in PNG datastreams that are embedded in MNG datastreams) and
	; 3. The application called png_permit_mng_features with a mask that
	;    included PNG_FLAG_MNG_FILTER_64 and
	; 4. The filter_method is 64 and
	; 5. The color_type is RGB or RGBA

	mov eax,[edi+png_struct.mng_features_permitted]
	and eax,PNG_FLAG_MNG_FILTER_64
	jz @f
	cmp byte[edi+png_struct.filter_type],PNG_INTRAPIXEL_DIFFERENCING
	jne @f ;if (..!=0 && ..==..)
		; Intrapixel differencing
		mov eax,[edi+png_struct.row_buf]
		inc eax
		stdcall png_do_write_intrapixel, ebx, eax
	@@:
end if

; Added at libpng-1.5.10
if PNG_WRITE_CHECK_FOR_INVALID_INDEX_SUPPORTED eq 1
	; Check for out-of-range palette index
	cmp byte[ebx+png_row_info.color_type],PNG_COLOR_TYPE_PALETTE
	jne @f
	cmp dword[edi+png_struct.num_palette_max],0
	jl @f ;if (..==.. && ..>=0)
		stdcall png_do_check_palette_indexes, edi, ebx
	@@:
end if

	; Find a filter if necessary, filter the row and write it out.
	mov ebx,ebp
	sub ebx,sizeof.png_row_info
	stdcall png_write_find_filter, edi, ebx

	cmp dword[edi+png_struct.write_row_fn],0
	je .end_f ;if (..!=0)
		movzx eax,byte[edi+png_struct.pass]
		stdcall dword[edi+png_struct.write_row_fn], edi, [edi+png_struct.row_number], eax
.end_f:
popad
	ret
endp

; Set the automatic flush interval or 0 to turn flushing off
;void (png_structrp png_ptr, int nrows)
align 4
proc png_set_flush uses eax edi, png_ptr:dword, nrows:dword
	png_debug 1, 'in png_set_flush'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	mov eax,[nrows]
	cmp eax,0
	jge @f ;(nrows < 0 ? 0 : nrows)
		xor eax,eax
	@@: 
	mov [edi+png_struct.flush_dist],eax
.end_f:
	ret
endp

; Flush the current output buffers now
;void (png_structrp png_ptr)
align 4
proc png_write_flush uses eax edi, png_ptr:dword
	png_debug 1, 'in png_write_flush'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	; We have already written out all of the data
	mov eax,[edi+png_struct.num_rows]
	cmp [edi+png_struct.row_number],eax
	jge .end_f ;if (..>=..) return

	stdcall png_compress_IDAT, 0, 0, Z_SYNC_FLUSH
	mov dword[edi+png_struct.flush_rows],0
	stdcall png_flush, edi
.end_f:
popad
	ret
endp

; Free any memory used in png_ptr struct without freeing the struct itself.
;void (png_structrp png_ptr)
align 4
proc png_write_destroy uses eax edi, png_ptr:dword
	png_debug 1, 'in png_write_destroy'

	; Free any memory zlib uses
	mov edi,[png_ptr]
	mov eax,[edi+png_struct.flags]
	and eax,PNG_FLAG_ZSTREAM_INITIALIZED
	jz @f ;if (..!=0)
		mov eax,edi
		add eax,png_struct.zstream
		stdcall [deflateEnd], eax
	@@:

	; Free our memory.  png_free checks NULL for us.
	mov eax,edi
	add eax,png_struct.zbuffer_list
	stdcall png_free_buffer_list, edi, eax
	stdcall png_free, edi, [edi+png_struct.row_buf]
	mov dword[edi+png_struct.row_buf],0
if PNG_WRITE_FILTER_SUPPORTED eq 1
	stdcall png_free, edi, [edi+png_struct.prev_row]
	stdcall png_free, edi, [edi+png_struct.try_row]
	stdcall png_free, edi, [edi+png_struct.tst_row]
	mov dword[edi+png_struct.prev_row],0
	mov dword[edi+png_struct.try_row],0
	mov dword[edi+png_struct.tst_row],0
end if

if PNG_SET_UNKNOWN_CHUNKS_SUPPORTED eq 1
	stdcall png_free, edi, [edi+png_struct.chunk_list]
	mov dword[edi+png_struct.chunk_list],0
end if

	; The error handling and memory handling information is left intact at this
	; point: the jmp_buf may still have to be freed.  See png_destroy_png_struct
	; for how this happens.
	ret
endp

; Free all memory used by the write.
; In libpng 1.6.0 this API changed quietly to no longer accept a NULL value for
; *png_ptr_ptr.  Prior to 1.6.0 it would accept such a value and it would free
; the passed in info_structs but it would quietly fail to free any of the data
; inside them.  In 1.6.0 it quietly does nothing (it has to be quiet because it
; has no png_ptr.)

;void (png_structpp png_ptr_ptr, png_infopp info_ptr_ptr)
align 4
proc png_destroy_write_struct uses edi esi, png_ptr_ptr:dword, info_ptr_ptr:dword
	png_debug 1, 'in png_destroy_write_struct'

	mov esi,[png_ptr_ptr]
	cmp esi,0
	je @f ;if (..!=0)
		mov edi,[esi]

		cmp edi,0
		je @f ;if (..!=0) ;added in libpng 1.6.0
			stdcall png_destroy_info_struct, edi, [info_ptr_ptr]

			mov dword[esi],0
			stdcall png_write_destroy, edi
			stdcall png_destroy_png_struct, edi
	@@:
	ret
endp

; Allow the application to select one or more row filters to use.
;void (png_structrp png_ptr, int method, int filters)
align 4
proc png_set_filter uses eax ebx ecx edi, png_ptr:dword, method:dword, filters:dword
	png_debug 1, 'in png_set_filter'
pushad
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

if PNG_MNG_FEATURES_SUPPORTED eq 1
	mov eax,[edi+png_struct.mng_features_permitted]
	and eax,PNG_FLAG_MNG_FILTER_64
	je @f
	cmp dword[method], PNG_INTRAPIXEL_DIFFERENCING
	jne @f ;if (..!=0 && ..==..)
		mov dword[method], PNG_FILTER_TYPE_BASE
	@@:
end if
	cmp dword[method], PNG_FILTER_TYPE_BASE
	jne .end0 ;if (..==..)
	mov ebx,[filters]
	and ebx,PNG_ALL_FILTERS or 0x07 ;switch (..)

if PNG_WRITE_FILTER_SUPPORTED eq 1
	cmp ebx,5
	je .end2
	cmp ebx,6
	je .end2
	cmp ebx,7
	je .end2
		jmp @f
	.end2:
		cStr ,'Unknown row filter for method 0'
		stdcall png_app_error, edi, eax
		; FALL THROUGH
	@@:
end if ;WRITE_FILTER
	cmp ebx,PNG_FILTER_VALUE_NONE
	jne @f
		mov byte[edi+png_struct.do_filter],PNG_FILTER_NONE
		jmp .end1
	@@:
if PNG_WRITE_FILTER_SUPPORTED eq 1
	cmp ebx,PNG_FILTER_VALUE_SUB
	jne @f
		mov byte[edi+png_struct.do_filter],PNG_FILTER_SUB
		jmp .end1
	@@:
	cmp ebx,PNG_FILTER_VALUE_UP
	jne @f
		mov byte[edi+png_struct.do_filter],PNG_FILTER_UP
		jmp .end1
	@@:
	cmp ebx,PNG_FILTER_VALUE_AVG
	jne @f
		mov byte[edi+png_struct.do_filter],PNG_FILTER_AVG
		jmp .end1
	@@:
	cmp ebx,PNG_FILTER_VALUE_PAETH
	jne @f
		mov byte[edi+png_struct.do_filter],PNG_FILTER_PAETH
		jmp .end1
	@@: ;default:
		mov eax,[filters]
		mov byte[edi+png_struct.do_filter],al
		jmp .end1
else
	@@: ;default:
		cStr ,'Unknown row filter for method 0'
		stdcall png_app_error edi, eax
end if ;WRITE_FILTER
	.end1:

if PNG_WRITE_FILTER_SUPPORTED eq 1
	; If we have allocated the row_buf, this means we have already started
	; with the image and we should have allocated all of the filter buffers
	; that have been selected.  If prev_row isn't already allocated, then
	; it is too late to start using the filters that need it, since we
	; will be missing the data in the previous row.  If an application
	; wants to start and stop using particular filters during compression,
	; it should start out with all of the filters, and then remove them
	; or add them back after the start of compression.

		; NOTE: this is a nasty constraint on the code, because it means that the
		; prev_row buffer must be maintained even if there are currently no
		; 'prev_row' requiring filters active.

		cmp dword[edi+png_struct.row_buf],0
		je .end3 ;if (..!=0)
			;ebx = num_filters
			;ecx = buf_size

			; Repeat the checks in png_write_start_row; 1 pixel high or wide
			; images cannot benefit from certain filters.  If this isn't done here
			; the check below will fire on 1 pixel high images.

			cmp dword[edi+png_struct.height],1
			jne @f ;if (..==..)
				and dword[filters],not (PNG_FILTER_UP or PNG_FILTER_AVG or PNG_FILTER_PAETH)
			@@:
			cmp dword[edi+png_struct.width],1
			jne @f ;if (..==..)
				and dword[filters],not (PNG_FILTER_SUB or PNG_FILTER_AVG or PNG_FILTER_PAETH)
			@@:
			mov eax,[filters]
			and eax,PNG_FILTER_UP or PNG_FILTER_AVG or PNG_FILTER_PAETH
			jz @f
			cmp dword[edi+png_struct.prev_row],0
			je @f;if (..!=0 && ..==0)
				; This is the error case, however it is benign - the previous row
				; is not available so the filter can't be used.  Just warn here.

				png_app_warning edi, 'png_set_filter: UP/AVG/PAETH cannot be added after start'
				and dword[filters],not (PNG_FILTER_UP or PNG_FILTER_AVG or PNG_FILTER_PAETH)
			@@:

			xor ebx,ebx

			mov eax,[filters]
			and eax,PNG_FILTER_SUB
			jz @f ;if (..)
				inc ebx
			@@:
			mov eax,[filters]
			and eax,PNG_FILTER_UP
			jz @f ;if (..)
				inc ebx
			@@:
			mov eax,[filters]
			and eax,PNG_FILTER_AVG
			jz @f ;if (..)
				inc ebx
			@@:
			mov eax,[filters]
			and eax,PNG_FILTER_PAETH
			jz @f ;if (..)
				inc ebx
			@@:
			; Allocate needed row buffers if they have not already been
			; allocated.

			movzx eax,byte[edi+png_struct.usr_channels]
			movzx ecx,byte[edi+png_struct.usr_bit_depth]
			imul eax,ecx ;.bit_depth * .channels
			mov ecx,[edi+png_struct.width]
			inc ecx
			PNG_ROWBYTES eax, ecx
			mov ecx, eax

			cmp dword[edi+png_struct.try_row],0
			jne @f ;if (..==0)
				stdcall png_malloc, edi, ecx
				mov [edi+png_struct.try_row],eax
			@@:

			cmp ebx,1
			jle .end3 ;if (..>..)
			cmp dword[edi+png_struct.tst_row],0
			jne .end3 ;if (..==0)
				stdcall png_malloc, edi, ecx
				mov [edi+png_struct.tst_row],eax
		.end3:
		mov eax,[filters]
		mov byte[edi+png_struct.do_filter],al
end if
		jmp .end_f
	.end0: ;else
		png_error edi, 'Unknown custom filter method'
.end_f:
popad
	ret
endp

; Provide floating and fixed point APIs
;void (png_structrp png_ptr, int heuristic_method,
;    int num_weights, png_const_doublep filter_weights, png_const_doublep filter_costs)
align 4
proc png_set_filter_heuristics, png_ptr:dword, heuristic_method:dword, num_weights:dword, filter_weights:dword, filter_costs:dword
	ret
endp

;void (png_structrp png_ptr, int heuristic_method,
;    int num_weights, png_const_fixed_point_p filter_weights,
;    png_const_fixed_point_p filter_costs)
align 4
proc png_set_filter_heuristics_fixed, png_ptr:dword, heuristic_method:dword, num_weights:dword, filter_weights:dword, filter_costs:dword
	ret
endp

;void (png_structrp png_ptr, int level)
align 4
proc png_set_compression_level uses edi, png_ptr:dword, level:dword
	png_debug 1, 'in png_set_compression_level'

	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return

	m2m [edi+png_struct.zlib_level], [level]
	@@:
	ret
endp

;void (png_structrp png_ptr, int mem_level)
align 4
proc png_set_compression_mem_level uses edi, png_ptr:dword, mem_level:dword
	png_debug 1, 'in png_set_compression_mem_level'

	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return

	m2m [edi+png_struct.zlib_mem_level], [mem_level]
	@@:
	ret
endp

;void (png_structrp png_ptr, int strategy)
align 4
proc png_set_compression_strategy uses edi, png_ptr:dword, strategy:dword
	png_debug 1, 'in png_set_compression_strategy'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	; The flag setting here prevents the libpng dynamic selection of strategy.

	or dword[edi+png_struct.flags], PNG_FLAG_ZLIB_CUSTOM_STRATEGY
	m2m [edi+png_struct.zlib_strategy], [strategy]
.end_f:
	ret
endp

; If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
; smaller value of window_bits if it can do so safely.

;void (png_structrp png_ptr, int window_bits)
align 4
proc png_set_compression_window_bits uses eax edi, png_ptr:dword, window_bits:dword
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	; Prior to 1.6.0 this would warn but then set the window_bits value. This
	; meant that negative window bits values could be selected that would cause
	; libpng to write a non-standard PNG file with raw deflate or gzip
	; compressed IDAT or ancillary chunks.  Such files can be read and there is
	; no warning on read, so this seems like a very bad idea.

	mov eax,[window_bits]
	cmp eax,15
	jle @f ;if (..>..)
		png_warning edi, 'Only compression windows <= 32k supported by PNG'
		mov eax,15
		jmp .end0
	@@: ;else if (..<..)
	cmp eax,8
	jge @f
		png_warning edi, 'Only compression windows >= 256 supported by PNG'
		mov eax,8
	.end0:

	mov [edi+png_struct.zlib_window_bits],eax
.end_f:
	ret
endp

;void (png_structrp png_ptr, int method)
align 4
proc png_set_compression_method uses eax edi, png_ptr:dword, method:dword
	png_debug 1, 'in png_set_compression_method'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	; This would produce an invalid PNG file if it worked, but it doesn't and
	; deflate will fault it, so it is harmless to just warn here.

	mov eax,[method]
	cmp eax,8
	je @f ;if (..!=..)
		png_warning edi, 'Only compression method 8 is supported by PNG'
	@@:
	mov [edi+png_struct.zlib_method],eax
.end_f:
	ret
endp

; The following were added to libpng-1.5.4
if PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED eq 1
;void (png_structrp png_ptr, int level)
align 4
proc png_set_text_compression_level uses edi, png_ptr:dword, level:dword
	png_debug 1, 'in png_set_text_compression_level'

	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return

	m2m [edi+png_struct.zlib_text_level], [level]
	@@:
	ret
endp

;void (png_structrp png_ptr, int mem_level)
align 4
proc png_set_text_compression_mem_level uses edi, png_ptr:dword, mem_level:dword
	png_debug 1, 'in png_set_text_compression_mem_level'

	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return

	m2m [edi+png_struct.zlib_text_mem_level], [mem_level]
	@@:
	ret
endp

;void (png_structrp png_ptr, int strategy)
align 4
proc png_set_text_compression_strategy uses edi, png_ptr:dword, strategy:dword
	png_debug 1, 'in png_set_text_compression_strategy'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	m2m [edi+png_struct.zlib_text_strategy], [strategy]
.end_f:
	ret
endp

; If PNG_WRITE_OPTIMIZE_CMF_SUPPORTED is defined, libpng will use a
; smaller value of window_bits if it can do so safely.

;void (png_structrp png_ptr, int window_bits)
align 4
proc png_set_text_compression_window_bits uses eax edi, png_ptr:dword, window_bits:dword
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	mov eax,[window_bits]
	cmp eax,15
	jle @f ;if (..>..)
		png_warning edi, 'Only compression windows <= 32k supported by PNG'
		mov eax,15
		jmp .end0
	@@: ;else if (..<..)
	cmp eax,8
	jge @f
		png_warning edi, 'Only compression windows >= 256 supported by PNG'
		mov eax,8
	.end0:

	mov [edi+png_struct.zlib_text_window_bits],eax
.end_f:
	ret
endp

;void (png_structrp png_ptr, int method)
align 4
proc png_set_text_compression_method uses edi, png_ptr:dword, method:dword
	png_debug 1, 'in png_set_text_compression_method'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	cmp dword[method],8
	je @f ;if (..!=..)
		png_warning edi, 'Only compression method 8 is supported by PNG'
	@@:
	m2m [edi+png_struct.zlib_text_method], [method]
.end_f:
	ret
endp
end if ;WRITE_CUSTOMIZE_ZTXT_COMPRESSION
; end of API added to libpng-1.5.4

;void (png_structrp png_ptr, png_write_status_ptr write_row_fn)
align 4
proc png_set_write_status_fn uses edi, png_ptr:dword, write_row_fn:dword
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return
		m2m [edi+png_struct.write_row_fn], [write_row_fn]
	@@:
	ret
endp

;void (png_structrp png_ptr, png_user_transform_ptr write_user_transform_fn)
align 4
proc png_set_write_user_transform_fn uses edi, png_ptr:dword, write_user_transform_fn:dword
	png_debug 1, 'in png_set_write_user_transform_fn'

	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return
		or dword[edi+png_struct.transformations], PNG_USER_TRANSFORM
		m2m [edi+png_struct.write_user_transform_fn], [write_user_transform_fn]
	@@:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int transforms, voidp params)
align 4
proc png_write_png, png_ptr:dword, info_ptr:dword, transforms:dword, params:dword
pushad
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f
	mov esi,[info_ptr]
	cmp esi,0
	je .end_f ;if(..==0 || ..==0) return

	and dword[esi+png_info_def.valid],PNG_INFO_IDAT
	cmp esi,0 ;if(..==0)
	jne @f
		cStr ,'no rows for png_write_image to write'
		stdcall png_app_error edi, eax
		jmp .end_f ;return
	@@:

	; Write the file header information.
	stdcall png_write_info, edi, esi

	; ------ these transformations don't touch the info structure -------

	; Invert monochrome pixels
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_INVERT_MONO
	jz @f ;if(..!=0)
if PNG_WRITE_INVERT_SUPPORTED eq 1
		stdcall png_set_invert_mono,edi
else
		cStr ,'PNG_TRANSFORM_INVERT_MONO not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Shift the pixels up to a legal bit depth and fill in
	; as appropriate to correctly scale the image.

	mov eax,[transforms]
	and eax,PNG_TRANSFORM_SHIFT
	jz @f ;if(..!=0)
if PNG_WRITE_SHIFT_SUPPORTED eq 1
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_sBIT
	jz @f ;if(..!=0)
		mov eax,esi
		add eax,png_info_def.sig_bit
		stdcall png_set_shift, edi, eax
else
		cStr ,'PNG_TRANSFORM_SHIFT not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Pack pixels into bytes
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_PACKING
	jz @f ;if(..!=0)
if PNG_WRITE_PACK_SUPPORTED eq 1
		stdcall png_set_packing, edi
else
		cStr ,'PNG_TRANSFORM_PACKING not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Swap location of alpha bytes from ARGB to RGBA
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_SWAP_ALPHA
	jz @f ;if(..!=0)
if PNG_WRITE_SWAP_ALPHA_SUPPORTED eq 1
		stdcall png_set_swap_alpha, edi
else
		cStr ,'PNG_TRANSFORM_SWAP_ALPHA not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Remove a filler (X) from XRGB/RGBX/AG/GA into to convert it into
	; RGB, note that the code expects the input color type to be G or RGB; no
	; alpha channel.

	mov eax,[transforms]
	and eax,PNG_TRANSFORM_STRIP_FILLER_AFTER or PNG_TRANSFORM_STRIP_FILLER_BEFORE
	jz .end_0 ;if(..!=0)
if PNG_WRITE_FILLER_SUPPORTED eq 1
	and eax,PNG_TRANSFORM_STRIP_FILLER_AFTER
	jz .end_1 ;if(..!=0)
		mov eax,[transforms]
		and eax,PNG_TRANSFORM_STRIP_FILLER_BEFORE
		jz @f ;if(..!=0)
            cStr ,'PNG_TRANSFORM_STRIP_FILLER: BEFORE+AFTER not supported'
			stdcall png_app_error edi, eax
		@@:

		; Continue if ignored - this is the pre-1.6.10 behavior
		stdcall png_set_filler, edi, 0, PNG_FILLER_AFTER
		jmp .end_0
	.end_1: ;else if ((transforms & PNG_TRANSFORM_STRIP_FILLER_BEFORE) != 0)
		stdcall png_set_filler, edi, 0, PNG_FILLER_BEFORE
else
		cStr ,'PNG_TRANSFORM_STRIP_FILLER not supported'
		stdcall png_app_error edi, eax
end if
	.end_0:

	; Flip BGR pixels to RGB
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_BGR
	jz @f ;if(..!=0)
if PNG_WRITE_BGR_SUPPORTED eq 1
		stdcall png_set_bgr, edi
else
		cStr ,'PNG_TRANSFORM_BGR not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Swap bytes of 16-bit files to most significant byte first
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_SWAP_ENDIAN
	jz @f ;if(..!=0)
if PNG_WRITE_SWAP_SUPPORTED eq 1
		stdcall png_set_swap, edi
else
		cStr ,'PNG_TRANSFORM_SWAP_ENDIAN not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Swap bits of 1-bit, 2-bit, 4-bit packed pixel formats
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_PACKSWAP
	jz @f ;if(..!=0)
if PNG_WRITE_PACKSWAP_SUPPORTED eq 1
		stdcall png_set_packswap, edi
else
		cStr ,'PNG_TRANSFORM_PACKSWAP not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; Invert the alpha channel from opacity to transparency
	mov eax,[transforms]
	and eax,PNG_TRANSFORM_INVERT_ALPHA
	jz @f ;if(..!=0)
if PNG_WRITE_INVERT_ALPHA_SUPPORTED eq 1
		stdcall png_set_invert_alpha, edi
else
		cStr ,'PNG_TRANSFORM_INVERT_ALPHA not supported'
		stdcall png_app_error edi, eax
end if
	@@:

	; ----------------------- end of transformations -------------------

	; Write the bits
	stdcall png_write_image, edi, dword[esi+png_info_def.row_pointers]

	; It is REQUIRED to call this to finish writing the rest of the file
	stdcall png_write_end, edi, esi

.end_f:
popad
	ret
endp

if PNG_SIMPLIFIED_WRITE_SUPPORTED eq 1
; Initialize the write structure - general purpose utility.
;int (png_imagep image)
align 4
proc png_image_write_init uses ebx ecx edx edi esi, image:dword
	mov ebx,[image]
	stdcall png_create_write_struct, PNG_LIBPNG_VER_STRING, ebx, png_safe_error, png_safe_warning
	;eax = png_ptr

	test eax,eax
	jz .end0 ;if (..!=0)
		mov edi,eax
		or dword[eax+png_struct.transformations],PNG_BGR ;transformation rgb for KoliriOS
		stdcall png_create_info_struct, edi
		;eax = info_ptr

		test eax,eax
		jz .end1 ;if (..!=0)
			mov esi,eax
			stdcall png_malloc_warn, edi, sizeof.png_control
			;control = eax

			test eax,eax
			jz .end2 ;if (..!=0)
				push eax
				mov edx,edi ; edx = png_ptr
				mov ecx,sizeof.png_control
				mov edi,eax
				xor eax,eax
				rep stosb ;memset(control, 0, (sizeof.control))
				pop eax

				mov [eax+png_control.png_ptr], edx
				mov [eax+png_control.info_ptr], esi
				mov [eax+png_control.for_write], 1

				mov [ebx+png_image.opaque], eax
				xor eax,eax
				inc eax
				jmp .end_f
			.end2:

			; Error clean up
			push esi
			mov esi,esp
			stdcall png_destroy_info_struct, edi, esi
			add esp,4
		.end1:

		push edi
		mov edi,esp
		stdcall png_destroy_write_struct, edi, 0
		add esp,4
	.end0:

	std_png_image_error ebx, 'png_image_write_: out of memory'
.end_f:
	ret
endp

; Arguments to png_image_write_main:
struct png_image_write_control
	; Arguments:
	image  dd ? ;png_imagep
	buffer dd ? ;png_const_voidp
	row_stride dd ? ;int_32
	colormap   dd ? ;png_const_voidp
	convert_to_8bit dd ? ;int
	; Local variables:
	first_row dd ? ;png_const_voidp
	row_bytes dd ? ;ptrdiff_t
	local_row dd ? ;voidp
	; Byte count for memory writing
	memory    dd ? ;bytep
	memory_bytes dd ? ;png_alloc_size_t ;not used for STDIO
	output_bytes dd ? ;png_alloc_size_t ;running total
ends

; Write uint_16 input to a 16-bit PNG; the png_ptr has already been set to
; do any necessary byte swapping.  The component order is defined by the
; png_image format value.

;int (voidp argument)
align 4
proc png_write_image_16bit uses ebx ecx edx, argument:dword
locals
	display dd ? ;png_image_write_control* ;= argument
	image   dd ? ;png_imagep ;= display->image
	png_ptr dd ? ;png_structrp ;= image->opaque->png_ptr
	input_row  dd ? ;const_uint_16p ;= display->first_row
	output_row dd ? ;uint_16p ;= display->local_row
	row_end  dd ? ;uint_16p
	channels dd ? ;const int ;= (image->format & PNG_FORMAT_FLAG_COLOR) != 0 ? 3 : 1;
	aindex   dd 0 ;int  ;= 0
	y dd ? ;uint_32 ;= image->height
endl
	mov ebx,[argument]
	mov [display],ebx
	mov edx,[ebx+png_image_write_control.image]
	mov [image],edx
	mov ecx,[edx+png_image.opaque]
	mov ecx,[ecx+png_control.png_ptr]
	mov [png_ptr],ecx
	mov ecx,[ebx+png_image_write_control.first_row]
	mov [input_row],ecx
	mov ecx,[ebx+png_image_write_control.local_row]
	mov [output_row],ecx

	mov ecx,1
	mov eax,[edx+png_image.format]
	and eax,PNG_FORMAT_FLAG_COLOR
	jz @f
		mov ecx,3
	@@:
	mov [channels],ecx
	mov eax,[edx+png_image.height]
	mov [y],eax

	mov eax,[edx+png_image.format]
	and eax,PNG_FORMAT_FLAG_ALPHA
	jz .end0 ;if (..!=0)
if PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED eq 1
	mov eax,[edx+png_image.format]
	and eax,PNG_FORMAT_FLAG_AFIRST
	jz @f ;if (..!=0)
		mov dword[aindex],-1
		inc dword[input_row] ;To point to the first component
		inc dword[output_row]
		jmp .end1
	@@: ;else
end if
		mov eax,[channels]
		mov [aindex],eax
		jmp .end1
	.end0: ;else
		png_error [png_ptr], 'png_write_image: internal call error'
	.end1:

	; Work out the output row end and count over this, note that the increment
	; above to 'row' means that row_end can actually be beyond the end of the
	; row; this is correct.

	mov eax,[channels]
	inc eax
	imul eax,[edx+png_image.width]
	add eax,[output_row]
	mov [row_end],eax

;   while (y-- > 0)
;   {
;      const_uint_16p in_ptr = input_row;
;      uint_16p out_ptr = output_row;

;      while (out_ptr < row_end)
;      {
;         const uint_16 alpha = in_ptr[aindex];
;         uint_32 reciprocal = 0;
;         int c;

;         out_ptr[aindex] = alpha;

	; Calculate a reciprocal.  The correct calculation is simply
	; component/alpha*65535 << 15. (I.e. 15 bits of precision); this
	; allows correct rounding by adding .5 before the shift.  'reciprocal'
	; is only initialized when required.

;         if (alpha > 0 && alpha < 65535)
;            reciprocal = ((0xffff<<15)+(alpha>>1))/alpha;

;         c = channels;
;         do /* always at least one channel */
;         {
;            uint_16 component = *in_ptr++;

	; The following gives 65535 for an alpha of 0, which is fine,
	; otherwise if 0/0 is represented as some other value there is more
	; likely to be a discontinuity which will probably damage
	; compression when moving from a fully transparent area to a
	; nearly transparent one.  (The assumption here is that opaque
	; areas tend not to be 0 intensity.)

;            if (component >= alpha)
;               component = 65535;

	; component<alpha, so component/alpha is less than one and
	; component*reciprocal is less than 2^31.

;            else if (component > 0 && alpha < 65535)
;            {
;               uint_32 calc = component * reciprocal;
;               calc += 16384; /* round to nearest */
;               component = (uint_16)(calc >> 15);
;            }

;            *out_ptr++ = component;
;         }
;         while (--c > 0);

	; Skip to next component (skip the intervening alpha channel)
;         ++in_ptr;
;         ++out_ptr;
;      }

;      png_write_row(png_ptr, display->local_row);
;      input_row += display->row_bytes/(sizeof (uint_16));
;   }

	xor eax,eax
	inc eax ;return 1
	ret
endp

; Given 16-bit input (1 to 4 channels) write 8-bit output.  If an alpha channel
; is present it must be removed from the components, the components are then
; written in sRGB encoding.  No components are added or removed.

; Calculate an alpha reciprocal to reverse pre-multiplication.  As above the
; calculation can be done to 15 bits of accuracy; however, the output needs to
; be scaled in the range 0..255*65535, so include that scaling here.

;#   define UNP_RECIPROCAL(alpha) ((((0xffff*0xff)<<7)+(alpha>>1))/alpha)

;byte (uint_32 component, uint_32 alpha, uint_32 reciprocal/*from the above macro*/)
align 4
proc png_unpremultiply, component:dword, alpha:dword, reciprocal:dword
	; The following gives 1.0 for an alpha of 0, which is fine, otherwise if 0/0
	; is represented as some other value there is more likely to be a
	; discontinuity which will probably damage compression when moving from a
	; fully transparent area to a nearly transparent one.  (The assumption here
	; is that opaque areas tend not to be 0 intensity.)

	; There is a rounding problem here; if alpha is less than 128 it will end up
	; as 0 when scaled to 8 bits.  To avoid introducing spurious colors into the
	; output change for this too.

	mov eax,[alpha]
	cmp [component],eax
	jge @f
	cmp eax,128
	jge .end0
	@@: ;if (..>=.. || ..<..)
		mov eax,255
		jmp .end_f
		; component<alpha, so component/alpha is less than one and
		; component*reciprocal is less than 2^31.

	.end0: ;else if (component > 0)
	cmp dword[component],0
	jle .end1
		; The test is that alpha/257 (rounded) is less than 255, the first value
		; that becomes 255 is 65407.
		; NOTE: this must agree with the PNG_DIV257 macro (which must, therefore,
		; be exact!)  [Could also test reciprocal != 0]

;      if (alpha < 65407)
;      {
;         component *= reciprocal;
;         component += 64; /* round to nearest */
;         component >>= 7;
;      }

;      else
;         component *= 255;

		; Convert the component to sRGB.
		PNG_sRGB_FROM_LINEAR [component]
		and eax,0xff
		jmp .end_f
	.end1: ;else
		xor eax,eax
.end_f:
	ret
endp

;int (voidp argument)
align 4
proc png_write_image_8bit uses ebx ecx edx edi esi, argument:dword
locals
	display dd ? ;png_image_write_control* ;= argument
	image   dd ? ;png_imagep ;= display->image
	png_ptr dd ? ;png_structrp ;= image->opaque->png_ptr
	input_row  dd ? ;const_uint_16p ;= display->first_row
	output_row dd ? ;uint_16p ;= display->local_row
	row_end  dd ? ;uint_16p
	channels dd ? ;const int ;= (image->format & PNG_FORMAT_FLAG_COLOR) != 0 ? 3 : 1;
	aindex   dd 0 ;int  ;= 0
	y dd ? ;uint_32 ;= image->height
	component dd ? ;uint_32
endl
	mov ebx,[argument]
	mov [display],ebx
	mov edx,[ebx+png_image_write_control.image]
	mov [image],edx
	mov ecx,[edx+png_image.opaque]
	mov ecx,[ecx+png_control.png_ptr]
	mov [png_ptr],ecx
	mov ecx,[ebx+png_image_write_control.first_row]
	mov [input_row],ecx
	mov ecx,[ebx+png_image_write_control.local_row]
	mov [output_row],ecx

	mov ecx,1
	mov eax,[edx+png_image.format]
	and eax,PNG_FORMAT_FLAG_COLOR
	jz @f
		mov ecx,3
	@@:
	mov [channels],ecx
	mov eax,[edx+png_image.height]
	mov [y],eax

	mov eax,[edx+png_image.format]
	and eax,PNG_FORMAT_FLAG_ALPHA
	jz .end0 ;if (..!=0)

if PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED eq 1
	mov eax,[edx+png_image.format]
	and eax,PNG_FORMAT_FLAG_AFIRST
	jz .end2 ;if (..!=0)
		mov [aindex],-1
		inc [input_row] ; To point to the first component
		inc [output_row]
		jmp @f
	.end2: ;else
end if
		mov eax,[channels]
		mov [aindex],eax
	.@@:

	; Use row_end in place of a loop counter:
	mov ecx,[channels]
	inc ecx
	imul ecx,[edx+png_image.width]
	add ecx,[output_row]
	;ecx = row_end

;      while (y-- > 0)
;      {
;         const_uint_16p in_ptr = input_row;
;         bytep out_ptr = output_row;

;         while (out_ptr < row_end)
;         {
;            uint_16 alpha = in_ptr[aindex];
;            byte alphabyte = (byte)PNG_DIV257(alpha);
;            uint_32 reciprocal = 0;
;            int c;

	; Scale and write the alpha channel.
;            out_ptr[aindex] = alphabyte;

;            if (alphabyte > 0 && alphabyte < 255)
;               reciprocal = UNP_RECIPROCAL(alpha);

;            c = channels;
;            do /* always at least one channel */
;               *out_ptr++ = png_unpremultiply(*in_ptr++, alpha, reciprocal);
;            while (--c > 0);

	; Skip to next component (skip the intervening alpha channel)
;            ++in_ptr;
;            ++out_ptr;
;         } /* while out_ptr < row_end */

;         png_write_row(png_ptr, display->local_row);
;         input_row += display->row_bytes/(sizeof (uint_16));
;      } /* while y */
		jmp .end1
	.end0: ;else
		; No alpha channel, so the row_end really is the end of the row and it
		; is sufficient to loop over the components one by one.

		mov ecx,[edx+png_image.width]
		imul ecx,[channels]
		add ecx,[output_row]
		;ecx = row_end

		.cycle2: ;while (y-- > 0)
			cmp dword[y],0
			jle .cycle2end
			mov esi,[input_row]
			mov edi,[output_row]
			;esi = in_ptr
			;edi = out_ptr

			.cycle3: ;while (..<..)
				cmp edi,ecx
				jge .cycle3end
				xor eax,eax
				lodsw

				imul eax,255
				mov [component],eax
				PNG_sRGB_FROM_LINEAR [component]
				stosb
				jmp .cycle3
align 4
			.cycle3end:

			stdcall png_write_row, [png_ptr], [output_row]
			mov eax,[ebx+png_image_write_control.row_bytes]
			shr eax,1 ;sizeof.uint_16
			add [input_row],eax
			dec dword[y]
			jmp .cycle2
align 4
		.cycle2end:
	.end1:

	xor eax,eax
	inc eax
	ret
endp

;void (png_image_write_control *display)
align 4
proc png_image_set_PLTE, display:dword
locals
	image dd ? ;png_imagep ;= display->image
	cmap dd ? ;void * ;= display->colormap
	entries dd ? ;int

	; NOTE: the caller must check for cmap != NULL and entries != 0
	format dd ? ;uint_32 ;= image->format
	channels dd ? ;int
	afirst dd 0
	bgr dd 0
	num_trans dd 0
	palette rb 256*sizeof.png_color
	tRNS rb 256 ;byte[]
endl
pushad
	mov edx,[display]
	mov ebx,[edx+png_image_write_control.image]
	mov [image],ebx
	mov eax,[edx+png_image_write_control.colormap]
	mov [cmap],eax
	mov eax,[ebx+png_image.colormap_entries]
	cmp eax,256
	jle @f
		mov eax,256
	@@:
	mov [entries],eax
	mov ecx,[ebx+png_image.format]
	mov [format],ecx
	PNG_IMAGE_SAMPLE_CHANNELS ecx
	mov [channels],eax

if (PNG_FORMAT_BGR_SUPPORTED eq 1) & (PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED eq 1)
	mov eax,ecx
	and eax,PNG_FORMAT_FLAG_AFIRST
	jz @f
	mov eax,ecx
	and eax,PNG_FORMAT_FLAG_ALPHA
	jz @f
		mov dword[afirst],-1
	@@:
end if

if PNG_FORMAT_BGR_SUPPORTED eq 1
	mov eax,ecx
	and eax,PNG_FORMAT_FLAG_BGR
	jz @f
		mov dword[bgr],2
	@@:
end if

;   int i;

	xor eax,eax
	mov ecx,(256*sizeof.png_color)/4
	mov edi,ebp
	sub edi,256+256*sizeof.png_color
	rep stosd ;memset(palette, 0, ...
	not eax
	mov ecx,256/4
	;;mov edi,ebp ;if 'tRNS' after 'palette' this code can be comment
	;;sub edi,256
	rep stosd ;memset(tRNS, 255, ...


;   for (i=num_trans=0; i<entries; ++i)
;   {
		; This gets automatically converted to sRGB with reversal of the
		; pre-multiplication if the color-map has an alpha channel.

;      if ((format & PNG_FORMAT_FLAG_LINEAR) != 0)
;      {
;         png_const_uint_16p entry = cmap;

;         entry += i * channels;

;         if ((channels & 1) != 0) /* no alpha */
;         {
;            if (channels >= 3) /* RGB */
;            {
;               palette[i].blue = (byte)PNG_sRGB_FROM_LINEAR(255 *
;                   entry[(2 ^ bgr)]);
;               palette[i].green = (byte)PNG_sRGB_FROM_LINEAR(255 *
;                   entry[1]);
;               palette[i].red = (byte)PNG_sRGB_FROM_LINEAR(255 *
;                   entry[bgr]);
;            }

;            else /* Gray */
;               palette[i].blue = palette[i].red = palette[i].green =
;                  (byte)PNG_sRGB_FROM_LINEAR(255 * *entry);
;         }

;         else /* alpha */
;         {
;            uint_16 alpha = entry[afirst ? 0 : channels-1];
;            byte alphabyte = (byte)PNG_DIV257(alpha);
;            uint_32 reciprocal = 0;

		; Calculate a reciprocal, as in the png_write_image_8bit code above
		; this is designed to produce a value scaled to 255*65535 when
		; divided by 128 (i.e. asr 7).

;            if (alphabyte > 0 && alphabyte < 255)
;               reciprocal = (((0xffff*0xff)<<7)+(alpha>>1))/alpha;

;            tRNS[i] = alphabyte;
;            if (alphabyte < 255)
;               num_trans = i+1;

;            if (channels >= 3) /* RGB */
;            {
;               palette[i].blue = png_unpremultiply(entry[afirst + (2 ^ bgr)],
;                   alpha, reciprocal);
;               palette[i].green = png_unpremultiply(entry[afirst + 1], alpha,
;                   reciprocal);
;               palette[i].red = png_unpremultiply(entry[afirst + bgr], alpha,
;                   reciprocal);
;            }

;            else /* gray */
;               palette[i].blue = palette[i].red = palette[i].green =
;                   png_unpremultiply(entry[afirst], alpha, reciprocal);
;         }
;      }

;      else /* Color-map has sRGB values */
;      {
;         bytep entry = cmap;

;         entry += i * channels;

;         switch (channels)
;         {
;            case 4:
;               tRNS[i] = entry[afirst ? 0 : 3];
;               if (tRNS[i] < 255)
;                  num_trans = i+1;
;               /* FALL THROUGH */
;            case 3:
;               palette[i].blue = entry[afirst + (2 ^ bgr)];
;               palette[i].green = entry[afirst + 1];
;               palette[i].red = entry[afirst + bgr];
;               break;

;            case 2:
;               tRNS[i] = entry[1 ^ afirst];
;               if (tRNS[i] < 255)
;                  num_trans = i+1;
;               /* FALL THROUGH */
;            case 1:
;               palette[i].blue = palette[i].red = palette[i].green =
;                  entry[afirst];
;               break;

;            default:
;               break;
;         }
;      }
;   }

	mov ecx,[ebx+png_image.opaque]
	mov eax,ebp
	sub eax,256+256*sizeof.png_color
	stdcall png_set_PLTE, [ecx+png_control.png_ptr], [ecx+png_control.info_ptr], eax, [entries]

	cmp dword[num_trans],0
	jle @f ;if (..>0)
		mov eax,ebp
		sub eax,256
		stdcall png_set_tRNS, [ecx+png_control.png_ptr], [ecx+png_control.info_ptr], eax, [num_trans], 0
	@@:

	mov eax,[entries]
	mov [ebx+png_image.colormap_entries],eax
popad
	ret
endp

;int (voidp argument)
align 4
proc png_image_write_main uses ebx ecx edx esi edi, argument:dword
locals
	display dd ? ;= png_image_write_control * = argument
	image dd ? ;= display->image
	png_ptr dd ? ;= image->opaque->png_ptr
	info_ptr dd ? ;= image->opaque->info_ptr
	format dd ? ;= image->format

	colormap dd ?
	linear dd ?
	alpha dd ?
	write_16bit dd ? ;= linear && !colormap && (display->convert_to_8bit == 0)
endl
	mov edx,[argument]
	mov [display],edx
	mov ebx,[edx+png_image_write_control.image]
	mov [image],ebx
	mov ecx,[ebx+png_image.format]
	mov [format],ecx
	mov eax,[ebx+png_image.opaque]
	mov edi,[eax+png_control.png_ptr]
	mov [png_ptr],edi
	mov esi,[eax+png_control.info_ptr]
	mov [info_ptr],esi

	; The following four ints are actually booleans
	and ecx,PNG_FORMAT_FLAG_COLORMAP
	mov [colormap],ecx
	not ecx
	mov eax,[format]
	and eax,PNG_FORMAT_FLAG_LINEAR
	mov [linear],eax
	mov eax,[format]
	and eax,ecx
	and eax,PNG_FORMAT_FLAG_ALPHA
	and eax,ecx
	mov [alpha],eax
	xor eax,eax ;false
	cmp dword[edx+png_image_write_control.convert_to_8bit],0
	jne @f
		not eax ;true
	@@:
	and eax,[linear]
	and eax,ecx
	mov [write_16bit],eax

if PNG_BENIGN_ERRORS_SUPPORTED eq 1
	; Make sure we error out on any bad situation
	stdcall png_set_benign_errors, edi, 0 ;error
end if

	; Default the 'row_stride' parameter if required, also check the row stride
	; and total image size to ensure that they are within the system limits.

	PNG_IMAGE_PIXEL_CHANNELS [ebx+png_image.format]
	;eax = channels 

	push edx
	mov ecx,eax
	mov eax,0x7FFFFFFF
	xor edx,edx
	div ecx
	pop edx
	cmp [ebx+png_image.width],eax
	jg .end0 ;if (..<=..) ;no overflow
		imul ecx,[ebx+png_image.width]

		cmp dword[edx+png_image_write_control.row_stride],0
		jne @f ;if (..==0)
			mov [edx+png_image_write_control.row_stride],ecx
		@@:
		mov eax,[edx+png_image_write_control.row_stride]
		cmp eax,0
		jge .end2 ;if (..<0)
			neg eax
			inc eax
		.end2:

		cmp eax,ecx
		jl .end3 ;if (..>=..)
			; Now check for overflow of the image buffer calculation; this
			; limits the whole image size to 32 bits for API compatibility with
			; the current, 32-bit, PNG_IMAGE_BUFFER_SIZE macro.

			push edx
			mov eax,0xFFFFFFFF
			xor edx,edx
			div ecx
			pop edx
			cmp [ebx+png_image.height],eax
			jle @f ;if (..>..)
				mov eax,[ebx+png_image.opaque]
				mov eax,[eax+png_control.png_ptr]
				png_error eax, 'memory image too large'
			@@:
			jmp .end1
		.end3: ;else
			mov eax,[ebx+png_image.opaque]
			mov eax,[eax+png_control.png_ptr]
			png_error eax, 'supplied row stride too small'
		jmp .end1
	.end0: ;else
		mov eax,[ebx+png_image.opaque]
		mov eax,[eax+png_control.png_ptr]
		png_error eax, 'image row stride too large'
	.end1:

	; Set the required transforms then write the rows in the correct order.
	mov eax,[format]
	and eax,PNG_FORMAT_FLAG_COLORMAP
	jz .end4 ;if (..!=0)
		cmp dword[edx+png_image_write_control.colormap],0
		je .end6
		mov eax,[ebx+png_image.colormap_entries]
		cmp eax,0
		jle .end6 ;if (..!=0 && ..>0)
			;eax = entries
			xor ecx,ecx
			inc ecx ;=1
			cmp eax,2
			jle @f
				shl ecx,1 ;=2
			cmp eax,4
			jle @f
				shl ecx,1 ;=4
			cmp eax,16
			jle @f
				shl ecx,1 ;=8
			@@:
			stdcall png_set_IHDR, edi, esi, [ebx+png_image.width], [ebx+png_image.height],\
				ecx, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,\
				PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE

			stdcall png_image_set_PLTE, edx
			jmp .end5
		.end6: ;else
			mov eax,[ebx+png_image.opaque]
			mov eax,[eax+png_control.png_ptr]
			png_error eax, 'no color-map for color-mapped image'
		jmp .end5
	.end4: ;else
		xor ecx,ecx
		mov eax,[format]
		and eax,PNG_FORMAT_FLAG_COLOR
		jz @f
			or ecx,PNG_COLOR_MASK_COLOR
		@@:
		mov eax,[format]
		and eax,PNG_FORMAT_FLAG_ALPHA
		jz @f
			or ecx,PNG_COLOR_MASK_ALPHA
		@@:
		mov eax,8
		cmp dword[write_16bit],0
		je @f
			mov eax,16
		@@:
		stdcall png_set_IHDR, edi, esi, [ebx+png_image.width], [ebx+png_image.height],\
			eax, ecx, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE
	.end5:

	; Counter-intuitively the data transformations must be called *after*
	; png_write_info, not before as in the read code, but the 'set' functions
	; must still be called before.  Just set the color space information, never
	; write an interlaced image.

	cmp dword[write_16bit],0
	je @f ;if (..!=0)
		; The gamma here is 1.0 (linear) and the cHRM chunk matches sRGB.
		stdcall png_set_gAMA_fixed, edi, esi, PNG_GAMMA_LINEAR

		mov eax,[ebx+png_image.flags]
		and eax,PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB
		jnz @f ;if (..==0)
			stdcall png_set_cHRM_fixed, edi, esi,\
			31270, 32900,\ ;white
			64000, 33000,\ ;red
			30000, 60000,\ ;green
			15000,  6000   ;blue
		jmp .end7
	@@:
	mov eax,[ebx+png_image.flags]
	and eax,PNG_IMAGE_FLAG_COLORSPACE_NOT_sRGB
	jnz @f ;else if (..==0)
		stdcall png_set_sRGB, edi, esi, PNG_sRGB_INTENT_PERCEPTUAL
		jmp .end7
	@@: ;else
		; Else writing an 8-bit file and the *colors* aren't sRGB, but the 8-bit
		; space must still be gamma encoded.
		stdcall png_set_gAMA_fixed, edi, esi, PNG_GAMMA_sRGB_INVERSE
	.end7:

	; Write the file header.
	stdcall png_write_info, edi, esi

	; Now set up the data transformations (*after* the header is written),
	; remove the handled transformations from the 'format' flags for checking.

	; First check for a little endian system if writing 16-bit files.

	cmp dword[write_16bit],0
	je @f ;if (..!=0)
;      uint_16 le = 0x0001;

;      if ((*(bytep) & le) != 0)
		stdcall png_set_swap, edi
	@@:

if PNG_SIMPLIFIED_WRITE_BGR_SUPPORTED eq 1
	mov eax,[format]
	and eax,PNG_FORMAT_FLAG_BGR
	jz .end12 ;if (..!=0)
		cmp dword[colormap],0
		jne @f
		mov eax,[format]
		and eax,PNG_FORMAT_FLAG_COLOR
		jz @f ;if (..==0 && ..!=0)
			stdcall png_set_bgr, edi
		@@:
		and dword[format], not PNG_FORMAT_FLAG_BGR
	.end12:
end if

if PNG_SIMPLIFIED_WRITE_AFIRST_SUPPORTED eq 1
	mov eax,[format]
	and eax,PNG_FORMAT_FLAG_AFIRST
	jz .end13 ;if (..!=0)
		cmp dword[colormap],0
		jne @f
		mov eax,[format]
		and eax,PNG_FORMAT_FLAG_ALPHA
		jz @f ;if (..==0 && ..!=0)
			stdcall png_set_swap_alpha, edi
		@@:
		and dword[format], not PNG_FORMAT_FLAG_AFIRST
	.end13:
end if

	; If there are 16 or fewer color-map entries we wrote a lower bit depth
	; above, but the application data is still byte packed.

	cmp dword[colormap],0
	je @f
	cmp dword[ebx+png_image.colormap_entries],16
	jg @f ;if (..!=0 && ..<=16)
		stdcall png_set_packing, edi
	@@:

	; That should have handled all (both) the transforms.
	mov eax,[format]
	and eax, not (PNG_FORMAT_FLAG_COLOR or PNG_FORMAT_FLAG_LINEAR or PNG_FORMAT_FLAG_ALPHA or PNG_FORMAT_FLAG_COLORMAP)
	jz @f ;if (..!=0)
		png_error edi, 'png_write_image: unsupported transformation'
	@@:

	push esi
	;ecx - row ;bytep
	;esi - row_bytes
	mov ecx,[edx+png_image_write_control.buffer]
	mov esi,[edx+png_image_write_control.row_stride]

	cmp dword[linear],0
	je @f ;if (..!=0)
		shl esi,1 ;*= sizeof.uint_16
	@@:
	cmp esi,0
	jge @f ;if (..<0)
		mov eax,[ebx+png_image.height]
		dec eax
		imul eax,esi
		sub ecx,eax
	@@:
	mov [edx+png_image_write_control.first_row],ecx
	mov [edx+png_image_write_control.row_bytes],esi
	pop esi

	; Apply 'fast' options if the flag is set.
	mov eax,[ebx+png_image.flags]
	and eax,PNG_IMAGE_FLAG_FAST
	jz @f ;if (..!=0)
		stdcall png_set_filter, edi, PNG_FILTER_TYPE_BASE, PNG_NO_FILTERS
		; NOTE: determined by experiment using pngstest, this reflects some
		; balance between the time to write the image once and the time to read
		; it about 50 times.  The speed-up in pngstest was about 10-20% of the
		; total (user) time on a heavily loaded system.

if PNG_WRITE_CUSTOMIZE_COMPRESSION_SUPPORTED eq 1
		stdcall png_set_compression_level, edi, 3
end if
	@@:

	; Check for the cases that currently require a pre-transform on the row
	; before it is written.  This only applies when the input is 16-bit and
	; either there is an alpha channel or it is converted to 8-bit.

	cmp dword[linear],0
	je @f
	cmp dword[alpha],0
	je @f
		jmp .end10
	@@:
	cmp dword[colormap],0
	jne .end9
	cmp dword[edx+png_image_write_control.convert_to_8bit],0
	je .end9
	.end10: ;if ((..!=0 && ..!=0 ) || (..==0 && ..!=0))
		stdcall png_get_rowbytes, edi, esi
		stdcall png_malloc, edi, eax
		mov ecx,eax ;ecx = row
		
		mov [edx+png_image_write_control.local_row],ecx
		cmp dword[write_16bit],0
		je @f ;if (..!=0)
			stdcall png_safe_execute, ebx, png_write_image_16bit, edx
			jmp .end11
		@@: ;else
			stdcall png_safe_execute, ebx, png_write_image_8bit, edx
		.end11:
		mov dword[edx+png_image_write_control.local_row],0

		stdcall png_free, edi, ecx

		; Skip the 'write_end' on error:
		test eax,eax
		jz .end_f ;if (..==0) return 0
		jmp .end8

	; Otherwise this is the case where the input is in a format currently
	; supported by the rest of the libpng write code; call it directly.

	.end9: ;else
if 1 ;;; IDAT compress all (only 24 bit)
		cmp dword[ebx+png_image.height],1
		jl .end8
		mov ecx,[edx+png_image_write_control.row_bytes]
		inc ecx
		imul ecx,[ebx+png_image.height]
		stdcall create_compress_IDAT, edi, [edx+png_image_write_control.first_row], ecx, [ebx+png_image.width], [ebx+png_image.height]
else ;;; IDAT compress by lines
		mov ecx,[ebx+png_image.height]
		cmp ecx,1
		jl .end8
		mov eax,[edx+png_image_write_control.first_row]
		mov ebx,[edx+png_image_write_control.row_bytes]
		@@:
			stdcall png_write_row, edi, eax
			add eax, ebx
			loop @b
end if
	.end8:

	stdcall png_write_end, edi, esi
	xor eax,eax
	inc eax
.end_f:
	ret
endp

align 16
proc create_compress_IDAT, png_ptr:dword, buf:dword, len:dword, width:dword, height:dword
locals
	m1 dd ? ;memory for compress IDAT
	buf_f dd ? ;memory for IDAT
	mins dd ? ;minimum sum
endl
pushad
	mov edi,[png_ptr]
png_debug 1, 'IDAT compress all'

	;create buffer with filters
	stdcall png_zalloc, edi, 1, [len]
	test eax,eax
	jz .end_f
	mov [buf_f],eax

	mov eax,ZLIB_IO_MAX
	cmp eax,[len]
	jge @f
		mov eax,[len]
	@@:
	stdcall png_zalloc, edi, 1, eax
	test eax,eax
	jz .end0
	mov [m1],eax

	;init buffer with filters
	mov ebx,[width]
	mov edx,[height]
	mov edi,[buf_f]
	mov esi,[buf]
	.cycle0:
	cmp edx,1
	jl .cycle0end
		mov ecx,ebx
		xor al,al
		stosb ;insert filter (0 - none)
align 4
		.cycle1:
			lodsb   ;1
			inc edi ;
			movsb   ;2
			stosb   ;3
			lodsb   ;
			mov [edi-3],al
			loop .cycle1
		dec edx
		jmp .cycle0
	.cycle0end:

	;make filters
	mov edx,[height]
	mov esi,[width]
	imul esi,3 ;esi - rowbytes

	inc esi
	mov edi,[png_ptr]
	cmp dword[edi+png_struct.try_row],0
	jne @f ;if (..==0)
		stdcall png_malloc, edi, esi
		mov [edi+png_struct.try_row],eax
	@@:
	cmp dword[edi+png_struct.tst_row],0
	jne @f ;if (..==0)
		stdcall png_malloc, edi, esi
		mov [edi+png_struct.tst_row],eax
	@@:
	dec esi

	mov edi,[buf_f]
	add edi,[len]
	.cycle3:
		dec edx
		cmp edx,1
		jl .cycle3end
		sub edi,esi
		dec edi ;move in perv row

		;init pointers for function png_setup_up_row
		mov ebx,[png_ptr]
		mov [ebx+png_struct.row_buf],edi
		mov [ebx+png_struct.prev_row],edi
		sub [ebx+png_struct.prev_row],esi
		dec dword[ebx+png_struct.prev_row]

		;calculate start minimum sum
		push esi
		xor eax,eax
		xor ebx,ebx
		mov ecx,esi
		mov esi,edi
		inc esi
		.cycle2:
			lodsb
			png_setup_abs ebx
			loop .cycle2
		pop esi
		mov [mins],ebx

		push edx
		mov edx,[png_ptr]
		mov eax,[edx+png_struct.tst_row]
		mov byte[eax],0 ;not filter

		; Up filter
		stdcall png_setup_up_row, edx, esi, [mins]
		cmp eax,[mins]
		jge @f ;if (..<..)
			mov [mins],eax
			stdcall copy_row_mins, [edx+png_struct.tst_row], [edx+png_struct.try_row]
		@@:

		; Find out how many bytes offset each pixel is
		movzx ebx,byte[edx+png_struct.pixel_depth]
		add ebx,7
		shr ebx,3

		; Sub filter
		stdcall png_setup_sub_row, edx, ebx, esi, [mins]
		cmp eax,[mins]
		jge @f ;if (..<..)
			mov [mins],eax
			stdcall copy_row_mins, [edx+png_struct.tst_row], [edx+png_struct.try_row]
		@@:

		; Avg filter
		stdcall png_setup_avg_row, edx, ebx, esi, [mins]
		cmp eax,[mins]
		jge @f ;if (..<..)
			mov [mins],eax
			stdcall copy_row_mins, [edx+png_struct.tst_row], [edx+png_struct.try_row]
		@@:

		; Paeth filter
		stdcall png_setup_paeth_row, edx, ebx, esi, [mins]
		cmp eax,[mins]
		jge @f ;if (..<..)
			mov [mins],eax
			stdcall copy_row_mins, [edx+png_struct.tst_row], [edx+png_struct.try_row]
		@@:

		; Copy best row
		mov eax,[edx+png_struct.tst_row]
		cmp byte[eax],0
		je @f
			stdcall copy_row_mins, edi, [edx+png_struct.tst_row]
		@@:
		pop edx
		jmp .cycle3
	.cycle3end:
	
	mov edi,[png_ptr]
	mov esi,edi
	add esi,png_struct.zstream
	stdcall [deflateInit2], esi,\
		-1, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY

	mov eax,[buf_f]
	mov [edi+png_struct.zstream.next_in],eax
	mov eax,[len]
	mov [edi+png_struct.zstream.avail_in],eax
	mov eax,[m1]
	mov [edi+png_struct.zstream.next_out],eax
	xor ecx,ecx
align 4
	.cycle4:
		mov dword[edi+png_struct.zstream.avail_out],16*1024

		stdcall [deflate], esi, Z_FINISH ;Z_NO_FLUSH
		cmp eax,Z_STREAM_ERROR
		je .end1

		add ecx,16*1024
		sub ecx,[edi+png_struct.zstream.avail_out]
		cmp dword[edi+png_struct.zstream.avail_out],0
	je .cycle4 ;while (strm.avail_out == 0)
if 0
	mov dword[edi+png_struct.zstream.avail_out],16*1024
	stdcall [deflate], esi, Z_FINISH
	add ecx,16*1024
	sub ecx,[edi+png_struct.zstream.avail_out]
	cmp eax,Z_STREAM_ERROR
	je .end1
end if
	stdcall [deflateEnd], esi

if PNG_WRITE_OPTIMIZE_CMF_SUPPORTED eq 1
	mov eax,[edi+png_struct.mode]
	and eax,PNG_HAVE_IDAT
	jnz @f
	cmp byte[edi+png_struct.compression_type],PNG_COMPRESSION_TYPE_BASE
	jne @f ;if (..==0 && ..==..)
		stdcall png_image_size, edi
		stdcall optimize_cmf, [m1], eax
	@@:
end if
	stdcall png_write_complete_chunk, edi, png_IDAT, [m1], ecx
	or dword[edi+png_struct.mode],PNG_HAVE_IDAT

	jmp @f
	.end1:
		png_debug 1, 'Z_STREAM_ERROR'
	@@:

	;free memory
	cmp dword[edi+png_struct.try_row],0
	je @f ;if (..!=0)
		stdcall png_free, edi, [edi+png_struct.try_row]
		mov dword[edi+png_struct.try_row],0
	@@:
	cmp dword[edi+png_struct.tst_row],0
	je @f ;if (..!=0)
		stdcall png_free, edi, [edi+png_struct.tst_row]
		mov dword[edi+png_struct.tst_row],0
	@@:
	stdcall png_free, edi, [m1]
.end0:
	stdcall png_free, edi, [buf_f]
.end_f:
popad
	ret
endp

;input:
; esi - rowbytes
align 4
proc copy_row_mins uses edi esi, dest:dword, sour:dword
	mov ecx,esi
	inc ecx
	mov edi,[dest]
	mov esi,[sour]
	rep movsb
	ret
endp

;void (png_structp png_ptr, bytep/*const*/ data, png_size_t size)
align 4
proc image_memory_write uses eax ebx ecx edi esi, png_ptr:dword, p2data:dword, size:dword
	mov edi,[png_ptr]
	mov esi,[edi+png_struct.io_ptr] ;esi = png_image_write_control *display
	mov ebx,[esi+png_image_write_control.output_bytes] ;ebx = ob

	; Check for overflow; this should never happen:
	mov eax,PNG_SIZE_MAX
	sub eax,ebx
	mov ecx,[size]
	cmp ecx,eax
	jg .end1 ;if (..<=..)
		; I don't think libpng ever does this, but just in case:
		cmp ecx,0
		jle .end0 ;if (..>0)
			mov eax,ebx
			add eax,ecx
			cmp [esi+png_image_write_control.memory_bytes],eax
			jl @f ;if (..>=..) ;writing
				push esi
				mov edi,[esi+png_image_write_control.memory]
				add edi,ebx
				mov esi,[p2data]
				rep movsb ;memcpy(...
				pop esi
			@@:

			; Always update the size:
			add ebx,[size]
			mov [esi+png_image_write_control.output_bytes],ebx
		.end0:
		jmp .end2
	.end1: ;else
		png_error edi, 'png_image_write_to_memory: PNG too big'
	.end2:
	ret
endp

;void (png_structp png_ptr)
align 4
proc image_memory_flush, png_ptr:dword
	ret
endp

;int (voidp argument)
align 4
proc png_image_write_memory uses ebx, argument:dword
	; The rest of the memory-specific init and write_main in an error protected
	; environment.  This case needs to use callbacks for the write operations
	; since libpng has no built in support for writing to memory.

	mov eax,[argument]
	mov ebx,[eax+png_image_write_control.image]
	mov ebx,[ebx+png_image.opaque]
	stdcall png_set_write_fn, [ebx+png_control.png_ptr], eax, image_memory_write, image_memory_flush

	stdcall png_image_write_main, [argument]
	ret
endp

;int (png_imagep image, void *memory,
;    png_alloc_size_t * PNG_RESTRICT memory_bytes, int convert_to_8bit,
;    const void *buffer, int_32 row_stride, const void *colormap)
align 4
proc png_image_write_to_memory uses ebx ecx edx edi esi, image:dword, memory:dword,\
	memory_bytes:dword, convert_to_8bit:dword, buffer:dword, row_stride:dword, colormap:dword
locals
	display png_image_write_control ;local struct
endl
;edi - display png_image_write_control

	; Write the image to the given buffer, or count the bytes if it is NULL
	mov ebx,[image]
	cmp ebx,0
	je .end0
	cmp dword[ebx+png_image.version],PNG_IMAGE_VERSION
	jne .end0 ;if (..!=0 && ..==..)
		cmp dword[memory_bytes],0
		je .end2
		cmp dword[buffer],0
		je .end2 ;if (..!=0 && ..!=0)
			; This is to give the caller an easier error detection in the NULL
			; case and guard against uninitialized variable problems:

			cmp dword[memory],0
			jne @f ;if(..==0)
				mov eax,[memory_bytes]
				mov dword[eax],0
			@@:

			stdcall png_image_write_init, ebx
			test eax,eax
			jz .end3 ;if (..!=0)
				mov ecx,sizeof.png_image_write_control
				mov edi,ebp
				sub edi,ecx
				xor eax,eax
				rep stosb ;memset(&display, 0, sizeof.display))
				sub edi,sizeof.png_image_write_control
				mov [edi+png_image_write_control.image],ebx
				mov eax,[buffer]
				mov [edi+png_image_write_control.buffer],eax
				mov eax,[row_stride]
				mov [edi+png_image_write_control.row_stride],eax
				mov eax,[colormap]
				mov [edi+png_image_write_control.colormap],eax
				mov eax,[convert_to_8bit]
				mov [edi+png_image_write_control.convert_to_8bit],eax
				mov eax,[memory]
				mov [edi+png_image_write_control.memory],eax
				mov eax,[memory_bytes]
				mov eax,[eax]
				mov [edi+png_image_write_control.memory_bytes],eax
				mov dword[edi+png_image_write_control.output_bytes], 0

				stdcall png_safe_execute, ebx, png_image_write_memory, edi
				mov ecx,eax ;ecx = result
				stdcall png_image_free, ebx

				; write_memory returns true even if we ran out of buffer.
				cmp ecx,0 ;if (..)
				je .end4
					; On out-of-buffer this function returns '0' but still updates
					; memory_bytes:

					mov edx,[edi+png_image_write_control.output_bytes]
					mov eax,[memory_bytes]
					cmp dword[memory],0
					je @f ;if (..!=0 && ..>..)
					cmp edx,[eax]
					jle @f
						xor ecx,ecx
					@@:
					mov [eax],edx
				.end4:

				mov eax,ecx
				jmp .end_f
			.end3: ;else
			xor eax,eax
			jmp .end_f
		.end2: ;else
			std_png_image_error ebx, 'png_image_write_to_memory: invalid argument'
			jmp .end_f
	.end0:
	cmp ebx,0
	je .end1 ;else if (..!=0)
		std_png_image_error ebx, 'png_image_write_to_memory: incorrect PNG_IMAGE_VERSION'
		jmp .end_f
	.end1: ;else
		xor eax,eax
.end_f:
	ret
endp

;int (png_imagep image, FILE *file, int convert_to_8bit,
;    const void *buffer, int_32 row_stride, const void *colormap)
align 4
proc png_image_write_to_stdio, image:dword, file:dword, convert_to_8bit:dword, buffer:dword, row_stride:dword, colormap:dword
	; Write the image to the given (FILE*).
;   if (image != NULL && image->version == PNG_IMAGE_VERSION)
;   {
;      if (file != NULL && buffer != NULL)
;      {
;         if (png_image_write_init(image) != 0)
;         {
;            png_image_write_control display;
;            int result;

	; This is slightly evil, but png_init_io doesn't do anything other
	; than this and we haven't changed the standard IO functions so
	; this saves a 'safe' function.

;            image->opaque->png_ptr->io_ptr = file;

;            memset(&display, 0, (sizeof display));
;            display.image = image;
;            display.buffer = buffer;
;            display.row_stride = row_stride;
;            display.colormap = colormap;
;            display.convert_to_8bit = convert_to_8bit;

;            result = png_safe_execute(image, png_image_write_main, &display);
;            png_image_free(image);
;            return result;
;         }

;         else
;            return 0;
;      }

;      else
;         return png_image_error(image,
;             "png_image_write_to_stdio: invalid argument");
;   }

;   else if (image != NULL)
;      return png_image_error(image,
;          "png_image_write_to_stdio: incorrect PNG_IMAGE_VERSION");

;   else
;      return 0;
	ret
endp

;int (png_imagep image, const char *file_name,
;    int convert_to_8bit, const void *buffer, int_32 row_stride,
;    const void *colormap)
align 4
proc png_image_write_to_file, image:dword, file_name:dword, convert_to_8bit:dword, buffer:dword, row_stride:dword, colormap:dword
	; Write the image to the named file.
;   if (image != NULL && image->version == PNG_IMAGE_VERSION)
;   {
;      if (file_name != NULL && buffer != NULL)
;      {
;         FILE *fp = fopen(file_name, "wb");

;         if (fp != NULL)
;         {
;            if (png_image_write_to_stdio(image, fp, convert_to_8bit, buffer,
;                row_stride, colormap) != 0)
;            {
;               int error; /* from fflush/fclose */

	; Make sure the file is flushed correctly.
;               if (fflush(fp) == 0 && ferror(fp) == 0)
;               {
;                  if (fclose(fp) == 0)
;                     return 1;

;                  error = errno; /* from fclose */
;               }

;               else
;               {
;                  error = errno; /* from fflush or ferror */
;                  (void)fclose(fp);
;               }

;               (void)remove(file_name);
	; The image has already been cleaned up; this is just used to
	; set the error (because the original write succeeded).

;               return png_image_error(image, strerror(error));
;            }

;            else
;            {
	; Clean up: just the opened file.
;               (void)fclose(fp);
;               (void)remove(file_name);
;               return 0;
;            }
;         }

;         else
;            return png_image_error(image, strerror(errno));
;      }

;      else
;         return png_image_error(image,
;             "png_image_write_to_file: invalid argument");
;   }

;   else if (image != NULL)
;      return png_image_error(image,
;          "png_image_write_to_file: incorrect PNG_IMAGE_VERSION");

;   else
;      return 0;
	ret
endp
end if ;SIMPLIFIED_WRITE
