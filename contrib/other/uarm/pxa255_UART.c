#include "pxa255_UART.h"
#include "mem.h"



//TODO: signal handler for Ctl+C and send break to fifo :)
//todo: recalc ints after eveyr write and every call to "process" from SoC


#define UART_IER_DMAE		0x80	//DMA enable
#define UART_IER_UUE		0x40	//Uart unit enable
#define UART_IER_NRZE		0x20	//NRZI enable
#define UART_IER_RTOIE		0x10	//transmit timeout interrupt enable
#define UART_IER_MIE		0x08	//modem interrupt enable
#define UART_IER_RLSE		0x04	//receiver line status interrupt enable
#define UART_IER_TIE		0x02	//transmit data request interrupt enable
#define UART_IER_RAVIE		0x01	//receiver data available interrupt enable

#define UART_IIR_FIFOES		0xC0	//fifo enable status
#define UART_IIR_TOD		0x08	//character timout interrupt pending
#define UART_IIR_RECV_ERR	0x06	//receive error(overrun, parity, framing, break)
#define UART_IIR_RECV_DATA	0x04	//receive data available
#define UART_IIR_RCV_TIMEOUT	0x0C	//receive data in buffer and been a while since we've seen more
#define UART_IIR_SEND_DATA	0x02	//transmit fifo requests data
#define UART_IIR_MODEM_STAT	0x00	//modem lines changed state(CTS, DSR, DI, DCD)
#define UART_IIR_NOINT		0x01	//no interrupt pending


#define UART_FCR_ITL_MASK	0xC0	//mask for ITL part of FCR
#define UART_FCR_ITL_1		0x00	//interrupt when >=1 byte in recv fifo
#define UART_FCR_ITL_8		0x40	//interrupt when >=8 byte in recv fifo
#define UART_FCR_ITL_16		0x80	//interrupt when >=16 byte in recv fifo
#define UART_FCR_ITL_32		0xC0	//interrupt when >=32 byte in recv fifo
#define UART_FCR_RESETTF	0x04	//reset tranmitter fifo
#define UART_FCR_RESETRF	0x02	//reset receiver fifo
#define UART_FCR_TRFIFOE	0x01	//transmit and receive fifo enable

#define UART_LCR_DLAB		0x80	//divisor latch access bit
#define UART_LCR_SB		0x40	//send break
#define UART_LCR_STKYP		0x20	//sticky parity (send parity bit but dont care what value)
#define UART_LCR_EPS		0x10	//even parity select
#define UART_LCR_PEN		0x08	//parity enable
#define UART_LCR_STB		0x04	//stop bits (1 = 2, 0 = 1)
#define UART_LCR_WLS_MASK	0x03	//mask for WLS values
#define UART_LCR_WLS_8		0x03	//8 bit words
#define UART_LCR_WLS_7		0x02	//7 bit words
#define UART_LCR_WLS_6		0x01	//6 bit words
#define UART_LCR_WLS_5		0x00	//5 bit words

#define UART_LSR_FIFOE		0x80	//fifo contails an error (framing, parity, or break)
#define UART_LSR_TEMT		0x40	//tranmitter empty (shift reg is empty and no more byte sin fifo/no byte in holding reg)
#define UART_LSR_TDRQ		0x20	//transmitter data request (see docs)
#define UART_LSR_BI		0x10	//send when char at front of fifo (or in holding reg) was a break char (chr reads as zero by itself)
#define UART_LSR_FE		0x08	//same as above, but for framing errors
#define UART_LSR_PE		0x04	//dame as above, but for parity errors
#define UART_LSR_OE		0x02	//recv fifo overran
#define UART_LSR_DR		0x01	//byte received

#define UART_MCR_LOOP		0x10	//loop modem control lines (not full loopback)
#define UART_MCR_OUT2		0x08	//when loop is 0 enables or disables UART interrupts
#define UART_MCR_OUT1		0x04	//force RI to 1
#define UART_MCR_RTS		0x02	//1 -> nRTS is 0
#define UART_MCR_DTR		0x01	//0 -> nDTR is 0

#define UART_MSR_DCD		0x80
#define UART_MSR_RI		0x40
#define UART_MSR_DSR		0x20
#define UART_MSR_CTS		0x10
#define UART_MSR_DDCD		0x08	//dcd changed since last read
#define UART_MSR_TERI		0x04	//ri has changed from 0 to 1 since last read
#define UART_MSR_DDSR		0x02	//dsr changed since last read
#define UART_MSR_DCTS		0x01	//cts changed since last read



static void pxa255uartPrvRecalc(Pxa255uart* uart);


static void pxa255uartPrvIrq(Pxa255uart* uart, Boolean raise){
	
	pxa255icInt(uart->ic, uart->irq, !(uart->MCR & UART_MCR_LOOP) && (uart->MCR & UART_MCR_OUT2) && raise/* only raise if ints are enabled */);
}

static UInt16 pxa255uartPrvDefaultRead(_UNUSED_ void* userData){			//these are special funcs since they always get their own userData - the uart pointer :)
	
	return UART_CHAR_NONE;	//we read nothing..as always
}

static void pxa255uartPrvDefaultWrite(_UNUSED_ UInt16 chr, _UNUSED_ void* userData){	//these are special funcs since they always get their own userData - the uart pointer :)
	
	//nothing to do here
}

static UInt16 pxa255uartPrvGetchar(Pxa255uart* uart){
	
	Pxa255UartReadF func = uart->readF;
	void* data = (func == pxa255uartPrvDefaultRead) ? uart : uart->accessFuncsData;
	
	return func(data);
} 

static void pxa255uartPrvPutchar(Pxa255uart* uart, UInt16 chr){
	
	Pxa255UartWriteF func = uart->writeF;
	void* data = (func == pxa255uartPrvDefaultWrite) ? uart : uart->accessFuncsData;
	
	func(chr, data);
}

UInt8 pxa255uartPrvFifoUsed(UartFifo* fifo){	//return num spots used
	
	UInt8 v;
	
	if(fifo->read == UART_FIFO_EMPTY) return 0;
	v = fifo->write + UART_FIFO_DEPTH - fifo->read;
	if(v > UART_FIFO_DEPTH) v -=UART_FIFO_DEPTH;
	
	return v;
}

void pxa255uartPrvFifoFlush(UartFifo* fifo){
	
	fifo->read = UART_FIFO_EMPTY;
	fifo->write = UART_FIFO_EMPTY;
}

Boolean pxa255uartPrvFifoPut(UartFifo* fifo, UInt16 val){	//return success
	
	if(fifo->read == UART_FIFO_EMPTY){
		
		fifo->read = 0;
		fifo->write = 1;
		fifo->buf[0] = val;	
	}
	else if(fifo->read != fifo->write){	//only if not full
		
		fifo->buf[fifo->write++] = val;
		if(fifo->write == UART_FIFO_DEPTH) fifo->write = 0;
	}
	else return false;
	
	return true;
}

UInt16 pxa255uartPrvFifoGet(UartFifo* fifo){
	
	UInt16 ret;
	
	if(fifo->read == UART_FIFO_EMPTY){
		
		ret = 0xFFFF;	//why not?
	}
	else{
		
		ret = fifo->buf[fifo->read++];
		if(fifo->read == UART_FIFO_DEPTH) fifo->read = 0;
		
		if(fifo->read == fifo->write){	//it is now empty
			
			fifo->read = UART_FIFO_EMPTY;
			fifo->write = UART_FIFO_EMPTY;
		}
	}
	
	return ret;
}

UInt16 pxa255uartPrvFifoPeekNth(UartFifo* fifo, UInt8 n){
	
	UInt16 ret;
	
	
	if(fifo->read == UART_FIFO_EMPTY){
		
		ret = 0xFFFF;	//why not?
	}
	else{
		
		n += fifo->read;
		if(n >= UART_FIFO_DEPTH) n-= UART_FIFO_DEPTH;
		ret = fifo->buf[n];
	}
	
	return ret;
}

UInt16 pxa255uartPrvFifoPeek(UartFifo* fifo){
	
	return pxa255uartPrvFifoPeekNth(fifo, 0);
}


static void sendVal(Pxa255uart* uart, UInt16 v){
	
	if(uart->LSR & UART_LSR_TEMT){	//if transmit, put in shift register immediately if it's idle
			
		uart->transmitShift = v;
		uart->LSR &=~ UART_LSR_TEMT;	
	}
	else if(uart->FCR & UART_FCR_TRFIFOE){	//put in tx fifo if in fifo mode
		
		pxa255uartPrvFifoPut(&uart->TX, v);
		if(pxa255uartPrvFifoUsed(&uart->TX) > UART_FIFO_DEPTH / 2){	//we go went below half-full buffer - set appropriate bit...
			
			uart->LSR &=~ UART_LSR_TDRQ;
		}
	}
	else if(uart->LSR & UART_LSR_TDRQ){	//send without fifo if in polled mode
			
		uart->transmitHolding = v;
		uart->LSR &=~ UART_LSR_TDRQ;
	}
	else{
		
		//nothing to do - buffer is full so we ignore this request
	}	
}

static Boolean pxa255uartPrvMemAccessF(void* userData, UInt32 pa, UInt8 size, Boolean write, void* buf){

	Pxa255uart* uart = userData;
	Boolean DLAB = (uart->LCR & UART_LCR_DLAB) != 0;
	Boolean recalcValues = false;
	UInt8 t, val = 0;
	
	if(size != 4 && size != 1) {
		err_str(__FILE__ ": Unexpected ");
	//	err_str(write ? "write" : "read");
	//	err_str(" of ");
	//	err_dec(size);
	//	err_str(" bytes to 0x");
	//	err_hex(pa);
	//	err_str("\r\n");
		return true;		//we do not support non-word accesses
	}
	
	pa = (pa - uart->baseAddr) >> 2;
	
	if(write){
		recalcValues = true;
		val = (size == 1) ? *(UInt8*)buf : *(UInt32*)buf;
		
		switch(pa){
			case 0:
				if(DLAB){				//if DLAB - set "baudrate"...
					uart->DLL = val;
					recalcValues = false;
				}
				else{
					
					sendVal(uart, val);
				}
				break;
			
			case 1:
				if(DLAB){
					
					uart->DLH = val;
					recalcValues = false;
				}
				else{
					t = uart->IER ^ val;
				
					if(t & UART_IER_DMAE){
						
						err_str("pxa255UART: DMA mode cannot be enabled");
						t &=~ UART_IER_DMAE;	//undo the change
					}
					
					if(t & UART_IER_UUE){
						
						if(val & UART_IER_UUE){
							
							uart->LSR = UART_LSR_TEMT | UART_LSR_TDRQ;
							uart->MSR = UART_MSR_CTS;
						}	
					}
				
					uart->IER ^= t;
				}
				break;
			
			case 2:
				t = uart->FCR ^ val;
				if(t & UART_FCR_TRFIFOE){
					if(val & UART_FCR_TRFIFOE){			//fifos are now on - perform other actions as requested
						
						if(val & UART_FCR_RESETRF){
							
							pxa255uartPrvFifoFlush(&uart->RX);	//clear the RX fifo now
						}
						if(val & UART_FCR_RESETTF){
							
							pxa255uartPrvFifoFlush(&uart->TX);	//clear the TX fifo now
							uart->LSR = UART_LSR_TEMT | UART_LSR_TDRQ;
						}
						uart->IIR = UART_IIR_FIFOES  |UART_IIR_NOINT;
					}
					else{
						pxa255uartPrvFifoFlush(&uart->TX);
						pxa255uartPrvFifoFlush(&uart->RX);
						uart->LSR = UART_LSR_TEMT | UART_LSR_TDRQ;
						uart->IIR = UART_IIR_NOINT;
					}
				}
				uart->FCR = val;
				break;
			
			case 3:
				t = uart->LCR ^ val;
				if(t & UART_LCR_SB){
					if(val & UART_LCR_SB){	//break set (tx line pulled low)
				
				
					}
					else{			//break cleared (tx line released)
						
						sendVal(uart, UART_CHAR_BREAK);
					}
				}
				uart->LCR = val;
				break;
			
			case 4:
				uart->MCR = val;
				break;
			
			case 7:
				uart->SPR = val;
				break;
			
			case 8:
				uart->ISR = val;
				if(val & 3){
					err_str("UART: IrDA mode set on UART\n");
				}
				break;
		}
	}
	else{
		switch(pa){
			case 0:
				if(DLAB) val = uart->DLL;
				else if(!(uart->LSR & UART_LSR_DR)){	//no data-> too bad
						
					val = 0;	
				}
				else if(uart->FCR & UART_FCR_TRFIFOE){	//fifo mode -> read fifo
					
					val = pxa255uartPrvFifoGet(&uart->RX);
					if(!pxa255uartPrvFifoUsed(&uart->RX)) uart->LSR &=~ UART_LSR_DR;
					recalcValues = true;		//error bits might have changed
				}
				else{					//polled mode -> read rx polled reg
					
					val = uart->receiveHolding;
					uart->LSR &=~ UART_LSR_DR;
				}
				break;
			
			case 1:
				if(DLAB) val = uart->DLH;
				else val = uart->IER;
				break;
			
			case 2:
				val = uart->IIR;
				break;
			
			case 3:
				val = uart->LCR;
				break;
			
			case 4:
				val = uart->MCR;
				break;
			
			case 5:
				val = uart->LSR;
				break;
			
			case 6:
				val = uart->MSR;
				break;
			
			case 7:
				val = uart->SPR;
				break;
			
			case 8:
				val = uart->ISR;
				break;
		}
		if(size == 1) *(UInt8*)buf = val;
		else *(UInt32*)buf = val;
	}
	
	if(recalcValues) pxa255uartPrvRecalc(uart);
	
	return true;
}

void pxa255uartSetFuncs(Pxa255uart* uart, Pxa255UartReadF readF, Pxa255UartWriteF writeF, void* userData){
	
	if(!readF) readF = pxa255uartPrvDefaultRead;		//these are special funcs since they get their own private data - the uart :)
	if(!writeF) writeF = pxa255uartPrvDefaultWrite;
	
	uart->readF = readF;
	uart->writeF = writeF;
	uart->accessFuncsData = userData;
}

Boolean pxa255uartInit(Pxa255uart* uart, ArmMem* physMem, Pxa255ic* ic, UInt32 baseAddr, UInt8 irq){
	
	__mem_zero(uart, sizeof(Pxa255uart));
	uart->ic = ic;
	uart->irq = irq;
	uart->baseAddr = baseAddr;
	uart->IIR = UART_IIR_NOINT;
	uart->IER = UART_IER_UUE | UART_IER_NRZE; 		//uart on
	uart->LSR = UART_LSR_TEMT | UART_LSR_TDRQ;
	uart->MSR = UART_MSR_CTS;
	pxa255uartPrvFifoFlush(&uart->TX);
	pxa255uartPrvFifoFlush(&uart->RX);
	
	
	pxa255uartSetFuncs(uart, NULL, NULL, NULL);
	
	return memRegionAdd(physMem, baseAddr, PXA255_UART_SIZE, pxa255uartPrvMemAccessF, uart);
}

void pxa255uartProcess(Pxa255uart* uart){		//send and rceive up to one character
	
	UInt8 t;
	UInt16 v;
	
	//first process sending (if any)
	if(!(uart->LSR & UART_LSR_TEMT)){
		
		pxa255uartPrvPutchar(uart, uart->transmitShift);
		
		if(uart->FCR & UART_FCR_TRFIFOE){	//fifo mode
			
			t = pxa255uartPrvFifoUsed(&uart->TX);
			
			if(t--){
				
				uart->transmitShift = pxa255uartPrvFifoGet(&uart->TX);
				if(t <= UART_FIFO_DEPTH / 2) uart->LSR |= UART_LSR_TDRQ;	//above half full - clear TDRQ bit
			}
			else{
				
				uart->LSR |= UART_LSR_TEMT;
			}
		}
		else if (uart->LSR & UART_LSR_TDRQ){
			
			uart->LSR |= UART_LSR_TEMT;
		}
		else{
			
			uart->transmitShift = uart->transmitHolding;
			uart->LSR |= UART_LSR_TDRQ;
		}
	}
	
	//now process receiving
	v = pxa255uartPrvGetchar(uart);
	if(v != UART_CHAR_NONE){
		
		uart->cyclesSinceRecv = 0;
		uart->LSR |= UART_LSR_DR;
		
		if(uart->FCR & UART_FCR_TRFIFOE){	//fifo mode
		
			if(!pxa255uartPrvFifoPut(&uart->RX, v)){
				
				uart->LSR |= UART_LSR_OE;	
			}
		}
		else{
			
			if(uart->LSR & UART_LSR_DR) uart->LSR |= UART_LSR_OE;
			else uart->receiveHolding = v;
		}
	}
	else if(uart->cyclesSinceRecv <= 4){
		uart->cyclesSinceRecv++;
	}
	
	pxa255uartPrvRecalc(uart);
}

static void pxa255uartPrvRecalcCharBits(Pxa255uart* uart, UInt16 c){
	
	if(c & UART_CHAR_BREAK) uart->LSR |= UART_LSR_BI;
	if(c & UART_CHAR_FRAME_ERR) uart->LSR |= UART_LSR_FE;
	if(c & UART_CHAR_PAR_ERR) uart->LSR |= UART_LSR_PE;
}

static void pxa255uartPrvRecalc(Pxa255uart* uart){
	
	Boolean errorSet = false;
	UInt8 v;
	
	uart->LSR &=~ UART_LSR_FIFOE;
	uart->IIR &= UART_IIR_FIFOES;	//clear all other bits... 
	uart->LSR &=~ (UART_LSR_BI | UART_LSR_FE | UART_LSR_PE);
	
	if(uart->FCR & UART_FCR_TRFIFOE){	//fifo mode
		
		
		//check rx fifo for errors
		for(v = 0; v < pxa255uartPrvFifoUsed(&uart->RX); v++){
			
			if((pxa255uartPrvFifoPeekNth(&uart->RX, v) >> 8) && (uart->IER & UART_IER_RLSE)){
				
				uart->LSR |= UART_LSR_FIFOE;
				uart->IIR |= UART_IIR_RECV_ERR;
				errorSet = true;
				break;
			}
		}
		
		v = pxa255uartPrvFifoUsed(&uart->RX);
		if(v){
			pxa255uartPrvRecalcCharBits(uart, pxa255uartPrvFifoPeek(&uart->RX));
		}
		
		switch(uart->FCR & UART_FCR_ITL_MASK){
			
			case UART_FCR_ITL_1:
				v = v >= 1;
				break;
			
			case UART_FCR_ITL_8:
				v = v >= 8;
				break;
			
			case UART_FCR_ITL_16:
				v = v >= 16;
				break;
			
			case UART_FCR_ITL_32:
				v = v >= 32;
				break;
		}
		if(v && (uart->IER & UART_IER_RAVIE) && !errorSet){
			
			errorSet = true;
			uart->IIR |= UART_IIR_RECV_DATA;
		}
		if(pxa255uartPrvFifoUsed(&uart->RX) && uart->cyclesSinceRecv >= 4 && (uart->IER & UART_IER_RAVIE) && !errorSet){
			
			errorSet = true;
			uart->IIR |= UART_IIR_RCV_TIMEOUT;	
		}
	}
	else{		//polling mode
		
		UInt16 c = uart->receiveHolding;
		
		if(uart->LSR & UART_LSR_DR){
			
			pxa255uartPrvRecalcCharBits(uart, c);
			
			if((c >> 8) && !errorSet && (uart->IER & UART_IER_RLSE)){
				
				uart->IIR |= UART_IIR_RECV_ERR;
				errorSet = true;
			}
			else if(!errorSet && (uart->IER & UART_IER_RAVIE)){
				
				uart->IIR |= UART_IIR_RECV_DATA;
				errorSet = true;
			}
		}
	}
	
	if(uart->LSR & UART_LSR_TDRQ && !errorSet && (uart->IER & UART_IER_TIE)){
			
		errorSet = true;
		uart->IIR |= UART_IIR_SEND_DATA;
	}
	
	if(!errorSet) uart->IIR |= UART_IIR_NOINT;
	pxa255uartPrvIrq(uart, errorSet);
}
