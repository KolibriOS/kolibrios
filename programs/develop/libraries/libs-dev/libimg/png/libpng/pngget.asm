
; pngget.asm - retrieval of values from info struct

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc


;uint_32 (png_structrp png_ptr, png_inforp info_ptr, uint_32 flag)
align 4
proc png_get_valid, png_ptr:dword, info_ptr:dword, flag:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		mov eax,[eax+png_info_def.valid]
		and eax,[flag]
	@@:
	ret
endp

;png_size_t (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_rowbytes, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		mov eax,[eax+png_info_def.rowbytes]
	@@:
	ret
endp

;bytepp (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_rows, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		mov eax,[eax+png_info_def.row_pointers]
	@@:
	ret
endp

; Easy access to info, added in libpng-0.99
;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_image_width, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		mov eax,[eax+png_info_def.width]
	@@:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_image_height, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		mov eax,[eax+png_info_def.height]
	@@:
	ret
endp

;byte (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_bit_depth, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		movzx eax,byte[eax+png_info_def.bit_depth]
	@@:
	ret
endp

;byte (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_color_type, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		movzx eax,byte[eax+png_info_def.color_type]
	@@:
	ret
endp

;byte (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_filter_type, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		movzx eax,byte[eax+png_info_def.filter_type]
	@@:
	ret
endp

;byte (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_interlace_type, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		movzx eax,byte[eax+png_info_def.interlace_type]
	@@:
	ret
endp

;byte (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_compression_type, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		mov eax,[eax+png_info_def.compression_type]
	@@:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_x_pixels_per_meter, png_ptr:dword, info_ptr:dword
if PNG_pHYs_SUPPORTED eq 1
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov esi,[info_ptr]
	cmp esi,0
	je @f
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_pHYs
	cmp eax,0
	je @f ;if (..!=0 && ..!=0 && ..!=0)
		png_debug1 1, 'in %s retrieval function', 'png_get_x_pixels_per_meter'

		cmp dword[esi+png_info_def.phys_unit_type],PNG_RESOLUTION_METER
		jne @f ;if (..==..)
			mov eax,[esi+png_info_def.x_pixels_per_unit]
			jmp .end_f
	@@:
end if
	xor eax,eax
.end_f:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_y_pixels_per_meter, png_ptr:dword, info_ptr:dword
if PNG_pHYs_SUPPORTED eq 1
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov esi,[info_ptr]
	cmp esi,0
	je @f
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_pHYs
	cmp eax,0
	je @f ;if (..!=0 && ..!=0 && ..!=0)
		png_debug1 1, 'in %s retrieval function', 'png_get_y_pixels_per_meter'

		cmp dword[esi+png_info_def.phys_unit_type],PNG_RESOLUTION_METER
		jne @f ;if (..==..)
			mov eax,[esi+png_info_def.y_pixels_per_unit]
			jmp .end_f
	@@:
end if
	xor eax,eax
.end_f:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_pixels_per_meter uses esi, png_ptr:dword, info_ptr:dword
if PNG_pHYs_SUPPORTED eq 1
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov esi,[info_ptr]
	cmp esi,0
	je @f
	mov eax,[esi+png_info_def.valid]
	and eax,PNG_INFO_pHYs
	cmp eax,0
	je @f ;if (..!=0 && ..!=0 && ..!=0)
		png_debug1 1, 'in %s retrieval function', 'png_get_pixels_per_meter'

		cmp dword[esi+png_info_def.phys_unit_type],PNG_RESOLUTION_METER
		jne @f
		mov eax,[esi+png_info_def.x_pixels_per_unit]
		cmp eax,[esi+png_info_def.y_pixels_per_unit]
		jne @f ;if (..==.. && ..==..)
			jmp .end_f
	@@:
end if
	xor eax,eax
.end_f:
	ret
endp

;float (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_pixel_aspect_ratio, png_ptr:dword, info_ptr:dword
if PNG_READ_pHYs_SUPPORTED eq 1
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_pHYs) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'png_get_aspect_ratio'

;      if (info_ptr->x_pixels_per_unit != 0)
;         return ((float)((float)info_ptr->y_pixels_per_unit
;             /(float)info_ptr->x_pixels_per_unit));
;   }
end if

;   return ((float)0.0);
	ret
endp

;png_fixed_point (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_pixel_aspect_ratio_fixed, png_ptr:dword, info_ptr:dword
if PNG_READ_pHYs_SUPPORTED eq 1
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_pHYs) != 0 &&
;       info_ptr->x_pixels_per_unit > 0 && info_ptr->y_pixels_per_unit > 0 &&
;       info_ptr->x_pixels_per_unit <= PNG_UINT_31_MAX &&
;       info_ptr->y_pixels_per_unit <= PNG_UINT_31_MAX)
;   {
;      png_fixed_point res;

		png_debug1 1, 'in %s retrieval function', 'png_get_aspect_ratio_fixed'

	; The following casts work because a PNG 4 byte integer only has a valid
	; range of 0..2^31-1; otherwise the cast might overflow.

;      if (png_muldiv(&res, (int_32)info_ptr->y_pixels_per_unit, PNG_FP_1,
;          (int_32)info_ptr->x_pixels_per_unit) != 0)
;         return res;
;   }
end if

;   return 0;
	ret
endp

;int_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_x_offset_microns, png_ptr:dword, info_ptr:dword
if PNG_oFFs_SUPPORTED eq 1
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_oFFs) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'png_get_x_offset_microns'

;      if (info_ptr->offset_unit_type == PNG_OFFSET_MICROMETER)
;         return (info_ptr->x_offset);
;   }
end if

	xor eax,eax
	ret
endp

;int_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_y_offset_microns, png_ptr:dword, info_ptr:dword
if PNG_oFFs_SUPPORTED eq 1
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_oFFs) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'png_get_y_offset_microns'

;      if (info_ptr->offset_unit_type == PNG_OFFSET_MICROMETER)
;         return (info_ptr->y_offset);
;   }
end if

	xor eax,eax
	ret
endp

;int_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_x_offset_pixels, png_ptr:dword, info_ptr:dword
if PNG_oFFs_SUPPORTED eq 1
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_oFFs) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'png_get_x_offset_pixels'

