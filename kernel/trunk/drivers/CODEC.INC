
align 4
proc detect_codec
	   locals
	     codec_id dd ?
	   endl

	   stdcall codec_read, dword 0x7C
	   shl eax, 16
	   mov [codec_id], eax

	   stdcall codec_read, dword 0x7E
	   or eax, [codec_id]

	   mov [codec.chip_id], eax
	   and eax, 0xFFFFFF00

	   mov edi, codecs
@@:
	   mov ebx, [edi]
	   test ebx, ebx
	   jz .unknown

	   cmp eax, ebx
	   jne .next
	   mov eax, [edi+4]
	   mov [codec.ac_vendor_ids], eax
	   stdcall detect_chip, [edi+8]
	   ret
.next:
	   add edi, 12
	   jmp @B
.unknown:
	   mov [codec.ac_vendor_ids], ac_unknown
	   mov [codec.chip_ids], chip_unknown
	   ret
endp

align 4
proc detect_chip stdcall, chip_tab:dword

	   mov eax, [codec.chip_id]
	   and eax, 0xFF

	   mov edi, [chip_tab]
@@:
	   mov ebx, [edi]
	   test ebx, ebx
	   jz .unknown

	   cmp eax,ebx
	   jne .next
	   mov eax, [edi+4]
	   mov [codec.chip_ids], eax
	   ret
.next:
	   add edi, 8
	   jmp @b
.unknown:
	   mov [codec.chip_ids], chip_unknown
	   ret
endp

align 4
proc setup_codec

        xor eax, eax
        stdcall codec_write, dword CODEC_AUX_VOL

        mov eax, 0x1010
        stdcall codec_write, dword CODEC_MASTER_VOL_REG

        mov ax, 0x08
        stdcall codec_write, dword 0x0C

        mov ax, 0x0808
	stdcall codec_write, dword CODEC_PCM_OUT_REG

        mov ax, 0x0808
        stdcall codec_write, dword 0x10

        mov ax, 0x0808
        stdcall codec_write, dword 0x12

        mov ax, 0x0808
        stdcall codec_write, dword 0x16


	stdcall codec_read, dword CODEC_EXT_AUDIO_CTRL_REG

	and  eax, 0FFFFh - BIT1 		 ; clear DRA (BIT1)
	or   eax, BIT0				 ; set VRA (BIT0)
	stdcall codec_write, dword CODEC_EXT_AUDIO_CTRL_REG

	stdcall set_sample_rate, dword 48000

.init_error:

	xor    eax, eax 			  ; exit with error
	ret

endp

align 4
proc set_master_vol stdcall, vol:dword

	   mov ebx, 63
	   mov ecx, 20644
	   mov eax, [vol]
	   cmp eax, 90
	   jna @f
	   mov eax, 90
@@:
	   mul ecx
	   shr eax, 15
	   sub ebx, eax
	   mov ah, bl
	   mov al, bl
	   stdcall codec_write, dword CODEC_MASTER_VOL_REG
	   ret
endp

align 4
proc get_master_vol stdcall, pvol:dword

	   stdcall codec_read, dword CODEC_MASTER_VOL_REG
	   and eax, 0x3F
	   mov ebx, 63
	   mov ecx, 20644

	   xchg eax, ebx
	   sub eax, ebx
	   shl eax, 15
	   xor edx, edx
	   div ecx
           mov ebx, [pvol]
           mov [ebx], eax
	   ret
endp

align 4
proc set_sample_rate stdcall, rate:dword
	   mov eax, [rate]
	   stdcall codec_write, dword CODEC_PCM_FRONT_DACRATE_REG
	   ret
endp

align 16
ac_unknown     db 'unknown manufacturer',13,10,0
ac_Realtek     db 'Realtek Semiconductor',13,10,0
ac_Analog      db 'Analog Devices',13,10,0
ac_CMedia      db 'C-Media Electronics',13,10,0
chip_unknown   db 'unknown chip', 13,10,0

CHIP_ANALOG    equ 0x41445300
CHIP_REALTEK   equ 0x414C4700
CHIP_CMEDIA    equ 0x434D4900

align 16
codecs	       dd CHIP_ANALOG, ac_Analog,  chips_Analog
	       dd CHIP_CMEDIA, ac_CMedia,  chips_CMedia
	       dd CHIP_REALTEK,ac_Realtek, chips_Realtek
	       dd 0

align 16
chips_Analog   dd 0x03, chip_AD1819
	       dd 0x40, chip_AD1881
	       dd 0x48, chip_AD1881A
	       dd 0x60, chip_AD1884
	       dd 0x61, chip_AD1886
	       dd 0x62, chip_AD1887
	       dd 0x63, chip_AD1886A
	       dd 0x70, chip_AD1980
	       dd 0x75, chip_AD1985
	       dd 0

chips_Realtek  dd 0x20, chip_ALC650
	       dd 0x21, chip_ALC650D
	       dd 0x22, chip_ALC650E
	       dd 0x23, chip_ALC650F
	       dd 0x60, chip_ALC655
	       dd 0x80, chip_ALC658
	       dd 0x81, chip_ALC658D
	       dd 0x90, chip_ALC850
	       dd 0

chips_CMedia   dd 0x41, chip_CM9738
	       dd 0x61, chip_CM9739
	       dd 0x69, chip_CM9780
	       dd 0x78, chip_CM9761
	       dd 0x82, chip_CM9761
	       dd 0x83, chip_CM9761
	       dd 0

align 16
;Analog Devices
chip_AD1819      db 'AD1819 ',0dh,0ah,00h
chip_AD1881      db 'AD1881 ',0dh,0ah,00h
chip_AD1881A     db 'AD1881A',0dh,0ah,00h
chip_AD1884      db 'AD1885 ',0dh,0ah,00h
chip_AD1885      db 'AD1885 ',0dh,0ah,00h
chip_AD1886      db 'AD1886 ',0dh,0ah,00h
chip_AD1886A     db 'AD1886A',0dh,0ah,00h
chip_AD1887      db 'AD1887 ',0dh,0ah,00h
chip_AD1980      db 'AD1980 ',0dh,0ah,00h
chip_AD1985      db 'AD1985 ',0dh,0ah,00h

;Realtek
chip_ALC650	 db 'ALC650 ',0dh,0ah,00h
chip_ALC650D	 db 'ALC650D',0dh,0ah,00h
chip_ALC650E	 db 'ALC650E',0dh,0ah,00h
chip_ALC650F	 db 'ALC650F',0dh,0ah,00h
chip_ALC655	 db 'ALC655 ',0dh,0ah,00h
chip_ALC658	 db 'ALC658 ',0dh,0ah,00h
chip_ALC658D	 db 'ALC658D',0dh,0ah,00h
chip_ALC850	 db 'ALC850 ',0dh,0ah,00h

;CMedia
chip_CM9738	 db 'CMI9738', 0dh,0ah,0
chip_CM9739	 db 'CMI9739', 0dh,0ah,0
chip_CM9780	 db 'CMI9780', 0dh,0ah,0
chip_CM9761	 db 'CMI9761', 0dh,0ah,0

