;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:30 ******* Source: ATmega161.xml ***********
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "m161def.inc"
;* Title             : Register/Bit Definitions for the ATmega161
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATmega161
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

#ifndef _M161DEF_INC_
#define _M161DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATmega161
#pragma AVRPART ADMIN PART_NAME ATmega161
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x94
.equ	SIGNATURE_002	= 0x01

#pragma AVRPART CORE CORE_VERSION V2E


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
.equ	SPMCR	= 0x37
.equ	EMCUCR	= 0x36
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	TCCR0	= 0x33
.equ	TCNT0	= 0x32
.equ	OCR0	= 0x31
.equ	SFIOR	= 0x30
.equ	TCCR1A	= 0x2f
.equ	TCCR1B	= 0x2e
.equ	TCNT1H	= 0x2d
.equ	TCNT1L	= 0x2c
.equ	OCR1AH	= 0x2b
.equ	OCR1AL	= 0x2a
.equ	OCR1BH	= 0x29
.equ	OCR1BL	= 0x28
.equ	TCCR2	= 0x27
.equ	ASSR	= 0x26
.equ	ICR1H	= 0x25
.equ	ICR1L	= 0x24
.equ	TCNT2	= 0x23
.equ	OCR2	= 0x22
.equ	WDTCR	= 0x21
.equ	UBRRHI	= 0x20
.equ	EEARH	= 0x1f
.equ	EEARL	= 0x1e
.equ	EEDR	= 0x1d
.equ	EECR	= 0x1c
.equ	PORTA	= 0x1b
.equ	DDRA	= 0x1a
.equ	PINA	= 0x19
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
.equ	UDR0	= 0x0c
.equ	UCSR0A	= 0x0b
.equ	UCSR0B	= 0x0a
.equ	UBRR0	= 0x09
.equ	ACSR	= 0x08
.equ	PORTE	= 0x07
.equ	DDRE	= 0x06
.equ	PINE	= 0x05
.equ	UDR1	= 0x03
.equ	UCSR1A	= 0x02
.equ	UCSR1B	= 0x01
.equ	UBRR1	= 0x00


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


; ***** USART0 ***********************
; UDR0 - USART I/O Data Register
.equ	UDR00	= 0	; USART I/O Data Register bit 0
.equ	UDR01	= 1	; USART I/O Data Register bit 1
.equ	UDR02	= 2	; USART I/O Data Register bit 2
.equ	UDR03	= 3	; USART I/O Data Register bit 3
.equ	UDR04	= 4	; USART I/O Data Register bit 4
.equ	UDR05	= 5	; USART I/O Data Register bit 5
.equ	UDR06	= 6	; USART I/O Data Register bit 6
.equ	UDR07	= 7	; USART I/O Data Register bit 7

; UCSR0A - USART Control and Status Register A
.equ	MPCM0	= 0	; Multi-processor Communication Mode
.equ	U2X0	= 1	; Double the USART transmission speed
.equ	OR0	= 3	; Data overRun
.equ	FE0	= 4	; Framing Error
.equ	UDRE0	= 5	; USART Data Register Empty
.equ	TXC0	= 6	; USART Transmitt Complete
.equ	RXC0	= 7	; USART Receive Complete

; UCSR0B - USART Control and Status Register B
.equ	TXB80	= 0	; Transmit Data Bit 8
.equ	RXB80	= 1	; Receive Data Bit 8
.equ	CHR90	= 2	; 9-Bit Character
.equ	TXEN0	= 3	; Transmitter Enable
.equ	RXEN0	= 4	; Receiver Enable
.equ	UDR0IE0	= 5	; USART Data register Empty Interrupt Enable
.equ	TXCIE0	= 6	; TX Complete Interrupt Enable
.equ	RXCIE0	= 7	; RX Complete Interrupt Enable

