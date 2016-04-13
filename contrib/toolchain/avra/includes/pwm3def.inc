;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:30 ******* Source: AT90PWM3.xml ************
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "pwm3def.inc"
;* Title             : Register/Bit Definitions for the AT90PWM3
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : AT90PWM3
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

#ifndef _PWM3DEF_INC_
#define _PWM3DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device AT90PWM3
#pragma AVRPART ADMIN PART_NAME AT90PWM3
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x93
.equ	SIGNATURE_002	= 0x81

#pragma AVRPART CORE CORE_VERSION V2E


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	PICR2H	= 0xff	; MEMORY MAPPED
.equ	PICR2L	= 0xfe	; MEMORY MAPPED
.equ	PFRC2B	= 0xfd	; MEMORY MAPPED
.equ	PFRC2A	= 0xfc	; MEMORY MAPPED
.equ	PCTL2	= 0xfb	; MEMORY MAPPED
.equ	PCNF2	= 0xfa	; MEMORY MAPPED
.equ	OCR2RBH	= 0xf9	; MEMORY MAPPED
.equ	OCR2RBL	= 0xf8	; MEMORY MAPPED
.equ	OCR2SBH	= 0xf7	; MEMORY MAPPED
.equ	OCR2SBL	= 0xf6	; MEMORY MAPPED
.equ	OCR2RAH	= 0xf5	; MEMORY MAPPED
.equ	OCR2RAL	= 0xf4	; MEMORY MAPPED
.equ	OCR2SAH	= 0xf3	; MEMORY MAPPED
.equ	OCR2SAL	= 0xf2	; MEMORY MAPPED
.equ	POM2	= 0xf1	; MEMORY MAPPED
.equ	PSOC2	= 0xf0	; MEMORY MAPPED
.equ	PICR1H	= 0xef	; MEMORY MAPPED
.equ	PICR1L	= 0xee	; MEMORY MAPPED
.equ	PFRC1B	= 0xed	; MEMORY MAPPED
.equ	PFRC1A	= 0xec	; MEMORY MAPPED
.equ	PCTL1	= 0xeb	; MEMORY MAPPED
.equ	PCNF1	= 0xea	; MEMORY MAPPED
.equ	OCR1RBH	= 0xe9	; MEMORY MAPPED
.equ	OCR1RBL	= 0xe8	; MEMORY MAPPED
.equ	OCR1SBH	= 0xe7	; MEMORY MAPPED
.equ	OCR1SBL	= 0xe6	; MEMORY MAPPED
.equ	OCR1RAH	= 0xe5	; MEMORY MAPPED
.equ	OCR1RAL	= 0xe4	; MEMORY MAPPED
.equ	OCR1SAH	= 0xe3	; MEMORY MAPPED
.equ	OCR1SAL	= 0xe2	; MEMORY MAPPED
.equ	PSOC1	= 0xe0	; MEMORY MAPPED
.equ	PICR0H	= 0xdf	; MEMORY MAPPED
.equ	PICR0L	= 0xde	; MEMORY MAPPED
.equ	PFRC0B	= 0xdd	; MEMORY MAPPED
.equ	PFRC0A	= 0xdc	; MEMORY MAPPED
.equ	PCTL0	= 0xdb	; MEMORY MAPPED
.equ	PCNF0	= 0xda	; MEMORY MAPPED
.equ	OCR0RBH	= 0xd9	; MEMORY MAPPED
.equ	OCR0RBL	= 0xd8	; MEMORY MAPPED
.equ	OCR0SBH	= 0xd7	; MEMORY MAPPED
.equ	OCR0SBL	= 0xd6	; MEMORY MAPPED
.equ	OCR0RAH	= 0xd5	; MEMORY MAPPED
.equ	OCR0RAL	= 0xd4	; MEMORY MAPPED
.equ	OCR0SAH	= 0xd3	; MEMORY MAPPED
.equ	OCR0SAL	= 0xd2	; MEMORY MAPPED
.equ	PSOC0	= 0xd0	; MEMORY MAPPED
.equ	EUDR	= 0xce	; MEMORY MAPPED
.equ	MUBRRH	= 0xcd	; MEMORY MAPPED
.equ	MUBRRL	= 0xcc	; MEMORY MAPPED
.equ	EUCSRC	= 0xca	; MEMORY MAPPED
.equ	EUCSRB	= 0xc9	; MEMORY MAPPED
.equ	EUCSRA	= 0xc8	; MEMORY MAPPED
.equ	UDR	= 0xc6	; MEMORY MAPPED
.equ	UBRRH	= 0xc5	; MEMORY MAPPED
.equ	UBRRL	= 0xc4	; MEMORY MAPPED
.equ	UCSRC	= 0xc2	; MEMORY MAPPED
.equ	UCSRB	= 0xc1	; MEMORY MAPPED
.equ	UCSRA	= 0xc0	; MEMORY MAPPED
.equ	AC2CON	= 0xaf	; MEMORY MAPPED
.equ	AC1CON	= 0xae	; MEMORY MAPPED
.equ	AC0CON	= 0xad	; MEMORY MAPPED
.equ	DACH	= 0xac	; MEMORY MAPPED
.equ	DACL	= 0xab	; MEMORY MAPPED
.equ	DACON	= 0xaa	; MEMORY MAPPED
.equ	PIM2	= 0xa5	; MEMORY MAPPED
.equ	PIFR2	= 0xa4	; MEMORY MAPPED
.equ	PIM1	= 0xa3	; MEMORY MAPPED
.equ	PIFR1	= 0xa2	; MEMORY MAPPED
.equ	PIM0	= 0xa1	; MEMORY MAPPED
.equ	PIFR0	= 0xa0	; MEMORY MAPPED
.equ	OCR1BH	= 0x8b	; MEMORY MAPPED
.equ	OCR1BL	= 0x8a	; MEMORY MAPPED
.equ	OCR1AH	= 0x89	; MEMORY MAPPED
.equ	OCR1AL	= 0x88	; MEMORY MAPPED
.equ	ICR1H	= 0x87	; MEMORY MAPPED
.equ	ICR1L	= 0x86	; MEMORY MAPPED
.equ	TCNT1H	= 0x85	; MEMORY MAPPED
.equ	TCNT1L	= 0x84	; MEMORY MAPPED
.equ	TCCR1C	= 0x82	; MEMORY MAPPED
.equ	TCCR1B	= 0x81	; MEMORY MAPPED
.equ	TCCR1A	= 0x80	; MEMORY MAPPED
.equ	DIDR1	= 0x7f	; MEMORY MAPPED
.equ	DIDR0	= 0x7e	; MEMORY MAPPED
.equ	ADMUX	= 0x7c	; MEMORY MAPPED
.equ	ADCSRB	= 0x7b	; MEMORY MAPPED
.equ	ADCSRA	= 0x7a	; MEMORY MAPPED
.equ	ADCH	= 0x79	; MEMORY MAPPED
.equ	ADCL	= 0x78	; MEMORY MAPPED
.equ	AMP1CSR	= 0x77	; MEMORY MAPPED
.equ	AMP0CSR	= 0x76	; MEMORY MAPPED
.equ	TIMSK1	= 0x6f	; MEMORY MAPPED
.equ	TIMSK0	= 0x6e	; MEMORY MAPPED
.equ	EICRA	= 0x69	; MEMORY MAPPED
.equ	OSCCAL	= 0x66	; MEMORY MAPPED
.equ	PRR	= 0x64	; MEMORY MAPPED
.equ	CLKPR	= 0x61	; MEMORY MAPPED
.equ	WDTCSR	= 0x60	; MEMORY MAPPED
.equ	SREG	= 0x3f
.equ	SPH	= 0x3e
.equ	SPL	= 0x3d
.equ	SPMCSR	= 0x37
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	SMCR	= 0x33
.equ	ACSR	= 0x30
.equ	SPDR	= 0x2e
.equ	SPSR	= 0x2d
.equ	SPCR	= 0x2c
.equ	PLLCSR	= 0x29
.equ	OCR0B	= 0x28
.equ	OCR0A	= 0x27
.equ	TCNT0	= 0x26
.equ	TCCR0B	= 0x25
.equ	TCCR0A	= 0x24
.equ	GTCCR	= 0x23
.equ	EEARH	= 0x22
.equ	EEARL	= 0x21
.equ	EEDR	= 0x20
.equ	EECR	= 0x1f
.equ	GPIOR0	= 0x1e
.equ	EIMSK	= 0x1d
.equ	EIFR	= 0x1c
.equ	GPIOR3	= 0x1b
.equ	GPIOR2	= 0x1a
.equ	GPIOR1	= 0x19
.equ	TIFR1	= 0x16
.equ	TIFR0	= 0x15
.equ	PORTE	= 0x0e
.equ	DDRE	= 0x0d
.equ	PINE	= 0x0c
.equ	PORTD	= 0x0b
.equ	DDRD	= 0x0a
.equ	PIND	= 0x09
.equ	PORTC	= 0x08
.equ	DDRC	= 0x07
.equ	PINC	= 0x06
.equ	PORTB	= 0x05
.equ	DDRB	= 0x04
.equ	PINB	= 0x03


; ***** BIT DEFINITIONS **************************************************

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


; ***** PORTC ************************
; PORTC - Port C Data Register
.equ	PORTC0	= 0	; Port C Data Register bit 0
.equ	PC0	= 0	; For compatibility
.equ	PORTC1	= 1	; Port C Data Register bit 1
.equ	PC1	= 1	; For compatibility
.equ	PORTC2	= 2	; Port C Data Register bit 2
.equ	PC2	= 2	; For compatibility
.equ	PORTC3	= 3	; Port C Data Register bit 3
.equ	PC3	= 3	; For compatibility
.equ	PORTC4	= 4	; Port C Data Register bit 4
.equ	PC4	= 4	; For compatibility
.equ	PORTC5	= 5	; Port C Data Register bit 5
.equ	PC5	= 5	; For compatibility
.equ	PORTC6	= 6	; Port C Data Register bit 6
.equ	PC6	= 6	; For compatibility
.equ	PORTC7	= 7	; Port C Data Register bit 7
.equ	PC7	= 7	; For compatibility

; DDRC - Port C Data Direction Register
.equ	DDC0	= 0	; Port C Data Direction Register bit 0
.equ	DDC1	= 1	; Port C Data Direction Register bit 1
.equ	DDC2	= 2	; Port C Data Direction Register bit 2
.equ	DDC3	= 3	; Port C Data Direction Register bit 3
.equ	DDC4	= 4	; Port C Data Direction Register bit 4
.equ	DDC5	= 5	; Port C Data Direction Register bit 5
.equ	DDC6	= 6	; Port C Data Direction Register bit 6
.equ	DDC7	= 7	; Port C Data Direction Register bit 7

; PINC - Port C Input Pins
.equ	PINC0	= 0	; Port C Input Pins bit 0
.equ	PINC1	= 1	; Port C Input Pins bit 1
.equ	PINC2	= 2	; Port C Input Pins bit 2
.equ	PINC3	= 3	; Port C Input Pins bit 3
.equ	PINC4	= 4	; Port C Input Pins bit 4
.equ	PINC5	= 5	; Port C Input Pins bit 5
.equ	PINC6	= 6	; Port C Input Pins bit 6
.equ	PINC7	= 7	; Port C Input Pins bit 7


; ***** PORTD ************************
; PORTD - Port D Data Register
.equ	PORTD0	= 0	; Port D Data Register bit 0
.equ	PD0	= 0	; For compatibility
.equ	PORTD1	= 1	; Port D Data Register bit 1
.equ	PD1	= 1	; For compatibility
.equ	PORTD2	= 2	; Port D Data Register bit 2
.equ	PD2	= 2	; For compatibility
.equ	PORTD3	= 3	; Port D Data Register bit 3
.equ	PD3	= 3	; For compatibility
.equ	PORTD4	= 4	; Port D Data Register bit 4
.equ	PD4	= 4	; For compatibility
.equ	PORTD5	= 5	; Port D Data Register bit 5
.equ	PD5	= 5	; For compatibility
.equ	PORTD6	= 6	; Port D Data Register bit 6
.equ	PD6	= 6	; For compatibility
.equ	PORTD7	= 7	; Port D Data Register bit 7
.equ	PD7	= 7	; For compatibility

; DDRD - Port D Data Direction Register
.equ	DDD0	= 0	; Port D Data Direction Register bit 0
.equ	DDD1	= 1	; Port D Data Direction Register bit 1
.equ	DDD2	= 2	; Port D Data Direction Register bit 2
.equ	DDD3	= 3	; Port D Data Direction Register bit 3
.equ	DDD4	= 4	; Port D Data Direction Register bit 4
.equ	DDD5	= 5	; Port D Data Direction Register bit 5
.equ	DDD6	= 6	; Port D Data Direction Register bit 6
.equ	DDD7	= 7	; Port D Data Direction Register bit 7

; PIND - Port D Input Pins
.equ	PIND0	= 0	; Port D Input Pins bit 0
.equ	PIND1	= 1	; Port D Input Pins bit 1
.equ	PIND2	= 2	; Port D Input Pins bit 2
.equ	PIND3	= 3	; Port D Input Pins bit 3
.equ	PIND4	= 4	; Port D Input Pins bit 4
.equ	PIND5	= 5	; Port D Input Pins bit 5
.equ	PIND6	= 6	; Port D Input Pins bit 6
.equ	PIND7	= 7	; Port D Input Pins bit 7


; ***** BOOT_LOAD ********************
; SPMCSR - Store Program Memory Control Register
.equ	SPMCR	= SPMCSR	; For compatibility
.equ	SPMEN	= 0	; Store Program Memory Enable
.equ	PGERS	= 1	; Page Erase
.equ	PGWRT	= 2	; Page Write
.equ	BLBSET	= 3	; Boot Lock Bit Set
.equ	RWWSRE	= 4	; Read While Write section read enable
.equ	ASRE	= RWWSRE	; For compatibility
.equ	RWWSB	= 6	; Read While Write Section Busy
.equ	ASB	= RWWSB	; For compatibility
.equ	SPMIE	= 7	; SPM Interrupt Enable


; ***** EEPROM ***********************
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
.equ	EERIE	= 3	; EEPROM Ready Interrupt Enable


; ***** PSC0 *************************
; PICR0H - PSC 0 Input Capture Register High
.equ	PICR0_8	= 0	; 
.equ	PICR0_9	= 1	; 
.equ	PICR0_10	= 2	; 
.equ	PICR0_11	= 3	; 

; PICR0L - PSC 0 Input Capture Register Low
.equ	PICR0_0	= 0	; 
.equ	PICR0_1	= 1	; 
.equ	PICR0_2	= 2	; 
.equ	PICR0_3	= 3	; 
.equ	PICR0_4	= 4	; 
.equ	PICR0_5	= 5	; 
.equ	PICR0_6	= 6	; 
.equ	PICR0_7	= 7	; 

; PFRC0B - PSC 0 Input B Control
.equ	PRFM0B0	= 0	; PSC 0 Retrigger and Fault Mode for Part B
.equ	PRFM0B1	= 1	; PSC 0 Retrigger and Fault Mode for Part B
.equ	PRFM0B2	= 2	; PSC 0 Retrigger and Fault Mode for Part B
.equ	PRFM0B3	= 3	; PSC 0 Retrigger and Fault Mode for Part B
.equ	PFLTE0B	= 4	; PSC 0 Filter Enable on Input Part B
.equ	PELEV0B	= 5	; PSC 0 Edge Level Selector on Input Part B
.equ	PISEL0B	= 6	; PSC 0 Input Select for Part B
.equ	PCAE0B	= 7	; PSC 0 Capture Enable Input Part B

; PFRC0A - PSC 0 Input A Control
.equ	PRFM0A0	= 0	; PSC 0 Retrigger and Fault Mode for Part A
.equ	PRFM0A1	= 1	; PSC 0 Retrigger and Fault Mode for Part A
.equ	PRFM0A2	= 2	; PSC 0 Retrigger and Fault Mode for Part A
.equ	PRFM0A3	= 3	; PSC 0 Retrigger and Fault Mode for Part A
.equ	PFLTE0A	= 4	; PSC 0 Filter Enable on Input Part A
.equ	PELEV0A	= 5	; PSC 0 Edge Level Selector on Input Part A
.equ	PISEL0A	= 6	; PSC 0 Input Select for Part A
.equ	PCAE0A	= 7	; PSC 0 Capture Enable Input Part A

