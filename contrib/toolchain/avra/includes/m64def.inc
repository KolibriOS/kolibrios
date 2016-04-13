;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:31 ******* Source: ATmega64.xml ************
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "m64def.inc"
;* Title             : Register/Bit Definitions for the ATmega64
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATmega64
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

#ifndef _M64DEF_INC_
#define _M64DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATmega64
#pragma AVRPART ADMIN PART_NAME ATmega64
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x96
.equ	SIGNATURE_002	= 0x02

#pragma AVRPART CORE CORE_VERSION V2E


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	UCSR1C	= 0x9d	; MEMORY MAPPED
.equ	UDR1	= 0x9c	; MEMORY MAPPED
.equ	UCSR1A	= 0x9b	; MEMORY MAPPED
.equ	UCSR1B	= 0x9a	; MEMORY MAPPED
.equ	UBRR1L	= 0x99	; MEMORY MAPPED
.equ	UBRR1H	= 0x98	; MEMORY MAPPED
.equ	UCSR0C	= 0x95	; MEMORY MAPPED
.equ	UBRR0H	= 0x90	; MEMORY MAPPED
.equ	ADCSRB	= 0x8e	; MEMORY MAPPED
.equ	TCCR3C	= 0x8c	; MEMORY MAPPED
.equ	TCCR3A	= 0x8b	; MEMORY MAPPED
.equ	TCCR3B	= 0x8a	; MEMORY MAPPED
.equ	TCNT3H	= 0x89	; MEMORY MAPPED
.equ	TCNT3L	= 0x88	; MEMORY MAPPED
.equ	OCR3AH	= 0x87	; MEMORY MAPPED
.equ	OCR3AL	= 0x86	; MEMORY MAPPED
.equ	OCR3BH	= 0x85	; MEMORY MAPPED
.equ	OCR3BL	= 0x84	; MEMORY MAPPED
.equ	OCR3CH	= 0x83	; MEMORY MAPPED
.equ	OCR3CL	= 0x82	; MEMORY MAPPED
.equ	ICR3H	= 0x81	; MEMORY MAPPED
.equ	ICR3L	= 0x80	; MEMORY MAPPED
.equ	ETIMSK	= 0x7d	; MEMORY MAPPED
.equ	ETIFR	= 0x7c	; MEMORY MAPPED
.equ	TCCR1C	= 0x7a	; MEMORY MAPPED
.equ	OCR1CH	= 0x79	; MEMORY MAPPED
.equ	OCR1CL	= 0x78	; MEMORY MAPPED
.equ	TWCR	= 0x74	; MEMORY MAPPED
.equ	TWDR	= 0x73	; MEMORY MAPPED
.equ	TWAR	= 0x72	; MEMORY MAPPED
.equ	TWSR	= 0x71	; MEMORY MAPPED
.equ	TWBR	= 0x70	; MEMORY MAPPED
.equ	OSCCAL	= 0x6f	; MEMORY MAPPED
.equ	XMCRA	= 0x6d	; MEMORY MAPPED
.equ	XMCRB	= 0x6c	; MEMORY MAPPED
.equ	EICRA	= 0x6a	; MEMORY MAPPED
.equ	SPMCSR	= 0x68	; MEMORY MAPPED
.equ	PORTG	= 0x65	; MEMORY MAPPED
.equ	DDRG	= 0x64	; MEMORY MAPPED
.equ	PING	= 0x63	; MEMORY MAPPED
.equ	PORTF	= 0x62	; MEMORY MAPPED
.equ	DDRF	= 0x61	; MEMORY MAPPED
.equ	SREG	= 0x3f
.equ	SPH	= 0x3e
.equ	SPL	= 0x3d
.equ	XDIV	= 0x3c
.equ	EICRB	= 0x3a
.equ	EIMSK	= 0x39
.equ	EIFR	= 0x38
.equ	TIMSK	= 0x37
.equ	TIFR	= 0x36
.equ	MCUCR	= 0x35
.equ	MCUCSR	= 0x34
.equ	TCCR0	= 0x33
.equ	TCNT0	= 0x32
.equ	OCR0	= 0x31
.equ	ASSR	= 0x30
.equ	TCCR1A	= 0x2f
.equ	TCCR1B	= 0x2e
.equ	TCNT1H	= 0x2d
.equ	TCNT1L	= 0x2c
.equ	OCR1AH	= 0x2b
.equ	OCR1AL	= 0x2a
.equ	OCR1BH	= 0x29
.equ	OCR1BL	= 0x28
.equ	ICR1H	= 0x27
.equ	ICR1L	= 0x26
.equ	TCCR2	= 0x25
.equ	TCNT2	= 0x24
.equ	OCR2	= 0x23
.equ	OCDR	= 0x22
.equ	WDTCR	= 0x21
.equ	SFIOR	= 0x20
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
.equ	UBRR0L	= 0x09
.equ	ACSR	= 0x08
.equ	ADMUX	= 0x07
.equ	ADCSRA	= 0x06
.equ	ADCH	= 0x05
.equ	ADCL	= 0x04
.equ	PORTE	= 0x03
.equ	DDRE	= 0x02
.equ	PINE	= 0x01
.equ	PINF	= 0x00


; ***** BIT DEFINITIONS **************************************************

; ***** ANALOG_COMPARATOR ************
; SFIOR - Special Function IO Register
.equ	ACME	= 3	; Analog Comparator Multiplexer Enable

; ACSR - Analog Comparator Control And Status Register
.equ	ACIS0	= 0	; Analog Comparator Interrupt Mode Select bit 0
.equ	ACIS1	= 1	; Analog Comparator Interrupt Mode Select bit 1
.equ	ACIC	= 2	; Analog Comparator Input Capture Enable
.equ	ACIE	= 3	; Analog Comparator Interrupt Enable
.equ	ACI	= 4	; Analog Comparator Interrupt Flag
.equ	ACO	= 5	; Analog Compare Output
.equ	ACBG	= 6	; Analog Comparator Bandgap Select
.equ	ACD	= 7	; Analog Comparator Disable


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

; ADCSRA - The ADC Control and Status register A
.equ	ADCSR	= ADCSRA	; For compatibility
.equ	ADPS0	= 0	; ADC  Prescaler Select Bits
.equ	ADPS1	= 1	; ADC  Prescaler Select Bits
.equ	ADPS2	= 2	; ADC  Prescaler Select Bits
.equ	ADIE	= 3	; ADC Interrupt Enable
.equ	ADIF	= 4	; ADC Interrupt Flag
.equ	ADATE	= 5	; ADC  Auto Trigger Enable
.equ	ADFR	= ADATE	; For compatibility
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

; ADCSRB - The ADC Control and Status register B
.equ	ADTS0	= 0	; ADC Auto Trigger Source bit 0
.equ	ADTS1	= 1	; ADC Auto Trigger Source bit 1
.equ	ADTS2	= 2	; ADC Auto Trigger Source bit 2


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


; ***** TWI **************************
; TWBR - TWI Bit Rate register
.equ	I2BR	= TWBR	; For compatibility
.equ	TWBR0	= 0	; 
.equ	TWBR1	= 1	; 
.equ	TWBR2	= 2	; 
.equ	TWBR3	= 3	; 
.equ	TWBR4	= 4	; 
.equ	TWBR5	= 5	; 
.equ	TWBR6	= 6	; 
.equ	TWBR7	= 7	; 

; TWCR - TWI Control Register
.equ	I2CR	= TWCR	; For compatibility
.equ	TWIE	= 0	; TWI Interrupt Enable
.equ	I2IE	= TWIE	; For compatibility
.equ	TWEN	= 2	; TWI Enable Bit
.equ	I2EN	= TWEN	; For compatibility
.equ	ENI2C	= TWEN	; For compatibility
.equ	TWWC	= 3	; TWI Write Collition Flag
.equ	I2WC	= TWWC	; For compatibility
.equ	TWSTO	= 4	; TWI Stop Condition Bit
.equ	I2STO	= TWSTO	; For compatibility
.equ	TWSTA	= 5	; TWI Start Condition Bit
.equ	I2STA	= TWSTA	; For compatibility
.equ	TWEA	= 6	; TWI Enable Acknowledge Bit
.equ	I2EA	= TWEA	; For compatibility
.equ	TWINT	= 7	; TWI Interrupt Flag
.equ	I2INT	= TWINT	; For compatibility

; TWSR - TWI Status Register
.equ	I2SR	= TWSR	; For compatibility
.equ	TWPS0	= 0	; TWI Prescaler
.equ	TWS0	= TWPS0	; For compatibility
.equ	I2GCE	= TWPS0	; For compatibility
.equ	TWPS1	= 1	; TWI Prescaler
.equ	TWS1	= TWPS1	; For compatibility
.equ	TWS3	= 3	; TWI Status
.equ	I2S3	= TWS3	; For compatibility
.equ	TWS4	= 4	; TWI Status
.equ	I2S4	= TWS4	; For compatibility
.equ	TWS5	= 5	; TWI Status
.equ	I2S5	= TWS5	; For compatibility
.equ	TWS6	= 6	; TWI Status
.equ	I2S6	= TWS6	; For compatibility
.equ	TWS7	= 7	; TWI Status
.equ	I2S7	= TWS7	; For compatibility

; TWDR - TWI Data register
.equ	I2DR	= TWDR	; For compatibility
.equ	TWD0	= 0	; TWI Data Register Bit 0
.equ	TWD1	= 1	; TWI Data Register Bit 1
.equ	TWD2	= 2	; TWI Data Register Bit 2
.equ	TWD3	= 3	; TWI Data Register Bit 3
.equ	TWD4	= 4	; TWI Data Register Bit 4
.equ	TWD5	= 5	; TWI Data Register Bit 5
.equ	TWD6	= 6	; TWI Data Register Bit 6
.equ	TWD7	= 7	; TWI Data Register Bit 7

; TWAR - TWI (Slave) Address register
.equ	I2AR	= TWAR	; For compatibility
.equ	TWGCE	= 0	; TWI General Call Recognition Enable Bit
.equ	TWA0	= 1	; TWI (Slave) Address register Bit 0
.equ	TWA1	= 2	; TWI (Slave) Address register Bit 1
.equ	TWA2	= 3	; TWI (Slave) Address register Bit 2
.equ	TWA3	= 4	; TWI (Slave) Address register Bit 3
.equ	TWA4	= 5	; TWI (Slave) Address register Bit 4
.equ	TWA5	= 6	; TWI (Slave) Address register Bit 5
.equ	TWA6	= 7	; TWI (Slave) Address register Bit 6


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
.equ	UPE0	= 2	; Parity Error
.equ	DOR0	= 3	; Data overRun
.equ	FE0	= 4	; Framing Error
.equ	UDRE0	= 5	; USART Data Register Empty
.equ	TXC0	= 6	; USART Transmitt Complete
.equ	RXC0	= 7	; USART Receive Complete

; UCSR0B - USART Control and Status Register B
.equ	TXB80	= 0	; Transmit Data Bit 8
.equ	RXB80	= 1	; Receive Data Bit 8
.equ	UCSZ02	= 2	; Character Size
.equ	UCSZ2	= UCSZ02	; For compatibility
.equ	TXEN0	= 3	; Transmitter Enable
.equ	RXEN0	= 4	; Receiver Enable
.equ	UDRIE0	= 5	; USART Data register Empty Interrupt Enable
.equ	TXCIE0	= 6	; TX Complete Interrupt Enable
.equ	RXCIE0	= 7	; RX Complete Interrupt Enable

; UCSR0C - USART Control and Status Register C
.equ	UCPOL0	= 0	; Clock Polarity
.equ	UCSZ00	= 1	; Character Size
.equ	UCSZ01	= 2	; Character Size
.equ	USBS0	= 3	; Stop Bit Select
.equ	UPM00	= 4	; Parity Mode Bit 0
.equ	UPM01	= 5	; Parity Mode Bit 1
.equ	UMSEL0	= 6	; USART Mode Select


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
.equ	UPE1	= 2	; Parity Error
.equ	DOR1	= 3	; Data overRun
.equ	FE1	= 4	; Framing Error
.equ	UDRE1	= 5	; USART Data Register Empty
.equ	TXC1	= 6	; USART Transmitt Complete
.equ	RXC1	= 7	; USART Receive Complete

; UCSR1B - USART Control and Status Register B
.equ	TXB81	= 0	; Transmit Data Bit 8
.equ	RXB81	= 1	; Receive Data Bit 8
.equ	UCSZ12	= 2	; Character Size
.equ	TXEN1	= 3	; Transmitter Enable
.equ	RXEN1	= 4	; Receiver Enable
.equ	UDRIE1	= 5	; USART Data register Empty Interrupt Enable
.equ	TXCIE1	= 6	; TX Complete Interrupt Enable
.equ	RXCIE1	= 7	; RX Complete Interrupt Enable

; UCSR1C - USART Control and Status Register C
.equ	UCPOL1	= 0	; Clock Polarity
.equ	UCSZ10	= 1	; Character Size
.equ	UCSZ11	= 2	; Character Size
.equ	USBS1	= 3	; Stop Bit Select
.equ	UPM10	= 4	; Parity Mode Bit 0
.equ	UPM11	= 5	; Parity Mode Bit 1
.equ	UMSEL1	= 6	; USART Mode Select


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
.equ	IVCE	= 0	; Interrupt Vector Change Enable
.equ	IVSEL	= 1	; Interrupt Vector Select
.equ	SM2	= 2	; Sleep Mode Select
.equ	SM0	= 3	; Sleep Mode Select
.equ	SM1	= 4	; Sleep Mode Select
.equ	SE	= 5	; Sleep Enable
.equ	SRW10	= 6	; External SRAM Wait State Select
.equ	SRE	= 7	; External SRAM Enable

; XMCRA - External Memory Control Register A
.equ	SRW11	= 1	; Wait state select bit upper page
.equ	SRW00	= 2	; Wait state select bit lower page
.equ	SRW01	= 3	; Wait state select bit lower page
.equ	SRL0	= 4	; Wait state page limit
.equ	SRL1	= 5	; Wait state page limit
.equ	SRL2	= 6	; Wait state page limit

; XMCRB - External Memory Control Register B
.equ	XMM0	= 0	; External Memory High Mask
.equ	XMM1	= 1	; External Memory High Mask
.equ	XMM2	= 2	; External Memory High Mask
.equ	XMBK	= 7	; External Memory Bus Keeper Enable

