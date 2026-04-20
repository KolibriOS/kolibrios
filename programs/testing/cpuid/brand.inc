;//////////////////////////////////////////////
;Decoding Brand ID for Intel



decodebrand:


cmp dword[smallvendor], 'cAMD'
jz amd_br
cmp dword[smallvendor], 'ntel'
jz intel_br
jmp Bi00   ; if not AMD or Intel

amd_br:      ;;;;;;;;;;;;;;;;;;;;; amd brand

xor eax,eax
inc eax
cpuid

cnnn0:
test bl, bl
jz cnnn1

rr:
mov byte [brand], bl ;
Text 60,250,0x00000000,abrand00, abrand00len-abrand00
ret
;jmp comppp

cnnn1:

mov	eax, 0x80000001 	      ; CPUID ext. function 0x80000001
cpuid

test ebx, ebx
je Bi00
mov [brand], ebx ;

comppp:
cmp  byte [brand+1], 4
jb   res1
jz   res2
ja   res3


res1:
Text 60,250,0x00000000,abrand0, abrand0len-abrand0
ret

res2:
Text 60,250,0x00000000,abrand1, abrand1len-abrand1
ret

res3:
Text 60,250,0x00000000,abrand2, abrand2len-abrand2
ret
	     ;;;;;;;;;;;;;;;;;;;;; intel brand
intel_br:
      xor eax,eax
      inc eax
      cpuid
      movzx ebx, bl
      mov [brand], ebx
cmp0:
test ebx, ebx
je Bi00

cmp1:
dec ebx
je Bi01

cmp2:
dec ebx
je Bi02

cmp3:
dec ebx
je Bi03

cmp4:
dec ebx
je Bi04

cmp6:
dec ebx
dec ebx
je Bi06

cmp7:
dec ebx
je Bi07

cmp8:
dec ebx
je Bi08

cmp9:
dec ebx
je Bi09

cmpA:
dec ebx
je Bi0A

cmpB:
dec ebx
je Bi0B

cmpC:
dec ebx
je Bi0C

cmpE:
dec ebx
dec ebx
je Bi0E

cmpF:
dec ebx
je Bi0F

cmp11:
dec ebx
dec ebx
je Bi11

cmp12:
dec ebx
je Bi12

cmp13:
dec ebx
je Bi13

cmp14:
dec ebx
je Bi14

cmp15:
dec ebx
je Bi15

cmp16:
dec ebx
je Bi16

cmp17:
dec ebx
je Bi17

;------------------
Bi00:
Text 60,250,0x00000000,brand0, brand0len-brand0
;Number 270,180,1*256,2,dword [brand],0x000000     was in v. 1.11
ret

Bi01:
Text 60,250,0x00000000,brand01, brand01len-brand01
ret

Bi02:
Text 60,250,0x00000000,brand02, brand02len-brand02
ret

Bi03:

cmp [m], 8
je E3
Text 60,250,0x00000000,brand03, brand03len-brand03
ret
E3:
Text 60,250,0x00000000,brand03d, brand03dlen-brand03d
ret

Bi04:
Text 60,250,0x00000000,brand04, brand04len-brand04
ret

Bi06:
Text 60,250,0x00000000,brand06, brand06len-brand06
ret

Bi07:
Text 60,250,0x00000000,brand07, brand07len-brand07
ret

Bi08:
Text 60,250,0x00000000,brand08, brand08len-brand08
ret

Bi09:
Text 60,250,0x00000000,brand09, brand09len-brand09
ret

Bi0A:
Text 60,250,0x00000000,brand0A, brand0Alen-brand0A
ret

Bi0B:

cmp [m], 13
jl Eb
Text 60,250,0x00000000,brand0B, brand0Blen-brand0B
ret
Eb:
Text 60,250,0x00000000,brand0Bd, brand0Bdlen-brand0Bd
ret

Bi0C:
Text 60,250,0x00000000,brand0C, brand0Clen-brand0C
ret

Bi0E:

cmp [m], 13
jl Ed
Text 60,250,0x00000000,brand0E, brand0Elen-brand0E
ret
Ed:
Text 60,250,0x00000000,brand0Ed, brand0Edlen-brand0Ed
ret

Bi0F:
Text 60,250,0x00000000,brand0F, brand0Flen-brand0F
ret

Bi11:
Text 60,250,0x00000000,brand11, brand11len-brand11
ret

Bi12:
Text 60,250,0x00000000,brand12, brand12len-brand12
ret

Bi13:
Text 60,250,0x00000000,brand13, brand13len-brand13
ret

Bi14:
Text 60,250,0x00000000,brand14, brand14len-brand14
ret

Bi15:
Text 60,250,0x00000000,brand15, brand15len-brand15
ret

Bi16:
Text 60,250,0x00000000,brand16, brand16len-brand16
ret

Bi17:
Text 60,250,0x00000000,brand17, brand17len-brand17
ret