; PCTL0 - PSC 0 Control Register
.equ	PRUN0	= 0	; PSC 0 Run
.equ	PCCYC0	= 1	; PSC0 Complete Cycle
.equ	PARUN0	= 2	; PSC0 Auto Run
.equ	PAOC0A	= 3	; PSC 0 Asynchronous Output Control A
.equ	PAOC0B	= 4	; PSC 0 Asynchronous Output Control B
.equ	PBFM0	= 5	; PSC 0 Balance Flank Width Modulation
.equ	PPRE00	= 6	; PSC 0 Prescaler Select 0
.equ	PPRE01	= 7	; PSC 0 Prescaler Select 1

; PCNF0 - PSC 0 Configuration Register
.equ	PCLKSEL0	= 1	; PSC 0 Input Clock Select
.equ	POP0	= 2	; PSC 0 Output Polarity
.equ	PMODE00	= 3	; PSC 0 Mode
.equ	PMODE01	= 4	; PSC 0 Mode
.equ	PLOCK0	= 5	; PSC 0 Lock
.equ	PALOCK0	= 6	; PSC 0 Autolock
.equ	PFIFTY0	= 7	; PSC 0 Fifty

; OCR0RBH - Output Compare RB Register High
.equ	OCR0RB_8	= 0	; 
.equ	OCR0RB_9	= 1	; 
.equ	OCR0RB_00	= 2	; 
.equ	OCR0RB_01	= 3	; 
.equ	OCR0RB_02	= 4	; 
.equ	OCR0RB_03	= 5	; 
.equ	OCR0RB_04	= 6	; 
.equ	OCR0RB_05	= 7	; 

; OCR0RBL - Output Compare RB Register Low
.equ	OCR0RB_0	= 0	; 
.equ	OCR0RB_1	= 1	; 
.equ	OCR0RB_2	= 2	; 
.equ	OCR0RB_3	= 3	; 
.equ	OCR0RB_4	= 4	; 
.equ	OCR0RB_5	= 5	; 
.equ	OCR0RB_6	= 6	; 
.equ	OCR0RB_7	= 7	; 

; OCR0SBH - Output Compare SB Register High
.equ	OCR0SB_8	= 0	; 
.equ	OCR0SB_9	= 1	; 
.equ	OCR0SB_00	= 2	; 
.equ	OCR0SB_01	= 3	; 

; OCR0SBL - Output Compare SB Register Low
.equ	OCR0SB_0	= 0	; 
.equ	OCR0SB_1	= 1	; 
.equ	OCR0SB_2	= 2	; 
.equ	OCR0SB_3	= 3	; 
.equ	OCR0SB_4	= 4	; 
.equ	OCR0SB_5	= 5	; 
.equ	OCR0SB_6	= 6	; 
.equ	OCR0SB_7	= 7	; 

; OCR0RAH - Output Compare RA Register High
.equ	OCR0RA_8	= 0	; 
.equ	OCR0RA_9	= 1	; 
.equ	OCR0RA_00	= 2	; 
.equ	OCR0RA_01	= 3	; 

; OCR0RAL - Output Compare RA Register Low
.equ	OCR0RA_0	= 0	; 
.equ	OCR0RA_1	= 1	; 
.equ	OCR0RA_2	= 2	; 
.equ	OCR0RA_3	= 3	; 
.equ	OCR0RA_4	= 4	; 
.equ	OCR0RA_5	= 5	; 
.equ	OCR0RA_6	= 6	; 
.equ	OCR0RA_7	= 7	; 

; OCR0SAH - Output Compare SA Register High
.equ	OCR0SA_8	= 0	; 
.equ	OCR0SA_9	= 1	; 
.equ	OCR0SA_00	= 2	; 
.equ	OCR0SA_01	= 3	; 

; OCR0SAL - Output Compare SA Register Low
.equ	OCR0SA_0	= 0	; 
.equ	OCR0SA_1	= 1	; 
.equ	OCR0SA_2	= 2	; 
.equ	OCR0SA_3	= 3	; 
.equ	OCR0SA_4	= 4	; 
.equ	OCR0SA_5	= 5	; 
.equ	OCR0SA_6	= 6	; 
.equ	OCR0SA_7	= 7	; 

; PSOC0 - PSC0 Synchro and Output Configuration
.equ	POEN0A	= 0	; PSCOUT00 Output Enable
.equ	POEN0B	= 2	; PSCOUT01 Output Enable
.equ	PSYNC00	= 4	; Synchronization Out for ADC Selection
.equ	PSYNC01	= 5	; Synchronization Out for ADC Selection

; PIM0 - PSC0 Interrupt Mask Register
.equ	PEOPE0	= 0	; End of Cycle Interrupt Enable
.equ	PEVE0A	= 3	; External Event A Interrupt Enable
.equ	PEVE0B	= 4	; External Event B Interrupt Enable
.equ	PSEIE0	= 5	; PSC 0 Synchro Error Interrupt Enable

; PIFR0 - PSC0 Interrupt Flag Register
.equ	PEOP0	= 0	; End of PSC0 Interrupt
.equ	PRN00	= 1	; Ramp Number
.equ	PRN01	= 2	; Ramp Number
.equ	PEV0A	= 3	; External Event A Interrupt
.equ	PEV0B	= 4	; External Event B Interrupt
.equ	PSEI0	= 5	; PSC 0 Synchro Error Interrupt


; ***** PSC1 *************************
; PICR1H - PSC 1 Input Capture Register High
.equ	PICR1_8	= 0	; 
.equ	PICR1_9	= 1	; 
.equ	PICR1_10	= 2	; 
.equ	PICR1_11	= 3	; 

; PICR1L - PSC 1 Input Capture Register Low
.equ	PICR1_0	= 0	; 
.equ	PICR1_1	= 1	; 
.equ	PICR1_2	= 2	; 
.equ	PICR1_3	= 3	; 
.equ	PICR1_4	= 4	; 
.equ	PICR1_5	= 5	; 
.equ	PICR1_6	= 6	; 
.equ	PICR1_7	= 7	; 

; PFRC1B - PSC 1 Input B Control
.equ	PRFM1B0	= 0	; PSC 1 Retrigger and Fault Mode for Part B
.equ	PRFM1B1	= 1	; PSC 1 Retrigger and Fault Mode for Part B
.equ	PRFM1B2	= 2	; PSC 1 Retrigger and Fault Mode for Part B
.equ	PRFM1B3	= 3	; PSC 1 Retrigger and Fault Mode for Part B
.equ	PFLTE1B	= 4	; PSC 1 Filter Enable on Input Part B
.equ	PELEV1B	= 5	; PSC 1 Edge Level Selector on Input Part B
.equ	PISEL1B	= 6	; PSC 1 Input Select for Part B
.equ	PCAE1B	= 7	; PSC 1 Capture Enable Input Part B

; PFRC1A - PSC 1 Input B Control
.equ	PRFM1A0	= 0	; PSC 1 Retrigger and Fault Mode for Part A
.equ	PRFM1A1	= 1	; PSC 1 Retrigger and Fault Mode for Part A
.equ	PRFM1A2	= 2	; PSC 1 Retrigger and Fault Mode for Part A
.equ	PRFM1A3	= 3	; PSC 1 Retrigger and Fault Mode for Part A
.equ	PFLTE1A	= 4	; PSC 1 Filter Enable on Input Part A
.equ	PELEV1A	= 5	; PSC 1 Edge Level Selector on Input Part A
.equ	PISEL1A	= 6	; PSC 1 Input Select for Part A
.equ	PCAE1A	= 7	; PSC 1 Capture Enable Input Part A

; PCTL1 - PSC 1 Control Register
.equ	PRUN1	= 0	; PSC 1 Run
.equ	PCCYC1	= 1	; PSC1 Complete Cycle
.equ	PARUN1	= 2	; PSC1 Auto Run
.equ	PAOC1A	= 3	; PSC 1 Asynchronous Output Control A
.equ	PAOC1B	= 4	; PSC 1 Asynchronous Output Control B
.equ	PBFM1	= 5	; Balance Flank Width Modulation
.equ	PPRE10	= 6	; PSC 1 Prescaler Select 0
.equ	PPRE11	= 7	; PSC 1 Prescaler Select 1

; PCNF1 - PSC 1 Configuration Register
.equ	PCLKSEL1	= 1	; PSC 1 Input Clock Select
.equ	POP1	= 2	; PSC 1 Output Polarity
.equ	PMODE10	= 3	; PSC 1 Mode
.equ	PMODE11	= 4	; PSC 1 Mode
.equ	PLOCK1	= 5	; PSC 1 Lock
.equ	PALOCK1	= 6	; PSC 1 Autolock
.equ	PFIFTY1	= 7	; PSC 1 Fifty

; OCR1RBH - Output Compare RB Register High
.equ	OCR1RB_8	= 0	; 
.equ	OCR1RB_9	= 1	; 
.equ	OCR1RB_10	= 2	; 
.equ	OCR1RB_11	= 3	; 
.equ	OCR1RB_12	= 4	; 
.equ	OCR1RB_13	= 5	; 
.equ	OCR1RB_14	= 6	; 
.equ	OCR1RB_15	= 7	; 

; OCR1RBL - Output Compare RB Register Low
.equ	OCR1RB_0	= 0	; 
.equ	OCR1RB_1	= 1	; 
.equ	OCR1RB_2	= 2	; 
.equ	OCR1RB_3	= 3	; 
.equ	OCR1RB_4	= 4	; 
.equ	OCR1RB_5	= 5	; 
.equ	OCR1RB_6	= 6	; 
.equ	OCR1RB_7	= 7	; 

; OCR1SBH - Output Compare SB Register High
.equ	OCR1SB_8	= 0	; 
.equ	OCR1SB_9	= 1	; 
.equ	OCR1SB_10	= 2	; 
.equ	OCR1SB_11	= 3	; 

; OCR1SBL - Output Compare SB Register Low
.equ	OCR1SB_0	= 0	; 
.equ	OCR1SB_1	= 1	; 
.equ	OCR1SB_2	= 2	; 
.equ	OCR1SB_3	= 3	; 
.equ	OCR1SB_4	= 4	; 
.equ	OCR1SB_5	= 5	; 
.equ	OCR1SB_6	= 6	; 
.equ	OCR1SB_7	= 7	; 

; OCR1RAH - Output Compare RA Register High
.equ	OCR1RA_8	= 0	; 
.equ	OCR1RA_9	= 1	; 
.equ	OCR1RA_10	= 2	; 
.equ	OCR1RA_11	= 3	; 

; OCR1RAL - Output Compare RA Register Low
.equ	OCR1RA_0	= 0	; 
.equ	OCR1RA_1	= 1	; 
.equ	OCR1RA_2	= 2	; 
.equ	OCR1RA_3	= 3	; 
.equ	OCR1RA_4	= 4	; 
.equ	OCR1RA_5	= 5	; 
.equ	OCR1RA_6	= 6	; 
.equ	OCR1RA_7	= 7	; 

; OCR1SAH - Output Compare SA Register High
.equ	OCR1SA_8	= 0	; 
.equ	OCR1SA_9	= 1	; 
.equ	OCR1SA_10	= 2	; 
.equ	OCR1SA_11	= 3	; 

; OCR1SAL - Output Compare SA Register Low
.equ	OCR1SA_0	= 0	; 
.equ	OCR1SA_1	= 1	; 
.equ	OCR1SA_2	= 2	; 
.equ	OCR1SA_3	= 3	; 
.equ	OCR1SA_4	= 4	; 
.equ	OCR1SA_5	= 5	; 
.equ	OCR1SA_6	= 6	; 
.equ	OCR1SA_7	= 7	; 

; PSOC1 - PSC1 Synchro and Output Configuration
.equ	POEN1A	= 0	; PSCOUT10 Output Enable
.equ	POEN1B	= 2	; PSCOUT11 Output Enable
.equ	PSYNC1_0	= 4	; Synchronization Out for ADC Selection
.equ	PSYNC1_1	= 5	; Synchronization Out for ADC Selection

; PIM1 - PSC1 Interrupt Mask Register
.equ	PEOPE1	= 0	; End of Cycle Interrupt Enable
.equ	PEVE1A	= 3	; External Event A Interrupt Enable
.equ	PEVE1B	= 4	; External Event B Interrupt Enable
.equ	PSEIE1	= 5	; PSC 1 Synchro Error Interrupt Enable

; PIFR1 - PSC1 Interrupt Flag Register
.equ	PEOP1	= 0	; End of PSC1 Interrupt
.equ	PRN10	= 1	; Ramp Number
.equ	PRN11	= 2	; Ramp Number
.equ	PEV1A	= 3	; External Event A Interrupt
.equ	PEV1B	= 4	; External Event B Interrupt
.equ	PSEI1	= 5	; PSC 1 Synchro Error Interrupt


; ***** PSC2 *************************
; PICR2H - PSC 2 Input Capture Register High
.equ	PICR2_8	= 0	; 
.equ	PICR2_9	= 1	; 
.equ	PICR2_10	= 2	; 
.equ	PICR2_11	= 3	; 

; PICR2L - PSC 2 Input Capture Register Low
.equ	PICR2_0	= 0	; 
.equ	PICR2_1	= 1	; 
.equ	PICR2_2	= 2	; 
.equ	PICR2_3	= 3	; 
.equ	PICR2_4	= 4	; 
.equ	PICR2_5	= 5	; 
.equ	PICR2_6	= 6	; 
.equ	PICR2_7	= 7	; 

; PFRC2B - PSC 2 Input B Control
.equ	PRFM2B0	= 0	; PSC 2 Retrigger and Fault Mode for Part B
.equ	PRFM2B1	= 1	; PSC 2 Retrigger and Fault Mode for Part B
.equ	PRFM2B2	= 2	; PSC 2 Retrigger and Fault Mode for Part B
.equ	PRFM2B3	= 3	; PSC 2 Retrigger and Fault Mode for Part B
.equ	PFLTE2B	= 4	; PSC 2 Filter Enable on Input Part B
.equ	PELEV2B	= 5	; PSC 2 Edge Level Selector on Input Part B
.equ	PISEL2B	= 6	; PSC 2 Input Select for Part B
.equ	PCAE2B	= 7	; PSC 2 Capture Enable Input Part B

; PFRC2A - PSC 2 Input B Control
.equ	PRFM2A0	= 0	; PSC 2 Retrigger and Fault Mode for Part A
.equ	PRFM2A1	= 1	; PSC 2 Retrigger and Fault Mode for Part A
.equ	PRFM2A2	= 2	; PSC 2 Retrigger and Fault Mode for Part A
.equ	PRFM2A3	= 3	; PSC 2 Retrigger and Fault Mode for Part A
.equ	PFLTE2A	= 4	; PSC 2 Filter Enable on Input Part A
.equ	PELEV2A	= 5	; PSC 2 Edge Level Selector on Input Part A
.equ	PISEL2A	= 6	; PSC 2 Input Select for Part A
.equ	PCAE2A	= 7	; PSC 2 Capture Enable Input Part A

; PCTL2 - PSC 2 Control Register
.equ	PRUN2	= 0	; PSC 2 Run
.equ	PCCYC2	= 1	; PSC2 Complete Cycle
.equ	PARUN2	= 2	; PSC2 Auto Run
.equ	PAOC2A	= 3	; PSC 2 Asynchronous Output Control A
.equ	PAOC2B	= 4	; PSC 2 Asynchronous Output Control B
.equ	PBFM2	= 5	; Balance Flank Width Modulation
.equ	PPRE20	= 6	; PSC 2 Prescaler Select 0
.equ	PPRE21	= 7	; PSC 2 Prescaler Select 1

; PCNF2 - PSC 2 Configuration Register
.equ	POME2	= 0	; PSC 2 Output Matrix Enable
.equ	PCLKSEL2	= 1	; PSC 2 Input Clock Select
.equ	POP2	= 2	; PSC 2 Output Polarity
.equ	PMODE20	= 3	; PSC 2 Mode
.equ	PMODE21	= 4	; PSC 2 Mode
.equ	PLOCK2	= 5	; PSC 2 Lock
.equ	PALOCK2	= 6	; PSC 2 Autolock
.equ	PFIFTY2	= 7	; PSC 2 Fifty

; OCR2RBH - Output Compare RB Register High
.equ	OCR2RB_8	= 0	; 
.equ	OCR2RB_9	= 1	; 
.equ	OCR2RB_10	= 2	; 
.equ	OCR2RB_11	= 3	; 
.equ	OCR2RB_12	= 4	; 
.equ	OCR2RB_13	= 5	; 
.equ	OCR2RB_14	= 6	; 
.equ	OCR2RB_15	= 7	; 

; OCR2RBL - Output Compare RB Register Low
.equ	OCR2RB_0	= 0	; 
.equ	OCR2RB_1	= 1	; 
.equ	OCR2RB_2	= 2	; 
.equ	OCR2RB_3	= 3	; 
.equ	OCR2RB_4	= 4	; 
.equ	OCR2RB_5	= 5	; 
.equ	OCR2RB_6	= 6	; 
.equ	OCR2RB_7	= 7	; 

; OCR2SBH - Output Compare SB Register High
.equ	OCR2SB_8	= 0	; 
.equ	OCR2SB_9	= 1	; 
.equ	OCR2SB_10	= 2	; 
.equ	OCR2SB_11	= 3	; 

; OCR2SBL - Output Compare SB Register Low
.equ	OCR2SB_0	= 0	; 
.equ	OCR2SB_1	= 1	; 
.equ	OCR2SB_2	= 2	; 
.equ	OCR2SB_3	= 3	; 
.equ	OCR2SB_4	= 4	; 
.equ	OCR2SB_5	= 5	; 
.equ	OCR2SB_6	= 6	; 
.equ	OCR2SB_7	= 7	; 

; OCR2RAH - Output Compare RA Register High
.equ	OCR2RA_8	= 0	; 
.equ	OCR2RA_9	= 1	; 
.equ	OCR2RA_10	= 2	; 
.equ	OCR2RA_11	= 3	; 

; OCR2RAL - Output Compare RA Register Low
.equ	OCR2RA_0	= 0	; 
.equ	OCR2RA_1	= 1	; 
.equ	OCR2RA_2	= 2	; 
.equ	OCR2RA_3	= 3	; 
.equ	OCR2RA_4	= 4	; 
.equ	OCR2RA_5	= 5	; 
.equ	OCR2RA_6	= 6	; 
.equ	OCR2RA_7	= 7	; 

; OCR2SAH - Output Compare SA Register High
.equ	OCR2SA_8	= 0	; 
.equ	OCR2SA_9	= 1	; 
.equ	OCR2SA_10	= 2	; 
.equ	OCR2SA_11	= 3	; 

; OCR2SAL - Output Compare SA Register Low
.equ	OCR2SA_0	= 0	; 
.equ	OCR2SA_1	= 1	; 
.equ	OCR2SA_2	= 2	; 
.equ	OCR2SA_3	= 3	; 
.equ	OCR2SA_4	= 4	; 
.equ	OCR2SA_5	= 5	; 
.equ	OCR2SA_6	= 6	; 
.equ	OCR2SA_7	= 7	; 

; POM2 - PSC 2 Output Matrix
.equ	POMV2A0	= 0	; Output Matrix Output A Ramp 0
.equ	POMV2A1	= 1	; Output Matrix Output A Ramp 1
.equ	POMV2A2	= 2	; Output Matrix Output A Ramp 2
.equ	POMV2A3	= 3	; Output Matrix Output A Ramp 3
.equ	POMV2B0	= 4	; Output Matrix Output B Ramp 0
.equ	POMV2B1	= 5	; Output Matrix Output B Ramp 2
.equ	POMV2B2	= 6	; Output Matrix Output B Ramp 2
.equ	POMV2B3	= 7	; Output Matrix Output B Ramp 3

; PSOC2 - PSC2 Synchro and Output Configuration
.equ	POEN2A	= 0	; PSCOUT20 Output Enable
.equ	POEN2C	= 1	; PSCOUT22 Output Enable
.equ	POEN2B	= 2	; PSCOUT21 Output Enable
.equ	POEN2D	= 3	; PSCOUT23 Output Enable
.equ	PSYNC2_0	= 4	; Synchronization Out for ADC Selection
.equ	PSYNC2_1	= 5	; Synchronization Out for ADC Selection
.equ	POS22	= 6	; PSC 2 Output 22 Select
.equ	POS23	= 7	; PSC 2 Output 23 Select

; PIM2 - PSC2 Interrupt Mask Register
.equ	PEOPE2	= 0	; End of Cycle Interrupt Enable
.equ	PEVE2A	= 3	; External Event A Interrupt Enable
.equ	PEVE2B	= 4	; External Event B Interrupt Enable
.equ	PSEIE2	= 5	; PSC 2 Synchro Error Interrupt Enable

; PIFR2 - PSC2 Interrupt Flag Register
.equ	PEOP2	= 0	; End of PSC2 Interrupt
.equ	PRN20	= 1	; Ramp Number
.equ	PRN21	= 2	; Ramp Number
.equ	PEV2A	= 3	; External Event A Interrupt
.equ	PEV2B	= 4	; External Event B Interrupt
.equ	PSEI2	= 5	; PSC 2 Synchro Error Interrupt


; ***** EUSART ***********************
; EUDR - EUSART I/O Data Register
.equ	EUDR0	= 0	; EUSART I/O Data Register bit 0
.equ	EUDR1	= 1	; EUSART I/O Data Register bit 1
.equ	EUDR2	= 2	; EUSART I/O Data Register bit 2
.equ	EUDR3	= 3	; EUSART I/O Data Register bit 3
.equ	EUDR4	= 4	; EUSART I/O Data Register bit 4
.equ	EUDR5	= 5	; EUSART I/O Data Register bit 5
.equ	EUDR6	= 6	; EUSART I/O Data Register bit 6
.equ	EUDR7	= 7	; EUSART I/O Data Register bit 7

; EUCSRA - EUSART Control and Status Register A
.equ	URxS0	= 0	; EUSART Control and Status Register A Bit 0
.equ	URxS1	= 1	; EUSART Control and Status Register A Bit 1
.equ	URxS2	= 2	; EUSART Control and Status Register A Bit 2
.equ	URxS3	= 3	; EUSART Control and Status Register A Bit 3
.equ	UTxS0	= 4	; EUSART Control and Status Register A Bit 4
.equ	UTxS1	= 5	; EUSART Control and Status Register A Bit 5
.equ	UTxS2	= 6	; EUSART Control and Status Register A Bit 6
.equ	UTxS3	= 7	; EUSART Control and Status Register A Bit 7

; EUCSRB - EUSART Control Register B
.equ	BODR	= 0	; Order Bit
.equ	EMCH	= 1	; Manchester Mode Bit
.equ	EUSBS	= 3	; EUSBS Enable Bit
.equ	EUSART	= 4	; EUSART Enable Bit

; EUCSRC - EUSART Status Register C
.equ	STP0	= 0	; Stop Bit 0
.equ	STP1	= 1	; Stop Bit 1
.equ	F1617	= 2	; F1617 Bit
.equ	FEM	= 3	; Frame Error Manchester Bit

; MUBRRH - Manchester Receiver Baud Rate Register High Byte
.equ	MUBRR8	= 0	; Manchester Receiver Baud Rate Register Bit 8
.equ	MUBRR9	= 1	; Manchester Receiver Baud Rate Register Bit 9
.equ	MUBRR10	= 2	; Manchester Receiver Baud Rate Register Bit 10
.equ	MUBRR11	= 3	; Manchester Receiver Baud Rate Register Bit 11
.equ	MUBRR12	= 4	; Manchester Receiver Baud Rate Register Bit 12
.equ	MUBRR13	= 5	; Manchester Receiver Baud Rate Register Bit 13
.equ	MUBRR14	= 6	; Manchester Receiver Baud Rate Register Bit 14
.equ	MUBRR15	= 7	; Manchester Receiver Baud Rate Register Bit 15

; MUBRRL - Manchester Receiver Baud Rate Register Low Byte
.equ	MUBRR0	= 0	; Manchester Receiver Baud Rate Register Bit 0
.equ	MUBRR1	= 1	; Manchester Receiver Baud Rate Register Bit 1
.equ	MUBRR2	= 2	; Manchester Receiver Baud Rate Register Bit 2
.equ	MUBRR3	= 3	; Manchester Receiver Baud Rate Register Bit 3
.equ	MUBRR4	= 4	; Manchester Receiver Baud Rate Register Bit 4
.equ	MUBRR5	= 5	; Manchester Receiver Baud Rate Register Bit 5
.equ	MUBRR6	= 6	; Manchester Receiver Baud Rate Register Bit 6
.equ	MUBRR7	= 7	; Manchester Receiver Baud Rate Register Bit 7


; ***** ANALOG_COMPARATOR ************
; AC0CON - Analog Comparator 0 Control Register
.equ	AC0M0	= 0	; Analog Comparator 0 Multiplexer Register
.equ	AC0M1	= 1	; Analog Comparator 0 Multiplexer Regsiter
.equ	AC0M2	= 2	; Analog Comparator 0 Multiplexer Register
.equ	AC0IS0	= 4	; Analog Comparator 0 Interrupt Select Bit
.equ	AC0IS1	= 5	; Analog Comparator 0  Interrupt Select Bit
.equ	AC0IE	= 6	; Analog Comparator 0 Interrupt Enable Bit
.equ	AC0EN	= 7	; Analog Comparator 0 Enable Bit

; AC1CON - Analog Comparator 1 Control Register
.equ	AC1M0	= 0	; Analog Comparator 1 Multiplexer Register
.equ	AC1M1	= 1	; Analog Comparator 1 Multiplexer Regsiter
.equ	AC1M2	= 2	; Analog Comparator 1 Multiplexer Register
.equ	AC1ICE	= 3	; Analog Comparator 1 Interrupt Capture Enable Bit
.equ	AC1IS0	= 4	; Analog Comparator 1 Interrupt Select Bit
.equ	AC1IS1	= 5	; Analog Comparator 1  Interrupt Select Bit
.equ	AC1IE	= 6	; Analog Comparator 1 Interrupt Enable Bit
.equ	AC1EN	= 7	; Analog Comparator 1 Enable Bit

; AC2CON - Analog Comparator 2 Control Register
.equ	AC2M0	= 0	; Analog Comparator 2 Multiplexer Register
.equ	AC2M1	= 1	; Analog Comparator 2 Multiplexer Regsiter
.equ	AC2M2	= 2	; Analog Comparator 2 Multiplexer Register
.equ	AC2SADE	= 3	; Analog Comparator 2 Start A/D Conversion Enable Bit
.equ	AC2IS0	= 4	; Analog Comparator 2 Interrupt Select Bit
.equ	AC2IS1	= 5	; Analog Comparator 2  Interrupt Select Bit
.equ	AC2IE	= 6	; Analog Comparator 2 Interrupt Enable Bit
.equ	AC2EN	= 7	; Analog Comparator 2 Enable Bit


; ***** DA_CONVERTER *****************
; DACH - DAC Data Register High Byte
.equ	DACH0	= 0	; DAC Data Register High Byte Bit 0
.equ	DACH1	= 1	; DAC Data Register High Byte Bit 1
.equ	DACH2	= 2	; DAC Data Register High Byte Bit 2
.equ	DACH3	= 3	; DAC Data Register High Byte Bit 3
.equ	DACH4	= 4	; DAC Data Register High Byte Bit 4
.equ	DACH5	= 5	; DAC Data Register High Byte Bit 5
.equ	DACH6	= 6	; DAC Data Register High Byte Bit 6
.equ	DACH7	= 7	; DAC Data Register High Byte Bit 7

; DACL - DAC Data Register Low Byte
.equ	DACL1	= 1	; DAC Data Register Low Byte Bit 1
.equ	DACL2	= 2	; DAC Data Register Low Byte Bit 2
.equ	DACL3	= 3	; DAC Data Register Low Byte Bit 3
.equ	DACL4	= 4	; DAC Data Register Low Byte Bit 4
.equ	DACL5	= 5	; DAC Data Register Low Byte Bit 5
.equ	DACL6	= 6	; DAC Data Register Low Byte Bit 6
.equ	DACL7	= 7	; DAC Data Register Low Byte Bit 7

; DACON - DAC Control Register
.equ	DAEN	= 0	; DAC Enable Bit
.equ	DAOE	= 1	; DAC Output Enable Bit
.equ	DALA	= 2	; DAC Left Adjust
.equ	DATS0	= 4	; DAC Trigger Selection Bit 0
.equ	DATS1	= 5	; DAC Trigger Selection Bit 1
.equ	DATS2	= 6	; DAC Trigger Selection Bit 2
.equ	DAATE	= 7	; DAC Auto Trigger Enable Bit


; ***** CPU **************************
; SREG - Status Register
.equ	SREG_C	= 0	; Carry Flag
.equ	SREG_Z	= 1	; Zero Flag
.equ	SREG_N	= 2	; Negative Flag
.equ	SREG_V	= 3	; Two's Complement Overflow Flag
.equ	SREG_S	= 4
.equ	SREG_H	= 5	; Half Carry Flag
.equ	SREG_T	= 6	; Bit Copy Storage
.equ	SREG_I	= 7	; Global Interrupt Enable

; MCUCR - MCU Control Register
.equ	IVCE	= 0	; Interrupt Vector Change Enable
.equ	IVSEL	= 1	; Interrupt Vector Select
.equ	PUD	= 4	; Pull-up disable
.equ	SPIPS	= 7	; SPI Pin Select

; MCUSR - MCU Status Register
.equ	PORF	= 0	; Power-on reset flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BORF	= 2	; Brown-out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag

; OSCCAL - Oscillator Calibration Value
.equ	CAL0	= 0	; Oscillator Calibration Value Bit0
.equ	CAL1	= 1	; Oscillator Calibration Value Bit1
.equ	CAL2	= 2	; Oscillator Calibration Value Bit2
.equ	CAL3	= 3	; Oscillator Calibration Value Bit3
.equ	CAL4	= 4	; Oscillator Calibration Value Bit4
.equ	CAL5	= 5	; Oscillator Calibration Value Bit5
.equ	CAL6	= 6	; Oscillator Calibration Value Bit6

; CLKPR - 
.equ	CLKPS0	= 0	; 
.equ	CLKPS1	= 1	; 
.equ	CLKPS2	= 2	; 
.equ	CLKPS3	= 3	; 
.equ	CPKPCE	= 7	; 

; SMCR - Sleep Mode Control Register
.equ	SE	= 0	; Sleep Enable
.equ	SM0	= 1	; Sleep Mode Select bit 0
.equ	SM1	= 2	; Sleep Mode Select bit 1
.equ	SM2	= 3	; Sleep Mode Select bit 2

; GPIOR3 - General Purpose IO Register 3
.equ	GPIOR30	= 0	; General Purpose IO Register 3 bit 0
.equ	GPIOR31	= 1	; General Purpose IO Register 3 bit 1
.equ	GPIOR32	= 2	; General Purpose IO Register 3 bit 2
.equ	GPIOR33	= 3	; General Purpose IO Register 3 bit 3
.equ	GPIOR34	= 4	; General Purpose IO Register 3 bit 4
.equ	GPIOR35	= 5	; General Purpose IO Register 3 bit 5
.equ	GPIOR36	= 6	; General Purpose IO Register 3 bit 6
.equ	GPIOR37	= 7	; General Purpose IO Register 3 bit 7

; GPIOR2 - General Purpose IO Register 2
.equ	GPIOR20	= 0	; General Purpose IO Register 2 bit 0
.equ	GPIOR21	= 1	; General Purpose IO Register 2 bit 1
.equ	GPIOR22	= 2	; General Purpose IO Register 2 bit 2
.equ	GPIOR23	= 3	; General Purpose IO Register 2 bit 3
.equ	GPIOR24	= 4	; General Purpose IO Register 2 bit 4
.equ	GPIOR25	= 5	; General Purpose IO Register 2 bit 5
.equ	GPIOR26	= 6	; General Purpose IO Register 2 bit 6
.equ	GPIOR27	= 7	; General Purpose IO Register 2 bit 7

; GPIOR1 - General Purpose IO Register 1
.equ	GPIOR10	= 0	; General Purpose IO Register 1 bit 0
.equ	GPIOR11	= 1	; General Purpose IO Register 1 bit 1
.equ	GPIOR12	= 2	; General Purpose IO Register 1 bit 2
.equ	GPIOR13	= 3	; General Purpose IO Register 1 bit 3
.equ	GPIOR14	= 4	; General Purpose IO Register 1 bit 4
.equ	GPIOR15	= 5	; General Purpose IO Register 1 bit 5
.equ	GPIOR16	= 6	; General Purpose IO Register 1 bit 6
.equ	GPIOR17	= 7	; General Purpose IO Register 1 bit 7

; GPIOR0 - General Purpose IO Register 0
.equ	GPIOR00	= 0	; General Purpose IO Register 0 bit 0
.equ	GPIOR01	= 1	; General Purpose IO Register 0 bit 1
.equ	GPIOR02	= 2	; General Purpose IO Register 0 bit 2
.equ	GPIOR03	= 3	; General Purpose IO Register 0 bit 3
.equ	GPIOR04	= 4	; General Purpose IO Register 0 bit 4
.equ	GPIOR05	= 5	; General Purpose IO Register 0 bit 5
.equ	GPIOR06	= 6	; General Purpose IO Register 0 bit 6
.equ	GPIOR07	= 7	; General Purpose IO Register 0 bit 7

; PLLCSR - PLL Control And Status Register
.equ	PLOCK	= 0	; PLL Lock Detector
.equ	PLLE	= 1	; PLL Enable
.equ	PCKE	= 2	; PCK Enable


; ***** PORTE ************************
; PORTE - Port E Data Register
.equ	PORTE0	= 0	; 
.equ	PE0	= 0	; For compatibility
.equ	PORTE1	= 1	; 
.equ	PE1	= 1	; For compatibility
.equ	PORTE2	= 2	; 
.equ	PE2	= 2	; For compatibility

; DDRE - Port E Data Direction Register
.equ	DDE0	= 0	; 
.equ	DDE1	= 1	; 
.equ	DDE2	= 2	; 

; PINE - Port E Input Pins
.equ	PINE0	= 0	; 
.equ	PINE1	= 1	; 
.equ	PINE2	= 2	; 


; ***** TIMER_COUNTER_0 **************
; TIMSK0 - Timer/Counter0 Interrupt Mask Register
.equ	TOIE0	= 0	; Timer/Counter0 Overflow Interrupt Enable
.equ	OCIE0A	= 1	; Timer/Counter0 Output Compare Match A Interrupt Enable
.equ	OCIE0B	= 2	; Timer/Counter0 Output Compare Match B Interrupt Enable

; TIFR0 - Timer/Counter0 Interrupt Flag register
.equ	TOV0	= 0	; Timer/Counter0 Overflow Flag
.equ	OCF0A	= 1	; Timer/Counter0 Output Compare Flag 0A
.equ	OCF0B	= 2	; Timer/Counter0 Output Compare Flag 0B

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
.equ	PSR10	= 0	; Prescaler Reset Timer/Counter1 and Timer/Counter0
.equ	ICPSEL1	= 6	; Timer1 Input Capture Selection Bit
.equ	TSM	= 7	; Timer/Counter Synchronization Mode


; ***** TIMER_COUNTER_1 **************
; TIMSK1 - Timer/Counter Interrupt Mask Register
.equ	TOIE1	= 0	; Timer/Counter1 Overflow Interrupt Enable
.equ	OCIE1A	= 1	; Timer/Counter1 Output CompareA Match Interrupt Enable
.equ	OCIE1B	= 2	; Timer/Counter1 Output CompareB Match Interrupt Enable
.equ	ICIE1	= 5	; Timer/Counter1 Input Capture Interrupt Enable

; TIFR1 - Timer/Counter Interrupt Flag register
.equ	TOV1	= 0	; Timer/Counter1 Overflow Flag
.equ	OCF1A	= 1	; Output Compare Flag 1A
.equ	OCF1B	= 2	; Output Compare Flag 1B
.equ	ICF1	= 5	; Input Capture Flag 1

; TCCR1A - Timer/Counter1 Control Register A
.equ	WGM10	= 0	; Waveform Generation Mode
.equ	WGM11	= 1	; Waveform Generation Mode
.equ	COM1B0	= 4	; Compare Output Mode 1B, bit 0
.equ	COM1B1	= 5	; Compare Output Mode 1B, bit 1
.equ	COM1A0	= 6	; Comparet Ouput Mode 1A, bit 0
.equ	COM1A1	= 7	; Compare Output Mode 1A, bit 1

; TCCR1B - Timer/Counter1 Control Register B
.equ	CS10	= 0	; Prescaler source of Timer/Counter 1
.equ	CS11	= 1	; Prescaler source of Timer/Counter 1
.equ	CS12	= 2	; Prescaler source of Timer/Counter 1
.equ	WGM12	= 3	; Waveform Generation Mode
.equ	WGM13	= 4	; Waveform Generation Mode
.equ	ICES1	= 6	; Input Capture 1 Edge Select
.equ	ICNC1	= 7	; Input Capture 1 Noise Canceler

; TCCR1C - Timer/Counter1 Control Register C
.equ	FOC1B	= 6	; 
.equ	FOC1A	= 7	; 

; GTCCR - General Timer/Counter Control Register
.equ	PSRSYNC	= 0	; Prescaler Reset Timer/Counter1 and Timer/Counter0
;.equ	TSM	= 7	; Timer/Counter Synchronization Mode


; ***** AD_CONVERTER *****************
; ADMUX - The ADC multiplexer Selection Register
.equ	MUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	MUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	MUX2	= 2	; Analog Channel and Gain Selection Bits
.equ	MUX3	= 3	; Analog Channel and Gain Selection Bits
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
.equ	ADASCR	= 3	; 
.equ	ADAP	= 4	; 

; DIDR0 - Digital Input Disable Register 0
.equ	ADC0D	= 0	; ADC0 Digital input Disable
.equ	ADC1D	= 1	; ADC1 Digital input Disable
.equ	ADC2D	= 2	; ADC2 Digital input Disable
.equ	ADC3D	= 3	; ADC3 Digital input Disable
.equ	ADC4D	= 4	; ADC4 Digital input Disable
.equ	ADC5D	= 5	; ADC5 Digital input Disable
.equ	ADC6D	= 6	; ADC6 Digital input Disable
.equ	ADC7D	= 7	; ADC7 Digital input Disable

; DIDR1 - 
.equ	ADC8D	= 0	; 
.equ	ADC9D	= 1	; 
.equ	ADC10D	= 2	; 
.equ	AMP0ND	= 3	; 
.equ	AMP0PD	= 4	; 
.equ	ACMP0D	= 5	; 


; ***** USART ************************
; UDR - USART I/O Data Register
.equ	UDR0	= 0	; USART I/O Data Register bit 0
.equ	UDR1	= 1	; USART I/O Data Register bit 1
.equ	UDR2	= 2	; USART I/O Data Register bit 2
.equ	UDR3	= 3	; USART I/O Data Register bit 3
.equ	UDR4	= 4	; USART I/O Data Register bit 4
.equ	UDR5	= 5	; USART I/O Data Register bit 5
.equ	UDR6	= 6	; USART I/O Data Register bit 6
.equ	UDR7	= 7	; USART I/O Data Register bit 7

; UCSRA - USART Control and Status register A
.equ	MPCM	= 0	; Multi-processor Communication Mode
.equ	U2X	= 1	; Double USART Transmission Bit
.equ	UPE	= 2	; USART Parity Error
.equ	DOR	= 3	; Data Overrun
.equ	FE	= 4	; Framing Error
.equ	UDRE	= 5	; USART Data Register Empty
.equ	TXC	= 6	; USART Transmitt Complete
.equ	RXC	= 7	; USART Receive Complete

; UCSRB - USART Control an Status register B
.equ	TXB8	= 0	; Transmit Data Bit 8
.equ	RXB8	= 1	; Receive Data Bit 8
.equ	UCSZ2	= 2	; Character Size
.equ	TXEN	= 3	; Transmitter Enable
.equ	RXEN	= 4	; Receiver Enable
.equ	UDRIE	= 5	; USART Data Register Empty Interrupt Enable
.equ	TXCIE	= 6	; TX Complete Interrupt Enable
.equ	RXCIE	= 7	; RX Complete Interrupt Enable

; UCSRC - USART Control an Status register C
.equ	UCPOL	= 0	; Clock Polarity
.equ	UCSZ0	= 1	; Character Size Bit 0
.equ	UCSZ1	= 2	; Character Size Bit 1
.equ	USBS	= 3	; Stop Bit Select
.equ	UPM0	= 4	; Parity Mode Bit 0
.equ	UPM1	= 5	; Parity Mode Bit 1
.equ	UMSEL0	= 6	; USART Mode Select

; UBRRH - USART Baud Rate Register High Byte
.equ	UBRR8	= 0	; USART Baud Rate Register Bit 8
.equ	UBRR9	= 1	; USART Baud Rate Register Bit 9
.equ	UBRR10	= 2	; USART Baud Rate Register Bit 10
.equ	UBRR11	= 3	; USART Baud Rate Register Bit 11

; UBRRL - USART Baud Rate Register Low Byte
.equ	UBRR0	= 0	; USART Baud Rate Register bit 0
.equ	UBRR1	= 1	; USART Baud Rate Register bit 1
.equ	UBRR2	= 2	; USART Baud Rate Register bit 2
.equ	UBRR3	= 3	; USART Baud Rate Register bit 3
.equ	UBRR4	= 4	; USART Baud Rate Register bit 4
.equ	UBRR5	= 5	; USART Baud Rate Register bit 5
.equ	UBRR6	= 6	; USART Baud Rate Register bit 6
.equ	UBRR7	= 7	; USART Baud Rate Register bit 7


