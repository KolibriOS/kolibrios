;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:31 ******* Source: ATtiny13.xml ************
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "tn13def.inc"
;* Title             : Register/Bit Definitions for the ATtiny13
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATtiny13
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

#ifndef _TN13DEF_INC_
#define _TN13DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATtiny13
#pragma AVRPART ADMIN PART_NAME ATtiny13
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x90
.equ	SIGNATURE_002	= 0x07

#pragma AVRPART CORE CORE_VERSION V2
#pragma AVRPART CORE NEW_INSTRUCTIONS lpm rd,z+


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	SREG	= 0x3f
.equ	SPL	= 0x3d
.equ	GIMSK	= 0x3b
.equ	GIFR	= 0x3a
.equ	TIMSK0	= 0x39
.equ	TIFR0	= 0x38
.equ	SPMCSR	= 0x37
.equ	OCR0A	= 0x36
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	TCCR0B	= 0x33
.equ	TCNT0	= 0x32
.equ	OSCCAL	= 0x31
.equ	TCCR0A	= 0x2f
.equ	DWDR	= 0x2e
.equ	OCR0B	= 0x29
.equ	GTCCR	= 0x28
.equ	CLKPR	= 0x26
.equ	WDTCR	= 0x21
.equ	EEAR	= 0x1e
.equ	EEDR	= 0x1d
.equ	EECR	= 0x1c
.equ	PORTB	= 0x18
.equ	DDRB	= 0x17
.equ	PINB	= 0x16
.equ	PCMSK	= 0x15
.equ	DIDR0	= 0x14
.equ	ACSR	= 0x08
.equ	ADMUX	= 0x07
.equ	ADCSRA	= 0x06
.equ	ADCH	= 0x05
.equ	ADCL	= 0x04
.equ	ADCSRB	= 0x03


; ***** BIT DEFINITIONS **************************************************

; ***** AD_CONVERTER *****************
; ADMUX - The ADC multiplexer Selection Register
.equ	MUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	MUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	ADLAR	= 5	; Left Adjust Result
.equ	REFS0	= 6	; Reference Selection Bit 0

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

; DIDR0 - Digital Input Disable Register 0
.equ	ADC1D	= 2	; ADC2 Digital input Disable
.equ	ADC3D	= 3	; ADC3 Digital input Disable
.equ	ADC2D	= 4	; ADC2 Digital input Disable
.equ	ADC0D	= 5	; ADC0 Digital input Disable


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


; ***** EEPROM ***********************
; EEAR - EEPROM Read/Write Access
.equ	EEARL	= EEAR	; For compatibility
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
.equ	EEPE	= EEWE	; For compatibility
.equ	EEMWE	= 2	; EEPROM Master Write Enable
.equ	EEMPE	= EEMWE	; For compatibility
.equ	EERIE	= 3	; EEProm Ready Interrupt Enable
.equ	EEPM0	= 4	; 
.equ	EEPM1	= 5	; 


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

; SPL - Stack Pointer Low Byte
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

; OSCCAL - Oscillator Calibration Register
.equ	CAL0	= 0	; Oscillatro Calibration Value Bit 0
.equ	CAL1	= 1	; Oscillatro Calibration Value Bit 1
.equ	CAL2	= 2	; Oscillatro Calibration Value Bit 2
.equ	CAL3	= 3	; Oscillatro Calibration Value Bit 3
.equ	CAL4	= 4	; Oscillatro Calibration Value Bit 4
.equ	CAL5	= 5	; Oscillatro Calibration Value Bit 5
.equ	CAL6	= 6	; Oscillatro Calibration Value Bit 6

; CLKPR - Clock Prescale Register
.equ	CLKPS0	= 0	; Clock Prescaler Select Bit 0
.equ	CLKPS1	= 1	; Clock Prescaler Select Bit 1
.equ	CLKPS2	= 2	; Clock Prescaler Select Bit 2
.equ	CLKPS3	= 3	; Clock Prescaler Select Bit 3
.equ	CLKPCE	= 7	; Clock Prescaler Change Enable

