include "macros.inc"
;
; OS function implementation
; SmallC for KolibriOS
;

;B+ General definitions

;B+ File defs
 ;const
  ;param
  BAD	equ -1
  files equ 100
  save_buffer 		equ 0x20000 ;32
  save_buffer_w 	equ 0x400000 ;32
  save_file_name	equ 0x20000

  ;system
  EOF equ -1

 ;memory
 fileinfo     equ I_END
 start_data   equ (fileinfo+16384)
 ;
 mem_heap equ 0x100000
 ;
 g_handle equ 0x300000
  ;dword - pointer - relative to file
  ;dword - begin of file
  ;dword - file size
  ;dword - 0/1 <=> read/write
;E:.

;E:.

init_osfunc:
;B+ Init OS functions
;B+ Clear file handles
  mov  edi,g_handle
  mov  ecx,files
  shl  ecx,2 ;*4
  xor  eax,eax
  cld
rep stosd
;E:.
  ret
;E:.

;B+ Main OS functions
ppp dd 70

_OS_fopen:
;B+ Implement "fopen"
  ;esp+4 - mode
  ;esp+8 - file name

;  mov  eax,-1
;  int  0x40

;  mov  ebx,[esp+8];
; push dword 10
; push dword [ppp]
; push ebx
; push dword 12
;  call _outstrg
;  add  esp,4*4
;  add  [ppp],10

;  cmp  byte [ebx+8],0
;  jne .l
;  mov  byte [ebx+8],'?'
;.l:
;  cmp  [ppp],80
;  je   .l

	mov ecx , [esp+4] ; file mode
    mov [file_mode],ecx
;B+ Copy file name
  
  
  mov  esi,[esp+8]
  mov  edi,[p_filename]
  mov  ecx,12
.next_copy:
lodsb
  ;fill name (space)
  or   al,al
  jz   .fill_space
  ;set upper case
  cmp  al,'a'
  jb   .good_char
  cmp  al,'z'
  ja   .good_char
  add  al,'A'-'a'
.good_char:
stosb
  dec  ecx
  jnz  .next_copy
.fill_space:
  mov  al,' '
  cld
rep stosb

mov  eax,[file_mode]
  cmp  byte [eax],'w'
  jne	.no_wri


;B+ Copy file name
  mov  esi,[esp+8]
  mov  edi,[w_file_name]
  mov  ecx,12
.next_copy2:
lodsb
  ;fill name (space)
  or   al,al
  jz   .fill_space2
  ;set upper case
  cmp  al,'a'
  jb   .good_char2
  cmp  al,'z'
  ja   .good_char2
  add  al,'A'-'a'
.good_char2:
stosb
  dec  ecx
  jnz  .next_copy2
.fill_space2:
  mov  al,' '
  cld
rep stosb

.no_wri:



;E:.
;B+ Find file handle
  mov  eax,g_handle
.new_handle:
  cmp  dword [eax+4],0
  je   .find_place
  add  eax,16
  cmp  eax,g_handle+files*16-16
  jne  .new_handle
  xor  eax,eax ; no free handle
  ret
.find_place:
; TMP:  mov  eax,[.ccc]
; TMP:  add  [.ccc],16
;E:.
 push eax
;B+ Test open mode
  mov  eax,[esp+4+4]
  cmp  byte [eax],'r'
  je   .open_read
  cmp  byte [eax],'w'
  je   .open_write
  ;bad mode
  add  esp,4
  mov  eax,eax ; invalid open mode
  ret
;E:.

; TMP:.ccc dd g_handle

.open_read:
;B+ Open for read
;B+ Read file

  ;Wait to read correct
  mov  ebx,100
  mov  eax,5
  int  0x40

  mov  eax,[g_fileend]
  mov  dword [file_parameters+2*4],2000 ;read all
  mov  dword [file_parameters+3*4],eax

  mov  dword [file_parameters],0
  mov  ebx,file_parameters
  mov  eax,58
  int  0x40
;E:.

;B+ TEST FILE FOUND
  or   eax,eax
  jz   .file_found
  cmp  eax,5
  je   .file_found

;  mov  ecx,eax ; eax
;  mov  ebx,8 shl 16 + 0x0100
;  mov  edx,100 shl 16 + 120
;  mov  esi,0xffffff
;  mov  eax,47
;  int  0x40

  ;file not found - return 0
  add  esp,4
  xor  eax,eax
  ret
.file_found:
;E:.
 pop  eax
 push ebx
  xchg eax,ebx
;B+ Fill file handle
  ;save current pointer
  xor  eax,eax
  mov  [ebx],eax

  ;save file begin
  mov  eax,[g_fileend]
  mov  [ebx+4],eax

  ;save file size
 pop  eax
  mov  [ebx+8],eax
  ;reserve file zone
  add  eax,7
  and  eax,not 7
  add  [g_fileend],eax

  ;save file mode
  mov  eax,0 ;read
  mov  [ebx+12],eax
;E:.
  xchg eax,ebx ;return pointer place
  ret
;E:.

.open_write:
;B+ Open for write
;B+ Reserve filename

; p_filename -> w_file_name

;pusha
;  mov eax, w_file_name
;  mov ebx, [p_filename]
;.ncpy:
;  mov ch, byte [ebx]
;  cmp ch, 0
;  je .ecpy
;  mov [eax], ch
;  inc dword [eax]
;  inc dword [ebx]
;jmp .ncpy
;
;.ecpy:
;
;popa

  mov [save_buffer_p], save_buffer_w

  mov  esi,[p_filename]
  mov  edi,[g_fileend]
  mov  ecx,12
  cld
rep movsb
  add  [g_fileend],16
;E:.
 pop  ebx
;B+ Fill file handle
  ;save begin pointer
  xor  eax,eax
  mov  [ebx],eax

  ;save file begin
  mov  eax,[g_fileend]
  mov  [ebx+4],eax

  ;save file zone
  mov  dword [ebx+8],save_buffer
  ;reserve file zone
  add  [g_fileend],save_buffer

  ;save file mode
  mov  eax,1 ;write
  mov  [ebx+12],eax
;E:.
  xchg eax,ebx ;return pointer place
  ret
;E:.

;E:.

_OS_fclos:
;B+ Close file
 ;esp+4 - file handle

;B+ Test write mode - save file
  mov  eax,[esp+4]
  mov  eax,[eax+12]
  cmp  eax,1
;E:.
  jne  .no_write

   mov eax, [esp+4]
   mov ecx, [eax]
   mov ebx, [eax+8]

   mov ebx, [save_buffer_p]
   sub ebx, save_buffer_w
	; ebx = number of read bytes = file size
    ; save loaded file
    mov  [dest_info.bytes],ebx ; file size in bytes
    
    mov  [dest_info.bytes+4], save_buffer_w
    ;mov eax, [p_filename];[w_file_name]
    ;mov [destination],eax
    mov  eax,70
    mov  ebx,dest_info
    mcall

    ; check if 58 function failed
    test eax,eax
    je	 .ok_write
    add  eax,7	      ; error number += 7
    cmp  eax,6+7
    jna  .copy_error
    mov  eax,7+7
    jmp  .copy_error

.copy_error:
  .ok_write:


;E:.
  jmp  .read

 .no_write:
;B+ Test read mode - if no error end
  cmp  eax,0
  je   .read
  mov  eax,BAD
  ret
;E:.
.read:

;B+ Relace memory
  ;find file size
  mov  eax,[esp+4]
  mov  ecx,[eax+8]
  add  ecx,7
  and  ecx,not 7
 push ecx

  ;mov memory
  mov  esi,[eax+4]
  mov  edi,esi
  add  esi,ecx
  mov  ecx,[g_fileend]
  sub  ecx,edi
  jz   .is_last
  shr  ecx,2
  inc  ecx ;not neccessery
  cld
rep movsd
  ;update gl. memory
.is_last:
 pop  ecx
  sub  dword [g_fileend],ecx

  ;update file pointers
  mov  edx,ecx
  mov  ecx,[eax+4]
  mov  eax,g_handle
.new_handle1:
  mov  ebx,[eax+4]
  cmp  ebx,ecx
  jbe  .no_update
  sub  ebx,edx
  mov  [eax+4],ebx
.no_update:
  add  eax,16
  cmp  eax,g_handle+files*16
  jne  .new_handle1

  ;clear handle
  mov  edi,[esp+4]
  xor  eax,eax
  cld
  stosd
  stosd
  stosd
  stosd
;E:.
  ret
;E:.

_OS_fgetc:
;B+ Load char from file
 ;esp+4 - input file

  mov  eax,[esp+4]
  mov  ebx,[eax]
  cmp  ebx,[eax+8]
  je   .eof
  inc  dword [eax]
  add  ebx,[eax+4]
  movzx eax,byte [ebx]
  ret
.eof:
  mov  eax,EOF
  ret
;E:.

;rrr db 'g',0

_OS_fputc:
;B+ Save char to file
 ;esp+4 - output file
 ;esp+8 - char to write

;push dword '<'
;mov  cl,1
;push dword 0
;call test_outch
;add  esp,8



;B+ Temp - write direct.
  cmp  dword [esp+4],__iob
  jne  .real_write0
  jmp  _OS_exit
.real_write0:
  cmp  dword [esp+4],__iob+32
  jne  .real_write1
  mov  [print_textcolor],0x00ffff
  jmp  test_outch
.real_write1:
  cmp  dword [esp+4],__iob+64
  jne  .real_write2
  mov  [print_textcolor],0x77ffff
  jmp  test_outch
.real_write2:
;E:.

mov ebx,[save_buffer_p]
mov eax,[esp+8]
mov [ebx],eax
inc dword [save_buffer_p]


ret

;push dword '<'
;mov  cl,1
;push dword 0
;call test_outch
;add  esp,8

  mov  eax,[esp+4]
  mov  ebx,[eax]
 push ebx
  cmp  ebx,[eax+8]
  jne  .write_normal



;B+ Alloc save_buffer bytes
  ;mov memory
  mov  ebx,[esp+4+4]
  mov  esi,[g_fileend]
  mov  edi,esi
  add  edi,save_buffer-4
  mov  ecx,esi
  sub  ecx,[ebx+4]
  sub  ecx,[ebx+8]
  shr  ecx,2
  jz   .is_last
  sub  esi,4
  std
rep movsd
.is_last:

  ;expand file size
  add  dword [eax+8],save_buffer

  ;update file pointers
  mov  ebx,g_handle
.new_handle:
  mov  ecx,[ebx+4]
  cmp  [eax+4],ecx
  jae  .no_update
  add  dword [ebx+4],save_buffer
.no_update:
  add  ebx,16
  cmp  ebx,g_handle+files*16-16
  jne  .new_handle
;E:.

.write_normal:
 pop  ebx
  inc  dword [eax]
  add  ebx,[eax+4]
  mov  cl,[esp+8]
  mov  byte [ebx],cl

;sub  [test_outch.x_coord],2
;
;push dword '>'
;mov  cl,1
;push dword 0
;call test_outch
;add  esp,8
;
;sub  [test_outch.x_coord],6

  xor  eax,eax
  ret
;E:.

_OS_callo:
;B+ Alloc memory
  ;find all size
  mov  eax,[esp+4]
  mov  ebx,[esp+8]
  mul  ebx
 push eax

  ;clear memory
  mov  edi,[.mem_p]
  xor  eax,eax
  mov  ecx,[esp]
  cld
rep  stosb

  ;update new memory pointer
 pop  ebx
 push dword [.mem_p]
  add  ebx,7
  and  ebx,not 7
  add  [.mem_p],ebx

  ;return value
 pop  eax
  ret

.mem_p dd mem_heap
;E:.

_OS_exit:
;B+ Exit program
;  ;TMP
;  mov  eax,-1
;  int  0x40
  mov  esp,[exit_esp]
  sub  esp,4
  ret
;E:.

;E:.




;B+ Test procedures

;B+ Definitions
LEFTMARGIN equ 11
BEGIN_CHARS equ 20
NL equ 10
;E:.

print_textcolor dd 0x00ffff

_file_beg:
;B+ Show begin of file - test fopen
 ;esp+4 - file handle (descriptor)

  mov  eax,[esp+4]
  mov  ebx,10 shl 16 + 30
  mov  ecx,[print_textcolor]
  mov  edx,[eax+4]
  mov  esi,BEGIN_CHARS
  mov  eax,4
  int  0x40
  ret
;E:.

_outstrg:
;B+ Draw black text - test function call
 ;esp+4*4 - x
 ;esp+4*3 - y
 ;esp+4*2 - *c
 ;esp+4*1 - len

  mov  ebx,[esp+4*4]
  shl  ebx,16
  mov  bx,[esp+4*3]
  mov  ecx,[print_textcolor]
  mov  edx,[esp+4*2]
  mov  esi,[esp+4]
  mov  eax,4
  int  0x40
  ret
;E:.

test_outch:
;B+ Draw one char - use as _OS_fputc, to test printf(...)
 ;esp+8 - char to write

  ;this is test! \b \r - not nesessary

  mov  al,[esp+8]
  cmp  al,NL
  jne  .no_newline
  add  [.y_coord],10
  mov  [.x_coord],LEFTMARGIN
  ret
.no_newline:

  mov  ebx,[.x_coord]
  shl  ebx,16
  mov  bx,word [.y_coord]
  mov  ecx,[print_textcolor]
  mov  [.out_char],al
  mov  edx,.out_char
  mov  esi,1
  mov  eax,4
  int  0x40

  add  [.x_coord],6
;mov  eax,5
;mov  ebx,5
;int  0x40
  ret

.x_coord dd LEFTMARGIN
.y_coord dd 60
.out_char db 0
;E:.

;E:.

;B+ Data section
;B+ Memory managment
g_fileend dd g_handle+files*4*4 ;from 2MB+100*4*4
;w_buff dd
;E:.

save_buffer_p dd save_buffer_w
;B+ File parameters
file_parameters:
 dd 0x0 ; mode
 dd 0x0  ; first block
 dd 1000 ; block read
 dd -1	 ; return place
 dd fileinfo ; work area
filepath: times 100 db 0

file_mode dd 0 ;file mode

dest_info:		     ; DESTINATION FILEINFO
	dd	2
	dd	0
	dd	0
.bytes	dd	?
	dd	save_buffer
	db      0
destination:
	dd  save_file_name
	;db	"EXAMPLE.ASM",0
	


w_file_name dd	save_file_name

p_filename dd 0x0
;E:.
;E:.

  ;TO DO
  mov  eax,-1
  int  0x40
  ;TO DO

