
; pngwutil.asm - utilities to write a PNG file

; Last changed in libpng 1.6.24 [August 4, 2016]
; Copyright (c) 1998-2002,2004,2006-2016 Glenn Randers-Pehrson
; (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
; (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)

; This code is released under the libpng license.
; For conditions of distribution and use, see the disclaimer
; and license in png.inc

; Place a 32-bit number into a buffer in PNG byte order.  We work
; with unsigned numbers for convenience, although one supported
; ancillary chunk uses signed (two's complement) numbers.

;void (bytep buf, uint_32 i)
align 4
proc png_save_uint_32 uses eax edi, buf:dword, i:dword
	mov eax,[i]
	bswap eax
	mov edi,[buf]
	stosd
	ret
endp

; Place a 16-bit number into a buffer in PNG byte order.
; The parameter is declared unsigned int, not uint_16,
; just to avoid potential problems on pre-ANSI C compilers.

;void (bytep buf, unsigned int i)
align 4
proc png_save_uint_16 uses eax edi, buf:dword, i:dword
	mov eax,[i]
	ror eax,16
	bswap eax
	mov edi,[buf]
	stosw
	ret
endp

; Simple function to write the signature.  If we have already written
; the magic bytes of the signature, or more likely, the PNG stream is
; being embedded into another stream and doesn't need its own signature,
; we should call png_set_sig_bytes() to tell libpng how many of the
; bytes have already been written.

align 4
png_signature db 137, 80, 78, 71, 13, 10, 26, 10

;void (png_structrp png_ptr)
align 4
proc png_write_sig uses eax ebx edi, png_ptr:dword
if PNG_IO_STATE_SUPPORTED eq 1
	; Inform the I/O callback that the signature is being written
	mov edi,[png_ptr]
	mov dword[edi+png_struct.io_state], PNG_IO_WRITING or PNG_IO_SIGNATURE
end if

	; Write the rest of the 8 byte signature
	movzx eax,byte[edi+png_struct.sig_bytes]
	mov ebx,8
	sub ebx,eax
	add eax,png_signature
	stdcall png_write_data, edi, eax, ebx

	cmp byte[edi+png_struct.sig_bytes], 3 ;if(..<3)
	jge @f
		or dword[edi+png_struct.mode], PNG_HAVE_PNG_SIGNATURE
	@@:
	ret
endp

; Write the start of a PNG chunk.  The type is the chunk type.
; The total_length is the sum of the lengths of all the data you will be
; passing in png_write_chunk_data().

;void (png_structrp png_ptr, uint_32 chunk_name, uint_32 length)
align 4
proc png_write_chunk_header uses ebx edi, png_ptr:dword, chunk_name:dword, length:dword
locals
	buf rb 8 ;ebp-8
endl

if (PNG_DEBUG eq 1) & (PNG_DEBUG > 0)
;   PNG_CSTRING_FROM_CHUNK(buf, chunk_name);
;   png_debug2(0, "Writing %s chunk, length = %lu", buf, (unsigned long)length);
end if

	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return

if PNG_IO_STATE_SUPPORTED eq 1
	; Inform the I/O callback that the chunk header is being written.
	; PNG_IO_CHUNK_HDR requires a single I/O call.

	mov dword[edi+png_struct.io_state], PNG_IO_WRITING or PNG_IO_CHUNK_HDR
end if

	; Write the length and the chunk name
	lea ebx,[ebp-8]
	stdcall png_save_uint_32, ebx, [length]
	m2m dword[ebx+4],dword[chunk_name]
	stdcall png_write_data, edi, ebx, 8

	; Put the chunk name into png_ptr->chunk_name
	m2m dword[edi+png_struct.chunk_name],dword[chunk_name]

	; Reset the crc and run it over the chunk name
	stdcall png_reset_crc, edi
	lea ebx,[ebp-4] ;buf + 4
	stdcall png_calculate_crc, edi, ebx, 4

if PNG_IO_STATE_SUPPORTED eq 1
	; Inform the I/O callback that chunk data will (possibly) be written.
	; PNG_IO_CHUNK_DATA does NOT require a specific number of I/O calls.

	mov dword[edi+png_struct.io_state], PNG_IO_WRITING or PNG_IO_CHUNK_DATA
end if
.end_f:
	ret
endp

;void (png_structrp png_ptr, bytep chunk_string, uint_32 length)
align 4
proc png_write_chunk_start uses eax, png_ptr:dword, chunk_string:dword, length:dword
	mov eax,[chunk_string]
	stdcall png_write_chunk_header, [png_ptr], [eax], [length]
	ret
endp

; Write the data of a PNG chunk started with png_write_chunk_header().
; Note that multiple calls to this function are allowed, and that the
; sum of the lengths from these calls *must* add up to the total_length
; given to png_write_chunk_header().

;void (png_structrp png_ptr, bytep data, png_size_t length)
align 4
proc png_write_chunk_data uses edi, png_ptr:dword, p2data:dword, length:dword
	; Write the data, and run the CRC over it
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

	cmp dword[p2data],0
	je .end_f
	cmp dword[length],0
	jle .end_f ;if (..!=0 && ..>0)
		stdcall png_write_data, edi, [p2data], [length]
		; Update the CRC after writing the data,
		; in case the user I/O routine alters it.
		stdcall png_calculate_crc, edi, [p2data], [length]
.end_f:
	ret
endp

; Finish a chunk started with png_write_chunk_header().
;void (png_structrp png_ptr)
align 4
proc png_write_chunk_end uses ebx edi, png_ptr:dword
locals
	buf rb 4 ;ebp-4
endl
	mov edi,[png_ptr]
	cmp edi,0
	je .end_f ;if (..==0) return

if PNG_IO_STATE_SUPPORTED eq 1
	; Inform the I/O callback that the chunk CRC is being written.
	; PNG_IO_CHUNK_CRC requires a single I/O function call.

	mov dword[edi+png_struct.io_state], PNG_IO_WRITING or PNG_IO_CHUNK_CRC
end if

	; Write the crc in a single operation
	lea ebx,[ebp-4]
	stdcall png_save_uint_32, ebx, [edi+png_struct.crc]

	stdcall png_write_data, edi, ebx, 4
.end_f:
	ret
endp

; Write a PNG chunk all at once.  The type is an array of ASCII characters
; representing the chunk name.  The array must be at least 4 bytes in
; length, and does not need to be null terminated.  To be safe, pass the
; pre-defined chunk names here, and if you need a new one, define it
; where the others are defined.  The length is the length of the data.
; All the data must be present.  If that is not possible, use the
; png_write_chunk_start(), png_write_chunk_data(), and png_write_chunk_end()
; functions instead.

;void (png_structrp png_ptr, uint_32 chunk_name, bytep data, png_size_t length)
align 4
proc png_write_complete_chunk uses edi, png_ptr:dword, chunk_name:dword, p3data:dword, length:dword
	mov edi,[png_ptr]
	test edi,edi
	jz .end_f ;if (..==0) return

	; On 64-bit architectures 'length' may not fit in a uint_32.
	cmp dword[length],PNG_UINT_31_MAX ;if(..>..)
	jle @f
		png_error edi, 'length exceeds PNG maximum'
	@@:
	stdcall png_write_chunk_header, edi, [chunk_name], [length]
	stdcall png_write_chunk_data, edi, [p3data], [length]
	stdcall png_write_chunk_end, edi
.end_f:
	ret
endp

; This is the API that calls the internal function above.
;void (png_structrp png_ptr, bytep chunk_string, bytep data, png_size_t length)
align 4
proc png_write_chunk, png_ptr:dword, chunk_string:dword, p3data:dword, length:dword
	stdcall png_write_complete_chunk, [png_ptr], [chunk_string], [p3data], [length]
	ret
endp

; This is used below to find the size of an image to pass to png_deflate_claim,
; so it only needs to be accurate if the size is less than 16384 bytes (the
; point at which a lower LZ window size can be used.)

;png_alloc_size_t (png_structrp png_ptr)
align 4
proc png_image_size uses ebx ecx edx edi esi, png_ptr:dword
; Only return sizes up to the maximum of a uint_32; do this by limiting
; the width and height used to 15 bits.

	mov edi,[png_ptr]
	mov ebx,[edi+png_struct.height]

	cmp dword[edi+png_struct.rowbytes],32768
	jge .end0
	cmp ebx,32768
	jge .end0 ;if (..<..  && ..<..)
		cmp byte[edi+png_struct.interlaced],0
		je .end1 ;if (..!=0)
			; Interlacing makes the image larger because of the replication of
			; both the filter byte and the padding to a byte boundary.

			xor esi,esi
			xor ecx,ecx
			.cycle0:
				PNG_PASS_COLS [edi+png_struct.width], ecx
				;eax = pw

				cmp eax,0
				jle @f ;if (..>0)
					mov edx,eax
					movzx eax,byte[edi+png_struct.pixel_depth]
					PNG_ROWBYTES eax, edx
					inc eax
					mov edx,eax
					PNG_PASS_ROWS ebx, ecx
					imul eax,edx
					add esi,eax
				@@:
				inc ecx
				cmp ecx,6
				jle .cycle0

			mov eax,esi
			jmp .end_f
		.end1: ;else
			mov eax,[edi+png_struct.rowbytes]
			inc eax
			imul eax,ebx
			jmp .end_f
	.end0: ;else
		mov eax,0xffffffff
.end_f:
	ret
endp

; This is the code to hack the first two bytes of the deflate stream (the
; deflate header) to correct the windowBits value to match the actual data
; size.  Note that the second argument is the *uncompressed* size but the
; first argument is the *compressed* data (and it must be deflate
; compressed.)

;void (bytep data, png_alloc_size_t data_size)
align 4
proc optimize_cmf, p1data:dword, data_size:dword
; Optimize the CMF field in the zlib stream.  The resultant zlib stream is
; still compliant to the stream specification.
	png_debug 1, 'optimize_cmf'
pushad
	cmp dword[data_size],16384
	jg .end_f ;if (..<=..) ;else windowBits must be 15
		mov esi,[p1data]
		movzx ebx,byte[esi]
		;ebx = z_cmf ;zlib compression method and flags

		mov eax,ebx
		and eax,0x0f
		cmp eax,8
		jne .end_f
		mov eax,ebx
		and eax,0xf0
		cmp eax,0x70
		jg .end_f ;if (..==.. && ..<=..)
			;ecx = z_cinfo
			;edi = half_z_window_size

			mov ecx,ebx
			shr ecx,4
			xor edi,edi
			inc edi
			shl edi,7
			shl edi,cl

			cmp [data_size],edi
			jg .end_f ;if (..<=..) ;else no change
				.cycle0: ;do
					shr edi,1
					dec ecx
					cmp ecx,0
					jle @f
					cmp [data_size],edi
					jle .cycle0
				@@: ;while (..>0 && ..<=..);

				and ebx,0x0f
				mov eax,ecx
				shl eax,4
				or ebx,eax

				mov byte[esi],bl
				movzx eax,byte[esi+1]
				and eax,0xe0
				shl ebx,8
				add ebx,eax
				add eax,0x1f
				xchg eax,ebx
				xor edx,edx
				mov ecx,0x1f
				div ecx
				sub ebx,edx
				mov byte[esi+1],bl
.end_f:
popad
	ret
endp

; Initialize the compressor for the appropriate type of compression.
;int (png_structrp png_ptr, uint_32 owner, png_alloc_size_t data_size)
;input:
; edi - png_ptr
align 4
proc png_deflate_claim uses ebx ecx, owner:dword, data_size:dword
locals
	level dd ? ;int
	method dd ? ;int
	windowBits dd ? ;int
	memLevel dd ? ;int
	strategy dd ? ;int
	msg rb 64 ;char[64]
endl
	png_debug 1, 'in png_deflate_claim'

	cmp dword[edi+png_struct.zowner],0
	je .end0 ;if (..!=0)
		lea ebx,[ebp-64]
if (PNG_WARNINGS_SUPPORTED eq 1) | (PNG_ERROR_TEXT_SUPPORTED eq 1)
		mov eax,[owner]
		mov [ebx],eax
		mov word[ebx+4],': '
		mov eax,[edi+png_struct.zowner]
		mov [ebx+6],eax
		; So the message that results is "<chunk> using zstream"; this is an
		; internal error, but is very useful for debugging.  i18n requirements
		; are minimal.

		cStr ,' using zstream'
		stdcall png_safecat, ebx, 64, 10, eax
end if
if PNG_RELEASE_BUILD eq 1
		png_warning edi, ebx

		; Attempt sane error recovery
		cmp dword[edi+png_struct.zowner],png_IDAT
		jne @f ;if (..==.) ;don't steal from IDAT
			cStr dword[edi+png_struct.zstream.msg],'in use by IDAT'
			mov eax,Z_STREAM_ERROR
			jmp .end_f
		@@:
		mov dword[edi+png_struct.zowner],0
else
		png_error edi, ebx
end if
	.end0:

	mov eax,[edi+png_struct.zlib_level]
	mov [level],eax
	mov eax,[edi+png_struct.zlib_method]
	mov [method],eax
	mov eax,[edi+png_struct.zlib_window_bits]
	mov [windowBits],eax
	mov eax,[edi+png_struct.zlib_mem_level]
	mov [memLevel],eax

	cmp dword[owner],png_IDAT
	jne .end1 ;if (..==..)
		mov eax,[edi+png_struct.flags]
		and eax,PNG_FLAG_ZLIB_CUSTOM_STRATEGY
		jz @f ;if (..!=0)
			mov eax,[edi+png_struct.zlib_strategy]
			mov dword[strategy],eax
			jmp .end2
		@@:
		cmp byte[edi+png_struct.do_filter],PNG_FILTER_NONE
		je @f ;else if (..!=..)
			mov dword[strategy],PNG_Z_DEFAULT_STRATEGY
			jmp .end2
		@@: ;else
			mov dword[strategy],PNG_Z_DEFAULT_NOFILTER_STRATEGY
		jmp .end2
	.end1: ;else
if PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED eq 1
		mov eax,[edi+png_struct.zlib_text_level]
		mov [level],eax
		mov eax,[edi+png_struct.zlib_text_method]
		mov [method],eax
		mov eax,[edi+png_struct.zlib_text_window_bits]
		mov [windowBits],eax
		mov eax,[edi+png_struct.zlib_text_mem_level]
		mov [memLevel],eax
		mov eax,[edi+png_struct.zlib_text_strategy]
		mov [strategy],eax
else
		; If customization is not supported the values all come from the
		; IDAT values except for the strategy, which is fixed to the
		; default.  (This is the pre-1.6.0 behavior too, although it was
		; implemented in a very different way.)

		mov dword[strategy],Z_DEFAULT_STRATEGY
end if
	.end2:

	; Adjust 'windowBits' down if larger than 'data_size'; to stop this
	; happening just pass 32768 as the data_size parameter.  Notice that zlib
	; requires an extra 262 bytes in the window in addition to the data to be
	; able to see the whole of the data, so if data_size+262 takes us to the
	; next windowBits size we need to fix up the value later.  (Because even
	; though deflate needs the extra window, inflate does not!)

	cmp dword[data_size],16384
	jg .end3 ;if (..<=..)
		; IMPLEMENTATION NOTE: this 'half_window_size' stuff is only here to
		; work round a Microsoft Visual C misbehavior which, contrary to C-90,
		; widens the result of the following shift to 64-bits if (and,
		; apparently, only if) it is used in a test.

		mov ecx,[windowBits]
		dec ecx
		xor eax,eax
		inc eax
		shl eax,cl ;eax = half_window_size
		mov ecx,[data_size]
		add ecx,262
		@@: ;while (..<=..)
			cmp ecx,eax
			jg .end3
			shr eax,1
			dec dword[windowBits]
			jmp @b
	.end3:

	; Check against the previous initialized values, if any.
	mov eax,[edi+png_struct.flags]
	and eax,PNG_FLAG_ZSTREAM_INITIALIZED
	jz .end4
	mov eax,[level]
	cmp [edi+png_struct.zlib_set_level],eax
	jne @f
	mov eax,[method]
	cmp [edi+png_struct.zlib_set_method],eax
	jne @f
	mov eax,[windowBits]
	cmp [edi+png_struct.zlib_set_window_bits],eax
	jne @f
	mov eax,[memLevel]
	cmp [edi+png_struct.zlib_set_mem_level],eax
	jne @f
	mov eax,[strategy]
	cmp [edi+png_struct.zlib_set_strategy],eax
	je .end4
	@@: ;if (..!=0 && (..!=.. || ..!=.. || ..!=.. || ..!=.. || ..!=..))
		mov eax,edi
		add eax,png_struct.zstream
		stdcall [deflateEnd], eax
		cmp eax,Z_OK
		je @f ;if (..!=..)
			png_warning edi, 'deflateEnd failed (ignored)'
		@@:
		and dword[edi+png_struct.flags], not PNG_FLAG_ZSTREAM_INITIALIZED
	.end4:

	; For safety clear out the input and output pointers (currently zlib
	; doesn't use them on Init, but it might in the future).

	mov dword[edi+png_struct.zstream.next_in],0
	mov dword[edi+png_struct.zstream.avail_in],0
	mov dword[edi+png_struct.zstream.next_out],0
	mov dword[edi+png_struct.zstream.avail_out],0

	; Now initialize if required, setting the new parameters, otherwise just
	; to a simple reset to the previous parameters.

	mov ecx,edi
	add ecx,png_struct.zstream
	mov eax,[edi+png_struct.flags]
	and eax,PNG_FLAG_ZSTREAM_INITIALIZED
	jz @f ;if (..!=0)
		stdcall [deflateReset], ecx
		jmp .end5
	@@: ;else
		stdcall [deflateInit2], ecx, [level], [method], [windowBits],\
			[memLevel], [strategy]

		cmp eax,Z_OK
		je .end5 ;if (..==..)
			or dword[edi+png_struct.flags],PNG_FLAG_ZSTREAM_INITIALIZED
	.end5:

	; The return code is from either deflateReset or deflateInit2; they have
	; pretty much the same set of error codes.

	cmp eax,Z_OK
	jne @f ;if (..==..)
		mov ecx,[owner]
		mov [edi+png_struct.zowner],ecx
		jmp .end_f
	@@: ;else
		stdcall png_zstream_error, edi, eax

.end_f:
	ret
endp

; Clean up (or trim) a linked list of compression buffers.
;void (png_structrp png_ptr, png_compression_bufferp *listp)
align 4
proc png_free_buffer_list uses eax ebx ecx edi, png_ptr:dword, listp:dword
	mov eax,[listp]
	mov ebx,[eax]
	;eax = png_compression_bufferp list

	cmp ebx,0
	je @f ;if (..!=0)
		mov dword[eax],0
		.cycle0: ;do
			mov ecx,[ebx+png_compression_buffer.next]
			stdcall png_free, edi, ebx
			mov ebx,ecx
		cmp ebx,0
		jne .cycle0 ;while (..!=0)
	@@:
	ret
endp

if PNG_WRITE_COMPRESSED_TEXT_SUPPORTED eq 1
; This pair of functions encapsulates the operation of (a) compressing a
; text string, and (b) issuing it later as a series of chunk data writes.
; The compression_state structure is shared context for these functions
; set up by the caller to allow access to the relevant local variables.

; compression_buffer (new in 1.6.0) is just a linked list of zbuffer_size
; temporary buffers.  From 1.6.0 it is retained in png_struct so that it will
; be correctly freed in the event of a write error (previous implementations
; just leaked memory.)

struct compression_state
	input      dd ? ;bytep      ;The uncompressed input data
	input_len  dd ? ;png_alloc_size_t ;Its length
	output_len dd ? ;uint_32          ;Final compressed length
	output     rb 1024 ;byte[1024]    ;First block of output
ends

;void (compression_state *comp, bytep input, png_alloc_size_t input_len)
align 4
proc png_text_compress_init uses eax ebx, comp:dword, input:dword, input_len:dword
	mov ebx,[comp]
	mov eax,[input]
	mov [ebx+compression_state.input],eax
	mov eax,[input_len]
	mov [ebx+compression_state.input_len],eax
	mov dword[ebx+compression_state.output_len],0
	ret
endp

; Compress the data in the compression state input
;int (png_structrp png_ptr, uint_32 chunk_name, compression_state *comp, uint_32 prefix_len)
align 4
proc png_text_compress uses ebx ecx edx edi esi, png_ptr:dword, chunk_name:dword, comp:dword, prefix_len:dword
locals
	output_len dd ? ;uint_32
	avail_in   dd ? ;uInt
	next       dd ? ;png_compression_buffer*
endl
	; To find the length of the output it is necessary to first compress the
	; input. The result is buffered rather than using the two-pass algorithm
	; that is used on the inflate side; deflate is assumed to be slower and a
	; PNG writer is assumed to have more memory available than a PNG reader.

	; IMPLEMENTATION NOTE: the zlib API deflateBound() can be used to find an
	; upper limit on the output size, but it is always bigger than the input
	; size so it is likely to be more efficient to use this linked-list
	; approach.

	mov ebx,[comp]
	mov edi,[png_ptr]
	stdcall png_deflate_claim, [chunk_name], [ebx+compression_state.input_len]

	cmp eax,Z_OK
	jne .end_f ;if (..!=Z_OK) return ..

	; Set up the compression buffers, we need a loop here to avoid overflowing a
	; uInt.  Use ZLIB_IO_MAX to limit the input.  The output is always limited
	; by the output buffer size, so there is no need to check that.  Since this
	; is ANSI-C we know that an 'int', hence a uInt, is always at least 16 bits
	; in size.

	mov edx,edi
	add edx,png_struct.zbuffer_list
	mov ecx,[ebx+compression_state.input_len] ;may be zero!
	;ecx = input_len
	;edx = end
	;esi = ret

	; zlib updates these for us:
	mov eax,[ebx+compression_state.input]
	mov [edi+png_struct.zstream.next_in],eax
	mov dword[edi+png_struct.zstream.avail_in],0 ;Set below
	mov eax,ebx
	add eax,compression_state.output
	mov [edi+png_struct.zstream.next_out],eax
	mov eax,sizeof.compression_state.output ;1024
	mov [edi+png_struct.zstream.avail_out],eax

	mov [output_len],eax

	.cycle0: ;do
		mov dword[avail_in],ZLIB_IO_MAX

		cmp [avail_in],ecx
		jle @f ;if (..>..)
			mov [avail_in],ecx
		@@:
		sub ecx,[avail_in]

		mov eax,[avail_in]
		mov [edi+png_struct.zstream.avail_in],eax

		cmp dword[edi+png_struct.zstream.avail_out],0
		jne .end0 ;if (..==0)
			; Chunk data is limited to 2^31 bytes in length, so the prefix
			; length must be counted here.

			mov eax,[output_len]
			add eax,[prefix_len]
			cmp eax,PNG_UINT_31_MAX
			jle @f ;if (..>..)
				mov esi,Z_MEM_ERROR
				jmp .cycle0end
			@@:

			; Need a new (malloc'ed) buffer, but there may be one present
			; already.

			mov eax,[edx]
			mov [next],eax

			test eax,eax
			jnz .end1 ;if (..==0)
				PNG_COMPRESSION_BUFFER_SIZE edi 
				stdcall png_malloc, edi, eax
				mov [next],eax

				test eax,eax
				jnz @f ;if (..==0)
					mov esi,Z_MEM_ERROR
					jmp .cycle0end
				@@:

				; Link in this buffer (so that it will be freed later)
				mov dword[eax+png_compression_buffer.next],0
				mov [edx],eax
			.end1:

			mov eax,[next]
			mov eax,[eax+png_compression_buffer.output]
			mov [edi+png_struct.zstream.next_out],eax
			mov eax,[edi+png_struct.zbuffer_size]
			mov [edi+png_struct.zstream.avail_out],eax
			add [output_len],eax

			; Move 'end' to the next buffer pointer.
			mov eax,[next]
			add eax,png_compression_buffer.next
			mov edx,eax
		.end0:

		; Compress the data
		mov eax,Z_FINISH
		cmp dword[input_len],0
		jle @f
			mov eax,Z_NO_FLUSH
		@@:
		push eax
		mov eax,edi
		add eax,png_struct.zstream
		stdcall [deflate], eax ;, ...
		mov esi,eax

		; Claw back input data that was not consumed (because avail_in is
		; reset above every time round the loop).

		mov eax,[edi+png_struct.zstream.avail_in]
		add [input_len],eax
		mov dword[edi+png_struct.zstream.avail_in],0 ;safety
		cmp esi,Z_OK
		je .cycle0 ;while (..==..)
	.cycle0end:

	; There may be some space left in the last output buffer. This needs to
	; be subtracted from output_len.

	mov eax,[edi+png_struct.zstream.avail_out]
	sub [output_len],eax
	mov dword[edi+png_struct.zstream.avail_out],0 ;safety
	mov eax,[output_len]
	mov [ebx+compression_state.output_len],eax

	; Now double check the output length, put in a custom message if it is
	; too long.  Otherwise ensure the z_stream::msg pointer is set to
	; something.

	mov eax,[output_len]
	add eax,[prefix_len]
	cmp eax,PNG_UINT_31_MAX
	jl @f ;if (..>=..)
		cStr dword[edi+png_struct.zstream.msg],'compressed data too long'
		mov esi,Z_MEM_ERROR
		jmp .end2
	@@: ;else
		stdcall png_zstream_error, edi, esi
	.end2:

	; Reset zlib for another zTXt/iTXt or image data
	mov dword[edi+png_struct.zowner],0

	; The only success case is Z_STREAM_END, input_len must be 0; if not this
	; is an internal error.

	cmp esi,Z_STREAM_END
	jne @f
	cmp dword[input_len],0
	jne @f ;if (..==.. && ..==0)
if PNG_WRITE_OPTIMIZE_CMF_SUPPORTED eq 1
		; Fix up the deflate header, if required
		mov eax,ebx
		add eax,compression_state.output
		stdcall optimize_cmf, eax, [ebx+compression_state.input_len]
end if
		; But Z_OK is returned, not Z_STREAM_END; this allows the claim
		; function above to return Z_STREAM_END on an error (though it never
		; does in the current versions of zlib.)

		mov eax,Z_OK
		jmp .end_f
	@@: ;else
		mov eax,esi
.end_f:
	ret
endp

; Ship the compressed text out via chunk writes
;void (png_structrp png_ptr, compression_state *comp)
align 4
proc png_write_compressed_data_out uses ebx edi, png_ptr:dword, comp:dword
locals
	output_len dd ? ;uint_32 ;= comp.output_len
	output dd ? ;bytep ;= comp.output
	avail  dd ? ;uint_32     ;= sizeof.comp.output
	next   dd ? ;png_compression_buffer* ;= png_ptr.zbuffer_list
endl
	mov ebx,[comp]
	mov eax,[ebx+compression_state.output_len]
	mov [output_len],eax
	mov eax,ebx
	add eax,compression_state.output
	mov [output],eax
	mov [avail],sizeof.compression_state.output ;1024
	mov edi,[png_ptr]
	mov eax,[edi+png_struct.zbuffer_list]
	mov [next],eax

	.cycle0: ;for (;;)
		mov eax,[output_len]
		cmp [avail],eax
		jle @f ;if (..>..)
			mov [avail],eax
		@@:

		stdcall png_write_chunk_data, edi, [output], [avail]

		mov [avail],eax
		sub [output_len],eax

		cmp dword[output_len],0
		je .cycle0end
		cmp dword[next],0
		je .cycle0end ;if (..==0 || ..==0) break

		mov eax,[edi+png_struct.zbuffer_size]
		mov [avail],eax
		mov eax,[next]
		add eax,png_compression_buffer.output
		mov [output],eax
		mov eax,[next]
		mov eax,[eax+png_compression_buffer.next]
		mov [next],eax
		jmp .cycle0
	.cycle0end:

	; This is an internal error; 'next' must have been NULL!
	cmp dword[output_len],0
	jle @f ;if (..>0)
		png_error edi, 'error writing ancillary chunked compressed data'
	@@:
	ret
endp
end if ;WRITE_COMPRESSED_TEXT

; Write the IHDR chunk, and update the png_struct with the necessary
; information.  Note that the rest of this code depends upon this
; information being correct.

;void (png_structrp png_ptr, uint_32 width, uint_32 height,
;    int bit_depth, int color_type, int compression_type, int filter_type, int interlace_type)
align 4
proc png_write_IHDR, png_ptr:dword, width:dword, height:dword, bit_depth:dword,\
	color_type:dword, compression_type:dword, filter_type:dword, interlace_type:dword
locals
	buf rb 13 ;byte[13] ;Buffer to store the IHDR info
endl
	png_debug 1, 'in png_write_IHDR'
pushad
	; Check that we have valid input data from the application info
	mov edi,[png_ptr]
	movzx ebx,byte[color_type]
	cmp ebx,PNG_COLOR_TYPE_GRAY
	jne .end_0
		cmp byte[bit_depth],1
		je @f
		cmp byte[bit_depth],2
		je @f
		cmp byte[bit_depth],4
		je @f
		cmp byte[bit_depth],8
		je @f
if PNG_WRITE_16BIT_SUPPORTED eq 1
		cmp byte[bit_depth],16
		je @f
end if
		jmp .def_0
	@@:
		mov byte[edi+png_struct.channels], 1
		jmp .end_s0
	.def_0: ;default
		png_error edi, 'Invalid bit depth for grayscale image'
		jmp .end_s0
	.end_0:

	cmp ebx,PNG_COLOR_TYPE_RGB
	jne .end_1
		cmp byte[bit_depth],8
		je @f ;if (..!=8)
if PNG_WRITE_16BIT_SUPPORTED eq 1
		cmp byte[bit_depth],16
		je @f ;if (..!=16)
end if
            png_error edi, 'Invalid bit depth for RGB image'
		@@:
		mov byte[edi+png_struct.channels], 3
		jmp .end_s0
	.end_1:

	cmp ebx,PNG_COLOR_TYPE_PALETTE
	jne .end_2
		cmp byte[bit_depth],1
		je @f
		cmp byte[bit_depth],2
		je @f
		cmp byte[bit_depth],4
		je @f
		cmp byte[bit_depth],8
		je @f
		jmp .def_1
		@@:
			mov byte[edi+png_struct.channels], 1
			jmp .end_s0
		.def_1: ;default
			png_error edi, 'Invalid bit depth for paletted image'
		jmp .end_s0
	.end_2:

	cmp ebx,PNG_COLOR_TYPE_GRAY_ALPHA
	jne .end_3
		cmp byte[bit_depth],8
		je @f ;if (..!=8)
		cmp byte[bit_depth],16
		je @f ;if (..!=16)
            png_error edi, 'Invalid bit depth for grayscale+alpha image'
		@@:
		mov byte[edi+png_struct.channels], 2
		jmp .end_s0
	.end_3:

	cmp ebx,PNG_COLOR_TYPE_RGB_ALPHA
	jne .end_4
		cmp byte[bit_depth],8
		je @f ;if (..!=8)
if PNG_WRITE_16BIT_SUPPORTED eq 1
		cmp byte[bit_depth],16
		je @f ;if (..!=16)
end if
			png_error edi, 'Invalid bit depth for RGBA image'
		@@:
		mov byte[edi+png_struct.channels], 4
		jmp .end_s0
	.end_4:

	;default:
		png_error edi, 'Invalid image color type specified'
	.end_s0:

	cmp byte[compression_type], PNG_COMPRESSION_TYPE_BASE
	je @f ;if (..!=..)
		png_warning edi, 'Invalid compression type specified'
		mov dword[compression_type], PNG_COMPRESSION_TYPE_BASE
	@@:

	; Write filter_method 64 (intrapixel differencing) only if
	; 1. Libpng was compiled with PNG_MNG_FEATURES_SUPPORTED and
	; 2. Libpng did not write a PNG signature (this filter_method is only
	;    used in PNG datastreams that are embedded in MNG datastreams) and
	; 3. The application called png_permit_mng_features with a mask that
	;    included PNG_FLAG_MNG_FILTER_64 and
	; 4. The filter_method is 64 and
	; 5. The color_type is RGB or RGBA

;   if (
if PNG_MNG_FEATURES_SUPPORTED eq 1
;       !((png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64) != 0 &&
;       ((png_ptr->mode & PNG_HAVE_PNG_SIGNATURE) == 0) &&
;       (color_type == PNG_COLOR_TYPE_RGB ||
;        color_type == PNG_COLOR_TYPE_RGB_ALPHA) &&
;       (filter_type == PNG_INTRAPIXEL_DIFFERENCING)) &&
end if
	cmp dword[filter_type],PNG_FILTER_TYPE_BASE
	je @f ;if (..!=..)
		png_warning edi, 'Invalid filter type specified'
		mov dword[filter_type], PNG_FILTER_TYPE_BASE
	@@:

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	cmp dword[interlace_type],PNG_INTERLACE_NONE
	je @f ;if (..!=..)
	cmp dword[interlace_type],PNG_INTERLACE_ADAM7
	je @f ;if (..!=..)
		png_warning edi, 'Invalid interlace type specified'
		mov dword[interlace_type], PNG_INTERLACE_ADAM7
	@@:
else
	mov dword[interlace_type], PNG_INTERLACE_NONE
end if

	; Save the relevant information
	mov al,byte[bit_depth]
	mov byte[edi+png_struct.bit_depth],al
	mov al,byte[color_type]
	mov byte[edi+png_struct.color_type],al
	mov al,byte[interlace_type]
	mov byte[edi+png_struct.interlaced],al
if PNG_MNG_FEATURES_SUPPORTED eq 1
	mov al,byte[filter_type]
	mov byte[edi+png_struct.filter_type],al
end if
	mov al,byte[compression_type]
	mov byte[edi+png_struct.compression_type],al
	mov eax,[width]
	mov [edi+png_struct.width],eax
	mov eax,[height]
	mov [edi+png_struct.height],eax

	movzx eax,byte[edi+png_struct.channels]
	imul ax,word[bit_depth]
	mov byte[edi+png_struct.pixel_depth],al
	PNG_ROWBYTES eax, [width]
	mov [edi+png_struct.rowbytes],eax
	; Set the usr info, so any transformations can modify it
	mov eax,[edi+png_struct.width]
	mov [edi+png_struct.usr_width],eax
	mov al,[edi+png_struct.bit_depth]
	mov [edi+png_struct.usr_bit_depth],al
	mov al,[edi+png_struct.channels]
	mov [edi+png_struct.usr_channels],al

	; Pack the header information into the buffer
	lea ebx,[ebp-13]
	stdcall png_save_uint_32, ebx, [width]
	add ebx,4
	stdcall png_save_uint_32, ebx, [height]
	add ebx,4
	mov al,byte[bit_depth]
	mov byte[ebx],al ;buf[8] = (byte)bit_depth
	inc ebx
	mov al,byte[color_type]
	mov byte[ebx],al ;buf[9] = (byte)color_type
	inc ebx
	mov al,byte[compression_type]
	mov byte[ebx],al ;buf[10] = (byte)compression_type
	inc ebx
	mov al,byte[filter_type]
	mov byte[ebx],al ;buf[11] = (byte)filter_type
	inc ebx
	mov al,byte[interlace_type]
	mov byte[ebx],al ;buf[12] = (byte)interlace_type
	sub ebx,12

	; Write the chunk
	stdcall png_write_complete_chunk, edi, png_IHDR, ebx, dword 13

	cmp byte[edi+png_struct.do_filter],PNG_NO_FILTERS
	jne .end_5 ;if (..==..)

	cmp byte[edi+png_struct.color_type],PNG_COLOR_TYPE_PALETTE
	je @f
	cmp byte[edi+png_struct.bit_depth],8
	jge .els_5 ;if ((..==..)||(..<..))
	@@:
		mov byte[edi+png_struct.do_filter], PNG_FILTER_NONE
		jmp .end_5
	.els_5: ;else
		mov byte[edi+png_struct.do_filter], PNG_ALL_FILTERS
	.end_5:

	mov dword[edi+png_struct.mode], PNG_HAVE_IHDR ;not READY_FOR_ZTXT
popad
	ret
endp

; Write the palette.  We are careful not to trust png_color to be in the
; correct order for PNG, so people can redefine it to any convenient
; structure.

;void (png_structrp png_ptr, png_const_colorp palette, uint_32 num_pal)
align 4
proc png_write_PLTE, png_ptr:dword, palette:dword, num_pal:dword
locals
	;max_palette_length dd ? ;uint_32
	i dd ?
	pal_ptr dd ? ;png_const_colorp
	buf rb 3 ;byte[3]
endl
	png_debug 1, 'in png_write_PLTE'

pushad
	mov edi,[png_ptr]
	movzx eax,byte[edi+png_struct.color_type]
	cmp eax,PNG_COLOR_TYPE_PALETTE
	je @f ;if (..==..)
		;mov dword[max_palette_length],PNG_MAX_PALETTE_LENGTH
		mov eax,PNG_MAX_PALETTE_LENGTH
		jmp .end0
	@@:
		mov cl,byte[edi+png_struct.bit_depth]
		xor eax,eax
		inc eax
		shl eax,cl
		;mov [max_palette_length],eax
	.end0:

if PNG_MNG_FEATURES_SUPPORTED eq 1
	cmp [num_pal],eax
	jg @f
	mov eax,[edi+png_struct.mng_features_permitted]
	and eax,PNG_FLAG_MNG_EMPTY_PLTE
	jnz .end1
	cmp [num_pal],0
	jne .end1
	@@:
end if

	cmp byte[edi+png_struct.color_type],PNG_COLOR_TYPE_PALETTE ;if (..==..)
	jne @f
		png_error edi, 'Invalid number of colors in palette'
		jmp .end1
	@@: ;else
		png_warning edi, 'Invalid number of colors in palette'
		jmp .end_f
	.end1:

	movzx eax,byte[edi+png_struct.color_type]
	and eax,PNG_COLOR_MASK_COLOR
	jnz @f ;if (..==0)
		png_warning edi, 'Ignoring request to write a PLTE chunk in grayscale PNG'
		jmp .end_f
	@@:

	movzx eax,word[num_pal]
	mov word[edi+png_struct.num_palette],ax
	png_debug1 3, 'num_palette = %d', eax

	imul eax,3
	stdcall png_write_chunk_header, edi, png_PLTE, eax
if PNG_POINTER_INDEXING_SUPPORTED eq 1

;   for (i = 0, pal_ptr = palette; i < num_pal; i++, pal_ptr++)
;   {
;      buf[0] = pal_ptr->red;
;      buf[1] = pal_ptr->green;
;      buf[2] = pal_ptr->blue;
;      png_write_chunk_data(png_ptr, buf, (png_size_t)3);
;   }

else
	; This is a little slower but some buggy compilers need to do this
	; instead

;   pal_ptr=palette;

;   for (i = 0; i < num_pal; i++)
;   {
;      buf[0] = pal_ptr[i].red;
;      buf[1] = pal_ptr[i].green;
;      buf[2] = pal_ptr[i].blue;
;      png_write_chunk_data(png_ptr, buf, (png_size_t)3);
;   }

end if
	stdcall png_write_chunk_end, edi
	or dword[edi+png_struct.mode], PNG_HAVE_PLTE
.end_f:
popad
	ret
endp

; This is similar to png_text_compress, above, except that it does not require
; all of the data at once and, instead of buffering the compressed result,
; writes it as IDAT chunks.  Unlike png_text_compress it *can* png_error out
; because it calls the write interface.  As a result it does its own error
; reporting and does not return an error code.  In the event of error it will
; just call png_error.  The input data length may exceed 32-bits.  The 'flush'
; parameter is exactly the same as that to deflate, with the following
; meanings:

; Z_NO_FLUSH: normal incremental output of compressed data
; Z_SYNC_FLUSH: do a SYNC_FLUSH, used by png_write_flush
; Z_FINISH: this is the end of the input, do a Z_FINISH and clean up

; The routine manages the acquire and release of the png_ptr->zstream by
; checking and (at the end) clearing png_ptr->zowner; it does some sanity
; checks on the 'mode' flags while doing this.

;void (png_structrp png_ptr, bytep input, png_alloc_size_t input_len, int flush)
;input:
; edi - png_ptr
align 4
proc png_compress_IDAT uses eax ebx ecx edx, input:dword, input_len:dword, flush:dword
	png_debug 1, 'in png_compress_IDAT'

	cmp dword[edi+png_struct.zowner],png_IDAT
	je .end0 ;if (..!=..)
		; First time.   Ensure we have a temporary buffer for compression and
		; trim the buffer list if it has more than one entry to free memory.
		; If 'WRITE_COMPRESSED_TEXT' is not set the list will never have been
		; created at this point, but the check here is quick and safe.

		cmp dword[edi+png_struct.zbuffer_list],0
		jne @f ;if (..==0)
			PNG_COMPRESSION_BUFFER_SIZE edi
			stdcall png_malloc, edi, eax
			mov [edi+png_struct.zbuffer_list],eax
			mov dword[eax+png_compression_buffer.next],0
			jmp .end1
		@@: ;else
			mov eax,[edi+png_struct.zbuffer_list]
			add eax,png_compression_buffer.next
			;eax = &...next
			stdcall png_free_buffer_list, edi, eax
		.end1:

		;It is a terminal error if we can't claim the zstream.
		stdcall png_image_size, edi
		stdcall png_deflate_claim, png_IDAT, eax
		cmp eax,Z_OK
		je @f ;if (..!=..)
			png_error edi, [edi+png_struct.zstream.msg]
		@@:

		; The output state is maintained in png_ptr->zstream, so it must be
		; initialized here after the claim.

		mov eax,[edi+png_struct.zbuffer_list]
		add eax,png_compression_buffer.output
		mov [edi+png_struct.zstream.next_out],eax
		mov eax,[edi+png_struct.zbuffer_size]
		mov [edi+png_struct.zstream.avail_out],eax
	.end0:

	; Now loop reading and writing until all the input is consumed or an error
	; terminates the operation.  The _out values are maintained across calls to
	; this function, but the input must be reset each time.

	mov eax,[input]
	mov [edi+png_struct.zstream.next_in],eax
	mov dword[edi+png_struct.zstream.avail_in],0 ;set below
align 4
	.cycle0:
	;INPUT: from the row data
		mov eax,ZLIB_IO_MAX

		cmp eax,[input_len]
		jbe @f ;if (..>..)
			mov eax,[input_len] ;safe because of the check
		@@:

		mov [edi+png_struct.zstream.avail_in],eax
		sub [input_len],eax

		mov eax,[flush]
		cmp dword[input_len],0
		jle @f
			mov eax,Z_NO_FLUSH
		@@:
		mov ecx,edi
		add ecx,png_struct.zstream
		stdcall [deflate], ecx, eax
		mov ebx,eax

		;Include as-yet unconsumed input
		mov eax,[edi+png_struct.zstream.avail_in]
		add [input_len],eax
		mov dword[edi+png_struct.zstream.avail_in],0

		; OUTPUT: write complete IDAT chunks when avail_out drops to zero. Note
		; that these two zstream fields are preserved across the calls, therefore
		; there is no need to set these up on entry to the loop.

		cmp dword[edi+png_struct.zstream.avail_out],0
		jne .end2 ;if (..==0)
			mov edx,[edi+png_struct.zbuffer_list]
			add edx,png_compression_buffer.output
			mov ecx,[edi+png_struct.zbuffer_size]
			;edx = data
			;ecx = size
			; Write an IDAT containing the data then reset the buffer.  The
			; first IDAT may need deflate header optimization.

if PNG_WRITE_OPTIMIZE_CMF_SUPPORTED eq 1
			mov eax,[edi+png_struct.mode]
			and eax,PNG_HAVE_IDAT
			jnz @f
			cmp byte[edi+png_struct.compression_type],PNG_COMPRESSION_TYPE_BASE
			jne @f ;if (..==0 && ..==..)
				stdcall png_image_size, edi
				stdcall optimize_cmf, edx, eax
			@@:
end if

			stdcall png_write_complete_chunk, edi, png_IDAT, edx, ecx
			or dword[edi+png_struct.mode],PNG_HAVE_IDAT

			mov [edi+png_struct.zstream.next_out],edx
			mov [edi+png_struct.zstream.avail_out],ecx

			; For SYNC_FLUSH or FINISH it is essential to keep calling zlib with
			; the same flush parameter until it has finished output, for NO_FLUSH
			; it doesn't matter.

			cmp ebx,Z_OK
			jne .end2
			cmp dword[flush],Z_NO_FLUSH
			jne .cycle0 ;if (..==.. && ..!=..) continue
		.end2:

		; The order of these checks doesn't matter much; it just affects which
		; possible error might be detected if multiple things go wrong at once.

		cmp ebx,Z_OK
		jne .end3 ;if (..==..) ;most likely return code!
			; If all the input has been consumed then just return.  If Z_FINISH
			; was used as the flush parameter something has gone wrong if we get
			; here.

			cmp dword[input_len],0
			jne .cycle0 ;if (..==0)
				cmp dword[flush],Z_FINISH
				jne .cycle0end ;if (..==..)
					png_error edi, 'Z_OK on Z_FINISH with output space'
				jmp .cycle0end
		.end3:
		cmp ebx,Z_STREAM_END
		jne .end4
		cmp dword[flush],Z_FINISH
		jne .end4 ;else if (..==.. && ..==..)
			; This is the end of the IDAT data; any pending output must be
			; flushed.  For small PNG files we may still be at the beginning.

			mov edx,[edi+png_struct.zbuffer_list]
			add edx,png_compression_buffer.output
			mov ecx,[edi+png_struct.zbuffer_size]
			mov eax,[edi+png_struct.zstream.avail_out]
			sub ecx,eax
			;edx = data
			;ecx = size

if PNG_WRITE_OPTIMIZE_CMF_SUPPORTED eq 1
			mov eax,[edi+png_struct.mode]
			and eax,PNG_HAVE_IDAT
			jnz @f
			cmp byte[edi+png_struct.compression_type],PNG_COMPRESSION_TYPE_BASE
			jne @f ;if (..==0 && ..==..)
				stdcall png_image_size, edi
				stdcall optimize_cmf, edx, eax
			@@:
end if
			stdcall png_write_complete_chunk, edi, png_IDAT, edx, ecx
			mov dword[edi+png_struct.zstream.avail_out],0
			mov dword[edi+png_struct.zstream.next_out],0
			or dword[edi+png_struct.mode], PNG_HAVE_IDAT or PNG_AFTER_IDAT

			mov dword[edi+png_struct.zowner],0 ;Release the stream
			jmp .cycle0end
		.end4: ;else
			; This is an error condition.
			stdcall png_zstream_error, edi, ebx
			png_error edi, [edi+png_struct.zstream.msg]
		jmp .cycle0
	.cycle0end:
	ret
endp

; Write an IEND chunk
;void (png_structrp png_ptr)
align 4
proc png_write_IEND uses edi, png_ptr:dword
	png_debug 1, 'in png_write_IEND'

	mov edi,[png_ptr]
	stdcall png_write_complete_chunk, edi, png_IEND, 0, 0
	or dword[edi+png_struct.mode], PNG_HAVE_IEND
	ret
endp

; Write a gAMA chunk
;void (png_structrp png_ptr, png_fixed_point file_gamma)
align 4
proc png_write_gAMA_fixed uses ebx, png_ptr:dword, file_gamma:dword
locals 
	buf rb 4 ;byte[4]
endl
	png_debug 1, 'in png_write_gAMA'

	; file_gamma is saved in 1/100,000ths
	lea ebx,[ebp-4]
	stdcall png_save_uint_32 ,ebx, [file_gamma]
	stdcall png_write_complete_chunk, [png_ptr], png_gAMA, ebx, 4
	ret
endp

; Write a sRGB chunk
;void (png_structrp png_ptr, int srgb_intent)
align 4
proc png_write_sRGB uses eax ebx, png_ptr:dword, srgb_intent:dword
locals
	buf db ? ;byte[1]
endl
	png_debug 1, 'in png_write_sRGB'

	cmp dword[srgb_intent], PNG_sRGB_INTENT_LAST ;if (..>=..)
	jl @f
		png_warning [png_ptr], 'Invalid sRGB rendering intent specified'
	@@:

	mov al,byte[srgb_intent]
	mov ebx,ebp
	dec ebx
	mov byte[ebx],al ;buf[0]=(byte)srgb_intent
	stdcall png_write_complete_chunk, [png_ptr], png_sRGB, ebx, 1
	ret
endp

; Write an iCCP chunk
;void (png_structrp png_ptr, charp name, bytep profile)
align 4
proc png_write_iCCP uses eax ebx ecx edi, png_ptr:dword, name:dword, profile:dword
locals
	name_len dd ? ;uint_32
	profile_len dd ? ;uint_32
	temp dd ? ;uint_32
	new_name rb 81 ;byte[81] ;1 byte for the compression byte
	comp compression_state
endl
	png_debug 1, 'in png_write_iCCP'

	; These are all internal problems: the profile should have been checked
	; before when it was stored.

	mov edi,[png_ptr]
	cmp dword[profile],0
	jne @f ;if (..==0)
		png_error edi, 'No profile for iCCP chunk' ;internal error
	@@:

	stdcall png_get_uint_32,[profile]
	mov [profile_len],eax

	cmp eax,132
	jge @f ;if (..<..)
		png_error edi, 'ICC profile too short'
	@@:

;   temp = (uint_32) (*(profile+8));
;   if (temp > 3 && (profile_len & 0x03))
;      png_error(png_ptr, "ICC profile length invalid (not a multiple of 4)");

;   {
;      uint_32 embedded_profile_len = png_get_uint_32(profile);

;      if (profile_len != embedded_profile_len)
;         png_error(png_ptr, "Profile length does not match profile");
;   }

	lea ebx,[ebp-sizeof.compression_state]
	mov ecx,ebx ;ecx = &comp
	sub ebx,81  ;ebx = &new_name
	stdcall png_check_keyword, edi, [name], ebx
	mov [name_len],eax

	test eax,eax
	jnz @f ;if (..==0)
		png_error edi, 'iCCP: invalid keyword'
	@@:

	inc dword[name_len]
	mov eax,[name_len]
	add eax,ebx
	mov byte[eax], PNG_COMPRESSION_TYPE_BASE

	; Make sure we include the NULL after the name and the compression type
	inc dword[name_len]

	stdcall png_text_compress_init, ecx, [profile], [profile_len]

	; Allow for keyword terminator and compression byte
;   if (png_text_compress(png_ptr, png_iCCP, &comp, name_len) != Z_OK)
;      png_error(png_ptr, png_ptr->zstream.msg);

;   png_write_chunk_header(png_ptr, png_iCCP, name_len + comp.output_len);

	stdcall png_write_chunk_data, edi, ebx, [name_len]

	stdcall png_write_compressed_data_out, edi, ecx

	stdcall png_write_chunk_end, edi
	ret
endp

; Write a sPLT chunk
;void (png_structrp png_ptr, png_const_sPLT_tp spalette)
align 4
proc png_write_sPLT, png_ptr:dword, spalette:dword
;   uint_32 name_len;
;   byte new_name[80];
;   byte entrybuf[10];
;   png_size_t entry_size = (spalette->depth == 8 ? 6 : 10);
;   png_size_t palette_size = entry_size * spalette->nentries;
;   png_sPLT_entryp ep;
if PNG_POINTER_INDEXING_SUPPORTED eq
;   int i;
end if

	png_debug 1, 'in png_write_sPLT'

;   name_len = png_check_keyword(png_ptr, spalette->name, new_name);

;   if (name_len == 0)
;      png_error(png_ptr, "sPLT: invalid keyword");

	; Make sure we include the NULL after the name
;   png_write_chunk_header(png_ptr, png_sPLT,
;       (uint_32)(name_len + 2 + palette_size));

;   png_write_chunk_data(png_ptr, (bytep)new_name,
;       (png_size_t)(name_len + 1));

;   png_write_chunk_data(png_ptr, &spalette->depth, (png_size_t)1);

	; Loop through each palette entry, writing appropriately
if PNG_POINTER_INDEXING_SUPPORTED eq 1
;   for (ep = spalette->entries; ep<spalette->entries + spalette->nentries; ep++)
;   {
;      if (spalette->depth == 8)
;      {
;         entrybuf[0] = (byte)ep->red;
;         entrybuf[1] = (byte)ep->green;
;         entrybuf[2] = (byte)ep->blue;
;         entrybuf[3] = (byte)ep->alpha;
;         png_save_uint_16(entrybuf + 4, ep->frequency);
;      }

;      else
;      {
;         png_save_uint_16(entrybuf + 0, ep->red);
;         png_save_uint_16(entrybuf + 2, ep->green);
;         png_save_uint_16(entrybuf + 4, ep->blue);
;         png_save_uint_16(entrybuf + 6, ep->alpha);
;         png_save_uint_16(entrybuf + 8, ep->frequency);
;      }

;      png_write_chunk_data(png_ptr, entrybuf, entry_size);
;   }
else
;   ep=spalette->entries;
;   for (i = 0; i>spalette->nentries; i++)
;   {
;      if (spalette->depth == 8)
;      {
;         entrybuf[0] = (byte)ep[i].red;
;         entrybuf[1] = (byte)ep[i].green;
;         entrybuf[2] = (byte)ep[i].blue;
;         entrybuf[3] = (byte)ep[i].alpha;
;         png_save_uint_16(entrybuf + 4, ep[i].frequency);
;      }

;      else
;      {
;         png_save_uint_16(entrybuf + 0, ep[i].red);
;         png_save_uint_16(entrybuf + 2, ep[i].green);
;         png_save_uint_16(entrybuf + 4, ep[i].blue);
;         png_save_uint_16(entrybuf + 6, ep[i].alpha);
;         png_save_uint_16(entrybuf + 8, ep[i].frequency);
;      }

;      png_write_chunk_data(png_ptr, entrybuf, entry_size);
;   }
end if

;   png_write_chunk_end(png_ptr);
	ret
endp

; Write the sBIT chunk
;void (png_structrp png_ptr, png_const_color_8p sbit, int color_type)
align 4
proc png_write_sBIT uses eax edi, png_ptr:dword, sbit:dword, color_type:dword
locals
	size dd ? ;png_size_t
	buf rb 4 ;byte[4]
endl
	png_debug 1, 'in png_write_sBIT'

	; Make sure we don't depend upon the order of PNG_COLOR_8
;   if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
;   {
;      byte maxbits;

;      maxbits = (byte)(color_type==PNG_COLOR_TYPE_PALETTE ? 8 :
;          png_ptr->usr_bit_depth);

;      if (sbit->red == 0 || sbit->red > maxbits ||
;          sbit->green == 0 || sbit->green > maxbits ||
;          sbit->blue == 0 || sbit->blue > maxbits)
;      {
;         png_warning(png_ptr, "Invalid sBIT depth specified");
;         return;
;      }

;      buf[0] = sbit->red;
;      buf[1] = sbit->green;
;      buf[2] = sbit->blue;
;      size = 3;
;   }

;   else
;   {
;      if (sbit->gray == 0 || sbit->gray > png_ptr->usr_bit_depth)
;      {
;         png_warning(png_ptr, "Invalid sBIT depth specified");
;         return;
;      }

;      buf[0] = sbit->gray;
;      size = 1;
;   }

;   if ((color_type & PNG_COLOR_MASK_ALPHA) != 0)
;   {
;      if (sbit->alpha == 0 || sbit->alpha > png_ptr->usr_bit_depth)
;      {
;         png_warning(png_ptr, "Invalid sBIT depth specified");
;         return;
;      }

;      buf[size++] = sbit->alpha;
;   }

;   png_write_complete_chunk(png_ptr, png_sBIT, buf, size);
.end_f:
	ret
endp

; Write the cHRM chunk
;void (png_structrp png_ptr, const png_xy *xy)
align 4
proc png_write_cHRM_fixed uses eax ebx, png_ptr:dword, xy:dword
locals
	buf rb 32 ;byte[32]
endl
	png_debug 1, 'in png_write_cHRM'

	; Each value is saved in 1/100,000ths
	mov eax,[xy]
	lea ebx,[ebp-32]
;   png_save_int_32(buf,      xy->whitex);
;   png_save_int_32(buf +  4, xy->whitey);

;   png_save_int_32(buf +  8, xy->redx);
;   png_save_int_32(buf + 12, xy->redy);

;   png_save_int_32(buf + 16, xy->greenx);
;   png_save_int_32(buf + 20, xy->greeny);

;   png_save_int_32(buf + 24, xy->bluex);
;   png_save_int_32(buf + 28, xy->bluey);

	stdcall png_write_complete_chunk, [png_ptr], png_cHRM, ebx, 32
	ret
endp

; Write the tRNS chunk
;void (png_structrp png_ptr, bytep trans_alpha, png_color_16p tran, int num_trans, int color_type)
align 4
proc png_write_tRNS uses eax ebx ecx edi, png_ptr:dword, trans_alpha:dword, tran:dword, num_trans:dword, color_type:dword
locals
	buf rb 6 ;byte[6]
endl
	png_debug 1, 'in png_write_tRNS'

	mov edi,[png_ptr]
	cmp byte[color_type],PNG_COLOR_TYPE_PALETTE
	jne .end0 ;if (..==..)
		cmp dword[num_trans],0
		jle @f
		movzx eax,word[edi+png_struct.num_palette]
		cmp [num_trans],eax
		jle .end1
		@@: ;if (..<=0 || ..>..)
			png_app_warning edi, 'Invalid number of transparent colors specified'
			jmp .end_f
		.end1:

		; Write the chunk out as it is
		stdcall png_write_complete_chunk, edi, png_tRNS, [trans_alpha], [num_trans]
		jmp .end_f
	.end0:

	cmp dword[color_type],PNG_COLOR_TYPE_GRAY
	jne .end2 ;else if (..==..)
		; One 16-bit value
		mov cl,[edi+png_struct.bit_depth]
		xor eax,eax
		inc eax
		shl eax,cl
		mov ecx,[tran]
		cmp word[ecx+png_color_16.gray],ax
		jl @f ;if (..>=..)
			png_app_warning edi, 'Ignoring attempt to write tRNS chunk out-of-range for bit_depth'
			jmp .end_f
		@@:
		movzx eax,word[ecx+png_color_16.gray]
		lea ebx,[ebp-6]
		stdcall png_save_uint_16, ebx, eax
		stdcall png_write_complete_chunk, edi, png_tRNS, ebx, 2
		jmp .end_f
	.end2:

	cmp dword[color_type],PNG_COLOR_TYPE_RGB
	jne .end3 ;else if (..== ..)
		; Three 16-bit values
		lea ebx,[ebp-6]
		mov ecx,[tran]
		movzx eax,word[ecx+png_color_16.red]
		stdcall png_save_uint_16, ebx, eax
		add ebx,2
		movzx eax,word[ecx+png_color_16.green]
		stdcall png_save_uint_16, ebx, eax
		add ebx,2
		movzx eax,word[ecx+png_color_16.blue]
		stdcall png_save_uint_16, ebx, eax
		sub ebx,4
if PNG_WRITE_16BIT_SUPPORTED eq 1
		cmp byte[edi+png_struct.bit_depth],8
		jne @f ;if (..==.. && ...
end if
		mov al,[ebx]
		or al,[ebx+2]
		or al,[ebx+4]
		or al,al
		jz @f ;if (..|..|..!=0)
			png_app_warning edi, 'Ignoring attempt to write 16-bit tRNS chunk when bit_depth is 8'
			jmp .end_f
		@@:
		stdcall png_write_complete_chunk, edi, png_tRNS, ebx, 6
		jmp .end_f
	.end3: ;else
		cStr ,<'Can',39,'t write tRNS with an alpha channel'>
		png_app_warning edi, eax
.end_f:
	ret
endp

; Write the background chunk
;void (png_structrp png_ptr, png_const_color_16p back, int color_type)
align 4
proc png_write_bKGD, png_ptr:dword, back:dword, color_type:dword
locals
	buf rb 6 ;byte[6]
endl
	png_debug 1, 'in png_write_bKGD'

;   if (color_type == PNG_COLOR_TYPE_PALETTE)
;   {
;      if (
if PNG_MNG_FEATURES_SUPPORTED eq 1
;          (png_ptr->num_palette != 0 ||
;          (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0) &&
end if
;         back->index >= png_ptr->num_palette)
;      {
;         png_warning(png_ptr, "Invalid background palette index");
;         return;
;      }

;      buf[0] = back->index;
;      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)1);
;   }

;   else if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
;   {
;      png_save_uint_16(buf, back->red);
;      png_save_uint_16(buf + 2, back->green);
;      png_save_uint_16(buf + 4, back->blue);
if PNG_WRITE_16BIT_SUPPORTED eq 1
;      if (png_ptr->bit_depth == 8 && (buf[0] | buf[2] | buf[4]) != 0)
else
;      if ((buf[0] | buf[2] | buf[4]) != 0)
end if
;      {
;         png_warning(png_ptr,
;             "Ignoring attempt to write 16-bit bKGD chunk when bit_depth is 8");

;         return;
;      }

;      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)6);
;   }

;   else
;   {
;      if (back->gray >= (1 << png_ptr->bit_depth))
;      {
;         png_warning(png_ptr,
;             "Ignoring attempt to write bKGD chunk out-of-range for bit_depth");

;         return;
;      }

;      png_save_uint_16(buf, back->gray);
;      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)2);
;   }
	ret
endp

; Write the histogram
;void (png_structrp png_ptr, png_const_uint_16p hist, int num_hist)
align 4
proc png_write_hIST, png_ptr:dword, hist:dword, num_hist:dword
locals
	i dd ? ;int
	buf rb 3 ;byte[3]
endl
	png_debug 1, 'in png_write_hIST'
pushad

	mov edi,[png_ptr]
	movzx eax,word[edi+png_struct.num_palette]
	cmp [num_hist],eax
	jle @f ;if (..>..)
;      png_debug2(3, "num_hist = %d, num_palette = %d", num_hist,
;          png_ptr->num_palette);

		png_warning edi, 'Invalid number of histogram entries specified'
		jmp .end_f
	@@:

	mov eax,[num_hist]
	shl eax,1
	stdcall png_write_chunk_header, edi, png_hIST, eax

;   for (i = 0; i < num_hist; i++)
;   {
;      png_save_uint_16(buf, hist[i]);
;      png_write_chunk_data(png_ptr, buf, (png_size_t)2);
;   }

	stdcall png_write_chunk_end, edi
.end_f:
popad
	ret
endp

; Write a tEXt chunk
;void (png_structrp png_ptr, charp key, charp text, png_size_t text_len)
align 4
proc png_write_tEXt uses eax edi, png_ptr:dword, key:dword, text:dword, text_len:dword
locals
	key_len dd ? ;uint_32
	new_key rb 80 ;byte[80]
endl
	png_debug 1, 'in png_write_tEXt'

;   key_len = png_check_keyword(png_ptr, key, new_key);

;   if (key_len == 0)
;      png_error(png_ptr, "tEXt: invalid keyword");

;   if (text == NULL || *text == '\0')
;      text_len = 0;

;   else
;      text_len = strlen(text);

;   if (text_len > PNG_UINT_31_MAX - (key_len+1))
;      png_error(png_ptr, "tEXt: text too long");

	; Make sure we include the 0 after the key
;   png_write_chunk_header(png_ptr, png_tEXt,
;       (uint_32)/*checked above*/(key_len + text_len + 1));

	; We leave it to the application to meet PNG-1.0 requirements on the
	; contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
	; any non-Latin-1 characters except for NEWLINE.  ISO PNG will forbid them.
	; The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.

;   png_write_chunk_data(png_ptr, new_key, key_len + 1);

;   if (text_len != 0)
;      png_write_chunk_data(png_ptr, (bytep)text, text_len);

;   png_write_chunk_end(png_ptr);
	ret
endp

if PNG_WRITE_zTXt_SUPPORTED eq 1
; Write a compressed text chunk
;void (png_structrp png_ptr, charp key, charp text, int compression)
align 4
proc png_write_zTXt uses eax edi, png_ptr:dword, key:dword, text:dword, compression:dword
locals
	key_len dd ? ;uint_32
	new_key rb 81 ;byte[81]
	comp compression_state
endl
	png_debug 1, 'in png_write_zTXt'

	mov edi,[png_ptr]
	cmp dword[compression],PNG_TEXT_COMPRESSION_NONE
	jne @f ;if (..==..)
		stdcall png_write_tEXt, edi, [key], [text], 0
		jmp .end_f
	@@:

;   if (compression != PNG_TEXT_COMPRESSION_zTXt)
;      png_error(png_ptr, "zTXt: invalid compression type");

;   key_len = png_check_keyword(png_ptr, key, new_key);

;   if (key_len == 0)
;      png_error(png_ptr, "zTXt: invalid keyword");

	; Add the compression method and 1 for the keyword separator.
;   new_key[++key_len] = PNG_COMPRESSION_TYPE_BASE;
;   ++key_len;

	; Compute the compressed data; do it now for the length
;   png_text_compress_init(&comp, (bytep)text,
;       text == NULL ? 0 : strlen(text));

;   if (png_text_compress(png_ptr, png_zTXt, &comp, key_len) != Z_OK)
;      png_error(png_ptr, png_ptr->zstream.msg);

	; Write start of chunk
;   png_write_chunk_header(png_ptr, png_zTXt, key_len + comp.output_len);

	; Write key
;   png_write_chunk_data(png_ptr, new_key, key_len);

	; Write the compressed data
;   png_write_compressed_data_out(png_ptr, &comp);

	; Close the chunk
	stdcall png_write_chunk_end, edi
.end_f:
	ret
endp
end if

if PNG_WRITE_iTXt_SUPPORTED eq 1
; Write an iTXt chunk
;void (png_structrp png_ptr, int compression, charp key,
;    charp lang, charp lang_key, charp text)
align 4
proc png_write_iTXt, png_ptr:dword, compression:dword, key:dword, lang:dword, lang_key:dword, text:dword
locals
	key_len dd ? ;uint_32
	prefix_len dd ?
	;png_size_t lang_len, lang_key_len;
	new_key rb 82 ;byte[82]
	comp compression_state
endl

	png_debug 1, 'in png_write_iTXt'
pushad
	mov edi,[png_ptr]
	lea ebx,[ebp-(82+sizeof.compression_state)]
	stdcall png_check_keyword, edi, [key], ebx
	mov [key_len],eax

	test eax,eax
	jnz @f ;if (..==0)
		png_error edi, 'iTXt: invalid keyword'
	@@:

	; Set the compression flag
;   switch (compression)
;   {
;      case PNG_ITXT_COMPRESSION_NONE:
;      case PNG_TEXT_COMPRESSION_NONE:
;         compression = new_key[++key_len] = 0; /* no compression */
;         break;

;      case PNG_TEXT_COMPRESSION_zTXt:
;      case PNG_ITXT_COMPRESSION_zTXt:
;         compression = new_key[++key_len] = 1; /* compressed */
;         break;

;      default:
;         png_error(png_ptr, "iTXt: invalid compression");
;   }

;   new_key[++key_len] = PNG_COMPRESSION_TYPE_BASE;
;   ++key_len; /* for the keywod separator */

	; We leave it to the application to meet PNG-1.0 requirements on the
	; contents of the text.  PNG-1.0 through PNG-1.2 discourage the use of
	; any non-Latin-1 characters except for NEWLINE.  ISO PNG, however,
	; specifies that the text is UTF-8 and this really doesn't require any
	; checking.

	; The NUL character is forbidden by PNG-1.0 through PNG-1.2 and ISO PNG.

	; TODO: validate the language tag correctly (see the spec.)

;   if (lang == NULL) lang = ""; /* empty language is valid */
;   lang_len = strlen(lang)+1;
;   if (lang_key == NULL) lang_key = ""; /* may be empty */
;   lang_key_len = strlen(lang_key)+1;
;   if (text == NULL) text = ""; /* may be empty */

	mov eax,[key_len]
	mov [prefix_len],eax
;   if (lang_len > PNG_UINT_31_MAX-prefix_len)
;      prefix_len = PNG_UINT_31_MAX;
;   else
;      prefix_len = (uint_32)(prefix_len + lang_len);

;   if (lang_key_len > PNG_UINT_31_MAX-prefix_len)
;      prefix_len = PNG_UINT_31_MAX;
;   else
;      prefix_len = (uint_32)(prefix_len + lang_key_len);

;   png_text_compress_init(&comp, (bytep)text, strlen(text));

;   if (compression != 0)
;   {
;      if (png_text_compress(png_ptr, png_iTXt, &comp, prefix_len) != Z_OK)
;         png_error(png_ptr, png_ptr->zstream.msg);
;   }

;   else
;   {
;      if (comp.input_len > PNG_UINT_31_MAX-prefix_len)
;         png_error(png_ptr, "iTXt: uncompressed text too long");

	; So the string will fit in a chunk:
;      comp.output_len = (uint_32)/*SAFE*/comp.input_len;
;   }

;   png_write_chunk_header(png_ptr, png_iTXt, comp.output_len + prefix_len);

;   png_write_chunk_data(png_ptr, new_key, key_len);

;   png_write_chunk_data(png_ptr, (bytep)lang, lang_len);

;   png_write_chunk_data(png_ptr, (bytep)lang_key, lang_key_len);

;   if (compression != 0)
;      png_write_compressed_data_out(png_ptr, &comp);

;   else
;      png_write_chunk_data(png_ptr, (bytep)text, comp.output_len);

	stdcall png_write_chunk_end, edi
popad
	ret
endp
end if

; Write the oFFs chunk
;void (png_structrp png_ptr, int_32 x_offset, int_32 y_offset, int unit_type)
align 4
proc png_write_oFFs uses eax ebx edi, png_ptr:dword, x_offset:dword, y_offset:dword, unit_type:dword
locals
	buf rb 9 ;byte[9]
endl
	png_debug 1, 'in png_write_oFFs'

	mov edi,[png_ptr]
	cmp dword[unit_type],PNG_OFFSET_LAST
	jl @f ;if (..>=..)
		png_warning edi, 'Unrecognized unit type for oFFs chunk'
	@@:

	lea ebx,[ebp-9]
	stdcall png_save_int_32, ebx, [x_offset]
	add ebx,4
	stdcall png_save_int_32, ebx, [y_offset]
	add ebx,4
	mov eax,[unit_type]
	mov [ebx],al
	sub ebx,8

	stdcall png_write_complete_chunk, edi, png_oFFs, ebx, 9
	ret
endp

; Write the pCAL chunk (described in the PNG extensions document)
;void (png_structrp png_ptr, charp purpose, int_32 X0,
;    int_32 X1, int type, int nparams, charp units, charpp params)
align 4
proc png_write_pCAL, png_ptr:dword, purpose:dword, X0:dword, X1:dword, type:dword,\
	nparams:dword, units:dword, params:dword
locals
	purpose_len dd ? ;uint_32
	units_len dd ?
	total_len dd ? ;png_size_t
	params_len dd ? ;png_size_tp
	buf rb 10 ;byte[10]
	new_purpose rb 80 ;byte[80]
	i dd ? ;int
endl
pushad
	png_debug1 1, 'in png_write_pCAL (%d parameters)', [nparams]
	mov edi,[png_ptr]

	cmp dword[type],PNG_EQUATION_LAST
	jl @f ;if (..>=..)
		png_error edi, 'Unrecognized equation type for pCAL chunk'
	@@:

	lea ebx,[ebp-84] ;ebx = &new_purpose
	stdcall png_check_keyword, edi, [purpose], ebx
	mov [purpose_len],eax

	test eax,eax
	jnz @f ;if(..==0)
		png_error edi, 'pCAL: invalid keyword'
	@@:

	inc dword[purpose_len] ; terminator

	png_debug1 3, 'pCAL purpose length = %d', [purpose_len]
;   units_len = strlen(units) + (nparams == 0 ? 0 : 1);
	png_debug1 3, 'pCAL units length = %d', [units_len]
;   total_len = purpose_len + units_len + 10;

;   params_len = (png_size_tp)png_malloc(png_ptr,
;       (png_alloc_size_t)(nparams * (sizeof (png_size_t))));

	; Find the length of each parameter, making sure we don't count the
	; null terminator for the last parameter.

;   for (i = 0; i < nparams; i++)
;   {
;      params_len[i] = strlen(params[i]) + (i == nparams - 1 ? 0 : 1);
;      png_debug2(3, "pCAL parameter %d length = %lu", i,
;          (unsigned long)params_len[i]);
;      total_len += params_len[i];
;   }

	png_debug1 3, 'pCAL total length = %d', [total_len]
	stdcall png_write_chunk_header, edi, png_pCAL, [total_len]
	stdcall png_write_chunk_data, edi, ebx, [purpose_len]
	lea ebx,[ebp-94] ;ebx = &buf
	stdcall png_save_int_32, ebx, [X0]
	add ebx,4
	stdcall png_save_int_32, ebx, [X1]
	add ebx,4
	mov eax,[type]
	mov [ebx],al
	inc ebx
	mov eax,[nparams]
	mov [ebx],al
	sub ebx,9
	stdcall png_write_chunk_data, edi, ebx, 10
	stdcall png_write_chunk_data, edi, [units], [units_len]

;   for (i = 0; i < nparams; i++)
;   {
;      png_write_chunk_data(png_ptr, (bytep)params[i], params_len[i]);
;   }

	stdcall png_free, edi, [params_len]
	stdcall png_write_chunk_end, edi
popad
	ret
endp

; Write the sCAL chunk
;void (png_structrp png_ptr, int unit, charp width, charp height)
align 4
proc png_write_sCAL_s uses eax ebx ecx edi esi, png_ptr:dword, unit:dword, width:dword, height:dword
locals
	total_len dd 2
	wlen dd ?
	hlen dd ?
	buf rb 64 ;byte[64]
endl
	png_debug 1, 'in png_write_sCAL_s'

	stdcall strlen,[width]
	add [total_len],eax
	mov [wlen],eax
	stdcall strlen,[height]
	add [total_len],eax
	mov [hlen],eax

	cmp dword[total_len],64
	jle @f ;if (..>..)
		cStr ,<'Can',39,'t write sCAL (buffer too small)'>
		png_warning [png_ptr], eax
		jmp .end_f
	@@:

	lea ebx,[ebp-64]
	mov eax,[unit]
	mov byte[ebx],al
	mov ecx,[wlen]
	inc ecx
	mov edi,ebx
	inc edi
	mov esi,[width]
	rep movsb ;Append the '\0' here
	mov ecx,[hlen]
	mov esi,[height]
	rep movsb ;Do NOT append the '\0' here

	png_debug1 3, 'sCAL total length = %u', [total_len]
	stdcall png_write_complete_chunk, [png_ptr], png_sCAL, ebx, [total_len]
.end_f:
	ret
endp

; Write the pHYs chunk
;void (png_structrp png_ptr, uint_32 x_pixels_per_unit,
;    uint_32 y_pixels_per_unit, int unit_type)
align 4
proc png_write_pHYs uses eax ebx, png_ptr:dword, x_pixels_per_unit:dword, y_pixels_per_unit:dword, unit_type:dword
locals
	buf rb 9 ;byte[9]
endl
	png_debug 1, 'in png_write_pHYs'

	cmp dword[unit_type],PNG_RESOLUTION_LAST
	jl @f ;if (..>=..)
		png_warning [png_ptr], 'Unrecognized unit type for pHYs chunk'
	@@:

	lea ebx,[ebp-9]
	stdcall png_save_uint_32, ebx, [x_pixels_per_unit]
	add ebx,4
	stdcall png_save_uint_32, ebx, [y_pixels_per_unit]
	add ebx,4
	mov al,byte[unit_type]
	mov byte[ebx],al
	sub ebx,8

	stdcall png_write_complete_chunk, [png_ptr], png_pHYs, ebx, 9
	ret
endp

; Write the tIME chunk.  Use either png_convert_from_struct_tm()
; or png_convert_from_time_t(), or fill in the structure yourself.

;void (png_structrp png_ptr, png_const_timep mod_time)
align 4
proc png_write_tIME uses eax ebx ecx, png_ptr:dword, mod_time:dword
locals
	buf rb 7 ;byte[7]
endl
	png_debug 1, 'in png_write_tIME'

	mov eax,[mod_time]
	mov cl,[eax+png_time.month]
	cmp cl,12
	jg @f
	cmp cl,1
	jl @f
	mov ch,[eax+png_time.day]
	cmp ch,31
	jg @f
	cmp ch,1
	jl @f
	cmp byte[eax+png_time.hour],23
	jg @f
	cmp byte[eax+png_time.second],60
	jg @f
		jmp .end0
	@@:
		png_warning [png_ptr], 'Invalid time specified for tIME chunk'
		jmp .end_f
	.end0:

	movzx ebx,word[eax+png_time.year]
	push ebx
	lea ebx,[ebp-7]
	stdcall png_save_uint_16, ebx ;, year
	add ebx,2
	mov byte[ebx],cl ;month
	inc ebx
	mov byte[ebx],ch ;day
	inc ebx
	mov cl,[eax+png_time.hour]
	mov byte[ebx],cl ;hour
	inc ebx
	mov cl,[eax+png_time.minute]
	mov byte[ebx],cl ;minute
	inc ebx
	mov cl,[eax+png_time.second]
	mov byte[ebx],cl ;second
	sub ebx,6

	stdcall png_write_complete_chunk, [png_ptr], png_tIME, ebx, 7
.end_f:
	ret
endp

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	; Arrays to facilitate easy interlacing - use pass (0 - 6) as index

	; Start of interlace block
	png_pass_start db 0, 4, 0, 2, 0, 1, 0
	; Offset to next interlace block
	png_pass_inc db 8, 8, 4, 4, 2, 2, 1
	; Start of interlace block in the y direction
	png_pass_ystart db 0, 0, 4, 0, 2, 0, 1
	; Offset to next interlace block in the y direction
	png_pass_yinc db 8, 8, 8, 4, 4, 2, 2
end if

; Initializes the row writing capability of libpng
;void (png_structrp png_ptr)
align 4
proc png_write_start_row uses eax ebx ecx edx edi, png_ptr:dword
locals
	buf_size dd ? ;png_alloc_size_t
	usr_pixel_depth dd ? ;int
if PNG_WRITE_FILTER_SUPPORTED eq 1
	filters db ? ;byte
end if
endl
	png_debug 1, 'in png_write_start_row'

	mov edi,[png_ptr]
	movzx eax,byte[edi+png_struct.usr_channels]
	movzx ebx,byte[edi+png_struct.usr_bit_depth]
	imul eax,ebx
	mov [usr_pixel_depth],eax
	PNG_ROWBYTES eax,[edi+png_struct.width]
	inc eax
	mov [buf_size],eax

	; 1.5.6: added to allow checking in the row write code.
	mov al,[edi+png_struct.pixel_depth]
	mov [edi+png_struct.transformed_pixel_depth],al

	mov eax,[usr_pixel_depth]
	mov [edi+png_struct.maximum_pixel_depth],al

	; Set up row buffer
	stdcall png_malloc, edi, [buf_size]
	mov [edi+png_struct.row_buf],eax

	mov byte[eax],PNG_FILTER_VALUE_NONE

if PNG_WRITE_FILTER_SUPPORTED eq 1
	mov al,byte[edi+png_struct.do_filter]

	cmp dword[edi+png_struct.height],1
	jne @f ;if (..==1)
		and al, 0xff and not(PNG_FILTER_UP or PNG_FILTER_AVG or PNG_FILTER_PAETH)
	@@:
	cmp dword[edi+png_struct.width],1
	jne @f ;if (..==1)
		and al, 0xff and not(PNG_FILTER_SUB or PNG_FILTER_AVG or PNG_FILTER_PAETH)
	@@:

	cmp al,0
	jne @f ;if (..==0)
		mov al,PNG_FILTER_NONE
	@@:

	mov [filters],al
	mov byte[edi+png_struct.do_filter],al

	;mov al,[filters]
	and al,PNG_FILTER_SUB or PNG_FILTER_UP or PNG_FILTER_AVG or PNG_FILTER_PAETH
	cmp al,0
	je .end0
	cmp dword[edi+png_struct.try_row],0
	jne .end0 ;if (..!=0) && ..==0)
		xor ebx,ebx

		stdcall png_malloc, edi, [buf_size]
		mov dword[edi+png_struct.try_row],eax

		mov al,[filters]
		and al,PNG_FILTER_SUB
		cmp al,0
		je @f
			inc ebx
		@@:
		mov al,[filters]
		and al,PNG_FILTER_UP
		cmp al,0
		je @f
			inc ebx
		@@:
		mov al,[filters]
		and al,PNG_FILTER_AVG
		cmp al,0
		je @f
			inc ebx
		@@:
		mov al,[filters]
		and al,PNG_FILTER_PAETH
		cmp al,0
		je @f
			inc ebx
		@@:
		cmp ebx,1
		jle .end0 ;if (..>1)
			stdcall png_malloc, edi, [buf_size]
			mov dword[edi+png_struct.tst_row],eax
	.end0:

	; We only need to keep the previous row if we are using one of the following
	; filters.

	mov al,[filters]
	and al,PNG_FILTER_AVG or PNG_FILTER_UP or PNG_FILTER_PAETH
	cmp al,0
	je @f ;if (..!=0)
		stdcall png_calloc, edi, [buf_size]
		mov dword[edi+png_struct.prev_row],eax
	@@:
