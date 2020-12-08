
; pngwtran.asm - transforms the data in a row for PNG writers

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc


; Pack pixels into bytes.  Pass the true bit depth in bit_depth.  The
; row_info bit depth should be 8 (one pixel per byte).  The channels
; should be 1 (this only happens on grayscale and paletted images).

;void (png_row_infop row_info, bytep row, uint_32 bit_depth)
align 4
proc png_do_pack, row_info:dword, row:dword, bit_depth:dword
	png_debug 1, 'in png_do_pack'

;   if (row_info->bit_depth == 8 &&
;      row_info->channels == 1)
;   {
;      switch ((int)bit_depth)
;      {
;         case 1:
;         {
;            bytep sp, dp;
;            int mask, v;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            sp = row;
;            dp = row;
;            mask = 0x80;
;            v = 0;

;            for (i = 0; i < row_width; i++)
;            {
;               if (*sp != 0)
;                  v |= mask;

;               sp++;

;               if (mask > 1)
;                  mask >>= 1;

;               else
;               {
;                  mask = 0x80;
;                  *dp = (byte)v;
;                  dp++;
;                  v = 0;
;               }
;            }

;            if (mask != 0x80)
;               *dp = (byte)v;

;            break;
;         }

;         case 2:
;         {
;            bytep sp, dp;
;            unsigned int shift;
;            int v;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            sp = row;
;            dp = row;
;            shift = 6;
;            v = 0;

;            for (i = 0; i < row_width; i++)
;            {
;               byte value;

;               value = (byte)(*sp & 0x03);
;               v |= (value << shift);

;               if (shift == 0)
;               {
;                  shift = 6;
;                  *dp = (byte)v;
;                  dp++;
;                  v = 0;
;               }

;               else
;                  shift -= 2;
;
;               sp++;
;            }

;            if (shift != 6)
;               *dp = (byte)v;

;            break;
;         }

;         case 4:
;         {
;            bytep sp, dp;
;            unsigned int shift;
;            int v;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            sp = row;
;            dp = row;
;            shift = 4;
;            v = 0;

;            for (i = 0; i < row_width; i++)
;            {
;               byte value;

;               value = (byte)(*sp & 0x0f);
;               v |= (value << shift);

;               if (shift == 0)
;               {
;                  shift = 4;
;                  *dp = (byte)v;
;                  dp++;
;                  v = 0;
;               }

;               else
;                  shift -= 4;
;
;               sp++;
;            }

;            if (shift != 4)
;               *dp = (byte)v;

;            break;
;         }

;         default:
;            break;
;      }

;      row_info->bit_depth = (byte)bit_depth;
;      row_info->pixel_depth = (byte)(bit_depth * row_info->channels);
;      row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth,
;          row_info->width);
;   }
	ret
endp

; Shift pixel values to take advantage of whole range.  Pass the
; true number of bits in bit_depth.  The row should be packed
; according to row_info->bit_depth.  Thus, if you had a row of
; bit depth 4, but the pixels only had values from 0 to 7, you
; would pass 3 as bit_depth, and this routine would translate the
; data to 0 to 15.

;void (png_row_infop row_info, bytep row, png_const_color_8p bit_depth)
align 4
proc png_do_shift, row_info:dword, row:dword, bit_depth:dword
	png_debug 1, 'in png_do_shift'

