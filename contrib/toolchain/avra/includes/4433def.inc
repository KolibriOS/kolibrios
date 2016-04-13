;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:30 ******* Source: AT90S4433.xml ***********
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "4433def.inc"
;* Title             : Register/Bit Definitions for the AT90S4433
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : AT90S4433
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

#ifndef _4433DEF_INC_
#define _4433DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device AT90S4433
#pragma AVRPART ADMIN PART_NAME AT90S4433
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x92
.equ	SIGNATURE_002	= 0x03

#pragma AVRPART CORE CORE_VERSION V1


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
.equ	TCCR1A	= 0x2f
.equ	TCCR1B	= 0x2e
.equ	TCNT1H	= 0x2d
.equ	TCNT1L	= 0x2c
.equ	OCR1H	= 0x2b
.equ	OCR1L	= 0x2a
.equ	ICR1H	= 0x27
.equ	ICR1L	= 0x26
.equ	WDTCR	= 0x21
.equ	EEAR	= 0x1e
.equ	EEDR	= 0x1d
.equ	EECR	= 0x1c
.equ	PORTB	= 0x18
.equ	DDRB	= 0x17
.equ	PINB	= 0x16
.equ	PORTC	= 0x15
.equ	DDRC	= 0x14
.equ	PINC	= 0x13
.equ	PORTD	= 0x12
.equ	DDRD	= 0x11
.equ	PIND	= 0x10
.equ	SPDR	= 0x0f
.equ	SPSR	= 0x0e
.equ	SPCR	= 0x0d
.equ	UDR	= 0x0c
.equ	UCSRA	= 0x0b
.equ	UCSRB	= 0x0a
.equ	UBRR	= 0x09
.equ	ACSR	= 0x08
.equ	ADMUX	= 0x07
.equ	ADCSR	= 0x06
.equ	ADCH	= 0x05
.equ	ADCL	= 0x04
.equ	UBRRHI	= 0x03


; ***** BIT DEFINITIONS **************************************************

; ***** ANALOG_COMPARATOR ************
; ACSR - Analog Comparator Control And Status Register
.equ	ACIS0	= 0	; Analog Comparator Interrupt Mode Select bit 0
.equ	ACIS1	= 1	; Analog Comparator Interrupt Mode Select bit 1
.equ	ACIC	= 2	; Analog Comparator Input Capture Enable
.equ	ACIE	= 3	; Analog Comparator Interrupt Enable
.equ	ACI	= 4	; Analog Comparator Interrupt Flag
.equ	ACO	= 5	; Analog Compare Output
.equ	AINBG	= 6	; Analog Comparator Bandgap Select
.equ	ACD	= 7	; Analog Comparator Disable


; ***** AD_CONVERTER *****************
; ADMUX - The ADC multiplexer Selection Register
.equ	MUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	MUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	MUX2	= 2	; Analog Channel and Gain Selection Bits
.equ	ADCBG	= 6	; ADC Bandgap Select

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
.equ	ADC8	= 0	; ADC Data Register High Byte Bit 0
.equ	ADC9	= 1	; ADC Data Register High Byte Bit 1

; ADCL - ADC Data Register Low Byte
.equ	ADC0	= 0	; ADC Data Register Low Byte Bit 0
.equ	ADC1	= 1	; ADC Data Register Low Byte Bit 1
.equ	ADC2	= 2	; ADC Data Register Low Byte Bit 2
.equ	ADC3	= 3	; ADC Data Register Low Byte Bit 3
.equ	ADC4	= 4	; ADC Data Register Low Byte Bit 4
.equ	ADC5	= 5	; ADC Data Register Low Byte Bit 5
.equ	ADC6	= 6	; ADC Data Register Low Byte Bit 6
.equ	ADC7	= 7	; ADC Data Register Low Byte Bit 7


; ***** UART *************************
; UDR - UART I/O Data Register
.equ	UDR0	= 0	; UART I/O Data Register bit 0
.equ	UDR1	= 1	; UART I/O Data Register bit 1
.equ	UDR2	= 2	; UART I/O Data Register bit 2
.equ	UDR3	= 3	; UART I/O Data Register bit 3
.equ	UDR4	= 4	; UART I/O Data Register bit 4
.equ	UDR5	= 5	; UART I/O Data Register bit 5
.equ	UDR6	= 6	; UART I/O Data Register bit 6
.equ	UDR7	= 7	; UART I/O Data Register bit 7