; UBRR0 - USART Baud Rate Register Byte
.equ	UBRR00	= 0	; USART Baud Rate Register bit 0
.equ	UBRR01	= 1	; USART Baud Rate Register bit 1
.equ	UBRR02	= 2	; USART Baud Rate Register bit 2
.equ	UBRR03	= 3	; USART Baud Rate Register bit 3
.equ	UBRR04	= 4	; USART Baud Rate Register bit 4
.equ	UBRR05	= 5	; USART Baud Rate Register bit 5
.equ	UBRR06	= 6	; USART Baud Rate Register bit 6
.equ	UBRR07	= 7	; USART Baud Rate Register bit 7

; UBRRHI - High Byte Baud Rate Register
.equ	UBRRHI00	= 0	; High Byte Baud Rate Register Port 0 Bit 0
.equ	UBRRHI01	= 1	; High Byte Baud Rate Register Port 0 Bit 1
.equ	UBRRHI02	= 2	; High Byte Baud Rate Register Port 0 Bit 2
.equ	UBRRHI03	= 3	; High Byte Baud Rate Register Port 0 Bit 3


; ***** USART1 ***********************
; UDR1 - USART I/O Data Register
.equ	UDR10	= 0	; USART I/O Data Register bit 0
.equ	UDR11	= 1	; USART I/O Data Register bit 1
.equ	UDR12	= 2	; USART I/O Data Register bit 2
.equ	UDR13	= 3	; USART I/O Data Register bit 3
.equ	UDR14	= 4	; USART I/O Data Register bit 4
.equ	UDR15	= 5	; USART I/O Data Register bit 5
.equ	UDR16	= 6	; USART I/O Data Register bit 6
.equ	UDR17	= 7	; USART I/O Data Register bit 7

; UCSR1A - USART Control and Status Register A
.equ	MPCM1	= 0	; Multi-processor Communication Mode
.equ	U2X1	= 1	; Double the USART transmission speed
.equ	OR1	= 3	; Data overRun
.equ	FE1	= 4	; Framing Error
.equ	UDRE1	= 5	; USART Data Register Empty
.equ	TXC1	= 6	; USART Transmitt Complete
.equ	RXC1	= 7	; USART Receive Complete

; UCSR1B - USART Control and Status Register B
.equ	TXB81	= 0	; Transmit Data Bit 8
.equ	RXB81	= 1	; Receive Data Bit 8
.equ	CHR91	= 2	; 9-Bit Character
.equ	TXEN1	= 3	; Transmitter Enable
.equ	RXEN1	= 4	; Receiver Enable
.equ	UDR1IE1	= 5	; USART Data register Empty Interrupt Enable
.equ	TXCIE1	= 6	; TX Complete Interrupt Enable
.equ	RXCIE1	= 7	; RX Complete Interrupt Enable

; UBRR1 - USART Baud Rate Register Byte
.equ	UBRR10	= 0	; USART Baud Rate Register bit 0
.equ	UBRR11	= 1	; USART Baud Rate Register bit 1
.equ	UBRR12	= 2	; USART Baud Rate Register bit 2
.equ	UBRR13	= 3	; USART Baud Rate Register bit 3
.equ	UBRR14	= 4	; USART Baud Rate Register bit 4
.equ	UBRR15	= 5	; USART Baud Rate Register bit 5
.equ	UBRR16	= 6	; USART Baud Rate Register bit 6
.equ	UBRR17	= 7	; USART Baud Rate Register bit 7

; UBRRHI - high Byte Baud Rate Register
.equ	UBRRHI10	= 4	; High Byte Baud Rate Register Port 0 Bit 0
.equ	UBRRHI11	= 5	; High Byte Baud Rate Register Port 0 Bit 1
.equ	UBRRHI12	= 6	; High Byte Baud Rate Register Port 0 Bit 2
.equ	UBRRHI13	= 7	; High Byte Baud Rate Register Port 0 Bit 3


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
.equ	EEWEE	= EEMWE	; For compatibility
.equ	EERIE	= 3	; EEPROM Ready Interrupt Enable


