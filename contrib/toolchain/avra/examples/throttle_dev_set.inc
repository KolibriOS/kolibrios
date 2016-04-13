;throttle_dev_set.inc

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
;* Hardware Settings                                     *
;*********************************************************

;
; REGISTER ASSGINMENTS
; Note that registes 16 - 31 are acessed by a larger instruction
; set than registers 0 - 15.
;


;*********************************************************
; REGISTER ASSIGNMENTS
; GLOBAL VISIBILITY  ;all functions have access to this NAME of the register 
; GLOBAL DURATION    ;the register always MEANS the same thing.
;*********************************************************
   .DEF  Flags_1=             r16      ; Global.   Status flags
   .DEF  Cycle_count=         r17      ; Global.   Count PWM cycles
   .DEF  throttle_set=        r18      ; Global.   Current Throttle Setting

.ifdef SWITCH_LOWPASS ENABLED
   .DEF  Flags_2=        r19      ; Global.   Switch "status" flags
.endif SWITCH_LOWPASS_ENABLED

.ifdef MOMENTUM_ENABLED
   .DEF  momentum_set=        r4       ; Global.   Input control
.endif ;MOMENTUM_ENABLED

.ifdef TRADITIONAL_ENABLED
   .ifdef WALKAROUND_ENABLED
      .DEF  throttle_hold=    r14      ; Global.   Previous throttle setting
      .DEF  Flags_2=          r19      ; Global.   Switch "status" flags
   .endif ;WALKAROUND_ENABLED
.endif ;TRADITIONAL_ENABLED

;*********************************************************
; REGISTER ASSIGNMENTS
; LOCAL VISIBILITY   ;only the translation unit has (should have)
;                     access to any NAME of the register
; GLOBAL DURATION    ;the register always MEANS the same thing.
;*********************************************************
.ifdef MOMENTUM_ENABLED
   .ifdef LOWPASS_ENABLED
      .DEF  error_hi_prev=    r2       ; Global.   History of error  
      .DEF  error_lo_prev=    r3       ; Global.   History of error
   .endif ;LOWPASS_ENABLED
.endif ;MOMENTUM_ENABLED

.ifdef TRADITIONAL_ENABLED

   .ifdef THROTTLE_LOWPASS_ENABLED
      .DEF  throttle_hi_prev= r9       ; Global.   History of throttle handle
      .DEF  throttle_lo_prev= r10      ; Global.   History of throttle handle
   .endif ;THROTTLE_LOWPASS_ENABLED

   .ifdef MOMENTUM_LOWPASS_ENABLED
      .DEF  momentum_hi_prev= r11      ; Global.   History of momentum handle
      .DEF  momentum_lo_prev= r12      ; Global.   History of momentum handle
   .endif ;THROTTLE_LOWPASS_ENABLED

.endif ;TRADITIONAL_ENABLED

.ifdef MOMENTUM_ENABLED
   .DEF  speed_hi_prev=       r5       ; Global.   Speed at last sample time
   .DEF  speed_lo_prev=       r6       ; Global.   Speed at last sample time
.endif ;MOMENTUM_ENABLED


;*********************************************************
; REGISTER ASSIGNMENTS
; LOCAL VISIBILITY   ;only the translation unit has (should have)
;                     access to any NAME of the register
; LOCAL DURATION     ;the register has different meanings in different contexts
;*********************************************************
   .DEF  Implicit=            r0       ; Local.    Used for implicit lpm
   .DEF  Sreg_stack=          r1       ; Local.    "stack" for SREG during interrupts

;  .DEF  ZL=                  r30      ; Local.    Used for Z pointer (low byte)
;  .DEF  ZH=                  r31      ; Local.    Used for Z pointer (high byte)

.MACRO   LOWLOCAL1
   .DEF @0 =                  r7       ; Local.    General Use 
.ENDMACRO

.MACRO   LOWLOCAL2
   .DEF @0 =                  r8       ; Local.    General Use
.ENDMACRO

.MACRO   HILOCAL1
   .DEF  @0 =                 r26      ; Local.    General Use
.ENDMACRO

.MACRO   HILOCAL2 
   .DEF  @0 =                 r27      ; Local.    General Use
.ENDMACRO

;*********************************************************
; REGISTER ASSIGNMENTS
; GLOBAL VISIBILITY  ;all functions have access to this NAME of the register 
; LOCAL DURATION     ;the register has different meanings in different contexts
;*********************************************************
.MACRO   B_TEMPLOCAL
   .DEF @0 =                  r23      ; Local.    Second Level Routine Safe
.ENDMACRO

.MACRO   B_TEMPLOCAL1
   .DEF @0 =                  r24      ; Local.    Second Level Routine Safe
.ENDMACRO

.MACRO   B_TEMPLOCAL2
   .DEF @0 =                  r25      ; Local.    Second Level Routine Safe
.ENDMACRO

   B_TEMPLOCAL    B_Temp               ; General use names
   B_TEMPLOCAL1   B_Temp1
   B_TEMPLOCAL2   b_Temp2

;ISR LOCALS

; UNUSED

;.DEF Not_used=               r29

; GLOBALS


;FLAGS_1 flag data

;.SET F_Flags_1=              0b11111111  ; All flags

.SET  F_accel=                0b00000001  ; Accelerating if 1, decel if 0
.SET  BF_accel=               0x00        ; Flag bit location
.SET  F_brake=                0b00000010  ; Brake set if 1
.SET  BF_brake=               0x01        ; 
.SET  F_stop=                 0b00000100
.SET  BF_stop=                0x02
.SET  F_reverse=              0b00001000
.SET  BF_reverse=             0x03
.SET  F_negative_err=         0b00010000
.SET  BF_negative_err=        0x04
.SET  F_use_backemf=          0b00100000
.SET  BF_use_backemf=         0x05
;.SET F_=                     0b01000000
;.SET BF_=                    0x06
;.SET F_=                     0b10000000
;.SET BF_=                    0x07

.ifdef SWITCH_LOWPASS_ENABLED
;FLAGS_2 flag data  - count and status for forward/reverse/brake/stop

.SET F_Flags_2=               0b00001111  ; All flags

.SET  F_stop_count=           0b00010000
.SET  BF_stop_count=          0x04
.SET  F_brake_count=          0b00100000
.SET  BF_brake_count=         0x05
.SET  F_reverse_count=        0b01000000
.SET  BF_reverse_count=       0x06
.SET  F_foreward_count=       0b10000000
.SET  BF_foreward_count=      0x07

;bits 0-3  the count- from 0-16

.endif SWITCH_LOWPASS_ENABLED

;PORT SETUP STUFF
.SET  emf_port=            0x05        ;PB5 -   INPUT --    Back EMF input
.SET  emf_port_bit=        0b00100000  ;PIN1    ADC0

.SET  momentum_port=       0x04        ;PB4 -   INPUT --    Momentum Level input    
.SET  momentum_port_bit=   0b00010000  ;PIN2    ADC3        Acceleration-indicator output

.SET  dir_in_port=         0x03        ;PB3 -   INPUT --    Direction/Brake   
.SET  dir_in_port_bit=     0b00001000  ;PIN3    ADC2        Deceleration-indicator output 

.SET  throttle_port=       0x02        ;PB2 -   INPUT --    Throttle Handle   MAY USE INT0
.SET  throttle_port_bit=   0b00000100  ;PIN7    ADC1        (Could also use INTO
                                       ;        INT0         as DCC input.)

.SET  pwm_port=            0x01        ;PB1 -   OUTPUT --   PWM (off L)
.SET  pwm_port_bit=        0b00000010  ;PIN6    NO ADC
                                       ;        ONLY PWM PIN.

.SET  dir_out_port=        0x00        ;PB0 -   OUTPUT --   '1' = forward
.SET  dir_out_port_bit=    0b00000001  ;PIN5    NO ADC      '0' = reverse




.SET  acsr_val=      0b10000000        ; bit 7 --AC0  1     --comparator   disabled

.SET  eecr_read_enable= 0b00000001     ; bit 4,5,6,7        --unused
                                       ; bit 3 --EERIE   0  --interrupt       disabled
                                       ; bit 2 --EEMWE   0  --master write    disabled
                                       ; bit 1 --EEWE    0  --write           disabled
                                       ; bit 0 --EERE    1  --read            ENABLED
                  
.SET  eecr_mwrite_enable=  0b00000100  ; bit 4,5,6,7        --unused
                                       ; bit 3 --EERIE   0  --interrupt       disabled
                                       ; bit 2 --EEMWE   1  --master write    ENABLED
                                       ; bit 1 --EEWE    0  --write           disabled
                                       ; bit 0 --EERE    0  --read            disabled
                  
.SET  eecr_write_enable=   0b00000110  ; bit 4,5,6,7        --unused
                                       ; bit 3 --EERIE   0  --interrupt       disabled
                                       ; bit 2 --EEMWE   1  --master write    ENABLED
                                       ; bit 1 --EEWE    1  --write           ENABLED
                                       ; bit 0 --EERE    0  --read            disabled

.SET  admux_emf=           0b00100000  ; bit 4,3            --unused
                                       ; bit 7  --REFS1  0  Two bits:
                                       ; bit 6  --REFS0  0  Select VCC volt reference
                                       ; bit 5  --ADLAR  1  left adjust, 8 bit data in ADCH
                                       ; bit 2  --MUX2   0  Three bits:
                                       ; bit 1  --MUX1   0  
                                       ; bit 0  --MUX0   0  Select ADC0  from PB5

.SET  admux_throttle=      0b00100001  ; bit 4,3            --unused
                                       ; bit 7  --REFS1  0  Two bits:
                                       ; bit 6  --REFS0  0  Select VCC volt reference
                                       ; bit 5  --ADLAR  1  left adjust, 8 bit data in ADCH
                                       ; bit 2  --MUX2   0  Three bits:
                                       ; bit 1  --MUX1   0  
                                       ; bit 0  --MUX0   1  Select ADC1  from PB2

.SET  admux_direction=     0b00100010  ; bit 4,3            --unused
                                       ; bit 7  --REFS1  0  Two bits:
                                       ; bit 6  --REFS0  0  Select VCC volt reference
                                       ; bit 5  --ADLAR  1  left adjust, 8 bit data in ADCH
                                       ; bit 2  --MUX2   0  Three bits:
                                       ; bit 1  --MUX1   1  
                                       ; bit 0  --MUX0   0  Select ADC2 from PB3

.SET  admux_momentum=      0b00100011  ; bit 4,3            --unused
                                       ; bit 7  --REFS1  0  Two bits:
                                       ; bit 6  --REFS0  0  Select VCC volt reference
                                       ; bit 5  --ADLAR  1  left adjust, 8 bit data in ADCH
                                       ; bit 2  --MUX2   0  Three bits:
                                       ; bit 1  --MUX1   1  
                                       ; bit 0  --MUX0   1  Select ADC3 from PB4

.SET  admux_off=     0b00000000        ; bit 4,3            --unused
                                       ; bit 7  --REFS1  0  Two bits:
                                       ; bit 6  --REFS0  0  Select VCC volt reference
                                       ; bit 5  --ADLAR  0  right adjust
                                       ; bit 2  --MUX2   0  Three bits:
                                       ; bit 1  --MUX1   0  
                                       ; bit 0  --MUX0   0  Select ADC0 from PB5

.SET  adcsr_off=     0b00010011        ; bit 7  --ADCEN  0  ADC enable        disable
                                       ; bit 6  --ADSC   0  Start conversion  no
                                       ; bit 5  --ADFR   0  Free running mode disable
                                       ; bit 4  --ADIF   1  Interrupt flag    clear
                                       ; bit 3  --ADIE   0  Interrupt enable  disabled
                                       ; bit 2  --ADPS2  0  Three bits:
                                       ; bit 1  --ADPS1  1  
                                       ; bit 0  --ADPS0  1  Prescaler div 8   (200kHz)

