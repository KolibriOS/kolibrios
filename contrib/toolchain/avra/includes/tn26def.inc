;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:31 ******* Source: ATtiny26.xml ************
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "tn26def.inc"
;* Title             : Register/Bit Definitions for the ATtiny26
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATtiny26
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

#ifndef _TN26DEF_INC_
#define _TN26DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATtiny26
#pragma AVRPART ADMIN PART_NAME ATtiny26
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x91
.equ	SIGNATURE_002	= 0x09

#pragma AVRPART CORE CORE_VERSION V1
#pragma AVRPART CORE NEW_INSTRUCTIONS lpm rd,z


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	SREG	= 0x3f
.equ	SP	= 0x3d
.equ	GIMSK	= 0x3b
.equ	GIFR	= 0x3a
.equ	TIMSK	= 0x39
.equ	TIFR	= 0x38
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	TCCR0	= 0x33
.equ	TCNT0	= 0x32
.equ	OSCCAL	= 0x31
.equ	TCCR1A	= 0x30
.equ	TCCR1B	= 0x2f
.equ	TCNT1	= 0x2e
.equ	OCR1A	= 0x2d
.equ	OCR1B	= 0x2c
.equ	OCR1C	= 0x2b
.equ	PLLCSR	= 0x29
.equ	WDTCR	= 0x21
.equ	EEAR	= 0x1e
.equ	EEDR	= 0x1d
.equ	EECR	= 0x1c
.equ	PORTA	= 0x1b
.equ	DDRA	= 0x1a
.equ	PINA	= 0x19
.equ	PORTB	= 0x18
.equ	DDRB	= 0x17
.equ	PINB	= 0x16
.equ	USIDR	= 0x0f
.equ	USISR	= 0x0e
.equ	USICR	= 0x0d
.equ	ACSR	= 0x08
.equ	ADMUX	= 0x07
.equ	ADCSRA	= 0x06
.equ	ADCH	= 0x05
.equ	ADCL	= 0x04


; ***** BIT DEFINITIONS **************************************************

; ***** AD_CONVERTER *****************
; ADMUX - The ADC multiplexer Selection Register
.equ	MUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	MUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	MUX2	= 2	; Analog Channel and Gain Selection Bits
.equ	MUX3	= 3	; Analog Channel and Gain Selection Bits
.equ	MUX4	= 4	; Analog Channel and Gain Selection Bits
.equ	ADLAR	= 5	; Left Adjust Result
.equ	REFS0	= 6	; Reference Selection Bit 0
.equ	REFS1	= 7	; Reference Selection Bit 1

; ADCSRA - The ADC Control and Status register
.equ	ADCSR	= ADCSRA	; For compatibility
.equ	ADPS0	= 0	; ADC  Prescaler Select Bits
.equ	ADPS1	= 1	; ADC  Prescaler Select Bits
.equ	ADPS2	= 2	; ADC  Prescaler Select Bits
.equ	ADIE	= 3	; ADC Interrupt Enable
.equ	ADIF	= 4	; ADC Interrupt Flag
.equ	ADFR	= 5	; ADC  Free Running Select
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


; ***** ANALOG_COMPARATOR ************
; ACSR - Analog Comparator Control And Status Register
.equ	ACIS0	= 0	; Analog Comparator Interrupt Mode Select bit 0
.equ	ACIS1	= 1	; Analog Comparator Interrupt Mode Select bit 1
.equ	ACME	= 2	; Analog Comparator Multiplexer Enable
.equ	ACIE	= 3	; Analog Comparator Interrupt Enable
.equ	ACI	= 4	; Analog Comparator Interrupt Flag
.equ	ACO	= 5	; Analog Compare Output
.equ	ACBG	= 6	; Analog Comparator Bandgap Select
.equ	ACD	= 7	; Analog Comparator Disable


; ***** USI **************************
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


; ***** PORTA ************************
; PORTA - Port A Data Register
.equ	PORTA0	= 0	; Port A Data Register bit 0
.equ	PA0	= 0	; For compatibility
.equ	PORTA1	= 1	; Port A Data Register bit 1
.equ	PA1	= 1	; For compatibility
.equ	PORTA2	= 2	; Port A Data Register bit 2
.equ	PA2	= 2	; For compatibility
.equ	PORTA3	= 3	; Port A Data Register bit 3
.equ	PA3	= 3	; For compatibility
.equ	PORTA4	= 4	; Port A Data Register bit 4
.equ	PA4	= 4	; For compatibility
.equ	PORTA5	= 5	; Port A Data Register bit 5
.equ	PA5	= 5	; For compatibility
.equ	PORTA6	= 6	; Port A Data Register bit 6
.equ	PA6	= 6	; For compatibility
.equ	PORTA7	= 7	; Port A Data Register bit 7
.equ	PA7	= 7	; For compatibility

; DDRA - Port A Data Direction Register
.equ	DDA0	= 0	; Data Direction Register, Port A, bit 0
.equ	DDA1	= 1	; Data Direction Register, Port A, bit 1
.equ	DDA2	= 2	; Data Direction Register, Port A, bit 2
.equ	DDA3	= 3	; Data Direction Register, Port A, bit 3
.equ	DDA4	= 4	; Data Direction Register, Port A, bit 4
.equ	DDA5	= 5	; Data Direction Register, Port A, bit 5
.equ	DDA6	= 6	; Data Direction Register, Port A, bit 6
.equ	DDA7	= 7	; Data Direction Register, Port A, bit 7

; PINA - Port A Input Pins
.equ	PINA0	= 0	; Input Pins, Port A bit 0
.equ	PINA1	= 1	; Input Pins, Port A bit 1
.equ	PINA2	= 2	; Input Pins, Port A bit 2
.equ	PINA3	= 3	; Input Pins, Port A bit 3
.equ	PINA4	= 4	; Input Pins, Port A bit 4
.equ	PINA5	= 5	; Input Pins, Port A bit 5
.equ	PINA6	= 6	; Input Pins, Port A bit 6
.equ	PINA7	= 7	; Input Pins, Port A bit 7


; ***** PORTB ************************
; PORTB - Port B Data Register
.equ	PORTB0	= 0	; Port B Data Register bit 0
.equ	PB0	= 0	; For compatibility
.equ	PORTB1	= 1	; Port B Data Register bit 1
.equ	PB1	= 1	; For compatibility
.equ	PORTB2	= 2	; Port B Data Register bit 2
.equ	PB2	= 2	; For compatibility
.equ	PORTB3	= 3	; Port B Data Register bit 3
.equ	PB3	= 3	; For compatibility
.equ	PORTB4	= 4	; Port B Data Register bit 4
.equ	PB4	= 4	; For compatibility
.equ	PORTB5	= 5	; Port B Data Register bit 5
.equ	PB5	= 5	; For compatibility
.equ	PORTB6	= 6	; Port B Data Register bit 6
.equ	PB6	= 6	; For compatibility
.equ	PORTB7	= 7	; Port B Data Register bit 7
.equ	PB7	= 7	; For compatibility

; DDRB - Port B Data Direction Register
.equ	DDB0	= 0	; Port B Data Direction Register bit 0
.equ	DDB1	= 1	; Port B Data Direction Register bit 1
.equ	DDB2	= 2	; Port B Data Direction Register bit 2
.equ	DDB3	= 3	; Port B Data Direction Register bit 3
.equ	DDB4	= 4	; Port B Data Direction Register bit 4
.equ	DDB5	= 5	; Port B Data Direction Register bit 5
.equ	DDB6	= 6	; Port B Data Direction Register bit 6
.equ	DDB7	= 7	; Port B Data Direction Register bit 7

; PINB - Port B Input Pins
.equ	PINB0	= 0	; Port B Input Pins bit 0
.equ	PINB1	= 1	; Port B Input Pins bit 1
.equ	PINB2	= 2	; Port B Input Pins bit 2
.equ	PINB3	= 3	; Port B Input Pins bit 3
.equ	PINB4	= 4	; Port B Input Pins bit 4
.equ	PINB5	= 5	; Port B Input Pins bit 5
.equ	PINB6	= 6	; Port B Input Pins bit 6
.equ	PINB7	= 7	; Port B Input Pins bit 7


; ***** EEPROM ***********************
; EEAR - EEPROM Read/Write Access
.equ	EEAR0	= 0	; EEPROM Read/Write Access bit 0
.equ	EEAR1	= 1	; EEPROM Read/Write Access bit 1
.equ	EEAR2	= 2	; EEPROM Read/Write Access bit 2
.equ	EEAR3	= 3	; EEPROM Read/Write Access bit 3
.equ	EEAR4	= 4	; EEPROM Read/Write Access bit 4
.equ	EEAR5	= 5	; EEPROM Read/Write Access bit 5
.equ	EEAR6	= 6	; EEPROM Read/Write Access bit 6

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
.equ	EEWE	= 1	; EEPROM Write Enable
.equ	EEMWE	= 2	; EEPROM Master Write Enable
.equ	EERIE	= 3	; EEProm Ready Interrupt Enable


; ***** WATCHDOG *********************
; WDTCR - Watchdog Timer Control Register
.equ	WDTCSR	= WDTCR	; For compatibility
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDCE	= 4	; Watchdog Change Enable
.equ	WDTOE	= WDCE	; For compatibility


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

; SP - Stack Pointer
.equ	SP0	= 0	; Stack Pointer Bit 0
.equ	SP1	= 1	; Stack Pointer Bit 1
.equ	SP2	= 2	; Stack Pointer Bit 2
.equ	SP3	= 3	; Stack Pointer Bit 3
.equ	SP4	= 4
.equ	SP5	= 5	; Stack Pointer Bit 5
.equ	SP6	= 6	; Stack Pointer Bit 6
.equ	SP7	= 7	; Stack Pointer Bit 7

; MCUCR - MCU Control Register
.equ	ISC00	= 0	; Interrupt Sense Control 0 bit 0
.equ	ISC01	= 1	; Interrupt Sense Control 0 bit 1
.equ	SM0	= 3	; Sleep Mode Select Bit 0
.equ	SM1	= 4	; Sleep Mode Select Bit 1
.equ	SE	= 5	; Sleep Enable
.equ	PUD	= 6	; Pull-up Disable

; MCUSR - MCU Status register
.equ	PORF	= 0	; Power-On Reset Flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BORF	= 2	; Brown-out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag

; OSCCAL - Status Register
.equ	CAL0	= 0	; Oscillator Calibration Value Bit 0
.equ	OSCCAL0	= CAL0	; For compatibility
.equ	CAL1	= 1	; Oscillator Calibration Value Bit 1
.equ	OSCCAL1	= CAL1	; For compatibility
.equ	CAL2	= 2	; Oscillator Calibration Value Bit 2
.equ	OSCCAL2	= CAL2	; For compatibility
.equ	CAL3	= 3	; Oscillator Calibration Value Bit 3
.equ	OSCCAL3	= CAL3	; For compatibility
.equ	CAL4	= 4
.equ	OSCCAL4	= CAL4	; For compatibility
.equ	CAL5	= 5	; Oscillator Calibration Value Bit 5
.equ	CAL6	= 6	; Oscillator Calibration Value Bit 6
.equ	CAL7	= 7	; Oscillator Calibration Value Bit 7


; ***** TIMER_COUNTER_0 **************
; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE0	= 1	; Timer/Counter0 Overflow Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag register
.equ	TOV0	= 1	; Timer/Counter0 Overflow Flag

; TCCR0 - Timer/Counter0 Control Register
.equ	CS00	= 0	; Clock Select0 bit 0
.equ	CS01	= 1	; Clock Select0 bit 1
.equ	CS02	= 2	; Clock Select0 bit 2
.equ	PSR0	= 3	; Prescaler Reset Timer/Counter0

; TCNT0 - Timer Counter 0
.equ	TCNT00	= 0	; Timer Counter 0 bit 0
.equ	TCNT01	= 1	; Timer Counter 0 bit 1
.equ	TCNT02	= 2	; Timer Counter 0 bit 2
.equ	TCNT03	= 3	; Timer Counter 0 bit 3
.equ	TCNT04	= 4	; Timer Counter 0 bit 4
.equ	TCNT05	= 5	; Timer Counter 0 bit 5
.equ	TCNT06	= 6	; Timer Counter 0 bit 6
.equ	TCNT07	= 7	; Timer Counter 0 bit 7


; ***** TIMER_COUNTER_1 **************
; TCCR1A - Timer/Counter Control Register A
.equ	PWM1B	= 0	; Pulse Width Modulator B Enable
.equ	PWM1A	= 1	; Pulse Width Modulator A Enable
.equ	FOC1B	= 2	; Force Output Compare Match 1B
.equ	FOC1A	= 3	; Force Output Compare Match 1A
.equ	COM1B0	= 4	; Comparator B Output Mode Bit 0
.equ	COM1B1	= 5	; Comparator B Output Mode Bit 1
.equ	COM1A0	= 6	; Comparator A Output Mode Bit 0
.equ	COM1A1	= 7	; Comparator A Output Mode Bit 1

; TCCR1B - Timer/Counter Control Register B
.equ	CS10	= 0	; Clock Select Bits
.equ	CS11	= 1	; Clock Select Bits
.equ	CS12	= 2	; Clock Select Bits
.equ	CS13	= 3	; Clock Select Bits
.equ	PSR1	= 6	; Prescaler Reset Timer/Counter1
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

; OCR1C - Output Compare Register
.equ	OCR1C0	= 0	; Output Compare Register C Bit 0
.equ	OCR1C1	= 1	; Output Compare Register C Bit 1
.equ	OCR1C2	= 2	; Output Compare Register C Bit 2
.equ	OCR1C3	= 3	; Output Compare Register C Bit 3
.equ	OCR1C4	= 4	; Output Compare Register C Bit 4
.equ	OCR1C5	= 5	; Output Compare Register C Bit 5
.equ	OCR1C6	= 6	; Output Compare Register C Bit 6
.equ	OCR1C7	= 7	; Output Compare Register C Bit 7

; TIMSK - Timer/Counter Interrupt Mask Register
;.equ	TOIE0	= 1	; Timer/Counter1 Overflow Interrupt Enable
.equ	TOIE1	= 2	; Timer/Counter1 Overflow Interrupt Enable
.equ	OCIE1B	= 5	; Timer/Counter1 Output Compare Interrupt Enable
.equ	OCIE1A	= 6	; Timer/Counter1 Output Compare Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag Register
.equ	TOV1	= 2	; Timer/Counter1 Overflow Flag
.equ	OCF1B	= 5	; Timer/Counter1 Output Compare Flag 1B
.equ	OCF1A	= 6	; Timer/Counter1 Output Compare Flag 1A

; PLLCSR - PLL Control and Status Register
.equ	PLOCK	= 0	; PLL Lock Detector
.equ	PLLE	= 1	; PLL Enable
.equ	PCKE	= 2	; PCK Enable


; ***** EXTERNAL_INTERRUPT ***********
; GIMSK - General Interrupt Mask Register
.equ	PCIE0	= 4	; Pin Change Interrupt Enable 0
.equ	PCIE1	= 5	; Pin Change Interrupt Enable 1
.equ	INT0	= 6	; External Interrupt Request 0 Enable

; GIFR - General Interrupt Flag register
.equ	PCIF	= 5	; Pin Change Interrupt Flag
.equ	INTF0	= 6	; External Interrupt Flag 0



; ***** LOCKSBITS ********************************************************
.equ	LB1	= 0	; Lockbit
.equ	LB2	= 1	; Lockbit


; ***** FUSES ************************************************************
; LOW fuse bits
.equ	CKSEL0	= 0	; Select Clock Source
.equ	CKSEL1	= 1	; Select Clock Source
.equ	CKSEL2	= 2	; Select Clock Source
.equ	CKSEL3	= 3	; Select Clock Source
.equ	SUT0	= 4	; Select start-up time
.equ	SUT1	= 5	; Select start-up time
.equ	CKOPT	= 6	; Oscillator options
.equ	PLLCK	= 7	; Use PLL for internal clock

; HIGH fuse bits
.equ	BODEN	= 0	; Brown out detector enable
.equ	BODLEVEL	= 1	; Brown out detector trigger level
.equ	EESAVE	= 2	; EEPROM memory is preserved through the Chip Erase
.equ	SPIEN	= 3	; Enable Serial Program and Data Downloading
.equ	RSTDISBL	= 4	; Select if PB/ is I/O pin or RESET pin



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x03ff	; Note: Word address
.equ	IOEND	= 0x003f
.equ	SRAM_START	= 0x0060
.equ	SRAM_SIZE	= 128
.equ	RAMEND	= 0x00df
.equ	XRAMEND	= 0x0000
.equ	E2END	= 0x007f
.equ	EEPROMEND	= 0x007f
.equ	EEADRBITS	= 7
#pragma AVRPART MEMORY PROG_FLASH 2048
#pragma AVRPART MEMORY EEPROM 128
#pragma AVRPART MEMORY INT_SRAM SIZE 128
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60





; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0001	; External Interrupt 0
.equ	PCI0addr	= 0x0002	; External Interrupt Request 0
.equ	OC1Aaddr	= 0x0003	; Timer/Counter1 Compare Match 1A
.equ	OC1Baddr	= 0x0004	; Timer/Counter1 Compare Match 1B
.equ	OVF1addr	= 0x0005	; Timer/Counter1 Overflow
.equ	OVF0addr	= 0x0006	; Timer/Counter0 Overflow
.equ	USI_STARTaddr	= 0x0007	; USI Start
.equ	USI_OVFaddr	= 0x0008	; USI Overflow
.equ	ERDYaddr	= 0x0009	; EEPROM Ready
.equ	ACIaddr	= 0x000a	; Analog Comparator
.equ	ADCCaddr	= 0x000b	; ADC Conversion Complete

.equ	INT_VECTORS_SIZE	= 12	; size in words

#pragma AVRPART CORE INSTRUCTIONS_NOT_SUPPORTED break

#endif  /* _TN26DEF_INC_ */

; ***** END OF FILE ******************************************************