; UCSRA - UART Control and Status register A
.equ	MPCM	= 0	; Mulit-processor Communication Mode
.equ	DOR	= 3	; Data overRun
.equ	FE	= 4	; Framing Error
.equ	UDRE	= 5	; UART Data Register Empty
.equ	TXC	= 6	; UART Transmitt Complete
.equ	RXC	= 7	; UART Receive Complete

; UCSRB - UART Control an Status register B
.equ	TXB8	= 0	; Transmit Data Bit 8
.equ	RXB8	= 1	; Receive Data Bit 8
.equ	CHR9	= 2	; 9-bit Characters
.equ	TXEN	= 3	; Transmitter Enable
.equ	RXEN	= 4	; Receiver Enable
.equ	UDRIE	= 5	; UART Data Register Empty Interrupt Enable
.equ	TXCIE	= 6	; TX Complete Interrupt Enable
.equ	RXCIE	= 7	; RX Complete Interrupt Enable

; UBRRHI - UART Baud Rate Register High Byte
.equ	UBRRHI0	= 0	; UART Baud Rate Register High Byte bit 0
.equ	UBRRHI1	= 1	; UART Baud Rate Register High Byte bit 1
.equ	UBRRHI2	= 2	; UART Baud Rate Register High Byte bit 2
.equ	UBRRHI3	= 3	; UART Baud Rate Register High Byte bit 3

; UBRR - UART Baud Rate Register
.equ	UBRR0	= 0	; UART Baud Rate Register bit 0
.equ	UBRR1	= 1	; UART Baud Rate Register bit 1
.equ	UBRR2	= 2	; UART Baud Rate Register bit 2
.equ	UBRR3	= 3	; UART Baud Rate Register bit 3
.equ	UBRR4	= 4	; UART Baud Rate Register bit 4
.equ	UBRR5	= 5	; UART Baud Rate Register bit 5
.equ	UBRR6	= 6	; UART Baud Rate Register bit 6
.equ	UBRR7	= 7	; UART Baud Rate Register bit 7


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
.equ	SP0	= 0	; Stack pointer bit 0
.equ	SP1	= 1	; Stack pointer bit 1
.equ	SP2	= 2	; Stack pointer bit 2
.equ	SP3	= 3	; Stack pointer bit 3
.equ	SP4	= 4	; Stack pointer bit 4
.equ	SP5	= 5	; Stack pointer bit 5
.equ	SP6	= 6	; Stack pointer bit 6
.equ	SP7	= 7	; Stack pointer bit 7

; MCUCR - MCU Control Register
.equ	ISC00	= 0	; Interrupt Sense Control 0 bit 0
.equ	ISC01	= 1	; Interrupt Sense Control 0 bit 1
.equ	ISC10	= 2	; Interrupt Sense Control 1 bit 0
.equ	ISC11	= 3	; Interrupt Sense Control 1 bit 1
.equ	SM	= 4	; Sleep Mode Select
.equ	SE	= 5	; Sleep Enable

; MCUSR - 
.equ	PORF	= 0	; Power-on Reset Flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BORF	= 2	; Brown-Out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag


; ***** EXTERNAL_INTERRUPT ***********
; GIMSK - General Interrupt Mask Register
.equ	INT0	= 6	; External Interrupt Request 0 Enable
.equ	INT1	= 7	; External Interrupt Request 1 Enable

; GIFR - General Interrupt Flag register
.equ	INTF0	= 6	; External Interrupt Flag 0
.equ	INTF1	= 7	; External Interrupt Flag 1


; ***** EEPROM ***********************
; EEAR - EEPROM Read/Write Access
.equ	EEAR0	= 0	; EEPROM Read/Write Access bit 0
.equ	EEAR1	= 1	; EEPROM Read/Write Access bit 1
.equ	EEAR2	= 2	; EEPROM Read/Write Access bit 2
.equ	EEAR3	= 3	; EEPROM Read/Write Access bit 3
.equ	EEAR4	= 4	; EEPROM Read/Write Access bit 4
.equ	EEAR5	= 5	; EEPROM Read/Write Access bit 5
.equ	EEAR6	= 6	; EEPROM Read/Write Access bit 6
.equ	EEAR7	= 7	; EEPROM Read/Write Access bit 7

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

