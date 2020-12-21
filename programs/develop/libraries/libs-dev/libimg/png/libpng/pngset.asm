
; pngset.asm - storage of image information into info struct

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc

; The functions here are used during reads to store data from the file
; into the info struct, and during writes to store application data
; into the info struct for writing into the file.  This abstracts the
; info struct and allows us to change the structure in the future.


;void (png_structrp png_ptr, png_inforp info_ptr, png_color_16p background)
align 4
proc png_set_bKGD uses ecx edi esi, png_ptr:dword, info_ptr:dword, background:dword
	png_debug1 1, 'in %s storage function', 'bKGD'

	cmp dword[png_ptr],0
	je .end_f
	mov edi,[info_ptr]
	or edi,edi
	jz .end_f
	mov esi,[background]
	or esi,esi
	jz .end_f ;if (..==0 || ..==0 || ..==0) return

	or dword[edi+png_info_def.valid],PNG_INFO_bKGD
	add edi,png_info_def.background
	mov ecx,sizeof.png_color_16
	rep movsb
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    png_fixed_point white_x, png_fixed_point white_y, png_fixed_point red_x,
;    png_fixed_point red_y, png_fixed_point green_x, png_fixed_point green_y,
;    png_fixed_point blue_x, png_fixed_point blue_y)
align 4
proc png_set_cHRM_fixed, png_ptr:dword, info_ptr:dword,\
	white_x:dword, white_y:dword, red_x:dword, red_y:dword,\
	green_x:dword, green_y:dword, blue_x:dword, blue_y:dword
;   png_xy xy;

	png_debug1 1, 'in %s storage function', 'cHRM fixed'

;   if (png_ptr == NULL || info_ptr == NULL)
;      return;

;   xy.redx = red_x;
;   xy.redy = red_y;
;   xy.greenx = green_x;
;   xy.greeny = green_y;
;   xy.bluex = blue_x;
;   xy.bluey = blue_y;
;   xy.whitex = white_x;
;   xy.whitey = white_y;

;   if (png_colorspace_set_chromaticities(png_ptr, &info_ptr->colorspace, &xy,
;       2/* override with app values*/) != 0)
;      info_ptr->colorspace.flags |= PNG_COLORSPACE_FROM_cHRM;

;   png_colorspace_sync_info(png_ptr, info_ptr);
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    png_fixed_point int_red_X, png_fixed_point int_red_Y,
;    png_fixed_point int_red_Z, png_fixed_point int_green_X,
;    png_fixed_point int_green_Y, png_fixed_point int_green_Z,
;    png_fixed_point int_blue_X, png_fixed_point int_blue_Y,
;    png_fixed_point int_blue_Z)
align 4
proc png_set_cHRM_XYZ_fixed uses edi esi, png_ptr:dword, info_ptr:dword,\
	int_red_X:dword, int_red_Y:dword, int_red_Z:dword,\
	int_green_X:dword, int_green_Y:dword, int_green_Z:dword,\
	int_blue_X:dword, int_blue_Y:dword, int_blue_Z:dword
;   png_XYZ XYZ;

	png_debug1 1, 'in %s storage function', 'cHRM XYZ fixed'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..==0 || ..==0) return

;   XYZ.red_X = int_red_X;
;   XYZ.red_Y = int_red_Y;
;   XYZ.red_Z = int_red_Z;
;   XYZ.green_X = int_green_X;
;   XYZ.green_Y = int_green_Y;
;   XYZ.green_Z = int_green_Z;
;   XYZ.blue_X = int_blue_X;
;   XYZ.blue_Y = int_blue_Y;
;   XYZ.blue_Z = int_blue_Z;

;   if (png_colorspace_set_endpoints(png_ptr, &info_ptr->colorspace,
;       &XYZ, 2) != 0)
;      info_ptr->colorspace.flags |= PNG_COLORSPACE_FROM_cHRM;

	stdcall png_colorspace_sync_info, edi, esi
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    double white_x, double white_y, double red_x, double red_y,
;    double green_x, double green_y, double blue_x, double blue_y)
align 4
proc png_set_cHRM, png_ptr:dword, info_ptr:dword,\
	white_x:dword, white_y:dword, red_x:dword, red_y:dword,\
	green_x:dword, green_y:dword, blue_x:dword, blue_y:dword
;   png_set_cHRM_fixed(png_ptr, info_ptr,
;       png_fixed(png_ptr, white_x, "cHRM White X"),
;       png_fixed(png_ptr, white_y, "cHRM White Y"),
;       png_fixed(png_ptr, red_x, "cHRM Red X"),
;       png_fixed(png_ptr, red_y, "cHRM Red Y"),
;       png_fixed(png_ptr, green_x, "cHRM Green X"),
;       png_fixed(png_ptr, green_y, "cHRM Green Y"),
;       png_fixed(png_ptr, blue_x, "cHRM Blue X"),
;       png_fixed(png_ptr, blue_y, "cHRM Blue Y"));
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, double red_X,
;    double red_Y, double red_Z, double green_X, double green_Y, double green_Z,
;    double blue_X, double blue_Y, double blue_Z)
align 4
proc png_set_cHRM_XYZ, png_ptr:dword, info_ptr:dword, red_X:dword, red_Y:dword, red_Z:dword, green_X:dword, green_Y:dword, green_Z:dword, blue_X:dword, blue_Y:dword, blue_Z:dword
;   png_set_cHRM_XYZ_fixed(png_ptr, info_ptr,
;       png_fixed(png_ptr, red_X, "cHRM Red X"),
;       png_fixed(png_ptr, red_Y, "cHRM Red Y"),
;       png_fixed(png_ptr, red_Z, "cHRM Red Z"),
;       png_fixed(png_ptr, green_X, "cHRM Green X"),
;       png_fixed(png_ptr, green_Y, "cHRM Green Y"),
;       png_fixed(png_ptr, green_Z, "cHRM Green Z"),
;       png_fixed(png_ptr, blue_X, "cHRM Blue X"),
;       png_fixed(png_ptr, blue_Y, "cHRM Blue Y"),
;       png_fixed(png_ptr, blue_Z, "cHRM Blue Z"));
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, png_fixed_point file_gamma)
align 4
proc png_set_gAMA_fixed uses eax edi esi, png_ptr:dword, info_ptr:dword, file_gamma:dword
	png_debug1 1, 'in %s storage function', 'gAMA'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..== 0 || ..== 0) return

	mov eax,esi
	add eax,png_info_def.colorspace
	stdcall png_colorspace_set_gamma, edi, eax, [file_gamma]
	stdcall png_colorspace_sync_info, edi, esi
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, double file_gamma)
align 4
proc png_set_gAMA uses eax, png_ptr:dword, info_ptr:dword, file_gamma:dword
	cStr ,'png_set_gAMA'
	stdcall png_fixed, [png_ptr], [file_gamma], eax
	stdcall png_set_gAMA_fixed, [png_ptr], [info_ptr], eax
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, png_uint_16p hist)
align 4
proc png_set_hIST uses edi esi, png_ptr:dword, info_ptr:dword, hist:dword
;   int i;

	png_debug1 1, 'in %s storage function', 'hIST'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..== 0 || ..== 0) return

;   if (info_ptr->num_palette == 0 || info_ptr->num_palette
;       > PNG_MAX_PALETTE_LENGTH)
;   {
;      png_warning(png_ptr,
;          "Invalid palette size, hIST allocation skipped");

;      return;
;   }

;   png_free_data(png_ptr, info_ptr, PNG_FREE_HIST, 0);

	; Changed from info->num_palette to PNG_MAX_PALETTE_LENGTH in
	; version 1.2.1

;   info_ptr->hist = png_malloc_warn(png_ptr,
;       PNG_MAX_PALETTE_LENGTH * (sizeof (uint_16)));

