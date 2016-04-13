;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:31 ******* Source: ATtiny15.xml ************
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "tn15def.inc"
;* Title             : Register/Bit Definitions for the ATtiny15
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATtiny15
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

#ifndef _TN15DEF_INC_
#define _TN15DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATtiny15
#pragma AVRPART ADMIN PART_NAME ATtiny15
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x90
.equ	SIGNATURE_002	= 0x06

#pragma AVRPART CORE CORE_VERSION V0E


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	SREG	= 0x3f
.equ	GIMSK	= 0x3b
.equ	GIFR	= 0x3a
.equ	TIMSK	= 0x39
.equ	TIFR	= 0x38
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	TCCR0	= 0x33
.equ	TCNT0	= 0x32
.equ	OSCCAL	= 0x31
.equ	TCCR1	= 0x30
.equ	TCNT1	= 0x2f
.equ	OCR1A	= 0x2e
.equ	OCR1B	= 0x2d
.equ	SFIOR	= 0x2c
.equ	WDTCR	= 0x21
.equ	EEAR	= 0x1e
.equ	EEDR	= 0x1d
.equ	EECR	= 0x1c
.equ	PORTB	= 0x18
.equ	DDRB	= 0x17
.equ	PINB	= 0x16
.equ	ACSR	= 0x08
.equ	ADMUX	= 0x07
.equ	ADCSR	= 0x06
.equ	ADCH	= 0x05
.equ	ADCL	= 0x04


; ***** BIT DEFINITIONS **************************************************

; ***** AD_CONVERTER *****************
; ADMUX - The ADC multiplexer Selection Register
.equ	MUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	MUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	MUX2	= 2	; Analog Channel and Gain Selection Bits
.equ	ADLAR	= 5	; Left Adjust Result
.equ	REFS0	= 6	; Reference Selection Bit 0
.equ	REFS1	= 7	; Reference Selection Bit 1

; ADCSR - The ADC Control and Status register
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
.equ	ACIE	= 3	; Analog Comparator Interrupt Enable
.equ	ACI	= 4	; Analog Comparator Interrupt Flag
.equ	ACO	= 5	; Analog Compare Output
.equ	ACBG	= 6	; Analog Comparator Bandgap Select
.equ	AINBG6	= ACBG	; For compatibility
.equ	ACD	= 7	; Analog Comparator Disable


; ***** EEPROM ***********************
; EEAR - EEPROM Read/Write Access
.equ	EEAR0	= 0	; EEPROM Read/Write Access bit 0
.equ	EEAR1	= 1	; EEPROM Read/Write Access bit 1
.equ	EEAR2	= 2	; EEPROM Read/Write Access bit 2
.equ	EEAR3	= 3	; EEPROM Read/Write Access bit 3
.equ	EEAR4	= 4	; EEPROM Read/Write Access bit 4
.equ	EEAR5	= 5	; EEPROM Read/Write Access bit 5

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


; ***** TIMER_COUNTER_0 **************
; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE0	= 1	; Timer/Counter0 Overflow Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag register
.equ	TOV0	= 1	; Timer/Counter0 Overflow Flag

; TCCR0 - Timer/Counter0 Control Register
.equ	CS00	= 0	; Clock Select0 bit 0
.equ	CS01	= 1	; Clock Select0 bit 1
.equ	CS02	= 2	; Clock Select0 bit 2

; TCNT0 - Timer Counter 0
.equ	TCNT00	= 0	; Timer Counter 0 bit 0
.equ	TCNT01	= 1	; Timer Counter 0 bit 1
.equ	TCNT02	= 2	; Timer Counter 0 bit 2
.equ	TCNT03	= 3	; Timer Counter 0 bit 3
.equ	TCNT04	= 4	; Timer Counter 0 bit 4
.equ	TCNT05	= 5	; Timer Counter 0 bit 5
.equ	TCNT06	= 6	; Timer Counter 0 bit 6
.equ	TCNT07	= 7	; Timer Counter 0 bit 7


; ***** WATCHDOG *********************
; WDTCR - Watchdog Timer Control Register
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDTOE	= 4	; RW
.equ	WDDE	= WDTOE	; For compatibility


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
.equ	CAL1	= 1	; Oscillator Calibration Value Bit 1
.equ	CAL2	= 2	; Oscillator Calibration Value Bit 2
.equ	CAL3	= 3	; Oscillator Calibration Value Bit 3
.equ	CAL4	= 4
.equ	CAL5	= 5	; Oscillator Calibration Value Bit 5
.equ	CAL6	= 6	; Oscillator Calibration Value Bit 6
.equ	CAL7	= 7	; Oscillator Calibration Value Bit 7


; ***** EXTERNAL_INTERRUPT ***********
; GIMSK - General Interrupt Mask Register
.equ	PCIE	= 5	; Pin Change Interrupt Enable
.equ	INT0	= 6	; External Interrupt Request 0 Enable

; GIFR - General Interrupt Flag register
.equ	PCIF	= 5	; Pin Change Interrupt Flag
.equ	INTF0	= 6	; External Interrupt Flag 0


; ***** TIMER_COUNTER_1 **************
; TCCR1 - Timer/Counter Control Register
.equ	CS10	= 0	; Clock Select Bits
.equ	CS11	= 1	; Clock Select Bits
.equ	CS12	= 2	; Clock Select Bits
.equ	CS13	= 3	; Clock Select Bits
.equ	COM1A0	= 4	; Compare Output Mode, Bit 1
.equ	COM1A1	= 5	; Compare Output Mode, Bit 0
.equ	PWM1	= 6	; Pulse Width Modulator Enable
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

; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE1	= 2	; Timer/Counter1 Overflow Interrupt Enable
.equ	OCIE1A	= 6	; OCIE1A: Timer/Counter1 Output Compare Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag Register
.equ	TOV1	= 2	; Timer/Counter1 Overflow Flag
.equ	OCF1A	= 6	; Timer/Counter1 Output Compare Flag 1A

; SFIOR - Special Function IO Register
.equ	PSR0	= 0	; Prescaler Reset Timer/Counter0
.equ	PSR1	= 1	; Prescaler Reset Timer/Counter1
.equ	FOC1A	= 2	; Force Output Compare 1A



; ***** LOCKSBITS ********************************************************
.equ	LB1	= 0	; Lockbit
.equ	LB2	= 1	; Lockbit


; ***** FUSES ************************************************************
; LOW fuse bits



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x01ff	; Note: Word address
.equ	IOEND	= 0x003f
.equ	SRAM_SIZE	= 0
.equ	RAMEND	= 0x0000
.equ	XRAMEND	= 0x0000
.equ	E2END	= 0x003f
.equ	EEPROMEND	= 0x003f
.equ	EEADRBITS	= 6
#pragma AVRPART MEMORY PROG_FLASH 1024
#pragma AVRPART MEMORY EEPROM 64
#pragma AVRPART MEMORY INT_SRAM SIZE 0
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x0





; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0001	; External Interrupt 0
.equ	PCI0addr	= 0x0002	; External Interrupt Request 0
.equ	OC1addr	= 0x0003	; Timer/Counter1 Compare Match
.equ	OVF1addr	= 0x0004	; Timer/Counter1 Overflow
.equ	OVF0addr	= 0x0005	; Timer/Counter0 Overflow
.equ	ERDYaddr	= 0x0006	; EEPROM Ready
.equ	ACIaddr	= 0x0007	; Analog Comparator
.equ	ADCCaddr	= 0x0008	; ADC Conversion Ready

.equ	INT_VECTORS_SIZE	= 9	; size in words

#pragma AVRPART CORE INSTRUCTIONS_NOT_SUPPORTED break

#endif  /* _TN15DEF_INC_ */

; ***** END OF FILE ******************************************************
