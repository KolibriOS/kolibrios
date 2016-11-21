
; pngwio.asm - functions for data output

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2014,2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc

; This file provides a location for all output.  Users who need
; special handling are expected to write functions that have the same
; arguments as these and perform similar functions, but that possibly
; use different output methods.  Note that you shouldn't change these
; functions, but rather write replacement functions and then change
; them at run time with png_set_write_fn(...).


; Write the data to whatever output you are using.  The default routine
; writes to a file pointer.  Note that this routine sometimes gets called
; with very small lengths, so you should implement some kind of simple
; buffering if you are using unbuffered writes.  This should never be asked
; to write more than 64K on a 16-bit machine.

;void (png_structrp png_ptr, bytep data, png_size_t length)
align 4
proc png_write_data uses edi, png_ptr:dword, p2data:dword, length:dword
	; NOTE: write_data_fn must not change the buffer!
	mov edi,[png_ptr]
	cmp dword[edi+png_struct.write_data_fn],0
	je @f ;if (..!=0)
		stdcall dword[edi+png_struct.write_data_fn], edi, [p2data], [length]
		jmp .end_f
	@@: ;else
		png_error edi, 'Call to NULL write function'
	.end_f:
	ret
endp

; This is the function that does the actual writing of data.  If you are
; not writing to a standard C stream, you should create a replacement
; write_data function and use it at run time with png_set_write_fn(), rather
; than changing the library.

;void (png_structp png_ptr, bytep data, png_size_t length)
align 4
proc png_default_write_data uses eax edi, png_ptr:dword, p2data:dword, length:dword
;   png_size_t check;

	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

;   check = fwrite(p2data, 1, length, (png_FILE_p)(png_ptr->io_ptr));

;   if (check != length)
;      png_error(png_ptr, "Write Error");
.end_f:
	ret
endp

; This function is called to output any data pending writing (normally
; to disk).  After png_flush is called, there should be no data pending
; writing in any buffers.

;void (png_structrp png_ptr)
align 4
proc png_flush uses edi, png_ptr:dword
	mov edi,[png_ptr]
	cmp dword[edi+png_struct.output_flush_fn],0
	je @f ;if (..!=..)
		stdcall dword[edi+png_struct.output_flush_fn],edi
	@@:
	ret
endp

;void (png_structp png_ptr)
align 4
proc png_default_flush uses eax edi, png_ptr:dword
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return
;;;		stdcall fflush, [edi+png_struct.io_ptr]
	@@:
	ret
endp

; This function allows the application to supply new output functions for
; libpng if standard C streams aren't being used.

; This function takes as its arguments:
; png_ptr       - pointer to a png output data structure
; io_ptr        - pointer to user supplied structure containing info about
;                 the output functions.  May be NULL.
; write_data_fn - pointer to a new output function that takes as its
;                 arguments a pointer to a png_struct, a pointer to
;                 data to be written, and a 32-bit unsigned int that is
;                 the number of bytes to be written.  The new write
;                 function should call png_error(png_ptr, "Error msg")
;                 to exit and output any fatal error messages.  May be
;                 NULL, in which case libpng's default function will
;                 be used.
; flush_data_fn - pointer to a new flush function that takes as its
;                 arguments a pointer to a png_struct.  After a call to
;                 the flush function, there should be no data in any buffers
;                 or pending transmission.  If the output method doesn't do
;                 any buffering of output, a function prototype must still be
;                 supplied although it doesn't have to do anything.  If
;                 PNG_WRITE_FLUSH_SUPPORTED is not defined at libpng compile
;                 time, output_flush_fn will be ignored, although it must be
;                 supplied for compatibility.  May be NULL, in which case
;                 libpng's default function will be used, if
;                 PNG_WRITE_FLUSH_SUPPORTED is defined.  This is not
;                 a good idea if io_ptr does not point to a standard
;                 *FILE structure.

;void (png_structrp png_ptr, voidp io_ptr,
;    png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)
align 4
proc png_set_write_fn uses eax edi, png_ptr:dword, io_ptr:dword, write_data_fn:dword, output_flush_fn:dword
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	mov eax,[io_ptr]
	mov [edi+png_struct.io_ptr],eax

if PNG_STDIO_SUPPORTED eq 1
	mov eax,png_default_write_data ;else
	cmp dword[write_data_fn],0
	je @f ;if (..!=0)
		mov eax,[write_data_fn]
	@@: 
else
	mov eax,[write_data_fn]
end if
	mov [edi+png_struct.write_data_fn],eax

if PNG_WRITE_FLUSH_SUPPORTED eq 1
	if PNG_STDIO_SUPPORTED eq 1
		mov eax,[png_default_flush] ;else
		cmp dword[output_flush_fn],0
		je @f ;if (..!=0)
			mov eax,[output_flush_fn]
		@@:
	else
		mov eax,[output_flush_fn]
	end if
	mov [edi+png_struct.output_flush_fn],eax
end if ;WRITE_FLUSH

if PNG_READ_SUPPORTED eq 1
	; It is an error to read while writing a png file
	cmp dword[edi+png_struct.read_data_fn],0
	je @f ;if (..!=0)
		mov dword[edi+png_struct.read_data_fn], 0

		png_warning edi, <'Can',39,'t set both read_data_fn and write_data_fn in the same structure'>
	@@:
end if
.end_f:
	ret
endp