end if ;WRITE_FILTER

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	; If interlaced, we need to set up width and height of pass
	cmp byte[edi+png_struct.interlaced],0
	je @f
	mov eax,[edi+png_struct.transformations]
	and eax,PNG_INTERLACE
	jnz @f ;if(..!=0 && ..==0)
		movzx ecx,byte[png_pass_yinc]
		mov eax,[edi+png_struct.height]
		add eax,ecx
		dec eax
		movzx edx,byte[png_pass_ystart]
		sub eax,edx
		xor edx,edx
		div ecx
		mov [edi+png_struct.num_rows],eax

		movzx ecx,byte[png_pass_inc]
		mov eax,[edi+png_struct.width]
		add eax,ecx
		dec eax
		movzx edx,byte[png_pass_start]
		sub eax,edx
		xor edx,edx
		div ecx
		mov [edi+png_struct.usr_width],eax
		jmp .end1
	@@: ;else
end if
		mov eax,[edi+png_struct.height]
		mov [edi+png_struct.num_rows],eax
		mov eax,[edi+png_struct.width]
		mov [edi+png_struct.usr_width],eax
	.end1:
	ret
endp

; Internal use only.  Called when finished processing a row of data.
;void (png_structrp png_ptr)
align 4
proc png_write_finish_row uses eax ecx edx edi, png_ptr:dword
	png_debug 1, 'in png_write_finish_row'

	mov edi,[png_ptr]
	; Next row
	inc dword[edi+png_struct.row_number]

	; See if we are done
	mov eax,[edi+png_struct.row_number]
	cmp eax,[edi+png_struct.num_rows]
	jl .end_f ;if (..<..) return

