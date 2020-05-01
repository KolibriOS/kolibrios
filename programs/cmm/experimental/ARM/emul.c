// version 0.03
// Author: Pavel Iakovlev
// http://shell-storm.org/online/Online-Assembler-and-Disassembler/?inst=&arch=arm#assembly - online compiler (Little endian:)


#pragma option OST
#pragma option ON
#pragma option cri-
#pragma option -CPA
#initallvar 0
#jumptomain FALSE

#startaddress 0x10000

#code32 TRUE

char   os_name[8]   = {'M','E','N','U','E','T','0','1'};
dword  os_version   = 0x00000001;
dword  start_addr   = #main;
dword  final_addr   = #______STOP______+32;
dword  alloc_mem    = 20000;
dword  x86esp_reg   = 20000;
dword  I_Param      = #param;
dword  I_Path       = #program_path;
char param[4096] ={0};
char program_path[4096] = {0};

// test opcode arm, compiler (http://shell-storm.org/online/Online-Assembler-and-Disassembler/?inst=mov+r0%2C1%0D%0Amov+r5%2C2%0D%0Amov+r2%2C+r0%2C+lsl+r5&arch=arm#assembly) (Little endian:)

dword test_bytecode = "\x04\x10\x5f\xe5\x7b\x00\x00\x00"; 

// --------------------

struct _reg // registers arm
{
	dword r0;
	dword r1;
	dword r2;
	dword r3;
	dword r4;
	dword r5;
	dword r6;
	dword r7;
	dword r8;
	dword r9;
	dword r10;
	dword r11;
	dword r12; // (Intra-Procedure-call scratch register)
	dword r13; // (Stack Pointer)
	dword r14; // (Link Register)
	dword r15; // PC (Program Counter)
};

_reg reg = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // clear and init registers
dword REG = #reg;

struct _flags
{
	byte negative;
	byte zero;
	byte carry;
	byte overflow;
};

_flags flags = {0,0,0,0}; // clear and init flags

struct _mode
{
	byte User;
	byte FastInterrupt;
	byte Interrupt;
	byte Supervisor;
};

_mode mode = {0,0,0,0}; // processor mode 

struct _mask 
{
	byte IRQ;
	byte FIRQ;
};

_mask mask = {0,0}; // processor mask

void main()
{

	callOpcode(#test_bytecode,1);
	
	EAX = -1;
	$int 0x40;
}

dword callOpcode(dword binary, lengthInstruction)
{
	dword command = 0;
	dword PC = 0;
	byte flag = 0;
	byte pMask = 0;
	byte pMode = 0;
	while(lengthInstruction)
	{
		//PC    = reg.r15 >> 2 & 0xFFFFFF;
		flag  = reg.r15 >> 28;
		pMask = reg.r15 >> 26;
		
		flags.negative = flag & 0x8;
		flags.zero     = flag & 0x4;
		flags.carry    = flag & 0x2;
		flags.overflow = flag & 0x1;
		
		mask.IRQ  = pMask & 0x2;
		mask.FIRQ = pMask & 0x1;
		
		switch(reg.r15 & 3)
		{
			case 0:
				DSDWORD[#mode] = 0x000000FF;
			break;
			case 1:
				DSDWORD[#mode] = 0x0000FF00;
			break;
			case 2:
				DSDWORD[#mode] = 0x00FF0000;
			break;
			case 3:
				DSDWORD[#mode] = 0xFF000000;
			break;
		}
		
		command = DSDWORD[binary + PC]; // generation PC instruction
		//EAX = DSDWORD[command >> 28 << 2 + #opcodeExec]; // get opcodeExecition call instruction
		//EAX(command); // call opcodeExecition
		//IF (command & 0xC000000 == 0) opcodeExec0(command);
		if (command & 0x0FFFFFF0 == 0x12FFF10) BranchExchange(command);
		else if (command & 0x0FF00FF0 == 0x1000090) SingleDataSwap(command);
		else if (command & 0x0FC000F0 == 0x0000090) Multiply(command);
		else if (command & 0x0FC000F0 == 0x0800090) MultiplyLong(command);
		else if (command & 0x0C000000 == 0x0000000) DataProcessing(command);
		else if (command & 0xE000010 == 0x6000010) ;// undefined
		else if (command & 0xC000000 == 0x4000000) SingleDataTransfer(command, binary);
		
		PC += 4; // addition 4 for reg15 or PC instruction
		//PC <<= 2;
		
		flag = 0;
		IF (flags.negative) flag |= 0x8;
		IF (flags.zero)     flag |= 0x4;
		IF (flags.carry)    flag |= 0x2;
		IF (flags.overflow) flag |= 0x1;
		
		pMask = 0;
		IF (mask.IRQ)  pMask |= 0x2;
		IF (mask.FIRQ) pMask |= 0x1;
		
		if (mode.User)               pMode = 0;
		else IF (mode.FastInterrupt) pMode = 1;
		else IF (mode.Interrupt)     pMode = 2;
		else IF (mode.Supervisor)    pMode = 3;
		
		//reg.r15 = flag << 28 | PC | pMode;
		lengthInstruction--;
	}
}

dword Multiply(dword command)
{
	
}

dword MultiplyLong(dword command)
{
	
}

dword SingleDataSwap(dword command)
{
	
}

dword BranchExchange(dword command)
{
	
}

dword SingleDataTransfer(dword command, binary) 
{
	dword Rd = #reg;
	dword Rn = #reg;
	dword offset = 0;

	Rd += command >> 12 & 0xF << 2;
	Rn += command >> 16 & 0xF << 2;
	offset = command & 0xFFF;
	IF (command >> 16 & 0xF != 15) IF (command & 0x800000 == 0) $neg offset;
	
	IF (command & 0x400000) // byte
	{
		IF (command >> 16 & 0xF == 15) 
		{
			IF (command & 0x100000) DSDWORD[Rd] = DSBYTE[binary + offset];
			ELSE DSBYTE[binary + offset] = DSDWORD[Rd];
			
		}
		ELSE 
		{
			Rn = DSDWORD[Rn];
			IF (command & 0x2000000 == 0) Rn += offset;
			IF (command & 0x100000) DSDWORD[Rd] = DSDWORD[binary + Rn];
			ELSE DSDWORD[binary + Rn] = DSDWORD[Rd];
		}
	}
	ELSE // dword
	{
		Rn = DSDWORD[Rn];
		IF (command & 0x2000000 == 0) Rn += offset;
		IF (command & 0x100000) DSDWORD[Rd] = DSDWORD[binary + Rn];
		ELSE DSDWORD[binary + Rn] = DSDWORD[Rd];
	}

}

dword DataProcessing(dword command) // Data Processing / PSR Transfer
{
	dword opcode = 0;
	dword Rd = #reg;
	dword Rn = #reg;
	dword operand = 0;
	word sdvig = 0;
	word context = 0;
	byte typeSdvig = 0;
	opcode = command >> 21 & 0xF;
	Rd += command >> 12 & 0xF << 2;
	Rn += command >> 16 & 0xF << 2;
	context = command & 0xFFF;

	IF (command & 0x2000000) operand = context;
	ELSE operand = DSDWORD[context & 1111b << 2 + #reg];
	
	typeSdvig = context >> 5 & 11b;
	IF (context & 10000b) sdvig = DSBYTE[context >> 8 & 1111b << 2 + #reg];
	ELSE sdvig = context >> 7 & 11111b;

	switch (typeSdvig) // type sdvig
	{
		case 0: // logic left
			operand <<= sdvig;
		break;
		case 1: // logic right
			operand >>= sdvig;
		break;
		case 2: // arifmetic left
			
		break;
		case 3: // arifmetic right
			
		break;
	}
	
	switch (opcode)
	{
		case 0: // and
			DSDWORD[Rd] = DSDWORD[Rn] & operand;
		break;
		case 1: // eor
			DSDWORD[Rd] = DSDWORD[Rn] | operand;
		break;
		case 2: // sub
			DSDWORD[Rd] = DSDWORD[Rn] - operand;
		break;
		case 3: // rsb
			DSDWORD[Rd] = operand - DSDWORD[Rn];
		break;
		case 4: // add
			DSDWORD[Rd] = DSDWORD[Rn] + operand;
		break;
		case 5: // adc
			DSDWORD[Rd] = DSDWORD[Rn] + operand;
		break;
		case 6: // sbc
			
		break;
		case 7: // rsc
			
		break;
		case 8: // tst
			
		break;
		case 9: // teq
			
		break;
		case 10: // cmp 
			
		break;
		case 11: // cmn 
			
		break;
		case 12: // orr
			DSDWORD[Rd] = DSDWORD[Rn] | operand;
		break;
		case 13: // mov
			DSDWORD[Rd] = operand;
		break;
		case 14: // bic
			$not operand;
			DSDWORD[Rd] = DSDWORD[Rn] & operand;
		break;
		case 15: // mvn 
			DSDWORD[Rd] = DSDWORD[Rn] + operand;
		break;
	}
	
}



______STOP______: