;throttle_momentum.asm

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
;  ***************************************************************************************:
.LIST

.ifdef MOMENTUM_ENABLED
;********************************************************************************
;* MOMENTUM_ADJUST                                                               *
;* Top level routine                                                             *
;*                                                                               *
;* Momentum simulates the mass of the train.  Since model trains have little     *
;* mass, the locomotive speed can directly follow the throttle setting; in       *
;* other words, a model train can accelerate and decelerate instantly.           *
;* Real trains are very massive, and therefore they do not accelerate or         *
;* decelerate quickly.                                                           *
;*                                                                               *
;* According to Newtons law, the acceleration is proportional to the force,      *
;* and inversely proportional to the mass.  Therefore, the more massive the      *
;* train, the more slowly the train will accelerate or decelerate.  Also, the    *
;* more force the locomotive can provide, the faster the train will accelerate.  *
;* Deceler depends on the braking capability of the overall train.               *
;*                                                                               *
;* If force were constant, Newtons law states that acceleration would be         *
;* constant too.  This subroutine assumes that somewhat more force is            *
;* available at low speeds than at high speeds, so that acceleration will be     *
;* greater at low speeds.  The subroutine also assumes that opposing forces      *
;* (friction, wind resistance, etc) are stronger at higher speeds.  This         *
;* assumption also means that acceleration will be greater at low speeds.        *
;*                                                                               *
;* This subroutine calculates acceleration/deceleration by this simple           *
;* Method:                                                                       *
;*                                                                               *
;*    The rate of speed change (accleration and deceleration) depends on the     *
;*    current speed as                                                           * 
;*    {Speed_Max (0xFF) - Current_Speed} / {Tau*Rate}                            *
;*                                                                               *
;*       Where    T = index of sample time                                       *
;*                t = real time                                                  *
;*                Tau = time constant                                            *
;*                Rate = Update rate (nominally 100Hz)                           *
;*                                                                               *
;* That is, the acceleration/deceleration is maximum at zero speed, and          *
;* approaches zero at maximum speed.                                             *
;*                                                                               *
;* When accelerating, the speed at the next sample period is                     *
;*    Speed(T) = Speed(T-1) + {0xFF - Speed(T-1)} / {Tau*Rate}                   *
;*                                                                               *
;*    Giving an acceleration curve that looks like a normal exponential.         *
;*    Speed(t) = 0xFF { 1 - exp( - t / Tau) }                                    *
;*                                                                               *
;*                                        *                   *                  *
;*                       *                                                       *
;*             *                                                                 *
;*         *                                                                     *
;*       *                                                                       *
;*      *                                                                        *
;*     *                                                                         *
;*     *                                                                         *
;*                                                                               *
;*                                                                               *
;* When decelerating, the change rate equation is the same, but the change       *
;* is subtracted, as                                                             *
;*    Speed(T) = Speed(T) - {0xFF - Speed(T-1)} / {Tau*Rate}                     *
;*                                                                               *
;*    Giving a deceleration curve that looks like                                *
;*    Speed(t) = 0xFF { 1 - exp( -(T1 - t) / Tau) }                              *
;* which is a mirror image of the acceleration, NOT a normal exponential.        *
;*                                                                               *
;*     *              *                                                          *
;*                                        *                                      *
;*                                            *                                  *
;*                                              *                                *
;*                                               *                               *
;*                                                *                              *
;*                                                *                              *
;*                                                                               *
;*                                                                               *
;* In each case, the acceleration or deceleration is "clipped" at the current    *
;* throttle setting so that the speed doesn't overshoot or undershoot.           *
;*                                                                               *
;* Three different values of Tau are used, that is                               *
;*    Tau_accel      Corresponding to acceleration under power                   *
;*    Tau_coast      Corresponding to deceleration when coasting                 *
;*    Tau_brake      Corresponding to deceleration when braking                  *
;*                                                                               *
;* To permit finer control of momentum, the throttle setting is converted to a   *
;* 16 bit number, where the 8 msb's correspond to the throttle setting from      *
;* the throttle handle and sent forward.                                         *
;*                                                                               *
;* Inputs:  throttle_set      Throttle handle position ( 0x00 to 0xFF )          *
;*          speed_hi_prev     Hi byte of (T-1) throttle setting (stored)         *
;*          speed_lo_prev     Lo byte of (T-1) throttle setting (stored)         *
;* Returns: throttle_set      Adjusted throttle setting (T)                      *
;*          speed_hi_prev     Hi byte of (T) throttle setting  (stored)          *
;*          speed_lo_prev     Lo byte of (T) throttle setting  (stored)          *
;* Changed: B_Temp                                                               * 
;*          B_Temp1                                                              *
;*          B_Temp2                                                              *
;*          B_Temp3                                                              *
;* Calls:   NONE                                                                 *  
;* Goto:    MOMENTUM_ADJUST_RETURN                                               *
;********************************************************************************
   B_TEMPLOCAL2 _time_constant_adj