; ***** EXTERNAL_INTERRUPT ***********
; GIMSK - General Interrupt Mask Register
.equ	INT2	= 5	; External Interrupt Request 2 Enable
.equ	INT0	= 6	; External Interrupt Request 0 Enable
.equ	INT1	= 7	; External Interrupt Request 1 Enable

; GIFR - General Interrupt Flag Register
.equ	INTF2	= 5	; External Interrupt Flag 2
.equ	INTF0	= 6	; External Interrupt Flag 0
.equ	INTF1	= 7	; External Interrupt Flag 1


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
.equ	ISC10	= 2	; Interrupt Sense Control 1 bit 1
.equ	ISC11	= 3	; Interrupt Sense Control 1 bit 1
.equ	SM1	= 4	; Sleep Mode Select
.equ	SE	= 5	; Sleep Enable
.equ	SRW10	= 6	; External SRAM Wait State Select
.equ	SRE	= 7	; External SRAM Enable

; MCUSR - MCU Status Register
.equ	PORF	= 0	; Power-on reset flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BORF	= 2	; Brown-out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag

; EMCUCR - Extended MCU Control Register
.equ	ISC2	= 0	; Interrupt Sense Control 2
.equ	SRW11	= 1	; Wait State Select Bit 1 for Upper Sector
.equ	SRW00	= 2	; Wait State Select Bit 0 for Lower Sector
.equ	SRW01	= 3	; Wait State Select Bit 1 for Lower Sector
.equ	SRL0	= 4	; Wait State Sector Limit Bit 0
.equ	SRL1	= 5	; Wait State Sector Limit Bit 1
.equ	SRL2	= 6	; Wait State Sector Limit Bit 2
.equ	SM0	= 7	; Sleep mode Select Bit 0

; SPMCR - Store Program Memory Control Register
.equ	SPMEN	= 0	; Store Program Memory Enable
.equ	PGERS	= 1	; Page Erase
.equ	PGWRT	= 2	; Page Write
.equ	BLBSET	= 3	; Boot Lock Bit Set


; ***** TIMER_COUNTER_0 **************
; TCCR0 - Timer/Counter Control Register
.equ	CS00	= 0	; Clock Select 1
.equ	CS01	= 1	; Clock Select 1
.equ	CS02	= 2	; Clock Select 2
.equ	WGM01	= 3	; Waveform Generation Mode 1
.equ	COM00	= 4	; Compare match Output Mode 0
.equ	COM01	= 5	; Compare Match Output Mode 1
.equ	WGM00	= 6	; Waveform Generation Mode 0
.equ	FOC0	= 7	; Force Output Compare

; TCNT0 - Timer/Counter Register
.equ	TCNT0_0	= 0	; 
.equ	TCNT0_1	= 1	; 
.equ	TCNT0_2	= 2	; 
.equ	TCNT0_3	= 3	; 
.equ	TCNT0_4	= 4	; 
.equ	TCNT0_5	= 5	; 
.equ	TCNT0_6	= 6	; 
.equ	TCNT0_7	= 7	; 

; OCR0 - Output Compare Register
.equ	OCR0_0	= 0	; 
.equ	OCR0_1	= 1	; 
.equ	OCR0_2	= 2	; 
.equ	OCR0_3	= 3	; 
.equ	OCR0_4	= 4	; 
.equ	OCR0_5	= 5	; 
.equ	OCR0_6	= 6	; 
.equ	OCR0_7	= 7	; 

; TIMSK - Timer/Counter Interrupt Mask Register
.equ	OCIE0	= 0	; Timer/Counter0 Output Compare Match Interrupt register
.equ	TOIE0	= 1	; Timer/Counter0 Overflow Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag register
.equ	OCF0	= 0	; Output Compare Flag 0
.equ	TOV0	= 1	; Timer/Counter0 Overflow Flag