;   if (info_ptr->hist == NULL)
;   {
;      png_warning(png_ptr, "Insufficient memory for hIST chunk data");

;      return;
;   }

;   info_ptr->free_me |= PNG_FREE_HIST;

;   for (i = 0; i < info_ptr->num_palette; i++)
;      info_ptr->hist[i] = hist[i];

	or dword[esi+png_info_def.valid], PNG_INFO_hIST
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    uint_32 width, uint_32 height, int bit_depth,
;    int color_type, int interlace_type, int compression_type, int filter_type)
align 4
proc png_set_IHDR, png_ptr:dword, info_ptr:dword,\
	width:dword, height:dword, bit_depth:dword, color_type:dword,\
	interlace_type:dword, compression_type:dword, filter_type:dword
	png_debug1 1, 'in %s storage function', 'IHDR'
pushad
	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..== 0 || ..== 0) return

	mov eax,[width]
	mov [esi+png_info_def.width],eax
	mov eax,[height]
	mov [esi+png_info_def.height],eax
	movzx eax,byte[filter_type]
	mov [esi+png_info_def.filter_type],al
	push eax
	movzx eax,byte[compression_type]
	mov [esi+png_info_def.compression_type],al
	push eax
	movzx eax,byte[interlace_type]
	mov [esi+png_info_def.interlace_type],al
	push eax
	movzx ebx,byte[color_type]
	mov [esi+png_info_def.color_type],bl
	push ebx
	movzx ecx,byte[bit_depth]
	mov [esi+png_info_def.bit_depth],cl

	stdcall png_check_IHDR, edi, dword[esi+png_info_def.width], dword[esi+png_info_def.height], ecx
		;, color_type, interlace_type, compression_type, filter_type

	cmp ebx,PNG_COLOR_TYPE_PALETTE
	jne @f ;if (..==..)
		mov byte[esi+png_info_def.channels], 1
		jmp .end0
	@@:
	mov eax,ebx
	and eax,PNG_COLOR_MASK_COLOR
	or eax,eax
	jz @f ;else if (..!=0)
		mov byte[esi+png_info_def.channels], 3
		jmp .end0
	@@: ;else
		mov byte[esi+png_info_def.channels], 1
	.end0:

	mov eax,ebx
	and eax,PNG_COLOR_MASK_ALPHA
	or eax,eax
	jz @f ;else if (..!=0)
		inc byte[esi+png_info_def.channels]
	@@:

	movzx eax,byte[esi+png_info_def.channels]
	imul eax,ecx
	mov byte[esi+png_info_def.pixel_depth],al ;channels * bit_depth

	PNG_ROWBYTES eax, [width]
	mov [esi+png_info_def.rowbytes], eax
.end_f:
popad
	ret
endp

;if PNG_oFFs_SUPPORTED
;void (png_structrp png_ptr, png_inforp info_ptr,
;    int_32 offset_x, int_32 offset_y, int unit_type)
align 4
proc png_set_oFFs uses eax esi, png_ptr:dword, info_ptr:dword, offset_x:dword, offset_y:dword, unit_type:dword
	png_debug1 1, 'in %s storage function', 'oFFs'

	cmp dword[png_ptr],0
	je @f
	mov esi,[info_ptr]
	or esi,esi
	jz @f ;if (..==0 || ..==0) return

	mov eax,[offset_x]
	mov [esi+png_info_def.x_offset],eax
	mov eax,[offset_y]
	mov [esi+png_info_def.y_offset],eax
	mov al,[unit_type]
	mov [esi+png_info_def.offset_unit_type],al
	or dword[esi+png_info_def.valid], PNG_INFO_oFFs
	@@:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    charp purpose, int_32 X0, int_32 X1, int type,
;    int nparams, charp units, charpp params)
align 4
proc png_set_pCAL uses edi esi, png_ptr:dword, info_ptr:dword, purpose:dword, X0:dword, X1:dword, type:dword, nparams:dword, units:dword, params:dword
;   png_size_t length;
;   int i;

	png_debug1 1, 'in %s storage function', 'pCAL'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f
	cmp dword[purpose],0
	je .end_f
	cmp dword[units],0
	je .end_f
	cmp dword[nparams],0
	jle @f
	cmp dword[params],0
	jne @f
		jmp .end_f
	@@: ;if (..==0 || ..==0 || ..==0 || ..==0 || (nparams > 0 && params == 0)) return

;   length = strlen(purpose) + 1;
	png_debug1 3, 'allocating purpose for info (%lu bytes)','(unsigned long)length'

	; TODO: validate format of calibration name and unit name

	; Check that the type matches the specification.
	cmp dword[type],0
	jl @f
	cmp dword[type],3
	jle .end0 ;if (..<0 || ..>3)
	@@:
		png_error edi, 'Invalid pCAL equation type'
	.end0:

	cmp dword[nparams],0
	jl @f
	cmp dword[nparams],255
	jle .end1 ;if (..<0 || ..>255)
	@@:
		png_error edi, 'Invalid pCAL parameter count'
	.end1:

	; Validate params[nparams]
;   for (i=0; i<nparams; ++i)
;   {
;      if (params[i] == NULL ||
;          !png_check_fp_string(params[i], strlen(params[i])))
;         png_error(png_ptr, "Invalid format for pCAL parameter");
;   }

;   info_ptr->pcal_purpose = png_malloc_warn(png_ptr, length);

;   if (info_ptr->pcal_purpose == NULL)
;   {
;      png_warning(png_ptr, "Insufficient memory for pCAL purpose");

;      return;
;   }

;   memcpy(info_ptr->pcal_purpose, purpose, length);

	png_debug 3, 'storing X0, X1, type, and nparams in info'
;   info_ptr->pcal_X0 = X0;
;   info_ptr->pcal_X1 = X1;
;   info_ptr->pcal_type = (byte)type;
;   info_ptr->pcal_nparams = (byte)nparams;

;   length = strlen(units) + 1;
	png_debug1 3, 'allocating units for info (%lu bytes)','(unsigned long)length'

;   info_ptr->pcal_units = png_malloc_warn(png_ptr, length);

;   if (info_ptr->pcal_units == NULL)
;   {
;      png_warning(png_ptr, "Insufficient memory for pCAL units");

;      return;
;   }

;   memcpy(info_ptr->pcal_units, units, length);

;   info_ptr->pcal_params = png_malloc_warn(png_ptr,
;       (png_size_t)((nparams + 1) * (sizeof (charp))));

;   if (info_ptr->pcal_params == NULL)
;   {
;      png_warning(png_ptr, "Insufficient memory for pCAL params");

;      return;
;   }

;   memset(info_ptr->pcal_params, 0, (nparams + 1) * (sizeof (charp)));

;   for (i = 0; i < nparams; i++)
;   {
;      length = strlen(params[i]) + 1;
;      png_debug2(3, "allocating parameter %d for info (%lu bytes)", i,
;          (unsigned long)length);

;      info_ptr->pcal_params[i] = (charp)png_malloc_warn(png_ptr, length);

;      if (info_ptr->pcal_params[i] == NULL)
;      {
;         png_warning(png_ptr, "Insufficient memory for pCAL parameter");

;         return;
;      }

;      memcpy(info_ptr->pcal_params[i], params[i], length);
;   }

	or dword[esi+png_info_def.valid],PNG_INFO_pCAL
	or dword[esi+png_info_def.free_me],PNG_FREE_PCAL
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    int unit, charp swidth, charp sheight)
align 4
proc png_set_sCAL_s, png_ptr:dword, info_ptr:dword, unit:dword, swidth:dword, sheight:dword
;   png_size_t lengthw = 0, lengthh = 0;

	png_debug1 1, 'in %s storage function', 'sCAL'