if PNG_WRITE_INTERLACING_SUPPORTED eq 1
	; If interlaced, go to next pass
	cmp byte[edi+png_struct.interlaced],0
	je .end0 ;if (..!=0)
		mov dword[edi+png_struct.row_number],0
		mov eax,[edi+png_struct.transformations]
		and eax,PNG_INTERLACE
		jz @f ;if (..!=0)
			inc byte[edi+png_struct.pass]
			jmp .end1
		@@: ;else
			; Loop until we find a non-zero width or height pass
			.cycle0: ;do
				inc byte[edi+png_struct.pass]
				cmp byte[edi+png_struct.pass],7
				jge .cycle0end ;if (..>=..) break

				movzx ecx,byte[edi+png_struct.pass]
				add ecx,png_pass_inc
				movzx ecx,byte[ecx]
				mov eax,[edi+png_struct.width]
				add eax,ecx
				dec eax
				movzx edx,byte[edi+png_struct.pass]
				add edx,png_pass_start
				movzx edx,byte[edx]
				sub eax,edx
				xor edx,edx
				div ecx
				mov [edi+png_struct.usr_width],eax

				movzx ecx,byte[edi+png_struct.pass]
				add ecx,png_pass_yinc
				movzx ecx,byte[ecx]
				mov eax,[edi+png_struct.height]
				add eax,ecx
				dec eax
				movzx edx,byte[edi+png_struct.pass]
				add edx,png_pass_ystart
				movzx edx,byte[edx]
				sub eax,edx
				xor edx,edx
				div ecx
				mov [edi+png_struct.num_rows],eax

				mov eax,[edi+png_struct.transformations]
				and eax,PNG_INTERLACE
				jnz .cycle0end ;if(..!=0) break

				cmp dword[edi+png_struct.usr_width],0
				je .cycle0
				cmp dword[edi+png_struct.num_rows],0
				je .cycle0
			.cycle0end: ;while (..==0 || ..==0)
		.end1:

		; Reset the row above the image for the next pass
		cmp byte[edi+png_struct.pass],7
		jge .end0 ;if (..<..)
			cmp dword[edi+png_struct.prev_row],0
			je .end_f ;if (..!=0)
				movzx eax,byte[edi+png_struct.usr_channels]
				movzx edx,byte[edi+png_struct.usr_bit_depth]
				imul eax,edx
				PNG_ROWBYTES eax, [edi+png_struct.width]
				inc eax
				push edi
				mov ecx,eax
				xor eax,eax
				mov edi,[edi+png_struct.prev_row]
				rep stosb ;memset(...
				pop edi
			jmp .end_f
	.end0:
end if

	; If we get here, we've just written the last row, so we need
	; to flush the compressor
	stdcall png_compress_IDAT, 0, 0, Z_FINISH
.end_f:
	ret
endp

; Pick out the correct pixels for the interlace pass.
; The basic idea here is to go through the row with a source
; pointer and a destination pointer (sp and dp), and copy the
; correct pixels for the pass.  As the row gets compacted,
; sp will always be >= dp, so we should never overwrite anything.
; See the default: case for the easiest code to understand.

;void (png_row_infop row_info, bytep row, int pass)
align 4
proc png_do_write_interlace, row_info:dword, row:dword, pass:dword
	png_debug 1, 'in png_do_write_interlace'

	; We don't have to do anything on the last pass (6)
	cmp dword[pass],6
	jge .end_f ;if (..<..)
	; Each pixel depth is handled separately
;      switch (row_info->pixel_depth)
;      {
;         case 1:
;         {
;            bytep sp;
;            bytep dp;
;            unsigned int shift;
;            int d;
;            int value;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            dp = row;
;            d = 0;
;            shift = 7;

;            for (i = png_pass_start[pass]; i < row_width;
;               i += png_pass_inc[pass])
;            {
;               sp = row + (png_size_t)(i >> 3);
;               value = (int)(*sp >> (7 - (int)(i & 0x07))) & 0x01;
;               d |= (value << shift);

;               if (shift == 0)
;               {
;                  shift = 7;
;                  *dp++ = (byte)d;
;                  d = 0;
;               }

;               else
;                  shift--;

;            }
;            if (shift != 7)
;               *dp = (byte)d;

;            break;
;         }

;         case 2:
;         {
;            bytep sp;
;            bytep dp;
;            unsigned int shift;
;            int d;
;            int value;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            dp = row;
;            shift = 6;
;            d = 0;

;            for (i = png_pass_start[pass]; i < row_width;
;               i += png_pass_inc[pass])
;            {
;               sp = row + (png_size_t)(i >> 2);
;               value = (*sp >> ((3 - (int)(i & 0x03)) << 1)) & 0x03;
;               d |= (value << shift);

;               if (shift == 0)
;               {
;                  shift = 6;
;                  *dp++ = (byte)d;
;                  d = 0;
;               }

;               else
;                  shift -= 2;
;            }
;            if (shift != 6)
;               *dp = (byte)d;

;            break;
;         }

;         case 4:
;         {
;            bytep sp;
;            bytep dp;
;            unsigned int shift;
;            int d;
;            int value;
;            uint_32 i;
;            uint_32 row_width = row_info->width;

;            dp = row;
;            shift = 4;
;            d = 0;
;            for (i = png_pass_start[pass]; i < row_width;
;                i += png_pass_inc[pass])
;            {
;               sp = row + (png_size_t)(i >> 1);
;               value = (*sp >> ((1 - (int)(i & 0x01)) << 2)) & 0x0f;
;               d |= (value << shift);

;               if (shift == 0)
;               {
;                  shift = 4;
;                  *dp++ = (byte)d;
;                  d = 0;
;               }

;               else
;                  shift -= 4;
;            }
;            if (shift != 4)
;               *dp = (byte)d;

;            break;
;         }

;         default:
;         {
;            bytep sp;
;            bytep dp;
;            uint_32 i;
;            uint_32 row_width = row_info->width;
;            png_size_t pixel_bytes;

	; Start at the beginning
;            dp = row;

	; Find out how many bytes each pixel takes up
;            pixel_bytes = (row_info->pixel_depth >> 3);

	; Loop through the row, only looking at the pixels that matter
;            for (i = png_pass_start[pass]; i < row_width;
;               i += png_pass_inc[pass])
;            {
	; Find out where the original pixel is
;               sp = row + (png_size_t)i * pixel_bytes;

	; Move the pixel
;               if (dp != sp)
;                  memcpy(dp, sp, pixel_bytes);

	; Next pixel
;               dp += pixel_bytes;
;            }
;            break;
;         }
;      }
	; Set new row width
;      row_info->width = (row_info->width +
;          png_pass_inc[pass] - 1 -
;          png_pass_start[pass]) /
;          png_pass_inc[pass];

;      row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth,
;          row_info->width);
.end_f:
	ret
