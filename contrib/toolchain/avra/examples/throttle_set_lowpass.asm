;throttle_set_lowpass.asm

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

.ifdef   THROTTLE_LOWPASS_ENABLED
   ;*****************************************************************
   ;* A transversal low pass filter                                  *
   ;*                                                                *
   ;* This is copied from throttle_backemf.asm.                      *
   ;* See the documentation there.                                   *
   ;*****************************************************************
   HILOCAL1    throttle_lo             ; assign local variables
   HILOCAL2    throttle_hi

   mov   throttle_lo,throttle_set
   clr   throttle_hi 

   ;****
   ;* 1. Add in cumulative previous throttle
   ;****
   add   throttle_lo,throttle_lo_prev        ; Add in scaled previous samples
   adc   throttle_hi,throttle_hi_prev        ;

   ;****
   ;* 2.
   ;****
   mov   throttle_lo_prev,throttle_lo        ; Store new value
   mov   throttle_hi_prev,throttle_hi        ; Store new value

   ;****
   ;* 3. 
   ;****

   B_TEMPLOCAL _lowpass_lo_byte
   ldi   _lowpass_lo_byte, throttle_lowpass_gain
   rcall DIVIDE_16_SIMPLE

   ;****
   ;* 4.
   ;****
   sub   throttle_lo_prev,throttle_lo
   sbc   throttle_hi_prev,throttle_hi

   mov   throttle_set,throttle_lo
.endif ;THROTTLE_LOWPASS_FILTER
