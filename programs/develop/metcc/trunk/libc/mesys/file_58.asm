format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_read_file,20
;arg1 - file name
;arg2 - file offset
;arg3 - size to read
;arg4 - data
;arg5 - file size
  push	ebp
  mov	ebp,esp
  xor	eax,eax
  mov	[file_struct.operation],eax
  mov	eax,[ebp+12]
  mov	[file_struct.offset],eax
  mov	eax,[ebp+16]
  mov	[file_struct.size],eax
  mov	eax,[ebp+20]
  mov	[file_struct.data],eax
  mov	[file_struct.temp_buffer],temp_buffer
  mov	edx,[ebp+8]
  call	copy_file_name
  push	ebx
  mov	ebx,file_struct
  mov	eax,58
  int	0x40
  mov	ecx,[ebp+24]
  test	ecx,ecx
  jz	.no_file_size
  mov	[ecx],ebx
.no_file_size:
  pop	ebx
  pop	ebp
  ret   20

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

public_stdcall _msys_write_file,12
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
  ret   12

public_stdcall _msys_run_program,8
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
  ret   8
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


