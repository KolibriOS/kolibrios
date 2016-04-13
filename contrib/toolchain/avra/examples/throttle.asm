;  ***************************************************************************************
;  * PWM MODEL RAILROAD THROTTLE                                                          *
;  *                                                                                      *
;  * "throttle.asm"                                                                       *
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
;  ***************************************************************************************
;  * Fixed Version for avra >= 1.2.2 because .DSEG cannot be used, if device has no SRAM  *
;  * B.Arenfeld, 10.05.2007                                                               *
;  ***************************************************************************************
;  ***************************************************************************************
;  *For Attiny 15                                                                         *
;  *                                                                                      *
;  *For compilation with:  Avra 0.70 or later                                             *
;  *                                                                                      *
;  *                       Atmel avrasm32.exe will not work becaue of the                 *
;  *                       use of preprocessor directives #ifdef, #ifndef                 *
;  *                       , and #endif, which Atmel doesn't support!                     *
;  *                                                                                      *
;  *Compiling requires the following files:                                               *
;  *     "tn15def.inc"              Labels and identifiers for tiny15                     *
;  *     "throttle_dev_set.inc"     Tiny 15 device settings                               *
;  *     "throttle_op_set.inc"      Operation settings   (THIS IS THE ONE TO EDIT THE     *
;  *                                   COMPLIE-TIME OPTIONS ON THE WAY THE THROTTLE       *
;  *                                   PERFORMS                                           *
;  *                                                                                      *
;  *Depending on the compile time options, the following files are also required:         *
;  *     "throttle_divide.asm"            Two division routines.  (one from atmel)        *
;  *     "throttle_set_lowpass.asm"       Lowpass filter on throttle handle               *
;  *     "throttle_momentum.asm"          Compute speed according to momentum             *
;  *     "throttle_momentum_lowpass.asm"  Lowpass filter on momentum handle               *
;  *     "throttle_backemf.asm"           Adjust pwm based on motor speed                 *
;  *     "throttle_pulse.asm"             Provide pulse assist at low motor speeds        *
;  *     "throttle_multiply.asm"          atmel multiplication routine                    *
;  *                                                                                      *
;  *Subroutine Categories and Stack                                                       *
;  *                                                                                      *
;  *  The Tiny 15 has a three level stack which handles return addresses for              *
;  *  subroutine calls and for interrupt service routines.  The categories below          *
;  *  ensure that the stack does not overflow. The four (4) categories are:               *
;  *                                                                                      *
;  *  -- Top Level Routines:        These routines are never called, and never            *
;  *                                return ("ret") and so the stack is, "empty"           *
;  *                                These routines may call any subroutine.               *
;  *                                Register Variables: Temp, Temp1, ...                  *
;  *                                                                                      *
;  *  -- First Level Subroutines:   Stack has 1 entry.  These routines may call           *
;  *                                Second Level Subroutines only.                        *
;  *                                Register Variables: A_Temp, A_Temp1, ...              *
;  *                                                                                      *
;  *  -- Second Level Subroutines:  Stack has 2 entries.  These routines may NOT          *
;  *                                call any subroutines.                                 *
;  *                                Register Variables: B_Temp, B_Temp1, ...              *
;  *                                                                                      *
;  *  -- Interrupt Service          These occur assynchronously, and therefor may         *
;  *                                occur during Second Level Subroutines.  If so,        *
;  *                                the stack has 3 entries and IS FULL.  These routines  *
;  *                                may NOT call any subroutines.                         *
;  *                                Register Variables: I_Temp, I_Temp1, ...              *
;  *                                                                                      *
;  *  Register variables are reserved for each level of routine.  Each level may freely   *
;  *  use the register variables for it's own level.  Some sharing of variables may       *
;  *  occur subject to these guidelines:                                                  *
;  *  -- No category execpt Interrupt Service may use Interrupt Service variables.  This  *
;  *     is because ISRs occur asynchronously.                                            *
;  *  -- A category may NOT use variables reserved for higher level categories.           *
;  *  -- A category may use lower level routine variables as long as their use does not   *
;  *     span any subroutine calls.                                                       *
;  *                                                                                      *
;  *Other settings                                                                        *
;  *  There is no way to put these settings into this file, but these must also be        *
;  *  done:                                                                               *
;  *                                                                                      *
;  *  BODLEVEL:                     0        4.0V                                         *
;  *  BOODEN:                       0        ENABLED  (brown out detection)               *
;  *  SPIEN:                        0        ENABLED  (in circuit programming)            *
;  *  RSTDISBL                      0        DISABLED (reset on PB5)                      *
;  *  CKSEL                         11                (very quickly rising power)         *
;  *  LB1                           1                 (LB1 & LB2: No lock)                *
;  *  LB2                           1                                                     *
;  *                                                                                      *
;  *  Calibration byte              into flash byte address as specified in               *
;  *                                osccal_location.                                      *
;  *                                                                                      *
;  *  Notes regarding these settings:                                                     *
;  *  --Brown out detection.  The datasheet warns against using the EEPROM without        *
;  *    brownout detection because of the possibility of errant execution at very low     *
;  *    voltage levels.                                                                   *
;  *                                                                                      *
;   **************************************************************************************



;*****************************************************************************************
;*****************************************************************************************
;* Included files                                                                         *
;*****************************************************************************************
;*****************************************************************************************
.INCLUDE    "tn15def.inc"              ; Labels and identifiers for tiny15
.INCLUDE    "throttle_op_set.inc"      ; Operation settings
.INCLUDE    "throttle_dev_set.inc"     ; Tiny 15 device settings

;*****************************************************************************************
;*****************************************************************************************
;* DATA TABLE                                                                             *
;*****************************************************************************************
;*****************************************************************************************
;.CSEG
;.ORG    0x01E0                        ; Program .ORG 0x01E0 actually means byte
                                       ; location 0x03C0.

;*****************************************************************************************
;*****************************************************************************************
;* Data: reserved for OSCCAL byte                                                         *
;*****************************************************************************************
;*****************************************************************************************

; Fix : The Tiny15 has NO SRAM. Use of .DSEG is invalid ! 
;.DSEG
;
;.SET  osccal_location = 0x3FF          ;Place in the last byte of program memory.
;                                       ;High byte of program memory 1FF
;
;.ORG  osccal_location                  ;reserve this byte for oscillator
;.BYTE 1                                ;calibration value


; Fixed Version for avra >= 1.2.2
.CSEG
.EQU    osccal_location = 0x3FF         ; The last flash byte is used for the calibration 
					; value and is replaced by the programmer

; Now reserve the last word in flash memory. If you are sure, that the program doesn't use
; the last flash word, you can disable the following lines. If not, it's better to enable
; them to check for overlapping code segments.

.ORG	0x01FF				; Last word in flash memory
.DB	0xff,0xff			; Fill with dummy values. Only last byte is used
					; but flash is organized in words.
					; 0xff is the value of unprogrammed flash 


;*****************************************************************************************
;*****************************************************************************************
;* Reset and Interrupt Vectors                                                            *
;*****************************************************************************************
;*****************************************************************************************
.CSEG
.ORG  0x000 
   rjmp  ST_RESET

.ifdef OVERLOAD_ENABLED
   rjmp  ST_PWM_LEVEL_OFF              ; INT0 interrupt handler
.else
   reti                                ; Not used.
.endif ;OVERLOAD_ENABLED

   reti                                ; Not used.    rjmp  PIN_CHANGE 
   reti                                ; Not used.    rjmp  TIM1_CMP
   reti                                ; Not used.    rjmp  TIM1_OVF
   reti                                ; Not used.    rjmp  TIM0_OVF
   reti                                ; Not used.    rjmp  EE_RDY
   reti                                ; Not used.    rjmp  ANA_COMP
   reti                                ; Not used.    rjmp  ADC


;*****************************************************************************************
;*****************************************************************************************
;* Top level routines.  The basic program is a state machine, with states all being       *
;*                      top level routines.  These routines are never used as subroutines *
;*                      and therefore can call any subroutine.                            *
;*****************************************************************************************
;*****************************************************************************************

;********************************************************************************
;* ST_RESET                                                                      *
;* This is power on reset.  The reset vector points here.                        *
;*                                                                               *
;* Inputs:  none                                                                 *
;* Returns: none                                                                 *
;* Changed: B_Temp                                                               *    
;* Calls:                                                                        *  
;* Goto:    ST_MOTOR_OFF                                                         *
;********************************************************************************
ST_RESET:
   cli                                 ; Disable interrupts

   ldi   B_Temp,(dir_out_port_bit | pwm_port_bit | dir_in_port_bit | momentum_port_bit)
   out   DDRB,B_Temp                   ; Assign output port directions.
                                       ; Inclusion in the above list makes the
                                       ; port an output port

   ldi   B_Temp,0x00                   ; A "1" makes output logic level high
   out   PORTB,B_Temp                  ; A "1" assigns pullups on inputs
                                       ; Therefore all outputs are at logic low, and
                                       ; all inputs do not have a pullup assigned

   ldi   B_Temp,acsr_val               ; Disable comparator and interrupt
   out   ACSR,B_Temp                   ; Using port for PWM
                                       ; (comparator defaults to powered up)

   ldi   B_Temp,0b01000010             ; Disable pullups.
   out   MCUCR,B_Temp                  ; Set sleep mode (moot)
                                       ; INT0 interrupt on falling edge
                                       
   ldi   ZL,low(osccal_location)       ; r30
   ldi   ZH,high(osccal_location)      ; r31
   lpm   
   out   OSCCAL,Implicit               ; Place calibration byte

   ldi   B_Temp,0b00001010             ; Enable watchdog
   out   WDTCR,B_Temp                  ; timout 64mS (nom)

   ldi   B_Temp1,pwm_period            ; Set pwm oscillator period
   out   OCR1B,B_Temp1

   ldi   B_Temp1,tccr1_enable_t1       ; Turn on the PWM oscillator 
   out   TCCR1,B_Temp1

   ldi   Flags_1,(0b00000000 | F_stop) ; Set emergency stop flag so that
                                       ; throttle doesn't start on powerup
.ifdef TRADITIONAL_ENABLED
   .ifdef MOMENTUM_LOWPASS_ENABLED
      clr   momentum_lo_prev           ; MOMENTUM LOWPASS
      clr   momentum_hi_prev           ; Clear the history
   .endif ;MOMENTUM_LOWPASS_ENABLED
.endif ;TRADITIONAL_ENABLED

;  rjmp  ST_EMERGENCY_STOP             ; ***EXIT STATE***

;********************************************************************************
;* ST_EMERGENCY_STOP                                                             *
;*                                                                               *
;* Reset to "off" state.                                                         *
;* Clear global variables associated with momentum and lowpass filters.          *
;*                                                                               *
;* Inputs:  none                                                                 *
;* Returns: none                                                                 *
;* Changed: Global variables cleared                                             *    
;* Calls:   None                                                                 *  
;* Goto:    ST_PWM_LEVEL_OFF           If throttle is zero                       *
;********************************************************************************
ST_EMERGENCY_STOP:

.ifdef BACKEMF_ENABLED
   .ifdef LOWPASS_ENABLED              ; BACKEMF LOWPASS
      clr   error_hi_prev              ; Clear the history
      clr   error_lo_prev
   .endif ;LOWPASS_ENABLED
.endif ;BACKEMF_ENABLED

.ifdef MOMENTUM_ENABLED                ; MOMENTUM
   clr   speed_lo_prev                 ; Clear the history
   clr   speed_hi_prev
.endif ;MOMENTUM_ENABLED

.ifdef TRADITIONAL_ENABLED
   .ifdef WALKAROUND_ENABLED
      clr   throttle_hold              ; Clear the history
   .endif ;WALKAROUND_ENABLED

   .ifdef THROTTLE_LOWPASS_ENABLED
      clr   throttle_lo_prev           ; THROTTLE LOWPASS
      clr   throttle_hi_prev           ; Clear the history
   .endif ;THROTTLE_LOWPASS_ENABLED

.endif ;TRADITIONAL_ENABLED

;  rjmp  ST_PWM_LEVEL_OFF              ; ***EXIT STATE***


;********************************************************************************
;* ST_PWM_LEVEL_OFF                                                              *
;* ST_MEASUREMENT_SETTLE                                                         *
;* 1. If entered at ST_PWM_LEVEL_OFF turn pwm off                                *
;* 2. Set the ADC ports to input                                                 *
;* 3. Pause to let ADC inputs (including back-emf) settle.                       *
;* 4. Read the throttle controller.                                              *
;* 5. Set LED ports and overload ports (also ADC inputs) to output               *
;* 6. If throttle_set is not zero, or if motor is still running by momentum      *
;*    continue running motor (jump to ST_SET_NEW_PWM)                            *
;* 7. If throttle set is zero and motor is not running, then set the direction   *
;*    relay and test backemf input to determine backemf mode.                    *
;* 8. Turn of motor (jump to ST_PWM_LEVEL_OFF)                                   *
;*                                                                               *
;* Inputs:  none                                                                 *
;* Returns: none                                                                 *
;* Changed: B_Temp, B_Temp1                                                      *    
;* Calls:   READ_THROTTLE                                                        *  
;* Goto:    ST_PWM_LEVEL_OFF           If throttle is zero                       *
;*          ST_SET_NEW_PWM             After delay                               *
;********************************************************************************
ST_PWM_LEVEL_OFF:
   clr   B_Temp                        ; Set PWM duty = 0.
   rcall SET_PWM_DUTY                  ; i.e. turn off the power

ST_MEASUREMENT_SETTLE:
   ;********************************************
   ;* Set all measurement ports for input and pause.
   ;* During the pause:
   ;* 1. inductive current in the locomotive falls to zero, and
   ;*    the backemf voltage appears on the backemf port
   ;* 2. the momentum, direction, and throttle voltages stabilize
   ;********************************************
.ifdef TRADITIONAL_ENABLED
   .ifdef LEDS_ENABLED
      cbi   DDRB,momentum_port         ; Make input port (pullup must be disabled)
      cbi   DDRB,dir_in_port           ; Make input port (pullup must be disabled)
   .endif ;LEDS_ENABLED

   .ifdef OVERLOAD_ENABLED
      in    B_Temp,GIMSK               ; disable INT0 interrupt
      andi  B_Temp,0b10111111
      out   GIMSK,B_Temp

      cbi   DDRB,throttle_port         ; Make input port (pullup must be disabled)
   .endif ;OVERLOAD_ENABLED
.endif ;TRADITIONAL_ENABLED

   sei                                 ; Enable interrupts
   wdr                                 ; Reset watchdog timer

   ldi   B_Temp1,pwm_full_count        ; Pause for inputs to settle
   rcall COUNT_PWM_CYCLES
   clr   Cycle_count

   ;********************************************
   ;* Read the input ports and make some
   ;* mode decisions based on those inputs.
   ;********************************************

   rcall READ_THROTTLE                 ; Find throttle handle position in throttle_set

.ifdef TRADITIONAL_ENABLED
   .ifdef MOMENTUM_ENABLED
      .ifdef MOMENTUM_LOWPASS_ENABLED
         .include "throttle_momentum_lowpass.asm"
      .endif;MOMENTUM_LOWPASS_ENABLED
   .endif; MOMENTUM_ENABLED
.endif ;TRADITIONAL_ENABLED

.ifdef DIRECTION_ENABLED               ; Check Stop, and Adjust Direction
CHECKING_STOP:
   sbrs  Flags_1,BF_stop               ; Check stop flag is set
   rjmp  DONE_CHECKING_STOP

   cpi   throttle_set,0x00             ; If throttle handle is at zero
   brne  ST_EMERGENCY_STOP             ; reset the emergency stop flag
   cbr   Flags_1,F_stop                ; reset emergency stop flag.
   rjmp  ST_EMERGENCY_STOP             ; ALWAYS STOP
DONE_CHECKING_STOP:

CHECKING_DIRECTION:

   .ifdef MOMENTUM_ENABLED
      mov   B_Temp,speed_hi_prev       ; Don't set direction unless the actual 
      cpi   B_Temp,direction_threshold ; speed is less than direction_threshold
      brsh  DONE_CHECKING_DIRECTION
   .else
      cpi   throttle_set,0x00          ; Don't set direction unless the throttle
      brne  DONE_CHECKING_DIRECTION    ; handle is at zero
   .endif ;MOMENTUM_ENABLED

   sbic  PORTB,dir_out_port            ; Find port direction
   rjmp  PORT_REVERSE
   ;rjmp  PORT_FORWARD

   PORT_FORWARD:
   sbrs  Flags_1,BF_reverse            ; If port says forward
   rjmp  DONE_CHECKING_DIRECTION
   sbi   PORTB,dir_out_port            ; But flag says reverse, then reverse
   rjmp  ST_EMERGENCY_STOP
      
   PORT_REVERSE:
   sbrc  Flags_1,BF_reverse            ; If port says reverse
   rjmp  DONE_CHECKING_DIRECTION
   cbi   PORTB,dir_out_port            ; But flag says foreward, then forward
   rjmp  ST_EMERGENCY_STOP

DONE_CHECKING_DIRECTION:
.endif ;DIRECTION_ENABLED

.ifdef TRADITIONAL_ENABLED
   .ifdef THROTTLE_LOWPASS_ENABLED
      .include "throttle_set_lowpass.asm"
   .endif ;THROTTLE_LOWPASS_ENABLED
.endif ;TRADITIONAL_ENABLED

   cpi   throttle_set,0x00             ; Run the pwm unless the throttle
   brne  ST_SET_NEW_PWM                ; is zero

.ifdef MOMENTUM_ENABLED
   mov   B_Temp,speed_hi_prev          ; In momentum mode, run the pwm unless
   cpi   B_Temp,0x00                   ; the actual throttle setting reaches zero
   brne  ST_SET_NEW_PWM
.endif ;MOMENTUM_ENABLED

   ;********************************************
   ;* Only arrive here if the throttle is set for 0 speed
   ;* and the locomotive is actually stopped (momentum)
   ;********************************************

.ifdef BACKEMF_ENABLED
   ;********************************************
   ;* The backemf measurement should be at or near zero,
   ;* since the locomotive is stopped.  If it isn't,
   ;* do not use backemf speed control.
   ;********************************************
   sbr   Flags_1,F_use_backemf         ; Default to use backemf

   rcall ADC_SETUP_EMF                 ; 4 lines read the backemf
WAIT_FOR_VALID:
   sbis  ADCSR,ADIF
   rjmp  WAIT_FOR_VALID
   
   in    B_Temp,ADCH                   ; Read the measurement

   
   cpi   B_Temp,0x40                   ; Test measurement
   brlo  END_CHECK_BACKEMF_MODE        ; If small, use backemf adjustment.

   cbr   Flags_1,F_use_backemf         ; Otherwise, don't use backemf
END_CHECK_BACKEMF_MODE:
.endif ;BACKEMF_ENABLED

.ifdef TRADITIONAL_ENABLED
   .ifdef LOCO_LIGHT_ENABLED 
      ldi   throttle_set,light_pwm
      rjmp  STABLE_PWM_SET
   .else
      rjmp  ST_PWM_LEVEL_OFF
   .endif ;LOCO_LIGHT_ON
.else
   rjmp  ST_PWM_LEVEL_OFF
.endif ;TRADITIONAL_ENABLED


;********************************************************************************
;* ST_SET_NEW_PWM                                                                *
;* Compute the pwm setting based upon momentum, backemf, and throttle setting    *
;* Inputs:  throttle_set                                                         *
;* Returns: none                                                                 *
;* Changed: throttle_set, other variables in included files                      *    
;* Calls:   various in included files                                            *  
;* Goto:    ST_PWM_LEVEL_ON                                                      *
;********************************************************************************
ST_SET_NEW_PWM:

.ifdef TRADITIONAL_ENABLED
   .ifdef LEDS_ENABLED
      cbi   PORTB,dir_in_port          ; logic low out (turn off LED)
      sbi   DDRB,dir_in_port           ; Assign output to drive led (output is low)

      cbi   PORTB,momentum_port        ; logic low out (turn off LED)
      sbi   DDRB,momentum_port         ; Assign output to drive led (output is low)
   .endif ;LEDS_ENABLED

   .ifdef OVERLOAD_ENABLED
      ;********************************************
      ;* The thottle port is driven to logic high.  If this port gets pulled
      ;* low (overload), this triggers the INT0 interrupt, which will shut off
      ;* the pwm.
      ;********************************************
      sbi   PORTB,throttle_port        ; Logic hi out.   
      sbi   DDRB,throttle_port         ; Make output port.
   .endif ;OVERLOAD_ENABLED
.endif ;TRADITIONAL_ENABLED

.ifdef MOMENTUM_ENABLED
   .include "throttle_momentum.asm"    ; momentum adjustment
.endif ;MOMENTUM_ENABLED

.ifdef TRADITIONAL_ENABLED
   .ifdef WALKAROUND_ENABLED
      mov   throttle_hold,throttle_set
   .endif ;WALKAROUND_ENABLED
.endif ;TRADITIONAL_ENABLED

.ifdef BACKEMF_ENABLED                 ;********************************************
                                       ; Adjust throttle_set according to 
                                       ; measured backemf.
                                       ;********************************************
                                       
   sbrs  Flags_1,BF_use_backemf        ; If the flag is set, use backemf
   rjmp  DONT_BACKEMF                  ; Otherwise, don't

   .include "throttle_backemf.asm"     
                                       ; If using backemf, don't use throttle_scale
   rjmp  ST_PWM_LEVEL_ON               ; ***EXIT STATE***

   DONT_BACKEMF:
.endif ;BACKEMF


   ;*****************************************************************
   ;* Scale the throttle_set between 0 and pwm_period                *
   ;* multiply pwm_period and throttle_set and divide by 256         *
   ;* read answer from hi byte of return.                            *
   ;*****************************************************************   


   HILOCAL1       _main_scale_multiplicand
   B_TEMPLOCAL    _main_scale_multiplier
   B_TEMPLOCAL1   _main_scale_result_hi

   ldi   _main_scale_multiplicand,pwm_period - pwm_min
   mov   _main_scale_multiplier,throttle_set
   rcall mpy8u                                           ; multiply
   mov   throttle_set,_main_scale_result_hi              ; read result

;  rjmp  ST_PWM_LEVEL_ON               ; ***EXIT STATE***


;********************************************************************************
;* ST_PWM_LEVEL_ON                                                               *
;* 1. Enable overload testing                                                    *
;* 2. Produce pulse if required                                                  *
;* 3. Run pwm at throttle_set                                                    *
;* 4. Wait for a while                                                           *
;*                                                                               *
;* Inputs:  throttle_set                                                         *
;* Returns: none                                                                 *
;* Changed: B_Temp, B_Temp1, various                                             *    
;* Calls:   SET_PWM_DUTY                                                         *
;*          COUNT_PWM_CYCLES                                                     *
;* Goto:    ST_PWM_LEVEL_OFF           After PWM goes to off state               *
;********************************************************************************
ST_PWM_LEVEL_ON:

.ifdef TRADITIONAL_ENABLED
   .ifdef OVERLOAD_ENABLED
      ldi   B_Temp,0b01000000          ; clear INT0 interrupt
      out   GIFR,B_Temp

      in    B_Temp,GIMSK               ; enable INT0 interrupt
      ori   B_Temp,0b01000000
      out   GIMSK,B_Temp
   .endif ;OVERLOAD_ENABLED
.endif ;TRADITIONAL_ENABLED

   cpi   throttle_set,light_pwm        ; never run pwm lower than light_pwm level
   brsh  DONE_CHECKING_MINIMUM
   ldi   throttle_set,light_pwm
   rjmp  STABLE_PWM_SET

DONE_CHECKING_MINIMUM:

.ifdef PULSE_ENABLED                   ; Produce pulses during output
   .ifdef   BACKEMF_ENABLED
      sbrc  Flags_1,BF_use_backemf     ; If the flag is set to use backemf
      rjmp  STABLE_PWM_SET             ; don't pulse
   .endif   ;BACKEMF_ENABLED
   
                                       ; Pass in:  throttle_set
   .include "throttle_pulse.asm"
.endif ;PULSE_ENABLED


STABLE_PWM_SET:
   mov   B_Temp,throttle_set           ; Stabilize at throttle_set
   rcall SET_PWM_DUTY

   ldi   B_Temp1,pwm_full_count-pwm_settle_count
   rcall COUNT_PWM_CYCLES              ; Wait for end of interval

.ifdef BACKEMF_ENABLED
   sbrc  Flags_1,BF_use_backemf        ; If the flag is set to use backemf
   rjmp ST_PWM_LEVEL_OFF               ; ***EXIT STATE***
.endif ;BACKEMF_ENABLED
   rjmp ST_MEASUREMENT_SETTLE          ; ***EXIT STATE***

;*****************************************************************************************
;*****************************************************************************************
;* First Level Subroutines.                                                               *
;* These routines include the routines which are called by other code and also call       *
;* Second Level Subroutines.                                                              *
;*****************************************************************************************
;*****************************************************************************************

;********************************************************************************
;* READ_THROTTLE                                                                 *
;* First Level Subroutine                                                        *
;*                                                                               *
;* Read the throttle controls, which are:                                        *
;*    Momentum level (analog): Returned in "momentum_set"                        *
;*       Returns and 8 bit number, with '0' meaning minimum momentum.            *
;*                                                                               *
;*    Direction, brake, and stop switch.                                         *
;*       Returns value in flags: F_brake, F_reverse, and F_stop                  *
;*                                                                               *
;*    Throttle setting (analog: Returned in "throttle_set"                       *
;*       Returns an 8 bit number (0x00 to 0xFF; 0 to 255),                       *
;*       where '0' means "motor off" and 0xFF (255) means full speed.            *
;*                                                                               *
;* If a speed table is implemented, it will be in this routine                   *
;*                                                                               *
;* Just now, this value comes from the analog input and is converted by the      *
;* ADC.  The raw 8 bit number is returned.                                       *
;*                                                                               *
;* Inputs:  None                                                                 *
;* Returns: Momentum setting in "momentum_set"                                   *
;*          Switch positions in F_brake, F_reverse, and F_stop                   *
;*          Throttle setting in "throttle_set"                                   *
;* Changed: Cycle_count incremented by up to 5                                   *
;* Calls:   ADC_SETUP_MOMENTUM                                                   *
;*          ADC_SETUP_DIRECTION                                                  *
;*          ADC_SETUP_THROTTLE                                                   *
;********************************************************************************
READ_THROTTLE:
.ifdef TRADITIONAL_ENABLED
   .ifdef DIRECTION_ENABLED
      ;********************************************
      ;* Measure the direction, brake, and stop switches and
      ;* set the flags appropriately
      ;********************************************
      rcall ADC_SETUP_DIRECTION        ; Setup to read
   WAIT_FOR_VALID_DIRECTION:
      sbis  ADCSR,ADIF                 ; Check for ADC completion
      rjmp  WAIT_FOR_VALID_DIRECTION

      in    B_Temp,ADCH                ; Read value

      .ifdef WALKAROUND_ENABLED
         cpi   B_Temp,0x90             ; Above this threshold 
                                       ; deactivates handheld controller
;        brsh  HOLD_THROTTLE

         brlo  HOLD_THROTTLE_NOT
         rjmp  HOLD_THROTTLE
HOLD_THROTTLE_NOT:         
      .endif ;WALKAROUND_ENABLED

      cpi   B_Temp,0x1B                ; Below this threshold (0.53V) sets 'stop' flag
      brsh  TEST_BRAKE_LEVEL           ; Typical stop voltage is 0.30V

.ifdef SWITCH_LOWPASS_ENABLED
      sbrs  Flags_2,BF_stop_count      ; If the stop count flag is not set, then
      clr   Flags_2                    ; set the counter to zero

      cbr   Flags_2,F_stop_count       ; clear the stop count flag
      inc   Flags_2                    ; increment the counter

      cpi   Flags_2,stop_count_max     ; compare the count to the maximum
      sbr   Flags_2,F_stop_count       ; set the stop count flag

      brlo  END_READ_DIRECTION         ; if the count is lower, don't change status flag

      dec   Flags_2,F_stop_count       ; decrement stop count flag
.endif;SWITCH_LOWPASS_ENABLED

      sbr   Flags_1,F_stop
      rjmp  END_READ_DIRECTION

   TEST_BRAKE_LEVEL:
      cpi   B_Temp,0x37                ; Below this threshold (1.07V) sets 'brake' flag
      brsh  TEST_REVERSE_LEVEL         ; Typical brake voltage 0.87V

.ifdef SWITCH_LOWPASS_ENABLED
      sbrs  Flags_2,BF_brake_count     ; If the brake count flag is not set, then
      clr   Flags_2                    ; set the counter to zero

      cbr   Flags_2,F_brake_count      ; clear the break count flag
      inc   Flags_2                    ; increment the counter

      cpi   Flags_2,brake_count_max    ; compare the count to the maximum
      sbr   Flags_2,F_brake_count      ; set the break count flag

      brlo  END_READ_DIRECTION         ; if the count is lower, don't change status flag

      dec   Flags_2,F_brake_count      ; decrement break count flag
.endif;SWITCH_LOWPASS_ENABLED

      sbr   Flags_1,F_brake
      rjmp  END_READ_DIRECTION

   TEST_REVERSE_LEVEL:
      cpi   B_Temp,0x53                ; Below this threshold (1.62V) sets 'reverse' flag
      brsh  TEST_FOREWARD_LEVEL        ; Typical reverse level 1.40V

.ifdef SWITCH_LOWPASS_ENABLED
      sbrs  Flags_2,BF_reverse_count   ; If the reverse count flag is not set, then
      clr   Flags_2                    ; set the counter to zero

      cbr   Flags_2,F_reverse_count    ; clear the reverse count flag
      inc   Flags_2                    ; increment the counter

      cpi   Flags_2,reverse_count_max  ; compare the count to the maximum
      sbr   Flags_2,F_reverse_count    ; set the reverse count flag

      brlo  END_READ_DIRECTION         ; if the count is lower, don't change status flag

      dec   Flags_2,F_reverse_count    ; decrement reverse count flag
.endif;SWITCH_LOWPASS_ENABLED

      cbr  Flags_1,F_brake             ; Clear brake flag
      sbr   Flags_1,F_reverse          ; Set brake flag
      rjmp  END_READ_DIRECTION

   TEST_FOREWARD_LEVEL:                ; Typical "nothing" 1.95V
   ;no test required

.ifdef SWITCH_LOWPASS_ENABLED
      sbrs  Flags_2,BF_foreward_count  ; If the foreward count flag is not set, then
      clr   Flags_2                    ; set the counter to zero

      cbr   Flags_2,F_foreward_count   ; clear the foreward count flag
      inc   Flags_2                    ; increment the counter

      cpi   Flags_2,foreward_count_max ; compare the count to the maximum
      sbr   Flags_2,F_foreward_count   ; set the foreward count flag

      brlo  END_READ_DIRECTION         ; if the count is lower, don't change status flag

      dec   Flags_2,F_foreward_count   ; decrement forward count flag
.endif;SWITCH_LOWPASS_ENABLED

      cbr  Flags_1,F_brake             ; Clear brake flag
      cbr  Flags_1,F_reverse           ; Clear reverse flag (i.e., foreward)

   END_READ_DIRECTION:
   .endif ;DIRECTION_ENABLED

   .ifdef MOMENTUM_ENABLED
      ;********************************************
      ;* Measure and adjust the momentum input
      ;********************************************
      rcall ADC_SETUP_MOMENTUM         ; Setup to read
   WAIT_FOR_VALID_MOMENTUM:
      sbis  ADCSR,ADIF                 ; Wait for ADC completion
      rjmp  WAIT_FOR_VALID_MOMENTUM
   
      in    momentum_set,ADCH          ; Read value

      .ifdef WALKAROUND_ENABLED
         ldi   B_Temp,0x90
         cp    momentum_set,B_Temp     ; Above this threshold 
                                       ; deactivates handheld controller
         brsh  HOLD_THROTTLE
      .endif ;WALKAROUND_ENABLED

      ldi   B_Temp,0x40                      
      sub   momentum_set,B_Temp        ; Subtract offset (1/4 of 0xFF)
   
      brsh  END_READ_MOMENTUM                
      sub   momentum_set,momentum_set  ; If smaller than offset, make zero

   END_READ_MOMENTUM:
   .endif ;MOMENTUM_ENABLED

   ;********************************************
   ;* Read the throttle level
   ;********************************************
   rcall ADC_SETUP_THROTTLE
WAIT_FOR_VALID_THROTTLE:
   sbis  ADCSR,ADIF                    ; Check for ADC completion
   rjmp  WAIT_FOR_VALID_THROTTLE

   in    throttle_set,ADCH             ; Read throttle value

   subi  throttle_set,0x08             ; Subtract offset (force zero)
   brcc  DONE_READ_THROTTLE            ; If new throttle is negative,

   clr   throttle_set                  ; make throttle zero.
DONE_READ_THROTTLE:  

   subi  Cycle_count,256-3             ; Normal arrival here occurs after 3 adc
   ret                                 ; conversions, which take 195uS, or 4.875
                                       ; pwm cycles
.ifdef WALKAROUND_ENABLED
   HOLD_THROTTLE:                      ; Normal arrival here occurs after 1 adc
                                       ; conversion, which takes 65uS, or 1.625
                                       ; pwm cycles

   cbr   Flags_1,F_brake               ; Clear brake flag
   mov   throttle_set,throttle_hold    ; Use previous value.

   .ifdef SWITCH_LOWPASS_ENABLED
      clr   Flags_2
   .endif;SWITCH_LOWPASS_ENABLED

   .ifdef MOMENTUM_ENABLED
      ldi   B_Temp,0x40
      mov   momentum_set,B_Temp        ; 'long' momentum
   .endif ;MOMENTUM_ENABLED
      ret
.endif ;WALKAROUND_ENABLED

.else  ;NOT TRADITIONAL_THROTTLE
      sbr   Flags_1,F_stop
      ret
.endif ;TRADITIONAL_THROTTLE


;********************************************************************************
;* COUNT_PWM_CYCLES                                                              *
;* First evel Subroutine                                                         *
;*                                                                               *
;* Increment Cycle_count timer each PWM cycle.                                   *
;* Return when Cycle_count = B_Temp1                                             *
;*                                                                               *
;* Inputs:  B_Temp1                    Exit when count reaches this number       *
;* Returns: None                                                                 *
;* Changed: B_Temp,Cycle_count                                                   *
;* Calls:   None                                                                 *
;********************************************************************************
COUNT_PWM_CYCLES:
   in    B_Temp,TIFR                   ; Wait for pwm timer to reset                   
   sbrs  B_Temp,OCF1A
   rjmp  COUNT_PWM_CYCLES
   
   ldi   B_Temp,0b01000000             ; reset interrupt flag
   out   TIFR,B_Temp

   inc   Cycle_count                   ; increment counter and repeat
   cp    Cycle_count,B_Temp1
   brne  COUNT_PWM_CYCLES
   ret

;*****************************************************************************************
;*****************************************************************************************
;* Second Level Subroutines.                                                              *
;* These routines make no further subroutine calls.                                       *
;*****************************************************************************************
;*****************************************************************************************

.include "throttle_divide.asm"

.include "throttle_multiply.asm"

;********************************************************************************
;* SET_PWM_DUTY                                                                  *
;* Second Level Subroutine                                                       *
;*                                                                               *
;* Inputs:  B_Temp            PWM on count                                       *  
;* Returns: None                                                                 *
;* Changed: None                                                                 *
;* Calls:   Not allowed                                                          *
;********************************************************************************
SET_PWM_DUTY:

   out   OCR1A,B_Temp                  ; Set the PWM equal to the input B_Temp
   ret

;********************************************************************************
;* ADC_SETUP_DIRECTION                                                           *
;* ADC_SETUP_MOMENTUM                                                            *
;* ADC_SETUP_THROTTLE                                                            *
;* ADC_SETUP_BACK_EMF                                                            *
;* Second Level Subroutine                                                       *
;*                                                                               *
;* The ADC is switched off, and restarted on the selected port.                  *
;*                                                                               *
;* Inputs:  None                                                                 *
;* Returns: None                                                                 *
;* Changed: Various B_Temp variables                                             *
;* Calls:   Not allowed                                                          *
;********************************************************************************
.ifdef DIRECTION_ENABLED
ADC_SETUP_DIRECTION:
   ldi   B_Temp,admux_direction        ; Setup MUX for direction/brake measurement
   rjmp  ADC_SETUP
.endif ;DIRECTION_ENABLED

.ifdef MOMENTUM_ENABLED
ADC_SETUP_MOMENTUM:
   ldi   B_Temp,admux_momentum         ; Setup MUX for momentum set measurement
   rjmp  ADC_SETUP
.endif ;MOMENUTM_ENABLED

.ifdef BACKEMF_ENABLED
ADC_SETUP_EMF:
   ldi   B_Temp,admux_emf              ; Setup MUX for back_emf measurement
   rjmp  ADC_SETUP
.endif ;BACKEMF_ENABLED

.ifdef TRADITIONAL_ENABLED
ADC_SETUP_THROTTLE:
   ldi   B_Temp,admux_throttle         ; Setup MUX for analog measure
;  rjmp  ADC_SETUP                     ; of throttle.
.endif ;TRADITIONAL_ENABLED   
   
ADC_SETUP:
   ldi   B_Temp1,adcsr_off             ; Turn off the ADC
   out   ADCSR,B_Temp1                 ;  
   out   ADMUX,B_Temp                  ; Setup MUX as per entry point
   ldi   B_Temp,adcsr_enable           ; enable ADC, disable interrupt, clear
   out   ADCSR,B_Temp                  ; interrupt flag, free-running.
   
   ret


;*****************************************************************************************
;*****************************************************************************************
;* Interrupt Service routines.                                                            *
;* These routines can occur assynchronously.  Therfore, they might occur during a second  *
;* level routine.  Therefore THEY MAY NOT CALL ANY SUBROUTINES.                           *
;*****************************************************************************************
;*****************************************************************************************