endp

; This filters the row, chooses which filter to use, if it has not already
; been specified by the application, and then writes the row out with the
; chosen filter.

;void png_write_filtered_row(png_structrp png_ptr, bytep filtered_row,
;    png_size_t row_bytes);

;png_size_t (png_structrp png_ptr, const uint_32 bpp, const png_size_t row_bytes, const png_size_t lmins)
align 4
proc png_setup_sub_row uses ebx ecx edx edi esi, png_ptr:dword, bpp:dword, row_bytes:dword, lmins:dword
	mov ebx,[png_ptr]
	mov edi,[ebx+png_struct.try_row]
	mov byte[edi],PNG_FILTER_VALUE_SUB

	mov ecx,[bpp]
	inc edi
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	xor eax,eax
	xor edx,edx
	.cycle0:
		lodsb
		stosb
		png_setup_abs edx
		loop .cycle0

	mov ecx,[row_bytes]
	sub ecx,[bpp]
	mov ebx,[ebx+png_struct.row_buf]
	inc ebx
	.cycle1:
		lodsb
		sub al,byte[ebx]
		stosb
		png_setup_abs edx
		cmp edx,[lmins]
		jg .cycle1end ;if (..>..) ;We are already worse, don't continue.
		inc ebx
		loop .cycle1
	.cycle1end:
	mov eax,edx
	ret