; DDRC - Port C Data Direction Register
.equ	DDC0	= 0	; Port C Data Direction Register bit 0
.equ	DDC1	= 1	; Port C Data Direction Register bit 1
.equ	DDC2	= 2	; Port C Data Direction Register bit 2
.equ	DDC3	= 3	; Port C Data Direction Register bit 3
.equ	DDC4	= 4	; Port C Data Direction Register bit 4
.equ	DDC5	= 5	; Port C Data Direction Register bit 5

; PINC - Port C Input Pins
.equ	PINC0	= 0	; Port C Input Pins bit 0
.equ	PINC1	= 1	; Port C Input Pins bit 1
.equ	PINC2	= 2	; Port C Input Pins bit 2
.equ	PINC3	= 3	; Port C Input Pins bit 3
.equ	PINC4	= 4	; Port C Input Pins bit 4
.equ	PINC5	= 5	; Port C Input Pins bit 5


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


; ***** TIMER_COUNTER_1 **************
; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TICIE1	= 3	; Timer/Counter1 Input Capture Interrupt Enable
.equ	OCIE1	= 6	; Timer/Counter1 Output Compare Match Interrupt Enable
.equ	TOIE1	= 7	; Timer/Counter1 Overflow Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag register
.equ	ICF1	= 3	; Input Capture Flag 1
.equ	OCF1	= 6	; Output Compare Flag 1
.equ	TOV1	= 7	; Timer/Counter1 Overflow Flag

; TCCR1A - Timer/Counter1 Control Register A
.equ	PWM10	= 0	; Pulse Width Modulator Select Bit 0
.equ	PWM11	= 1	; Pulse Width Modulator Select Bit 1
.equ	COM10	= 6	; Compare Ouput Mode 1, bit 0
.equ	COM11	= 7	; Compare Output Mode 1, bit 1

; TCCR1B - Timer/Counter1 Control Register B
.equ	CS10	= 0	; Clock Select1 bit 0
.equ	CS11	= 1	; Clock Select1 bit 1
.equ	CS12	= 2	; Clock Select1 bit 2
.equ	CTC1	= 3	; Clear Timer/Counter1 on Compare Match
.equ	ICES1	= 6	; Input Capture 1 Edge Select
.equ	ICNC1	= 7	; Input Capture 1 Noise Canceler


; ***** WATCHDOG *********************
; WDTCR - Watchdog Timer Control Register
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDTOE	= 4	; RW
.equ	WDDE	= WDTOE	; For compatibility



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
.equ	FLASHEND	= 0x07ff	; Note: Word address
.equ	IOEND	= 0x003f
.equ	SRAM_START	= 0x0060
.equ	SRAM_SIZE	= 128
.equ	RAMEND	= 0x00df
.equ	XRAMEND	= 0x0000
.equ	E2END	= 0x00ff
.equ	EEPROMEND	= 0x00ff
.equ	EEADRBITS	= 8
#pragma AVRPART MEMORY PROG_FLASH 4096
#pragma AVRPART MEMORY EEPROM 256
#pragma AVRPART MEMORY INT_SRAM SIZE 128
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60





; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0001	; External Interrupt 0
.equ	INT1addr	= 0x0002	; External Interrupt 1
.equ	ICP1addr	= 0x0003	; Timer/Counter Capture Event
.equ	OC1addr	= 0x0004	; Timer/Counter1 Compare Match
.equ	OVF1addr	= 0x0005	; Timer/Counter1 Overflow
.equ	OVF0addr	= 0x0006	; Timer/Counter0 Overflow
.equ	SPIaddr	= 0x0007	; Serial Transfer Complete
.equ	URXCaddr	= 0x0008	; UART, Rx Complete
.equ	UDREaddr	= 0x0009	; UART Data Register Empty
.equ	UTXCaddr	= 0x000a	; UART, Tx Complete
.equ	ADCCaddr	= 0x000b	; ADC Conversion Complete
.equ	ERDYaddr	= 0x000c	; EEPROM Ready
.equ	ACIaddr	= 0x000d	; Analog Comparator

.equ	INT_VECTORS_SIZE	= 14	; size in words

#pragma AVRPART CORE INSTRUCTIONS_NOT_SUPPORTED break

#endif  /* _4433DEF_INC_ */

; ***** END OF FILE ******************************************************
