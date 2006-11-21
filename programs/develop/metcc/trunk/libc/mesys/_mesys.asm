format ELF
section '.text' executable

public _msys_draw_window
_msys_draw_window:
;arg1 - xcoord
;arg2 - ycoord
;arg3 - xsize
;arg4 - ysize
;arg5 - workcolor
;arg6 - type
;arg7 - captioncolor
;arg8 - windowtype
;arg9 - bordercolor
  push ebp
  mov  ebp,esp
  push ebx esi edi
  mov  ebx,[ebp+8]
  shl  ebx,16
  mov  bx,[ebp+16]
  mov  ecx,[ebp+12]
  shl  ecx,16
  mov  cx,[ebp+20]
  mov  edx,[ebp+28]
  shl  edx,24
  add  edx,[ebp+24]
  mov  esi,[ebp+36]
  shl  esi,24
  add  esi,[ebp+32]
  mov  edi,[ebp+40]
  xor  eax,eax
  int  0x40
  pop  edi esi ebx
  pop  ebp
  ret

public _msys_read_file
_msys_read_file:
;arg1 - file name
;arg2 - file offset
;arg3 - size to read
;arg4 - data
;arg5 - temp buffer
;arg6 - file size
  push	ebp
  mov	ebp,esp
  xor	eax,eax
  mov	[file_struct.operation],eax
  mov	eax,[ebp+12]
  mov	[file_struct.offset],eax
  mov	eax,[ebp+16]
  mov	[file_struct.offset],eax
  mov	eax,[ebp+20]
  mov	[file_struct.offset],eax
  mov	[file_struct.temp_buffer],temp_buffer
  mov	edx,[ebp+8]
  call	copy_file_name
  push	ebx
  mov	ebx,file_struct
  mov	eax,58
  int	0x40
  mov	ecx,[ebp+28]
  test	ecx,ecx
  jz	.no_file_size
  mov	[ecx],ebx
.no_file_size:
  pop	ebx
  pop	ebp
  ret

copy_file_name:
  push	esi edi
  cld
  mov	edi,edx
  xor	eax,eax
  xor	ecx,ecx
  dec	ecx
  repnz scasb
  not	ecx
  mov	edi,file_struct.path
  mov	esi,edx
  rep	movsb
  pop	edi esi
  ret

public _msys_write_file
_msys_write_file:
;arg1 - file name
;arg2 - size
;arg3 - data
  push	ebp
  mov	ebp,esp
  xor	eax,eax
  mov	[file_struct.offset],eax
  inc	eax
  mov	[file_struct.operation],eax
  mov	eax,[ebp+12]
  mov	[file_struct.size],eax
  mov	eax,[ebp+16]
  mov	[file_struct.data],eax
  mov	[file_struct.temp_buffer],temp_buffer
  mov	edx,[ebp+8]
  call	copy_file_name
  push	ebx
  mov	eax,58
  mov	ebx,file_struct
  int	0x40
  pop	ebx
  pop	ebp
  ret

public _msys_run_program
_msys_run_program:
;arg1 - program name
;arg2 - parameters
  push	ebp
  mov	ebp,esp
  mov	[file_struct.operation],16
  xor	eax,eax
  mov	[file_struct.offset],eax
  mov	[file_struct.data],eax
  mov	eax,[ebp+12]
  mov	[file_struct.param],eax
  mov	[file_struct.temp_buffer],temp_buffer;
  mov	edx,[ebp+8]
  call	copy_file_name
  push	ebx
  mov	eax,58
  mov	ebx,file_struct
  int	0x40
  pop	ebx
  pop	ebp
  ret

public _msys_debug_out
_msys_debug_out:
;arg1 - char to out
  push	ebx
  mov	ecx,[esp+8]
  mov	ebx,1
  mov	eax,63
  int	0x40
  pop	ebx
  ret
section '.data' writeable
section '.bss' writeable
file_struct:
.operation   rd 1
.offset      rd 1
.param:
.size	     rd 1
.data	     rd 1
.temp_buffer rd 1
.path	     rb 1024
  temp_buffer rb 4096
		