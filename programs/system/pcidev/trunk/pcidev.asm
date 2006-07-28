;***************************************************************
; project name:    PCI Device Enumeration
; target platform: MenuetOS, x86 (IA-32), x86-64 achitectures
; compiler:        flat assmebler 1.60
; version:         1.25
; last update:     2th October 2005
; maintained by:   Sergey Kuzmin aka Wildwest
; e-mail:          kuzmin_serg@list.ru
; project site:    http://coolthemes.narod.ru/pcidev.html
;***************************************************************
;Summary: This program will attempt to scan the PCI Bus
;        and display basic information about each device
;        connected to the PCI Bus.
;***************************************************************
;HISTORY:
;keep dates in european format (dd/mm/yyyy), please
; '!' means big changes
; to-do:
;PCI version should be normalized (0210->02.10)
;----------------------------------------------------------------
;1.25: PCIDEV   02/10/2005
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:  !Description is based on Class and SubClass now (PCI 3.0)
;           deleted label Descriptions (names of Classes)
;           names of Classes and SubClasses are in the end of Vendors.inc
;----------------------------------------------------------------
;1.20: PCIDEV   16/08/2005
;Author:    Victor Alberto Gil Hanla a.k.a. vhanla <vhanla@gmail.com>
;Features: !added many vendor lists (865)
;           previous version's list deleted
;           previous Company Name searching and printing changed
;NOTE:
;           It was only tested in my old PII Computer,
;           there might be some bugs... PLZ test it and send comments
;----------------------------------------------------------------
;1.15: PCIDEV   03/06/2005
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:  added quantity of devices,
;         ! detection of Company Name based on Vendor ID,
;           database of VenID (35 ID's),
;           macros.inc for smaller size,
;           interface+(skinned window),
;           VenID before DevID in 'table'(was DevID before VenID)
;----------------------------------------------------------------
;1.0: PCIDEV    30/01/2005
;Author:    Jason Delozier
;Features:  able to detect PCI version, quantity of PCI buses,
;           Vendor&Device ID for appropriate Device on Bus;
;           detect Revision, Class and Subclass of Device,
;           and make Description based on Class
;-------------------------------------------------------------
use32

	       org    0x0

	       db     'MENUET01'; 8 byte id
	       dd     0x01		; header version
	       dd     START		; start of code
	       dd     I_END		; size of image
	       dd     0xE000	; memory for app E000
	       dd     0xE000	; esp E000
	       dd     0x0 , 0x0 ; I_Param , I_Icon

include 'macros.inc'
include 'VENDORS.INC'

START:				; start of execution
     call draw_window

still:
    mov  eax,10 		; wait here for event
    int  0x40

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    jmp  still

  red:				; redraw
    call draw_window
    jmp  still
  key:				; key
    mov  eax,2			; just read it and ignore
    int  0x40
    jmp  still
  button:			; button
    mov  eax,17 		; get id
    int  0x40

    cmp  ah,1			; button id=1 ?
    jne  noclose

    mov  eax,-1 		; close this program
    int  0x40
  noclose:
    jmp  still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    int  0x40

				   ; DRAW WINDOW
    mov  eax,0			   ; function 0 : define and draw window
    mov  ebx,20*65536+750	   ; [x start] *65536 + [x size]
    mov  ecx,20*65536+550	   ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff 	   ; color of work area RRGGBB,8->color gl
    mov  esi,0x805080d0 	   ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005080d0 	   ; color of frames    RRGGBB
    int  0x40

				   ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff 	   ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt		   ; pointer to text beginning
    mov  esi,labellen-labelt	   ; text length
    int  0x40

    mov [total],0

    mov ebx, 20*65536+25	   ;Draw work format
    mov ecx, 0x224466
    mov edx, PCIWin
    mov esi, 106 ; lenght of line
newline:
    mov eax, 4
    int 0x40
    add ebx, 10
    add edx, 106  ; lenght of line 
    cmp [edx], byte 'x'
    jne newline


    call Get_PCI_Info

    mov cx, [PCI_Version]
    mov eax, 47
    mov ebx, 0x00040100
    mov esi, 0x00000000
    mov edx, 110*65536+45
    int 0x40

    mov cl, [PCI_LastBus]
    mov eax, 47
    mov ebx, 0x00020100
    mov esi, 0x00000000
    mov edx, 110*65536+55
    int 0x40

   call scan_Pci

    xor ecx, ecx
	mov cl, [total]
    mov eax, 47
    mov ebx, 0x00020000
    mov esi, 0x00000000
    mov edx, 150*65536+65
    int 0x40


    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    int  0x40

    ret
;   ***********************************************
;   *******  END WINDOW DEFINITIONS & DRAW  *******
;   ***********************************************

Get_PCI_Info:
    mov eax, 62
    mov ebx, 0
    int 0x40
    mov [PCI_Version], ax
    ;mov [PCI_Version_h], ah
    ;mov [PCI_Version_l], al

    mov eax, 62
    mov ebx, 1
    int 0x40
    mov [PCI_LastBus], al
ret



scan_Pci:
   cmp [PCI_LastBus],0xff
   jne Pci_Exists
   ret
Pci_Exists:
   mov [V_Bus], 0	  ;reset varibles
   mov [V_Dev], 0	  ;
   mov edx,  20*65536+110 ;start write position

Start_Enum:
   mov bl, 6	     ;get Device ID / Vendor ID
   mov bh, [V_Bus]
   mov ch, [V_Dev]
   mov cl, 0
   mov eax, 62
   int 0x40

   cmp ax, 0	     ;check next slot if nothing exists
   je nextDev
   cmp ax, 0xffff
   je nextDev

   mov [PCI_Vendor], ax
   shr eax, 16
   mov [PCI_Device], ax

   mov eax, 62		  ;PCI Sys Function
   mov bl, 4		  ;Read config byte
   mov bh, [V_Bus]	  ;Bus #
   mov ch, [V_Dev]	  ;Device # on bus
   mov cl, 0x08 	  ;Register to read (Get Revision)
   int 0x40		  ;Read it
   mov [PCI_Rev], al	  ;Save it

   mov eax, 62		  ;PCI Sys Function
   mov cl, 0x0b 	  ;Register to read (Get class)
   int 0x40		  ;Read it
   mov [PCI_Class], al	  ;Save it

   mov eax, 62		  ;PCI Sys Function
   mov cl, 0x0a 	  ;Register to read (Get Subclass)
   int 0x40		  ;Read it
   mov [PCI_SubClass], al ;Save it

   call Print_New_Device

nextDev:
   inc [V_Dev]
   jno Start_Enum	  ;jump if no overflow (<256)

   mov [V_Dev], 0
   inc [V_Bus]
   mov al, [PCI_LastBus]

   cmp byte [V_Bus], al
   jbe Start_Enum
ret

Print_New_Device:
   inc [total]

   mov eax, 47		  ;Write number to screen
   mov ebx,0x00040100	  ;4 byte number, print in hexidecimal
   mov esi, 0x00000000	  ;Color of text
   xor ecx,ecx
   mov	cx,[PCI_Vendor]   ;Pointer to number to be written
   int 0x40		  ;Write it

   push dx		  ;Calculate position
   mov dx, 62		  ;
   shl edx, 16		  ;
   pop dx		  ;
   mov cx, [PCI_Device]   ;Write Vendor ID
   int 0x40		  ;

   mov ebx, 0x00020100	  ;2 byte number, in hexidecimal format
   push dx		  ;move write cursor
   mov dx, 110		  ;
   shl edx, 16		  ;
   pop dx		  ;
   mov cl, [V_Bus]	  ;write bus number
   int 0x40		  ;

   push dx		  ;move write cursor
   mov dx, 146		  ;
   shl edx, 16		  ;
   pop dx		  ;
   mov cl, [V_Dev]	  ;Write device number
   int 0x40		  ;

   push dx		  ;move write cursor
   mov dx, 179		  ;
   shl edx, 16		  ;
   pop dx		  ;
   mov cl, [PCI_Rev]	  ;Draw Revision to screen
   int 0x40		  ;

   push dx		  ;move write cursor
   mov dx, 215		  ;
   shl edx, 16		  ;
   pop dx		  ;
   mov cl, [PCI_Class]	  ;Draw Class to screen
   int 0x40		  ;

   push dx		  ;move write curson
   mov dx, 266		  ;
   shl edx, 16		  ;
   pop dx		  ;
   mov cl, [PCI_SubClass] ;Draw Sub Class to screen
   int 0x40		  ;


			  ;Write Names
   push dx		  ;remember our current write line
   mov esi, 17		  ;length of text
   mov bx, 310		  ;X-position of text on screen
   shl ebx, 16		  ;
   mov bx, dx		  ;Current Y position
   mov ecx, 0		  ;color of text

   mov [cur], dx

   mov [r],0
;**********************************************************
; modified part by vhanla (I know it is not a fastest way to search)
; it needs optimization... HELP this project!
;-------------------------------------------------------------------------------
  cmp [PCI_Vendor],4807 ; Check if Vendor's value is less than this number
   jae next1		; if it is less, let's continue, or jump to next1
   mov [listcount],255	; number of available lists into this range
   mov [r],1		; counter
   mov [valor],50	; List's number of characters = 50
  rep1:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[_FIRSTPART+bx]
   mov [address],_FIRSTPART
   add [address],ebx
   sub [address],50
   pop ebx
   cmp [PCI_Vendor],ax
   jz ex
   add [valor],52	; Number of characters plus value (WORD) 50+2 bytes
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next1
   inc [r]
   jmp rep1
;-------------------------------------------------------------------------------
next1:
  cmp [PCI_Vendor],5320
   jae next2
   mov [listcount],255	; number of available lists into this range
   mov [r],1
   mov [valor],50
  rep2:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[_SECONDPART+bx]
   mov [address],_SECONDPART
   add [address],ebx
   sub [address],50
   pop ebx
   cmp [PCI_Vendor],ax
   jz ex
   add [valor],52
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next2
   inc [r]
   jmp rep2
;-------------------------------------------------------------------------------
next2:
  cmp [PCI_Vendor],5580
   jae next3
   mov [listcount],255	; number of available lists into this range
   mov [r],1
   mov [valor],50
  rep3:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[_THIRDPART+bx]
   mov [address],_THIRDPART
   add [address],ebx
   sub [address],50
   pop ebx
   cmp [PCI_Vendor],ax
   jz ex
   add [valor],52
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next3
   inc [r]
   jmp rep3
;-------------------------------------------------------------------------------
next3:
  cmp [PCI_Vendor],65535 ;the last in a WORD
   jae next4
   mov [listcount],100	; number of available lists into this range
   mov [r],1
   mov [valor],50
  rep4:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[_FOURTHPART+bx]
   mov [address],_FOURTHPART
   add [address],ebx
   sub [address],50
   pop ebx
   cmp [PCI_Vendor],ax
   jz ex
   add [valor],52
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next4
   inc [r]
   jmp rep4
;-------------------------------------------------------------------------------
next4:

;-------------------------------------------------------------------------------
; tHIS PART FOR OTHER ... LISTS
   mov [listcount],7  ; number of available lists into this range
   mov [r],1
   mov [valor],50
  repO:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[PREVIOUSVERSIONLIST+bx]
   mov [address],PREVIOUSVERSIONLIST
   add [address],ebx
   sub [address],50
   pop ebx
   cmp [PCI_Vendor],ax
   jz ex
   add [valor],52
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next5
   inc [r]
   jmp repO
;-------------------------------------------------------------------------------
next5:
   mov [address],_UNKNOWN
;-------------------------------------------------------------------------------
  ex:
;  lets print the vendor Name

   mov eax,4
   mov edx,[address]
   mov esi,50
   int 0x40
;-------------------------------------------------------------------------------
; END OF SEARCHING AND PRINTING
;------------------------------------------------------------------

  ; mov esi, 26		  ;length of text
  ; mov bx, 560		  ;X-position of text on screen
  ; shl ebx, 16		  ;
  ; mov bx, [cur]	  ;Current Y position
  ; mov ecx, 0		  ;color of text


  ; xor eax, eax 	  ;calculate pointer to string
 ;  mov al, [PCI_Class]	  ;Determine Description based on class
 ;  mov dl,  26		  ;Each description is 26 characters long
 ;  mul dl		  ;get offset to correct description
 ;  mov edx, Descriptions  ;starting point of description text
 ;  add edx, eax 	  ;calculated corresponding description
 ;  mov eax, 4		  ;draw text sys function
 ;  int 0x40
 
   cmp [PCI_Class], 00h
   je sub0
   cmp [PCI_Class], 01h
   je sub1
   cmp [PCI_Class], 02h
   je sub2
   cmp [PCI_Class], 03h
   je sub3
   cmp [PCI_Class], 04h
   je sub4
   cmp [PCI_Class], 05h
   je sub5
   cmp [PCI_Class], 06h
   je sub6
   cmp [PCI_Class], 07h
   je sub7
   cmp [PCI_Class], 08h
   je sub8
   cmp [PCI_Class], 09h
   je sub9
   cmp [PCI_Class], 0Ah
   je sub10
   cmp [PCI_Class], 0Bh
   je sub11
   cmp [PCI_Class], 0Ch
   je sub12
   cmp [PCI_Class], 0Dh
   je sub13
   cmp [PCI_Class], 0Eh
   je sub14
   cmp [PCI_Class], 0Fh
   je sub15
   cmp [PCI_Class], 10h
   je sub16
   cmp [PCI_Class], 11h
   je sub17
   jmp endd
   
sub0:
   mov [listcount],2  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu0:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class0+bx]
   mov [address],Class0
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu0
  
jmp endd   

sub1:
   mov [listcount],8  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu1:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class1+bx]
   mov [address],Class1
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu1
jmp endd   


sub2:
   mov [listcount],8  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu2:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class2+bx]
   mov [address],Class2
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu2
jmp endd   

sub3:
   mov [listcount],4  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu3:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class3+bx]
   mov [address],Class3
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu3
jmp endd   


sub4:
   mov [listcount],4  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu4:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class4+bx]
   mov [address],Class4
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu4
jmp endd   

sub5:
   mov [listcount],3  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu5:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class5+bx]
   mov [address],Class5
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu5
jmp endd   

sub6:
   mov [listcount],12  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu6:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class6+bx]
   mov [address],Class6
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
 jz next50
   inc [r]
   jmp repu6
jmp endd  

sub7:
   mov [listcount],7  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu7:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class7+bx]
   mov [address],Class7
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
    jz next50
   inc [r]
   jmp repu7
jmp endd   

sub8:
   mov [listcount],8  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu8:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class8+bx]
   mov [address],Class8
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
    jz next50
   inc [r]
   jmp repu8
jmp endd    

sub9:
   mov [listcount],6  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu9:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class9+bx]
   mov [address],Class9
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu9
jmp endd   

sub10:
   mov [listcount],2  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu10:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[ClassA+bx]
   mov [address],ClassA
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
  jz next50
   inc [r]
   jmp repu10