; DWDR - Debug Wire Data Register
.equ	DWDR0	= 0	; Debug Wire Data Register Bit 0
.equ	DWDR1	= 1	; Debug Wire Data Register Bit 1
.equ	DWDR2	= 2	; Debug Wire Data Register Bit 2
.equ	DWDR3	= 3	; Debug Wire Data Register Bit 3
.equ	DWDR4	= 4	; Debug Wire Data Register Bit 4
.equ	DWDR5	= 5	; Debug Wire Data Register Bit 5
.equ	DWDR6	= 6	; Debug Wire Data Register Bit 6
.equ	DWDR7	= 7	; Debug Wire Data Register Bit 7

; SPMCSR - Store Program Memory Control and Status Register
.equ	SPMEN	= 0	; Store program Memory Enable
.equ	PGERS	= 1	; Page Erase
.equ	PGWRT	= 2	; Page Write
.equ	RFLB	= 3	; Read Fuse and Lock Bits
.equ	CTPB	= 4	; Clear Temporary Page Buffer


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


; ***** EXTERNAL_INTERRUPT ***********
; MCUCR - MCU Control Register
;.equ	ISC00	= 0	; Interrupt Sense Control 0 Bit 0
;.equ	ISC01	= 1	; Interrupt Sense Control 0 Bit 1

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


; ***** TIMER_COUNTER_0 **************
; TIMSK0 - Timer/Counter0 Interrupt Mask Register
.equ	TOIE0	= 1	; Timer/Counter0 Overflow Interrupt Enable
.equ	OCIE0A	= 2	; Timer/Counter0 Output Compare Match A Interrupt Enable
.equ	OCIE0B	= 3	; Timer/Counter0 Output Compare Match B Interrupt Enable

; TIFR0 - Timer/Counter0 Interrupt Flag register
.equ	TOV0	= 1	; Timer/Counter0 Overflow Flag
.equ	OCF0A	= 2	; Timer/Counter0 Output Compare Flag 0A
.equ	OCF0B	= 3	; Timer/Counter0 Output Compare Flag 0B

; OCR0A - Timer/Counter0 Output Compare Register
.equ	OCR0_0	= 0	; 
.equ	OCR0_1	= 1	; 
.equ	OCR0_2	= 2	; 
.equ	OCR0_3	= 3	; 
.equ	OCR0_4	= 4	; 
.equ	OCR0_5	= 5	; 
.equ	OCR0_6	= 6	; 
.equ	OCR0_7	= 7	; 

; TCCR0A - Timer/Counter  Control Register A
.equ	WGM00	= 0	; Waveform Generation Mode
.equ	WGM01	= 1	; Waveform Generation Mode
.equ	COM0B0	= 4	; Compare Match Output B Mode
.equ	COM0B1	= 5	; Compare Match Output B Mode
.equ	COM0A0	= 6	; Compare Match Output A Mode
.equ	COM0A1	= 7	; Compare Match Output A Mode

; TCNT0 - Timer/Counter0
.equ	TCNT0_0	= 0	; 
.equ	TCNT0_1	= 1	; 
.equ	TCNT0_2	= 2	; 
.equ	TCNT0_3	= 3	; 
.equ	TCNT0_4	= 4	; 
.equ	TCNT0_5	= 5	; 
.equ	TCNT0_6	= 6	; 
.equ	TCNT0_7	= 7	; 

; TCCR0B - Timer/Counter Control Register B
.equ	CS00	= 0	; Clock Select
.equ	CS01	= 1	; Clock Select
.equ	CS02	= 2	; Clock Select
.equ	WGM02	= 3	; Waveform Generation Mode
.equ	FOC0B	= 6	; Force Output Compare B
.equ	FOC0A	= 7	; Force Output Compare A

