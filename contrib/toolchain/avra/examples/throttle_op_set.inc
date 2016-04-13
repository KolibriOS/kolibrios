;throttle_op_set.inc

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

;*********************************************************
;* Operation Settings                                    *
;*********************************************************



;*********************************************************
;* Compile time options
;*********************************************************
.define TRADITIONAL_ENABLED            ; A traditional throttle.  NOT
                                       ; a dcc decoder

   .define WALKAROUND_ENABLED          ; Removable, walkaround throttle
                                       ; (Does nothing unless the
                                       ;  TRADITIONAL_ENABLED is also defined)

   .define THROTTLE_LOWPASS_ENABLED    ; Lowpass filter on throttle handle
                                       ; (Does nothing unless the
                                       ;  TRADITIONAL_ENABLED is also defined)

   .define MOMENTUM_LOWPASS_ENABLED    ; Lowpass filter on momentum handle
                                       ; (Does nothing unless the
                                       ;  TRADITIONAL_ENABLED and
                                       ;  MOMENTUM_ENABLED are defined)

   .define SWITCH_LOWPASS_ENABLED      ; Lowpass filter on direction switches

   .define LEDS_ENABLED                ; Acceleration/Deceleration indicators
                                       ; (Does nothing unless the
                                       ;  TRADITIONAL_ENABLED is also defined)

   .define LOCO_LIGHT_ENABLED          ; Keep pwm on at low level so light
                                       ; remains on.
                                       ; (Does nothing unless the
                                       ;  TRADITIONAL_ENABLED is also defined)

.define BACKEMF_ENABLED                ; If enabled, speed is compensated
                                       ; according to motor back emf

   .define BACKEMF_SCALE_ENABLED       ; If enabled, the speed compensation
                                       ; is reduced as the throttle setting
                                       ; is increased.
                                       ; (Does nothing unless the
                                       ;  BACKEMF_ENABLED is also defined)


   .define LOWPASS_ENABLED             ; Provide a lowpass filter on the
                                       ; the back-emf error amplifier
                                       ; (Does nothing unless the
                                       ;  BACKEMF_ENABLED is also defined)

.define OVERLOAD_ENABLED               ; Enable overload protection

.define PULSE_ENABLED                  ; Pulse power at low throttle

   ;.define PULSE_AMPLITUDE_SCALE      ; Increase pulse size at lowest throttle
                                       ; (Does nothing unless the
                                       ;  PULSE_ENABLED is also defined)

   .define PULSE_WIDTH_SCALE           ; Increase pulse width at higher throttle
                                       ; (Does nothing unless the
                                       ;  PULSE_ENABLED is also defined)

.define MOMENTUM_ENABLED               ; Simulate momentum

.define DIRECTION_ENABLED              ; Direction/brake/STOP input
;*********************************************************

;*********************************************************
;* Variables for all compilations
;*********************************************************

   ;*******************
   ;* High frequency pwm settings
   ;*******************
.SET  pwm_period=                0xFF  ; The pwm clock ticks at 156.25nS per tick
                                       ; The pwm runs at pwm_period * 156.25nS
                                       ; The pwm frequency is 1/ (156.25nS * pwm_period)
                                       ; Maximum is 255 (0xFF)

.SET  pwm_max=                   0xFF  ; Maximum pwm "duty cycle" is equal to
                                       ; pwm_max / pwm_period

.SET  pwm_min=                   0x08  ; Minimum PWM "duty cycle" is equal to
                                       ; pwm_min / pwm_period

   
   ;*******************
   ;* Sample rate for throttle/ backemf/ pulses, etc
   ;*******************
.SET  pwm_settle_count=          16    ; settling time for analog measurements
                                       ; (x 40uS)
.SET  pwm_full_count=            250   ; time (x 40uS) between recalculations
                                       ; (255 max)