; OSCCAL - Oscillator Calibration Value
.equ	CAL0	= 0	; Oscillator Calibration Value
.equ	CAL1	= 1	; Oscillator Calibration Value
.equ	CAL2	= 2	; Oscillator Calibration Value
.equ	CAL3	= 3	; Oscillator Calibration Value
.equ	CAL4	= 4	; Oscillator Calibration Value
.equ	CAL5	= 5	; Oscillator Calibration Value
.equ	CAL6	= 6	; Oscillator Calibration Value
.equ	CAL7	= 7	; Oscillator Calibration Value

; XDIV - XTAL Divide Control Register
.equ	XDIV0	= 0	; XTAl Divide Select Bit 0
.equ	XDIV1	= 1	; XTAl Divide Select Bit 1
.equ	XDIV2	= 2	; XTAl Divide Select Bit 2
.equ	XDIV3	= 3	; XTAl Divide Select Bit 3
.equ	XDIV4	= 4	; XTAl Divide Select Bit 4
.equ	XDIV5	= 5	; XTAl Divide Select Bit 5
.equ	XDIV6	= 6	; XTAl Divide Select Bit 6
.equ	XDIVEN	= 7	; XTAL Divide Enable

; MCUCSR - MCU Control And Status Register
.equ	PORF	= 0	; Power-on reset flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BORF	= 2	; Brown-out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag
.equ	JTRF	= 4	; JTAG Reset Flag
.equ	JTD	= 7	; JTAG Interface Disable


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


; ***** JTAG *************************
; OCDR - On-Chip Debug Related Register in I/O Memory
.equ	OCDR0	= 0	; On-Chip Debug Register Bit 0
.equ	OCDR1	= 1	; On-Chip Debug Register Bit 1
.equ	OCDR2	= 2	; On-Chip Debug Register Bit 2
.equ	OCDR3	= 3	; On-Chip Debug Register Bit 3
.equ	OCDR4	= 4	; On-Chip Debug Register Bit 4
.equ	OCDR5	= 5	; On-Chip Debug Register Bit 5
.equ	OCDR6	= 6	; On-Chip Debug Register Bit 6
.equ	OCDR7	= 7	; On-Chip Debug Register Bit 7
.equ	IDRD	= OCDR7	; For compatibility

; MCUCSR - MCU Control And Status Register
;.equ	JTRF	= 4	; JTAG Reset Flag
;.equ	JTD	= 7	; JTAG Interface Disable


; ***** MISC *************************
; SFIOR - Special Function IO Register
.equ	PSR321	= 0	; Prescaler Reset Timer/Counter3, Timer/Counter2, and Timer/Counter1
.equ	PSR1	= PSR321	; For compatibility
.equ	PSR2	= PSR321	; For compatibility
.equ	PSR3	= PSR321	; For compatibility
.equ	PSR0	= 1	; Prescaler Reset Timer/Counter0
.equ	PUD	= 2	; Pull Up Disable
;.equ	ACME	= 3	; Analog Comparator Multiplexer Enable
.equ	TSM	= 7	; Timer/Counter Synchronization Mode


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

; EICRB - External Interrupt Control Register B
.equ	ISC40	= 0	; External Interrupt 7-4 Sense Control Bit
.equ	ISC41	= 1	; External Interrupt 7-4 Sense Control Bit
.equ	ISC50	= 2	; External Interrupt 7-4 Sense Control Bit
.equ	ISC51	= 3	; External Interrupt 7-4 Sense Control Bit
.equ	ISC60	= 4	; External Interrupt 7-4 Sense Control Bit
.equ	ISC61	= 5	; External Interrupt 7-4 Sense Control Bit
.equ	ISC70	= 6	; External Interrupt 7-4 Sense Control Bit
.equ	ISC71	= 7	; External Interrupt 7-4 Sense Control Bit

; EIMSK - External Interrupt Mask Register
.equ	GICR	= EIMSK	; For compatibility
.equ	GIMSK	= EIMSK	; For compatibility
.equ	INT0	= 0	; External Interrupt Request 0 Enable
.equ	INT1	= 1	; External Interrupt Request 1 Enable
.equ	INT2	= 2	; External Interrupt Request 2 Enable
.equ	INT3	= 3	; External Interrupt Request 3 Enable
.equ	INT4	= 4	; External Interrupt Request 4 Enable
.equ	INT5	= 5	; External Interrupt Request 5 Enable
.equ	INT6	= 6	; External Interrupt Request 6 Enable
.equ	INT7	= 7	; External Interrupt Request 7 Enable

