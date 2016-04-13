;throttle_momentum_lowpass.asm

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

.ifdef   MOMENTUM_LOWPASS_ENABLED
   ;*****************************************************************
   ;* A transversal low pass filter                                  *
   ;*                                                                *
   ;* This is copied from throttle_backemf.asm.                      *
   ;* See the documentation there.                                   *
   ;*****************************************************************
   HILOCAL1    _momentum_lo             ; assign local variables
   HILOCAL2    _momentum_hi

   mov   _momentum_lo,momentum_set
   clr   _momentum_hi 

   ;****
   ;* 1. Add in cumulative previous throttle
   ;****
   add   _momentum_lo,momentum_lo_prev        ; Add in scaled previous samples
   adc   _momentum_hi,momentum_hi_prev        ;

   ;****
   ;* 2.
   ;****
   mov   momentum_lo_prev,_momentum_lo        ; Store new value
   mov   momentum_hi_prev,_momentum_hi        ; Store new value

   ;****
   ;* 3. 
   ;****

   B_TEMPLOCAL _lowpass_lo_byte
   ldi   _lowpass_lo_byte, momentum_lowpass_gain
   rcall DIVIDE_16_SIMPLE

   ;****
   ;* 4.
   ;****
   sub   momentum_lo_prev,_momentum_lo
   sbc   momentum_hi_prev,_momentum_hi

   mov   momentum_set,_momentum_lo
.endif ;MOMENTUM_LOWPASS_ENABLED
