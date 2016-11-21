
; pngmem.asm - stub functions for memory allocation

; Last changed in libpng 1.6.24 [August 4, 2016%]
; Copyright (c) 1998-2002,2004,2006-2014,2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc

; This file provides a location for all memory allocation.  Users who
; need special memory handling are expected to supply replacement
; functions for png_malloc() and png_free(), and to use
; png_create_read_struct_2() and png_create_write_struct_2() to
; identify the replacement functions.

; Free a png_struct
;void (png_structrp png_ptr)
align 4
proc png_destroy_png_struct uses eax ecx edi esi, png_ptr:dword
locals
	dummy_struct png_struct
endl
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..!=0)
		; png_free might call png_error and may certainly call
		; png_get_mem_ptr, so fake a temporary png_struct to support this.

		mov ecx,sizeof.png_struct
		mov esi,edi
		mov edi,ebp
		sub edi,ecx
		rep movsb ;dummy_struct = *png_ptr
		mov edi,[png_ptr]
		xor eax,eax
		mov ecx,sizeof.png_struct
		rep stosb ;memset(png_ptr, 0, (sizeof *png_ptr))
		mov esi,ebp
		sub esi,sizeof.png_struct
		stdcall png_free, esi, [png_ptr]

if PNG_SETJMP_SUPPORTED eq 1
		; We may have a jmp_buf left to deallocate.
		stdcall png_free_jmpbuf, esi
end if
	@@:
	ret
endp

; Allocate memory.  For reasonable files, size should never exceed
; 64K.  However, zlib may allocate more than 64K if you don't tell
; it not to.  See zconf.h and png.h for more information.  zlib does
; need to allocate exactly 64K, so whatever you call here must
; have the ability to do that.

;voidp (const_structrp png_ptr, png_alloc_size_t size)
align 4
proc png_calloc uses ebx ecx edi, png_ptr:dword, size:dword
	stdcall png_malloc, [png_ptr], [size]

	cmp eax,0
	je @f ;if (..!=0)
		mov ebx,eax
		mov edi,eax
		mov ecx,[size]
		xor eax,eax
		rep stosb ;memset(ret, 0, size)
		mov eax,ebx
	@@:
	ret
endp

; png_malloc_base, an internal function added at libpng 1.6.0, does the work of
; allocating memory, taking into account limits and PNG_USER_MEM_SUPPORTED.
; Checking and error handling must happen outside this routine; it returns NULL
; if the allocation cannot be done (for any reason.)

;voidp (const_structrp png_ptr, png_alloc_size_t size)
align 4
proc png_malloc_base uses ebx ecx, png_ptr:dword, size:dword
	; Moved to png_malloc_base from png_malloc_default in 1.6.0; the DOS
	; allocators have also been removed in 1.6.0, so any 16-bit system now has
	; to implement a user memory handler.  This checks to be sure it isn't
	; called with big numbers.

	; Some compilers complain that this is always true.  However, it
	; can be false when integer overflow happens.

	cmp dword[size],0
	jle .end0
	cmp dword[size],PNG_SIZE_MAX
	jg .end0 ;   if (..>.. && ..<=..)
if PNG_MAX_MALLOC_64K eq 1
	cmp dword[size],65536
	jg .end0
end if
if PNG_USER_MEM_SUPPORTED eq 1
	mov ebx,[png_ptr]
	cmp ebx,0
	je @f
	cmp dword[ebx+png_struct.malloc_fn],0
	je @f ;if (..!=0 && ..!=0)
		stdcall [ebx+png_struct.malloc_fn], ebx, [size]
		jmp .end_f
	@@: ;else
end if
		;stdcall [mem.alloc], [size]
		mcall SF_SYS_MISC, SSF_MEM_ALLOC, [size]
		jmp .end_f ;checked for truncation above
	.end0: ;else
	xor eax,eax
.end_f:
	ret
endp

; This is really here only to work round a spurious warning in GCC 4.6 and 4.7
; that arises because of the checks in png_realloc_array that are repeated in
; png_malloc_array.

;voidp (const_structrp png_ptr, int nelements, size_t element_size)
align 4
proc png_malloc_array_checked, png_ptr:dword, nelements:dword, element_size:dword
;   png_alloc_size_t req = nelements; /* known to be > 0 */

;   if (req <= PNG_SIZE_MAX/element_size)
;      return png_malloc_base(png_ptr, req * element_size);

	; The failure case when the request is too large
	xor eax,eax
.end_f:
	ret
endp

;voidp (const_structrp png_ptr, int nelements, size_t element_size)
align 4
proc png_malloc_array, png_ptr:dword, nelements:dword, element_size:dword
;   if (nelements <= 0 || element_size == 0)
;      png_error(png_ptr, "internal error: array alloc");

	stdcall png_malloc_array_checked, [png_ptr], [nelements], [element_size]
	ret
endp