; EIFR - External Interrupt Flag Register
.equ	GIFR	= EIFR	; For compatibility
.equ	INTF0	= 0	; External Interrupt Flag 0
.equ	INTF1	= 1	; External Interrupt Flag 1
.equ	INTF2	= 2	; External Interrupt Flag 2
.equ	INTF3	= 3	; External Interrupt Flag 3
.equ	INTF4	= 4	; External Interrupt Flag 4
.equ	INTF5	= 5	; External Interrupt Flag 5
.equ	INTF6	= 6	; External Interrupt Flag 6
.equ	INTF7	= 7	; External Interrupt Flag 7


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
; PORTE - Data Register, Port E
.equ	PORTE0	= 0	; 
.equ	PE0	= 0	; For compatibility
.equ	PORTE1	= 1	; 
.equ	PE1	= 1	; For compatibility
.equ	PORTE2	= 2	; 
.equ	PE2	= 2	; For compatibility
.equ	PORTE3	= 3	; 
.equ	PE3	= 3	; For compatibility
.equ	PORTE4	= 4	; 
.equ	PE4	= 4	; For compatibility
.equ	PORTE5	= 5	; 
.equ	PE5	= 5	; For compatibility
.equ	PORTE6	= 6	; 
.equ	PE6	= 6	; For compatibility
.equ	PORTE7	= 7	; 
.equ	PE7	= 7	; For compatibility

; DDRE - Data Direction Register, Port E
.equ	DDE0	= 0	; 
.equ	DDE1	= 1	; 
.equ	DDE2	= 2	; 
.equ	DDE3	= 3	; 
.equ	DDE4	= 4	; 
.equ	DDE5	= 5	; 
.equ	DDE6	= 6	; 
.equ	DDE7	= 7	; 

; PINE - Input Pins, Port E
.equ	PINE0	= 0	; 
.equ	PINE1	= 1	; 
.equ	PINE2	= 2	; 
.equ	PINE3	= 3	; 
.equ	PINE4	= 4	; 
.equ	PINE5	= 5	; 
.equ	PINE6	= 6	; 
.equ	PINE7	= 7	; 


; ***** PORTF ************************
; PORTF - Data Register, Port F
.equ	PORTF0	= 0	; 
.equ	PF0	= 0	; For compatibility
.equ	PORTF1	= 1	; 
.equ	PF1	= 1	; For compatibility
.equ	PORTF2	= 2	; 
.equ	PF2	= 2	; For compatibility
.equ	PORTF3	= 3	; 
.equ	PF3	= 3	; For compatibility
.equ	PORTF4	= 4	; 
.equ	PF4	= 4	; For compatibility
.equ	PORTF5	= 5	; 
.equ	PF5	= 5	; For compatibility
.equ	PORTF6	= 6	; 
.equ	PF6	= 6	; For compatibility
.equ	PORTF7	= 7	; 
.equ	PF7	= 7	; For compatibility

; DDRF - Data Direction Register, Port F
.equ	DDF0	= 0	; 
.equ	DDF1	= 1	; 
.equ	DDF2	= 2	; 
.equ	DDF3	= 3	; 
.equ	DDF4	= 4	; 
.equ	DDF5	= 5	; 
.equ	DDF6	= 6	; 
.equ	DDF7	= 7	; 

; PINF - Input Pins, Port F
.equ	PINF0	= 0	; 
.equ	PINF1	= 1	; 
.equ	PINF2	= 2	; 
.equ	PINF3	= 3	; 
.equ	PINF4	= 4	; 
.equ	PINF5	= 5	; 
.equ	PINF6	= 6	; 
.equ	PINF7	= 7	; 


; ***** PORTG ************************
; PORTG - Data Register, Port G
.equ	PORTG0	= 0	; 
.equ	PG0	= 0	; For compatibility
.equ	PORTG1	= 1	; 
.equ	PG1	= 1	; For compatibility
.equ	PORTG2	= 2	; 
.equ	PG2	= 2	; For compatibility
.equ	PORTG3	= 3	; 
.equ	PG3	= 3	; For compatibility
.equ	PORTG4	= 4	; 
.equ	PG4	= 4	; For compatibility

; DDRG - Data Direction Register, Port G
.equ	DDG0	= 0	; 
.equ	DDG1	= 1	; 
.equ	DDG2	= 2	; 
.equ	DDG3	= 3	; 
.equ	DDG4	= 4	; 

; PING - Input Pins, Port G
.equ	PING0	= 0	; 
.equ	PING1	= 1	; 
.equ	PING2	= 2	; 
.equ	PING3	= 3	; 
.equ	PING4	= 4	; 


; ***** TIMER_COUNTER_0 **************
; TCCR0 - Timer/Counter Control Register
.equ	CS00	= 0	; Clock Select 0
.equ	CS01	= 1	; Clock Select 1
.equ	CS02	= 2	; Clock Select 2
.equ	WGM01	= 3	; Waveform Generation Mode 1
.equ	CTC0	= WGM01	; For compatibility
.equ	COM00	= 4	; Compare match Output Mode 0
.equ	COM01	= 5	; Compare Match Output Mode 1
.equ	WGM00	= 6	; Waveform Generation Mode 0
.equ	PWM0	= WGM00	; For compatibility
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