;   if (png_ptr == NULL || info_ptr == NULL)
;      return;

	; Double check the unit (should never get here with an invalid
	; unit unless this is an API call.)

;   if (unit != 1 && unit != 2)
;      png_error(png_ptr, "Invalid sCAL unit");

;   if (swidth == NULL || (lengthw = strlen(swidth)) == 0 ||
;       swidth[0] == 45 /* '-' */ || !png_check_fp_string(swidth, lengthw))
;      png_error(png_ptr, "Invalid sCAL width");

;   if (sheight == NULL || (lengthh = strlen(sheight)) == 0 ||
;       sheight[0] == 45 /* '-' */ || !png_check_fp_string(sheight, lengthh))
;      png_error(png_ptr, "Invalid sCAL height");

;   info_ptr->scal_unit = (byte)unit;

;   ++lengthw;

;   png_debug1(3, "allocating unit for info (%u bytes)", (uint)lengthw);

;   info_ptr->scal_s_width = png_malloc_warn(png_ptr, lengthw);

;   if (info_ptr->scal_s_width == NULL)
;   {
;      png_warning(png_ptr, "Memory allocation failed while processing sCAL");

;      return;
;   }

;   memcpy(info_ptr->scal_s_width, swidth, lengthw);

;   ++lengthh;

;   png_debug1(3, "allocating unit for info (%u bytes)", (uint)lengthh);

;   info_ptr->scal_s_height = png_malloc_warn(png_ptr, lengthh);

;   if (info_ptr->scal_s_height == NULL)
;   {
;      png_free (png_ptr, info_ptr->scal_s_width);
;      info_ptr->scal_s_width = NULL;

;      png_warning(png_ptr, "Memory allocation failed while processing sCAL");

;      return;
;   }

;   memcpy(info_ptr->scal_s_height, sheight, lengthh);
;
;   info_ptr->valid |= PNG_INFO_sCAL;
;   info_ptr->free_me |= PNG_FREE_SCAL;
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int unit,
;    double width, double height)
align 4
proc png_set_sCAL, png_ptr:dword, info_ptr:dword, unit:dword, width:dword, height:dword
	png_debug1 1, 'in %s storage function', 'sCAL'

	; Check the arguments.
;   if (width <= 0)
;      png_warning(png_ptr, "Invalid sCAL width ignored");

;   else if (height <= 0)
;      png_warning(png_ptr, "Invalid sCAL height ignored");

;   else
;   {
	; Convert 'width' and 'height' to ASCII.
;      char swidth[PNG_sCAL_MAX_DIGITS+1];
;      char sheight[PNG_sCAL_MAX_DIGITS+1];

;      png_ascii_from_fp(png_ptr, swidth, (sizeof swidth), width,
;          PNG_sCAL_PRECISION);
;      png_ascii_from_fp(png_ptr, sheight, (sizeof sheight), height,
;          PNG_sCAL_PRECISION);

;      png_set_sCAL_s(png_ptr, info_ptr, unit, swidth, sheight);
;   }
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int unit,
;    png_fixed_point width, png_fixed_point height)
align 4
proc png_set_sCAL_fixed uses ebx ecx edi, png_ptr:dword, info_ptr:dword, unit:dword, width:dword, height:dword
locals
	swidth  rb PNG_sCAL_MAX_DIGITS+1 ;char[]
	sheight rb PNG_sCAL_MAX_DIGITS+1 ;char[]
endl
	png_debug1 1, 'in %s storage function', 'sCAL'
	mov edi,[png_ptr]

	; Check the arguments.
	cmp dword[width],0
	jg @f ;if (..<=0)
		png_warning edi, 'Invalid sCAL width ignored'
		jmp .end0
	@@:
	cmp dword[height],0
	jg @f ;else if (..<=0)
		png_warning edi, 'Invalid sCAL height ignored'
		jmp .end0
	@@: ;else
		; Convert 'width' and 'height' to ASCII.
		mov ebx,ebp
		sub ebx,PNG_sCAL_MAX_DIGITS+1 ;sheight
		mov ecx,ebx
		sub ecx,PNG_sCAL_MAX_DIGITS+1 ;swidth

		stdcall png_ascii_from_fixed, edi, ecx, PNG_sCAL_MAX_DIGITS+1, [width]
		stdcall png_ascii_from_fixed, edi, ebx, PNG_sCAL_MAX_DIGITS+1, [height]

		stdcall png_set_sCAL_s, edi, [info_ptr], [unit], ecx, ebx
	.end0:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    uint_32 res_x, uint_32 res_y, int unit_type)
align 4
proc png_set_pHYs, png_ptr:dword, info_ptr:dword, res_x:dword, res_y:dword, unit_type:dword
	png_debug1 1, 'in %s storage function', 'pHYs'

;   if (png_ptr == NULL || info_ptr == NULL)
;      return;

;   info_ptr->x_pixels_per_unit = res_x;
;   info_ptr->y_pixels_per_unit = res_y;
;   info_ptr->phys_unit_type = (byte)unit_type;
;   info_ptr->valid |= PNG_INFO_pHYs;
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, png_colorp palette, int num_palette)
align 4
proc png_set_PLTE uses eax edi esi, png_ptr:dword, info_ptr:dword, palette:dword, num_palette:dword
;   uint_32 max_palette_length;

	png_debug1 1, 'in %s storage function', 'PLTE'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..==0 || ..==0) return

;   max_palette_length = (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ?
;      (1 << info_ptr->bit_depth) : PNG_MAX_PALETTE_LENGTH;

;   if (num_palette < 0 || num_palette > (int) max_palette_length)
;   {
;      if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
;         png_error(png_ptr, "Invalid palette length");

;      else
;      {
;         png_warning(png_ptr, "Invalid palette length");

;         return;
;      }
;   }

;   if ((num_palette > 0 && palette == NULL) ||
;      (num_palette == 0
if PNG_MNG_FEATURES_SUPPORTED eq 1
;            && (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0
end if
;      ))
;   {
;      png_error(png_ptr, "Invalid palette");
;   }

	; It may not actually be necessary to set png_ptr->palette here;
	; we do it for backward compatibility with the way the png_handle_tRNS
	; function used to do the allocation.

	; 1.6.0: the above statement appears to be incorrect; something has to set
	; the palette inside png_struct on read.

	stdcall png_free_data, edi, esi, PNG_FREE_PLTE, 0

	; Changed in libpng-1.2.1 to allocate PNG_MAX_PALETTE_LENGTH instead
	; of num_palette entries, in case of an invalid PNG file or incorrect
	; call to png_set_PLTE() with too-large sample values.

;   png_ptr->palette = png_calloc(png_ptr,
;       PNG_MAX_PALETTE_LENGTH * (sizeof (png_color)));

;   if (num_palette > 0)
;      memcpy(png_ptr->palette, palette, num_palette * (sizeof (png_color)));
	mov eax,[edi+png_struct.palette]
	mov [esi+png_info_def.palette],eax
;   info_ptr->num_palette = png_ptr->num_palette = (uint_16)num_palette;

	or dword[esi+png_info_def.free_me], PNG_FREE_PLTE
	or dword[esi+png_info_def.valid], PNG_INFO_PLTE
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, png_color_8p sig_bit)
align 4
proc png_set_sBIT, png_ptr:dword, info_ptr:dword, sig_bit:dword
	png_debug1 1, 'in %s storage function', 'sBIT'

;   if (png_ptr == NULL || info_ptr == NULL || sig_bit == NULL)
;      return;

;   info_ptr->sig_bit = *sig_bit;
;   info_ptr->valid |= PNG_INFO_sBIT;
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int srgb_intent)
align 4
proc png_set_sRGB uses eax edi esi, png_ptr:dword, info_ptr:dword, srgb_intent:dword
	png_debug1 1, 'in %s storage function', 'sRGB'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..==0 || ..==0)

	mov eax,esi
	add eax,png_info_def.colorspace
	stdcall png_colorspace_set_sRGB, edi, eax, [srgb_intent]
	stdcall png_colorspace_sync_info, edi, esi
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int srgb_intent)
align 4
proc png_set_sRGB_gAMA_and_cHRM, png_ptr:dword, info_ptr:dword, srgb_intent:dword
	png_debug1 1, 'in %s storage function', 'sRGB_gAMA_and_cHRM'

;   if (png_ptr == NULL || info_ptr == NULL)
;      return;

;   if (png_colorspace_set_sRGB(png_ptr, &info_ptr->colorspace,
;       srgb_intent) != 0)
;   {
	; This causes the gAMA and cHRM to be written too
;      info_ptr->colorspace.flags |=
;         PNG_COLORSPACE_FROM_gAMA|PNG_COLORSPACE_FROM_cHRM;
;   }

;   png_colorspace_sync_info(png_ptr, info_ptr);
.end_f:
	ret
endp

;void (const_structrp png_ptr, png_inforp info_ptr,
;    charp name, int compression_type, bytep profile, uint_32 proflen)
align 4
proc png_set_iCCP uses edi esi, png_ptr:dword, info_ptr:dword, name:dword, compression_type:dword, profile:dword, proflen:dword
;   charp new_iccp_name;
;   bytep new_iccp_profile;
;   png_size_t length;

	png_debug1 1, 'in %s storage function', 'iCCP'

	mov esi,[info_ptr]
;   if (png_ptr == NULL || info_ptr == NULL || name == NULL || profile == NULL)
;      return;

;   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
;      png_app_error(png_ptr, "Invalid iCCP compression method");

	; Set the colorspace first because this validates the profile; do not
	; override previously set app cHRM or gAMA here (because likely as not the
	; application knows better than libpng what the correct values are.)  Pass
	; the info_ptr color_type field to png_colorspace_set_ICC because in the
	; write case it has not yet been stored in png_ptr.

;   {
;      int result = png_colorspace_set_ICC(png_ptr, &info_ptr->colorspace, name,
;          proflen, profile, info_ptr->color_type);

;      png_colorspace_sync_info(png_ptr, info_ptr);

	; Don't do any of the copying if the profile was bad, or inconsistent.
;      if (result == 0)
;         return;

	; But do write the gAMA and cHRM chunks from the profile.
;      info_ptr->colorspace.flags |=
;         PNG_COLORSPACE_FROM_gAMA|PNG_COLORSPACE_FROM_cHRM;
;   }

;   length = strlen(name)+1;
;   new_iccp_name = png_malloc_warn(png_ptr, length);

;   if (new_iccp_name == NULL)
;   {
;      png_benign_error(png_ptr, "Insufficient memory to process iCCP chunk");
;      return;
;   }

;   memcpy(new_iccp_name, name, length);
;   new_iccp_profile = png_malloc_warn(png_ptr, proflen);

;   if (new_iccp_profile == NULL)
;   {
;      png_free(png_ptr, new_iccp_name);
;      png_benign_error(png_ptr,
;          "Insufficient memory to process iCCP profile");
;      return;
;   }

;   memcpy(new_iccp_profile, profile, proflen);

;   png_free_data(png_ptr, info_ptr, PNG_FREE_ICCP, 0);

;   info_ptr->iccp_proflen = proflen;
;   info_ptr->iccp_name = new_iccp_name;
;   info_ptr->iccp_profile = new_iccp_profile;
	or dword[esi+png_info_def.free_me],PNG_FREE_ICCP
	or dword[esi+png_info_def.valid],PNG_INFO_iCCP
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, png_textp text_ptr, int num_text)
align 4
proc png_set_text uses eax edi, png_ptr:dword, info_ptr:dword, text_ptr:dword, num_text:dword
	mov edi,[png_ptr]
	stdcall png_set_text_2, edi, [info_ptr], [text_ptr], [num_text]

	or eax,eax
	jz @f ;if (..!=0)
		png_error edi, 'Insufficient memory to store text'
	@@:
	ret
endp

;int (png_structrp png_ptr, png_inforp info_ptr,
;    png_textp text_ptr, int num_text)
align 4
proc png_set_text_2, png_ptr:dword, info_ptr:dword, text_ptr:dword, num_text:dword
;   int i;

	png_debug1 1, 'in %lx storage function', 'png_ptr == NULL ? 0xabadca11 : (unsigned long)png_ptr->chunk_name'