.ifdef TRADITIONAL_ENABLED
   .ifdef LEDS_ENABLED
      sbrc  Flags_1,BF_brake           ; If the brake flag is set,
      sbi   PORTB,dir_in_port          ; Port Output: Indicate deceleration
   .endif ;LEDS_ENABLED
.endif ;TRADITIONAL_ENABLED

   ;*******************************************************************
   ;* Adjust the value of "momentum_set".
   ;* This adjustment makes it easier to fine adjust low momentum settings
   ;* while still permitting large momentum settings.
   ;* 
   ;* The ammount of momentum to apply comes in in "momentum_set"
   ;* which is read in READ_THROTTLE.  The nominal range is
   ;* 0x00 to 0x40.  This value is multiplied by two and squared,
   ;* giving a new range from 0x00 to 0x4000.  The update rate is 100Hz,
   ;* and so the new range corresponds to a time constant from
   ;* 0(decimal) to 164(decimal) seconds.  Since the adjustment was
   ;* done by performing a square, the adjusted value is non-linear
   ;* with the input value.
   ;*******************************************************************
   lsl   momentum_set                        ; multiply by two

   HILOCAL1    _mset_multiplier              ; supply to mpy8u
   B_TEMPLOCAL _mset_multiplicand            ; supply to mpy8u

   mov   _mset_multiplier,momentum_set       ;  
   mov   _mset_multiplicand,momentum_set     ;
   rcall mpy8u                               ; square

   B_TEMPLOCAL1   _mset_hi_byte              ; return from mpy8u
   B_TEMPLOCAL    _mset_lo_byte              ; return from mpy8u

   ;*******************************************************************
   ;* Compute the difference between the maximum throttle and
   ;* the current throttle
   ;*******************************************************************
   HILOCAL2 _mset_diff_hi_byte
   HILOCAL1 _mset_diff_lo_byte

   ldi   _mset_diff_hi_byte,0xFF             ; Maximum possible speed
   ldi   _mset_diff_lo_byte,0xFF             ;                      

   sub   _mset_diff_lo_byte,speed_lo_prev    ; Difference between max speed 
   sbc   _mset_diff_hi_byte,speed_hi_prev    ; and current speed

   ;*******************************************************************
   ;* Determine whether to accelerate, decelerate, or remain unchanged.
   ;* Compare the throttle handle setting with the actual speed
   ;*******************************************************************
   cp    throttle_set,speed_hi_prev    ; Test if throttle position is larger
                                       ; or smaller than the speed.

   breq  EVEN_SPEED                    ; If the throttle position is the same
                                       ; as the speed.
                                       
   brlo  SETUP_DECELERATE              ; If the throttle position is smaller
                                       ; than the speed, then need to decelerate.

;  brsh  SETUP_ACCELERATE              ; If the throttle position is larger
                                       ; than the speed, then need to accelerate.

SETUP_ACCELERATE:
.ifdef TRADITIONAL_ENABLED
   .ifdef LEDS_ENABLED
      cpi   throttle_set,accel_led_threshold ; If the throttle is less than minimum
      brlo  END_SET_ACCEL_LED                ; don't light led

      mov   B_Temp2,throttle_set             ; If the throttle is closer than led_threshold
      subi  B_Temp2,accel_led_threshold      ; don't light led
      cp    B_Temp2,speed_hi_prev      
      brlo  END_SET_ACCEL_LED

      sbi   PORTB,momentum_port        ; Port Output: Indicate acceleration
      END_SET_ACCEL_LED:
   .endif LEDS_ENABLED
.endif TRADITIONAL_ENABLED

   sbr   Flags_1,F_accel               ; Set accelerating flag
                                       ; Indicate acceleration

   ldi   _time_constant_adj,accel_offset+1   ; Acceleration time constant adjust

   rjmp  CHECK_BRAKE

