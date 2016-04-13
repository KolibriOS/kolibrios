;throttle_multiply.asm
;
;********************************************************************************
;* mpy8u                                                                         *
;* Second Level Subroutine                                                       *
;*                                                                               *
;* Program from Atmel file avr200.asm                                            *
;*                                                                               *
;* Since the 25kHz pwm cycle is 64 clock cycles long, this subroutine            *
;* requires just under 1 25kHz clock cycles.                                     *
;*                                                                               *
;* A single line was added which adds 3 to Cycle_count                           *
;*                                                                               *
;* Inputs:  HILOCAL1 and B_TEMPLOCAL                                             *
;*                                                                               *
;* Returns: HILOCAL1 x B_TEMPLOCAL = B_TEMPLOCAL1:B_TEMPLOCAL                    *
;*                                                                               *
;* Changed: B_TEMPLOCAL2                                                         *
;*                                                                               *
;* Calls:   Not allowed                                                          *  
;********************************************************************************

   HILOCAL1       mc8u                 ; multiplicand
   B_TEMPLOCAL    mp8u                 ; multiplier
   B_TEMPLOCAL    m8uL                 ; result Low byte
   B_TEMPLOCAL1   m8uH                 ; result High byte
   B_TEMPLOCAL2   mcnt8u               ; loop counter


;<ATMEL ROUTINE>
;***************************************************************************
;*
;* "mpy8u" - 8x8 Bit Unsigned Multiplication
;*
;* This subroutine multiplies the two register variables mp8u and mc8u.
;* The result is placed in registers m8uH, m8uL
;*  
;* Number of words   :9 + return
;* Number of cycles  :58 + return
;* Low registers used   :None
;* High registers used  :4 (mp8u,mc8u/m8uL,m8uH,mcnt8u)  
;*
;* Note: Result Low byte and the multiplier share the same register.
;* This causes the multiplier to be overwritten by the result.
;*
;***************************************************************************

;***** Subroutine Register Variables

;.def mc8u  =r16     ;multiplicand
;.def mp8u  =r17     ;multiplier
;.def m8uL  =r17     ;result Low byte
;.def m8uH  =r18     ;result High byte
;.def mcnt8u   =r19     ;loop counter

;***** Code


mpy8u:   
   clr   m8uH                          ;clear result High byte
   ldi   mcnt8u,8                      ;init loop counter
   lsr   mp8u                          ;rotate multiplier
   
m8u_1:
   brcc  m8u_2                         ;carry set 
   add   m8uH,mc8u                     ;add multiplicand to result High byte
m8u_2:
   ror   m8uH                          ;rotate right result High byte
   ror   m8uL                          ;rotate right result L byte and multiplier
   dec   mcnt8u                        ;decrement loop counter
   brne  m8u_1                         ;if not done, loop more
   ret

;<END ATMEL ROUTINE>