;   if (png_ptr == NULL || info_ptr == NULL || num_text <= 0 || text_ptr == NULL)
;      return(0);

	; Make sure we have enough space in the "text" array in info_struct
	; to hold all of the incoming text_ptr objects.  This compare can't overflow
	; because max_text >= num_text (anyway, subtract of two positive integers
	; can't overflow in any case.)

;   if (num_text > info_ptr->max_text - info_ptr->num_text)
;   {
;      int old_num_text = info_ptr->num_text;
;      int max_text;
;      png_textp new_text = NULL;

;      /* Calculate an appropriate max_text, checking for overflow. */
;      max_text = old_num_text;
;      if (num_text <= INT_MAX - max_text)
;      {
;         max_text += num_text;

;         /* Round up to a multiple of 8 */
;         if (max_text < INT_MAX-8)
;            max_text = (max_text + 8) & ~0x7;

;         else
;            max_text = INT_MAX;

	; Now allocate a new array and copy the old members in; this does all
	; the overflow checks.

;         new_text = png_realloc_array(png_ptr,
;             info_ptr->text, old_num_text, max_text-old_num_text,
;             sizeof *new_text);
;      }

;      if (new_text == NULL)
;      {
;         png_chunk_report(png_ptr, "too many text chunks",
;             PNG_CHUNK_WRITE_ERROR);

;         return 1;
;      }

;      png_free(png_ptr, info_ptr->text);

;      info_ptr->text = new_text;
;      info_ptr->free_me |= PNG_FREE_TEXT;
;      info_ptr->max_text = max_text;
;      /* num_text is adjusted below as the entries are copied in */

;      png_debug1(3, "allocated %d entries for info_ptr->text", max_text);
;   }

;   for (i = 0; i < num_text; i++)
;   {
;      size_t text_length, key_len;
;      size_t lang_len, lang_key_len;
;      png_textp textp = &(info_ptr->text[info_ptr->num_text]);

;      if (text_ptr[i].key == NULL)
;          continue;

;      if (text_ptr[i].compression < PNG_TEXT_COMPRESSION_NONE ||
;          text_ptr[i].compression >= PNG_TEXT_COMPRESSION_LAST)
;      {
;         png_chunk_report(png_ptr, "text compression mode is out of range",
;             PNG_CHUNK_WRITE_ERROR);
;         continue;
;      }

;      key_len = strlen(text_ptr[i].key);

;      if (text_ptr[i].compression <= 0)
;      {
;         lang_len = 0;
;         lang_key_len = 0;
;      }

;      else
if PNG_iTXt_SUPPORTED eq 1
;      {
;         /* Set iTXt data */

;         if (text_ptr[i].lang != NULL)
;            lang_len = strlen(text_ptr[i].lang);
;
;         else
;            lang_len = 0;

;         if (text_ptr[i].lang_key != NULL)
;            lang_key_len = strlen(text_ptr[i].lang_key);

;         else
;            lang_key_len = 0;
;      }
else ;iTXt
;      {
;         png_chunk_report(png_ptr, "iTXt chunk not supported",
;             PNG_CHUNK_WRITE_ERROR);
;         continue;
;      }
end if

;      if (text_ptr[i].text == NULL || text_ptr[i].text[0] == '\0')
;      {
;         text_length = 0;
if PNG_iTXt_SUPPORTED eq 1
;         if (text_ptr[i].compression > 0)
;            textp->compression = PNG_ITXT_COMPRESSION_NONE;

;         else
end if
;            textp->compression = PNG_TEXT_COMPRESSION_NONE;
;      }

;      else
;      {
;         text_length = strlen(text_ptr[i].text);
;         textp->compression = text_ptr[i].compression;
;      }

;      textp->key = png_malloc_base(png_ptr,
;          key_len + text_length + lang_len + lang_key_len + 4);

;      if (textp->key == NULL)
;      {
;         png_chunk_report(png_ptr, "text chunk: out of memory",
;             PNG_CHUNK_WRITE_ERROR);

;         return 1;
;      }

;      png_debug2(2, "Allocated %lu bytes at %p in png_set_text",
;          (unsigned long)(uint_32)
;          (key_len + lang_len + lang_key_len + text_length + 4),
;          textp->key);

;      memcpy(textp->key, text_ptr[i].key, key_len);
;      *(textp->key + key_len) = '\0';

;      if (text_ptr[i].compression > 0)
;      {
;         textp->lang = textp->key + key_len + 1;
;         memcpy(textp->lang, text_ptr[i].lang, lang_len);
;         *(textp->lang + lang_len) = '\0';
;         textp->lang_key = textp->lang + lang_len + 1;
;         memcpy(textp->lang_key, text_ptr[i].lang_key, lang_key_len);
;         *(textp->lang_key + lang_key_len) = '\0';
;         textp->text = textp->lang_key + lang_key_len + 1;
;      }

;      else
;      {
;         textp->lang=NULL;
;         textp->lang_key=NULL;
;         textp->text = textp->key + key_len + 1;
;      }

;      if (text_length != 0)
;         memcpy(textp->text, text_ptr[i].text, text_length);

;      *(textp->text + text_length) = '\0';

if PNG_iTXt_SUPPORTED eq 1
;      if (textp->compression > 0)
;      {
;         textp->text_length = 0;
;         textp->itxt_length = text_length;
;      }

;      else
end if
;      {
;         textp->text_length = text_length;
;         textp->itxt_length = 0;
;      }

;      info_ptr->num_text++;
;      png_debug1(3, "transferred text chunk %d", info_ptr->num_text);
;   }

	xor eax,eax
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, png_timep mod_time)
align 4
proc png_set_tIME uses eax ebx ecx edi esi, png_ptr:dword, info_ptr:dword, mod_time:dword
	png_debug1 1, 'in %s storage function', 'tIME'

	mov ebx,[png_ptr]
	or ebx,ebx
	jz .end_f
	mov edi,[info_ptr]
	or edi,edi
	jz .end_f
	mov esi,[mod_time]
	or esi,esi
	jz .end_f
	mov eax,[ebx+png_struct.mode]
	and eax,PNG_WROTE_tIME
	or eax,eax
	jnz .end_f ;if (..==0 || ..==0 || ..==0 || ..!=0) return

	cmp byte[esi+png_time.month],0
	je @f
	cmp byte[esi+png_time.month],12
	jg @f
	cmp byte[esi+png_time.day],0
	je @f
	cmp byte[esi+png_time.day],31
	jg @f
	cmp byte[esi+png_time.hour],23
	jg @f
	cmp byte[esi+png_time.minute],59
	jg @f
	cmp byte[esi+png_time.second],60
	jle .end0
	@@: ;if (..==0 || ..>.. || ..==0 || ..>.. || ..>.. || ..>.. || ..>..)
		png_warning ebx, 'Ignoring invalid time value'
		jmp .end_f
	.end0:

	mov ecx,sizeof.png_time
	push edi
	add edi,png_info_def.mod_time
	rep movsb
	pop edi
	or dword[edi+png_info_def.valid],PNG_INFO_tIME
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr,
;    bytep trans_alpha, int num_trans, png_color_16p trans_color)
align 4
proc png_set_tRNS, png_ptr:dword, info_ptr:dword, trans_alpha:dword, num_trans:dword, trans_color:dword
	png_debug1 1, 'in %s storage function', 'tRNS'

;   if (png_ptr == NULL || info_ptr == NULL)
;      return;

;   if (trans_alpha != NULL)
;   {
	; It may not actually be necessary to set png_ptr->trans_alpha here;
	; we do it for backward compatibility with the way the png_handle_tRNS
	; function used to do the allocation.

	; 1.6.0: The above statement is incorrect; png_handle_tRNS effectively
	; relies on png_set_tRNS storing the information in png_struct
	; (otherwise it won't be there for the code in pngrtran.c).


;       png_free_data(png_ptr, info_ptr, PNG_FREE_TRNS, 0);

;       if (num_trans > 0 && num_trans <= PNG_MAX_PALETTE_LENGTH)
;       {
	; Changed from num_trans to PNG_MAX_PALETTE_LENGTH in version 1.2.1
;          info_ptr->trans_alpha = png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH);
;          memcpy(info_ptr->trans_alpha, trans_alpha, (png_size_t)num_trans);
;       }
;       png_ptr->trans_alpha = info_ptr->trans_alpha;
;   }

;   if (trans_color != NULL)
;   {
if PNG_WARNINGS_SUPPORTED eq 1
;      if (info_ptr->bit_depth < 16)
;      {
;         int sample_max = (1 << info_ptr->bit_depth) - 1;
;
;         if ((info_ptr->color_type == PNG_COLOR_TYPE_GRAY &&
;             trans_color->gray > sample_max) ||
;             (info_ptr->color_type == PNG_COLOR_TYPE_RGB &&
;             (trans_color->red > sample_max ||
;             trans_color->green > sample_max ||
;             trans_color->blue > sample_max)))
;            png_warning(png_ptr,
;                "tRNS chunk has out-of-range samples for bit_depth");
;      }
end if

;      info_ptr->trans_color = *trans_color;

;      if (num_trans == 0)
;         num_trans = 1;
;   }

;   info_ptr->num_trans = (uint_16)num_trans;

;   if (num_trans != 0)
;   {
;      info_ptr->valid |= PNG_INFO_tRNS;
;      info_ptr->free_me |= PNG_FREE_TRNS;
;   }
	ret
endp

;if PNG_sPLT_SUPPORTED
;void (png_structrp png_ptr,
;    png_inforp info_ptr, png_sPLT_tp entries, int nentries)

;  entries        - array of png_sPLT_t structures
;                   to be added to the list of palettes
;                   in the info structure.

;  nentries       - number of palette structures to be
;                   added.

align 4
proc png_set_sPLT, png_ptr:dword, info_ptr:dword, entries:dword, nentries:dword
;   png_sPLT_tp np;

;   if (png_ptr == NULL || info_ptr == NULL || nentries <= 0 || entries == NULL)
;      return;

	; Use the internal realloc function, which checks for all the possible
	; overflows.  Notice that the parameters are (int) and (size_t)

;   np = png_realloc_array(png_ptr,
;       info_ptr->splt_palettes, info_ptr->splt_palettes_num, nentries,
;       sizeof *np);

;   if (np == NULL)
;   {
;      /* Out of memory or too many chunks */
;      png_chunk_report(png_ptr, "too many sPLT chunks", PNG_CHUNK_WRITE_ERROR);

;      return;
;   }

;   png_free(png_ptr, info_ptr->splt_palettes);
;   info_ptr->splt_palettes = np;
;   info_ptr->free_me |= PNG_FREE_SPLT;

;   np += info_ptr->splt_palettes_num;

;   do
;   {
;      png_size_t length;

;      /* Skip invalid input entries */
;      if (entries->name == NULL || entries->entries == NULL)
;      {
;         /* png_handle_sPLT doesn't do this, so this is an app error */
;         png_app_error(png_ptr, "png_set_sPLT: invalid sPLT");
;         /* Just skip the invalid entry */
;         continue;
;      }

;      np->depth = entries->depth;

	; In the event of out-of-memory just return - there's no point keeping
	; on trying to add sPLT chunks.

;      length = strlen(entries->name) + 1;
;      np->name = png_malloc_base(png_ptr, length);

;      if (np->name == NULL)
;         break;

;      memcpy(np->name, entries->name, length);

	; IMPORTANT: we have memory now that won't get freed if something else
	; goes wrong; this code must free it.  png_malloc_array produces no
	; warnings; use a png_chunk_report (below) if there is an error.

;      np->entries = png_malloc_array(png_ptr,
;          entries->nentries, sizeof (png_sPLT_entry));

;      if (np->entries == NULL)
;      {
;         png_free(png_ptr, np->name);
;         np->name = NULL;
;         break;
;      }

;      np->nentries = entries->nentries;
	; This multiply can't overflow because png_malloc_array has already
	; checked it when doing the allocation.

;      memcpy(np->entries, entries->entries,
;          entries->nentries * sizeof (png_sPLT_entry));

	; Note that 'continue' skips the advance of the out pointer and out
	; count, so an invalid entry is not added.

;      info_ptr->valid |= PNG_INFO_sPLT;
;      ++(info_ptr->splt_palettes_num);
;      ++np;
;   }
;   while (++entries, --nentries);

;   if (nentries > 0)
;      png_chunk_report(png_ptr, "sPLT out of memory", PNG_CHUNK_WRITE_ERROR);
	ret
endp
;end if /* sPLT */

;if PNG_STORE_UNKNOWN_CHUNKS_SUPPORTED
;byte (png_structrp png_ptr, int location)
align 4
proc check_location, png_ptr:dword, location:dword
;   location &= (PNG_HAVE_IHDR|PNG_HAVE_PLTE|PNG_AFTER_IDAT);

	; New in 1.6.0; copy the location and check it.  This is an API
	; change; previously the app had to use the
	; png_set_unknown_chunk_location API below for each chunk.

;   if (location == 0 && (png_ptr->mode & PNG_IS_READ_STRUCT) == 0)
;   {
;      /* Write struct, so unknown chunks come from the app */
;      png_app_warning(png_ptr,
;          "png_set_unknown_chunks now expects a valid location");
;      /* Use the old behavior */
;      location = (byte)(png_ptr->mode &
;          (PNG_HAVE_IHDR|PNG_HAVE_PLTE|PNG_AFTER_IDAT));
;   }

	; This need not be an internal error - if the app calls
	; png_set_unknown_chunks on a read pointer it must get the location right.

;   if (location == 0)
;      png_error(png_ptr, "invalid location in png_set_unknown_chunks");

	; Now reduce the location to the top-most set bit by removing each least
	; significant bit in turn.

;   while (location != (location & -location))
;      location &= ~(location & -location);

	; The cast is safe because 'location' is a bit mask and only the low four
	; bits are significant.

;   return (byte)location;
	ret
endp

;void (png_structrp png_ptr,
;    png_inforp info_ptr, png_unknown_chunkp unknowns, int num_unknowns)
align 4
proc png_set_unknown_chunks uses edi esi, png_ptr:dword, info_ptr:dword, unknowns:dword, num_unknowns:dword
;   png_unknown_chunkp np;

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f
	cmp dword[num_unknowns],0
	jle .end_f
	cmp dword[unknowns],0
	je .end_f ;if (..== 0 || ..== 0 || ..<=0 || ..==0) return

	; Check for the failure cases where support has been disabled at compile
	; time.  This code is hardly ever compiled - it's here because
	; STORE_UNKNOWN_CHUNKS is set by both read and write code (compiling in this
	; code) but may be meaningless if the read or write handling of unknown
	; chunks is not compiled in.

;#  if !defined(PNG_READ_UNKNOWN_CHUNKS_SUPPORTED) && \
;      defined(PNG_READ_SUPPORTED)
;      if ((png_ptr->mode & PNG_IS_READ_STRUCT) != 0)
;      {
;         png_app_error(png_ptr, "no unknown chunk support on read");
;
;         return;
;      }
;#  endif
;#  if !defined(PNG_WRITE_UNKNOWN_CHUNKS_SUPPORTED) && \
;      defined(PNG_WRITE_SUPPORTED)
;      if ((png_ptr->mode & PNG_IS_READ_STRUCT) == 0)
;      {
;         png_app_error(png_ptr, "no unknown chunk support on write");
;
;         return;
;      }
;#  endif

	; Prior to 1.6.0 this code used png_malloc_warn; however, this meant that
	; unknown critical chunks could be lost with just a warning resulting in
	; undefined behavior.  Now png_chunk_report is used to provide behavior
	; appropriate to read or write.

;   np = png_realloc_array(png_ptr,
;       info_ptr->unknown_chunks, info_ptr->unknown_chunks_num, num_unknowns,
;       sizeof *np);

;   if (np == NULL)
;   {
;      png_chunk_report(png_ptr, "too many unknown chunks",
;          PNG_CHUNK_WRITE_ERROR);

;      return;
;   }

;   png_free(png_ptr, info_ptr->unknown_chunks);
;   info_ptr->unknown_chunks = np; /* safe because it is initialized */
;   info_ptr->free_me |= PNG_FREE_UNKN;

;   np += info_ptr->unknown_chunks_num;

	; Increment unknown_chunks_num each time round the loop to protect the
	; just-allocated chunk data.

;   for (; num_unknowns > 0; --num_unknowns, ++unknowns)
;   {
;      memcpy(np->name, unknowns->name, (sizeof np->name));
;      np->name[(sizeof np->name)-1] = '\0';
;      np->location = check_location(png_ptr, unknowns->location);

;      if (unknowns->size == 0)
;      {
;         np->data = NULL;
;         np->size = 0;
;      }

;      else
;      {
;         np->data = png_malloc_base(png_ptr, unknowns->size);

;         if (np->data == NULL)
;         {
;            png_chunk_report(png_ptr, "unknown chunk: out of memory",
;                PNG_CHUNK_WRITE_ERROR);
;            /* But just skip storing the unknown chunk */
;            continue;
;         }

;         memcpy(np->data, unknowns->data, unknowns->size);
;         np->size = unknowns->size;
;      }

	; These increments are skipped on out-of-memory for the data - the
	; unknown chunk entry gets overwritten if the png_chunk_report returns.
	; This is correct in the read case (the chunk is just dropped.)

;      ++np;
;      ++(info_ptr->unknown_chunks_num);
;   }
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int chunk, int location)
align 4
proc png_set_unknown_chunk_location, png_ptr:dword, info_ptr:dword, chunk:dword, location:dword
	; This API is pretty pointless in 1.6.0 because the location can be set
	; before the call to png_set_unknown_chunks.

	; TODO: add a png_app_warning in 1.7

;   if (png_ptr != NULL && info_ptr != NULL && chunk >= 0 &&
;      chunk < info_ptr->unknown_chunks_num)
;   {
;      if ((location & (PNG_HAVE_IHDR|PNG_HAVE_PLTE|PNG_AFTER_IDAT)) == 0)
;      {
;         png_app_error(png_ptr, "invalid unknown chunk location");
	; Fake out the pre 1.6.0 behavior:
;         if ((location & PNG_HAVE_IDAT) != 0) /* undocumented! */
;            location = PNG_AFTER_IDAT;

;         else
;            location = PNG_HAVE_IHDR; /* also undocumented */
;      }

;      info_ptr->unknown_chunks[chunk].location =
;         check_location(png_ptr, location);
;   }
	ret
endp
;end if /* STORE_UNKNOWN_CHUNKS */

;uint_32 (png_structrp png_ptr, uint_32 mng_features)
align 4
proc png_permit_mng_features, png_ptr:dword, mng_features:dword
	png_debug 1, 'in png_permit_mng_features'