EVEN_SPEED:                            ; Arrive here if throttle_set=current speed

   sbrc  Flags_1,BF_brake              ; If the brake flag is set, decelerate
   rjmp  CHECK_BRAKE                   ;
 
   rjmp  DONE_WITH_MOMENTUM            ; Otherwise adjustment is necessary


SETUP_DECELERATE:
   cbr   Flags_1,F_accel               ; Clear accelerating flag 

.ifdef TRADITIONAL_ENABLED
   .ifdef LEDS_ENABLED

      cpi   throttle_set,0xff-decel_led_threshold  ; If the throttle is more than maximum
      brsh  END_SET_DECEL_LED                      ; don't light led

      mov   B_Temp2,throttle_set
      subi  B_Temp2,0x00-decel_led_threshold       ; If the throttle is closer than the led
      cp    B_Temp2,speed_hi_prev                  ; threshold, don't light the led
      brsh  END_SET_DECEL_LED

      sbi   PORTB,dir_in_port          ; Port Output: Indicate deceleration 
      END_SET_DECEL_LED:
   .endif LEDS_ENABLED
.endif TRADITIONAL_ENABLED

   ldi   _time_constant_adj,0+1       ; Coasting deceleration time const. adjust.
;  rjmp  CHECK_BRAKE

CHECK_BRAKE:                           ; Always check for the brake.

   sbrs  Flags_1,BF_brake              ; If brake flag is not set,
   rjmp  ADJUST_TAU                    ; proceed.  

                                       ; Brake overrides acceleration
                                       ; or coasting.
   cbr   Flags_1,F_accel               ; clear accelerating flag
                                       ; Indicate deceleration

   ldi   _time_constant_adj,brake_offset+1   ; Braking deceleration time const. adjust.

;  rjmp  ADJUST_TAU

ADJUST_TAU:
   ;B_TEMP2=B_TEMPLOCAL2
   dec   _time_constant_adj            ; Divide tau_base by 2^_time_constant_adj
   breq  DIVIDE_TAU                    ; to produce adjusted tau.

   lsr   _mset_hi_byte                              
   ror   _mset_lo_byte
   rjmp  ADJUST_TAU

DIVIDE_TAU:
   sbr   _mset_lo_byte,0b00000001      ; Force last bit 1.  Prevent divide by zero.

   rcall div16u                        ; Divide _mset_diff_hi_byte:_mset_diff_lo_byte
                                       ;  (difference) 
                                       ; by _mset_hi_byte:_mset_lo_byte (dividor)

   sbrs  Flags_1,BF_accel              ; add or subtract change depending
   rjmp  SUBTRACT_CHANGE               ; on F_accel flag
   ;rjmp ADD_CHANGE

ADD_CHANGE:                            ; Case accelerating

;  HILOCAL2 _mset_diff_hi_byte
;  HILOCAL1 _mset_diff_lo_byte

   add   speed_lo_prev,_mset_diff_lo_byte        ; Add in the change
   adc   speed_hi_prev,_mset_diff_hi_byte

   cp    throttle_set,speed_hi_prev    ; If larger than the throttle_set value 
   brlo  USE_SET_SPEED                 ; clamp at throttle_set value                 

   rjmp  DONE_WITH_MOMENTUM

SUBTRACT_CHANGE:                       ; Case decelerating

   sbrc  Flags_1,BF_brake              ; If the brake flag is set,
   clr   throttle_set                  ; decelerate all the way to zero

   sub   speed_lo_prev,_mset_diff_lo_byte        ; Subtract the change
   sbc   speed_hi_prev,_mset_diff_hi_byte        ;

   brlo  USE_SET_SPEED                 ; If less than zero
                                       ; clamp at throttle_set value

   cp    speed_hi_prev,throttle_set    ; If less than the throttle_set value
   brlo  USE_SET_SPEED                 ; clamp at throttle_set value

   rjmp  DONE_WITH_MOMENTUM

USE_SET_SPEED:                         ; Use the throttle_set value directly
   mov   speed_hi_prev,throttle_set
   clr   speed_lo_prev

DONE_WITH_MOMENTUM:
   mov   throttle_set,speed_hi_prev    ; Put the new value into throttle_set. 

.endif ;MOMENTUM_ENABLED