;*********************************************************
;* Variables associated with backemf speed control
;*********************************************************
.ifdef BACKEMF_ENABLED

   ;*******************
   ;* General settings
   ;*******************

   .SET  error_mult=             2     ; Error Multiplier.  The error between
                                       ; the throttle_set and back_emf is
                                       ; multiplied by 2^error_mult 
                                       ; limit -8 < n < 7
                                       ; 1/256 to 128

   .ifdef BACKEMF_SCALE_ENABLED
      .SET  err_scale=           5     ; Maximum error gain is as set by error_mult.
                                       ; Error gain decreases exponentially toward
                                       ; zero.  When the throttle is set at 2^err_scale
                                       ; the error be reduced by 2
                                       ; Err scale must not be less than 0.
                                       ; The sum of err_scale and err_mult MUST NOT
                                       ; exceed 7.
   .endif   ;BACKEMF_SCALE_ENABLED

   .ifdef LOWPASS_ENABLED

      .SET  emf_lowpass_gain=    2     ; tau is about 2^emf_lowpass_gain / 100 
                                       ; emf_lowpass_gain 0 to 8 (7?)
   .endif   ;LOWPASS_ENABLED

.endif   ;BACKEMF_ENABLED

;*********************************************************
;* Variables associated with lowspeed pulses
;*********************************************************
.ifdef PULSE_ENABLED
   .SET  pwm_min=                0x00  ; override previous setting
   .SET  pulse_slope_up=         0x0C
   .SET  pulse_slope_down=       0x08
   .SET  pulse_width_min=        0x08  ; minimum 0x01
.endif ; PULSE_ENABLED

;*********************************************************
;* Variables associated with momentum
;*********************************************************
.ifdef MOMENTUM_ENABLED
                                       ; MAY NOT BE MORE THAN 7

   .SET  accel_offset=1                ; divide tau_base by 2^accel_offset to give
                                       ; acceleration time constant
   .SET  brake_offset=2                ; divide tau_base by 2^brake_offset to give
                                       ; brake time constant


.endif MOMENTUM_ENABLED


;*********************************************************
;* Variables associated with direction
;*********************************************************
.ifdef DIRECTION_ENABLED

   .SET  direction_threshold=   16     ; direction relay will switch as long as
                                       ; the throttle (handle and momentum) are
                                       ; less than this value
.endif ;DIRECTION_ENABLED

;*********************************************************
;* Variables associated with traditional throttle
;*********************************************************
.ifdef TRADITIONAL_ENABLED


.ifdef SWITCH_LOWPASS_ENABLED
   .SET  stop_count_max=  10           ; number of identical samples to indicate
   .SET  brake_count_max=  10          ; number of identical samples to indicate
   .SET  reverse_count_max=  10        ; number of identical samples to indicate
   .SET  foreward_count_max=  10       ; number of identical samples to indicate
.endif ;SWITCH_LOWPASS_ENABLED

.ifdef THROTTLE_LOWPASS_ENABLED
   .SET  throttle_lowpass_gain=  5     ; tau is about 2^throttle_lowpass_gain / 100 
                                       ; throttle_lowpass_gain range 0 to 8 (7?)
.endif ;THROTTLE_LOWPASS_ENABLED

.ifdef MOMENTUM_LOWPASS_ENABLED
   .SET  momentum_lowpass_gain=  7     ; tau is about 2^momentum_lowpass_gain / 100 
                                       ; momentum_lowpass_gain range 0 to 8 (7?)
.endif ;MOMENTUM_LOWPASS_ENABLED

.ifdef LEDS_ENABLED

   .SET  accel_led_threshold=2         ; dont light led when acceleration/deceleration
                                       ; is closer than this to the actual speed
   .SET  decel_led_threshold=2         ; dont light led when acceleration/deceleration
                                       ; is closer than this to the actual speed
.endif;LEDS_ENABLED

.ifdef LOCO_LIGHT_ENABLED
   .SET  light_pwm=             6      ; run this pwm "duty cycle when throttle is off
.endif ;LOCO_LIGHT_ENABLED             ; to keep loco light on  
.endif ;TRADITIONAL_ENABLED

.LIST