; SFIOR - Special Function IO Register
.equ	PSR10	= 0	; Prescaler Reset Timer/Counter1 and Timer/Counter0


; ***** TIMER_COUNTER_2 **************
; TIMSK - Timer/Counter Interrupt Mask register
.equ	OCIE2	= 2	; Timer/Counter2 Output Compare Match Interrupt Enable
.equ	TOIE2	= 4	; Timer/Counter2 Overflow Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag Register
.equ	OCF2	= 2	; Output Compare Flag 2
.equ	TOV2	= 4	; Timer/Counter2 Overflow Flag

; TCCR2 - Timer/Counter2 Control Register
.equ	CS20	= 0	; Clock Select bit 0
.equ	CS21	= 1	; Clock Select bit 1
.equ	CS22	= 2	; Clock Select bit 2
.equ	CTC2	= 3	; Clear Timer/Counter2 on Compare Match
.equ	COM20	= 4	; Compare Output Mode bit 0
.equ	COM21	= 5	; Compare Output Mode bit 1
.equ	PWM2	= 6	; Pulse Width Modulator Enable
.equ	FOC2	= 7	; Force Output Compare

; TCNT2 - Timer/Counter2
.equ	TCNT2_0	= 0	; Timer/Counter 2 bit 0
.equ	TCNT2_1	= 1	; Timer/Counter 2 bit 1
.equ	TCNT2_2	= 2	; Timer/Counter 2 bit 2
.equ	TCNT2_3	= 3	; Timer/Counter 2 bit 3
.equ	TCNT2_4	= 4	; Timer/Counter 2 bit 4
.equ	TCNT2_5	= 5	; Timer/Counter 2 bit 5
.equ	TCNT2_6	= 6	; Timer/Counter 2 bit 6
.equ	TCNT2_7	= 7	; Timer/Counter 2 bit 7

; OCR2 - Timer/Counter2 Output Compare Register
.equ	OCR2_0	= 0	; Timer/Counter2 Output Compare Register Bit 0
.equ	OCR2_1	= 1	; Timer/Counter2 Output Compare Register Bit 1
.equ	OCR2_2	= 2	; Timer/Counter2 Output Compare Register Bit 2
.equ	OCR2_3	= 3	; Timer/Counter2 Output Compare Register Bit 3
.equ	OCR2_4	= 4	; Timer/Counter2 Output Compare Register Bit 4
.equ	OCR2_5	= 5	; Timer/Counter2 Output Compare Register Bit 5
.equ	OCR2_6	= 6	; Timer/Counter2 Output Compare Register Bit 6
.equ	OCR2_7	= 7	; Timer/Counter2 Output Compare Register Bit 7

; ASSR - Asynchronous Status Register
.equ	TCR2UB	= 0	; Timer/counter Control Register2 Update Busy
.equ	OCR2UB	= 1	; Output Compare Register2 Update Busy
.equ	TCN2UB	= 2	; Timer/Counter2 Update Busy
.equ	AS2	= 3	; Asynchronous Timer/counter2

; SFIOR - Specil Function IO Register
.equ	PSR2	= 1	; Prescaler Reset Timer/Counter2


; ***** TIMER_COUNTER_1 **************
; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TICIE1	= 3	; Timer/Counter1 Input Capture Interrupt Enable
.equ	OCIE1B	= 5	; Timer/Counter1 Output CompareB Match Interrupt Enable
.equ	OCIE1A	= 6	; Timer/Counter1 Output CompareA Match Interrupt Enable
.equ	TOIE1	= 7	; Timer/Counter1 Overflow Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag register
.equ	ICF1	= 3	; Input Capture Flag 1
.equ	OCF1B	= 5	; Output Compare Flag 1B
.equ	OCF1A	= 6	; Output Compare Flag 1A
.equ	TOV1	= 7	; Timer/Counter1 Overflow Flag