endp

;void (png_structrp png_ptr, const uint_32 bpp, const png_size_t row_bytes)
align 4
proc png_setup_sub_row_only, png_ptr:dword, bpp:dword, row_bytes:dword
pushad
	mov ebx,[png_ptr]
	mov edi,[ebx+png_struct.try_row]
	mov byte[edi],PNG_FILTER_VALUE_SUB

	mov ecx,[bpp]
	inc edi
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	rep movsb

	mov ecx,[row_bytes]
	sub ecx,[bpp]
	mov edx,[ebx+png_struct.row_buf]
	inc edx
align 4
	.cycle0:
		lodsb
		sub al,byte[edx]
		stosb
		inc edx
		loop .cycle0
popad
	ret
endp

;png_size_t (png_structrp png_ptr, const png_size_t row_bytes, const png_size_t lmins)
align 4
proc png_setup_up_row uses ebx ecx edx edi esi, png_ptr:dword, row_bytes:dword, lmins:dword
	mov ebx,[png_ptr]
	mov edi,[ebx+png_struct.try_row]
	mov byte[edi],PNG_FILTER_VALUE_UP

	mov ecx,[row_bytes]
	inc edi
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	mov ebx,[ebx+png_struct.prev_row]
	inc ebx
	xor edx,edx
	.cycle0:
		lodsb
		sub al,byte[ebx]
		stosb
		png_setup_abs edx
		cmp edx,[lmins]
		jg .cycle0end ;if (..>..) ;We are already worse, don't continue.
		inc ebx
		loop .cycle0
	.cycle0end:
	mov eax,edx
	ret