jmp endd   

sub11:
   mov [listcount],7  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu11:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[ClassB+bx]
   mov [address],ClassB
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
    jz next50
   inc [r]
   jmp repu11
jmp endd   

sub12:
   mov [listcount],10  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu12:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[ClassC+bx]
   mov [address],ClassC
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu12
jmp endd   

sub13:
   mov [listcount],8  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu13:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[ClassD+bx]
   mov [address],ClassD
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
    jz next50
   inc [r]
   jmp repu13
jmp endd   

sub14:
   mov [listcount],1  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu14:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[ClassE+bx]
   mov [address],ClassE
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu14
jmp endd  

sub15:
   mov [listcount],4  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu15:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[ClassF+bx]
   mov [address],ClassF
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu15
jmp endd  

sub16:
   mov [listcount],3  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu16:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class10+bx]
   mov [address],Class10
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu16
jmp endd  

sub17:
   mov [listcount],5  ; number of available lists into this range
   mov [r],1
   mov [valor],32
  repu17:
   push ebx
   xor ebx,ebx
   mov bx,[valor]
   mov ax,word[Class11+bx]
   mov [address],Class11
   add [address],ebx
   sub [address],32
   pop ebx
   cmp word [PCI_SubClass],ax
   jz endd 
   add [valor],34
   push eax
   mov ah,[r]
   cmp [listcount],ah
   pop eax
   jz next50
   inc [r]
   jmp repu17
