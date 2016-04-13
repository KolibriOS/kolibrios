;***** THIS IS A MACHINE GENERATED FILE - DO NOT EDIT ********************
;***** Created: 2005-01-11 10:31 ******* Source: ATmega406.xml ***********
;*************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number            : AVR000
;* File Name         : "m406def.inc"
;* Title             : Register/Bit Definitions for the ATmega406
;* Date              : 2005-01-11
;* Version           : 2.14
;* Support E-mail    : avr@atmel.com
;* Target MCU        : ATmega406
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

#ifndef _M406DEF_INC_
#define _M406DEF_INC_


#pragma partinc 0

; ***** SPECIFY DEVICE ***************************************************
.device ATmega406
#pragma AVRPART ADMIN PART_NAME ATmega406
.equ	SIGNATURE_000	= 0x1e
.equ	SIGNATURE_001	= 0x95
.equ	SIGNATURE_002	= 0x07

#pragma AVRPART CORE CORE_VERSION V2E


; ***** I/O REGISTER DEFINITIONS *****************************************
; NOTE:
; Definitions marked "MEMORY MAPPED"are extended I/O ports
; and cannot be used with IN/OUT instructions
.equ	BPPLR	= 0xf8	; MEMORY MAPPED
.equ	BPCR	= 0xf7	; MEMORY MAPPED
.equ	CBPTR	= 0xf6	; MEMORY MAPPED
.equ	BPOCD	= 0xf5	; MEMORY MAPPED
.equ	BPSCD	= 0xf4	; MEMORY MAPPED
.equ	BPDUV	= 0xf3	; MEMORY MAPPED
.equ	BPIR	= 0xf2	; MEMORY MAPPED
.equ	CBCR	= 0xf1	; MEMORY MAPPED
.equ	FCSR	= 0xf0	; MEMORY MAPPED
.equ	CADICH	= 0xe9	; MEMORY MAPPED
.equ	CADICL	= 0xe8	; MEMORY MAPPED
.equ	CADRDC	= 0xe7	; MEMORY MAPPED
.equ	CADRCC	= 0xe6	; MEMORY MAPPED
.equ	CADCSRB	= 0xe5	; MEMORY MAPPED
.equ	CADCSRA	= 0xe4	; MEMORY MAPPED
.equ	CADAC3	= 0xe3	; MEMORY MAPPED
.equ	CADAC2	= 0xe2	; MEMORY MAPPED
.equ	CADAC1	= 0xe1	; MEMORY MAPPED
.equ	CADAC0	= 0xe0	; MEMORY MAPPED
.equ	BGCRR	= 0xd1	; MEMORY MAPPED
.equ	BGCCR	= 0xd0	; MEMORY MAPPED
.equ	CCSR	= 0xc0	; MEMORY MAPPED
.equ	TWBCSR	= 0xbe	; MEMORY MAPPED
.equ	TWAMR	= 0xbd	; MEMORY MAPPED
.equ	TWCR	= 0xbc	; MEMORY MAPPED
.equ	TWDR	= 0xbb	; MEMORY MAPPED
.equ	TWAR	= 0xba	; MEMORY MAPPED
.equ	TWSR	= 0xb9	; MEMORY MAPPED
.equ	TWBR	= 0xb8	; MEMORY MAPPED
.equ	OCR1AH	= 0x89	; MEMORY MAPPED
.equ	OCR1AL	= 0x88	; MEMORY MAPPED
.equ	TCNT1H	= 0x85	; MEMORY MAPPED
.equ	TCNT1L	= 0x84	; MEMORY MAPPED
.equ	TCCR1B	= 0x81	; MEMORY MAPPED
.equ	DIDR0	= 0x7e	; MEMORY MAPPED
.equ	VADMUX	= 0x7c	; MEMORY MAPPED
.equ	VADCSR	= 0x7a	; MEMORY MAPPED
.equ	VADCH	= 0x79	; MEMORY MAPPED
.equ	VADCL	= 0x78	; MEMORY MAPPED
.equ	TIMSK1	= 0x6f	; MEMORY MAPPED
.equ	TIMSK0	= 0x6e	; MEMORY MAPPED
.equ	PCMSK1	= 0x6c	; MEMORY MAPPED
.equ	PCMSK0	= 0x6b	; MEMORY MAPPED
.equ	EICRA	= 0x69	; MEMORY MAPPED
.equ	PCICR	= 0x68	; MEMORY MAPPED
.equ	FOSCCAL	= 0x66	; MEMORY MAPPED
.equ	PRR0	= 0x64	; MEMORY MAPPED
.equ	WUTCSR	= 0x62	; MEMORY MAPPED
.equ	WDTCSR	= 0x60	; MEMORY MAPPED
.equ	SREG	= 0x3f
.equ	SPH	= 0x3e
.equ	SPL	= 0x3d
.equ	SPMCSR	= 0x37
.equ	MCUCR	= 0x35
.equ	MCUSR	= 0x34
.equ	SMCR	= 0x33
.equ	OCDR	= 0x31
.equ	GPIOR2	= 0x2b
.equ	GPIOR1	= 0x2a
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
.equ	PCIFR	= 0x1b
.equ	TIFR1	= 0x16
.equ	TIFR0	= 0x15
.equ	PORTD	= 0x0b
.equ	DDRD	= 0x0a
.equ	PIND	= 0x09
.equ	PORTC	= 0x08
.equ	PORTB	= 0x05
.equ	DDRB	= 0x04
.equ	PINB	= 0x03
.equ	PORTA	= 0x02
.equ	DDRA	= 0x01
.equ	PINA	= 0x00