endp

;void (png_structrp png_ptr, const png_size_t row_bytes)
align 4
proc png_setup_up_row_only, png_ptr:dword, row_bytes:dword
pushad
	mov ebx,[png_ptr]
	mov edi,[ebx+png_struct.try_row]
	mov byte[edi],PNG_FILTER_VALUE_UP

	mov ecx,[row_bytes]
	inc edi
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	mov ebx,[ebx+png_struct.prev_row]
	inc ebx
	.cycle0:
		lodsb
		sub al,byte[ebx]
		stosb
		inc ebx
		loop .cycle0
popad
	ret
endp

;png_size_t (png_structrp png_ptr, const uint_32 bpp, const png_size_t row_bytes, const png_size_t lmins)
align 4
proc png_setup_avg_row uses ebx ecx edx edi esi, png_ptr:dword, bpp:dword, row_bytes:dword, lmins:dword
locals
	sum dd 0 ;png_size_t
endl
	mov ebx,[png_ptr]
	mov edi,[ebx+png_struct.try_row]
	mov byte[edi],PNG_FILTER_VALUE_AVG

	mov ecx,[bpp]
	inc edi
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	mov ebx,[ebx+png_struct.prev_row]
	inc ebx
	.cycle0:
		lodsb
		mov ah,byte[ebx]
		shr ah,1
		sub al,ah
		stosb
		png_setup_abs dword[sum]
		inc ebx
		loop .cycle0

	mov ecx,[row_bytes]
	sub ecx,[bpp]
	mov eax,[png_ptr]
	mov edx,[eax+png_struct.row_buf]
	inc edx
	.cycle1:
		lodsb
		shl eax,24
		movzx ax,byte[ebx]
		add al,byte[edx]
		jnc @f
			mov ah,1
		@@:
		shr ax,1
		rol eax,8
		sub al,ah
		stosb
		png_setup_abs dword[sum]
		mov eax,[sum]
		cmp eax,[lmins]
		jg .cycle1end ;if (..>..) ;We are already worse, don't continue.
		inc ebx
		inc edx
		loop .cycle1
	.cycle1end:
	mov eax,[sum]
	ret
