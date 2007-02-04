; optimize the code for size
macro add arg1,arg2
 {
   if arg1 in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
      if arg2 eqtype 0
	 if arg2 = 1
	    inc arg1
	 else
	    add arg1,arg2
	 end if
      else
	 add arg1,arg2
      end if
   else
      add arg1,arg2
   end if
 }

macro sub arg1,arg2
 {
   if arg2 eqtype 0
      if arg2 = 1
	 dec arg1
      else
	 sub arg1,arg2
      end if
   else
      sub arg1,arg2
   end if
 }

macro mov arg1,arg2
 {
   if arg1 in <eax,ebx,ecx,edx,esi,edi,ebp,esp>
      if arg2 eqtype 0
	 if arg2 = 0
	    xor arg1,arg1
	 else if arg2 = 1
	    xor arg1,arg1
	    inc arg1
	 else if arg2 = -1
	    or	arg1,-1
	 else if arg2 > -128 & arg2 < 128
	    push arg2
	    pop  arg1
	 else
	    mov  arg1,arg2
	 end if
      else
	 mov arg1,arg2
      end if
   else
      mov arg1,arg2
   end if
 }


; Interface

macro Window xStart,yStart,xSize,ySize,bColor,gColor,fColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xSize
 mov ecx,yStart
 shl ecx,16
 add ecx,ySize
 mov edx,bColor
 mov esi,gColor
 mov edi,fColor
 xor eax,eax
 int 0x40
}


;WriteTextToWindow
macro Text xStart,yStart,rgbColor,pText,nTextLen
{
 mov ebx,xStart
 shl ebx,16
 add ebx,yStart
 mov ecx,rgbColor
 mov edx,pText
 mov esi,nTextLen
 mov eax,4
 int 0x40
}

;DisplayNumber
macro Number xStart,yStart,nPrintType,noOfDigits,Data,rgbColor
{

 mov edx,xStart
 shl edx,16
 add edx,yStart
 mov ebx,noOfDigits
 shl ebx,16
 or ebx,nPrintType
 mov ecx,Data
 mov esi,rgbColor
 mov eax,47
 int 0x40
}

macro DrawLine xStart,xEnd,yStart,yEnd,rgbColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xEnd
 mov ecx,yStart
 shl ecx,16
 add ecx, yEnd
 mov edx,rgbColor
 mov eax,38
 int 0x40
}

macro PutImage xPos,yPos,xImage,yImage,pImage
{
 mov ecx,xImage
 shl ecx,16
 add ecx, yImage
 mov edx,xPos
 shl edx,16
 add edx,yPos
 mov ebx,pImage
 mov eax,7
 int 0x40
}

macro Button xStart,yStart,xSize,ySize,nID,rgbColor
{
 mov ebx,xStart
 shl ebx,16
 add ebx,xSize
 mov ecx,yStart
 shl ecx,16
 add ecx,ySize
 mov edx,nID
 mov esi,rgbColor
 mov eax,8
 int 0x40
}

macro CreateTread EntryPoint,StackPos
{
 xor ebx,ebx
 inc ebx
 mov ecx,EntryPoint
 mov edx,StackPos
 mov eax,51
 int 0x40
}


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
cmp bl, 00h
ja rr
je cnnn1

rr:
mov byte [brand], bl ;
Text 275,210,0x00000000,abrand00, abrand00len-abrand00
jmp B000
;jmp comppp

cnnn1:

mov	eax, 0x80000001 	      ; CPUID ext. function 0x80000001
cpuid

cmp ebx, 0
je Bi00
mov word [brand], bx ;

comppp:
cmp  [brand], 400h
jl res1
cmp  [brand], 500h
jl res2
jae res3


res1:
Text 275,210,0x00000000,abrand0, abrand0len-abrand0
jmp B000

res2:
Text 275,210,0x00000000,abrand1, abrand1len-abrand1
jmp B000

res3:
Text 275,210,0x00000000,abrand2, abrand2len-abrand2
jmp B000
	     ;;;;;;;;;;;;;;;;;;;;; intel brand
intel_br:
      xor eax,eax
      inc eax
      cpuid
cmp0:
cmp bl, 00h
je Bi00

cmp1:
cmp bl, 01h
je Bi01

cmp2:
cmp bl, 02h
je Bi02

