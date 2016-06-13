#define _POINTER_

#include "tok.h"

void dopointerproc()
{
unsigned int snum;
int razr=r16;
ITOK wtok;
char *wbuf;
SINFO wstr;
	if(am32)razr=r32;
	itok.rm=itok.sib;
	itok.sib=am32==0?CODE16:CODE32;
	if((FixUp)

	&&((itok.rm==rm_d32&&am32)||(itok.rm==rm_d16&&am32==0)))//16.07.04 16:01
	//18.08.04 18:03  itok.rm==rm_d32 was itok.rm==CODE32

	itok.flag|=f_reloc;
	else compressoffset(&itok);
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	unsigned int oaddESP=addESP;
	if(wtok.npointr<2){
		itok.flag&=~f_far;
//		itok.post=0;
		snum=initparamproc();
		CheckAllMassiv(wbuf,razr,&wstr,&wtok);
		outseg(&wtok,2);
		op(0xFF); op(0x10+wtok.rm+((wtok.flag&f_far)!=0?8:0));
		outaddress(&wtok);
	}
	else{
int reg=BX;
		getpointeradr(&wtok,wbuf,&wstr,wtok.npointr-1,am32==0?r16:r32,reg);
		snum=initparamproc();
		op66(razr);
		op(0xFF);
		op(0xD0+reg); 	/* CALL reg with stack params */
	}
	if((wtok.flag&f_typeproc)==tp_cdecl)CorrectStack(snum);
	else addESP=oaddESP;
	clearregstat(0);
}

void dopointer()
{
	if(itok.type==tk_proc){
		if(tok2==tk_openbracket){
			dopointerproc();
			nexttok();
			return;
		}
		itok.rm=itok.sib;
		itok.sib=am32==0?CODE16:CODE32;
		compressoffset(&itok);
	}
int razr=r16;
	if(am32||(itok.flag&f_far))razr=r32;
	do_d_wordvar(0,razr);
}

void getpointeradr(ITOK *gstok,char *&gbuf,SINFO *gstr,int numpointer,int razr, int reg)
{
	if(gstok->flag&f_far){
		CheckAllMassiv(gbuf,4,gstr,gstok);
		outseg(gstok,2);
		op(0xC4);
		op(gstok->rm+8*reg);
		outaddress(gstok);
		itok.segm=ES;
		itok.sib=CODE16;
		if(reg==BX)itok.rm=rm_BX;
		else if(reg==SI)itok.rm=rm_SI;
		else if(reg==DI)itok.rm=rm_DI;
		warningreg(segs[0]);
	}
	else{
		int retreg;
		if((gstok->flag&f_useidx)==0&&(retreg=CheckIDZReg(gstok->name,reg,razr))!=NOINREG){
			if(am32==0&&retreg!=BX&&retreg!=SI&&retreg!=DI){
				GenRegToReg(reg,retreg,razr);
				IDZToReg(gstok->name,reg,razr);
			}
			else if(retreg!=SKIPREG)reg=retreg;
			if(gbuf){
				free(gbuf);
				gbuf=NULL;
			}
			if(gstr->bufstr){
				free(gstr->bufstr);
				gstr->bufstr=NULL;
			}
			goto nomov;
		}
		if((gstok->flag&f_useidx))IDZToReg(gstok->name,reg,razr);
		CheckAllMassiv(gbuf,razr,gstr,gstok);
		outseg(gstok,2);
		op(0x8B);
		op(reg*8+gstok->rm);
		outaddress(gstok);
nomov:
		itok.segm=DS;
		if(am32==0){
			itok.sib=CODE16;
			if(reg==BX)itok.rm=rm_BX;
			else if(reg==SI)itok.rm=rm_SI;
			else if(reg==DI)itok.rm=rm_DI;
		}
		else{
			itok.sib=CODE32;
			itok.rm=reg;
		}
	}
	itok.post=0;
	itok.number=0;
	itok.flag=gstok->flag&(!f_reloc);
	warningreg(regs[am32][reg]);
	while(numpointer){
		outseg(&itok,2);
		op(0x8B);
		op(itok.rm+reg*8);
		outaddress(&itok);
		numpointer--;
	}
}

void dovalpointer()
{
int sign=0,rettype,pointr,razr=r8;
int rrettype;
int numpointer=0;
int hnumber=0;
//int i=0;
int reg=BX;
int npointr=0;
	do{
		nexttok();
		numpointer++;
	}while(tok==tk_mult);
ITOK wtok;
char *wbuf;
SINFO wstr;
unsigned char ocha;
unsigned int oinptr;
unsigned int olinenum;
int otok2;
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	if(numpointer==itok.npointr){
		switch(itok.type){
			case tk_qword:
			case tk_double:
				razr+=4;
			case tk_dword:
			case tk_float:
				razr+=2;
			case tk_word:
				razr++;
				break;
			case tk_long:
				razr+=2;
			case tk_int:
				razr++;
			case tk_char:
				sign=1;
				break;
		}
		rrettype=rettype=itok.type;
		oinptr=inptr2;
		ocha=cha2;
		olinenum=linenumber;
		otok2=tok2;
		nexttok();
		if(tok==tk_assign){
			nexttok();
			convert_type(&sign,(int *)&rettype,&pointr);
			while(tok==tk_mult){
				nexttok();
				npointr++;
			}
			if(npointr>itok.npointr)unuseableinput();
			if(tok2==tk_assign){
				hnumber=MultiAssign(razr,USEALLREG,npointr);
				if(reg==hnumber)reg=DI;
				goto getfromax;
			}
		}
		linenumber=linenum2=olinenum;
		cha2=ocha;
		inptr2=oinptr;
		tok2=otok2;
		itok=wtok;
		if(itok.type==tk_proc){
			dopointer();
			return;
		}
		getpointeradr(&itok,wbuf,&wstr,numpointer-1,razr,reg);
		tok=tk_charvar+itok.type-tk_char;
		switch(itok.type){
			case tk_long:
			case tk_dword:
			case tk_int:
			case tk_word:
				do_d_wordvar(sign,razr);
				return;
			case tk_char:
			case tk_byte:
				dobytevar(sign);
				return;
			case tk_float:
				dofloatvar();
				return;
			case tk_qword:
				doqwordvar();
				return;
			case tk_double:
//				dodoublevar();
				return;
		}
getfromax:
		convert_returnvalue(rrettype,rettype);
ITOK otok=itok;
		getpointeradr(&wtok,wbuf,&wstr,numpointer-1,razr,reg);
		switch(rrettype){
			case tk_char:
			case tk_byte:
				outseg(&itok,2);
				op(0x88);
				op(itok.rm+hnumber*8);	//[reg]=AL
				break;
			case tk_int:
			case tk_word:
			case tk_long:
			case tk_dword:
			case tk_qword:
				op66(razr);
				outseg(&itok,2);
				op(0x89);
				op(itok.rm+hnumber*8); // MOV [rmword],AX
				break;
			case tk_float:
				outseg(&itok,2);	//fstp var
				op(0xd9);
				op(itok.rm+0x18);
				fwait3();
				break;
			case tk_double:
				outseg(&itok,2);	//fstpq var
				op(0xdd);
				op(itok.rm+0x18);
				fwait3();
				break;
		}
		itok=otok;
	}
	else if(numpointer<itok.npointr){
		razr=r16;
		rettype=tk_word;
		if(am32||(itok.flag&f_far)){
			razr=r32;
			rettype=tk_dword;
		}
		rrettype=rettype;
		doreg_32(AX,razr);
		goto getfromax;
	}
	else unuseableinput();
}