jmp endd  

next50:
   mov [address],_UNKNOWND
   
endd:
  
   xor eax, eax
   xor ebx, ebx
   xor ecx, ecx
   xor edx, edx
   
   mov bx, 560		  ;X-position of text on screen
   shl ebx, 16		  ;
   mov bx, [cur]	  ;Current Y position
   mov ecx, 0		  ;color of text

   mov eax,4
   mov edx,[address]
   mov esi,32
   int 0x40
 
   mov dx, 20		  ;Starting Y coordinate
   shl edx, 16		  ;
   pop dx		  ;
   add edx, 10		  ;Next line down

ret



; DATA AREA

labelt:
       db 'PCI Device Enumeration v 1.25 by J. Delozier, S. Kuzmin and V. Hanla'
labellen:

PCIWin:
db 'Please remember to enable PCI Access to Applications in Setup Menu.                                       '
db '                                                                                                          '
db 'PCI Version  =                                                                                            '
db 'Last PCI Bus =                                                                                            '
db 'Quantity of devices =                                                                                     '
db '                                                                                                          '
db 'VenID  DevID  Bus#  Dev#  Rev  Class  Subclass                 Company                      Description   '
db '-----  -----  ----  ----  ---  -----  -------- ------------------------------------------ ----------------'
db 'x'

valor dw ?
r db ?
cur dw ?
total db ?
address dd ?
listcount db ?
V_Bus	     db 0
V_Dev	     db 0
PCI_Version dw 0
PCI_LastBus  db 0
PCI_Device   dw 0
PCI_Vendor   dw 0
PCI_Bus      db 0
PCI_Dev      db 0
PCI_Rev      db 0
PCI_Class    db 0
PCI_SubClass db 0

I_END:
