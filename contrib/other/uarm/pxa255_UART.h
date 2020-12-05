#ifndef _PXA255_UART_H_
#define _PXA255_UART_H_

#include "mem.h"
#include "cpu.h"
#include "pxa255_IC.h"


/*
	PXA255 UARTs
	
	PXA255 has three. they are identical, but at diff base addresses. this implements one. instanciate more than one of this struct to make all 3 work if needed.
	PURRPOSE: this is how linux talks to us :)


	by default we read nothing and write nowhere (buffer drains fast into nothingness)
	this can be changed by addidng appropriate callbacks

*/

#define PXA255_FFUART_BASE	0x40100000UL
#define PXA255_BTUART_BASE	0x40200000UL
#define PXA255_STUART_BASE	0x40700000UL
#define PXA255_UART_SIZE	0x00010000UL

#define UART_FIFO_DEPTH		64


#define UART_CHAR_BREAK		0x800
#define UART_CHAR_FRAME_ERR	0x400
#define UART_CHAR_PAR_ERR	0x200
#define UART_CHAR_NONE		0x100

typedef UInt16	(*Pxa255UartReadF)(void* userData);
typedef void	(*Pxa255UartWriteF)(UInt16 chr, void* userData);

#define UART_FIFO_EMPTY	0xFF

typedef struct{

	UInt8 read;
	UInt8 write;
	UInt16 buf[UART_FIFO_DEPTH];

}UartFifo;

typedef struct{

	Pxa255ic* ic;
	UInt32 baseAddr;
	
	Pxa255UartReadF readF;
	Pxa255UartWriteF writeF;
	void* accessFuncsData;
	
	UartFifo TX, RX;
	
	UInt16 transmitShift;	//char currently "sending"
	UInt16 transmitHolding;	//holding register for no-fifo mode
	
	UInt16 receiveHolding;	//char just received
	
	UInt8 irq:5;
	UInt8 cyclesSinceRecv:3;
	
	UInt8 IER;		//interrupt enable register
	UInt8 IIR;		//interrupt information register
	UInt8 FCR;		//fifo control register
	UInt8 LCR;		//line control register
	UInt8 LSR;		//line status register
	UInt8 MCR;		//modem control register
	UInt8 MSR;		//modem status register
	UInt8 SPR;		//scratchpad register
	UInt8 DLL;		//divisor latch low
	UInt8 DLH;		//divior latch high;
	UInt8 ISR;		//infrared selection register
	
	
	
}Pxa255uart;

Boolean pxa255uartInit(Pxa255uart* uart, ArmMem* physMem, Pxa255ic* ic, UInt32 baseAddr, UInt8 irq);
void pxa255uartProcess(Pxa255uart* uart);		//write out data in TX fifo and read data into RX fifo

void pxa255uartSetFuncs(Pxa255uart* uart, Pxa255UartReadF readF, Pxa255UartWriteF writeF, void* userData);

#endif