;      if (info_ptr->offset_unit_type == PNG_OFFSET_PIXEL)
;         return (info_ptr->x_offset);
;   }
end if

	xor eax,eax
	ret
endp

;int_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_y_offset_pixels, png_ptr:dword, info_ptr:dword
if PNG_oFFs_SUPPORTED eq 1
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_oFFs) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'png_get_y_offset_pixels'

;      if (info_ptr->offset_unit_type == PNG_OFFSET_PIXEL)
;         return (info_ptr->y_offset);
;   }
end if

	xor eax,eax
	ret
endp

;uint_32 (uint_32 ppm)
align 4
proc ppi_from_ppm, ppm:dword
;#if 0
	; The conversion is *(2.54/100), in binary (32 digits):
	; .00000110100000001001110101001001

;   uint_32 t1001, t1101;
;   ppm >>= 1;                  /* .1 */
;   t1001 = ppm + (ppm >> 3);   /* .1001 */
;   t1101 = t1001 + (ppm >> 1); /* .1101 */
;   ppm >>= 20;                 /* .000000000000000000001 */
;   t1101 += t1101 >> 15;       /* .1101000000000001101 */
;   t1001 >>= 11;               /* .000000000001001 */
;   t1001 += t1001 >> 12;       /* .000000000001001000000001001 */
;   ppm += t1001;               /* .000000000001001000001001001 */
;   ppm += t1101;               /* .110100000001001110101001001 */
;   return (ppm + 16) >> 5;/* .00000110100000001001110101001001 */
;#else
	; The argument is a PNG unsigned integer, so it is not permitted
	; to be bigger than 2^31.

;   png_fixed_point result;
;   if (ppm <= PNG_UINT_31_MAX && png_muldiv(&result, (int_32)ppm, 127,
;       5000) != 0)
;      return result;

	; Overflow.
;   return 0;
;end if
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_pixels_per_inch, png_ptr:dword, info_ptr:dword
	stdcall png_get_pixels_per_meter, [png_ptr], [info_ptr]
	stdcall ppi_from_ppm, eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_x_pixels_per_inch, png_ptr:dword, info_ptr:dword
	stdcall png_get_x_pixels_per_meter, [png_ptr], [info_ptr]
	stdcall ppi_from_ppm, eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_y_pixels_per_inch, png_ptr:dword, info_ptr:dword
	stdcall png_get_y_pixels_per_meter, [png_ptr], [info_ptr]
	stdcall ppi_from_ppm, eax
	ret
endp

;png_fixed_point (png_structrp png_ptr, int_32 microns)
align 4
proc png_fixed_inches_from_microns, png_ptr:dword, microns:dword
	; Convert from metres * 1,000,000 to inches * 100,000, meters to
	; inches is simply *(100/2.54), so we want *(10/2.54) == 500/127.
	; Notice that this can overflow - a warning is output and 0 is
	; returned.

	stdcall png_muldiv_warn, [png_ptr], [microns], 500, 127
	ret
endp

;png_fixed_point (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_x_offset_inches_fixed, png_ptr:dword, info_ptr:dword
	stdcall png_get_x_offset_microns, [png_ptr], [info_ptr]
	stdcall png_fixed_inches_from_microns, [png_ptr], eax
	ret
endp

;png_fixed_point (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_y_offset_inches_fixed, png_ptr:dword, info_ptr:dword
	stdcall png_get_y_offset_microns, [png_ptr], [info_ptr]
	stdcall png_fixed_inches_from_microns, [png_ptr], eax
	ret
endp

;float (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_x_offset_inches, png_ptr:dword, info_ptr:dword
	; To avoid the overflow do the conversion directly in floating
	; point.

;   return (float)(png_get_x_offset_microns(png_ptr, info_ptr) * .00003937);
	ret
endp

;float (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_y_offset_inches, png_ptr:dword, info_ptr:dword
	; To avoid the overflow do the conversion directly in floating
	; point.

;   return (float)(png_get_y_offset_microns(png_ptr, info_ptr) * .00003937);
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    uint_32 *res_x, uint_32 *res_y, int *unit_type)
align 4
proc png_get_pHYs_dpi, png_ptr:dword, info_ptr:dword, res_x:dword, res_y:dword, unit_type:dword
;   uint_32 retval = 0;

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_pHYs) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'pHYs'

;      if (res_x != NULL)
;      {
;         *res_x = info_ptr->x_pixels_per_unit;
;         retval |= PNG_INFO_pHYs;
;      }

;      if (res_y != NULL)
;      {
;         *res_y = info_ptr->y_pixels_per_unit;
;         retval |= PNG_INFO_pHYs;
;      }

;      if (unit_type != NULL)
;      {
;         *unit_type = (int)info_ptr->phys_unit_type;
;         retval |= PNG_INFO_pHYs;

;         if (*unit_type == 1)
;         {
;            if (res_x != NULL) *res_x = (uint_32)(*res_x * .0254 + .50);
;            if (res_y != NULL) *res_y = (uint_32)(*res_y * .0254 + .50);
;         }
;      }
;   }

;   return (retval);
	ret
endp

; png_get_channels really belongs in here, too, but it's been around longer

;byte (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_channels, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		movzx eax,byte[eax+png_info_def.channels]
	@@:
	ret
endp

;bytep (png_structrp png_ptr, png_inforp info_ptr)
align 4
proc png_get_signature, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	mov eax,[info_ptr]
	cmp eax,0
	je @f ;if (..!=0 || ..!=0)
		movzx eax,byte[eax+png_info_def.signature]
	@@:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_color_16p *background)
align 4
proc png_get_bKGD, png_ptr:dword, info_ptr:dword, background:dword
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_bKGD) != 0 &&
;       background != NULL)
;   {
		png_debug1 1, 'in %s retrieval function', 'bKGD'

;      *background = &(info_ptr->background);
;      return (PNG_INFO_bKGD);
;   }

	xor eax,eax
	ret
endp

;if PNG_cHRM_SUPPORTED
; The XYZ APIs were added in 1.5.5 to take advantage of the code added at the
; same time to correct the rgb grayscale coefficient defaults obtained from the
; cHRM chunk in 1.5.4

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    double *white_x, double *white_y, double *red_x, double *red_y,
;    double *green_x, double *green_y, double *blue_x, double *blue_y)
align 4
proc png_get_cHRM, png_ptr:dword, info_ptr:dword, white_x:dword, white_y:dword, red_x:dword, red_y:dword, green_x:dword, green_y:dword, blue_x:dword, blue_y:dword
	; Quiet API change: this code used to only return the end points if a cHRM
	; chunk was present, but the end points can also come from iCCP or sRGB
	; chunks, so in 1.6.0 the png_get_ APIs return the end points regardless and
	; the png_set_ APIs merely check that set end points are mutually
	; consistent.

;   if (png_ptr != NULL && info_ptr != NULL &&
;      (info_ptr->colorspace.flags & PNG_COLORSPACE_HAVE_ENDPOINTS) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'cHRM'

;      if (white_x != NULL)
;         *white_x = png_float(png_ptr,
;             info_ptr->colorspace.end_points_xy.whitex, "cHRM white X");
;      if (white_y != NULL)
;         *white_y = png_float(png_ptr,
;             info_ptr->colorspace.end_points_xy.whitey, "cHRM white Y");
;      if (red_x != NULL)
;         *red_x = png_float(png_ptr, info_ptr->colorspace.end_points_xy.redx,
;             "cHRM red X");
;      if (red_y != NULL)
;         *red_y = png_float(png_ptr, info_ptr->colorspace.end_points_xy.redy,
;             "cHRM red Y");
;      if (green_x != NULL)
;         *green_x = png_float(png_ptr,
;             info_ptr->colorspace.end_points_xy.greenx, "cHRM green X");
;      if (green_y != NULL)
;         *green_y = png_float(png_ptr,
;             info_ptr->colorspace.end_points_xy.greeny, "cHRM green Y");
;      if (blue_x != NULL)
;         *blue_x = png_float(png_ptr, info_ptr->colorspace.end_points_xy.bluex,
;             "cHRM blue X");
;      if (blue_y != NULL)
;         *blue_y = png_float(png_ptr, info_ptr->colorspace.end_points_xy.bluey,
;             "cHRM blue Y");
;      return (PNG_INFO_cHRM);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    double *red_X, double *red_Y, double *red_Z, double *green_X,
;    double *green_Y, double *green_Z, double *blue_X, double *blue_Y,
;    double *blue_Z)
align 4
proc png_get_cHRM_XYZ, png_ptr:dword, info_ptr:dword, red_X:dword, red_Y:dword, red_Z:dword, green_X:dword, green_Y:dword, green_Z:dword, blue_X:dword, blue_Y:dword, blue_Z:dword
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->colorspace.flags & PNG_COLORSPACE_HAVE_ENDPOINTS) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'cHRM_XYZ(float)'

;      if (red_X != NULL)
;         *red_X = png_float(png_ptr, info_ptr->colorspace.end_points_XYZ.red_X,
;             "cHRM red X");
;      if (red_Y != NULL)
;         *red_Y = png_float(png_ptr, info_ptr->colorspace.end_points_XYZ.red_Y,
;             "cHRM red Y");
;      if (red_Z != NULL)
;         *red_Z = png_float(png_ptr, info_ptr->colorspace.end_points_XYZ.red_Z,
;             "cHRM red Z");
;      if (green_X != NULL)
;         *green_X = png_float(png_ptr,
;             info_ptr->colorspace.end_points_XYZ.green_X, "cHRM green X");
;      if (green_Y != NULL)
;         *green_Y = png_float(png_ptr,
;             info_ptr->colorspace.end_points_XYZ.green_Y, "cHRM green Y");
;      if (green_Z != NULL)
;         *green_Z = png_float(png_ptr,
;             info_ptr->colorspace.end_points_XYZ.green_Z, "cHRM green Z");
;      if (blue_X != NULL)
;         *blue_X = png_float(png_ptr,
;             info_ptr->colorspace.end_points_XYZ.blue_X, "cHRM blue X");
;      if (blue_Y != NULL)
;         *blue_Y = png_float(png_ptr,
;             info_ptr->colorspace.end_points_XYZ.blue_Y, "cHRM blue Y");
;      if (blue_Z != NULL)
;         *blue_Z = png_float(png_ptr,
;             info_ptr->colorspace.end_points_XYZ.blue_Z, "cHRM blue Z");
;      return (PNG_INFO_cHRM);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_fixed_point *int_red_X, png_fixed_point *int_red_Y,
;    png_fixed_point *int_red_Z, png_fixed_point *int_green_X,
;    png_fixed_point *int_green_Y, png_fixed_point *int_green_Z,
;    png_fixed_point *int_blue_X, png_fixed_point *int_blue_Y,
;    png_fixed_point *int_blue_Z)
align 4
proc png_get_cHRM_XYZ_fixed, png_ptr:dword, info_ptr:dword, int_red_X:dword, int_red_Y:dword, int_red_Z:dword, int_green_X:dword, int_green_Y:dword, int_green_Z:dword, int_blue_X:dword, int_blue_Y:dword, int_blue_Z:dword
;   if (png_ptr != NULL && info_ptr != NULL &&
;      (info_ptr->colorspace.flags & PNG_COLORSPACE_HAVE_ENDPOINTS) != 0)
;   {
	png_debug1 1, 'in %s retrieval function', 'cHRM_XYZ'

;      if (int_red_X != NULL)
;         *int_red_X = info_ptr->colorspace.end_points_XYZ.red_X;
;      if (int_red_Y != NULL)
;         *int_red_Y = info_ptr->colorspace.end_points_XYZ.red_Y;
;      if (int_red_Z != NULL)
;         *int_red_Z = info_ptr->colorspace.end_points_XYZ.red_Z;
;      if (int_green_X != NULL)
;         *int_green_X = info_ptr->colorspace.end_points_XYZ.green_X;
;      if (int_green_Y != NULL)
;         *int_green_Y = info_ptr->colorspace.end_points_XYZ.green_Y;
;      if (int_green_Z != NULL)
;         *int_green_Z = info_ptr->colorspace.end_points_XYZ.green_Z;
;      if (int_blue_X != NULL)
;         *int_blue_X = info_ptr->colorspace.end_points_XYZ.blue_X;
;      if (int_blue_Y != NULL)
;         *int_blue_Y = info_ptr->colorspace.end_points_XYZ.blue_Y;
;      if (int_blue_Z != NULL)
;         *int_blue_Z = info_ptr->colorspace.end_points_XYZ.blue_Z;
;      return (PNG_INFO_cHRM);
;   }

	xor eax,eax
.end_f:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_fixed_point *white_x, png_fixed_point *white_y, png_fixed_point *red_x,
;    png_fixed_point *red_y, png_fixed_point *green_x, png_fixed_point *green_y,
;    png_fixed_point *blue_x, png_fixed_point *blue_y)
align 4
proc png_get_cHRM_fixed, png_ptr:dword, info_ptr:dword, white_x:dword, white_y:dword, red_x:dword, red_y:dword, green_x:dword, green_y:dword, blue_x:dword, blue_y:dword
	png_debug1 1, 'in %s retrieval function', 'cHRM'

;   if (png_ptr != NULL && info_ptr != NULL &&
;      (info_ptr->colorspace.flags & PNG_COLORSPACE_HAVE_ENDPOINTS) != 0)
;   {
;      if (white_x != NULL)
;         *white_x = info_ptr->colorspace.end_points_xy.whitex;
;      if (white_y != NULL)
;         *white_y = info_ptr->colorspace.end_points_xy.whitey;
;      if (red_x != NULL)
;         *red_x = info_ptr->colorspace.end_points_xy.redx;
;      if (red_y != NULL)
;         *red_y = info_ptr->colorspace.end_points_xy.redy;
;      if (green_x != NULL)
;         *green_x = info_ptr->colorspace.end_points_xy.greenx;
;      if (green_y != NULL)
;         *green_y = info_ptr->colorspace.end_points_xy.greeny;
;      if (blue_x != NULL)
;         *blue_x = info_ptr->colorspace.end_points_xy.bluex;
;      if (blue_y != NULL)
;         *blue_y = info_ptr->colorspace.end_points_xy.bluey;
;      return (PNG_INFO_cHRM);
;   }

	xor eax,eax
	ret
endp
;end if

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_fixed_point *file_gamma)
align 4
proc png_get_gAMA_fixed, png_ptr:dword, info_ptr:dword, file_gamma:dword
	png_debug1 1, 'in %s retrieval function', 'gAMA'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->colorspace.flags & PNG_COLORSPACE_HAVE_GAMMA) != 0 &&
;       file_gamma != NULL)
;   {
;      *file_gamma = info_ptr->colorspace.gamma;
;      return (PNG_INFO_gAMA);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr, double *file_gamma)
align 4
proc png_get_gAMA, png_ptr:dword, info_ptr:dword, file_gamma:dword
	png_debug1 1, 'in %s retrieval function', 'gAMA(float)'

;   if (png_ptr != NULL && info_ptr != NULL &&
;      (info_ptr->colorspace.flags & PNG_COLORSPACE_HAVE_GAMMA) != 0 &&
;      file_gamma != NULL)
;   {
;      *file_gamma = png_float(png_ptr, info_ptr->colorspace.gamma,
;          "png_get_gAMA");
;      return (PNG_INFO_gAMA);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr, int *file_srgb_intent)
align 4
proc png_get_sRGB, png_ptr:dword, info_ptr:dword, file_srgb_intent:dword
	png_debug1 1, 'in %s retrieval function', 'sRGB'

;   if (png_ptr != NULL && info_ptr != NULL &&
;      (info_ptr->valid & PNG_INFO_sRGB) != 0 && file_srgb_intent != NULL)
;   {
;      *file_srgb_intent = info_ptr->colorspace.rendering_intent;
;      return (PNG_INFO_sRGB);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_charpp name, int *compression_type, bytepp profile, uint_32 *proflen)
align 4
proc png_get_iCCP, png_ptr:dword, info_ptr:dword, name:dword, compression_type:dword, profile:dword, proflen:dword
	png_debug1 1, 'in %s retrieval function', 'iCCP'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_iCCP) != 0 &&
;       name != NULL && compression_type != NULL && profile != NULL &&
;           proflen != NULL)
;   {
;      *name = info_ptr->iccp_name;
;      *profile = info_ptr->iccp_profile;
;      *proflen = png_get_uint_32(info_ptr->iccp_profile);
	; This is somewhat irrelevant since the profile data returned has
	; actually been uncompressed.

;      *compression_type = PNG_COMPRESSION_TYPE_BASE;
;      return (PNG_INFO_iCCP);
;   }

	xor eax,eax
	ret
endp

;int (png_structrp png_ptr, png_inforp info_ptr,
;    png_sPLT_tpp spalettes)
align 4
proc png_get_sPLT, png_ptr:dword, info_ptr:dword, spalettes:dword
;   if (png_ptr != NULL && info_ptr != NULL && spalettes != NULL)
;   {
;      *spalettes = info_ptr->splt_palettes;
;      return info_ptr->splt_palettes_num;
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    uint_16p hist)
align 4
proc png_get_hIST, png_ptr:dword, info_ptr:dword, hist:dword
	png_debug1 1, 'in %s retrieval function', 'hIST'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_hIST) != 0 && hist != NULL)
;   {
;      *hist = info_ptr->hist;
;      return (PNG_INFO_hIST);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    uint_32 *width, uint_32 *height, int *bit_depth,
;    int *color_type, int *interlace_type, int *compression_type,
;    int *filter_type)
align 4
proc png_get_IHDR, png_ptr:dword, info_ptr:dword,\
	width:dword, height:dword, bit_depth:dword, color_type:dword,\
	interlace_type:dword, compression_type:dword, filter_type:dword
	png_debug1 1, 'in %s retrieval function', 'IHDR'

;   if (png_ptr == NULL || info_ptr == NULL)
;      return (0);

;   if (width != NULL)
;       *width = info_ptr->width;

;   if (height != NULL)
;       *height = info_ptr->height;

;   if (bit_depth != NULL)
;       *bit_depth = info_ptr->bit_depth;

;   if (color_type != NULL)
;       *color_type = info_ptr->color_type;

;   if (compression_type != NULL)
;      *compression_type = info_ptr->compression_type;

;   if (filter_type != NULL)
;      *filter_type = info_ptr->filter_type;

;   if (interlace_type != NULL)
;      *interlace_type = info_ptr->interlace_type;

	; This is redundant if we can be sure that the info_ptr values were all
	; assigned in png_set_IHDR().  We do the check anyhow in case an
	; application has ignored our advice not to mess with the members
	; of info_ptr directly.

;   png_check_IHDR(png_ptr, info_ptr->width, info_ptr->height,
;       info_ptr->bit_depth, info_ptr->color_type, info_ptr->interlace_type,
;       info_ptr->compression_type, info_ptr->filter_type);

	xor eax,eax
	inc eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    int_32 *offset_x, int_32 *offset_y, int *unit_type)
align 4
proc png_get_oFFs, png_ptr:dword, info_ptr:dword, offset_x:dword, offset_y:dword, unit_type:dword
	png_debug1 1, 'in %s retrieval function', 'oFFs'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_oFFs) != 0 &&
;       offset_x != NULL && offset_y != NULL && unit_type != NULL)
;   {
;      *offset_x = info_ptr->x_offset;
;      *offset_y = info_ptr->y_offset;
;      *unit_type = (int)info_ptr->offset_unit_type;
;      return (PNG_INFO_oFFs);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    charp *purpose, int_32 *X0, int_32 *X1, int *type, int *nparams,
;    charp *units, charpp *params)
align 4
proc png_get_pCAL, png_ptr:dword, info_ptr:dword, purpose:dword, X0:dword, X1:dword, type:dword, nparams:dword, units:dword, params:dword
	png_debug1 1, 'in %s retrieval function', 'pCAL'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_pCAL) != 0 &&
;       purpose != NULL && X0 != NULL && X1 != NULL && type != NULL &&
;       nparams != NULL && units != NULL && params != NULL)
;   {
;      *purpose = info_ptr->pcal_purpose;
;      *X0 = info_ptr->pcal_X0;
;      *X1 = info_ptr->pcal_X1;
;      *type = (int)info_ptr->pcal_type;
;      *nparams = (int)info_ptr->pcal_nparams;
;      *units = info_ptr->pcal_units;
;      *params = info_ptr->pcal_params;
;      return (PNG_INFO_pCAL);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    int *unit, png_fixed_point *width, png_fixed_point *height)
align 4
proc png_get_sCAL_fixed, png_ptr:dword, info_ptr:dword, unit:dword, width:dword, height:dword
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_sCAL) != 0)
;   {
;      *unit = info_ptr->scal_unit;
	;TODO: make this work without FP support; the API is currently eliminated
	; if neither floating point APIs nor internal floating point arithmetic
	; are enabled.

;      *width = png_fixed(png_ptr, atof(info_ptr->scal_s_width), "sCAL width");
;      *height = png_fixed(png_ptr, atof(info_ptr->scal_s_height),
;          "sCAL height");
;      return (PNG_INFO_sCAL);
;   }

