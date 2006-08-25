;***************************************************************
; project name:    PCI Device Enumeration
; target platform: KolibriOS and MenuetOS
; compiler:        flat assmebler 1.63.2
; version:         1.31
; last update:     13(th) May 2006
; maintained by:   Jason Delozier (cordata51)
; e-mail:          cordata51@hotmail.com
; project site:    http://www.asmfreaks.com/menuetos
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
;1.31: PCIDEV   13/05/2006
;Author:    Jason Delozier <cordata51@hotmail.com>
;Features:
;           fixed
;                 * ! bug in Company Name look up code that would cause Unknown Name errors.
;                 * ! possible bugs, many instructions missing byte, word, dword prefixes
;                 * ! possible bug which could have occured after removing  "PREVIOUSVERSIONLIST"
;                     entry in loop up code and not fixing jump parameters.
;           added
;                 *  comments to various parts of the code
;           optimized
;                 * various parts of the source, too many to remember and mention.
;                 * changed entries for Subclasses in vendors.inc to Byte format, saves a little space.
;----------------------------------------------------------------
;1.30: PCIDEV   11/05/2006
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;           added
;                * 3 new vendor id's (ESS from Madis Kalme and 2 id's
;                  forgotten from 1.15 release: Broadcom -SiByte and Chaintech Comp.)
;                       changed
;                            * I don't know why other devs (Jason or Victor) changed window style
;                  to old ugly one, so I changed it back to skinned type 3.
;                * the same goes to the use of macroc.inc - it is enabled again.
;           deleted
;                * there is no more label "PREVIOUSVERSIONLIST" - id's moved to the
;                  appropriate parts of global list.
;----------------------------------------------------------------
;1.29: PCIDEV   30/04/2006
;Author:    Jason Delozier <cordata51@hotmail.com>
;Features:
;           fixed
;                 * ! bug that would not allow devices with device
;                   numbers > 16 to be displayed.
;           added
;                 * ! another heading called "FNC" (function) which allows
;                   the multipurpose Device/Function varible to be split and
;                   displayed to the user properly.
;                 * horizontal bars to display for easier reading.
;           optimized
;                 * vendor/description search routines for speed and space.
;----------------------------------------------------------------
;1.25: PCIDEV   02/10/2005
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;            changed
;                                       * ! Description is based on Class and SubClass
;                     now (PCI 3.0). The Names of Classes and SubClasses
;                     are in the end of Vendors.inc
;            deleted
;                  * label "Descriptions" (names of Classes)
;
;----------------------------------------------------------------
;1.20: PCIDEV   16/08/2005
;Author:    Victor Alberto Gil Hanla a.k.a. vhanla <vhanla@gmail.com>
;Features:
;           added
;                  * ! many vendor lists (865)
;           deleted
;                  * previous version's list
;           changed
;                  * previous Company Name searching and printing
;----------------------------------------------------------------
;1.15: PCIDEV   03/06/2005
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Features:
;           added
;                                  * quantity of devices,
;                                  * ! detection of Company Name based on Vendor ID,
;                                  * database of VenID (35 ID's),
;                                  * macros.inc for smaller size,
;           changed
;                                  * interface+(skinned window),
;                                  * VenID before DevID in 'table'(was DevID before VenID)
;----------------------------------------------------------------
;1.0: PCIDEV    30/01/2005
;Author:    Jason Delozier
;Features:
;          able to
;                 * detect PCI version,
;                 * quantity of PCI buses,
;                 * Vendor&Device ID for appropriate Device on Bus;
;                 * detect Revision, Class and Subclass of Device,
;                 * and make Description based on Class
;-------------------------------------------------------------
use32

	org    0x0

	db     'MENUET01'; 8 byte id
	dd     0x01	 ; header version
	dd     START	 ; start of code
	dd     I_END	 ; size of image
	dd     0xFFFF	 ; memory for app = 64 KB
	dd     I_END	 ; esp
	dd     0x0 , 0x0 ; I_Param , I_Icon


include 'macros.inc'
include 'vendors.inc'
include 'lang.inc'

START:	  ; start of execution
     call draw_window

still:
    mov  eax,10   ; wait here for event
    int  0x40

    cmp  eax,1	 ; redraw request ?
    je	red
    cmp  eax,2	 ; key in buffer ?
    je	key
    cmp  eax,3	 ; button in buffer ?
    je	button

    jmp  still

  red:	  ; redraw
    mov eax, 9		    ;window redraw requested so get new window coordinates and size
    mov ebx, Proc_Info	    ;area to store process information
    mov ecx, -1 	    ;
    int 0x40		    ;get the process information
    mov eax,[Proc_Info+34]  ;store the window coordinates into the Form Structure
    mov [Form+2], ax	    ;x start position
    mov eax,[Proc_Info+38]  ;
    mov [Form+6],ax	    ;ystart position
    mov eax,[Proc_Info+42]  ;
    mov [Form],ax	    ;window width
    mov eax,[Proc_Info+46]  ;
    mov [Form+4] ,ax	    ;window height
    call draw_window	    ;go redraw window now
    jmp  still

  key:	  ; key
    mov  eax,2	 ; just read it and ignore
    int  0x40
    jmp  still
  button:   ; button
    mov  eax,17   ; get id
    int  0x40

    cmp  ah,1	; button id=1 ?
    jne  noclose

    mov  eax,-1   ; close this program
    int  0x40
  noclose:
    jmp  still




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
Form:
   dw 780 ;window width
   dw 100 ;window x start
   dw 420 ;window height
   dw 100 ;window y start



draw_window:

    mov byte [total],0

    mov  eax,12      ; function 12:tell os about windowdraw
    mov  ebx,1	     ; 1, start of draw
    int  0x40
    ; DRAW WINDOW
    mov  eax,0			   ; function 0 : define and draw window
    mov  ebx, dword [Form]	   ; x/width of window
    mov  ecx, dword [Form+4]	   ; y/height of window
    mov  edx,0x03ffffff 	   ; color of work area RRGGBB,8->color gl
    mov  esi,0x805080d0 	   ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x005080d0 	   ; color of frames    RRGGBB
    int  0x40			   ; draw the window
    ; WINDOW LABEL
    mov  eax,4			   ; function 4 : write text to window
    mov  ebx,8*65536+8		   ; [x start] *65536 + [y start]
    mov  ecx,0x10ddeeff 	   ; font 1 & color ( 0xF0RRGGBB )
    mov  edx,labelt		   ; pointer to text beginning
    mov  esi,labellen-labelt	   ; text length
    int  0x40

    ;draw captions to window
    mov ebx, 20*65536+25	   ;x start, ystart of text
    mov ecx, 0x224466		   ;color of text
    mov edx, dword PCIWin	   ;start of text buffer
    mov esi, 106		   ;lenght of line
newline:			   ;
    mov eax, 4			   ;draw text system function
    int 0x40			   ;draw the text
    add ebx, 10 		   ;one line down
    add edx, esi		   ;add lenght of line to offset of text buffer
    cmp byte[edx], byte 'x'	   ;is it the end of buffer?
    jne newline 		   ;if not draw another line of text

    ;Insert horizontal bars  in list area
    mov eax, 13 		   ;draw bar system function
    mov ebx, 18 		   ;set Xstart position of bar
    shl ebx, 16 		   ;
    mov bx,word [Form]		   ;get width of window
    sub bx, 32			   ;bar is 32 pixels shorter then window width
    mov ecx, 109*65536+10	   ;set Ystart(109) and Height(10) of bar
    mov edx, 0xb6b6b6		   ;set color of bar
   again:			   ;begin draw bar loop
    int 0x40			   ;draw bar to window area
    shr ecx, 16 		   ;move the Ystart position to working area
    add ecx, 34 		   ;add 34 pixels to Y Start (moves bar down)
    cmp cx,word [Form+4]	   ;is the Ystart position outside of window area
    jae nomo			   ;if so stop drawing bars
    sub ecx, 14 		   ;if not, we only need 20 pixels between bar tops
    shl ecx, 16 		   ;set that values as Ystart
    add ecx, 10 		   ;Bar Height is always 10 pixels
    jmp again			   ;draw another bar
   nomo:			   ;done drawing bars here

    ;start PCI stuff
    call Get_PCI_Info		   ;get pci version and last bus
    mov cx,word [PCI_Version]	   ;number to draw
    mov eax, 47 		   ;draw number system function
    xor esi, esi		   ;color of text
    mov ebx, 0x00040100 	   ;4 digits to draw in hex format
    mov edx, 110*65536+45	   ;x/y start position of number
    int 0x40			   ;draw pci version to window
    mov cl,byte [PCI_LastBus]	   ;number to draw
    mov ebx, 0x00020100 	   ;2 digits hex format
    add edx, 10 		   ;one line below pci version
    int 0x40			   ;draw the last bus to window

    call scan_Pci		   ;scan for and draw each pci device

    movzx ecx, byte [total]	   ;number to draw
    mov eax, 47 		   ;draw number system function
    mov ebx, 0x00020000 	   ;2 digits to draw in decimal format
    xor esi, esi		   ;color of text
    mov edx, 150*65536+65	   ;x/y position to draw to
    int 0x40			   ;draw total number of devices to window

    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    int  0x40

    ret
;   ***********************************************
;   *******  END WINDOW DEFINITIONS & DRAW  *******
;   ***********************************************


;******************************************************
;* Gets the PCI Versioin and Last Bus
Get_PCI_Info:
    mov eax, 62
    xor ebx, ebx
    int 0x40
    mov word [PCI_Version], ax
    mov eax, 62
    mov ebx, 1
    int 0x40
    mov byte [PCI_LastBus], al
ret
;******************************************************


;******************************************************
;* Get all devices on PCI Bus
scan_Pci:
   cmp	byte [PCI_LastBus],0xff ;0xFF means no pci bus found
   jne	Pci_Exists		;
   ret				;if no bus then leave
Pci_Exists:
   mov	byte [V_Bus], 0 	;reset varibles
   mov	byte [V_Dev], 0 	;
   mov	edx,  20*65536+110	;set start write position
Start_Enum:
   mov	bl, 6			;get a dword
   mov	bh, byte [V_Bus]	;bus of pci device
   mov	ch, byte [V_Dev]	;device number/function
   mov	cl, 0			;offset to device/vendor id
   mov	eax, 62 		;pci system function
   int	0x40			;get ID's

   cmp	ax, 0			;Vendor ID should not be 0 or 0xFFFF
   je	nextDev 		;check next device if nothing exists here
   cmp	ax, 0xffff		;
   je	nextDev 		;

   mov	word [PCI_Vendor], ax	;There is a device here, save the ID's
   shr	eax, 16 		;
   mov	word [PCI_Device], ax	;
   mov	eax, 62 		;PCI Sys Function
   mov	bl, 4			;Read config byte
   mov	bh, byte [V_Bus]	;Bus #
   mov	ch, byte [V_Dev]	;Device # on bus
   mov	cl, 0x08		;Register to read (Get Revision)
   int	0x40			;Read it
   mov	byte [PCI_Rev], al	;Save it
   mov	eax, 62 		;PCI Sys Function
   mov	cl, 0x0b		;Register to read (Get class)
   int	0x40			;Read it
   mov	byte [PCI_Class], al	;Save it
   mov	eax, 62 		;PCI Sys Function
   mov	cl, 0x0a		;Register to read (Get Subclass)
   int	0x40			;Read it
   mov	byte [PCI_SubClass], al ;Save it
   inc	byte [total]		;one more device found
   call Print_New_Device	;print device info to screen
nextDev:
   inc	byte [V_Dev]		;next device on this bus
   jnz	Start_Enum		;jump until we reach zero
				;(used to be JNO which caused bug!!! 30-4-2006, JMD)
   mov	byte [V_Dev], 0 	;reset device number
   inc	byte [V_Bus]		;next bus
   mov	al, byte [PCI_LastBus]	;get last bus
   cmp	byte [V_Bus], al	;was it last bus
   jbe	Start_Enum		;if not jump to keep searching
ret
;******************************************************



;******************************************************
;* Print device info to screen
Print_New_Device:
   mov	 eax, 47		;Write number to screen system function
   mov	 ebx, 0x00040100	;4 byte number, print in hexidecimal
   xor	 esi, esi              	;Color of text
   movzx ecx,word [PCI_Vendor]	;Pointer to number to be written
   int	 0x40			;Write Vendor ID
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 54*65536		;X start becomes 54
   movzx ecx,word [PCI_Device]	;get Vendor ID
   int	 0x40			;Draw Vendor ID to Window
   mov	 ebx, 0x00020100	;2 digit number, in hexidecimal format
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 98*65536		;X start becomes 98
   movzx ecx,byte [V_Bus]	;get bus number
   int	 0x40			;draw bus number to screen
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 128*65536 	;X start becomes 128
   movzx ecx,byte [V_Dev]	;get device number
   shr	 ecx, 3 		;device number is bits 3-7
   int	 0x40			;Draw device Number To Window
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 155*65536 	;X start becomes 155
   movzx ecx, byte [V_Dev]	;get Function number
   and	 ecx, 7 		;function is first 3 bits
   int	 0x40			;Draw Function Number To Window
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 179*65536 	;X start becomes 179
   movzx ecx,byte [PCI_Rev]	;get revision number
   int	 0x40			;Draw Revision to screen
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 215*65536 	;X start becomes 215
   movzx ecx,byte [PCI_Class]	;get PCI_Class
   int	 0x40			;Draw Class to screen
   and	 edx, 0xFFFF		;*****************************************
   or	 edx, 266*65536 	;X start becomes 266
   movzx ecx,byte [PCI_SubClass];get sub class
   int	 0x40			;Draw Sub Class to screen
   ;Write Names
   movzx ebx, dx		;Set y position
   or	 ebx, 310*65536 	;set Xposition to 310

;**********************************************************
;Prints the Vendor's Name based on Vendor ID
;
; modified part by vhanla (I know it is not a fastest way to search)
; it needs optimization... HELP this project!
;
; Modified on 30-04-2006 by JMD for size
;-----------------------------------------------------------------------------
    ;first determine which list to find the vendor in
    mov ax, word [PCI_Vendor]
    mov ecx, 255	   ;# vendors in most lists
    cmp ax,4800 	   ;Check if Vendor's value is less than this number
    jae next1		   ;if it is less, let's continue, or jump to next1
      mov edx, _FIRSTPART  ;select this list
      jmp rep1		   ;start searching list
  next1:		   ;
    cmp ax,5314 	   ;same thing happening here as above^
    jae next2		   ;
      mov edx, _SECONDPART ;
      jmp rep1		   ;
  next2:		   ;
    cmp ax,5574 	   ;
    jae next3		   ;
      mov edx, _THIRDPART  ;
      jmp rep1		   ;
  next3:		   ;
      mov ecx, 110	   ;only 110 vendors in this list
      mov edx, _FOURTHPART ;

  rep1:
   cmp ax,word[edx+50]	   ;are Vendor ID's the same?
   je  ex		   ;if so jump to print the description to screen
   add edx, 52		   ;if not put us at start of next description
   dec ecx		   ;one less description in list
   jnz rep1		   ;was it our last?
   mov edx, _UNKNOWN	   ;if so we dont know this Vendor

 ex:
  ;lets print the vendor Name
   xor ecx, ecx 	   ;font color
   mov eax,4		   ;OS CMD
   mov esi,50		   ;Length of text
   int 0x40		   ;Print the text
;------------------------------------------------------------------

;------------------------------------------------------------------
;Get description based on Class/Subclass
   cmp	 byte [PCI_Class], 11h	  ;we only know of 17 classes
   ja	 endd			  ;if its more then, its unknown to us, so jump
   movzx eax, byte [PCI_Class]	  ;load our class
   shl	 eax, 3 		  ;multiply for jump table
   mov	 ecx, [ClassList+eax+4]   ;number of descriptions for this class
   mov	 edx, [ClassList+eax]	  ;start of description list for class
   movzx eax, byte [PCI_SubClass] ;get subclass
 repu1:
   cmp	 al,byte[edx+32]	  ;are subclasses the same?
   je	 endd			  ;if so jump to print the description to screen
   add	 edx, 33		  ;if not put us at start of next description
   dec	 ecx			  ;one less description in list
   jnz	 repu1			  ;was it our last?
   mov	 edx,_UNKNOWND		  ;if so its unknown device
 endd:
   and	 ebx, 0x0000FFFF	  ;clear X position
   or	 ebx, 0x02300000	  ;set X position to 560 pixels
   xor	 ecx, ecx		  ;color of text
   mov	 eax,4			  ;draw text system function
   mov	 esi,32 		  ;length of text to draw
   int	 0x40			  ;draw the text

   movzx edx, bx		  ;get y coordinate
   add	 edx, 0x0014000A	  ;add 10 to y coordinate and set x coordinate to 20
ret

ClassList:
 dd Class0 ,  2, Class1 , 8, Class2,  8, Class3, 4
 dd Class4 ,  4, Class5 , 3, Class6, 12, Class7, 7
 dd Class8 ,  8, Class9 , 6, ClassA,  2, ClassB, 7
 dd ClassC , 10, ClassD , 8, ClassE,  1, ClassF, 4
 dd Class10,  3, Class11, 5
;------------------------------------------------------------------



; DATA AREA

labelt:
db 'PCI Device Enumeration v 1.31 by J. Delozier, S. Kuzmin and V. Hanla'
 labellen:

if lang eq de
PCIWin:
db 'Im Einstellungsmenue muss der PCI Zugriff eingeschaltet sein.      '
db '                                       '
db '                                                                   '
db '                                       '
db 'PCI Version  =                                                     '
db '                                       '
db 'Letzter PCI Bus =                                                  '
db '                                       '
db 'Anzahl von Geraeten =                                              '
db '                                       '
db '                                                                   '
db '                                       '
db 'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass               Herste'
db 'ller                    Beschreibung   '
db '----- ----- ---- ---- --- ---  -----  -------- --------------------'
db '---------------------- ----------------'
db 'x'
else
PCIWin:
db 'Please remember to enable PCI Access to Applications in Setup Menu.'
db '                                       '
db '                                                                   '
db '                                       '
db 'PCI Version  =                                                     '
db '                                       '
db 'Last PCI Bus =                                                     '
db '                                       '
db 'Number of devices =                                                '
db '                                       '
db '                                                                   '
db '                                       '
db 'VenID DevID Bus# Dev# Fnc Rev  Class  Subclass                 Comp'
db 'any                      Description   '
db '----- ----- ---- ---- --- ---  -----  -------- --------------------'
db '---------------------- ----------------'
db 'x'
end if


total	     db 0
V_Bus	     db 0
V_Dev	     db 0
PCI_Version  dw 0
PCI_LastBus  db 0
PCI_Device   dw 0
PCI_Vendor   dw 0
PCI_Bus      db 0
PCI_Dev      db 0
PCI_Rev      db 0
PCI_Class    db 0
PCI_SubClass db 0


Proc_Info:
 times 1024 db 0

I_END: