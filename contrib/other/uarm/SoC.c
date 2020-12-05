#include "SoC.h"
#include "CPU.h"
#include "MMU.h"
#include "mem.h"
#include "callout_RAM.h"
#include "RAM.h"
#include "cp15.h"
#include "math64.h"
#include "pxa255_IC.h"
#include "pxa255_TIMR.h"
#include "pxa255_RTC.h"
#include "pxa255_UART.h"
#include "pxa255_PwrClk.h"
#include "pxa255_GPIO.h"
#include "pxa255_DMA.h"
#include "pxa255_DSP.h"
#include "pxa255_LCD.h"
#ifdef EMBEDDED
	#include <avr/io.h>
#endif


#define ERR(s)	do{err_str(s " Halting\r\n"); while(1); }while(0)

static const UInt8 embedded_boot[] =	{
						0x01, 0x00, 0x8F, 0xE2, 0x10, 0xFF, 0x2F, 0xE1, 0x04, 0x27, 0x01, 0x20, 0x00, 0x21, 0x00, 0xF0,
						0x0D, 0xF8, 0x0A, 0x24, 0x24, 0x07, 0x65, 0x1C, 0x05, 0x27, 0x00, 0x22, 0x00, 0xF0, 0x06, 0xF8,
						0x20, 0x60, 0x24, 0x1D, 0x49, 0x1C, 0x80, 0x29, 0xF8, 0xD1, 0x28, 0x47, 0xBC, 0x46, 0xBB, 0xBB,
						0x70, 0x47
					};

#define ROM_BASE	0x00000000UL
#define ROM_SIZE	sizeof(embedded_boot)

#define RAM_BASE	0xA0000000UL
#define RAM_SIZE	0x01000000UL	//16M @ 0xA0000000


static Boolean vMemF(ArmCpu* cpu, void* buf, UInt32 vaddr, UInt8 size, Boolean write, Boolean priviledged, UInt8* fsrP){
	
	SoC* soc = cpu->userData;
	UInt32 pa;
	
	if(size & (size - 1)){	//size is not a power of two
		
		return false;	
	}
	if(vaddr & (size - 1)){	//bad alignment
		
		return false;	
	}

	return mmuTranslate(&soc->mmu, vaddr, priviledged, write, &pa, fsrP) && memAccess(&soc->mem, pa, size, write, buf);
}


static Boolean hyperF(ArmCpu* cpu){		//return true if handled

	SoC* soc = cpu->userData;
	
	if(cpu->regs[12] == 0){
		err_str("Hypercall 0 caught\r\n");
		soc->go = false;
	}
	else if(cpu->regs[12] == 1){
		err_dec(cpu->regs[0]);
	}
	else if(cpu->regs[12] == 2){
		char x[2];
		x[1] = 0;
		x[0] = cpu->regs[0];
		err_str(x);
	}
	else if(cpu->regs[12] == 3){			//get ram size
		cpu->regs[0] = RAM_SIZE;
	}
	else if(cpu->regs[12] == 4){			//block device access perform [do a read or write]
		
		//IN:
		// R0 = op
		// R1 = sector
		
		return soc->blkF(soc->blkD, cpu->regs[1], soc->blkDevBuf, cpu->regs[0]);
	}
	else if(cpu->regs[12] == 5){			//block device buffer access [read or fill emulator's buffer]
		
		//IN:
		// R0 = word value
		// R1 = word offset (0, 1, 2...)
		// R2 = op (1 = write, 0 = read)
		//OUT:
		// R0 = word value
		
		if(cpu->regs[1] >= BLK_DEV_BLK_SZ / sizeof(UInt32)) return false;	//invalid request
		
		if(cpu->regs[2] == 0){
			
			cpu->regs[0] = soc->blkDevBuf[cpu->regs[1]];
		}
		else if(cpu->regs[2] == 1){
			
			soc->blkDevBuf[cpu->regs[1]] = cpu->regs[0];
		}
		else return false;
	}
	return true;
}

static void setFaultAdrF(ArmCpu* cpu, UInt32 adr, UInt8 faultStatus){
	
	SoC* soc = cpu->userData;
	
	cp15SetFaultStatus(&soc->cp15, adr, faultStatus);
}

static void emulErrF(_UNUSED_ ArmCpu* cpu, const char* str){
	err_str("Emulation error: <<");
	err_str(str);
	err_str(">> halting\r\n");
	while(1);
}

static Boolean pMemReadF(void* userData, UInt32* buf, UInt32 pa){	//for DMA engine and MMU pagetable walks

	ArmMem* mem = userData;

	return memAccess(mem, pa, 4, false, buf);
}

static void dumpCpuState(ArmCpu* cpu, char* label){

	UInt8 i;

	if(label){
		err_str("CPU ");
		err_str(label);
		err_str("\r\n");
	}
	
	for(i = 0; i < 16; i++){
		err_str("R");
		err_dec(i);
		err_str("\t= 0x");
		err_hex(cpuGetRegExternal(cpu, i));
		err_str("\r\n");	
	}
	err_str("CPSR\t= 0x");
	err_hex(cpuGetRegExternal(cpu, ARM_REG_NUM_CPSR));
	err_str("\r\nSPSR\t= 0x");
	err_hex(cpuGetRegExternal(cpu, ARM_REG_NUM_SPSR));
	err_str("\r\n");
}

static UInt16 socUartPrvRead(void* userData){			//these are special funcs since they always get their own userData - the uart :)
	
	SoC* soc = userData;
	UInt16 v;
	int r;
	
	r = soc->rcF();
	if(r == CHAR_CTL_C) v = UART_CHAR_BREAK;
	else if(r == CHAR_NONE) v = UART_CHAR_NONE;
	else if(r >= 0x100) v = UART_CHAR_NONE;		//we canot send this char!!!
	else v = r;
	
	return v;
}

static void socUartPrvWrite(UInt16 chr, void* userData){	//these are special funcs since they always get their own userData - the uart :)
	
	SoC* soc = userData;
	
	
	if(chr == UART_CHAR_NONE) return;
	
	soc->wcF(chr);
}

void LinkError_SIZEOF_STRUCT_SOC_wrong();


void socRamModeAlloc(SoC* soc, _UNUSED_ void* ignored){
	
	UInt32* ramB = emu_alloc(RAM_SIZE);
	if(!ramB) ERR("Cannot allocate RAM buffer");
	if(!ramInit(&soc->ram.RAM, &soc->mem, RAM_BASE, RAM_SIZE, ramB)) ERR("Cannot init RAM");
	
	soc->calloutMem = false;	
}

void socRamModeCallout(SoC* soc, void* callout){
	
	if(!coRamInit(&soc->ram.coRAM, &soc->mem, RAM_BASE, RAM_SIZE, callout)) ERR("Cannot init coRAM");
	
	soc->calloutMem = true;	
}

#define ERR_(s)	ERR("error");

void socInit(SoC* soc, SocRamAddF raF, void*raD, readcharF rc, writecharF wc, blockOp blkF, void* blkD){

printf ("SoC init! \n");
	Err e;
	
	soc->rcF = rc;
	soc->wcF = wc;
	
	soc->blkF = blkF;
	soc->blkD = blkD;

	soc->go = true;
	
	e = cpuInit(&soc->cpu, ROM_BASE, vMemF, emulErrF, hyperF, &setFaultAdrF);
	if(e){
		err_str("Failed to init CPU: ");
	//	err_dec(e);
	//	err_str(". Halting\r\n");
		while(1);
	}
	printf("CPU init\n");
	soc->cpu.userData = soc;
	
	memInit(&soc->mem);
	mmuInit(&soc->mmu, pMemReadF, &soc->mem);
	printf("Init complete\n");
	if(ROM_SIZE > sizeof(soc->romMem)) {
	//	err_str("Failed to init CPU: ");
		err_str("ROM_SIZE to small");
	//	err_str(". Halting\r\n");
		while(1);
	}
	
	printf("RAF\n");
	
	raF(soc, raD);
	
	if(!ramInit(&soc->ROM, &soc->mem, ROM_BASE, ROM_SIZE, soc->romMem)) ERR_("Cannot init ROM");
	
	cp15Init(&soc->cp15, &soc->cpu, &soc->mmu);
	
	__mem_copy(soc->romMem, embedded_boot, sizeof(embedded_boot));
	
	printf("Things...\n");
	
	if(!pxa255icInit(&soc->ic, &soc->cpu, &soc->mem)) ERR_("Cannot init PXA255's interrupt controller");
	if(!pxa255timrInit(&soc->timr, &soc->mem, &soc->ic)) ERR_("Cannot init PXA255's OS timers");
	if(!pxa255rtcInit(&soc->rtc, &soc->mem, &soc->ic)) ERR_("Cannot init PXA255's RTC");
	if(!pxa255uartInit(&soc->ffuart, &soc->mem, &soc->ic,PXA255_FFUART_BASE, PXA255_I_FFUART)) ERR_("Cannot init PXA255's FFUART");
	if(!pxa255uartInit(&soc->btuart, &soc->mem, &soc->ic,PXA255_BTUART_BASE, PXA255_I_BTUART)) ERR_("Cannot init PXA255's BTUART");
	if(!pxa255uartInit(&soc->stuart, &soc->mem, &soc->ic,PXA255_STUART_BASE, PXA255_I_STUART)) ERR_("Cannot init PXA255's STUART");
	if(!pxa255pwrClkInit(&soc->pwrClk, &soc->cpu, &soc->mem)) ERR_("Cannot init PXA255's Power and Clock manager");
	if(!pxa255gpioInit(&soc->gpio, &soc->mem, &soc->ic)) ERR_("Cannot init PXA255's GPIO controller");
	if(!pxa255dmaInit(&soc->dma, &soc->mem, &soc->ic)) ERR_("Cannot init PXA255's DMA controller");
	if(!pxa255dspInit(&soc->dsp, &soc->cpu)) ERR_("Cannot init PXA255's cp0 DSP");
	if(!pxa255lcdInit(&soc->lcd, &soc->mem, &soc->ic)) ERR_("Cannot init PXA255's LCD controller");

printf("go?\n");

	pxa255uartSetFuncs(&soc->ffuart, socUartPrvRead, socUartPrvWrite, soc);	
}

void gdbCmdWait(SoC* soc, unsigned gdbPort, int* ss);

void socRun(SoC* soc, UInt32 gdbPort){
	
	
	printf("go2?\n");
	
	UInt32 prevRtc = 0;
	UInt32 cyclesCapt = 0;
	
	UInt32 cycles = 0;	//make 64 if you REALLY need it... later
	
	#ifdef GDB_SUPPORT
		int ss = 1;	//for gdb stub single step
	#else
		gdbPort = 0;	//use the param somehow to quiet GCC
	#endif
	
	printf("run !\n");
	
	while(soc->go){
		//printf("Soc go...\n");
		cycles++;

	#ifdef EMBEDDED	
		if(!(PIND & 0x10)){	//btn down
		
			if(!prevRtc){
			
				do{
					prevRtc = gRtc;
				}while(prevRtc != gRtc);
				cyclesCapt = 0;
			}
			else{
			
				UInt32 t;
				
				//we only care to go on if the rtc is now different
				do{
					t = gRtc;
				}while(t != gRtc);
				
				if(t != prevRtc){
				
					if(!cyclesCapt){
						
						//this code assumes we're called often enough that the next rtc vals we see is the NEXT second, not the one after or any other such thing
						cyclesCapt = cycles;
						prevRtc = t;
					}
					else{
					
						err_dec(cycles - cyclesCapt);
						err_str(" Hz\r\n");
					
						cyclesCapt = 0;
						prevRtc = 0;
					}
				}
			}
		}
	#endif		
		
		if(!(cycles & 0x00000FUL)) pxa255timrTick(&soc->timr);
		if(!(cycles & 0x0000FFUL)) pxa255uartProcess(&soc->ffuart);
		if(!(cycles & 0x000FFFUL)) pxa255rtcUpdate(&soc->rtc);
		if(!(cycles & 0x01FFFFUL)) pxa255lcdFrame(&soc->lcd);
		
		#ifdef GDB_SUPPORT
			gdbCmdWait(soc, gdbPort, &ss);
		#endif
		
		cpuCycle(&soc->cpu);
	}
}









#ifdef GDB_SUPPORT


	
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/select.h>
	#include <unistd.h>
	#include <errno.h>
	#include <stdlib.h>
	#include <netinet/in.h>
	#include <string.h>
	#include <stdio.h>
	
	
	
	static int socdBkptDel(SoC* soc, UInt32 addr, UInt8 sz){
		
		UInt8 i;
		
		for(i = 0; i < soc->nBkpt; i++){
			
			if(soc->bkpt[i] == addr){
				
				soc->nBkpt--;
				soc->bkpt[i] = soc->bkpt[soc->nBkpt];
				i--;
			}	
		}
		return 1;
	}
	
	
	static int socdBkptAdd(SoC* soc, UInt32 addr, UInt8 sz){	//boolean
		
		socdBkptDel(soc, addr, sz);
		
		if(soc->nBkpt == MAX_BKPT) return 0;
		
		soc->bkpt[soc->nBkpt++] = addr;
		
		return 1;
	}
	
	static int socdWtpDel(SoC* soc, UInt32 addr, UInt8 sz){
		
		UInt8 i;
		
		for(i = 0; i < soc->nWtp; i++){
			
			if(soc->wtpA[i] == addr && soc->wtpS[i] == sz){
				
				soc->nWtp--;
				soc->wtpA[i] = soc->wtpA[soc->nWtp];
				soc->wtpS[i] = soc->wtpS[soc->nWtp];
				i--;
			}	
		}
		return 1;
	}
	
	
	static int socdWtpAdd(SoC* soc, UInt32 addr, UInt8 sz){	//boolean
		
		socdWtpDel(soc, addr, sz);
		
		if(soc->nWtp == MAX_WTP) return 0;
		
		soc->wtpA[soc->nWtp] = addr;
		soc->wtpS[soc->nWtp] = sz;
		soc->nWtp++;
		
		return 1;
	}
	
	
	UInt32 htoi(const char** cP){
		
		UInt32 i = 0;
		const char* in = *cP;
		char c;
		
		while((c = *in) != 0){
			
			if(c >= '0' && c <= '9') i = (i * 16) + (c - '0');
			else if(c >= 'a' && c <= 'f') i = (i * 16) + (c + 10 - 'a');
			else if(c >= 'A' && c <= 'F') i = (i * 16) + (c + 10 - 'A');
			else break;
			in++;
		}
		
		*cP = in;
		
		return i;
	}
	
	static UInt32 swap32(UInt32 x){
		
		return ((x >> 24) & 0xff) | ((x >> 8) & 0xff00) | ((x & 0xff00) << 8) | ((x & 0xff) << 24);	
	}
	
	
	int gdb_memAccess(SoC* soc, UInt32 addr, UInt8* buf, int write){
		
		UInt32 pa = 0;
		UInt8 fsr = 0;
		
		return mmuTranslate(&soc->mmu, addr, true, false, &pa, &fsr) && memAccess(&soc->mem, pa, 1, write | 0x80, buf);
	}
	
	static int addRegToStr(SoC* soc, char* str, int reg){
		
		if(reg == 0x19 || reg < 0x10){
			if(reg == 0x19) reg = ARM_REG_NUM_CPSR;
			sprintf(str + strlen(str), "%08x", swap32(cpuGetRegExternal(&soc->cpu, reg)));
		}
		else if(reg >= 0x10 && reg < 0x18){
			
			strcat(str, "000000000000000000000000");
		}
		else if(reg == 0x18){	//fps
			
			strcat(str, "00000000");
		}
		else return 0;
		
		return 1;
	}
	
	static int interpPacket(SoC* soc,const char* in, char* out, int* ss){	//return 0 if we failed to interp a command, 1 is all ok, -1 to send no reply and run
		
		ArmCpu* cpu = &soc->cpu;
		unsigned char c;
		unsigned addr, len;
		unsigned char* ptr;
		int i;
		int ret = 1;
		
		
		
		if(strcmp(in, "qSupported") == 0){
			
			strcpy(out, "PacketSize=99");	
		}
		else if(strcmp(in, "vCont?") == 0){
			
			out[0] = 0;
		}
		else if(strcmp(in, "s") == 0){		//single step
			
			*ss = 1;
			return -1;
		}
		else if(strcmp(in, "c") == 0 || in[0] == 'C'){		//continue [with signal, which we ignore]
			
			return -1;
		}
		else if(in[0] == 'Z' || in[0] == 'z'){
			
			char op = in[0];
			char type = in[1];
			int (*f)(SoC* soc, UInt32 addr, UInt8 sz) = NULL;
			
			in += 3;
			addr = htoi(&in);
			if(*in++ != ',') goto fail;	//no comma?
			len = htoi(&in);
			if(*in) goto fail;		//string not over?
			
			if(type == '0' || type == '1'){	//bkpt
				
				f = (op == 'Z') ? socdBkptAdd : socdBkptDel;
			}
	/*
			else if(type == '2' || type == '3'){	//wtp
				
				f = (op == 'Z') ? socdWtpAdd : socdWtpDel;
			}
			else goto fail;
	*/
			strcpy(out,f(soc, addr, len) ? "OK" : "e00");
		}
		else if(in[0] == 'H' && (in[1] == 'c' || in[1] == 'g')){
			strcpy(out, "OK");	
		}
		else if(in[0] == 'q'){
			
			if(in[1] == 'C'){
				
				strcpy(out, "");	
			}
			else if(strcmp(in  +1, "Offsets") == 0){
				
				strcpy(out, "Text=0;Data=0;Bss=0");
			}
			else goto fail;
		}
		else if(in[0] == 'p'){	//read register
			
			in++;
			i = htoi(&in);
			if(*in) goto fail;	//string not over?
			
			out[0] = 0;
			if(!addRegToStr(soc, out, i)) goto fail;
		}
		else if(strcmp(in, "g") == 0){	//read all registers
			
			out[0] = 0;
			for(i = 0; i < 0x1a; i++) if(!addRegToStr(soc, out, i)) goto fail;
		}
		else if(in[0] == 'P'){	//write register
			
			in++;
			i = htoi(&in);
			if(*in++ != '=') goto fail;	//string not over?
			if(i == 0x19 || i <16){
				if(i == 0x19) i = ARM_REG_NUM_CPSR;
				addr = htoi(&in);
				sprintf(out, "OK");
				cpuSetReg(cpu, i, addr);
			}
			else strcpy(out,"e00");
		}
		else if(in[0] == 'm'){	//read memory
			
			in++;
			addr = htoi(&in);
			if(*in++ != ',') goto fail;
			len = htoi(&in);
			if(*in) goto fail;
			out[0] = 0;
			while(len--){
				
				if(!gdb_memAccess(soc, addr++, &c, false)) break;
				sprintf(out + strlen(out), "%02x", c);	
			}
		}
		else if(strcmp(in, "?") == 0){
			
			strcpy(out,"S05");	
		}
		else goto fail;
		
	send_pkt:
		return ret;
		
	fail:
		out[0] = 0;
		ret = 0;
		goto send_pkt;
	}
	
	static void sendpacket(int sock, char* packet, int withAck){
		
		unsigned int c;
		int i;
				
		c = 0;
		for(i = 0; i < strlen(packet); i++) c += packet[i];
		memmove(packet + (withAck ? 2 : 1), packet, strlen(packet) + 1);
		if(withAck){
			packet[0] = '+';
			packet[1] = '$';
		}
		else{
			packet[0] = '$';
		}
		sprintf(packet + strlen(packet), "#%02x", c & 0xFF);
		
		//printf("sending packet <<%s>>\n", packet);
		send(sock, packet, strlen(packet), 0);	
	}
	
	void gdbCmdWait(SoC* soc, unsigned gdbPort, int* ss){
		
		ArmCpu* cpu = &soc->cpu;
		static int running = 0;
		static int sock = -1;
		char packet[4096];
		struct timeval tv = {0};
		fd_set set;
		int ret;
		
		if(*ss && running){
			
			strcpy(packet,"S05");
			sendpacket(sock, packet, 0);
			running = 0;	//perform single step
		}
		*ss = 0;
		
		if(running){	//check for breakpoints
			
			UInt8 i;
			
			for(i = 0; i < soc->nBkpt; i++){
				
				if(soc->cpu.regs[15] == soc->bkpt[i]){
					
				//	printf("bkpt hit: pc=0x%08lX bk=0x%08lX i=%d\n", soc->cpu.regs[15], soc->bkpt[i], i);
					strcpy(packet,"S05");
					sendpacket(sock, packet, 0);
					running = 0;	//perform breakpoint hit
					break;
				}
			}
		}
		
		if(gdbPort){
			
			if(sock == -1){	//no socket yet - make one
				
				struct sockaddr_in sa = {AF_INET, htons(gdbPort)};
				socklen_t sl = sizeof(sa);
				
				inet_aton("127.0.0.1", &sa.sin_addr.s_addr);
				
				sock = socket(PF_INET, SOCK_STREAM, 0);
				if(sock == -1){
					err_str("gdb socket creation fails: ");
					err_dec(errno);
					ERR("\n");
				}
				
				ret = bind(sock, (struct sockaddr*)&sa, sizeof(sa));
				if(ret){
					err_str("gdb socket bind fails: ");
					err_dec(errno);
					ERR("\n");
				}
				
				ret = listen(sock, 1);
				if(ret){
					err_str("gdb socket listen fails: ");
					err_dec(errno);
					ERR("\n");
				}
				
				ret = accept(sock, (struct sockaddr*)&sa, &sl);
				if(ret == -1){
					err_str("gdb socket accept fails: ");
					err_dec(errno);
					ERR("\n");
				}
				close(sock);
				sock = ret;
				
				soc->nBkpt = 0;
				soc->nWtp = 0;
			}
		}
		if(gdbPort){
				
			do{
		
				FD_ZERO(&set);
				FD_SET(sock, &set);
				tv.tv_sec = running ? 0 : 0x00f00000UL;
				do{
					ret = select(sock + 1, &set, NULL, NULL, &tv);
				}while(!ret && !running);
				if(ret < 0){
					err_str("select fails: ");
					err_dec(errno);
					ERR("\n");
				}
				if(ret > 0){
					char c;
					char* p;
					int i, len = 0, esc = 0, end = 0;
					
					ret = recv(sock, &c, 1, 0);
					if(ret != 1) ERR("failed to receive byte (1)\n");
					
					if(c == 3){
						strcpy(packet,"S11");
						sendpacket(sock, packet, 0);
						running = 0;	//perform breakpoint hit
					}
					else if(c != '$'){
						//printf("unknown packet header '%c'\n", c);
					}
					else{
						do{
							if(esc){
								c = c ^ 0x20;
								esc = 0;
							}
							else if(c == 0x7d){
								esc = 1;
							}
							
							if(!esc){	//we cannot be here if we're being escaped
								
								packet[len++] = c;
								if(end == 0 && c == '#') end = 2;
								else if(end){
									
									end--;
									if(!end) break;
								}
								
								ret = recv(sock, &c, 1, 0);
								if(ret != 1) ERR("failed to receive byte (2)\n");
							}
						}while(1);
						packet[len] = 0;
						
						memmove(packet, packet + 1, len);
						len -= 4;
						packet[len] = 0;
						ret = interpPacket(soc, p = strdup(packet), packet, ss);
						if(ret == 0) printf("how do i respond to packet <<%s>>\n", p);
						if(ret == -1){	//ack it anyways
							char c = '+';
							send(sock, &c, 1, 0);
							running = 1;
						}
						else sendpacket(sock, packet, 1);
						
						emu_free(p);
					}
				}
			}while(!running);
		}
	}

#endif