;   return(0);
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    int *unit, double *width, double *height)
align 4
proc png_get_sCAL, png_ptr:dword, info_ptr:dword, unit:dword, width:dword, height:dword
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_sCAL) != 0)
;   {
;      *unit = info_ptr->scal_unit;
;      *width = atof(info_ptr->scal_s_width);
;      *height = atof(info_ptr->scal_s_height);
;      return (PNG_INFO_sCAL);
;   }

;   return(0);
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    int *unit, charpp width, charpp height)
align 4
proc png_get_sCAL_s, png_ptr:dword, info_ptr:dword, unit:dword, width:dword, height:dword
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_sCAL) != 0)
;   {
;      *unit = info_ptr->scal_unit;
;      *width = info_ptr->scal_s_width;
;      *height = info_ptr->scal_s_height;
;      return (PNG_INFO_sCAL);
;   }

	xor eax,eax
.end_f:
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    uint_32 *res_x, uint_32 *res_y, int *unit_type)
align 4
proc png_get_pHYs, png_ptr:dword, info_ptr:dword, res_x:dword, res_y:dword, unit_type:dword
;   uint_32 retval = 0;

	png_debug1 1, 'in %s retrieval function', 'pHYs'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_pHYs) != 0)
;   {
;      if (res_x != NULL)
;      {
;         *res_x = info_ptr->x_pixels_per_unit;
;         retval |= PNG_INFO_pHYs;
;      }

;      if (res_y != NULL)
;      {
;         *res_y = info_ptr->y_pixels_per_unit;
;         retval |= PNG_INFO_pHYs;
;      }

;      if (unit_type != NULL)
;      {
;         *unit_type = (int)info_ptr->phys_unit_type;
;         retval |= PNG_INFO_pHYs;
;      }
;   }

;   return (retval);
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_colorp *palette, int *num_palette)
align 4
proc png_get_PLTE, png_ptr:dword, info_ptr:dword, palette:dword, num_palette:dword
	png_debug1 1, 'in %s retrieval function', 'PLTE'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_PLTE) != 0 && palette != NULL)
;   {
;      *palette = info_ptr->palette;
;      *num_palette = info_ptr->num_palette;
;      png_debug1(3, "num_palette = %d", *num_palette);
;      return (PNG_INFO_PLTE);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr, png_color_8p *sig_bit)
align 4
proc png_get_sBIT, png_ptr:dword, info_ptr:dword, sig_bit:dword
	png_debug1 1, 'in %s retrieval function', 'sBIT'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_sBIT) != 0 && sig_bit != NULL)
;   {
;      *sig_bit = &(info_ptr->sig_bit);
;      return (PNG_INFO_sBIT);
;   }

	xor eax,eax
	ret
endp

;int (png_structrp png_ptr, png_inforp info_ptr, png_textp *text_ptr, int *num_text)
align 4
proc png_get_text, png_ptr:dword, info_ptr:dword, text_ptr:dword, num_text:dword
;   if (png_ptr != NULL && info_ptr != NULL && info_ptr->num_text > 0)
;   {
;      png_debug1(1, "in 0x%lx retrieval function",
;         (unsigned long)png_ptr->chunk_name);

;      if (text_ptr != NULL)
;         *text_ptr = info_ptr->text;

;      if (num_text != NULL)
;         *num_text = info_ptr->num_text;

;      return info_ptr->num_text;
;   }

;   if (num_text != NULL)
;      *num_text = 0;

;   return(0);
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    png_timep *mod_time)
align 4
proc png_get_tIME, png_ptr:dword, info_ptr:dword, mod_time:dword
	png_debug1 1, 'in %s retrieval function', 'tIME'

;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_tIME) != 0 && mod_time != NULL)
;   {
;      *mod_time = &(info_ptr->mod_time);
;      return (PNG_INFO_tIME);
;   }

	xor eax,eax
	ret
endp