; ASSR - Asynchronus Status Register
.equ	TCR0UB	= 0	; Timer/Counter Control Register 0 Update Busy
.equ	OCR0UB	= 1	; Output Compare register 0 Busy
.equ	TCN0UB	= 2	; Timer/Counter0 Update Busy
.equ	AS0	= 3	; Asynchronus Timer/Counter 0

; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE0	= 0	; Timer/Counter0 Overflow Interrupt Enable
.equ	OCIE0	= 1	; Timer/Counter0 Output Compare Match Interrupt register

; TIFR - Timer/Counter Interrupt Flag register
.equ	TOV0	= 0	; Timer/Counter0 Overflow Flag
.equ	OCF0	= 1	; Output Compare Flag 0

; SFIOR - Special Function IO Register
;.equ	PSR0	= 1	; Prescaler Reset Timer/Counter0
;.equ	TSM	= 7	; Timer/Counter Synchronization Mode


; ***** TIMER_COUNTER_1 **************
; TIMSK - Timer/Counter Interrupt Mask Register
.equ	TOIE1	= 2	; Timer/Counter1 Overflow Interrupt Enable
.equ	OCIE1B	= 3	; Timer/Counter1 Output CompareB Match Interrupt Enable
.equ	OCIE1A	= 4	; Timer/Counter1 Output CompareA Match Interrupt Enable
.equ	TICIE1	= 5	; Timer/Counter1 Input Capture Interrupt Enable

; ETIMSK - Extended Timer/Counter Interrupt Mask Register
.equ	OCIE1C	= 0	; Timer/Counter 1, Output Compare Match C Interrupt Enable

; TIFR - Timer/Counter Interrupt Flag register
.equ	TOV1	= 2	; Timer/Counter1 Overflow Flag
.equ	OCF1B	= 3	; Output Compare Flag 1B
.equ	OCF1A	= 4	; Output Compare Flag 1A
.equ	ICF1	= 5	; Input Capture Flag 1

; ETIFR - Extended Timer/Counter Interrupt Flag register
.equ	OCF1C	= 0	; Timer/Counter 1, Output Compare C Match Flag

; SFIOR - Special Function IO Register
;.equ	PSR321	= 0	; Prescaler Reset, T/C3, T/C2, T/C1
;.equ	TSM	= 7	; Timer/Counter Synchronization Mode

; TCCR1A - Timer/Counter1 Control Register A
.equ	WGM10	= 0	; Waveform Generation Mode Bit 0
.equ	PWM10	= WGM10	; For compatibility
.equ	WGM11	= 1	; Waveform Generation Mode Bit 1
.equ	PWM11	= WGM11	; For compatibility
.equ	COM1C0	= 2	; Compare Output Mode 1C, bit 0
.equ	COM1C1	= 3	; Compare Output Mode 1C, bit 1
.equ	COM1B0	= 4	; Compare Output Mode 1B, bit 0
.equ	COM1B1	= 5	; Compare Output Mode 1B, bit 1
.equ	COM1A0	= 6	; Compare Ouput Mode 1A, bit 0
.equ	COM1A1	= 7	; Compare Output Mode 1A, bit 1

; TCCR1B - Timer/Counter1 Control Register B
.equ	CS10	= 0	; Clock Select bit 0
.equ	CS11	= 1	; Clock Select 1 bit 1
.equ	CS12	= 2	; Clock Select1 bit 2
.equ	WGM12	= 3	; Waveform Generation Mode
.equ	CTC10	= WGM12	; For compatibility
.equ	WGM13	= 4	; Waveform Generation Mode
.equ	CTC11	= WGM13	; For compatibility
.equ	ICES1	= 6	; Input Capture 1 Edge Select
.equ	ICNC1	= 7	; Input Capture 1 Noise Canceler

; TCCR1C - Timer/Counter1 Control Register C
.equ	FOC1C	= 5	; Force Output Compare for channel C
.equ	FOC1B	= 6	; Force Output Compare for channel B
.equ	FOC1A	= 7	; Force Output Compare for channel A


; ***** TIMER_COUNTER_2 **************
; TCCR2 - Timer/Counter Control Register
.equ	CS20	= 0	; Clock Select
.equ	CS21	= 1	; Clock Select
.equ	CS22	= 2	; Clock Select
.equ	WGM21	= 3	; Waveform Generation Mode
.equ	CTC2	= WGM21	; For compatibility
.equ	COM20	= 4	; Compare Match Output Mode
.equ	COM21	= 5	; Compare Match Output Mode
.equ	WGM20	= 6	; Wafeform Generation Mode
.equ	PWM2	= WGM20	; For compatibility
.equ	FOC2	= 7	; Force Output Compare

; TCNT2 - Timer/Counter Register
.equ	TCNT2_0	= 0	; Timer/Counter Register Bit 0
.equ	TCNT2_1	= 1	; Timer/Counter Register Bit 1
.equ	TCNT2_2	= 2	; Timer/Counter Register Bit 2
.equ	TCNT2_3	= 3	; Timer/Counter Register Bit 3
.equ	TCNT2_4	= 4	; Timer/Counter Register Bit 4
.equ	TCNT2_5	= 5	; Timer/Counter Register Bit 5
.equ	TCNT2_6	= 6	; Timer/Counter Register Bit 6
.equ	TCNT2_7	= 7	; Timer/Counter Register Bit 7

; OCR2 - Output Compare Register
.equ	OCR2_0	= 0	; Output Compare Register Bit 0
.equ	OCR2_1	= 1	; Output Compare Register Bit 1
.equ	OCR2_2	= 2	; Output Compare Register Bit 2
.equ	OCR2_3	= 3	; Output Compare Register Bit 3
.equ	OCR2_4	= 4	; Output Compare Register Bit 4
.equ	OCR2_5	= 5	; Output Compare Register Bit 5
.equ	OCR2_6	= 6	; Output Compare Register Bit 6
.equ	OCR2_7	= 7	; Output Compare Register Bit 7

; TIMSK - 
.equ	TOIE2	= 6	; 
.equ	OCIE2	= 7	; 

; TIFR - Timer/Counter Interrupt Flag Register
.equ	TOV2	= 6	; Timer/Counter2 Overflow Flag
.equ	OCF2	= 7	; Output Compare Flag 2


; ***** TIMER_COUNTER_3 **************
; ETIMSK - Extended Timer/Counter Interrupt Mask Register
.equ	OCIE3C	= 1	; Timer/Counter3, Output Compare Match Interrupt Enable
.equ	TOIE3	= 2	; Timer/Counter3 Overflow Interrupt Enable
.equ	OCIE3B	= 3	; Timer/Counter3 Output CompareB Match Interrupt Enable
.equ	OCIE3A	= 4	; Timer/Counter3 Output CompareA Match Interrupt Enable
.equ	TICIE3	= 5	; Timer/Counter3 Input Capture Interrupt Enable

; ETIFR - Extended Timer/Counter Interrupt Flag register
.equ	OCF3C	= 1	; Timer/Counter3 Output Compare C Match Flag
.equ	TOV3	= 2	; Timer/Counter3 Overflow Flag
.equ	OCF3B	= 3	; Output Compare Flag 1B
.equ	OCF3A	= 4	; Output Compare Flag 1A
.equ	ICF3	= 5	; Input Capture Flag 1

; SFIOR - Special Function IO Register
;.equ	PSR321	= 0	; Prescaler Reset, T/C3, T/C2, T/C1
;.equ	PSR1	= PSR321	; For compatibility
;.equ	PSR2	= PSR321	; For compatibility
;.equ	TSM	= 7	; Timer/Counter Synchronization Mode

; TCCR3A - Timer/Counter3 Control Register A
.equ	WGM30	= 0	; Waveform Generation Mode Bit 0
.equ	PWM30	= WGM30	; For compatibility
.equ	WGM31	= 1	; Waveform Generation Mode Bit 1
.equ	PWM31	= WGM31	; For compatibility
.equ	COM3C0	= 2	; Compare Output Mode 3C, bit 0
.equ	COM3C1	= 3	; Compare Output Mode 3C, bit 1
.equ	COM3B0	= 4	; Compare Output Mode 3B, bit 0
.equ	COM3B1	= 5	; Compare Output Mode 3B, bit 1
.equ	COM3A0	= 6	; Comparet Ouput Mode 3A, bit 0
.equ	COM3A1	= 7	; Compare Output Mode 3A, bit 1

; TCCR3B - Timer/Counter3 Control Register B
.equ	CS30	= 0	; Clock Select 3 bit 0
.equ	CS31	= 1	; Clock Select 3 bit 1
.equ	CS32	= 2	; Clock Select3 bit 2
.equ	WGM32	= 3	; Waveform Generation Mode
.equ	CTC30	= WGM32	; For compatibility
.equ	WGM33	= 4	; Waveform Generation Mode
.equ	CTC31	= WGM33	; For compatibility
.equ	ICES3	= 6	; Input Capture 3 Edge Select
.equ	ICNC3	= 7	; Input Capture 3  Noise Canceler

; TCCR3C - Timer/Counter3 Control Register C
.equ	FOC3C	= 5	; Force Output Compare for channel C
.equ	FOC3B	= 6	; Force Output Compare for channel B
.equ	FOC3A	= 7	; Force Output Compare for channel A


; ***** WATCHDOG *********************
; WDTCR - Watchdog Timer Control Register
.equ	WDTCSR	= WDTCR	; For compatibility
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDCE	= 4	; Watchdog Change Enable
.equ	WDTOE	= WDCE	; For compatibility



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
.equ	BODEN	= 6	; Brown out detector enable
.equ	BODLEVEL	= 7	; Brown out detector trigger level