;   if (row_info->color_type != PNG_COLOR_TYPE_PALETTE)
;   {
;      int shift_start[4], shift_dec[4];
;      int channels = 0;

;      if ((row_info->color_type & PNG_COLOR_MASK_COLOR) != 0)
;      {
;         shift_start[channels] = row_info->bit_depth - bit_depth->red;
;         shift_dec[channels] = bit_depth->red;
;         channels++;

;         shift_start[channels] = row_info->bit_depth - bit_depth->green;
;         shift_dec[channels] = bit_depth->green;
;         channels++;

;         shift_start[channels] = row_info->bit_depth - bit_depth->blue;
;         shift_dec[channels] = bit_depth->blue;
;         channels++;
;      }

;      else
;      {
;         shift_start[channels] = row_info->bit_depth - bit_depth->gray;
;         shift_dec[channels] = bit_depth->gray;
;         channels++;
;      }

;      if ((row_info->color_type & PNG_COLOR_MASK_ALPHA) != 0)
;      {
;         shift_start[channels] = row_info->bit_depth - bit_depth->alpha;
;         shift_dec[channels] = bit_depth->alpha;
;         channels++;
;      }

	; With low row depths, could only be grayscale, so one channel
;      if (row_info->bit_depth < 8)
;      {
;         bytep bp = row;
;         png_size_t i;
;         unsigned int mask;
;         png_size_t row_bytes = row_info->rowbytes;

;         if (bit_depth->gray == 1 && row_info->bit_depth == 2)
;            mask = 0x55;

;         else if (row_info->bit_depth == 4 && bit_depth->gray == 3)
;            mask = 0x11;

;         else
;            mask = 0xff;

;         for (i = 0; i < row_bytes; i++, bp++)
;         {
;            int j;
;            unsigned int v, out;

;            v = *bp;
;            out = 0;

;            for (j = shift_start[0]; j > -shift_dec[0]; j -= shift_dec[0])
;            {
;               if (j > 0)
;                  out |= v << j;

;               else
;                  out |= (v >> (-j)) & mask;
;            }

;            *bp = (byte)(out & 0xff);
;         }
;      }

;      else if (row_info->bit_depth == 8)
;      {
;         bytep bp = row;
;         uint_32 i;
;         uint_32 istop = channels * row_info->width;

;         for (i = 0; i < istop; i++, bp++)
;         {

;            const unsigned int c = i%channels;
;            int j;
;            unsigned int v, out;

;            v = *bp;
;            out = 0;

;            for (j = shift_start[c]; j > -shift_dec[c]; j -= shift_dec[c])
;            {
;               if (j > 0)
;                  out |= v << j;

;               else
;                  out |= v >> (-j);
;            }

;            *bp = (byte)(out & 0xff);
;         }
;      }

;      else
;      {
;         bytep bp;
;         uint_32 i;
;         uint_32 istop = channels * row_info->width;

;         for (bp = row, i = 0; i < istop; i++)
;         {
;            const unsigned int c = i%channels;
;            int j;
;            unsigned int value, v;

;            v = png_get_uint_16(bp);
;            value = 0;

;            for (j = shift_start[c]; j > -shift_dec[c]; j -= shift_dec[c])
;            {
;               if (j > 0)
;                  value |= v << j;

;               else
;                  value |= v >> (-j);
;            }
;            *bp++ = (byte)((value >> 8) & 0xff);
;            *bp++ = (byte)(value & 0xff);
;         }
;      }
;   }
	ret
endp

;void (png_row_infop row_info, bytep row)
align 4
proc png_do_write_swap_alpha, row_info:dword, row:dword
	png_debug 1, 'in png_do_write_swap_alpha'

;   {
;      if (row_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
;      {
;         if (row_info->bit_depth == 8)
;         {
		; This converts from ARGB to RGBA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
;               byte save = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = save;
;            }
;         }

if PNG_WRITE_16BIT_SUPPORTED eq 1
;         else
;         {
		; This converts from AARRGGBB to RRGGBBAA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;
;
;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
;               byte save[2];
;               save[0] = *(sp++);
;               save[1] = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = save[0];
;               *(dp++) = save[1];
;            }
;         }
end if ;WRITE_16BIT
;      }
;
;      else if (row_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
;      {
;         if (row_info->bit_depth == 8)
;         {
		; This converts from AG to GA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
;               byte save = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = save;
;            }
;         }

if PNG_WRITE_16BIT_SUPPORTED eq 1
;         else
;         {
	; This converts from AAGG to GGAA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
;               byte save[2];
;               save[0] = *(sp++);
;               save[1] = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = *(sp++);
;               *(dp++) = save[0];
;               *(dp++) = save[1];
;            }
;         }
end if ;WRITE_16BIT
;      }
;   }
	ret
endp

;void (png_row_infop row_info, bytep row)
align 4
proc png_do_write_invert_alpha, row_info:dword, row:dword
	png_debug 1, 'in png_do_write_invert_alpha'

;      if (row_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
;      {
;         if (row_info->bit_depth == 8)
;         {
	; This inverts the alpha channel in RGBA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
	; Does nothing
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);

;               sp+=3; dp = sp;
;               *dp = (byte)(255 - *(sp++));
;            }
;         }

;if PNG_WRITE_16BIT_SUPPORTED
;         else
;         {
		; This inverts the alpha channel in RRGGBBAA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
	; Does nothing
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);

;               sp+=6; dp = sp;
;               *(dp++) = (byte)(255 - *(sp++));
;               *dp     = (byte)(255 - *(sp++));
;            }
;         }
;end if /* WRITE_16BIT */
;      }

;      else if (row_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
;      {
;         if (row_info->bit_depth == 8)
;         {
		; This inverts the alpha channel in GA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;
;
;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
;               *(dp++) = *(sp++);
;               *(dp++) = (byte)(255 - *(sp++));
;            }
;         }

if PNG_WRITE_16BIT_SUPPORTED eq 1
;         else
;         {
	; This inverts the alpha channel in GGAA
;            bytep sp, dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;
;
;            for (i = 0, sp = dp = row; i < row_width; i++)
;            {
	; Does nothing
	;*(dp++) = *(sp++);
	;*(dp++) = *(sp++);

;               sp+=2; dp = sp;
;               *(dp++) = (byte)(255 - *(sp++));
;               *dp     = (byte)(255 - *(sp++));
;            }
;         }
end if ;WRITE_16BIT
;      }
	ret
endp

; Transform the data according to the user's wishes.  The order of
; transformations is significant.

;void (png_structrp png_ptr, png_row_infop row_info)
align 4
proc png_do_write_transformations uses eax ebx edi esi, png_ptr:dword, row_info:dword
	png_debug 1, 'in png_do_write_transformations'

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return
	mov esi,[row_info]
	mov ebx,[edi+png_struct.row_buf]
	inc ebx ;start of pixel data for row

if PNG_WRITE_USER_TRANSFORM_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_USER_TRANSFORM
	jz @f ;if (..!=0)
	mov eax,[edi+png_struct.write_user_transform_fn]
	test eax,eax
	jz @f ;if (..!=0)
		stdcall eax, edi, esi, ebx ;User write transform function
		; row_info:
		;  uint_32 width    ;width of row
		;  png_size_t rowbytes ;number of bytes in row
		;  byte  color_type ;color type of pixels
		;  byte  bit_depth  ;bit depth of samples
		;  byte  channels   ;number of channels (1-4)
		;  byte  pixel_depth ;bits per pixel (depth*channels)
	@@:
end if

if PNG_WRITE_FILLER_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_FILLER
	jz @f ;if (..!=0)
		mov eax,[edi+png_struct.flags]
		and eax,PNG_FLAG_FILLER_AFTER
		not eax
		stdcall png_do_strip_channel, esi, ebx, eax
	@@:
end if

if PNG_WRITE_PACKSWAP_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_PACKSWAP
	jz @f ;if (..!=0)
		stdcall png_do_packswap, esi, ebx
	@@:
end if

if PNG_WRITE_PACK_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_PACK
	jz @f ;if (..!=0)
		movzx eax,byte[edi+png_struct.bit_depth]
		stdcall png_do_pack, esi, ebx, eax
	@@:
end if

if PNG_WRITE_SWAP_SUPPORTED eq 1
if PNG_16BIT_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_SWAP_BYTES
	jz @f ;if (..!=0)
		stdcall png_do_swap, esi, ebx
	@@:
end if
end if

if PNG_WRITE_SHIFT_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_SHIFT
	jz @f ;if (..!=0)
		mov eax,edi
		add eax,png_struct.shift
		stdcall png_do_shift, esi, ebx, eax
	@@:
end if

if PNG_WRITE_SWAP_ALPHA_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_SWAP_ALPHA
	jz @f ;if (..!=0)
		stdcall png_do_write_swap_alpha, esi, ebx
	@@:
end if

if PNG_WRITE_INVERT_ALPHA_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_INVERT_ALPHA
	jz @f ;if (..!=0)
		stdcall png_do_write_invert_alpha, esi, ebx
	@@:
end if

if PNG_WRITE_BGR_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_BGR
	jz @f ;if (..!=0)
		stdcall png_do_bgr, esi, ebx
	@@:
end if

if PNG_WRITE_INVERT_SUPPORTED eq 1
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_INVERT_MONO
	jz @f ;if (..!=0)
		stdcall png_do_invert, esi, ebx
	@@:
end if
.end_f:
	ret
endp