endp

;void (png_structrp png_ptr, const uint_32 bpp, const png_size_t row_bytes)
align 4
proc png_setup_avg_row_only, png_ptr:dword, bpp:dword, row_bytes:dword
pushad
	mov ebx,[png_ptr]
	mov edi,[ebx+png_struct.try_row]
	mov byte[edi],PNG_FILTER_VALUE_AVG

	mov ecx,[bpp]
	inc edi
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	mov ebx,[ebx+png_struct.prev_row]
	inc ebx
	.cycle0:
		lodsb
		mov ah,byte[ebx]
		shr ah,1
		sub al,ah
		stosb
		inc ebx
		loop .cycle0

	mov ecx,[row_bytes]
	sub ecx,[bpp]
	mov eax,[png_ptr]
	mov edx,[eax+png_struct.row_buf]
	inc edx
	.cycle1:
		lodsb
		mov ah,byte[ebx]
		shr ah,1
		sub al,ah
		mov ah,byte[edx]
		shr ah,1
		sub al,ah
		stosb
		inc ebx
		inc edx
		loop .cycle1
popad
	ret
endp

;png_size_t (png_structrp png_ptr, const uint_32 bpp,
;    const png_size_t row_bytes, const png_size_t lmins)
align 4
proc png_setup_paeth_row uses ebx ecx edx edi esi, png_ptr:dword, bpp:dword, row_bytes:dword, lmins:dword
locals
	pp dd ?
	sum dd ?
	v dd ?
	lp dd ?
	cp dd ?
	a dd ?
	b dd ?
	c dd ?
	p dd ?
	pa dd ?
	pb dd ?
	pc dd ?
endl
	;ecx - i
	;edi - dp
	;esi - rp
	mov dword[sum],0
	mov ebx,[png_ptr]
	mov eax,[ebx+png_struct.try_row]
	mov byte[eax],PNG_FILTER_VALUE_PAETH
	xor ecx,ecx
	mov esi,[ebx+png_struct.row_buf]
	inc esi
	mov edi,[ebx+png_struct.try_row]
	inc edi
	mov eax,[ebx+png_struct.prev_row]
	inc eax
	mov [pp],eax
	jmp @f

align 4
.cycle0:
	inc ecx
	@@:
	cmp ecx,[bpp]
	jae .cycle0end
	lodsb
	mov edx,[pp]
	movzx edx,byte[edx]
	sub al,dl
	stosb
	and eax,0xff
	mov [v],eax
	inc dword[pp]
	cmp eax,0x80
	jge @f
		add [sum],eax
		jmp .cycle0
	@@:
	mov eax,0x100
	sub eax,[v]
	add [sum],eax
	jmp .cycle0
