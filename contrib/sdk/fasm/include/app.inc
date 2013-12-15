

include 'import32.inc'
include 'proc32.inc'

use32
org 0x0

  db 'MENUET02'
  dd 0x01
  dd __start
  dd __iend
  dd __bssend
  dd __stack
  dd __cmdline
	dd __pgmname
  dd 0x0
	dd __idata_start
	dd __idata_end
	dd main

IMAGE_DOS_SIGNATURE  					equ 0x5A4D
IMAGE_NT_SIGNATURE   					equ 0x00004550
IMAGE_FILE_MACHINE_I386     	equ 0x014c
IMAGE_NT_OPTIONAL_HDR32_MAGIC equ 0x10B
IMAGE_NT_HEADERS32_SIZE       equ 0xF8


__start:

.e_lfanew                     equ  0x3C

.FileHeader.NumberOfSections  equ  0x06

.OptionalHeader.ImageBase     equ  0x34
.SectionAlignment             equ  0x38
.FileAlignment                equ  0x3C

.OptionalHeader.SizeOfImage   equ  0x50
.OptionalHeader.SizeOfHeaders equ  0x54

.VirtualAddress               equ  0x0C
.SizeOfRawData                equ  0x10
.PointerToRawData             equ  0x14

.OptionalHeader.RelocDataDirectory.VirtualAddress   equ 0xA0
.OptionalHeader.RelocDataDirectory.Size             equ 0xA4

.SizeOfBlock                  equ  0x04


          mov   eax, 68
          mov   ebx, 12
          mov   ecx, STACK_SIZE
          add   ecx, 4095
          and   ecx, -4096
          int   0x40
          test  eax, eax
          jz    .exit

          add   ecx, eax
          mov   [fs:8], eax
          mov   [fs:12], ecx
          mov   esp, ecx

          sub   esp, 1024
          mov   eax, 9
          mov   ebx, esp
          mov   ecx, -1
          int   0x40

          mov   eax, [ebx+30]
          mov   [fs:0], eax
          add   esp, 1024

          mov   eax, 68
          mov   ebx, 27
          mov   ecx, libc_path
          int   0x40
          test  eax, eax
          jz    .exit

          push  edx
          push  eax

.validate_pe:

          cmp     edx, 0x3F
          jbe     .exit
          cmp     word [eax], IMAGE_DOS_SIGNATURE
          jne     .exit

          mov     edx, [eax+.e_lfanew]
          test    edx, edx
          jz      .exit

          add     edx, eax                ;edx = nt header
          jb      .exit

          cmp     dword [edx], IMAGE_NT_SIGNATURE
          jnz     .exit

          cmp     word [edx+0x04], IMAGE_FILE_MACHINE_I386
          jnz     .exit

          cmp     word [edx+0x18], IMAGE_NT_OPTIONAL_HDR32_MAGIC
          jnz     .exit

          mov     ecx, [edx+.SectionAlignment]
          cmp     ecx, 4095
          ja      .l1

          cmp     ecx, [edx+.FileAlignment]
          jne     .exit

          jmp     @F
.l1:
          cmp     ecx, [edx+.FileAlignment]
          jb      .exit
@@:
          test    ecx, ecx
          jz      .exit

          lea     eax, [ecx-1]
          test    ecx, eax
          jnz     .exit

          mov     ecx, [edx+.FileAlignment]
          test    ecx, ecx
          jz      .exit

          lea     ebx, [ecx-1]
          test    ecx, ebx
          jnz     .exit

          cmp     word [edx+.FileHeader.NumberOfSections], 96
          ja      .exit

.create_image:

          mov     ecx, [edx+.OptionalHeader.SizeOfImage]
          mov     eax, 68
          mov     ebx, 12
          int     0x40

          mov     ebp, eax
          test    eax, eax
          jz      .exit_2

          mov     ecx, [edx+.OptionalHeader.SizeOfHeaders]
          mov     esi, [esp]
          mov     edi, eax
          shr     ecx, 2                                      ;copy header
          rep movsd

          lea     eax, [edx+IMAGE_NT_HEADERS32_SIZE]          ;eax = MAGE_SECTION_HEADER
          movzx   ebx, word [edx+.FileHeader.NumberOfSections]
          test    ebx, ebx
          jz      @F

.copy_loop:
          mov     ecx, [eax+.SizeOfRawData]
          test    ecx, ecx
          jz      .next_section

          mov     esi, [eax+.PointerToRawData]
          test    esi, esi
          jz      .next_section

          add     esi, [esp]
          mov     edi, [eax+.VirtualAddress]
          add     edi, ebp
          shr     ecx, 2
          rep movsd
                                                   ;copy section
.next_section:
          add     eax, 0x28
          dec     ebx
          jnz     .copy_loop
@@:
          push    edx

          mov     esi, [edx+.OptionalHeader.RelocDataDirectory.Size]
          test    esi, esi
          jz      .call_libc

          mov     ebx, ebp
          sub     ebx, [edx+.OptionalHeader.ImageBase]                          ;delta
          mov     edx, [edx+.OptionalHeader.RelocDataDirectory.VirtualAddress]
          lea     ecx, [ebp+edx]                                                ;IMAGE_BASE_RELOCATION
          mov     eax, [ecx+.SizeOfBlock]
          test    eax, eax
          jz      .unmap_relocs

          mov     esi, ebx
          shr     esi, 16
          push    esi
align 4
.loop_block:
          sub     eax, 8
          lea     edx, [ecx+8]                                ;entry
          shr     eax, 1
          jz      .next_block
          lea     edi, [ecx+eax*2+8]                          ;last entry
align 4
.loop_reloc:

          mov     si, [edx]
          mov     eax, esi
          and     eax, 0FFFh
          add     eax, [ecx]                                  ;offset
          shr     si, 12                                      ;reloc type
          dec     si
          jnz     @F

.type_1:
          mov     esi, [esp]
          add     [eax+ebp], si
          jmp     .next_entry
@@:
          dec     si
          jnz     @F

.type_2:
          add     [eax+ebp], bx
          jmp     .next_entry
@@:
          dec     si
          jnz     .next_entry

.type_3:
          add     [eax+ebp], ebx
.next_entry:
          add     edx, 2
          cmp     edx, edi
          jne     .loop_reloc
.next_block:
          add     ecx, [ecx+.SizeOfBlock]
          mov     eax, [ecx+.SizeOfBlock]
          test    eax, eax
          jnz     .loop_block

          add     esp, 4
          pop     edx
          mov     esi, [edx+.OptionalHeader.RelocDataDirectory.Size]
          mov     edx, [edx+.OptionalHeader.RelocDataDirectory.VirtualAddress]

.unmap_relocs:
          mov     ebx, 26
          mov     eax, 68
          mov     ecx, ebp
          int     0x40

.call_libc:

          push  ebp
          mov   edx, [ebp+0x3C]
          add   ebp, [ebp+edx+0x28]

          call  ebp

.exit_2:
.exit:
          or    eax, -1
          int   0x40

libc_path db '/kolibrios/lib/libc.dll',0
















