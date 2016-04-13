;throttle_pulse.asm

.NOLIST
;  ***************************************************************************************
;  * PWM MODEL RAILROAD THROTTLE                                                          *
;  *                                                                                      *
;  * WRITTEN BY:  PHILIP DEVRIES                                                          *
;  *                                                                                      *
;  *  Copyright (C) 2003 Philip DeVries                                                   *
;  *                                                                                      *
;  *  This program is free software; you can redistribute it and/or modify                *
;  *  it under the terms of the GNU General Public License as published by                *
;  *  the Free Software Foundation; either version 2 of the License, or                   *
;  *  (at your option) any later version.                                                 *
;  *                                                                                      *
;  *  This program is distributed in the hope that it will be useful,                     *
;  *  but WITHOUT ANY WARRANTY; without even the implied warranty of                      *
;  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                       *
;  *  GNU General Public License for more details.                                        *
;  *                                                                                      *
;  *  You should have received a copy of the GNU General Public License                   *
;  *  along with this program; if not, write to the Free Software                         *
;  *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA           *
;  *                                                                                      *
;  ***************************************************************************************
.LIST

.ifdef PULSE_ENABLED
;********************************************************************************
;* PULSE_GENERATE                                                                *
;* Top level routine                                                             *
;*                                                                               *
;* Inputs:  throttle_set                                                         *
;* Returns: none                                                                 *
;* Changed: B_Temp,ramp_target,ramp_target1                                      *    
;* Calls:   SET_PWM_DUTY                                                         *
;*          COUNT_PWM_CYCLES                                                     *  
;* Goto:    none                                                                 *
;********************************************************************************

HILOCAL1 ramp_target
HILOCAL2 ramp_target1

   ;*****************************************************************
   ;* If throttle_set is less than 128 (0x80)
   ;*    Ramp up from wherever the pwm is up to (255 - throttle)
   ;* If throttle_set is greater than 127 0x7F)
   ;*    Ramp up to throttle_set
   ;*****************************************************************

   mov   ramp_target1,throttle_set
   sbrs  ramp_target1,7

.ifdef PULSE_AMPLITUDE_SCALE
   com   ramp_target1                  ; Ramp up to this value
.else
   ldi   ramp_target1,0x80
.endif

   mov   ramp_target,ramp_target1

   subi  ramp_target,pulse_slope_up    ; Ramp up value - pulse_slope

WAIT_FOR_PWM_1:                        ; Wait for PWM to reset to 0
   in    B_Temp,TIFR                   
   sbrs  B_Temp,OCF1A
   rjmp  WAIT_FOR_PWM_1
   
   ldi   B_Temp,0b01000000
   out   TIFR,B_Temp

   inc   Cycle_count

   in    B_Temp,OCR1A                  ; Find PWM value
   cp    B_Temp,ramp_target            ; Make sure won't go past max.
   brsh  DONE_SLOPING_UP

   subi  B_Temp, 0x100-pulse_slope_up  ; OCR1A + pulse_slope_up

   rcall SET_PWM_DUTY
   rjmp  WAIT_FOR_PWM_1

DONE_SLOPING_UP:
   mov   B_Temp,ramp_target1
   rcall SET_PWM_DUTY

   ;*****************************************************************
   ;* See if we need to slope down to the throttle setting
   ;*****************************************************************
   in    B_Temp,OCR1A                  ; Find PWM value
   cp    B_Temp,throttle_set           
   breq  PULSE_GENERATE_RETURN         ; Do nothing if already at final voltage

   ;*****************************************************************
   ;* Hang about at the top of the pulse for a while...
   ;*****************************************************************

.ifdef PULSE_WIDTH_SCALE
   mov   ramp_target1,throttle_set
.else
   clr   ramp_target1
.endif

   add   ramp_target1,Cycle_count

   subi  ramp_target1,0x100-pulse_width_min  ; ramp_target1 + pulse_width_min

   mov   B_Temp1,ramp_target1

   rcall COUNT_PWM_CYCLES

   ;*****************************************************************
   ;* Slope down
   ;*****************************************************************

   mov   ramp_target,throttle_set      ; Ramp DOWN to this value

   subi  ramp_target,0x100-pulse_slope_down  ;ramp_target + pulse_slope_down

WAIT_FOR_PWM_2:                        ; Wait for PWM to reset to 0
   in    B_Temp,TIFR                   
   sbrs  B_Temp,OCF1A
   rjmp  WAIT_FOR_PWM_2
   
   ldi   B_Temp,0b01000000
   out   TIFR,B_Temp

   inc   Cycle_count

   in    B_Temp,OCR1A                  ; Find PWM value
   cp    ramp_target,B_Temp            ; Make sure won't go past min
   brsh  PULSE_GENERATE_RETURN

   subi  B_Temp,pulse_slope_down

   rcall SET_PWM_DUTY
   rjmp  WAIT_FOR_PWM_2

PULSE_GENERATE_RETURN:
.endif ;PULSE_ENABLED