cmp3:
cmp bl, 03h
je Bi03

cmp4:
cmp bl, 04h
je Bi04

cmp6:
cmp bl, 06h
je Bi06

cmp7:
cmp bl, 07h
je Bi07

cmp8:
cmp bl, 08h
je Bi08

cmp9:
cmp bl, 09h
je Bi09

cmpA:
cmp bl, 0Ah
je Bi0A

cmpB:
cmp bl, 0Bh
je Bi0B

cmpC:
cmp bl, 0Ch
je Bi0C

cmpE:
cmp bl, 0Eh
je Bi0E

cmpF:
cmp bl, 0Fh
je Bi0F

cmp11:
cmp bl, 11h
je Bi11

cmp12:
cmp bl, 12h
je Bi12

cmp13:
cmp bl, 13h
je Bi13

cmp14:
cmp bl, 14h
je Bi14

cmp15:
cmp bl, 15h
je Bi15

cmp16:
cmp bl, 16h
je Bi16

cmp17:
cmp bl, 17h
je Bi17

jne Bi00

;------------------
Bi00:
mov byte [brand], bl ;00h
Text 65,270,0x00000000,brand0, brand0len-brand0
;Number 275,200,1*256,2,dword [brand],0x000000     was in v. 1.11
jmp B000

Bi01:
mov [brand], 01h
Text 65,270,0x00000000,brand01, brand01len-brand01
jmp B000

Bi02:
mov [brand], 02h
Text 65,270,0x00000000,brand02, brand02len-brand02
jmp B000

Bi03:
mov [brand], 03h

cmp [m], 8
je E3
Text 65,270,0x00000000,brand03, brand03len-brand03
jmp B000
E3:
Text 65,270,0x00000000,brand03d, brand03dlen-brand03d
jmp B000

Bi04:
mov [brand], 04h
Text 65,270,0x00000000,brand04, brand04len-brand04
jmp B000

Bi06:
mov [brand], 06h
Text 65,270,0x00000000,brand06, brand06len-brand06
jmp B000

Bi07:
mov [brand], 07h
Text 65,270,0x00000000,brand07, brand07len-brand07
jmp B000

Bi08:
mov [brand], 08h
Text 65,270,0x00000000,brand08, brand08len-brand08
jmp B000

Bi09:
mov [brand], 09h
Text 65,270,0x00000000,brand09, brand09len-brand09
jmp B000

Bi0A:
mov [brand], 0Ah
Text 65,270,0x00000000,brand0A, brand0Alen-brand0A
jmp B000

Bi0B:
mov [brand], 0Bh

cmp [m], 13
jl Eb
Text 65,270,0x00000000,brand0B, brand0Blen-brand0B
jmp B000
Eb:
Text 65,270,0x00000000,brand0Bd, brand0Bdlen-brand0Bd
jmp B000

Bi0C:
mov [brand], 0Ch
Text 65,270,0x00000000,brand0C, brand0Clen-brand0C
jmp B000

Bi0E:
mov [brand], 0Eh

cmp [m], 13
jl Ed
Text 65,270,0x00000000,brand0E, brand0Elen-brand0E
jmp B000
Ed:
Text 65,270,0x00000000,brand0Ed, brand0Edlen-brand0Ed
jmp B000

Bi0F:
mov [brand], 0Fh
Text 65,270,0x00000000,brand0F, brand0Flen-brand0F
jmp B000

Bi11:
mov [brand], 11h
Text 65,270,0x00000000,brand11, brand11len-brand11
jmp B000

Bi12:
mov [brand], 12h
Text 65,270,0x00000000,brand12, brand12len-brand12
jmp B000

Bi13:
mov [brand], 13h
Text 65,270,0x00000000,brand13, brand13len-brand13
jmp B000

Bi14:
mov [brand], 14h
Text 65,270,0x00000000,brand14, brand14len-brand14
jmp B000

Bi15:
mov [brand], 15h
Text 65,270,0x00000000,brand15, brand15len-brand15
jmp B000

Bi16:
mov [brand], 16h
Text 65,270,0x00000000,brand16, brand16len-brand16
jmp B000

Bi17:
mov [brand], 17h
Text 65,270,0x00000000,brand17, brand17len-brand17
jmp B000

B000:

ret