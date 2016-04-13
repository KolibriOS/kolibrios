;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:31 ******* Source: ATtiny45.xml ************
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "tn45def.inc"
;* Title             : Register/Bit Definitions for the ATtiny45
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATtiny45
;* 
;* DESCRIPTION
;* When including this file in the assembly program file, all I/O register 
;* names and I/O register bit names appearing in the data book can be used.
;* In addition, the six registers forming the three data pointers X, Y and 
;* Z have been assigned names XL - ZH. Highest RAM address for Internal 
;* SRAM is also defined 
;* 
;* The Register names are represented by their hexadecimal address.
;* 
;* The Register Bit names are represented by their bit number (0-7).
;* 
;* Please observe the difference in using the bit names with instructions
;* such as "sbr"/"cbr" (set/clear bit in register) and "sbrs"/"sbrc"
;* (skip if bit in register set/cleared). The following example illustrates
;* this:
;* 
;* in    r16,PORTB             ;read PORTB latch
;* sbr   r16,(1<<PB6)+(1<<PB5) ;set PB6 and PB5 (use masks, not bit#)
;* out   PORTB,r16             ;output to PORTB
;* 
;* in    r16,TIFR              ;read the Timer Interrupt Flag Register
;* sbrc  r16,TOV0              ;test the overflow flag (use bit#)
;* rjmp  TOV0_is_set           ;jump if set
;* ...                         ;otherwise do something else
;*************************************************************************

#ifndef _TN45DEF_INC_
#define _TN45DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATtiny45
#pragma AVRPART ADMIN PART_NAME ATtiny45
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x92
.equ	SIGNATURE_002	= 0x06

#pragma AVRPART CORE CORE_VERSION V2
#pragma AVRPART CORE NEW_INSTRUCTIONS lpm rd,z+


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	SREG	= 0x3f
.equ	SPH	= 0x3e
.equ	SPL	= 0x3d
.equ	GIMSK	= 0x3b
.equ	GIFR	= 0x3a
.equ	TIMSK	= 0x39
.equ	TIFR	= 0x38
.equ	SPMCSR	= 0x37
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	TCCR0B	= 0x33
.equ	TCNT0	= 0x32
.equ	OSCCAL	= 0x31
.equ	TCCR1	= 0x30
.equ	TCNT1	= 0x2f
.equ	OCR1A	= 0x2e
.equ	OCR1C	= 0x2d
.equ	GTCCR	= 0x2c
.equ	OCR1B	= 0x2b
.equ	TCCR0A	= 0x2a
.equ	OCR0A	= 0x29
.equ	OCR0B	= 0x28
.equ	PLLCSR	= 0x27
.equ	CLKPR	= 0x26
.equ	DTVALA	= 0x25
.equ	DTVALB	= 0x24
.equ	DTPS	= 0x23
.equ	DWDR	= 0x22
.equ	WDTCR	= 0x21
.equ	PRR	= 0x20
.equ	EEARH	= 0x1f
.equ	EEARL	= 0x1e
.equ	EEDR	= 0x1d
.equ	EECR	= 0x1c
.equ	PORTB	= 0x18
.equ	DDRB	= 0x17
.equ	PINB	= 0x16
.equ	PCMSK	= 0x15
.equ	DIDR0	= 0x14
.equ	GPIOR2	= 0x13
.equ	GPIOR1	= 0x12
.equ	GPIOR0	= 0x11
.equ	USIBR	= 0x10
.equ	USIDR	= 0x0f
.equ	USISR	= 0x0e
.equ	USICR	= 0x0d
.equ	ACSR	= 0x08
.equ	ADMUX	= 0x07
.equ	ADCSRA	= 0x06
.equ	ADCH	= 0x05
.equ	ADCL	= 0x04
.equ	ADCSRB	= 0x03


; ***** BIT DEFINITIONS **************************************************

; ***** PORTB ************************
; PORTB - Data Register, Port B
.equ	PORTB0	= 0	; 
.equ	PB0	= 0	; For compatibility
.equ	PORTB1	= 1	; 
.equ	PB1	= 1	; For compatibility
.equ	PORTB2	= 2	; 
.equ	PB2	= 2	; For compatibility
.equ	PORTB3	= 3	; 
.equ	PB3	= 3	; For compatibility
.equ	PORTB4	= 4	; 
.equ	PB4	= 4	; For compatibility
.equ	PORTB5	= 5	; 
.equ	PB5	= 5	; For compatibility

; DDRB - Data Direction Register, Port B
.equ	DDB0	= 0	; 
.equ	DDB1	= 1	; 
.equ	DDB2	= 2	; 
.equ	DDB3	= 3	; 
.equ	DDB4	= 4	; 
.equ	DDB5	= 5	; 

; PINB - Input Pins, Port B
.equ	PINB0	= 0	; 
.equ	PINB1	= 1	; 
.equ	PINB2	= 2	; 
.equ	PINB3	= 3	; 
.equ	PINB4	= 4	; 
.equ	PINB5	= 5	; 


; ***** ANALOG_COMPARATOR ************
; ADCSRB - ADC Control and Status Register B
.equ	ACME	= 6	; Analog Comparator Multiplexer Enable

; ACSR - Analog Comparator Control And Status Register
.equ	ACIS0	= 0	; Analog Comparator Interrupt Mode Select bit 0
.equ	ACIS1	= 1	; Analog Comparator Interrupt Mode Select bit 1
.equ	ACIE	= 3	; Analog Comparator Interrupt Enable
.equ	ACI	= 4	; Analog Comparator Interrupt Flag
.equ	ACO	= 5	; Analog Compare Output
.equ	ACBG	= 6	; Analog Comparator Bandgap Select
.equ	AINBG	= ACBG	; For compatibility
.equ	ACD	= 7	; Analog Comparator Disable

; DIDR0 - 
.equ	AIN0D	= 0	; AIN0 Digital Input Disable
.equ	AIN1D	= 1	; AIN1 Digital Input Disable


; ***** AD_CONVERTER *****************
; ADMUX - The ADC multiplexer Selection Register
.equ	MUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	MUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	MUX2	= 2	; Analog Channel and Gain Selection Bits
.equ	MUX3	= 3	; Analog Channel and Gain Selection Bits
.equ	REFS2	= 4	; Reference Selection Bit 2
.equ	ADLAR	= 5	; Left Adjust Result
.equ	REFS0	= 6	; Reference Selection Bit 0
.equ	REFS1	= 7	; Reference Selection Bit 1

; ADCSRA - The ADC Control and Status register
.equ	ADPS0	= 0	; ADC  Prescaler Select Bits
.equ	ADPS1	= 1	; ADC  Prescaler Select Bits
.equ	ADPS2	= 2	; ADC  Prescaler Select Bits
.equ	ADIE	= 3	; ADC Interrupt Enable
.equ	ADIF	= 4	; ADC Interrupt Flag
.equ	ADATE	= 5	; ADC Auto Trigger Enable
.equ	ADSC	= 6	; ADC Start Conversion
.equ	ADEN	= 7	; ADC Enable

; ADCH - ADC Data Register High Byte
.equ	ADCH0	= 0	; ADC Data Register High Byte Bit 0
.equ	ADCH1	= 1	; ADC Data Register High Byte Bit 1
.equ	ADCH2	= 2	; ADC Data Register High Byte Bit 2
.equ	ADCH3	= 3	; ADC Data Register High Byte Bit 3
.equ	ADCH4	= 4	; ADC Data Register High Byte Bit 4
.equ	ADCH5	= 5	; ADC Data Register High Byte Bit 5
.equ	ADCH6	= 6	; ADC Data Register High Byte Bit 6
.equ	ADCH7	= 7	; ADC Data Register High Byte Bit 7

; ADCL - ADC Data Register Low Byte
.equ	ADCL0	= 0	; ADC Data Register Low Byte Bit 0
.equ	ADCL1	= 1	; ADC Data Register Low Byte Bit 1
.equ	ADCL2	= 2	; ADC Data Register Low Byte Bit 2
.equ	ADCL3	= 3	; ADC Data Register Low Byte Bit 3
.equ	ADCL4	= 4	; ADC Data Register Low Byte Bit 4
.equ	ADCL5	= 5	; ADC Data Register Low Byte Bit 5
.equ	ADCL6	= 6	; ADC Data Register Low Byte Bit 6
.equ	ADCL7	= 7	; ADC Data Register Low Byte Bit 7

; ADCSRB - ADC Control and Status Register B
.equ	ADTS0	= 0	; ADC Auto Trigger Source 0
.equ	ADTS1	= 1	; ADC Auto Trigger Source 1
.equ	ADTS2	= 2	; ADC Auto Trigger Source 2
.equ	IPR	= 5	; Input Polarity Mode
.equ	BIN	= 7	; Bipolar Input Mode

; DIDR0 - Digital Input Disable Register 0
.equ	ADC1D	= 2	; ADC1 Digital input Disable
.equ	ADC3D	= 3	; ADC3 Digital input Disable
.equ	ADC2D	= 4	; ADC2 Digital input Disable
.equ	ADC0D	= 5	; ADC0 Digital input Disable


; ***** USI **************************
; USIBR - USI Buffer Register
.equ	USIBR0	= 0	; USI Buffer Register bit 0
.equ	USIBR1	= 1	; USI Buffer Register bit 1
.equ	USIBR2	= 2	; USI Buffer Register bit 2
.equ	USIBR3	= 3	; USI Buffer Register bit 3
.equ	USIBR4	= 4	; USI Buffer Register bit 4
.equ	USIBR5	= 5	; USI Buffer Register bit 5
.equ	USIBR6	= 6	; USI Buffer Register bit 6
.equ	USIBR7	= 7	; USI Buffer Register bit 7

; USIDR - USI Data Register
.equ	USIDR0	= 0	; USI Data Register bit 0
.equ	USIDR1	= 1	; USI Data Register bit 1
.equ	USIDR2	= 2	; USI Data Register bit 2
.equ	USIDR3	= 3	; USI Data Register bit 3
.equ	USIDR4	= 4	; USI Data Register bit 4
.equ	USIDR5	= 5	; USI Data Register bit 5
.equ	USIDR6	= 6	; USI Data Register bit 6
.equ	USIDR7	= 7	; USI Data Register bit 7

; USISR - USI Status Register
.equ	USICNT0	= 0	; USI Counter Value Bit 0
.equ	USICNT1	= 1	; USI Counter Value Bit 1
.equ	USICNT2	= 2	; USI Counter Value Bit 2
.equ	USICNT3	= 3	; USI Counter Value Bit 3
.equ	USIDC	= 4	; Data Output Collision
.equ	USIPF	= 5	; Stop Condition Flag
.equ	USIOIF	= 6	; Counter Overflow Interrupt Flag
.equ	USISIF	= 7	; Start Condition Interrupt Flag

; USICR - USI Control Register
.equ	USITC	= 0	; Toggle Clock Port Pin
.equ	USICLK	= 1	; Clock Strobe
.equ	USICS0	= 2	; USI Clock Source Select Bit 0
.equ	USICS1	= 3	; USI Clock Source Select Bit 1
.equ	USIWM0	= 4	; USI Wire Mode Bit 0
.equ	USIWM1	= 5	; USI Wire Mode Bit 1
.equ	USIOIE	= 6	; Counter Overflow Interrupt Enable
.equ	USISIE	= 7	; Start Condition Interrupt Enable


; ***** EXTERNAL_INTERRUPT ***********
; MCUCR - MCU Control Register
.equ	ISC00	= 0	; Interrupt Sense Control 0 Bit 0
.equ	ISC01	= 1	; Interrupt Sense Control 0 Bit 1

; GIMSK - General Interrupt Mask Register
.equ	GICR	= GIMSK	; For compatibility
.equ	PCIE	= 5	; Pin Change Interrupt Enable
.equ	INT0	= 6	; External Interrupt Request 0 Enable

; GIFR - General Interrupt Flag register
.equ	PCIF	= 5	; Pin Change Interrupt Flag
.equ	INTF0	= 6	; External Interrupt Flag 0

; PCMSK - Pin Change Enable Mask
.equ	PCINT0	= 0	; Pin Change Enable Mask Bit 0
.equ	PCINT1	= 1	; Pin Change Enable Mask Bit 1
.equ	PCINT2	= 2	; Pin Change Enable Mask Bit 2
.equ	PCINT3	= 3	; Pin Change Enable Mask Bit 3
.equ	PCINT4	= 4	; Pin Change Enable Mask Bit 4
.equ	PCINT5	= 5	; Pin Change Enable Mask Bit 5


; ***** EEPROM ***********************
; EEARL - EEPROM Address Register Low Byte
.equ	EEAR0	= 0	; EEPROM Read/Write Access Bit 0
.equ	EEAR1	= 1	; EEPROM Read/Write Access Bit 1
.equ	EEAR2	= 2	; EEPROM Read/Write Access Bit 2
.equ	EEAR3	= 3	; EEPROM Read/Write Access Bit 3
.equ	EEAR4	= 4	; EEPROM Read/Write Access Bit 4
.equ	EEAR5	= 5	; EEPROM Read/Write Access Bit 5
.equ	EEAR6	= 6	; EEPROM Read/Write Access Bit 6
.equ	EEAR7	= 7	; EEPROM Read/Write Access Bit 7

; EEARH - EEPROM Address Register High Byte
.equ	EEAR8	= 0	; EEPROM Read/Write Access Bit 0

; EEDR - EEPROM Data Register
.equ	EEDR0	= 0	; EEPROM Data Register bit 0
.equ	EEDR1	= 1	; EEPROM Data Register bit 1
.equ	EEDR2	= 2	; EEPROM Data Register bit 2
.equ	EEDR3	= 3	; EEPROM Data Register bit 3
.equ	EEDR4	= 4	; EEPROM Data Register bit 4
.equ	EEDR5	= 5	; EEPROM Data Register bit 5
.equ	EEDR6	= 6	; EEPROM Data Register bit 6
.equ	EEDR7	= 7	; EEPROM Data Register bit 7

; EECR - EEPROM Control Register
.equ	EERE	= 0	; EEPROM Read Enable
.equ	EEPE	= 1	; EEPROM Write Enable
.equ	EEMPE	= 2	; EEPROM Master Write Enable
.equ	EERIE	= 3	; EEPROM Ready Interrupt Enable
.equ	EEPM0	= 4	; EEPROM Programming Mode Bit 0
.equ	EEPM1	= 5	; EEPROM Programming Mode Bit 1


; ***** WATCHDOG *********************
; WDTCR - Watchdog Timer Control Register
.equ	WDTCSR	= WDTCR	; For compatibility
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDCE	= 4	; Watchdog Change Enable
.equ	WDTOE	= WDCE	; For compatibility
.equ	WDP3	= 5	; Watchdog Timer Prescaler Bit 3
.equ	WDIE	= 6	; Watchdog Timeout Interrupt Enable
.equ	WDIF	= 7	; Watchdog Timeout Interrupt Flag


; ***** TIMER_COUNTER_0 **************
; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE0	= 1	; Timer/Counter0 Overflow Interrupt Enable
.equ	OCIE0B	= 3	; Timer/Counter0 Output Compare Match B Interrupt Enable
.equ	OCIE0A	= 4	; Timer/Counter0 Output Compare Match A Interrupt Enable

; TIFR - Timer/Counter0 Interrupt Flag register
.equ	TOV0	= 1	; Timer/Counter0 Overflow Flag
.equ	OCF0B	= 3	; Timer/Counter0 Output Compare Flag 0B
.equ	OCF0A	= 4	; Timer/Counter0 Output Compare Flag 0A

; TCCR0A - Timer/Counter  Control Register A
.equ	WGM00	= 0	; Waveform Generation Mode
.equ	WGM01	= 1	; Waveform Generation Mode
.equ	COM0B0	= 4	; Compare Output Mode, Fast PWm
.equ	COM0B1	= 5	; Compare Output Mode, Fast PWm
.equ	COM0A0	= 6	; Compare Output Mode, Phase Correct PWM Mode
.equ	COM0A1	= 7	; Compare Output Mode, Phase Correct PWM Mode

; TCCR0B - Timer/Counter Control Register B
.equ	CS00	= 0	; Clock Select
.equ	CS01	= 1	; Clock Select
.equ	CS02	= 2	; Clock Select
.equ	WGM02	= 3	; 
.equ	FOC0B	= 6	; Force Output Compare B
.equ	FOC0A	= 7	; Force Output Compare A

; TCNT0 - Timer/Counter0
.equ	TCNT0_0	= 0	; 
.equ	TCNT0_1	= 1	; 
.equ	TCNT0_2	= 2	; 
.equ	TCNT0_3	= 3	; 
.equ	TCNT0_4	= 4	; 
.equ	TCNT0_5	= 5	; 
.equ	TCNT0_6	= 6	; 
.equ	TCNT0_7	= 7	; 

; OCR0A - Timer/Counter0 Output Compare Register
.equ	OCR0_0	= 0	; 
.equ	OCR0_1	= 1	; 
.equ	OCR0_2	= 2	; 
.equ	OCR0_3	= 3	; 
.equ	OCR0_4	= 4	; 
.equ	OCR0_5	= 5	; 
.equ	OCR0_6	= 6	; 
.equ	OCR0_7	= 7	; 

; OCR0B - Timer/Counter0 Output Compare Register
;.equ	OCR0_0	= 0	; 
;.equ	OCR0_1	= 1	; 
;.equ	OCR0_2	= 2	; 
;.equ	OCR0_3	= 3	; 
;.equ	OCR0_4	= 4	; 
;.equ	OCR0_5	= 5	; 
;.equ	OCR0_6	= 6	; 
;.equ	OCR0_7	= 7	; 

; GTCCR - General Timer/Counter Control Register
.equ	PSR0	= 0	; Prescaler Reset Timer/Counter1 and Timer/Counter0
.equ	TSM	= 7	; Timer/Counter Synchronization Mode


; ***** TIMER_COUNTER_1 **************
; TCCR1 - Timer/Counter Control Register
.equ	CS10	= 0	; Clock Select Bits
.equ	CS11	= 1	; Clock Select Bits
.equ	CS12	= 2	; Clock Select Bits
.equ	CS13	= 3	; Clock Select Bits
.equ	COM1A0	= 4	; Compare Output Mode, Bit 1
.equ	COM1A1	= 5	; Compare Output Mode, Bit 0
.equ	PWM1A	= 6	; Pulse Width Modulator Enable
.equ	CTC1	= 7	; Clear Timer/Counter on Compare Match

; TCNT1 - Timer/Counter Register
.equ	TCNT1_0	= 0	; Timer/Counter Register Bit 0
.equ	TCNT1_1	= 1	; Timer/Counter Register Bit 1
.equ	TCNT1_2	= 2	; Timer/Counter Register Bit 2
.equ	TCNT1_3	= 3	; Timer/Counter Register Bit 3
.equ	TCNT1_4	= 4	; Timer/Counter Register Bit 4
.equ	TCNT1_5	= 5	; Timer/Counter Register Bit 5
.equ	TCNT1_6	= 6	; Timer/Counter Register Bit 6
.equ	TCNT1_7	= 7	; Timer/Counter Register Bit 7

; OCR1A - Output Compare Register
.equ	OCR1A0	= 0	; Output Compare Register A Bit 0
.equ	OCR1A1	= 1	; Output Compare Register A Bit 1
.equ	OCR1A2	= 2	; Output Compare Register A Bit 2
.equ	OCR1A3	= 3	; Output Compare Register A Bit 3
.equ	OCR1A4	= 4	; Output Compare Register A Bit 4
.equ	OCR1A5	= 5	; Output Compare Register A Bit 5
.equ	OCR1A6	= 6	; Output Compare Register A Bit 6
.equ	OCR1A7	= 7	; Output Compare Register A Bit 7

; OCR1B - Output Compare Register
.equ	OCR1B0	= 0	; Output Compare Register B Bit 0
.equ	OCR1B1	= 1	; Output Compare Register B Bit 1
.equ	OCR1B2	= 2	; Output Compare Register B Bit 2
.equ	OCR1B3	= 3	; Output Compare Register B Bit 3
.equ	OCR1B4	= 4	; Output Compare Register B Bit 4
.equ	OCR1B5	= 5	; Output Compare Register B Bit 5
.equ	OCR1B6	= 6	; Output Compare Register B Bit 6
.equ	OCR1B7	= 7	; Output Compare Register B Bit 7

; OCR1C - Output compare register
.equ	OCR1C0	= 0	; 
.equ	OCR1C1	= 1	; 
.equ	OCR1C2	= 2	; 
.equ	OCR1C3	= 3	; 
.equ	OCR1C4	= 4	; 
.equ	OCR1C5	= 5	; 
.equ	OCR1C6	= 6	; 
.equ	OCR1C7	= 7	; 

; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE1	= 2	; Timer/Counter1 Overflow Interrupt Enable
.equ	OCIE1B	= 5	; OCIE1A: Timer/Counter1 Output Compare B Interrupt Enable
.equ	OCE1A	= 6	; OCIE1A: Timer/Counter1 Output Compare Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag Register
.equ	TOV1	= 2	; Timer/Counter1 Overflow Flag
.equ	OCF1B	= 5	; Timer/Counter1 Output Compare Flag 1B
.equ	OCF1A	= 6	; Timer/Counter1 Output Compare Flag 1A

; GTCCR - Timer counter control register
.equ	PSR1	= 1	; Prescaler Reset Timer/Counter1
.equ	FOC1A	= 2	; Force Output Compare 1A
.equ	FOC1B	= 3	; Force Output Compare Match 1B
.equ	COM1B0	= 4	; Comparator B Output Mode
.equ	COM1B1	= 5	; Comparator B Output Mode
.equ	PWM1B	= 6	; Pulse Width Modulator B Enable

; DTPS - Dead time prescaler register
.equ	DTPS0	= 0	; 
.equ	DTPS1	= 1	; 

; DTVALA - Dead time value register
.equ	DTVL0	= 0	; 
.equ	DTVL1	= 1	; 
.equ	DTVL2	= 2	; 
.equ	DTVL3	= 3	; 
.equ	DTVH0	= 4	; 
.equ	DTVH1	= 5	; 
.equ	DTVH2	= 6	; 
.equ	DTVH3	= 7	; 

; DTVALB - Dead time value B
;.equ	DTVL0	= 0	; 
;.equ	DTVL1	= 1	; 
;.equ	DTVL2	= 2	; 
;.equ	DTVL3	= 3	; 
;.equ	DTVH0	= 4	; 
;.equ	DTVH1	= 5	; 
;.equ	DTVH2	= 6	; 
;.equ	DTVH3	= 7	; 


; ***** CPU **************************
; SREG - Status Register
.equ	SREG_C	= 0	; Carry Flag
.equ	SREG_Z	= 1	; Zero Flag
.equ	SREG_N	= 2	; Negative Flag
.equ	SREG_V	= 3	; Two's Complement Overflow Flag
.equ	SREG_S	= 4	; Sign Bit
.equ	SREG_H	= 5	; Half Carry Flag
.equ	SREG_T	= 6	; Bit Copy Storage
.equ	SREG_I	= 7	; Global Interrupt Enable

; MCUCR - MCU Control Register
;.equ	ISC00	= 0	; Interrupt Sense Control 0 bit 0
;.equ	ISC01	= 1	; Interrupt Sense Control 0 bit 1
.equ	SM0	= 3	; Sleep Mode Select Bit 0
.equ	SM1	= 4	; Sleep Mode Select Bit 1
.equ	SE	= 5	; Sleep Enable
.equ	PUD	= 6	; Pull-up Disable

; MCUSR - MCU Status register
.equ	PORF	= 0	; Power-On Reset Flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BORF	= 2	; Brown-out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag

; PRR - Power Reduction Register
.equ	PRADC	= 0	; Power Reduction ADC
.equ	PRUSI	= 1	; Power Reduction USI
.equ	PRTIM0	= 2	; Power Reduction Timer/Counter0
.equ	PRTIM1	= 3	; Power Reduction Timer/Counter1

; OSCCAL - Oscillator Calibration Register
.equ	CAL0	= 0	; Oscillatro Calibration Value Bit 0
.equ	CAL1	= 1	; Oscillatro Calibration Value Bit 1
.equ	CAL2	= 2	; Oscillatro Calibration Value Bit 2
.equ	CAL3	= 3	; Oscillatro Calibration Value Bit 3
.equ	CAL4	= 4	; Oscillatro Calibration Value Bit 4
.equ	CAL5	= 5	; Oscillatro Calibration Value Bit 5
.equ	CAL6	= 6	; Oscillatro Calibration Value Bit 6

; PLLCSR - PLL Control and status register
.equ	PLOCK	= 0	; PLL Lock detector
.equ	PLLE	= 1	; PLL Enable
.equ	PCKE	= 2	; PCK Enable
.equ	LSM	= 7	; Low speed mode

; CLKPR - Clock Prescale Register
.equ	CLKPS0	= 0	; Clock Prescaler Select Bit 0
.equ	CLKPS1	= 1	; Clock Prescaler Select Bit 1
.equ	CLKPS2	= 2	; Clock Prescaler Select Bit 2
.equ	CLKPS3	= 3	; Clock Prescaler Select Bit 3
.equ	CLKPCE	= 7	; Clock Prescaler Change Enable

; DWDR - debugWire data register
.equ	DWDR0	= 0	; 
.equ	DWDR1	= 1	; 
.equ	DWDR2	= 2	; 
.equ	DWDR3	= 3	; 
.equ	DWDR4	= 4	; 
.equ	DWDR5	= 5	; 
.equ	DWDR6	= 6	; 
.equ	DWDR7	= 7	; 

; GPIOR2 - General Purpose IO register 2
.equ	GPIOR20	= 0	; 
.equ	GPIOR21	= 1	; 
.equ	GPIOR22	= 2	; 
.equ	GPIOR23	= 3	; 
.equ	GPIOR24	= 4	; 
.equ	GPIOR25	= 5	; 
.equ	GPIOR26	= 6	; 
.equ	GPIOR27	= 7	; 

; GPIOR1 - General Purpose register 1
.equ	GPIOR10	= 0	; 
.equ	GPIOR11	= 1	; 
.equ	GPIOR12	= 2	; 
.equ	GPIOR13	= 3	; 
.equ	GPIOR14	= 4	; 
.equ	GPIOR15	= 5	; 
.equ	GPIOR16	= 6	; 
.equ	GPIOR17	= 7	; 

; GPIOR0 - General purpose register 0
.equ	GPIOR00	= 0	; 
.equ	GPIOR01	= 1	; 
.equ	GPIOR02	= 2	; 
.equ	GPIOR03	= 3	; 
.equ	GPIOR04	= 4	; 
.equ	GPIOR05	= 5	; 
.equ	GPIOR06	= 6	; 
.equ	GPIOR07	= 7	; 


; ***** BOOT_LOAD ********************
; SPMCSR - Store Program Memory Control Register
.equ	SPMEN	= 0	; Store Program Memory Enable
.equ	PGERS	= 1	; Page Erase
.equ	PGWRT	= 2	; Page Write
.equ	RFLB	= 3	; Read fuse and lock bits
.equ	CTPB	= 4	; Clear temporary page buffer



; ***** LOCKSBITS ********************************************************
.equ	LB1	= 0	; Lockbit
.equ	LB2	= 1	; Lockbit


; ***** FUSES ************************************************************
; LOW fuse bits
.equ	CKSEL0	= 0	; Select Clock source
.equ	CKSEL1	= 1	; Select Clock source
.equ	CKSEL2	= 2	; Select Clock source
.equ	CKSEL3	= 3	; Select Clock source
.equ	SUT0	= 4	; Select start-up time
.equ	SUT1	= 5	; Select start-up time
.equ	CKOUT	= 6	; Clock Output Enable
.equ	CKDIV8	= 7	; Divide clock by 8

; HIGH fuse bits
.equ	BODLEVEL0	= 0	; Brown-out Detector trigger level
.equ	BODLEVEL1	= 1	; Brown-out Detector trigger level
.equ	BODLEVEL2	= 2	; Brown-out Detector trigger level
.equ	EESAVE	= 3	; EEPROM memory is preserved through the Chip Erase
.equ	WDTON	= 4	; Watchdog Timer always on
.equ	SPIEN	= 5	; Enable Serial Program and Data Downloading
.equ	DWEN	= 6	; DebugWIRE Enable
.equ	RSTDISBL	= 7	; External Reset disable

; EXTENDED fuse bits
.equ	SELFPRGEN	= 0	; Self-Programming Enable



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x07ff	; Note: Word address
.equ	IOEND	= 0x003f
.equ	SRAM_START	= 0x0060
.equ	SRAM_SIZE	= 256
.equ	RAMEND	= 0x015f
.equ	XRAMEND	= 0x0000
.equ	E2END	= 0x00ff
.equ	EEPROMEND	= 0x00ff
.equ	EEADRBITS	= 8
#pragma AVRPART MEMORY PROG_FLASH 4096
#pragma AVRPART MEMORY EEPROM 256
#pragma AVRPART MEMORY INT_SRAM SIZE 256
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60



; ***** BOOTLOADER DECLARATIONS ******************************************
.equ	NRWW_START_ADDR	= 0x0
.equ	NRWW_STOP_ADDR	= 0x7ff
.equ	RWW_START_ADDR	= 0x0
.equ	RWW_STOP_ADDR	= 0x0
.equ	PAGESIZE	= 32



; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0001	; External Interrupt 0
.equ	PCI0addr	= 0x0002	; Pin change Interrupt Request 0
.equ	OC1Aaddr	= 0x0003	; Timer/Counter1 Compare Match 1A
.equ	OVF1addr	= 0x0004	; Timer/Counter1 Overflow
.equ	OVF0addr	= 0x0005	; Timer/Counter0 Overflow
.equ	ERDYaddr	= 0x0006	; EEPROM Ready
.equ	ACIaddr	= 0x0007	; Analog comparator
.equ	ADCCaddr	= 0x0008	; ADC Conversion ready
.equ	OC1Baddr	= 0x0009	; Timer/Counter1 Compare Match B
.equ	OC0Aaddr	= 0x000a	; Timer/Counter0 Compare Match A
.equ	OC0Baddr	= 0x000b	; Timer/Counter0 Compare Match B
.equ	WDTaddr	= 0x000c	; Watchdog Time-out
.equ	USI_STARTaddr	= 0x000d	; USI START
.equ	USI_OVFaddr	= 0x000e	; USI Overflow

.equ	INT_VECTORS_SIZE	= 15	; size in words

#endif  /* _TN45DEF_INC_ */

; ***** END OF FILE ******************************************************