;uint_32 (png_structrp png_ptr, png_inforp info_ptr,
;    bytep *trans_alpha, int *num_trans, png_color_16p *trans_color)
align 4
proc png_get_tRNS, png_ptr:dword, info_ptr:dword, trans_alpha:dword, num_trans:dword, trans_color:dword
;   uint_32 retval = 0;
;   if (png_ptr != NULL && info_ptr != NULL &&
;       (info_ptr->valid & PNG_INFO_tRNS) != 0)
;   {
		png_debug1 1, 'in %s retrieval function', 'tRNS'

;      if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
;      {
;         if (trans_alpha != NULL)
;         {
;            *trans_alpha = info_ptr->trans_alpha;
;            retval |= PNG_INFO_tRNS;
;         }

;         if (trans_color != NULL)
;            *trans_color = &(info_ptr->trans_color);
;      }

;      else /* if (info_ptr->color_type != PNG_COLOR_TYPE_PALETTE) */
;      {
;         if (trans_color != NULL)
;         {
;            *trans_color = &(info_ptr->trans_color);
;            retval |= PNG_INFO_tRNS;
;         }

;         if (trans_alpha != NULL)
;            *trans_alpha = NULL;
;      }

;      if (num_trans != NULL)
;      {
;         *num_trans = info_ptr->num_trans;
;         retval |= PNG_INFO_tRNS;
;      }
;   }

;   return (retval);
	ret
endp

;int (png_structrp png_ptr, png_inforp info_ptr,
;    png_unknown_chunkpp unknowns)
align 4
proc png_get_unknown_chunks, png_ptr:dword, info_ptr:dword, unknowns:dword
;   if (png_ptr != NULL && info_ptr != NULL && unknowns != NULL)
;   {
;      *unknowns = info_ptr->unknown_chunks;
;      return info_ptr->unknown_chunks_num;
;   }

	xor eax,eax
	ret
endp

;byte (png_structrp png_ptr)
align 4
proc png_get_rgb_to_gray_status, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.rgb_to_gray_status]
	@@:
	ret
endp

;voidp (png_structrp png_ptr)
align 4
proc png_get_user_chunk_ptr, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.user_chunk_ptr]
	@@:
	ret
endp

;png_size_t (png_structrp png_ptr)
align 4
proc png_get_compression_buffer_size uses ebx, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je .end_f ;if (..==0) return 0

if PNG_WRITE_SUPPORTED eq 1
	mov ebx,[eax+png_struct.mode]
	and ebx,PNG_IS_READ_STRUCT
	cmp ebx,0
;   if (..!=0)
end if
;   {
if PNG_SEQUENTIAL_READ_SUPPORTED eq 1
		mov eax,[eax+png_struct.IDAT_read_size]
else
		mov eax,PNG_IDAT_READ_SIZE
end if
;   }

if PNG_WRITE_SUPPORTED eq 1
		jmp .end_f
;   else
		mov eax,[eax+png_struct.zbuffer_size]
end if
.end_f:
	ret
endp

; These functions were added to libpng 1.2.6 and were enabled
; by default in libpng-1.4.0
;uint_32 (png_structrp png_ptr)
align 4
proc png_get_user_width_max, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.user_width_max]
	@@:
	ret
endp

;uint_32 (png_structrp png_ptr)
align 4
proc png_get_user_height_max, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.user_height_max]
	@@:
	ret
endp

; This function was added to libpng 1.4.0
;uint_32 (png_structrp png_ptr)
align 4
proc png_get_chunk_cache_max, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.user_chunk_cache_max]
	@@:
	ret
endp

; This function was added to libpng 1.4.1
;png_alloc_size_t (png_structrp png_ptr)
align 4
proc png_get_chunk_malloc_max, png_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
		mov eax,[eax+png_struct.user_chunk_malloc_max]
	@@:
	ret
endp

; These functions were added to libpng 1.4.0
;uint_32 (png_structrp png_ptr)
align 4
proc png_get_io_state, png_ptr:dword
	mov eax,[png_ptr]
	mov eax,[eax+png_struct.io_state]
	ret
endp

;uint_32 (png_structrp png_ptr)
align 4
proc png_get_io_chunk_type, png_ptr:dword
	mov eax,[png_ptr]
	mov eax,[eax+png_struct.chunk_name]
	ret
endp

;int (png_const_structp png_ptr, png_const_infop info_ptr)
align 4
proc png_get_palette_max, png_ptr:dword, info_ptr:dword
	mov eax,[png_ptr]
	cmp eax,0
	je @f
	cmp dword[info_ptr],0
	je @f ;if (..!=0 && ..!=0)
		mov eax,[eax+png_struct.num_palette_max]
		jmp .end_f
	@@:
	xor eax,eax
	dec eax
.end_f:
	ret
endp