; OCR0B - Timer/Counter0 Output Compare Register
;.equ	OCR0_0	= 0	; 
;.equ	OCR0_1	= 1	; 
;.equ	OCR0_2	= 2	; 
;.equ	OCR0_3	= 3	; 
;.equ	OCR0_4	= 4	; 
;.equ	OCR0_5	= 5	; 
;.equ	OCR0_6	= 6	; 
;.equ	OCR0_7	= 7	; 

; GTCCR - General Timer Conuter Register
.equ	PSR10	= 0	; Prescaler Reset Timer/Counter0
.equ	TSM	= 7	; Timer/Counter Synchronization Mode


; ***** WATCHDOG *********************
; WDTCR - Watchdog Timer Control Register
.equ	WDP0	= 0	; Watch Dog Timer Prescaler bit 0
.equ	WDP1	= 1	; Watch Dog Timer Prescaler bit 1
.equ	WDP2	= 2	; Watch Dog Timer Prescaler bit 2
.equ	WDE	= 3	; Watch Dog Enable
.equ	WDCE	= 4	; Watchdog Change Enable
.equ	WDP3	= 5	; Watchdog Timer Prescaler Bit 3
.equ	WDTIE	= 6	; Watchdog Timeout Interrupt Enable
.equ	WDTIF	= 7	; Watchdog Timeout Interrupt Flag



; ***** LOCKSBITS ********************************************************
.equ	LB1	= 0	; Lockbit
.equ	LB2	= 1	; Lockbit


; ***** FUSES ************************************************************
; LOW fuse bits
.equ	CKSEL0	= 0	; Select Clock Source
.equ	CKSEL1	= 1	; Select Clock Source
.equ	SUT0	= 2	; Select start-up time
.equ	SUT1	= 3	; Select start-up time
.equ	CKDIV8	= 4	; Start up with system clock divided by 8
.equ	WDTON	= 5	; Watch dog timer always on
.equ	EESAVE	= 6	; Keep EEprom contents during chip erase
.equ	SPIEN	= 7	; SPI programming enable

; HIGH fuse bits
.equ	RSTDISBL	= 0	; Disable external reset
.equ	BODLEVEL0	= 1	; Enable BOD and select level
.equ	BODLEVEL1	= 2	; Enable BOD and select level
.equ	DWEN	= 3	; DebugWire Enable
.equ	SELFPRGEN	= 4	; Self Programming Enable



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
.equ	SRAM_START	= 0x0060
.equ	SRAM_SIZE	= 64
.equ	RAMEND	= 0x009f
.equ	XRAMEND	= 0x0000
.equ	E2END	= 0x003f
.equ	EEPROMEND	= 0x003f
.equ	EEADRBITS	= 6
#pragma AVRPART MEMORY PROG_FLASH 1024
#pragma AVRPART MEMORY EEPROM 64
#pragma AVRPART MEMORY INT_SRAM SIZE 64
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60



; ***** BOOTLOADER DECLARATIONS ******************************************
.equ	PAGESIZE	= 16



; ***** INTERRUPT VECTORS ************************************************
.equ	INT0addr	= 0x0001	; External Interrupt 0
.equ	PCI0addr	= 0x0002	; External Interrupt Request 0
.equ	OVF0addr	= 0x0003	; Timer/Counter0 Overflow
.equ	ERDYaddr	= 0x0004	; EEPROM Ready
.equ	ACIaddr	= 0x0005	; Analog Comparator
.equ	OC0Aaddr	= 0x0006	; Timer/Counter Compare Match A
.equ	OC0Baddr	= 0x0007	; Timer/Counter Compare Match B
.equ	WDTaddr	= 0x0008	; Watchdog Time-out
.equ	ADCCaddr	= 0x0009	; ADC Conversion Complete

.equ	INT_VECTORS_SIZE	= 10	; size in words

#endif  /* _TN13DEF_INC_ */

; ***** END OF FILE ******************************************************
