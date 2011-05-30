;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


AD_LOSEL   equ BIT5
AD_HPSEL   equ BIT10

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
	   mov esi, eax
	   call SysMsgBoardStr
	   stdcall detect_chip, [edi+8]

	   ret
.next:
	   add edi, 12
	   jmp @B
.unknown:
	   mov [codec.ac_vendor_ids], ac_unknown
	   mov [codec.chip_ids], chip_unknown

	   mov esi, chip_unknown
	   call SysMsgBoardStr
	   mov eax, [codec.chip_id]
	   call dword2str
	   call SysMsgBoardStr
	   ret
endp

align 4
proc detect_chip stdcall, chip_tab:dword

	   mov eax, [codec.chip_id]
	   and eax, 0xFF

	   mov edi, [chip_tab]
@@:
	   mov ebx, [edi]
	   cmp ebx, 0xFF
	   je .unknown

	   cmp eax,ebx
	   jne .next
	   mov eax, [edi+4]
	   mov [codec.chip_ids], eax
	   mov esi, eax
	   call SysMsgBoardStr
	   ret
.next:
	   add edi, 8
	   jmp @b
.unknown:
	   mov [codec.chip_ids], chip_unknown
	   mov esi, chip_unknown
	   call SysMsgBoardStr
	   mov eax, [codec.chip_id]
	   call dword2str
	   call SysMsgBoardStr
	   ret
endp

align 4
proc setup_codec

	   xor eax, eax
	   stdcall codec_write, dword CODEC_AUX_VOL

	   mov eax, 0x0B0B
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
	   and eax, 0FFFFh - BIT1		   ; clear DRA (BIT1)
	   or eax, BIT0 			  ; set VRA (BIT0)
	   stdcall codec_write, dword CODEC_EXT_AUDIO_CTRL_REG

	   stdcall set_sample_rate, dword 48000

.init_error:
	   xor	  eax, eax			     ; exit with error
	   ret
endp


; param
;  eax= volume  -10000 - 0 for both channels

align 4
set_master_vol:
	   cmp eax, 0
	   jl @F
	   xor eax, eax
	   jmp .set
@@:
	   cmp eax, -9450
	   jg .set
	   mov eax, -9450     ;clamp into 6 bits
.set:
	   cdq
	   mov ebx, -150
	   idiv ebx
	   mov ah, al
	   stdcall codec_write, dword CODEC_MASTER_VOL_REG
	   xor eax, eax
	   ret

align 4
proc get_master_vol stdcall, pvol:dword

	   stdcall codec_read, dword CODEC_MASTER_VOL_REG
	   and eax, 0x3F
	   imul eax, -150
	   mov ebx, [pvol]
	   mov [ebx], eax
	   xor eax, eax
	   ret
endp

align 4
proc set_sample_rate stdcall, rate:dword
	   mov eax, [rate]
	   stdcall codec_write, dword CODEC_PCM_FRONT_DACRATE_REG
	   ret
endp

patch_AD:
	   stdcall codec_read, 0x76
	   or ax, BIT5+BIT10
	   stdcall codec_write, 0x76
	   ret


align 16
ac_unknown     db 'unknown manufacturer',13,10,0
ac_Realtek     db 'Realtek Semiconductor',13,10,0
ac_Analog      db 'Analog Devices',13,10,0
ac_CMedia      db 'C-Media Electronics',13,10,0
ac_Cirrus      db 'Cirrus Logic',13,10,0
ac_Wolfson     db 'Wolfson Microelectronics',13,10,0
ac_VIA	       db 'VIA Technologies',13,10,0
ac_SigmaTel    db 'SigmaTel',13,10,0
ac_eMicro      db 'eMicro',13,10,0

chip_unknown   db 'unknown codec id ', 0

CHIP_ANALOG    equ 0x41445300
CHIP_REALTEK   equ 0x414C4700
CHIP_CMEDIA    equ 0x434D4900
CHIP_CIRRUS    equ 0x43525900
CHIP_WOLFSON   equ 0x574D4C00
CHIP_VIA       equ 0x56494100
CHIP_SIGMATEL  equ 0x83847600
CHIP_EMICRO    equ 0x454D4300

align 16
codecs	       dd CHIP_ANALOG, ac_Analog,  chips_Analog
	       dd CHIP_CMEDIA, ac_CMedia,  chips_CMedia
	       dd CHIP_REALTEK,ac_Realtek, chips_Realtek
	       dd CHIP_CIRRUS, ac_Cirrus,  chips_Cirrus
	       dd CHIP_WOLFSON,ac_Wolfson, chips_Wolfson
	       dd CHIP_VIA,    ac_VIA,	   chips_VIA
	       dd CHIP_SIGMATEL,    ac_SigmaTel,     chips_SigmaTel
	       dd CHIP_EMICRO,	  ac_eMicro,	 chips_eMicro
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
	       dd 0x72, chip_AD1981A
	       dd 0x74, chip_AD1981B
	       dd 0x75, chip_AD1985
	       dd 0xFF

chips_Realtek:
	       dd 0x10, chip_ALC201a
	       dd 0x20, chip_ALC650
	       dd 0x21, chip_ALC650D
	       dd 0x22, chip_ALC650E
	       dd 0x23, chip_ALC650F
	       dd 0x60, chip_ALC655
	       dd 0x80, chip_ALC658
	       dd 0x81, chip_ALC658D
	       dd 0x90, chip_ALC850
	       dd 0xFF

chips_CMedia   dd 0x41, chip_CM9738
	       dd 0x61, chip_CM9739
	       dd 0x69, chip_CM9780
	       dd 0x78, chip_CM9761
	       dd 0x82, chip_CM9761
	       dd 0x83, chip_CM9761
	       dd 0xFF

chips_Cirrus   dd 0x00, chip_CS4297
	       dd 0x10, chip_CS4297A
	       dd 0x20, chip_CS4298
	       dd 0x28, chip_CS4294
	       dd 0x30, chip_CS4299
	       dd 0x34, chip_CS4299D
	       dd 0x48, chip_CS4201
	       dd 0x58, chip_CS4205
	       dd 0x60, chip_CS4291
	       dd 0x70, chip_CS4202
	       dd 0xFF

chips_Wolfson  dd 0x00, chip_WM9700
	       dd 0x03, chip_WM9703
	       dd 0x04, chip_WM9704
	       dd 0xFF

chips_VIA      dd 0x61, chip_VIA1612A
	       dd 0xFF

chips_SigmaTel dd 0x58, chip_STAC9758
	       dd 0xFF

chips_eMicro   dd 0x28, chip_EM28028
	       dd 0xFF

align 16
;Analog Devices
chip_AD1819	 db 'AD1819 ',0dh,0ah,00h
chip_AD1881	 db 'AD1881 ',0dh,0ah,00h
chip_AD1881A	 db 'AD1881A',0dh,0ah,00h
chip_AD1884	 db 'AD1885 ',0dh,0ah,00h
chip_AD1885	 db 'AD1885 ',0dh,0ah,00h
chip_AD1886	 db 'AD1886 ',0dh,0ah,00h
chip_AD1886A	 db 'AD1886A',0dh,0ah,00h
chip_AD1887	 db 'AD1887 ',0dh,0ah,00h
chip_AD1980	 db 'AD1980 ',0dh,0ah,00h
chip_AD1981A	 db 'AD1981A',0dh,0ah,00h
chip_AD1981B	 db 'AD1981B',0dh,0ah,00h
chip_AD1985	 db 'AD1985 ',0dh,0ah,00h

;Realtek
chip_ALC201a	 db 'ALC201a',0dh,0ah,00h
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

;Cirrus
chip_CS4297	 db 'CS4297',13,10,0
chip_CS4297A	 db 'CS4297A',13,10,0
chip_CS4298	 db 'CS4298',13,10,0
chip_CS4294	 db 'CS4294',13,10,0
chip_CS4299	 db 'CS4299',13,10,0
chip_CS4299D	 db 'CS4299D',13,10,0
chip_CS4201	 db 'CS4201',13,10,0
chip_CS4205	 db 'CS4205',13,10,0
chip_CS4291	 db 'CS4291',13,10,0
chip_CS4202	 db 'CS4202',13,10,0

;Wolfson
chip_WM9700	 db 'WM9704',13,10,0
chip_WM9703	 db 'WM9703/9704',13,10,0
chip_WM9704	 db 'WM9704 (quad)',13,10,0

;VIA
chip_VIA1612A	 db 'VIA1612A',13,10,0

;SigmaTel
chip_STAC9758	 db 'STAC9758,59',13,10,0

;eMicro
chip_EM28028	 db 'EM28028',13,10,0