; ***** BIT DEFINITIONS **************************************************

; ***** AD_CONVERTER *****************
; VADMUX - The VADC multiplexer Selection Register
.equ	VADMUX0	= 0	; Analog Channel and Gain Selection Bits
.equ	VADMUX1	= 1	; Analog Channel and Gain Selection Bits
.equ	VADMUX2	= 2	; Analog Channel and Gain Selection Bits
.equ	VADMUX3	= 3	; Analog Channel and Gain Selection Bits

; VADCSR - The VADC Control and Status register
.equ	VADCCIE	= 0	; VADC Conversion Complete Interrupt Enable
.equ	VADCCIF	= 1	; VADC Conversion Complete Interrupt Flag
.equ	VADSC	= 2	; VADC Satrt Conversion
.equ	VADEN	= 3	; VADC Enable


; ***** EXTERNAL_INTERRUPT ***********
; EICRA - External Interrupt Control Register
.equ	ISC00	= 0	; External Interrupt Sense Control 0 Bit 0
.equ	ISC01	= 1	; External Interrupt Sense Control 0 Bit 1
.equ	ISC10	= 2	; External Interrupt Sense Control 1 Bit 0
.equ	ISC11	= 3	; External Interrupt Sense Control 1 Bit 1
.equ	ISC20	= 4	; External Interrupt Sense Control 2 Bit 0
.equ	ISC21	= 5	; External Interrupt Sense Control 2 Bit 1
.equ	ISC30	= 6	; External Interrupt Sense Control 3 Bit 0
.equ	ISC31	= 7	; External Interrupt Sense Control 3 Bit 1

; EIMSK - External Interrupt Mask Register
.equ	INT0	= 0	; External Interrupt Request 0 Enable
.equ	INT1	= 1	; External Interrupt Request 1 Enable
.equ	INT2	= 2	; External Interrupt Request 1 Enable
.equ	INT3	= 3	; External Interrupt Request 1 Enable

; EIFR - External Interrupt Flag Register
.equ	INTF0	= 0	; External Interrupt Flag 0
.equ	INTF1	= 1	; External Interrupt Flag 1
.equ	INTF2	= 2	; External Interrupt Flag 2
.equ	INTF3	= 3	; External Interrupt Flag 3

; PCICR - Pin Change Interrupt Control Register
.equ	PCIE0	= 0	; Pin Change Interrupt Enable 0
.equ	PCIE1	= 1	; Pin Change Interrupt Enable 1

; PCIFR - Pin Change Interrupt Flag Register
.equ	PCIF0	= 0	; Pin Change Interrupt Flag 1
.equ	PCIF1	= 1	; Pin Change Interrupt Flag 1


; ***** TIMER_COUNTER_1 **************
; TCCR1B - Timer/Counter1 Control Register B
.equ	CS10	= 0	; Clock Select1 bit 0
.equ	CS11	= 1	; Clock Select1 bit 1
.equ	CS12	= 2	; Clock Select1 bit 2
.equ	CTC1	= 3	; Clear Timer/Counter on Compare Match

; TIMSK1 - Timer/Counter Interrupt Mask Register
.equ	TOIE1	= 0	; Timer/Counter1 Overflow Interrupt Enable
.equ	OCIE1A	= 1	; Timer/Counter1 Output Compare Interrupt Enable

; TIFR1 - Timer/Counter Interrupt Flag register
.equ	TOV1	= 0	; Timer/Counter1 Overflow Flag
.equ	OCF1A	= 1	; Timer/Counter1 Output Compare Flag A

; GTCCR - General Timer/Counter Control Register
.equ	PSRSYNC	= 0	; Prescaler Reset
.equ	PSRASY	= 1	; 
.equ	TSM	= 7	; Timer/Counter Synchronization Mode


; ***** WAKEUP_TIMER *****************
; WUTCSR - Wake-up Timer Control Register
.equ	WUTP0	= 0	; Wake-up Timer Prescaler Bit 0
.equ	WUTP1	= 1	; Wake-up Timer Prescaler Bit 1
.equ	WUTP2	= 2	; Wake-up Timer Prescaler Bit 2
.equ	WUTE	= 3	; Wake-up Timer Enable
.equ	WUTR	= 4	; Wake-up Timer Reset
.equ	WUTCF	= 5	; Wake-up timer Calibration Flag
.equ	WUTIE	= 6	; Wake-up Timer Interrupt Enable
.equ	WUTIF	= 7	; Wake-up Timer Interrupt Flag


; ***** BATTERY_PROTECTION ***********
; BPPLR - Battery Protection Parameter Lock Register
.equ	BPPL	= 0	; Battery Protection Parameter Lock
.equ	BPPLE	= 1	; Battery Protection Parameter Lock Enable

; BPCR - Battery Protection Control Register
.equ	CCD	= 0	; 
.equ	DCD	= 1	; 
.equ	SCD	= 2	; 
.equ	DUVD	= 3	; 

; CBPTR - Current Battery Protection Timing Register
.equ	OCPT0	= 0	; 
.equ	OCPT1	= 1	; 
.equ	OCPT2	= 2	; 
.equ	OCPT3	= 3	; 
.equ	SCPT0	= 4	; 
.equ	SCPT1	= 5	; 
.equ	SCPT2	= 6	; 
.equ	SCPT3	= 7	; 

; BPOCD - Battery Protection OverCurrent Detection Level Register
.equ	CCDL0	= 0	; 
.equ	CCDL1	= 1	; 
.equ	CCDL2	= 2	; 
.equ	CCDL3	= 3	; 
.equ	DCDL0	= 4	; 
.equ	DCDL1	= 5	; 
.equ	DCDL2	= 6	; 
.equ	DCDL3	= 7	; 

; BPSCD - Battery Protection Short-Circuit Detection Level Register
.equ	SCDL0	= 0	; 
.equ	SCDL1	= 1	; 
.equ	SCDL2	= 2	; 
.equ	SCDL3	= 3	; 

; BPDUV - Battery Protection Deep Under Voltage Register
.equ	DUDL0	= 0	; 
.equ	DUDL1	= 1	; 
.equ	DUDL2	= 2	; 
.equ	DUDL3	= 3	; 
.equ	DUVT0	= 4	; 
.equ	DUVT1	= 5	; 

; BPIR - Battery Protection Interrupt Register
.equ	SCIE	= 0	; 
.equ	DOCIE	= 1	; 
.equ	COCIE	= 2	; 
.equ	DUVIE	= 3	; Deep Under-voltage Early Warning Interrupt Enable
.equ	SCIF	= 4	; 
.equ	DOCIF	= 5	; 
.equ	COCIF	= 6	; Charge Over-current Protection Activated Interrupt Flag
.equ	DUVIF	= 7	; Deep Under-voltage Early Warning Interrupt Flag


; ***** FET **************************
; FCSR - 
.equ	PFD	= 0	; Precharge FET disable
.equ	CFE	= 1	; Charge FET Enable
.equ	DFE	= 2	; Discharge FET Enable
.equ	CPS	= 3	; Current Protection Status
.equ	PWMOPC	= 4	; Pulse Width Modulation Modulation of OPC output
.equ	PWMOC	= 5	; Pulse Width Modulation of OC output


; ***** COULOMB_COUNTER **************
; CADCSRA - CC-ADC Control and Status Register A
.equ	CADSE	= 0	; When the CADSE bit is written to one, the ongoing CC-ADC conversion is aborted, and the CC-ADC enters Regular Current detection mode.
.equ	CADSI0	= 1	; The CADSI bits determine the current sampling interval for the Regular Current detection in Power-down mode. The actual settings remain to be determined.
.equ	CADSI1	= 2	; The CADSI bits determine the current sampling interval for the Regular Current detection in Power-down mode. The actual settings remain to be determined.
.equ	CADAS0	= 3	; CC_ADC Accumulate Current Select Bit 0
.equ	CADAS1	= 4	; CC_ADC Accumulate Current Select Bit 1
.equ	CADUB	= 5	; CC_ADC Update Busy
.equ	CADEN	= 7	; When the CADEN bit is cleared (zero), the CC-ADC is disabled. When the CADEN bit is set (one), the CC-ADC will continuously measure the voltage drop over the external sense resistor RSENSE. In Power-down, only the Regular Current detection is active. In Power-off, the CC-ADC is always disabled.

; CADCSRB - CC-ADC Control and Status Register B
.equ	CADICIF	= 0	; CC-ADC Instantaneous Current Interrupt Flag
.equ	CADRCIF	= 1	; CC-ADC Accumulate Current Interrupt Flag
.equ	CADACIF	= 2	; CC-ADC Accumulate Current Interrupt Flag
.equ	CADICIE	= 4	; CAD Instantenous Current Interrupt Enable
.equ	CADRCIE	= 5	; Regular Current Interrupt Enable
.equ	CADACIE	= 6	; 

; CADAC3 - ADC Accumulate Current
.equ	CADAC24	= 0	; 
.equ	CADAC25	= 1	; 
.equ	CADAC26	= 2	; 
.equ	CADAC27	= 3	; 
.equ	CADAC28	= 4	; 
.equ	CADAC29	= 5	; 
.equ	CADAC30	= 6	; 
.equ	CADAC31	= 7	; 

; CADAC2 - ADC Accumulate Current
.equ	CADAC16	= 0	; 
.equ	CADAC17	= 1	; 
.equ	CADAC18	= 2	; 
.equ	CADAC19	= 3	; 
.equ	CADAC20	= 4	; 
.equ	CADAC21	= 5	; 
.equ	CADAC22	= 6	; 
.equ	CADAC23	= 7	; 

; CADAC1 - ADC Accumulate Current
.equ	CADAC08	= 0	; 
.equ	CADAC09	= 1	; 
.equ	CADAC10	= 2	; 
.equ	CADAC11	= 3	; 
.equ	CADAC12	= 4	; 
.equ	CADAC13	= 5	; 
.equ	CADAC14	= 6	; 
.equ	CADAC15	= 7	; 

; CADAC0 - ADC Accumulate Current
.equ	CADAC00	= 0	; 
.equ	CADAC01	= 1	; 
.equ	CADAC02	= 2	; 
.equ	CADAC03	= 3	; 
.equ	CADAC04	= 4	; 
.equ	CADAC05	= 5	; 
.equ	CADAC06	= 6	; 
.equ	CADAC07	= 7	; 

; CADRCC - CC-ADC Regular Charge Current
.equ	CADRCC0	= 0	; When the CADSE bit is written to one, the ongoing CC-ADC conversion is aborted, and the CC-ADC enters Regular Current detection mode.
.equ	CADRCC1	= 1	; The CADSI bits determine the current sampling interval for the Regular Current detection in Power-down mode. The actual settings remain to be determined.
.equ	CADRCC2	= 2	; The CADSI bits determine the current sampling interval for the Regular Current detection in Power-down mode. The actual settings remain to be determined.
.equ	CADRCC3	= 3	; The CADACT bits determine the conversion time for the Accumulate Current output as shown in Table 43.
.equ	CADRCC4	= 4	; The CADACT bits determine the conversion time for the Accumulate Current output as shown in Table 43.
.equ	CADRCC5	= 5	; 
.equ	CADRCC6	= 6	; 
.equ	CADRCC7	= 7	; When the CADEN bit is cleared (zero), the CC-ADC is disabled. When the CADEN bit is set (one), the CC-ADC will continuously measure the voltage drop over the external sense resistor RSENSE. In Power-down, only the Regular Current detection is active. In Power-off, the CC-ADC is always disabled.

; CADRDC - CC-ADC Regular Discharge Current
.equ	CADRDC0	= 0	; When the CADSE bit is written to one, the ongoing CC-ADC conversion is aborted, and the CC-ADC enters Regular Current detection mode.
.equ	CADRDC1	= 1	; The CADSI bits determine the current sampling interval for the Regular Current detection in Power-down mode. The actual settings remain to be determined.
.equ	CADRDC2	= 2	; The CADSI bits determine the current sampling interval for the Regular Current detection in Power-down mode. The actual settings remain to be determined.
.equ	CADRDC3	= 3	; The CADACT bits determine the conversion time for the Accumulate Current output as shown in Table 43.
.equ	CADRDC4	= 4	; The CADACT bits determine the conversion time for the Accumulate Current output as shown in Table 43.
.equ	CADRDC5	= 5	; 
.equ	CADRDC6	= 6	; 
.equ	CADRDC7	= 7	; When the CADEN bit is cleared (zero), the CC-ADC is disabled. When the CADEN bit is set (one), the CC-ADC will continuously measure the voltage drop over the external sense resistor RSENSE. In Power-down, only the Regular Current detection is active. In Power-off, the CC-ADC is always disabled.


; ***** CELL_BALANCING ***************
; CBCR - Cell Balancing Control Register
.equ	CBE1	= 0	; Battery Protection Parameter Lock
.equ	CBE2	= 1	; Cell Balancing Enable 2
.equ	CBE3	= 2	; Cell Balancing Enable 4
.equ	CBE4	= 3	; Cell Balancing Enable 4


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
.equ	PUD	= 4	; Pull-up disable
.equ	JTD	= 7	; JTAG Disable

; MCUSR - MCU Status Register
.equ	PORF	= 0	; Power-on reset flag
.equ	EXTRF	= 1	; External Reset Flag
.equ	BODRF	= 2	; Brown-out Reset Flag
.equ	WDRF	= 3	; Watchdog Reset Flag
.equ	JTRF	= 4	; JTAG Reset Flag

; FOSCCAL - Fast Oscillator Calibration Value
.equ	FCAL0	= 0	; Oscillator Calibration Value Bit0
.equ	FCAL1	= 1	; Oscillator Calibration Value Bit1
.equ	FCAL2	= 2	; Oscillator Calibration Value Bit2
.equ	FCAL3	= 3	; Oscillator Calibration Value Bit3
.equ	FCAL4	= 4	; Oscillator Calibration Value Bit4
.equ	FCAL5	= 5	; Oscillator Calibration Value Bit5
.equ	FCAL6	= 6	; Oscillator Calibration Value Bit6
.equ	FCAL7	= 7	; Oscillator Calibration Value Bit7

; SMCR - Sleep Mode Control Register
.equ	SE	= 0	; Sleep Enable
.equ	SM0	= 1	; Sleep Mode Select bit 0
.equ	SM1	= 2	; Sleep Mode Select bit 1
.equ	SM2	= 3	; Sleep Mode Select bit 2

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

; CCSR - Clock Control and Status Register
.equ	ACS	= 0	; Asynchronous Clock Select
.equ	XOE	= 1	; 32 kHz Crystal Oscillator Enable

; DIDR0 - Digital Input Disable Register
.equ	VADC0D	= 0	; When this bit is written logic one, the digital input buffer of the corresponding V_ADC pin is disabled.
.equ	VADC1D	= 1	; When this bit is written logic one, the digital input buffer of the corresponding V_ADC pin is disabled.
.equ	VADC2D	= 2	; When this bit is written logic one, the digital input buffer of the corresponding V_ADC pin is disabled.
.equ	VADC3D	= 3	; When this bit is written logic one, the digital input buffer of the corresponding V_ADC pin is disabled.

; PRR0 - Power Reduction Register 0
.equ	PRVADC	= 0	; Power Reduction V-ADC
.equ	PRTIM0	= 1	; Power Reduction Timer/Counter0
.equ	PRTIM1	= 2	; Power Reduction Timer/Counter1
.equ	PRTWI	= 3	; Power Reduction TWI


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


; ***** TIMER_COUNTER_0 **************
; TCCR0A - Timer/Counter0 Control Register
.equ	WGM00	= 0	; Clock Select0 bit 0
.equ	WGM01	= 1	; Clock Select0 bit 1
.equ	COM0B0	= 4	; 
.equ	COM0B1	= 5	; 
.equ	COM0A0	= 6	; Waveform Generation Mode
.equ	COM0A1	= 7	; Force Output Compare

; TCCR0B - Timer/Counter0 Control Register
.equ	CS00	= 0	; Clock Select0 bit 0
.equ	CS01	= 1	; Clock Select0 bit 1
.equ	CS02	= 2	; Clock Select0 bit 2
.equ	WGM02	= 3	; 
.equ	FOC0B	= 6	; Waveform Generation Mode
.equ	FOC0A	= 7	; Force Output Compare

; TCNT0 - Timer Counter 0
.equ	TCNT00	= 0	; Timer Counter 0 bit 0
.equ	TCNT01	= 1	; Timer Counter 0 bit 1
.equ	TCNT02	= 2	; Timer Counter 0 bit 2
.equ	TCNT03	= 3	; Timer Counter 0 bit 3
.equ	TCNT04	= 4	; Timer Counter 0 bit 4
.equ	TCNT05	= 5	; Timer Counter 0 bit 5
.equ	TCNT06	= 6	; Timer Counter 0 bit 6
.equ	TCNT07	= 7	; Timer Counter 0 bit 7

; OCR0A - Output compare Register A
.equ	OCR0A0	= 0	; 
.equ	OCR0A1	= 1	; 
.equ	OCR0A2	= 2	; 
.equ	OCR0A3	= 3	; 
.equ	OCR0A4	= 4	; 
.equ	OCR0A5	= 5	; 
.equ	OCR0A6	= 6	; 
.equ	OCR0A7	= 7	; 

; OCR0B - Output compare Register B
.equ	OCR0B0	= 0	; 
.equ	OCR0B1	= 1	; 
.equ	OCR0B2	= 2	; 
.equ	OCR0B3	= 3	; 
.equ	OCR0B4	= 4	; 
.equ	OCR0B5	= 5	; 
.equ	OCR0B6	= 6	; 
.equ	OCR0B7	= 7	; 

; TIMSK0 - Timer/Counter Interrupt Mask Register
.equ	TOIE0	= 0	; Overflow Interrupt Enable
.equ	OCIE0A	= 1	; Output Compare Interrupt Enable
.equ	OCIE0B	= 2	; Output Compare Interrupt Enable

; TIFR0 - Timer/Counter Interrupt Flag register
.equ	TOV0	= 0	; Overflow Flag
.equ	OCF0A	= 1	; Output Compare Flag
.equ	OCF0B	= 2	; Output Compare Flag


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


; ***** PORTD ************************
; PORTD - Data Register, Port D
.equ	PORTD0	= 0	; 
.equ	PD0	= 0	; For compatibility
.equ	PORTD1	= 1	; 
.equ	PD1	= 1	; For compatibility

; DDRD
.equ	DDD0	= 0	; 
.equ	DDD1	= 1	; 

; PIND - Input Pins, Port D
.equ	PIND0	= 0	; 
.equ	PIND1	= 1	; 


; ***** BOOT_LOAD ********************
; SPMCSR - Store Program Memory Control Register
.equ	SPMEN	= 0	; Store Program Memory Enable
.equ	PGERS	= 1	; Page Erase
.equ	PGWRT	= 2	; Page Write
.equ	BLBSET	= 3	; Boot Lock Bit Set
.equ	RWWSRE	= 4	; Read While Write section read enable
.equ	SIGRD	= 5	; Signature Row Read
.equ	RWWSB	= 6	; Read While Write Section Busy
.equ	SPMIE	= 7	; SPM Interrupt Enable


; ***** TWI **************************
; TWBCSR - TWI Bus Control and Status Register
.equ	TWBCIP	= 0	; TWI Bus Connect/Disconnect Interrupt Polarity
.equ	TWBDT0	= 1	; TWI Bus Disconnect Time-out Period
.equ	TWBDT1	= 2	; TWI Bus Disconnect Time-out Period
.equ	TWBCIE	= 6	; TWI Bus Connect/Disconnect Interrupt Enable
.equ	TWBCIF	= 7	; TWI Bus Connect/Disconnect Interrupt Flag

; TWAMR - TWI (Slave) Address Mask Register
.equ	TWAM0	= 1	; 
.equ	TWAM1	= 2	; 
.equ	TWAM2	= 3	; 
.equ	TWAM3	= 4	; 
.equ	TWAM4	= 5	; 
.equ	TWAM5	= 6	; 
.equ	TWAM6	= 7	; 

; TWBR - TWI Bit Rate register
.equ	TWBR0	= 0	; 
.equ	TWBR1	= 1	; 
.equ	TWBR2	= 2	; 
.equ	TWBR3	= 3	; 
.equ	TWBR4	= 4	; 
.equ	TWBR5	= 5	; 
.equ	TWBR6	= 6	; 
.equ	TWBR7	= 7	; 