.SET  adcsr_enable=  0b11110011        ; bit 7  --ADCEN  1  ADC enable        ENABLED
                                       ; bit 6  --ADSC   1  Start conversion  yes
                                       ; bit 5  --ADFR   1  Free running mode ENABLED
                                       ; bit 4  --ADIF   1  Interrupt flag    clear
                                       ; bit 3  --ADIE   0  Interrupt enable  disabled
                                       ; bit 2  --ADPS2  0  Three bits:
                                       ; bit 1  --ADPS1  1
                                       ; bit 0  --ADPS0  1  Prescaler div 8   (200kHz)

.SET  mcucr_power_down_mode=  0b00110000
                                       ; bit 7              --unused
                                       ; bit 6  --PUD    0  pullups           ENABLED
                                       ; bit 5  --SE     1  sleep             ENABLED
                                       ; bit 4  --SM1    1  Two bits:
                                       ; bit 3  --SM0    0  Sleep in power down mode
                                       ; bit 2              --unused
                                       ; bit 1  --ISC01  0  Two bits:
                                       ; bit 0  --ISC00  0  INTO interrupt on logic
                                       ;                    low input


.SET  wdtcr_off_enable= 0b00011000     ; enable watchdog turn off
.SET  wdtcr_off=        0b00010000     ; watchdog turn off

.SET  gimsk_val_off=    0b00000000     ; bit 7,4,3,2,1,0    --unused
                                       ; bit 6 --INTO    0  INTO interrupt    disabled
                                       ; bit 5 --PCIE    0  pin change interrupt disabled

.SET  gimsk_val_pcie=   0b00100000     ; bit 7,4,3,2,1,0    --unused
                                       ; bit 6  --INTO   0  INTO interrupt    disabled
                                       ; bit 5  --PCIE   1  pin change interrupt ENABLED
                  
.SET  gifr_clear=       0b01100000     ; bit 7,4,3,2,1,0    --unused
                                       ; bit 6  --INTF0  1  INTO interrupt flag  cleared
                                       ; bit 5  --PCIF   1  pin ch. interrupt flag  cleared
                  
.SET  timsk_val_off=    0b00000000     ; bit 7,5,4,3,0      --unused
                                       ; bit 6  --OCIE1A 0  TC1 compare interrupt   disabled
                                       ; bit 2  --TOIE1  0  TC1 overflow interrupt  disabled
                                       ; bit 1  --TOIE0  0  TC0 overflow interrupt  disabled

.SET  timsk_enable_t1=  0b01000000     ; bit 7,5,4,3,0      --unused
                                       ; bit 6  --OCIE1A 0  TC1 compare interrupt   disabled
                                       ; bit 2  --TOIE1  0  TC1 overflow interrupt  disabled
                                       ; bit 1  --TOIE0  0  TC0 overflow interrupt  disabled

.SET  tifr_clear_tov0=  0b00000010     ; bit 7,5,4,3,0      --unused
                                       ; bit 6  --OCF1A  0  OCF1A flag  not cleared
                                       ; bit 2  --TOV1   0  TOV1 flag   not cleared
                                       ; bit 1  --TOV0   1  TOV0 flag   CLEARED

.SET  tccr1_enable_t1=  0b01100011
                                       ; bit 7  --CTC1   0  Clear on compare match  no
                                       ; bit 6  --PWM1   1  PWM mode                ENABLED
                                       ; bit 5  --COM1A1 1  Two bits:
                                       ; bit 4  --COM1A0 0  Clear output on OCR1A match
                                       ;                    Set output when TCNT1 = 0
                                       ; bit 3  --CS13   0  Four bits:
                                       ; bit 2  --CS12   0
                                       ; bit 3  --CS11   1
                                       ; bit 4  --CS10   1  CK*4: 156.25nS
                                       ;                    256cnt = 25kHz
.LIST