.cycle0end:

	mov eax,[ebx+png_struct.row_buf]
	inc eax
	mov [lp],eax
	mov eax,[ebx+png_struct.prev_row]
	inc eax
	mov [cp],eax
	jmp @f

align 4
.cycle1:
	inc ecx
	@@:
	cmp ecx,[row_bytes]
	jae .cycle1end
	mov eax,[pp]
	movzx ebx,byte[eax]
	mov [b],ebx
	inc dword[pp]
	mov eax,[cp]
	movzx ebx,byte[eax]
	mov [c],ebx
	inc dword[cp]
	mov eax,[lp]
	movzx ebx,byte[eax]
	mov [a],ebx
	inc dword[lp]
	mov eax,[b]
	sub eax,[c]
	mov [p],eax
	mov ebx,[a]
	sub ebx,[c]
	mov [pc],ebx
	mov eax,[p]
	cmp eax,0
	jge @f
		neg eax
	@@:
	mov [pa],eax
	mov eax,[pc]
	cmp eax,0
	jge @f
		neg eax
	@@:
	mov [pb],eax
	mov eax,[p]
	add eax,[pc]
	jns @f
		neg eax
	@@:
	mov [pc],eax
	mov eax,[pa]
	cmp eax,[pb]
	jg .end0
	cmp eax,[pc]
	jg .end0
		mov eax,[a]
		jmp .end1
	.end0:
		mov eax,[pb]
		cmp eax,[pc]
		jg .end2
			mov eax,[b]
			jmp .end1
		.end2:
			mov eax,[c]
	.end1:
	mov [p],eax
	movzx eax,byte[esi]
	sub eax,[p]
	and eax,0xff
	stosb
	mov [v],eax
	inc esi
	cmp dword[v],0x80
	jge .end3
		mov eax,[v]
		add [sum],eax
		jmp .end4
	.end3:
		mov eax,0x100
		sub eax,[v]
		add [sum],eax
	.end4:
	mov eax,[sum]
	cmp eax,[lmins] ;We are already worse, don't continue.
	jbe .cycle1
.cycle1end:
	mov eax,[sum]
	ret
endp

;void (png_structrp png_ptr, const uint_32 bpp, const png_size_t row_bytes)
align 4
proc png_setup_paeth_row_only, png_ptr:dword, bpp:dword, row_bytes:dword
locals
	pp dd ?
	lp dd ?
	cp dd ?
	a dd ?
	b dd ?
	c dd ?
	p dd ?
	pa dd ?
	pb dd ?
	pc dd ?
endl
pushad
	;ecx - i
	;edi - dp
	;esi - rp
	mov eax,[png_ptr]
	mov ebx,[eax+png_struct.try_row]
	mov byte[ebx],4
	xor ecx,ecx
	mov edx,[png_ptr]
	mov eax,[edx+png_struct.row_buf]
	inc eax
	mov esi,eax
	mov ebx,[png_ptr]
	mov edx,[ebx+png_struct.try_row]
	inc edx
	mov edi,edx
	mov eax,[png_ptr]
	mov ebx,[eax+png_struct.prev_row]
	inc ebx
	mov [pp],ebx
	jmp @f

align 4
.cycle0:
	inc ecx
	@@:
	cmp ecx,[bpp]
	jae .cycle0end
	lodsb
	mov ebx,[pp]
	movzx ebx,byte[ebx]
	sub al,bl
	stosb
	inc dword[pp]
	jmp .cycle0
.cycle0end:

	mov eax,[png_ptr]
	mov ebx,[eax+png_struct.row_buf]
	inc ebx
	mov [lp],ebx
	mov edx,[png_ptr]
	mov eax,[edx+png_struct.prev_row]
	inc eax
	mov [cp],eax
	jmp @f

align 4
.cycle1:
	inc ecx
	@@:
	cmp ecx,[row_bytes]
	jae .cycle1end
	mov eax,[pp]
	movzx ebx,byte[eax]
	mov [b],ebx
	inc dword[pp]
	mov eax,[cp]
	movzx ebx,byte[eax]
	mov [c],ebx
	inc dword[cp]
	mov eax,[lp]
	movzx ebx,byte[eax]
	mov [a],ebx
	inc dword[lp]
	mov eax,[b]
	sub eax,[c]
	mov [p],eax
	mov ebx,[a]
	sub ebx,[c]
	mov [pc],ebx
	mov eax,[p]
	cmp eax,0
	jge @f
		neg eax
	@@:
	mov [pa],eax
	mov eax,[pc]
	cmp eax,0
	jge @f
		neg eax
	@@:
	mov [pb],eax
	mov eax,[p]
	add eax,[pc]
	jns @f
		neg eax
	@@:
	mov [pc],eax
	mov eax,[pa]
	cmp eax,[pb]
	jg .end0
	cmp eax,[pc]
	jg .end0
		mov eax,[a]
		jmp .end1
	.end0:
		mov eax,[pb]
		cmp eax,[pc]
		jg .end2
			mov eax,[b]
			jmp .end1
		.end2:
			mov eax,[c]
	.end1:
	mov [p],eax
	movzx eax,byte[esi]
	sub eax,[p]
	and eax,0xff
	stosb
	inc esi
	jmp .cycle1
.cycle1end:
popad
	ret
endp

;void (png_structrp png_ptr, png_row_infop row_info)
align 4
proc png_write_find_filter, png_ptr:dword, row_info:dword
locals
	filter_to_do dd ? ;unsigned int ;= png_ptr->do_filter
	row_buf  dd ? ;bytep
	best_row dd ? ;bytep
	bpp      dd ? ;uint_32
	mins     dd ? ;png_size_t
	row_bytes dd ? ;png_size_t ;= row_info->rowbytes
endl
pushad
	mov edi,[png_ptr]
if PNG_WRITE_FILTER_SUPPORTED eq 0
	mov eax,[edi+png_struct.rowbytes]
	inc eax
	stdcall png_write_filtered_row, edi, [edi+png_struct.row_buf], eax
else
	mov esi,[row_info]
	movzx eax,byte[edi+png_struct.do_filter]
	mov [filter_to_do],eax
	mov eax,[esi+png_row_info.rowbytes]
	mov [row_bytes],eax

	png_debug 1, 'in png_write_find_filter'

	; Find out how many bytes offset each pixel is
	movzx eax,byte[edi+png_struct.pixel_depth]
	add eax,7
	shr eax,3
	mov [bpp],eax

	mov eax,[edi+png_struct.row_buf]
	mov [row_buf],eax
	mov dword[mins], PNG_SIZE_MAX - 256 ;so we can detect potential overflow of the
		;running sum

; The prediction method we use is to find which method provides the
; smallest value when summing the absolute values of the distances
; from zero, using anything >= 128 as negative numbers.  This is known
; as the "minimum sum of absolute differences" heuristic.  Other
; heuristics are the "weighted minimum sum of absolute differences"
; (experimental and can in theory improve compression), and the "zlib
; predictive" method (not implemented yet), which does test compressions
; of lines using different filter methods, and then chooses the
; (series of) filter(s) that give minimum compressed data size (VERY
; computationally expensive).

; GRR 980525:  consider also

;   (1) minimum sum of absolute differences from running average (i.e.,
;       keep running sum of non-absolute differences & count of bytes)
;       [track dispersion, too?  restart average if dispersion too large?]

;  (1b) minimum sum of absolute differences from sliding average, probably
;       with window size <= deflate window (usually 32K)

;   (2) minimum sum of squared differences from zero or running average
;       (i.e., ~ root-mean-square approach)



; We don't need to test the 'no filter' case if this is the only filter
; that has been chosen, as it doesn't actually do anything to the data.

	mov eax,[edi+png_struct.row_buf]
	mov [best_row],eax

	cmp dword[row_bytes],PNG_SIZE_MAX/128
	jl @f ;if (..>=..)
		; Overflow can occur in the calculation, just select the lowest set
		; filter.

		xor eax,eax
		sub eax,[filter_to_do]
		and [filter_to_do],eax
		jmp .end0
	@@:
	mov eax,[filter_to_do]
	and eax,PNG_FILTER_NONE
	jz .end0
	cmp dword[filter_to_do],PNG_FILTER_NONE
	je .end0 ;else if (..!=0 && ..!=..)
		; Overflow not possible and multiple filters in the list, including the
		; 'none' filter.

		push esi
		xor eax,eax
		xor ebx,ebx
		mov ecx,[row_bytes]
		mov esi,[row_buf]
		.cycle0:
			lodsb
			png_setup_abs ebx
			loop .cycle0
		pop esi
		mov [mins],ebx
	.end0:

	; Sub filter
	mov eax,[filter_to_do]
	cmp eax,PNG_FILTER_SUB
	jne @f ;if (..==..)
		; It's the only filter so no testing is needed
		stdcall png_setup_sub_row_only, edi, [bpp], [row_bytes]
		mov eax,[edi+png_struct.try_row]
		mov [best_row],eax
		jmp .end1
	@@:
	and eax,PNG_FILTER_SUB
	jz .end1 ;else if (..!=0)
		stdcall png_setup_sub_row, edi, [bpp], [row_bytes], [mins]
		cmp eax,[mins]
		jge .end1 ;if (..<..)
			mov [mins],eax
			mov eax,[edi+png_struct.try_row]
			mov [best_row],eax
			test eax,eax
			jz .end1 ;if (..!=0)
				mov eax,[edi+png_struct.tst_row]
				mov [edi+png_struct.try_row],eax
				mov eax,[best_row]
				mov [edi+png_struct.tst_row],eax
	.end1:

	; Up filter
	mov eax,[filter_to_do]
	cmp eax,PNG_FILTER_UP
	jne @f ;if (..==..)
		; It's the only filter so no testing is needed
		stdcall png_setup_up_row_only, edi, [row_bytes]
		mov eax,[edi+png_struct.try_row]
		mov [best_row],eax
		jmp .end2
	@@:
	and eax,PNG_FILTER_UP
	jz .end2 ;else if (..!=0)
		stdcall png_setup_up_row, edi, [row_bytes], [mins]
		cmp eax,[mins]
		jge .end2 ;if (..<..)
			mov [mins],eax
			mov eax,[edi+png_struct.try_row]
			mov [best_row],eax
			test eax,eax
			jz .end2 ;if (..!=0)
				mov eax,[edi+png_struct.tst_row]
				mov [edi+png_struct.try_row],eax
				mov eax,[best_row]
				mov [edi+png_struct.tst_row],eax
	.end2:

	; Avg filter
	mov eax,[filter_to_do]
	cmp eax,PNG_FILTER_AVG
	jne @f ;if (..==..)
		; It's the only filter so no testing is needed
		stdcall png_setup_avg_row_only, edi, [bpp], [row_bytes]
		mov eax,[edi+png_struct.try_row]
		mov [best_row],eax
		jmp .end3
	@@:
	and eax,PNG_FILTER_AVG
	jz .end3 ;else if (..!=0)
		stdcall png_setup_avg_row, edi, [bpp], [row_bytes], [mins]
		cmp eax,[mins]
		jge .end3 ;if (..<..)
			mov [mins],eax
			mov eax,[edi+png_struct.try_row]
			mov [best_row],eax
			test eax,eax
			jz .end3 ;if (..!=0)
				mov eax,[edi+png_struct.tst_row]
				mov [edi+png_struct.try_row],eax
				mov eax,[best_row]
				mov [edi+png_struct.tst_row],eax
	.end3:

	; Paeth filter
	mov eax,[filter_to_do]
	cmp eax,PNG_FILTER_PAETH
	jne @f ;if (..==..)
		; It's the only filter so no testing is needed
		stdcall png_setup_paeth_row_only, edi, [bpp], [row_bytes]
		mov eax,[edi+png_struct.try_row]
		mov [best_row],eax
		jmp .end4
	@@:
	and eax,PNG_FILTER_PAETH
	jz .end4 ;else if (..!=0)
		stdcall png_setup_paeth_row, edi, [bpp], [row_bytes], [mins]
		cmp eax,[mins]
		jge .end4 ;if (..<..)
			mov [mins],eax
			mov eax,[edi+png_struct.try_row]
			mov [best_row],eax
			test eax,eax
			jz .end4 ;if (..!=0)
				mov eax,[edi+png_struct.tst_row]
				mov [edi+png_struct.try_row],eax
				mov eax,[best_row]
				mov [edi+png_struct.tst_row],eax
	.end4:

	; Do the actual writing of the filtered row data from the chosen filter.
	mov eax,[esi+png_row_info.rowbytes]
	inc eax
	stdcall png_write_filtered_row, edi, [best_row], eax
end if ;WRITE_FILTER
popad
	ret
endp


; Do the actual writing of a previously filtered row.
;void (png_structrp png_ptr, bytep filtered_row,
;    png_size_t full_row_length/*includes filter byte*/)
align 4
proc png_write_filtered_row uses eax ebx edi, png_ptr:dword, filtered_row:dword, full_row_length:dword
	png_debug 1, 'in png_write_filtered_row'

	mov eax,[filtered_row]
	movzx eax,byte[eax]
	png_debug1 2, 'filter = %d', eax

	mov edi,[png_ptr]
	stdcall png_compress_IDAT, [filtered_row], [full_row_length], Z_NO_FLUSH

if PNG_WRITE_FILTER_SUPPORTED eq 1
	; Swap the current and previous rows
	mov eax,[edi+png_struct.prev_row]
	test eax,eax
	jz @f ;if (..!=0)
		;eax = tptr
		mov ebx,[edi+png_struct.row_buf]
		mov [edi+png_struct.prev_row],ebx
		mov [edi+png_struct.row_buf],eax
	@@:
end if ;WRITE_FILTER

	; Finish row - updates counters and flushes zlib if last row
	stdcall png_write_finish_row, edi

if PNG_WRITE_FLUSH_SUPPORTED eq 1
	inc dword[edi+png_struct.flush_rows]

	mov eax,[edi+png_struct.flush_dist]
	cmp eax,0
	jle @f
	cmp [edi+png_struct.flush_rows],eax
	jl @f ;if (..>0 && ..>=..)
		stdcall png_write_flush, edi
	@@:
end if ;WRITE_FLUSH
	ret
endp