;voidp (const_structrp png_ptr, const_voidp old_array,
;    int old_elements, int add_elements, size_t element_size)
align 4
proc png_realloc_array, png_ptr:dword, old_array:dword, old_elements:dword, add_elements:dword, element_size:dword
	; These are internal errors:
;   if (add_elements <= 0 || element_size == 0 || old_elements < 0 ||
;      (old_array == NULL && old_elements > 0))
;      png_error(png_ptr, "internal error: array realloc");

	; Check for overflow on the elements count (so the caller does not have to
	; check.)

;   if (add_elements <= INT_MAX - old_elements)
;   {
;      voidp new_array = png_malloc_array_checked(png_ptr,
;          old_elements+add_elements, element_size);
;
;      if (new_array != NULL)
;      {
	; Because png_malloc_array worked the size calculations below cannot
	; overflow.

;         if (old_elements > 0)
;            memcpy(new_array, old_array, element_size*(unsigned)old_elements);
;
;         memset((char*)new_array + element_size*(unsigned)old_elements, 0,
;             element_size*(unsigned)add_elements);
;
;         return new_array;
;      }
;   }

	xor eax,eax ;error
.end_f:
	ret
endp

; Various functions that have different error handling are derived from this.
; png_malloc always exists, but if PNG_USER_MEM_SUPPORTED is defined a separate
; function png_malloc_default is also provided.

;voidp (const_structrp png_ptr, png_alloc_size_t size)
align 4
proc png_malloc uses edi, png_ptr:dword, size:dword
	xor eax,eax
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return 0

	stdcall png_malloc_base, edi, [size]

	cmp eax,0
	jne @f ;if (..==0)
		png_error edi, 'Out of memory' ;'m' means png_malloc
	@@:
	ret
endp

;voidp (const_structrp png_ptr, png_alloc_size_t size)
align 4
proc png_malloc_default uses edi, png_ptr:dword, size:dword
	xor eax,eax
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return 0

	; Passing 'NULL' here bypasses the application provided memory handler.
	stdcall png_malloc_base, 0, [size] ;0 - use malloc

	cmp eax,0
	jne @f ;if (..==0)
		png_error edi, 'Out of Memory' ;'M' means png_malloc_default
	@@:
	ret
endp

; This function was added at libpng version 1.2.3.  The png_malloc_warn()
; function will issue a png_warning and return NULL instead of issuing a
; png_error, if it fails to allocate the requested memory.

;voidp (const_structrp png_ptr, png_alloc_size_t size)
align 4
proc png_malloc_warn uses edi, png_ptr:dword, size:dword
	mov edi,[png_ptr]
	cmp edi,0
	je .end0 ;if (..!=0)
		stdcall png_malloc_base, edi, [size]
		cmp eax,0
		jne .end_f ;if (..!=0) return ret

		png_warning edi, 'Out of memory'
	.end0:
	xor eax,eax
.end_f:
	ret
endp

; Free a pointer allocated by png_malloc().  If ptr is NULL, return
; without taking any action.

;void (const_structrp png_ptr, voidp ptr)
align 4
proc png_free uses eax ebx ecx, png_ptr:dword, p2ptr:dword
	mov ebx,[png_ptr]
	cmp ebx,0
	je .end_f
	mov ecx,[p2ptr]
	cmp ecx,0
	je .end_f ;if (..==0 || ..==0) return

if PNG_USER_MEM_SUPPORTED eq 1
	cmp dword[ebx+png_struct.free_fn],0
	je @f ;if (..!=0)
		stdcall dword[ebx+png_struct.free_fn], ebx, [p2ptr]
		jmp .end_f
	@@: ;else
end if
		mcall SF_SYS_MISC, SSF_MEM_FREE, [p2ptr]
.end_f:
	ret
endp

; This function is called when the application wants to use another method
; of allocating and freeing memory.

;void (png_structrp png_ptr, voidp mem_ptr, png_malloc_ptr malloc_fn, png_free_ptr free_fn)
align 4
proc png_set_mem_fn uses eax edi, png_ptr:dword, mem_ptr:dword, malloc_fn:dword, free_fn:dword
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..!=0)
		mov eax,[mem_ptr]
		mov [edi+png_struct.mem_ptr],eax
		mov eax,[malloc_fn]
		mov [edi+png_struct.malloc_fn],eax
		mov eax,[free_fn]
		mov [edi+png_struct.free_fn],eax
	@@:
	ret
endp

; This function returns a pointer to the mem_ptr associated with the user
; functions.  The application should free any memory associated with this
; pointer before png_write_destroy and png_read_destroy are called.

;voidp (const_structrp png_ptr)
align 4
proc png_get_mem_ptr uses edi, png_ptr:dword
	xor eax,eax
	mov edi,[png_ptr]
	cmp edi,0
	je @f ;if (..==0) return 0
		mov eax,[edi+png_struct.mem_ptr]
	@@:
	ret
endp