; TCCR1A - Timer/Counter1 Control Register A
.equ	WGM10	= 0	; Waveform Generation Mode
.equ	WGM11	= 1	; Waveform Generation Mode
.equ	FOC1B	= 2	; Force Output Compare 1B
.equ	FOC1A	= 3	; Force Output Compare 1A
.equ	COM1B0	= 4	; Compare Output Mode 1B, bit 0
.equ	COM1B1	= 5	; Compare Output Mode 1B, bit 1
.equ	COM1A0	= 6	; Comparet Ouput Mode 1A, bit 0
.equ	COM1A1	= 7	; Compare Output Mode 1A, bit 1

; TCCR1B - Timer/Counter1 Control Register B
.equ	CS10	= 0	; Prescaler source of Timer/Counter 1
.equ	CS11	= 1	; Prescaler source of Timer/Counter 1
.equ	CS12	= 2	; Prescaler source of Timer/Counter 1
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
.equ	SUT	= 3	; Start-up time
.equ	SPIEN	= 4	; Serial program downloading (SPI) enabled
.equ	BOOTRST	= 5	; Boot Reset Vector Enabled

; HIGH fuse bits

; EXTENDED fuse bits



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x1fff	; Note: Word address
.equ	IOEND	= 0x003f
.equ	SRAM_START	= 0x0060
.equ	SRAM_SIZE	= 1024
.equ	RAMEND	= 0x045f
.equ	XRAMEND	= 0xfbff
.equ	E2END	= 0x01ff
.equ	EEPROMEND	= 0x01ff
.equ	EEADRBITS	= 9
#pragma AVRPART MEMORY PROG_FLASH 16384
#pragma AVRPART MEMORY EEPROM 512
#pragma AVRPART MEMORY INT_SRAM SIZE 1024
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60



; ***** BOOTLOADER DECLARATIONS ******************************************
.equ	NRWW_START_ADDR	= 0x0
.equ	NRWW_STOP_ADDR	= 0x1fff
.equ	PAGESIZE	= 64
.equ	FIRSTBOOTSTART	= 0x1e00



; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0002	; External Interrupt 0
.equ	INT1addr	= 0x0004	; External Interrupt 1
.equ	INT2addr	= 0x0006	; External Interrupt 2
.equ	OC2addr	= 0x0008	; Timer/Counter2 Compare Match
.equ	OVF2addr	= 0x000a	; Timer/Counter2 Overflow
.equ	ICP1addr	= 0x000c	; Timer/Counter1 Capture Event
.equ	OC1Aaddr	= 0x000e	; Timer/Counter1 Compare Match A
.equ	OC1Baddr	= 0x0010	; Timer/Counter1 Compare Match B
.equ	OVF1addr	= 0x0012	; Timer/Counter1 Overflow
.equ	OC0addr	= 0x0014	; Timer/Counter0 Compare Match
.equ	OVF0addr	= 0x0016	; Timer/Counter0 Overflow
.equ	SPIaddr	= 0x0018	; Serial Transfer Complete
.equ	URXC0addr	= 0x001a	; UART0, Rx Complete
.equ	URXC1addr	= 0x001c	; UART1, Rx Complete
.equ	UDRE0addr	= 0x001e	; UART0 Data Register Empty
.equ	UDRE1addr	= 0x0020	; UART1 Data Register Empty
.equ	UTXC0addr	= 0x0022	; UART0, Tx Complete
.equ	UTXC1addr	= 0x0024	; UART1, Tx Complete
.equ	ERDYaddr	= 0x0026	; EEPROM Ready
.equ	ACIaddr	= 0x0028	; Analog Comparator

.equ	INT_VECTORS_SIZE	= 42	; size in words

#pragma AVRPART CORE INSTRUCTIONS_NOT_SUPPORTED break

#endif  /* _M161DEF_INC_ */

; ***** END OF FILE ******************************************************
