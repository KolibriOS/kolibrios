;***************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number		:AVR000
;* File Name		:"tn28def.inc"
;* Title		:Register/Bit Definitions for the ATtiny28
;* Date                 :99.01.28
;* Version              :1.30
;* Support E-mail	:avr@atmel.com
;* Target MCU		:ATtiny28
;*
;* DESCRIPTION
;* When including this file in the assembly program file, all I/O register
;* names and I/O register bit names appearing in the data book can be used.
;* In addition, the two registers forming the data pointers Z have been 
;* assigned names ZL - ZH. 
;*
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
.device ATtiny28

;***** I/O Register Definitions
.equ	SREG	=$3f
.equ	PORTA	=$1b
.equ	PACR	=$1a
.equ	PINA	=$19
.equ	PINB	=$16
.equ	PORTD	=$12
.equ	DDRD	=$11
.equ	PIND	=$10
.equ	ACSR	=$08
.equ	MCUCS	=$07
.equ    ICR     =$06
.equ    IFR     =$05
.equ	TCCR0	=$04
.equ	TCNT0	=$03
.equ	MODCR	=$02
.equ	WDTCR	=$01
.equ	OSCCAL	=$00


;***** Bit Definitions
.equ	PA3		=3
.equ	PA2		=2
.equ	PA1		=1
.equ	PA0		=0

.equ	DDA3	=3
.equ	PA2HC	=2
.equ	DDA1	=1
.equ	DDA0	=0

.equ	PINA3	=3
.equ	PINA1	=1
.equ	PINA0	=0

.equ	PINB7	=7
.equ	PINB6	=6
.equ	PINB5	=5
.equ	PINB4	=4
.equ	PINB3	=3
.equ	PINB2	=2
.equ	PINB1	=1
.equ	PINB0	=0

.equ	PD7	=7
.equ	PD6	=6
.equ	PD5	=5
.equ	PD4	=4
.equ	PD3	=3
.equ	PD2	=2
.equ	PD1	=1
.equ	PD0	=0

.equ	DDD7	=7
.equ	DDD6	=6
.equ	DDD5	=5
.equ	DDD4	=4
.equ	DDD3	=3
.equ	DDD2	=2
.equ	DDD1	=1
.equ	DDD0	=0

.equ	PIND7	=7
.equ	PIND6	=6
.equ	PIND5	=5
.equ	PIND4	=4
.equ	PIND3	=3
.equ	PIND2	=2
.equ	PIND1	=1
.equ	PIND0	=0

.equ	ACD	=7
.equ	ACO	=5
.equ	ACI	=4
.equ	ACIE	=3
.equ	ACIS1	=1
.equ	ACIS0	=0

.equ	PLUPB	=7
.equ	SE	=5
.equ	SM	=4
.equ    WDRF    =3
.equ    EXTRF   =1
.equ    PORF    =0

.equ	INT1	=7
.equ	INT0	=6
.equ	LLIE	=5
.equ	TOIE0	=4
.equ	ISC11	=3
.equ	ISC10	=2
.equ	ISC01	=1
.equ	ISC00	=0

.equ	INTF1	=7
.equ	INTF0	=6
.equ	TOV0	=4

.equ	FOV0	=7
.equ	OOM01	=4
.equ	OOM00	=3
.equ	CS02	=2
.equ	CS01	=1
.equ	CS00	=0

.equ	WDTOE	=4
.equ	WDE	=3
.equ	WDP2	=2
.equ	WDP1	=1
.equ	WDP0	=0

.equ    ONTIM4  =7
.equ    ONTIM3  =6
.equ    ONTIM2  =5
.equ    ONTIM1  =4
.equ    ONTIM0  =3
.equ    MCONF2  =2
.equ    MCONF1  =1
.equ    MCONF0  =0

.equ	CAL7	=7
.equ	CAL6	=6
.equ	CAL5	=5
.equ	CAL4	=4
.equ	CAL3	=3
.equ	CAL2	=2
.equ	CAL1	=1
.equ	CAL0	=0

.def	ZL	=r30
.def	ZH	=r31


.equ	FLASHEND = 0x07FF
.equ	RAMEND   = 0x03FF

.equ	INT0addr =$001	;External Interrupt0 Vector Address
.equ	INT1addr =$002	;External Interrupt1 Vector Address
.equ	LLINTaddr=$003	;Low level Interrupt Vector Address
.equ	OVF0addr =$004	;Overflow0 Interrupt Vector Address
.equ	ACIaddr  =$005	;Analog Comparator Interrupt Vector Address

#pragma AVRPART ADMIN PART_NAME ATtiny28
#pragma AVRPART CORE CORE_VERSION V0E
#pragma AVRPART MEMORY PROG_FLASH 2048
#pragma AVRPART MEMORY EEPROM 0
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x60
#pragma AVRPART MEMORY INT_SRAM SIZE  0