;   if (png_ptr == NULL)
;      return 0;

;   png_ptr->mng_features_permitted = mng_features & PNG_ALL_MNG_FEATURES;

;   return png_ptr->mng_features_permitted;
	ret
endp

;if PNG_HANDLE_AS_UNKNOWN_SUPPORTED
;uint (bytep list, uint count, bytep add, int keep)
align 4
proc add_one_chunk, list:dword, count:dword, p3add:dword, keep:dword
;   uint i;

	; Utility function: update the 'keep' state of a chunk if it is already in
	; the list, otherwise add it to the list.

;   for (i=0; i<count; ++i, list += 5)
;   {
;      if (memcmp(list, p3add, 4) == 0)
;      {
;         list[4] = (byte)keep;

;         return count;
;      }
;   }

;   if (keep != PNG_HANDLE_CHUNK_AS_DEFAULT)
;   {
;      ++count;
;      memcpy(list, p3add, 4);
;      list[4] = (byte)keep;
;   }

;   return count;
	ret
endp

;void (png_structrp png_ptr, int keep, bytep chunk_list, int num_chunks_in)
align 4
proc png_set_keep_unknown_chunks uses edi, png_ptr:dword, keep:dword, chunk_list:dword, num_chunks_in:dword
;   bytep new_list;
;   uint num_chunks, old_num_chunks;

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f ;if (..== 0) return

;   if (keep < 0 || keep >= PNG_HANDLE_CHUNK_LAST)
;   {
;      png_app_error(png_ptr, "png_set_keep_unknown_chunks: invalid keep");

;      return;
;   }

;   if (num_chunks_in <= 0)
;   {
;      png_ptr->unknown_default = keep;

	; '0' means just set the flags, so stop here
;      if (num_chunks_in == 0)
;        return;
;   }

;   if (num_chunks_in < 0)
;   {
	; Ignore all unknown chunks and all chunks recognized by
	; libpng except for IHDR, PLTE, tRNS, IDAT, and IEND

;      static byte chunks_to_ignore[] = {
;         98,  75,  71,  68, '\0',  /* bKGD */
;         99,  72,  82,  77, '\0',  /* cHRM */
;        103,  65,  77,  65, '\0',  /* gAMA */
;        104,  73,  83,  84, '\0',  /* hIST */
;        105,  67,  67,  80, '\0',  /* iCCP */
;        105,  84,  88, 116, '\0',  /* iTXt */
;        111,  70,  70, 115, '\0',  /* oFFs */
;        112,  67,  65,  76, '\0',  /* pCAL */
;        112,  72,  89, 115, '\0',  /* pHYs */
;        115,  66,  73,  84, '\0',  /* sBIT */
;        115,  67,  65,  76, '\0',  /* sCAL */
;        115,  80,  76,  84, '\0',  /* sPLT */
;        115,  84,  69,  82, '\0',  /* sTER */
;        115,  82,  71,  66, '\0',  /* sRGB */
;        116,  69,  88, 116, '\0',  /* tEXt */
;        116,  73,  77,  69, '\0',  /* tIME */
;        122,  84,  88, 116, '\0'   /* zTXt */
;      };

;      chunk_list = chunks_to_ignore;
;      num_chunks = (uint)/*SAFE*/(sizeof chunks_to_ignore)/5U;
;   }