; HIGH fuse bits
.equ	BOOTRST	= 0	; Select Reset Vector
.equ	BOOTSZ0	= 1	; Select Boot Size
.equ	BOOTSZ1	= 2	; Select Boot Size
.equ	EESAVE	= 3	; EEPROM memory is preserved through chip erase
.equ	CKOPT	= 4	; Oscillator Options
.equ	SPIEN	= 5	; Enable Serial programming and Data Downloading
.equ	JTAGEN	= 6	; Enable JTAG
.equ	OCDEN	= 7	; Enable OCD

; EXTENDED fuse bits
.equ	WDTON	= 0	; Watchdog timer always on
.equ	CompMode	= 1	; Compabillity mode



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x7fff	; Note: Word address
.equ	IOEND	= 0x00ff
.equ	SRAM_START	= 0x0100
.equ	SRAM_SIZE	= 4096
.equ	RAMEND	= 0x10ff
.equ	XRAMEND	= 0xffff
.equ	E2END	= 0x07ff
.equ	EEPROMEND	= 0x07ff
.equ	EEADRBITS	= 11
#pragma AVRPART MEMORY PROG_FLASH 65536
#pragma AVRPART MEMORY EEPROM 2048
#pragma AVRPART MEMORY INT_SRAM SIZE 4096
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100



; ***** BOOTLOADER DECLARATIONS ******************************************
.equ	NRWW_START_ADDR	= 0x7000
.equ	NRWW_STOP_ADDR	= 0x7fff
.equ	RWW_START_ADDR	= 0x0
.equ	RWW_STOP_ADDR	= 0x6fff
.equ	PAGESIZE	= 128
.equ	FIRSTBOOTSTART	= 0x7e00
.equ	SECONDBOOTSTART	= 0x7c00
.equ	THIRDBOOTSTART	= 0x7800
.equ	FOURTHBOOTSTART	= 0x7000
.equ	SMALLBOOTSTART	= FIRSTBOOTSTART
.equ	LARGEBOOTSTART	= FOURTHBOOTSTART



; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0002	; External Interrupt Request 0
.equ	INT1addr	= 0x0004	; External Interrupt Request 1
.equ	INT2addr	= 0x0006	; External Interrupt Request 2
.equ	INT3addr	= 0x0008	; External Interrupt Request 3
.equ	INT4addr	= 0x000a	; External Interrupt Request 4
.equ	INT5addr	= 0x000c	; External Interrupt Request 5
.equ	INT6addr	= 0x000e	; External Interrupt Request 6
.equ	INT7addr	= 0x0010	; External Interrupt Request 7
.equ	OC2addr	= 0x0012	; Timer/Counter2 Compare Match
.equ	OVF2addr	= 0x0014	; Timer/Counter2 Overflow
.equ	ICP1addr	= 0x0016	; Timer/Counter1 Capture Event
.equ	OC1Aaddr	= 0x0018	; Timer/Counter1 Compare Match A
.equ	OC1Baddr	= 0x001a	; Timer/Counter Compare Match B
.equ	OVF1addr	= 0x001c	; Timer/Counter1 Overflow
.equ	OC0addr	= 0x001e	; Timer/Counter0 Compare Match
.equ	OVF0addr	= 0x0020	; Timer/Counter0 Overflow
.equ	SPIaddr	= 0x0022	; SPI Serial Transfer Complete
.equ	URXC0addr	= 0x0024	; USART0, Rx Complete
.equ	UDRE0addr	= 0x0026	; USART0 Data Register Empty
.equ	UTXC0addr	= 0x0028	; USART0, Tx Complete
.equ	ADCCaddr	= 0x002a	; ADC Conversion Complete
.equ	ERDYaddr	= 0x002c	; EEPROM Ready
.equ	ACIaddr	= 0x002e	; Analog Comparator
.equ	OC1Caddr	= 0x0030	; Timer/Counter1 Compare Match C
.equ	ICP3addr	= 0x0032	; Timer/Counter3 Capture Event
.equ	OC3Aaddr	= 0x0034	; Timer/Counter3 Compare Match A
.equ	OC3Baddr	= 0x0036	; Timer/Counter3 Compare Match B
.equ	OC3Caddr	= 0x0038	; Timer/Counter3 Compare Match C
.equ	OVF3addr	= 0x003a	; Timer/Counter3 Overflow
.equ	URXC1addr	= 0x003c	; USART1, Rx Complete
.equ	UDRE1addr	= 0x003e	; USART1, Data Register Empty
.equ	UTXC1addr	= 0x0040	; USART1, Tx Complete
.equ	TWIaddr	= 0x0042	; 2-wire Serial Interface
.equ	SPMRaddr	= 0x0044	; Store Program Memory Read

.equ	INT_VECTORS_SIZE	= 70	; size in words

#endif  /* _M64DEF_INC_ */

; ***** END OF FILE ******************************************************
