;--------------------;
;     MACROS.INC     ;
;--------------------;

macro __mov reg,a {         ; mike.dld
 if ~a eq
   mov reg,a
 end if
}

macro mcall a,b,c,d,e,f {   ; mike.dld
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f
 int   0x40
}

;--------------------;
;      ASCL.INC      ;
;--------------------;

macro words2reg reg,hiword,lowword
{
if hiword eq
   if lowword eq
      ; not changes
   else
      if lowword eqtype 12
	 and reg,dword 0xffff0000
	 add reg,dword lowword
      else
	 and reg,dword 0xffff0000
	 add reg,dword lowword
      end if
   end if
else
   if lowword eq
      if hiword eqtype 12
	 and reg,dword 0x0000ffff
	 add reg,dword hiword*65536
      else
	 shl reg,16
	 add reg,dword hiword
	 ror reg,16
      end if
   else
      if lowword eqtype 12 & hiword eqtype 12
	 if lowword eq 0 & hiword eq 0
	       xor reg,reg
	    else
	       mov reg,dword hiword*65536+lowword
	 end if
      else
	 mov reg,dword hiword
	 shl reg,16
	 add reg,dword lowword
      end if
   end if
end if
}

;--------------------;
;    MOSUZIT.INC     ;
;--------------------;

;WriteTextToWindow
macro Text xStart,yStart,rgbColor,pText,nTextLen
{
    words2reg	ebx, xStart, yStart
    mov 	ecx, rgbColor
    mov 	edx, pText
    mov 	esi, nTextLen
    mov 	eax, 4
    int 	0x40
}

;DisplayNumber
macro Number xStart,yStart,nPrintType,noOfDigits,Data,rgbColor
{
    words2reg	edx,xStart,yStart
    words2reg	ebx,noOfDigits,nPrintType
    mov 	ecx,Data
    mov 	esi,rgbColor
    mov 	eax,47
    int 0x40
}

macro DrawLine xStart,xEnd,yStart,yEnd,rgbColor
{
    words2reg	ebx,xStart,xEnd
    words2reg	ecx,yStart,yEnd
    mov 	edx,rgbColor
    mov 	eax,38
    int 0x40
}
;********************************************************
macro Extracteax temp,fromBit,bitCount,dest
{
    mov temp, eax
    shr temp, fromBit
    and temp, (1 shl bitCount - 1)
    mov dest, temp
}
;********************************************************
;*********  AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM *********
;********************************************************
extract_bits:		  ;11xA is sync
;Defined as the following:
;   b1s    ; standard
;   b1l    ; layer
;   crc    ; CRC protection

;   Bita   ; Bitrate
;   sam    ; Sampling rate
;   pad    ; Padding
;   priv   ; Private bit

;   shan   ; Channel mode
;   modx   ; Mode extention
;   copy   ; Copyright
;   orig   ; Original
;   emph   ; Emphasis

;--------------------- Standard    (BB)
Extracteax ebx, 11, 2, [b1s]
;--------------------- Layer       (CC)
Extracteax ebx,  9, 2, [b1l]
;--------------------- CRC         (D)
Extracteax ebx,  8, 1, [crc]
;--------------------- Bitrate     (EEEE)
Extracteax ebx, 20, 4, [Bita]
;--------------------- Sample Rate (FF)
Extracteax ebx, 18, 2, [sam]
;--------------------- padding     (G)
Extracteax ebx, 17, 1, [pad]
;--------------------- Private bit (H)
Extracteax ebx, 16, 1, [priv]
;--------------------- Channels    (II)
Extracteax ebx, 30, 2, [shan]
;--------------------- Mode ext    (JJ)
Extracteax ebx, 28, 2, [modx]
;--------------------- Copyright   (K)
Extracteax ebx, 27, 1, [copy]
;--------------------- Original    (L)
Extracteax ebx, 26, 1, [orig]
;--------------------- Emphasis    (MM)
Extracteax ebx, 24, 2, [emph]

ret
;********************************************************
decode_standard:

comp0:
cmp [b1s], 00b
je S25

comp1:
cmp [b1s], 01b	  ; unknown
je S25

comp2:
cmp [b1s], 10b
je S20

comp3:
cmp [b1s], 11b
je S10

jmp S000

;-----

S25:
mov [S1], 2
mov [S2], 5
mov [id], 3  ;lowest freq
jmp S000

S20:
mov [S1], 2
mov [S2], 0
mov [id], 2  ;low freq
jmp S000

S10:
mov [S1], 1
mov [S2], 0
mov [id], 1  ;hi freq
jmp S000

S000:

ret

;********************************************************

decode_layer:

com0:
cmp [b1l], 00b
je L3	      ; unknown

com1:
cmp [b1l], 01b
je L3

com2:
cmp [b1l], 10b
je L2

com3:
cmp [b1l], 11b
je L1

jmp L000

;-----

L3:
mov [La], 3
jmp L000

L2:
mov [La], 2
jmp L000

L1:
mov [La], 1
jmp L000

L000:

ret

;********************************************************

decode_channels:

co0:
cmp [shan], 00b
je C4

co1:
cmp [shan], 01b
je C3

co2:
cmp [shan], 10b
je C2

co3:
cmp [shan], 11b
je C1

jmp C000

;-----
C4:
mov [K], 4 ;00=Stereo
jmp C000

C3:
mov [K], 3 ;01=Joint stereo
jmp C000

C2:
mov [K], 2 ;10=Dual Channel
jmp C000

C1:
mov [K], 1 ;11=Single Channel (Mono)
jmp C000

C000:

ret
;********************************************************
;--------------------------
;  decode_samplerate
;--------------------------
decode_samplerate:
   push  eax
   push  ecx

   mov	 eax, [sam]
   mov	 eax, [freq + eax*4]
   mov	 ecx, [b1s]
   cmp	 cl, 01      ; bad frequency
   jnz	 ok

ok:
   cmp	 cl, 0
   jz	 @f
   dec	 cl
@@:
   shl	 eax, cl
   mov	 [SR], eax

   pop	 ecx
   pop	 eax
ret

;********************************************************
;--------------------------
;  decode_bitrate
;--------------------------
decode_bitrate:
   push  eax
   push  ebx
   mov	 eax,[b1s]
   and	 eax,1
   shl	 eax,4
   lea	 eax,[eax*3]
   mov	 ebx,[b1l]
   dec	 ebx
   shl	 ebx, 4
   add	 eax, ebx
   add	 eax,[Bita]
   movzx eax,byte[Bitrate+eax]
   shl	 eax,3
   mov	 [BR],eax
   pop	 ebx
   pop	 eax
ret

;********************************************************

calculate_time_frame_count:

;----------------------------duration
xor eax, eax
xor ebx, ebx
xor ecx, ecx
xor edx, edx

mov ebx, dword[BR]
imul ebx, 125
; result in ebx


mov eax, dword[size]

div ebx


mov dword[time], eax
;---------------------------frame size

xor eax, eax
xor ebx, ebx
xor ecx, ecx
xor edx, edx

cmp [S1], 1
je MP1
cmp [S1], 2
je MP2

MP1:

mov eax, dword [BR]
imul eax, 144000
; result in eax


mov ebx, dword[SR]

div ebx

add eax, dword [pad]

mov dword[frames], eax

jmp exx

MP2:

mov eax, dword [BR]
imul eax, 72000
; result in eax


mov ebx, dword[SR]

div ebx

add eax, dword [pad]

mov dword[frames], eax

exx:
;--------------------------quantity of frames
xor eax, eax
xor ebx, ebx
xor ecx, ecx
xor edx, edx

mov eax, dword[size]
mov ebx, dword[frames]
div ebx
mov dword[framecount], eax

ret

;--------------------------
;  header_check
;  eax = проверяемый заголовок
;--------------------------
Header_Check:

   push  ebx
   mov	 ebx, eax

   ; Проверяем 11 бит в начале фрейма
   and	 eax, 0E0FFh
   cmp	 eax, 0E0FFh
   jnz	 bad

   ; Проверяем версию MPEG
   mov	 eax, ebx
   and	 eax, 1800h
   cmp	 eax, 0800h
   jz	 bad

   ; Проверяем layer
   mov	 eax, ebx
   test  eax, 0600h
   jz	 bad

   ; Проверяем битрейт
   mov	 eax, ebx
   and	 eax, 0F00000h
   cmp	 eax, 0F00000h
   jz	 bad
   test  eax, eax
   jz	 bad

   ; Проверяем частоту
   mov	 eax, ebx
   and	 eax, 0C0000h
   cmp	 eax, 0C0000h
   jz	 bad
.end_test:
   xor	 eax, eax
   pop	 ebx
   ret

bad:
   mov	 eax, 1
   pop	 ebx
   ret


;===========================
Win2Dos:
;===========================
	jcxz	.no_chars
	push	esi
	push	edi
	push	ecx
	mov	edi, esi
	cld
.next_char:
	lodsb
	cmp	al, 0xC0
	jae	.rus
	cmp	al, 0xA8
	jnz	@f
	mov	al, 0xF0
@@:
	cmp	al, 0xB8
	jnz	@f
	mov	al, 0xF1
@@:
	jmp	.char_ready
.rus:
	cmp	al, 0xF0
	jae	@f
	sub	al, 48
@@:
	sub	al, 16
.char_ready:
	stosb
	dec	ecx
	jnz	.next_char

	pop	ecx
	pop	edi
	pop	esi
.no_chars:
	ret