;   else /* num_chunks_in > 0 */
;   {
;      if (chunk_list == NULL)
;      {
;         /* Prior to 1.6.0 this was silently ignored, now it is an app_error
	; which can be switched off.

;         png_app_error(png_ptr, "png_set_keep_unknown_chunks: no chunk list");

;         return;
;      }

;      num_chunks = num_chunks_in;
;   }

;   old_num_chunks = png_ptr->num_chunk_list;
;   if (png_ptr->chunk_list == NULL)
;      old_num_chunks = 0;

	; Since num_chunks is always restricted to UINT_MAX/5 this can't overflow.

;   if (num_chunks + old_num_chunks > UINT_MAX/5)
;   {
;      png_app_error(png_ptr, "png_set_keep_unknown_chunks: too many chunks");

;      return;
;   }

	; If these chunks are being reset to the default then no more memory is
	; required because add_one_chunk above doesn't extend the list if the 'keep'
	; parameter is the default.

;   if (keep != 0)
;   {
;      new_list = png_malloc(png_ptr, 5 * (num_chunks + old_num_chunks));
;
;      if (old_num_chunks > 0)
;         memcpy(new_list, png_ptr->chunk_list, 5*old_num_chunks);
;   }

;   else if (old_num_chunks > 0)
;      new_list = png_ptr->chunk_list;

;   else
;      new_list = NULL;

	; Add the new chunks together with each one's handling code.  If the chunk
	; already exists the code is updated, otherwise the chunk is added to the
	; end.  (In libpng 1.6.0 order no longer matters because this code enforces
	; the earlier convention that the last setting is the one that is used.)

;   if (new_list != NULL)
;   {
;      bytep inlist;
;      bytep outlist;
;      uint i;

;      for (i=0; i<num_chunks; ++i)
;      {
;         old_num_chunks = add_one_chunk(new_list, old_num_chunks,
;             chunk_list+5*i, keep);
;      }

	; Now remove any spurious 'default' entries.
;      num_chunks = 0;
;      for (i=0, inlist=outlist=new_list; i<old_num_chunks; ++i, inlist += 5)
;      {
;         if (inlist[4])
;         {
;            if (outlist != inlist)
;               memcpy(outlist, inlist, 5);
;            outlist += 5;
;            ++num_chunks;
;         }
;      }

	; This means the application has removed all the specialized handling.
;      if (num_chunks == 0)
;      {
;         if (png_ptr->chunk_list != new_list)
;            png_free(png_ptr, new_list);
;
;         new_list = NULL;
;      }
;   }
;
;   else
;      num_chunks = 0;
;
;   png_ptr->num_chunk_list = num_chunks;
;
;   if (png_ptr->chunk_list != new_list)
;   {
;      if (png_ptr->chunk_list != NULL)
;         png_free(png_ptr, png_ptr->chunk_list);
;
;      png_ptr->chunk_list = new_list;
;   }
.end_f:
	ret
endp
;end if

;void (png_structrp png_ptr, voidp user_chunk_ptr, png_user_chunk_ptr read_user_chunk_fn)
align 4
proc png_set_read_user_chunk_fn uses eax edi, png_ptr:dword, user_chunk_ptr:dword, read_user_chunk_fn:dword
	png_debug 1, 'in png_set_read_user_chunk_fn'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f

	mov eax,[read_user_chunk_fn]
	mov [edi+png_struct.read_user_chunk_fn],eax
	mov eax,[user_chunk_ptr]
	mov [edi+png_struct.user_chunk_ptr],eax
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, bytepp row_pointers)
align 4
proc png_set_rows uses eax edi esi, png_ptr:dword, info_ptr:dword, row_pointers:dword
	png_debug1 1, 'in %s storage function', 'rows'

	mov edi,[png_ptr]
	or edi,edi
	jz .end_f
	mov esi,[info_ptr]
	or esi,esi
	jz .end_f ;if (..==0 || ..==0) return

	mov eax,[row_pointers]
	cmp dword[esi+png_info_def.row_pointers],0
	je @f
	cmp [esi+png_info_def.row_pointers],eax
	je @f ;if (..!=0 && ..!=..)
		stdcall png_free_data, edi, esi, PNG_FREE_ROWS, 0
	@@:
	mov [esi+png_info_def.row_pointers],eax

	or eax,eax
	jz .end_f ;if (..!=0)
		or dword[esi+png_info_def.valid],PNG_INFO_IDAT
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_size_t size)
align 4
proc png_set_compression_buffer_size uses edi, png_ptr:dword, size:dword
	mov edi,[png_ptr]
	or edi,edi
	jz .end_f ;if (..==0) return

;   if (size == 0 || size > PNG_UINT_31_MAX)
;      png_error(png_ptr, "invalid compression buffer size");

if PNG_SEQUENTIAL_READ_SUPPORTED eq 1
;   if ((png_ptr->mode & PNG_IS_READ_STRUCT) != 0)
;   {
;      png_ptr->IDAT_read_size = (uint_32)size; /* checked above */
;      return;
;   }
end if

if PNG_WRITE_SUPPORTED eq 1
;   if ((png_ptr->mode & PNG_IS_READ_STRUCT) == 0)
;   {
;      if (png_ptr->zowner != 0)
;      {
;         png_warning(png_ptr,
;             "Compression buffer size cannot be changed because it is in use");

;         return;
;      }

;#ifndef __COVERITY__
	; Some compilers complain that this is always false.  However, it
	; can be true when integer overflow happens.

;      if (size > ZLIB_IO_MAX)
;      {
;         png_warning(png_ptr,
;             "Compression buffer size limited to system maximum");
;         size = ZLIB_IO_MAX; /* must fit */
;      }
;end if

;      if (size < 6)
;      {
	; Deflate will potentially go into an infinite loop on a SYNC_FLUSH
	; if this is permitted.

;         png_warning(png_ptr,
;             "Compression buffer size cannot be reduced below 6");

;         return;
;      }

;      if (png_ptr->zbuffer_size != size)
;      {
;         png_free_buffer_list(png_ptr, &png_ptr->zbuffer_list);
;         png_ptr->zbuffer_size = (uInt)size;
;      }
;   }
end if
.end_f:
	ret
endp

;void (png_structrp png_ptr, png_inforp info_ptr, int mask)
align 4
proc png_set_invalid, png_ptr:dword, info_ptr:dword, mask:dword
;   if (png_ptr != NULL && info_ptr != NULL)
;      info_ptr->valid &= ~mask;
	ret
endp

; This function was added to libpng 1.2.6
;void (png_structrp png_ptr, uint_32 user_width_max, uint_32 user_height_max)
align 4
proc png_set_user_limits uses eax edi, png_ptr:dword, user_width_max:dword, user_height_max:dword
	; Images with dimensions larger than these limits will be
	; rejected by png_set_IHDR().  To accept any PNG datastream
	; regardless of dimensions, set both limits to 0x7fffffff.

	mov edi,[png_ptr]
	or edi,edi
	jz @f
		mov eax,[user_width_max]
		mov [edi+png_struct.user_width_max],eax
		mov eax,[user_height_max]
		mov [edi+png_struct.user_height_max],eax
	@@:
	ret
endp

; This function was added to libpng 1.4.0
;void (png_structrp png_ptr, uint_32 user_chunk_cache_max)
align 4
proc png_set_chunk_cache_max, png_ptr:dword, user_chunk_cache_max:dword
;   if (png_ptr != NULL)
;      png_ptr->user_chunk_cache_max = user_chunk_cache_max;
	ret
endp

; This function was added to libpng 1.4.1
;void (png_structrp png_ptr, png_alloc_size_t user_chunk_malloc_max)
align 4
proc png_set_chunk_malloc_max, png_ptr:dword, user_chunk_malloc_max:dword
;   if (png_ptr != NULL)
;      png_ptr->user_chunk_malloc_max = user_chunk_malloc_max;
	ret
endp

;void (png_structrp png_ptr, int allowed)
align 4
proc png_set_benign_errors uses edi, png_ptr:dword, allowed:dword
	png_debug 1, 'in png_set_benign_errors'

	; If allowed is 1, png_benign_error() is treated as a warning.
	; If allowed is 0, png_benign_error() is treated as an error (which
	; is the default behavior if png_set_benign_errors() is not called).

	mov edi,[png_ptr]
	cmp dword[allowed],0
	je @f ;if (..!=0)
		or dword[edi+png_struct.flags], PNG_FLAG_BENIGN_ERRORS_WARN or PNG_FLAG_APP_WARNINGS_WARN or PNG_FLAG_APP_ERRORS_WARN
		jmp .end0
	@@: ;else
		and dword[edi+png_struct.flags], not (PNG_FLAG_BENIGN_ERRORS_WARN or PNG_FLAG_APP_WARNINGS_WARN or PNG_FLAG_APP_ERRORS_WARN)
	.end0:
	ret
endp

; Whether to report invalid palette index; added at libng-1.5.10.
; It is possible for an indexed (color-type==3) PNG file to contain
; pixels with invalid (out-of-range) indexes if the PLTE chunk has
; fewer entries than the image's bit-depth would allow. We recover
; from this gracefully by filling any incomplete palette with zeros
; (opaque black).  By default, when this occurs libpng will issue
; a benign error.  This API can be used to override that behavior.

;void (png_structrp png_ptr, int allowed)
align 4
proc png_set_check_for_invalid_index, png_ptr:dword, allowed:dword
	png_debug 1, 'in png_set_check_for_invalid_index'

;   if (allowed > 0)
;      png_ptr->num_palette_max = 0;

;   else
;      png_ptr->num_palette_max = -1;
	ret
endp

; Check that the tEXt or zTXt keyword is valid per PNG 1.0 specification,
; and if invalid, correct the keyword rather than discarding the entire
; chunk.  The PNG 1.0 specification requires keywords 1-79 characters in
; length, forbids leading or trailing whitespace, multiple internal spaces,
; and the non-break space (0x80) from ISO 8859-1.  Returns keyword length.

; The 'new_key' buffer must be 80 characters in size (for the keyword plus a
; trailing '\0').  If this routine returns 0 then there was no keyword, or a
; valid one could not be generated, and the caller must png_error.

;uint_32 (png_structrp png_ptr, charp key, bytep new_key)
align 4
proc png_check_keyword, png_ptr:dword, key:dword, new_key:dword
;if PNG_WARNINGS_SUPPORTED
;   charp orig_key = key;
;end if
;   uint_32 key_len = 0;
;   int bad_character = 0;
;   int space = 1;

	png_debug 1, 'in png_check_keyword'

;   if (key == NULL)
;   {
;      *new_key = 0;
;      return 0;
;   }

;   while (*key && key_len < 79)
;   {
;      byte ch = (byte)*key++;

;      if ((ch > 32 && ch <= 126) || (ch >= 161 /*&& ch <= 255*/))
;         *new_key++ = ch, ++key_len, space = 0;

;      else if (space == 0)
;      {
	; A space or an invalid character when one wasn't seen immediately
	; before; output just a space.

;         *new_key++ = 32, ++key_len, space = 1;

;         /* If the character was not a space then it is invalid. */
;         if (ch != 32)
;            bad_character = ch;
;      }

;      else if (bad_character == 0)
;         bad_character = ch; /* just skip it, record the first error */
;   }

;   if (key_len > 0 && space != 0) /* trailing space */
;   {
;      --key_len, --new_key;
;      if (bad_character == 0)
;         bad_character = 32;
;   }

	; Terminate the keyword
;   *new_key = 0;

;   if (key_len == 0)
;      return 0;

if PNG_WARNINGS_SUPPORTED eq 1
	; Try to only output one warning per keyword:
;   if (*key != 0) /* keyword too long */
;      png_warning(png_ptr, "keyword truncated");

;   else if (bad_character != 0)
;   {
;      PNG_WARNING_PARAMETERS(p)

;      png_warning_parameter(p, 1, orig_key);
;      png_warning_parameter_signed(p, 2, PNG_NUMBER_FORMAT_02x, bad_character);

;      png_formatted_warning(png_ptr, p, "keyword \"@1\": bad character '0x@2'");
;   }
end if ;!WARNINGS

;   return key_len;
	ret
endp

