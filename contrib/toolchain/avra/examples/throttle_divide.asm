;throttle_divide.asm
;
;********************************************************************************
;* div16u                                                                        *
;* Second Level Subroutine                                                       *
;*                                                                               *
;* Program from Atmel file avr200.asm                                            *
;*                                                                               *
;* Since the 25kHz pwm cycle is 64 clock cycles long, this subroutine            *
;* requires 3.67 to 3.92 25kHz clock cycles.                                     *
;*                                                                               *
;* A single line was added which adds 3 to Cycle_count                           *
;*                                                                               *
;* Inputs:  HILOCAL2:HILOCAL1 and B_TEMPLOCAL1:B_TEMPLOCAL                       *
;* Returns: HILOCAL2:HILOCAL1 = HILOCAL2:HILOCAL1 / B_TEMPLOCAL1:B_TEMPLOCAL     *
;*          LOLOCAL2:LOLOCAL1 = remainder                                        *
;* Changed: B_TEMPLOCAL2                                                         *
;*                                                                               *
;* Calls:   Not allowed                                                          *  
;********************************************************************************


   B_TEMPLOCAL2   dcnt16u              ; Local counter

   HILOCAL1       dd16uL               ; 16 bit Innput
   HILOCAL2       dd16uH

   B_TEMPLOCAL    dv16uL               ; 16 bit Input
   B_TEMPLOCAL1   dv16uH

   HILOCAL1       dres16uL             ; 16 bit Output
   HILOCAL2       dres16uH

   LOWLOCAL1      drem16uL             ; 16 bit Remainder 
   LOWLOCAL2      drem16uH             ;



;<ATMEL ROUTINE>
;***************************************************************************
;*
;* "div16u" - 16/16 Bit Unsigned Division
;*
;* This subroutine divides the two 16-bit numbers 
;* "dd16uH:dd16uL" (dividend) and "dv16uH:dv16uL" (divisor). 
;* The result is placed in "dres16uH:dres16uL" and the remainder in
;* "drem16uH:drem16uL".
;*  
;* Number of words   :19
;* Number of cycles  :235/251 (Min/Max)
;* Low registers used   :2 (drem16uL,drem16uH)
;* High registers used  :5 (dres16uL/dd16uL,dres16uH/dd16uH,dv16uL,dv16uH,
;*           dcnt16u)
;*
;***************************************************************************

;***** Subroutine Register Variables

;.def drem16uL=   r14                  ; Reassigned
;.def drem16uH=   r15
;.def dres16uL=   r16
;.def dres16uH=   r17
;.def dd16uL=     r16
;.def dd16uH=     r17
;.def dv16uL=     r18
;.def dv16uH=     r19
;.def dcnt16u=    r20

;***** Code

div16u:
   clr   drem16uL                      ; clear remainder Low byte
   sub   drem16uH,drem16uH             ; clear remainder High byte and carry
   ldi   dcnt16u,17                    ; init loop counter

d16u_1:  
   rol   dd16uL                        ; shift left dividend
   rol   dd16uH
   dec   dcnt16u                       ; decrement counter
   brne  d16u_2                        ; if done

   subi  Cycle_count,256-3             ; Add 3 to Cycle_count

   ret                                 ; return

d16u_2:
   rol   drem16uL                      ; shift dividend into remainder
   rol   drem16uH

   sub   drem16uL,dv16uL               ; remainder = remainder - divisor
   sbc   drem16uH,dv16uH               ;

   brcc  d16u_3                        ;

   add   drem16uL,dv16uL               ; if result negative
   adc   drem16uH,dv16uH               ; restore remainder
   clc                                 ; clear carry to be shifted into result
   rjmp  d16u_1                        ;

d16u_3:                                ; if result NOT negative
   sec                                 ; set carry to be shifted into result
   rjmp  d16u_1

;<END ATMEL ROUTINE>

;********************************************************************************
;* DIVIDE_16_SIMPLE                                                              *
;* Second Level Subroutine                                                       *
;*                                                                               *
;* Inputs:  dd16uH:dd16ul and dv16uL                                             *
;* Returns: dres16uH:dres16uL = dd8uH:dd8uL / 2^dv16uL                           *
;*                                                                               *
;* Changed: nothing else                                                         *
;*          N.B that dd16uH, dd16uL, dv16uH and dv16uL are aliases for:          *
;*          dd16uH=error_hi                                                      *
;*          dd16uL=error_lo                                                      *  
;*          dv16uH=B_TempX                                                       *
;*          dv16uL=B_TempX                                                       *
;*          dcnt16u=B_TempX                                                      *
;* Calls:   Not allowed                                                          *  
;********************************************************************************


DIVIDE_16_SIMPLE:
   inc   dv16uL
DIVIDE_16_SIMPLE_LOOP:                           
   dec   dv16uL                        ; decrement counter
   brne  DIVIDE_BY_2
   ret   

DIVIDE_BY_2:
   asr   dd16uH                        ; divide by two   
   ror   dd16uL
   rjmp  DIVIDE_16_SIMPLE_LOOP
   