; ***** SPI **************************
; SPDR - SPI Data Register
.equ	SPDR0	= 0	; SPI Data Register bit 0
.equ	SPDR1	= 1	; SPI Data Register bit 1
.equ	SPDR2	= 2	; SPI Data Register bit 2
.equ	SPDR3	= 3	; SPI Data Register bit 3
.equ	SPDR4	= 4	; SPI Data Register bit 4
.equ	SPDR5	= 5	; SPI Data Register bit 5
.equ	SPDR6	= 6	; SPI Data Register bit 6
.equ	SPDR7	= 7	; SPI Data Register bit 7

; SPSR - SPI Status Register
.equ	SPI2X	= 0	; Double SPI Speed Bit
.equ	WCOL	= 6	; Write Collision Flag
.equ	SPIF	= 7	; SPI Interrupt Flag

; SPCR - SPI Control Register
.equ	SPR0	= 0	; SPI Clock Rate Select 0
.equ	SPR1	= 1	; SPI Clock Rate Select 1
.equ	CPHA	= 2	; Clock Phase
.equ	CPOL	= 3	; Clock polarity
.equ	MSTR	= 4	; Master/Slave Select
.equ	DORD	= 5	; Data Order
.equ	SPE	= 6	; SPI Enable
.equ	SPIE	= 7	; SPI Interrupt Enable


; ***** WATCHDOG *********************
; WDTCSR - Watchdog Timer Control Register
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDCE	= 4	; Watchdog Change Enable
.equ	WDP3	= 5	; Watchdog Timer Prescaler Bit 3
.equ	WDIE	= 6	; Watchdog Timeout Interrupt Enable
.equ	WDIF	= 7	; Watchdog Timeout Interrupt Flag


; ***** EXTERNAL_INTERRUPT ***********
; EICRA - External Interrupt Control Register A
.equ	ISC00	= 0	; External Interrupt Sense Control Bit
.equ	ISC01	= 1	; External Interrupt Sense Control Bit
.equ	ISC10	= 2	; External Interrupt Sense Control Bit
.equ	ISC11	= 3	; External Interrupt Sense Control Bit
.equ	ISC20	= 4	; External Interrupt Sense Control Bit
.equ	ISC21	= 5	; External Interrupt Sense Control Bit
.equ	ISC30	= 6	; External Interrupt Sense Control Bit
.equ	ISC31	= 7	; External Interrupt Sense Control Bit

; EIMSK - External Interrupt Mask Register
.equ	INT0	= 0	; External Interrupt Request 0 Enable
.equ	INT1	= 1	; External Interrupt Request 1 Enable
.equ	INT2	= 2	; External Interrupt Request 2 Enable
.equ	INT3	= 3	; External Interrupt Request 3 Enable

; EIFR - External Interrupt Flag Register
.equ	INTF0	= 0	; External Interrupt Flag 0
.equ	INTF1	= 1	; External Interrupt Flag 1
.equ	INTF2	= 2	; External Interrupt Flag 2
.equ	INTF3	= 3	; External Interrupt Flag 3



; ***** LOCKSBITS ********************************************************
.equ	LB1	= 0	; Lock bit
.equ	LB2	= 1	; Lock bit
.equ	BLB01	= 2	; Boot Lock bit
.equ	BLB02	= 3	; Boot Lock bit
.equ	BLB11	= 4	; Boot lock bit
.equ	BLB12	= 5	; Boot lock bit


; ***** FUSES ************************************************************
; LOW fuse bits
.equ	CKSEL0	= 0	; Select Clock Source
.equ	CKSEL1	= 1	; Select Clock Source
.equ	CKSEL2	= 2	; Select Clock Source
.equ	CKSEL3	= 3	; Select Clock Source
.equ	SUT0	= 4	; Select start-up time
.equ	SUT1	= 5	; Select start-up time
.equ	CKOUT	= 6	; Oscillator output option
.equ	CLKDIV8	= 7	; Divide clock by 8

; HIGH fuse bits
.equ	BOOTRST	= 0	; Select Reset Vector
.equ	BOOTSZ0	= 1	; Select Boot Size
.equ	BOOTSZ1	= 2	; Select Boot Size
.equ	EESAVE	= 3	; EEPROM memory is preserved through chip erase
.equ	WDTON	= 4	; Watchdog timer always on
.equ	SPIEN	= 5	; Enable Serial programming and Data Downloading
.equ	JTAGEN	= 6	; Enable JTAG
.equ	OCDEN	= 7	; Enable OCD

; EXTENDED fuse bits
.equ	TA0SEL	= 0	; (Reserved to factory tests)
.equ	BODLEVEL0	= 1	; Brown-out Detector trigger level
.equ	BODLEVEL1	= 2	; Brown-out Detector trigger level
.equ	BODLEVEL2	= 3	; Brown out detector trigger level



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x0fff	; Note: Word address
.equ	IOEND	= 0x00ff
.equ	SRAM_START	= 0x0100
.equ	SRAM_SIZE	= 512
.equ	RAMEND	= 0x02ff
.equ	XRAMEND	= 0x0000
.equ	E2END	= 0x01ff
.equ	EEPROMEND	= 0x01ff
.equ	EEADRBITS	= 9
#pragma AVRPART MEMORY PROG_FLASH 8192
#pragma AVRPART MEMORY EEPROM 512
#pragma AVRPART MEMORY INT_SRAM SIZE 512
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100



; ***** BOOTLOADER DECLARATIONS ******************************************
.equ	NRWW_START_ADDR	= 0xc00
.equ	NRWW_STOP_ADDR	= 0xfff
.equ	RWW_START_ADDR	= 0x0
.equ	RWW_STOP_ADDR	= 0xbff
.equ	PAGESIZE	= 32
.equ	FIRSTBOOTSTART	= 0xf80
.equ	SECONDBOOTSTART	= 0xf00
.equ	THIRDBOOTSTART	= 0xe00
.equ	FOURTHBOOTSTART	= 0xc00
.equ	SMALLBOOTSTART	= FIRSTBOOTSTART
.equ	LARGEBOOTSTART	= FOURTHBOOTSTART



; ***** INTERRUPT VECTORS ************************************************
.equ	PSC2_CAPTaddr	= 0x0001	; PSC2 Capture Event
.equ	PSC2_ECaddr	= 0x0002	; PSC2 End Cycle
.equ	PSC1_CAPTaddr	= 0x0003	; PSC1 Capture Event
.equ	PSC1_ECaddr	= 0x0004	; PSC1 End Cycle
.equ	PSC0_CAPTaddr	= 0x0005	; PSC0 Capture Event
.equ	PSC0_ECaddr	= 0x0006	; PSC0 End Cycle
.equ	ACI0addr	= 0x0007	; Analog Comparator 0
.equ	ACI1addr	= 0x0008	; Analog Comparator 1
.equ	ACI2addr	= 0x0009	; Analog Comparator 2
.equ	INT0addr	= 0x000a	; External Interrupt Request 0
.equ	ICP1addr	= 0x000b	; Timer/Counter1 Capture Event
.equ	OC1Aaddr	= 0x000c	; Timer/Counter1 Compare Match A
.equ	OC1Baddr	= 0x000d	; Timer/Counter Compare Match B
.equ	OVF1addr	= 0x000f	; Timer/Counter1 Overflow
.equ	OC0Aaddr	= 0x0010	; Timer/Counter0 Compare Match A
.equ	OVF0addr	= 0x0011	; Timer/Counter0 Overflow
.equ	ADCCaddr	= 0x0012	; ADC Conversion Complete
.equ	INT1addr	= 0x0013	; External Interrupt Request 1
.equ	SPIaddr	= 0x0014	; SPI Serial Transfer Complete
.equ	URXCaddr	= 0x0015	; USART, Rx Complete
.equ	UDREaddr	= 0x0016	; USART Data Register Empty
.equ	UTXCaddr	= 0x0017	; USART, Tx Complete
.equ	INT2addr	= 0x0018	; External Interrupt Request 2
.equ	WDTaddr	= 0x0019	; Watchdog Timeout Interrupt
.equ	ERDYaddr	= 0x001a	; EEPROM Ready
.equ	OC0Baddr	= 0x001b	; Timer Counter 0 Compare Match B
.equ	INT3addr	= 0x001c	; External Interrupt Request 3
.equ	SPMRaddr	= 0x001f	; Store Program Memory Read

.equ	INT_VECTORS_SIZE	= 29	; size in words

#endif  /* _PWM3DEF_INC_ */

; ***** END OF FILE ******************************************************