; TWCR - TWI Control Register
.equ	TWIE	= 0	; TWI Interrupt Enable
.equ	TWEN	= 2	; TWI Enable Bit
.equ	TWWC	= 3	; TWI Write Collition Flag
.equ	TWSTO	= 4	; TWI Stop Condition Bit
.equ	TWSTA	= 5	; TWI Start Condition Bit
.equ	TWEA	= 6	; TWI Enable Acknowledge Bit
.equ	TWINT	= 7	; TWI Interrupt Flag

; TWSR - TWI Status Register
.equ	TWPS0	= 0	; TWI Prescaler
.equ	TWPS1	= 1	; TWI Prescaler
.equ	TWS3	= 3	; TWI Status
.equ	TWS4	= 4	; TWI Status
.equ	TWS5	= 5	; TWI Status
.equ	TWS6	= 6	; TWI Status
.equ	TWS7	= 7	; TWI Status

; TWDR - TWI Data register
.equ	TWD0	= 0	; TWI Data Register Bit 0
.equ	TWD1	= 1	; TWI Data Register Bit 1
.equ	TWD2	= 2	; TWI Data Register Bit 2
.equ	TWD3	= 3	; TWI Data Register Bit 3
.equ	TWD4	= 4	; TWI Data Register Bit 4
.equ	TWD5	= 5	; TWI Data Register Bit 5
.equ	TWD6	= 6	; TWI Data Register Bit 6
.equ	TWD7	= 7	; TWI Data Register Bit 7

; TWAR - TWI (Slave) Address register
.equ	TWGCE	= 0	; TWI General Call Recognition Enable Bit
.equ	TWA0	= 1	; TWI (Slave) Address register Bit 0
.equ	TWA1	= 2	; TWI (Slave) Address register Bit 1
.equ	TWA2	= 3	; TWI (Slave) Address register Bit 2
.equ	TWA3	= 4	; TWI (Slave) Address register Bit 3
.equ	TWA4	= 5	; TWI (Slave) Address register Bit 4
.equ	TWA5	= 6	; TWI (Slave) Address register Bit 5
.equ	TWA6	= 7	; TWI (Slave) Address register Bit 6


; ***** BANDGAP **********************
; BGCRR - Bandgap Calibration of Resistor Ladder
.equ	BGCR0	= 0	; Bandgap Calibration of Resistor Ladder Bit 0
.equ	BGCR1	= 1	; Bandgap Calibration of Resistor Ladder Bit 1
.equ	BGCR2	= 2	; Bandgap Calibration of Resistor Ladder Bit 2
.equ	BGCR3	= 3	; Bandgap Calibration of Resistor Ladder Bit 3
.equ	BGCR4	= 4	; Bandgap Calibration of Resistor Ladder Bit 4
.equ	BGCR5	= 5	; Bandgap Calibration of Resistor Ladder Bit 5
.equ	BGCR6	= 6	; Bandgap Calibration of Resistor Ladder Bit 6
.equ	BGCR7	= 7	; Bandgap Calibration of Resistor Ladder Bit 7

; BGCCR - Bandgap Calibration Register
.equ	BGCC0	= 0	; BG Calibration of PTAT Current Bit 0
.equ	BGCC1	= 1	; BG Calibration of PTAT Current Bit 1
.equ	BGCC2	= 2	; BG Calibration of PTAT Current Bit 2
.equ	BGCC3	= 3	; BG Calibration of PTAT Current Bit 3
.equ	BGCC4	= 4	; BG Calibration of PTAT Current Bit 4
.equ	BGCC5	= 5	; BG Calibration of PTAT Current Bit 5
.equ	BGEN	= 7	; Setting the BGEN bit to one will enable the bandgap voltage reference. This bit must be set before enabling the CC_ADC or V_ADC, and must remain set while either ADC is enabled.


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
.equ	EEPM0	= 4	; EEPROM Programming Mode Bits
.equ	EEPM1	= 5	; EEPROM Programming Mode Bits



; ***** LOCKSBITS ********************************************************
.equ	LB1	= 0	; Lock bit
.equ	LB2	= 1	; Lock bit
.equ	BLB01	= 2	; Boot Lock bit
.equ	BLB02	= 3	; Boot Lock bit
.equ	BLB11	= 4	; Boot lock bit
.equ	BLB12	= 5	; Boot lock bit


; ***** FUSES ************************************************************
; LOW fuse bits
.equ	SUT0	= 1	; Select start-up time
.equ	SUT1	= 2	; Select start-up time
.equ	BOOTRST	= 3	; Select reset vector
.equ	BOOTSZ0	= 4	; Select boot size
.equ	BOOTSZ1	= 5	; Select boot size
.equ	EESAVE	= 6	; EEPROM memory is preserved through chip erase
.equ	WDTON	= 7	; Watchdog Timer Always On

; HIGH fuse bits
.equ	JTAGEN	= 0	; Enable JTAG
.equ	OCDEN	= 1	; Enable OCD



; ***** CPU REGISTER DEFINITIONS *****************************************
.def	XH	= r27
.def	XL	= r26
.def	YH	= r29
.def	YL	= r28
.def	ZH	= r31
.def	ZL	= r30



; ***** DATA MEMORY DECLARATIONS *****************************************
.equ	FLASHEND	= 0x4fff	; Note: Word address
.equ	IOEND	= 0x00ff
.equ	SRAM_START	= 0x0100
.equ	SRAM_SIZE	= 2048
.equ	RAMEND	= 0x08ff
.equ	XRAMEND	= 0x07ff
.equ	E2END	= 0x01ff
.equ	EEPROMEND	= 0x01ff
.equ	EEADRBITS	= 9
#pragma AVRPART MEMORY PROG_FLASH 40960
#pragma AVRPART MEMORY EEPROM 512
#pragma AVRPART MEMORY INT_SRAM SIZE 2048
#pragma AVRPART MEMORY INT_SRAM START_ADDR 0x100



; ***** BOOTLOADER DECLARATIONS ******************************************
.equ	NRWW_START_ADDR	= 0x4800
.equ	NRWW_STOP_ADDR	= 0x4fff
.equ	RWW_START_ADDR	= 0x0
.equ	RWW_STOP_ADDR	= 0x47ff
.equ	PAGESIZE	= 32
.equ	FIRSTBOOTSTART	= 0x4f00
.equ	SECONDBOOTSTART	= 0x4e00
.equ	THIRDBOOTSTART	= 0x4c00
.equ	FOURTHBOOTSTART	= 0x4800
.equ	SMALLBOOTSTART	= FIRSTBOOTSTART
.equ	LARGEBOOTSTART	= FOURTHBOOTSTART



; ***** INTERRUPT VECTORS ************************************************
.equ	BPINTaddr	= 0x0002	; Battery Protection Interrupt
.equ	INT0addr	= 0x0004	; External Interrupt Request 0
.equ	INT1addr	= 0x0006	; External Interrupt Request 1
.equ	INT2addr	= 0x0008	; External Interrupt Request 2
.equ	INT3addr	= 0x000a	; External Interrupt Request 3
.equ	PCI0addr	= 0x000c	; Pin Change Interrupt 0
.equ	PCI1addr	= 0x000e	; Pin Change Interrupt 1
.equ	WDTaddr	= 0x0010	; Watchdog Timeout Interrupt
.equ	WUTaddr	= 0x0012	; Wakeup timer overflow
.equ	OC1addr	= 0x0014	; Timer/Counter 1 Compare Match
.equ	OVF1addr	= 0x0016	; Timer/Counter 1 Overflow
.equ	OC0Aaddr	= 0x0018	; Timer/Counter0 Compare A Match
.equ	OC0Baddr	= 0x001a	; Timer/Counter0 Compare B Match
.equ	OVF0addr	= 0x001c	; Timer/Counter0 Overflow
.equ	TWICDaddr	= 0x001e	; Two-Wire Bus Connect/Disconnect
.equ	TWIaddr	= 0x0020	; Two-Wire Serial Interface
.equ	VADCaddr	= 0x0022	; Voltage ADC Conversion Complete
.equ	CADICaddr	= 0x0024	; Coulomb Counter ADC Conversion Complete
.equ	CADRCaddr	= 0x0026	; Coloumb Counter ADC Regular Current
.equ	CADACaddr	= 0x0028	; Coloumb Counter ADC Accumulator
.equ	ERDYaddr	= 0x002a	; EEPROM Ready
.equ	SPMRaddr	= 0x002c	; Store Program Memory Ready

.equ	INT_VECTORS_SIZE	= 46	; size in words

#endif  /* _M406DEF_INC_ */

; ***** END OF FILE ******************************************************
