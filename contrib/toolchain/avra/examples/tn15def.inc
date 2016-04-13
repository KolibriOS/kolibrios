.NOLIST

;***************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number		:AVR000
;* File Name		:"tn15def.inc"
;* Title		:Register/Bit Definitions for the ATtiny15
;* Date                 :99.07.05
;* Version              :1.00
;* Support telephone	:+47 72 88 87 20 (ATMEL Norway)
;* Support fax		:+47 72 88 87 18 (ATMEL Norway)
;* Support E-mail	:avr@atmel.com
;* Target MCU		:ATtiny15
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
;* in	r16,PORTB		;read PORTB latch
;* sbr	r16,(1<<PB6)+(1<<PB5)	;set PB6 and PB5 (use masks, not bit#)
;* out  PORTB,r16		;output to PORTB
;*
;* in	r16,TIFR		;read the Timer Interrupt Flag Register
;* sbrc	r16,TOV0		;test the overflow flag (use bit#)
;* rjmp	TOV0_is_set		;jump if set
;* ...				;otherwise do something else
;***************************************************************************

;***** Specify Device
.device ATtiny15

;***** I/O Register Definitions
.equ	SREG	=$3f
.equ	GIMSK	=$3b
.equ	GIFR	=$3a
.equ	TIMSK	=$39
.equ	TIFR	=$38
.equ	MCUCR	=$35
.equ	MCUSR	=$34
.equ	TCCR0	=$33
.equ	TCNT0	=$32
.equ	OSCCAL	=$31
.equ	TCCR1	=$30
.equ	TCNT1	=$2f
.equ	OCR1A	=$2e
.equ	OCR1B	=$2d
.equ	SFIOR	=$2c
.equ	WDTCR	=$21
.equ	EEAR	=$1e
.equ	EEDR	=$1d
.equ	EECR	=$1c
.equ	PORTB	=$18
.equ	DDRB	=$17
.equ	PINB	=$16
.equ	ACSR	=$08
.equ    ADMUX   =$07
.equ    ADCSR   =$06
.equ    ADCH    =$05
.equ    ADCL    =$04


;***** Bit Definitions
.equ	INT0	=6
.equ	PCIE	=5

.equ	INTF0	=6
.equ	PCIF	=5

.equ	OCIE1	=6
.equ	TOIE1	=2
.equ	TOIE0	=1

.equ	OCF1A	=6
.equ	TOV1	=2
.equ	TOV0	=1

.equ	PUD	=6
.equ	SE	=5
.equ	SM	=4
.equ	SM1	=4
.equ	SM0	=3
.equ	ISC01	=1
.equ	ISC00	=0

.equ	WDRF 	=3
.equ	BORF	=2
.equ	EXTRF	=1
.equ	PORF	=0

.equ	CS02	=2
.equ	CS01	=1
.equ	CS00	=0

.equ	CAL7	=7
.equ	CAL6	=6
.equ	CAL5	=5
.equ	CAL4	=4
.equ	CAL3	=3
.equ	CAL2	=2
.equ	CAL1	=1
.equ	CAL0	=0

.equ	CTC1	=7
.equ	PWM1	=6
.equ	COM11	=5
.equ	COM10	=4
.equ	CS13	=3
.equ	CS12	=2
.equ	CS11	=1
.equ	CS10	=0

.equ	FOCM	=2
.equ	PSR1	=1
.equ	PSR0	=0

.equ	WDTOE	=4
.equ	WDE	=3
.equ	WDP2	=2
.equ	WDP1	=1
.equ	WDP0	=0

.equ	EERIE	=3
.equ	EEMWE	=2
.equ	EEWE	=1
.equ	EERE	=0

.equ	PB4	=4
.equ	PB3	=3
.equ	PB2	=2
.equ	PB1	=1
.equ	PB0	=0

.equ	DDB5	=5
.equ	DDB4	=4
.equ	DDB3	=3
.equ	DDB2	=2
.equ	DDB1	=1
.equ	DDB0	=0

.equ	PINB5	=5
.equ	PINB4	=4
.equ	PINB3	=3
.equ	PINB2	=2
.equ	PINB1	=1
.equ	PINB0	=0

.equ	ACD	=7
.equ	ACO	=5
.equ	ACI	=4
.equ	ACIE	=3
.equ	ACIS1	=1
.equ	ACIS0	=0

.equ    REFS1	=7
.equ	REFS0	=6
.equ	ADLAR	=5
.equ    MUX2    =2
.equ    MUX1    =1
.equ    MUX0    =0

.equ    ADEN    =7
.equ    ADSC    =6
.equ    ADFR    =5
.equ    ADIF    =4
.equ    ADIE    =3
.equ    ADPS2   =2
.equ    ADPS1   =1
.equ    ADPS0   =0

.def	ZL	=r30
.def	ZH	=r31

.equ	INT0addr=$001	;External Interrupt0 Vector Address
.equ	PCINTaddr=$002	;Pin change Interrupt Vector Address
.equ	T1COMPaddr=$003	;Timer1 Compare match interrupt vector address
.equ	OVF1addr=$004	;Overflow 1 Interrupt Vector Address
.equ	OVF0addr=$005	;Overflow 1 Interrupt Vector Address
.equ	EERDYaddr =$006	;EEPROM Interrupt Vector Address
.equ	ACIaddr =$007	;Analog Comparator Interrupt Vector Address
.equ	ADCCaddr =$008	;ADC Interrupt Vector Address

.LIST
