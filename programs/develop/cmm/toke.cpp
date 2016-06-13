#define _TOKE_

#include <fcntl.h>
#include <unistd.h>
#include "tok.h"


unsigned char gotoendif=FALSE;

unsigned char atex=FALSE;

unsigned char usedirectiv=TRUE;	//идет обработка директивы

unsigned char parsecommandline = 0; 	// parse command line flag

unsigned char sdp_mode=FALSE;	//режим принудительной выгрузки динамических процедур

unsigned int startexit;

extern int	maxerrors; 				// number of errors to stop at

unsigned int postnumflag;	//флаг последнего идентификатора в вычислении номера

int calcnumber=FALSE;



char mesmain[]="main";

char *macroname[]={"inp","inportb","inport","inportd","outp","outportb",

                   "outport","outportd","sqrt","cos","sin","atan2",

									 "tan","log","log10","exp","atan","fabs",NULL};

enum{m_ib,m_ibt,m_iw,m_id,m_ob,m_obt,m_ow,m_od,m_sqrt,m_cos,m_sin,m_atan2,

     m_tan,m_log,m_log10,m_exp,m_atan,m_fabs,m_end};



int FindProcLib(int);

int RetAtExit();

int ConvRetCode(int i);

int CompConst(int firstval);

int dirmode;



#define NUMIFDEF 32	//максимальная вложеность директив ifdef/ifndef

int	endifcount=-1; 		 // depth count of ?if

char ujo[]="Unknown #jumptomain option";

char toelse[]="#else use once in #if";

char ido[]="Unknown #inline option";

unsigned char startuptomain=FALSE;

unsigned char dosstring=FALSE;

unsigned char useelse[NUMIFDEF];	//флаги использования директивы else

unsigned char jumptomain = CALL_NEAR; // jump to the main()

unsigned char resizemem = 1;					// set owned memory block to 64K

unsigned char fargc=FALSE;

unsigned int	startptrdata = 0x100; 	// data start address

unsigned int resizesizeaddress;  /* location of resize memory size descr. */

unsigned int stackstartaddress;  /* location of SP assignment */



/*-----------------18.09.98 23:20-------------------

 Реализация SYS

--------------------------------------------------*/

char sysname[8]="NO_NAME";

int sysatr=0x2000;

int sysstack=0;

int sysnumcom=0;

int syscom;

//переменные для компиляции rom-bios

int unsigned romsize=0;

int dataromstart,dataromsize;

int dataseg=0x70;



unsigned int numdomain=0;	//число процедур запускаемых до main

char *domain;	//буфер имен процедур запускаемых до main



int ifdefconst();

void CheckNumIF();

void KillVarOfTree(idrec **treestart);

void IncludeFile(char *fileincl,int tfind);



char *pragmalist[]={"option","line","startup","resource","pack","debug","indexregs",""};

enum{p_op,p_li,p_st,p_re,p_pa,p_db,p_idx,p_end};

int strpackdef=1;

int strpackcur=1;

struct STACKALIGN {

	STACKALIGN *prev;

	int size;

	char id[IDLENGTH];

};

#ifdef DEBUGMODE

int debug=FALSE;

#endif



STACKALIGN *stackalign=NULL;

/* -------------- constant math procedures start --------------- */



int calcqwordnumber(unsigned long long *retnum,unsigned long long number,int operand)

{

unsigned long long value;

	value=*retnum;

	switch(operand){

		case tk_minus: value-=number; break;

		case tk_plus: value+=number; break;

		case tk_xor: value^=number; break;

		case tk_and: value&=number; break;

		case tk_or: value|=number; break;

		case tk_mod: value%=number; break;

		case tk_div: value/=number; break;

		case tk_mult: value*=number; break;

		case tk_rr: value>>=number; break;

		case tk_ll: value<<=number; break;

		case tk_xorminus: value^=-number; break;

		case tk_andminus: value&=-number; break;

		case tk_orminus: value|=-number; break;

		case tk_modminus: value%=-number; break;

		case tk_divminus: value/=-number; break;

		case tk_multminus: value*=-number; break;

		case tk_rrminus: value>>=-number; break;

		case tk_llminus: value<<=-number; break;

		default: return FALSE;

	}

	*retnum=value;

	return TRUE;

}



unsigned long long doconstqwordmath()

{

unsigned long long value;

unsigned int htok;

int fundef;	//флаг использования неизв адреса

	CheckMinusNum();

	if(tok!=tk_number){

		numexpected();

		return(0);

	}

	if(itok.rm==tk_float)value=*(float *)&itok.number;

	else if(itok.rm==tk_double)value=*(double *)&itok.lnumber;

	else value=itok.lnumber;

	fundef=itok.rm;

	postnumflag=itok.flag;

//	usedirectiv=TRUE;

	calcnumber=TRUE;

	while(itok2.type==tp_opperand){

		if(fundef==tk_undefofs&&tok2!=tk_plus&&tok2!=tk_minus)break;

		nexttok();

		htok=tok;

		if(tok2!=tk_number){

			if(tok2!=tk_dollar&&tok2!=tk_not){

				calcnumber=FALSE;

				return(value);

			}

			nexttok();

			if(tok!=tk_number){

				calcnumber=FALSE;

				return(value);

			}

		}

		else nexttok();

		if(itok.rm==tk_float)itok.number=*(float *)&itok.number;

		else if(itok.rm==tk_double)itok.lnumber=*(double *)&itok.lnumber;

		if(calcqwordnumber(&value,itok.lnumber,htok)==FALSE)beep();

		postnumflag^=itok.flag;

	}

	nexttok();

	calcnumber=FALSE;

	return(value);

}



int calcdwordnumber(unsigned long *retnum,unsigned long number,int operand)

{

unsigned long value;

	value=*retnum;

	switch(operand){

		case tk_minus: value-=number; break;

		case tk_plus: value+=number; break;

		case tk_xor: value^=number; break;

		case tk_and: value&=number; break;

		case tk_or: value|=number; break;

		case tk_mod: value%=number; break;

		case tk_div: value/=number; break;

		case tk_mult: value*=number; break;

		case tk_rr: value>>=number; break;

		case tk_ll: value<<=number; break;

		case tk_xorminus: value^=-number; break;

		case tk_andminus: value&=-number; break;

		case tk_orminus: value|=-number; break;

		case tk_modminus: value%=-number; break;

		case tk_divminus: value/=-number; break;

		case tk_multminus: value*=-number; break;

		case tk_rrminus: value>>=-number; break;

		case tk_llminus: value<<=-number; break;

		default: return FALSE;

	}

	*retnum=value;

	return TRUE;

}



unsigned long doconstdwordmath()

{

unsigned long value;

unsigned int htok;

int fundef;	//флаг использования неизв адреса

	CheckMinusNum();

	if(tok!=tk_number){

		numexpected();

		return(0);

	}

	if(itok.rm==tk_float)value=*(float *)&itok.number;

	else value=itok.number;

	fundef=itok.rm;

	postnumflag=itok.flag;

//	usedirectiv=TRUE;

	calcnumber=TRUE;

	while(itok2.type==tp_opperand){

		if(fundef==tk_undefofs&&tok2!=tk_plus&&tok2!=tk_minus)break;

		nexttok();

		htok=tok;

		if(tok2!=tk_number){

			if(tok2!=tk_dollar&&tok2!=tk_not){

				calcnumber=FALSE;

				return(value);

			}

			nexttok();

			if(tok!=tk_number){

				calcnumber=FALSE;

				return(value);

			}

		}

		else nexttok();

		if(itok.rm==tk_float)itok.number=*(float *)&itok.number;

		if(itok.rm==tk_double)itok.number=*(double *)&itok.lnumber;

		if(calcdwordnumber(&value,itok.number,htok)==FALSE)beep();

		postnumflag^=itok.flag;

	}

	nexttok();

	calcnumber=FALSE;

	return(value);

}



int calclongnumber(long *retnum,long number,int operand)

{

long value;

	value=*retnum;

	switch(operand){

		case tk_minus: value-=number; break;

		case tk_plus: value+=number; break;

		case tk_xor: value^=number; break;

		case tk_and: value&=number; break;

		case tk_or: value|=number; break;

		case tk_mod: value%=number; break;

		case tk_div: value/=number; break;

		case tk_mult: value*=number; break;

		case tk_rr: value>>=number; break;

		case tk_ll: value<<=number; break;

		case tk_xorminus: value^=-number; break;

		case tk_andminus: value&=-number; break;

		case tk_orminus: value|=-number; break;

		case tk_modminus: value%=-number; break;

		case tk_divminus: value/=-number; break;

		case tk_multminus: value*=-number; break;

		case tk_rrminus: value>>=-number; break;

		case tk_llminus: value<<=-number; break;

		default: return FALSE;

	}

	*retnum=value;

	return TRUE;

}



signed long doconstlongmath()

//вычислить выражение

{

long value;

unsigned int htok;

int fundef;	//флаг использования неизв адреса

	CheckMinusNum();

	if(tok!=tk_number){

		numexpected();

		return(0);

	}

	fundef=itok.rm;

	if(itok.rm==tk_float)value=*(float *)&itok.number;

	else value=itok.number;

//	value=itok.number;

	postnumflag=itok.flag;

//	usedirectiv=TRUE;

	calcnumber=TRUE;

	while(itok2.type==tp_opperand){	//пока операнд

		if(fundef==tk_undefofs&&tok2!=tk_plus&&tok2!=tk_minus)break;

		nexttok();

//		printf("tok=%d tok2=%d\n",tok,tok2);

		htok=tok;

		if(tok2!=tk_number){

			if(tok2!=tk_dollar&&tok2!=tk_not){

				calcnumber=FALSE;

				return(value);

			}

			nexttok();

			if(tok!=tk_number){

				calcnumber=FALSE;

				return(value);

			}

		}

		else nexttok();

		if(itok.rm==tk_float)itok.number=*(float *)&itok.number;

		if(itok.rm==tk_double)itok.number=*(double *)&itok.lnumber;

		if(calclongnumber(&value,itok.number,htok)==FALSE)beep();

		postnumflag^=itok.flag;

	}

	nexttok();

	calcnumber=FALSE;

	return(value);

}



int calcfloatnumber(float *retnum,float number,int operand)

{

float value;

	value=*retnum;

	switch(operand){

		case tk_minus: value-=number; break;

		case tk_plus: value+=number; break;

		case tk_div: value/=number; break;

		case tk_mult: value*=number; break;

		case tk_divminus: value/=-number; break;

		case tk_multminus: value*=-number; break;

		default: return FALSE;

	}

	*retnum=value;

	return TRUE;

}



long doconstfloatmath()

//вычислить выражение

{

float value;

	postnumflag=0;

	CheckMinusNum();

	if(tok!=tk_number){

		numexpected();

		return(0);

	}

	if(itok.rm==tk_double)*(float *)&itok.number=itok.dnumber;

	else if(itok.rm!=tk_float){

		float temp=itok.number;

		*(float *)&itok.number=temp;

	}

	value=itok.fnumber;

//	usedirectiv=TRUE;

	calcnumber=TRUE;

	while(itok2.type==tp_opperand){	//пока операнд

		nexttok();

		if(tok2!=tk_number){

			calcnumber=FALSE;

			return *(long *) &value;//нет никаких действий

		}

		if(itok2.rm==tk_double)*(float *)&itok2.number=itok2.dnumber;

		else if(itok2.rm!=tk_float)*(float *)&itok2.number=itok2.number;

		itok2.rm=tk_float;

		if(calcfloatnumber(&value,itok2.fnumber,tok)==FALSE)beep();

		nexttok();

	}

	nexttok();

	calcnumber=FALSE;

	return *(long *) &value;

}



int calcdoublenumber(double *retnum,double number,int operand)

{

double value;

	value=*retnum;

	switch(operand){

		case tk_minus: value-=number; break;

		case tk_plus: value+=number; break;

		case tk_div: value/=number; break;

		case tk_mult: value*=number; break;

		case tk_divminus: value/=-number; break;

		case tk_multminus: value*=-number; break;

		default: return FALSE;

	}

	*retnum=value;

	return TRUE;

}



long long doconstdoublemath()

//вычислить выражение

{

double value;

	postnumflag=0;

	CheckMinusNum();

	if(tok!=tk_number){

		numexpected();

		return(0);

	}

	if(itok.rm==tk_float){

		itok.dnumber=*(float *)&itok.number;

		itok.rm=tk_double;

	}

	else if(itok.rm!=tk_double){

		itok.dnumber=itok.lnumber;

	}

	value=itok.dnumber;

//	usedirectiv=TRUE;

	calcnumber=TRUE;

	while(itok2.type==tp_opperand){	//пока операнд

		nexttok();

		if(tok2!=tk_number){

			calcnumber=FALSE;

			return *(long long *) &value;//нет никаких действий

		}

		if(itok2.rm==tk_float)itok2.dnumber=*(float *)&itok2.number;

		else if(itok2.rm!=tk_double)itok2.dnumber=itok2.lnumber;;

		itok2.rm=tk_double;

		if(calcdoublenumber(&value,itok2.dnumber,tok)==FALSE)beep();

		nexttok();

	}

	nexttok();

	calcnumber=FALSE;

	return *(long long *) &value;

}



/* ================= simple syntax procedures start =================== */



void nextseminext()

{

	nexttok();

	seminext();

}



void seminext()

{

	if(tok!=tk_semicolon)expected(';');

	else nexttok();

}



void beep() 			 /* beep for any internal errors */

{

	printf("\a");

}



int expecting(int want)

/* compares current token with want token.	If different, issues error

	 message and returns 1, else advances to next token and returns 0 */

{

	if(want!=tok){

		SwTok(want);

		return(1);

	}

	nexttok();

	return(0);

}



void expectingoperand(int want)

/* compares current token with want token.	If different, issues error

	 message and returns 1, else advances to next token and returns 0 */

{

	if(want!=tok)SwTok(want);

	else getoperand();

}



void SwTok(int want)

{

	switch(want){

		case tk_closebracket: expected(')'); break;

		case tk_openbracket: expected('('); break;

		case tk_semicolon: expected(';'); break;

		case tk_colon: expected(':'); break;

		case tk_openblock: expected('['); break;

		case tk_closeblock: expected(']'); break;

		case tk_openbrace: expected('{'); break;

		case tk_closebrace: expected('}'); break;

		case tk_camma: expected(','); break;

		default: preerror("expecting a different token"); break;

	}

}



/*-----------------03.07.99 22:48-------------------

 Внутренние процедуры

	--------------------------------------------------*/

void  outprocedure(unsigned char *array,unsigned int length)

{

	if((outptr+length)>=outptrsize)CheckCodeSize();

	memcpy(output+outptr,array,length);

	outptr+=length;

	if(splitdata==0)outptrdata=outptr;

}



#define MMBANER 11

unsigned char aabaner[]={

	0x53,0x50,0x48,0x49,0x4E,0x58,0x43,0x2d,0x2d,ver1,ver2};	//надпись SPHINXC--ver



#define MMEXP 18

unsigned char aaEXP[]={

	0xD9,0xE8,		 //	fld1

	0xD9,0xEA,		 //fldl2e

	0xD8,0xCA,		 //fmul	 st, st(2)

	0xDD,0xD2,		 //fst	 st(2)

	0xD9,0xF8,		 //fprem

	0xD9,0xF0,		 //f2xm1

	0xDE,0xC1,		 //faddp	 st(1),	st

	0xD9,0xFD,		 //fscale

	0xDD,0xD9 };	 //fstp	 st(1)



void CallExitProcess()

//вызов процедуры ExitProcess

{

	tok=tk_id;

	searchvar("ExitProcess");

	if(FastCallApi==TRUE){

		outword(0x15FF);

		AddApiToPost(itok.number);

	}

	else{

		addacall((unsigned int)itok.number,(unsigned char)CALL_32);

		callloc0();		/* produce CALL [#] */

	}

}



int includeit(int type)

{

int i=0;

	itok.post=1;

	if(strcmp("ABORT",itok.name)==0){

		RestoreStack();

		clearregstat();

#ifdef OPTVARCONST

		ClearLVIC();

#endif

		type=0;

		if(type==1&&dbg&2)AddCodeNullLine("ABORT()");

		if(comfile==file_w32)outword(0xC031);	//xor eax,eax

		if(atex==TRUE)i=RetAtExit();

		if(i==0){

			if(comfile==file_exe)outdword(0x21CD4CB4);

			else if(comfile==file_w32){

				if(jumptomain!=CALL_NONE)jumploc(startexit);

				else{

					op(0x50);

					CallExitProcess();

				}

			}

			else outword(0x20CD);

		}

		retproc=TRUE;

	}

	else if(strcmp("ATEXIT",itok.name)==0){

		if(type&&AlignProc)AlignCD(CS,alignproc);

		if(atex==FALSE)preerror("?atexit must be set for ATEXIT");

		if(type==1&&dbg&2)AddCodeNullLine("ATEXIT()");

		searchvar("__atexitproc");

		itok2=itok;

		searchvar("__numatexit");

		if(am32){

			op(0x51);	//push ecx

			outword(0x0D8B);	//mov ecx,numatex

			if(itok.post)setwordpost(&itok);

			outdword(itok.number);

			outdword(0x7310F983);

			outdword(0x8D04890F);  //cmp ecx,10 jnb bp mov [numatexit+4+ECX*4]=EAX

			if(itok2.post)setwordpost(&itok2);

			outdword(itok2.number);

			outword(0x05FF);	//inc numatexit

			if(itok.post)setwordpost(&itok);

			outdword(itok.number);

			outword(0xC031);	//xor eax,eax

			//bp:

			op(0x59);	//pop ECX

		}

		else{

			outword(0x368B);

			if(itok.post)setwordpost(&itok);

			outword(itok.number);

			op(0x83);

			outdword(0x0C7310FE);	//cmp si,10  jnb

			outdword(0x8489F601);	//si+=si

			if(itok2.post)setwordpost(&itok2);

			outword(itok2.number);

			outword(0x06FF);

			if(itok.post)setwordpost(&itok);

			outword(itok.number);

			outword(0xC031);

		}

	}

	else if(strcmp("EXIT",itok.name)==0){

		RestoreStack();

		clearregstat();

#ifdef OPTVARCONST

		ClearLVIC();

#endif

		type=0;

		if(dbg&2)AddCodeNullLine("EXIT()");

		if(atex==TRUE)i=RetAtExit();

		if(i==0){

			if(comfile==file_w32){

				if(jumptomain!=CALL_NONE)jumploc(startexit);

				else{

					op(0x50);

					CallExitProcess();

				}

			}

			else if(comfile==file_meos){

				outdword(0xCDFFC883);

				op(0x40);

			}

			else outdword(0x21CD4CB4);

		}

		retproc=TRUE;

	}

	else return ConvRetCode(FindProcLib(type));

	if(type==1)ret();	/* if it is a REG Proc not a MACRO */

	return (am32==FALSE?tk_word:tk_dword);

}



int ConvRetCode(int i)

{

	switch(i){

		case 0: return tk_void;

		case 1: return tk_char;

		case 2: return tk_byte;

		case 3: return tk_int;

		case 4: return tk_word;

		case 5: return tk_long;

		case 6: return tk_dword;

		case 7: return tk_float;

		default: return -1;

	}

}



int RetAtExit()

{

	if(comfile==file_exe||fobj!=FALSE||comfile==file_w32||comfile==file_meos){

		jumploc(startexit);

		return 1;

	}

	else callloc(startexit);

	return 0;

}



int includeproc()

{

	return ConvRetCode(FindProcLib(1));

}



/*-----------------18.01.99 22:42-------------------

 Макропроцедуры

 --------------------------------------------------*/

int CheckMacros()

{

int m,app=0,s32=0;

unsigned int num;

char *ofsstr=NULL;

int razr;

	for(m=0;m<m_end;m++)if(strcmp(macroname[m],(char *)string)==0)break;

	if(m==m_end)return tokens;

	nexttok();

	ofsstr=GetLecsem(tk_closebracket,tk_camma);

	nexttok();

	if(m<m_sqrt){

		switch(m){

			case m_id:

				s32++;

			case m_iw:

				app++;

			case m_ib:

			case m_ibt:

				if(tok!=tk_closebracket){

					if(tok==tk_number){

						num=doconstlongmath();

						if(num<256){

							if(s32!=0)op66(r32);

							else if(app==1)op66(r16);

							op(0xe4+app);

							op(num);

							break;

						}

						if(optimizespeed){

							op(0xB8+DX);	// MOV reg,#

							if(am32)outdword(num);

							else outword(num);

						}

						else{

							op66(r16);

							op(0xB8+DX);	// MOV reg,#

							outword(num);

						}

					}

					else getintoreg(EDX,r16,0,&ofsstr);

					if(ofsstr){

						IDZToReg(ofsstr,DX,r16);

						free(ofsstr);

						ofsstr=NULL;

					}

				}

				if(s32!=0)op66(r32);

				else if(app==1)op66(r16);

				op(0xEC+app);

				ClearReg(AL);

				break;

			case m_od:

				s32++;

			case m_ow:

				app++;

			case m_ob:

			case m_obt:

				if(tok!=tk_closebracket){

					if(s32!=0){

						do_e_axmath(0,r32,&ofsstr);

						razr=r32;

					}

					else if(app!=0){

						do_e_axmath(0,r16,&ofsstr);

						razr=r16;

					}

					else{

						doalmath(0,&ofsstr);

						razr=r8;

					}

					if(ofsstr){

						IDZToReg(ofsstr,AX,razr);

						free(ofsstr);

						ofsstr=NULL;

					}

					if(tok==tk_camma){

						ofsstr=GetLecsem(tk_camma,tk_closebracket);

						nexttok();

						if(tok==tk_number){

							num=doconstlongmath();

							if(num<256){

								if(s32!=0)op66(r32);

								else if(app==1)op66(r16);

								op(0xe6+app);

								op(num);

								goto enout;

							}

							if(optimizespeed){

								op(0xB8+DX);	// MOV reg,#

								if(am32)outdword(num);

								else outword(num);

							}

							else{

								op66(r16);

								op(0xB8+DX);	// MOV reg,#

								outword(num);

							}

						}

						else getintoreg(EDX,r16,0,&ofsstr);

						if(ofsstr){

							IDZToReg(ofsstr,DX,r16);

							free(ofsstr);

							ofsstr=NULL;

						}

					}

				}

				if(s32!=0)op66(r32);

				else if(app==1)op66(r16);

				op(0xEE + app);

enout:

				if(ofsstr)free(ofsstr);

				return tk_void;

		}

		if(ofsstr)free(ofsstr);

	}

	else{

		if(ofsstr)free(ofsstr);

		if(tok==tk_closebracket){

			missingpar(macroname[m]);

			return(tokens);

		}

		doeaxfloatmath(tk_fpust);

		switch(m){

			case m_sqrt:

				outword(0xFAD9);	//FSQRT

				return tk_fpust;	//результат в стеке fpu

			case m_cos:

				outword(0xFFD9);

				return tk_fpust;	//результат в стеке fpu

			case m_sin:

				outword(0xFED9);

				return tk_fpust;	//результат в стеке fpu

			case m_atan2:

				if(expecting(tk_camma)){

					missingpar(macroname[m]);

					return tokens;

				}

				doeaxfloatmath(tk_fpust);

				outword(0xF3D9);	//FPATAN

				return tk_fpust;	//результат в стеке fpu

			case m_tan:

				outdword(0xD8DDF2D9);	//FPTAN fstp st

				return tk_fpust;	//результат в стеке fpu

			case m_log:

				outword(0xEDD9);	//fldln2

				outdword(0xF1D9C9D9);	//fxch st1  fyl2x

				return tk_fpust;	//результат в стеке fpu

			case m_log10:

				outword(0xECD9);	//fldlg2

				outdword(0xF1D9C9D9);	//fxch st1  fyl2x

				return tk_fpust;	//результат в стеке fpu

			case m_exp:

				outprocedure(aaEXP,MMEXP);

				return tk_fpust;	//результат в стеке fpu

			case m_atan:

				outdword(0xF3D9E8D9);	//FLD1 FPATAN

				return tk_fpust;	//результат в стеке fpu

			case m_fabs:

				outword(0xE1D9);	//FABS

				return tk_fpust;	//результат в стеке fpu

		}

	}

	if(s32!=0)return tk_dword;

	if(app!=0)return  tk_word;

	return tk_byte;

}



/*-----------------06.02.99 16:09-------------------

 Работа с внешней библиотекой

	--------------------------------------------------*/

typedef struct _IPROC_

{

	unsigned short size;

	unsigned short ofs;

	unsigned char cpu;

}IPROC;



struct _HLIB_

{

	char name[33];

	unsigned char rettype;

	unsigned short size;

	unsigned char dosr[2];

	IPROC info[4];

}hlib;



int FindProcLib(int type)

{

static int lhandl=-1;

long ofs=0;

char m1[80];

int index;

	if(am32)return -1;

	if(lhandl==-1){

		sprintf(m1,"%s%s",findpath[0],"mainlib.ldp");

		if((lhandl=open(m1,O_RDONLY|O_BINARY))==-1)return -1;

	}

	lseek(lhandl,0,SEEK_SET);

		if(chip<3){

		if(optimizespeed==0)index=0;

		else index=1;

	}

	else{

		if(optimizespeed==0)index=2;

		else index=3;

	}

	for(;;){

		if(read(lhandl,&hlib,sizeof(_HLIB_))!=sizeof(_HLIB_))break;

		if(strcmp((char *)itok.name,hlib.name)==0){

int size;

			if((outptr+hlib.size)>=outptrsize)CheckCodeSize();

			lseek(lhandl,ofs+sizeof(_HLIB_)+hlib.info[index].ofs,SEEK_SET);

			size=hlib.info[index].size;

			if(type!=1)size--;

			else if(dbg&2){

				sprintf(m1,"%s()",itok.name);

				AddCodeNullLine(m1);

			}

			if(read(lhandl,output+outptr,size)!=size)break;

			outptr+=size;

			if(splitdata==0)outptrdata=outptr;

			if(cpu<hlib.info[index].cpu)cpu=hlib.info[index].cpu;

			if(hlib.dosr[1]!=0){

				if(hlib.dosr[1]>dos1){

					dos1=hlib.dosr[1];

					dos2=hlib.dosr[0];

				}

				else if(hlib.dosr[1]==dos1&&hlib.dosr[0]>dos2)dos2=hlib.dosr[0];

			}

			clearregstat();

#ifdef OPTVARCONST

			ClearLVIC();

#endif

			return hlib.rettype;

		}

		ofs+=sizeof(_HLIB_)+hlib.size;

		lseek(lhandl,ofs,SEEK_SET);

	}

	return -1;

}



void addconsttotree(char *keystring,long long constvalue,int type)

//вставить константу в дерево

{

struct idrec *ptr,*newptr;

int cmpresult;

	newptr=(struct idrec *)MALLOC(sizeof(struct idrec));//новый блок константы

	ptr=definestart;

	if(ptr==NULL)definestart=newptr;

	else{

		while(((cmpresult=strcmp(ptr->recid,keystring))<0&&ptr->left!=NULL)||(cmpresult>0&&ptr->right!=NULL)){

			if(cmpresult<0)ptr=ptr->left;

			else if(cmpresult>0)ptr=ptr->right;

		}

		if(cmpresult<0)ptr->left=newptr;

		else if(cmpresult>0)ptr->right=newptr;

		else{

			free(newptr);

			if(ptr->newid){

				free(ptr->newid);

				ptr->newid=NULL;

			}

			if(ptr->sbuf){

				free(ptr->sbuf);

				ptr->sbuf=NULL;

			}

			ptr->recnumber=constvalue;

			ptr->recrm=type;

			ptr->rectok=tk_number;

			ptr->recsegm=DS;

			ptr->recpost=0;

			ptr->flag=0;

			ptr->recsize=0;

			return;

		}

	}

	strcpy(newptr->recid,keystring);

	newptr->newid=NULL;

	newptr->rectok=tk_number;

	newptr->reclnumber=constvalue;

	newptr->recsegm=DS;

	newptr->recrm=type;

	newptr->recpost=0;

	newptr->flag=0;

	newptr->recsize=0;

	newptr->left=NULL;

	newptr->right=NULL;

//	newptr->count=1;

}



void addtodefine(char *keystring)//добавить строку в дерево define

{

struct idrec *ptr,*newptr,*left=NULL,*right=NULL;

int cmpresult;

//выделить память под новую проц

	newptr=(struct idrec *)MALLOC(sizeof(struct idrec));

	ptr=definestart;	//начало дерева

	if(ptr==NULL)definestart=newptr;

	else{	//поиск строки в дереве

		while(((cmpresult=strcmp(ptr->recid,keystring))<0&&ptr->left!=NULL)||

		       (cmpresult>0&&ptr->right!=NULL)){

			if(cmpresult<0)ptr=ptr->left;

			else if(cmpresult>0)ptr=ptr->right;

		}

		if(cmpresult<0)ptr->left=newptr;	//строка меньше

		else if(cmpresult>0)ptr->right=newptr;

		else{

			free(newptr);

			newptr=ptr;

			left=ptr->left;

			right=ptr->right;

			if(newptr->newid)free(newptr->newid);

			if(newptr->sbuf)free(newptr->sbuf);

		}

	}

	strcpy(newptr->recid,keystring);//скопир название

	newptr->newid=NULL;

	if(tok==tk_string){

		newptr->newid=(char *)MALLOC(itok.number);

		memcpy(newptr->newid,string,itok.number);

		if(itok.rm==1)newptr->sbuf=BackString((char *)string3);

	}

	else{

		if(string[0]!=0)newptr->newid=BackString((char *)string);

		newptr->sbuf=NULL;

	}

	newptr->rectok=tok;

	newptr->recnumber=itok.number;

	newptr->recsegm=itok.segm;

	newptr->recrm=itok.rm;

	newptr->recpost=itok.post;

	newptr->flag=itok.flag;

	newptr->recsize=itok.size;

	newptr->left=left;

	newptr->right=right;

	newptr->recsib=itok.sib;

	newptr->line=linenumber;

	newptr->file=currentfileinfo;

	newptr->count=0;

	newptr->type=itok.type;

	newptr->npointr=itok.npointr;

	itok.rec=newptr;

}



unsigned char get_directive_value()  //return the 0 or 1 value for directive

{

	nexttok();

	if(tok==tk_number){

		if(doconstlongmath())return(1);//если значение не нулевое вернуть 1

		return(0);

	}

	numexpected();

	nexttok();

	return(0);

}



int GetStringAsIt()

{

int tstr;

	if(tok2==tk_string){

		nexttok();

		tstr=1;

	}

	else{

		inptr=inptr2;

		cha=cha2;

		whitespace();

		if(cha=='<'){

			tstr=0;

			for(int i=0;i<STRLEN;i++){

				nextchar();

				if(cha=='>'){

					string3[i]=0;

					break;

				}

				if(cha==13||cha==26)goto errstr;

				string3[i]=cha;

			}

			nextchar();

		}

		else{

errstr:

			stringexpected();

			tstr=-1;

		}

		inptr2=inptr;

		linenum2=linenumber;

		cha2=cha;

	}

	return tstr;

}



void InitDefineConst()

{

	addconsttotree("TRUE",TRUE);

	addconsttotree("FALSE",FALSE);

	addconsttotree("__SECOND__",timeptr.tm_sec);

	addconsttotree("__MINUTE__",timeptr.tm_min);

	addconsttotree("__HOUR__",timeptr.tm_hour);

	addconsttotree("__DAY__",timeptr.tm_mday);

	addconsttotree("__MONTH__",timeptr.tm_mon);

	addconsttotree("__YEAR__",timeptr.tm_year+1900);

	addconsttotree("__WEEKDAY__",timeptr.tm_wday);

	addconsttotree("__VER1__",ver1);

	addconsttotree("__VER2__",ver2);

	tok=tk_string;

	itok.flag=zero_term;

	itok.rm=0;

	strcpy((char *)string,(char *)compilerstr);

	itok.number=strlen((char *)string);

	addtodefine("__COMPILER__");

	strcpy((char *)string,asctime(&timeptr));

	itok.number=strlen((char *)string)-1;

	string[itok.number]=0;

	addtodefine("__DATESTR__");

	DateToStr((char *)string);

	itok.number=11;

	addtodefine("__DATE__");

	sprintf((char *)string,"%02d:%02d:%02d",timeptr.tm_hour,timeptr.tm_min,timeptr.tm_sec);

	itok.number=8;

	addtodefine("__TIME__");

	strcpy((char *)string,"struct");

	tok=tk_struct;

	itok.number=6;

	addtodefine("class");

}



char *AddTextToBuf(char *buf,int *size,int start,int add)

{

int addsize;

	addsize=(add==0?itok.number-1:inptr)-start;

	*size+=addsize;

	if(!buf){

		buf=(char *)MALLOC(*size+1+add);

		buf[0]=0;

	}

	else buf=(char *)REALLOC(buf,*size+1+add);

//	printf("size=%d %s\n",*size+1+add,(char *)(input+start));

	strncat(buf,(char *)(input+start),addsize);

	buf[*size]=0;

	return buf;

}



void AddMacro(char *name,int numpar,char *paramstr)

{

int start,size;

char *bstring=NULL;

idrec *ptr;

	start=inptr-1;

	size=0;

	do{

		FastTok(2);

		if(tok==tk_comment1){

			inptr=itok.number;

			cha=input[itok.number-1];

			break;

		}

		if(tok==tk_comment2){

			bstring=AddTextToBuf(bstring,&size,start,0);

			start=inptr-1;

		}

	}while(tok!=tk_endline&&tok!=tk_eof);

	bstring=AddTextToBuf(bstring,&size,start,1);

	strbtrim(bstring);

	size=strlen(bstring);

	bstring[size]=0x20;

	bstring[size+1]=0;

	inptr2=inptr;

	cha2=cha;

	linenum2=linenumber;

	tok=tk_macro;

	itok.size=numpar;

	string[0]=0;

/*	int i;

	for(i=0,size=0;i<numpar;i++){

		puts(paramstr+size);

		size+=strlen(paramstr+size)+1;

	}*/



//	printf("tok=%d %s %s\n",tok,name,bstring);

	addtodefine(name);

	ptr=itok.rec;

	ptr->newid=paramstr;

	ptr->sbuf=bstring;

	ptr->line=linenumber-1;

}



void GetMacro(char *name)

{

int size,nsize,numpar=0;

char *paramstr;

	inptr=inptr2;

	cha=cha2;

	FastTok(0);

	FastTok(1);

	size=0;

	while(tok!=tk_closebracket&&tok!=tk_eof){

		if(tok!=tk_id)varexpected(numpar+1);

		nsize=strlen(itok.name)+1;

		if(size==0)paramstr=(char *)MALLOC(nsize);

		else paramstr=(char *)REALLOC(paramstr,size+nsize);

		strcpy(paramstr+size,itok.name);

		size+=nsize;

		numpar++;

		FastTok(0);

		if(tok!=tk_closebracket){

			if(tok!=tk_camma)expected(',');

			FastTok(1);

		}

	}

	if(tok==tk_eof)unexpectedeof();

	AddMacro(name,numpar,paramstr);

}



int pushpop(int i)

{

STACKALIGN *newpar;

	switch(i){

		case 1:

			newpar=(STACKALIGN *)MALLOC(sizeof(STACKALIGN));

			newpar->prev=stackalign;

			newpar->size=strpackcur;

			newpar->id[0]=0;

			stackalign=newpar;

			return TRUE;

		case 2:

			newpar=stackalign->prev;

			free(stackalign);

			stackalign=newpar;



			strpackcur=(stackalign!=NULL?stackalign->size:strpackdef);

			return TRUE;

	}

	return FALSE;

}



void doifdef(int intok)

{

int defcond;

int curcond;

int bracket;

int notflag;

int next;

int locrez,endrez,lastoper;

int stopscan;

unsigned int i;

int oscanlexmode;

	dirmode=dm_if;

/*	string3[0]=cha2;

	int j=1;

	for(i=inptr2;input[i]>13;i++,j++)string3[j]=input[i];

	string3[j]=0;

	printf("%s (%u) %s %s",(startfileinfo+currentfileinfo)->filename,linenumber,itok.name,string3);

	*/

	defcond=intok;

	if(intok==d_if||intok==d_elif)defcond=d_ifdef;

	curcond=defcond;

	oscanlexmode=scanlexmode;

	bracket=0;

	lastoper=tk_oror;

	endrez=0;

	stopscan=FALSE;

	do{

		scanlexmode=DEFLEX;

		nexttok();

		notflag=FALSE;

		next=TRUE;

		while(tok==tk_openbracket){

			nexttok();

			bracket++;

		}

		if(tok==tk_not){

			notflag=TRUE;

			nexttok();

		}

		if(tok==tk_id&&strcmp("defined",itok.name)==0){

			nexttok();

			curcond=(notflag==FALSE?d_ifdef:d_ifndef);

		}

		while(tok==tk_openbracket){

			nexttok();

			bracket++;

		}

//			printf("tok=%d type2=%d\n",tok,itok2.type);

		if(tok==tk_number){

			locrez=1;

			if(itok2.type==tp_opperand){

				i=doconstdwordmath();

				next=0;

				if(itok.type==tp_compare)locrez=CompConst(i);

			}

			else if(itok2.type==tp_compare){

				i=itok.number;

				nexttok();

				locrez=CompConst(i);

			}

		}

		else if((locrez=ifdefconst())==2){

			if(tok==tk_id||tok==tk_ID){

				if(FindTeg(TRUE))locrez=1;

				else locrez=0;

			}

			else locrez=1;

		}

		if(curcond==d_ifndef)locrez^=1;

		if(stopscan==FALSE){

			if(lastoper==tk_oror)endrez|=locrez;

			else{

				endrez&=locrez;

				if(endrez==FALSE)stopscan=TRUE;

			}

		}

//			printf("lastoper=%d endrez=%d locrez=%d\n",lastoper,endrez,locrez);

		if(next)nexttok();

		while(tok==tk_closebracket){

			nexttok();

			bracket--;

		}

		if(tok==tk_endline||tok2==tk_endline){

			scanlexmode=oscanlexmode;

//			retoldscanmode(oscanlexmode);

			dirmode=dm_other;

			nexttok();

			break;

		}

		if(tok==tk_oror||tok==tk_andand)lastoper=tok;

		else{

			preerror("bad token in 'ifdef/ifndef/if'");

//			printf("tok=%d\n",tok);

		}

	}while(tok!=tk_eof&&tok!=tk_endline);

	dirmode=dm_other;

	scanlexmode=oscanlexmode;

//	retoldscanmode(oscanlexmode);

	if(bracket>0)preerror("missing ')'");

	else if(bracket<0)preerror("extra ')'");

	gotoendif=1^endrez;

	if(intok!=d_elif){

		endifcount++;

		CheckNumIF();

		useelse[endifcount]=1;

	}

//	printf(" %s endifcount=%d\n",endrez==0?"FALSE":"TRUE",endifcount);

}



void InitIdxRegs()

{

unsigned char lreg[8];

int i,j;

	for(i=0;i<8;i++)lreg[i]=0;

	nexttok();

//	printf("tok=%d %s\n",tok,itok.name);

	for(i=0;tok==tk_reg32||tok==tk_reg||tok==tk_beg;i++){

		if(i<4){

			if(tok==tk_beg&&itok.number>3)itok.number-=4;

			idxregs[i]=itok.number;

			if(lreg[itok.number])preerror("expected unique register for \"pragma indexregs\"");

			lreg[itok.number]=1;

		}

		nexttok();

		if(tok==tk_camma)nexttok();

	}

	lreg[ESP]=lreg[EBP]=1;

	for(;i<4;i++){

		if(lreg[idxregs[i]]==0)lreg[idxregs[i]]=1;

		else{

			for(j=7;j>=0;j--){

				if(lreg[j]==0){

					lreg[j]=1;

					idxregs[i]=j;

					break;

				}

			}

		}

	}

}



void directive()

{

unsigned char next=1;

char holdid[IDLENGTH];

long longhold,longhold2;

long long llhold;

unsigned int i;

int oscanlexmode;

	holdid[0]=DS;

	usedirectiv=TRUE;

	dirmode=dm_other;

	switch(itok.number){

		case d_alignc:

			holdid[0]=CS;

		case d_align: //использовать байт вставки если нечетный адрес

			if(notdoneprestuff==TRUE)doprestuff();	//начальный код

			i=2;

			nexttok();

			next=0;

			if(tok==tk_number){

				i=doconstlongmath();

				if(i<2)i=2;

			}

			i=AlignCD(holdid[0],i);

			if(holdid[0]==DS)alignersize+=i;

			break;

		case d_aligner://значение байта вставки

			nexttok();

			if(tok==tk_number){

				aligner=(unsigned char)doconstlongmath();//вычислить значение

				next=0;

			}

			else numexpected();

			break;

		case d_alignw://выравнивание адресов

			alignword=get_directive_value();

			next=0;

			break;

//		case d_beep: beep(); break;

		case d_code: optimizespeed=0; break;

		case d_ctrl:

			killctrlc=get_directive_value();

			next=0;

			break;

		case d_define:

			dirmode=dm_def0;

			oscanlexmode=scanlexmode;

			scanlexmode=DEFLEX2;

			nexttok();

			scanlexmode=DEFLEX;

			dirmode=dm_def;

			strcpy(holdid,itok.name);

//			printf("1 line=%d\n",linenumber);

//			printf("tok=%d %s\n",tok,itok.name);

			if(tok==tk_id||tok==tk_ID){

				if(cha2=='('){

					GetMacro(holdid);

				}

				else{

					longhold=inptr2;

					longhold2=cha2;

					nexttok();

//		printf("2 line=%d\n",linenumber);

					i=tk_dword;

					if((tok==tk_float||tok==tk_double||tok==tk_qword)&&(tok2==tk_minus||tok2==tk_number)){

						i=tok;

						nexttok();

					}

//			printf("tok=%d tok2=%d %s %s\n",tok,tok2,itok.name,string);

					if(tok!=tk_endline&&tok!=tk_minus&&tok!=tk_number&&tok2!=tk_endline&&tok2!=tk_semicolon){

						inptr=longhold;

						cha=longhold2;

						AddMacro(holdid,0,NULL);

					}

					else{

						switch(tok){

							case tk_structvar:

								struct idrec *ptrs,*ptrsnew;

								ptrs=itok.rec;

								addtodefine(holdid);

								ptrsnew=itok.rec;

								if(ptrsnew->newid)free(ptrsnew->newid);

								ptrsnew->newid=ptrs->newid;

								break;

							case tk_eof: unexpectedeof(); break;

							case tk_string:

//			printf("tok=%d tok2=%d %s %s\n",tok,tok2,itok.name,string);

								if(cha2==13&&itok.name[0]==0){

									cha2=input[inptr2];

									inptr2++;

								}

								goto endef;

							case tk_minus:

								if(tok2==tk_number){

									if(i==tk_dword&&(itok2.rm==tk_float||itok2.rm==tk_double||

											itok2.rm==tk_qword))i=itok2.rm;

							case tk_number:

									if(i==tk_dword&&(itok.rm==tk_float||itok.rm==tk_double||itok.rm==tk_qword))i=itok.rm;

									switch(i){

										case tk_float:

											llhold=doconstfloatmath();

											break;

										case tk_double:

											llhold=doconstdoublemath();

											break;

										case tk_qword:

											llhold=doconstqwordmath();

											break;

										default:

											llhold=doconstdwordmath();

											break;

									}

									itok.flag=(unsigned char)postnumflag;

									addconsttotree(holdid,llhold,i);

									next=0;

									break;

								}

							case tk_proc:

							case tk_apiproc:

							case tk_declare:

							case tk_undefproc:

								tok=tk_id;

								strcpy((char *)string,itok.name);

							default:

endef:

								if(itok.type!=tp_modif)itok.type=tp_ucnovn;

								addtodefine(holdid); break;

						}

					}

				}

			}

			else idalreadydefined();

//			retoldscanmode(oscanlexmode);

			scanlexmode=oscanlexmode;

			if(tok==tk_endline){

//				nextchar();

				nexttok();

				next=0;

			}

//			printf("3 line=%d cha=%02X\n",linenumber,cha2);

			break;

		case d_DOS:

			nexttok();

			if(tok==tk_number){

				longhold=doconstlongmath();

				longhold2=dos1*256+dos2;

				if(longhold>longhold2){

					dos1=(unsigned char)(longhold/256);

					dos2=(unsigned char)(longhold%256);

				}

				next=0;

			}

			else numexpected();

			break;

		case d_endif:

			if(endifcount==-1) preerror("?endif without preceeding ?if");

			else endifcount--;

//			printf("%s (%u) endif: endifcount=%d\n",(startfileinfo+currentfileinfo)->filename,linenumber,endifcount);

			break;

		case d_ifdef:

		case d_if:

		case d_ifndef:

			doifdef(itok.number);

			if(gotoendif==0)useelse[endifcount]|=2;

			next=0;

			break;

		case d_incl:

			i=GetStringAsIt();

			usedirectiv=FALSE;

			if((int)i!=-1){

				char *a;

				if((a=strrchr((char *)string3,'.'))!=NULL){

					if(stricmp(a,".obj")==0){

						AddNameObj((char *)string3,i,0);

						break;

					}

					if(stricmp(a,".lib")==0){

						AddNameObj((char *)string3,i,1);

						break;

					}

				}

//				puts((char *)string3);

				IncludeFile((char *)string3,i);

			}

			break;

		case d_jump:

			nexttok();

			if(tok==tk_number){

				jumptomain=CALL_NEAR;

				if((unsigned int)itok.number==0){

					jumptomain=CALL_NONE;

					header=0;

				}

			}

			else if(stricmp(itok.name,"NONE")==0){

				jumptomain=CALL_NONE;

				header=0;

			}

			else if(stricmp(itok.name,"SHORT")==0)jumptomain=CALL_SHORT;

			else if(stricmp(itok.name,"NEAR")==0)jumptomain=CALL_NEAR;

			else preerror(ujo);

			break;

		case d_error:

			nexttok();

			if(tok==tk_number){

				maxerrors=doconstlongmath();

				next=0;

			}

			else numexpected();

			break;

		case d_command:

			parsecommandline=get_directive_value();

			next=0;

			break;

		case d_argc:

			parsecommandline=fargc=get_directive_value();

			next=0;

			break;

		case d_print:

			nexttok();

			i=0;

			if(strcmp(itok.name,"error")==0){

				i=1;

				nexttok();

			}

			switch(tok){

				case tk_string:

					printf("%s",string);

					break;

				case tk_number:

					printf("\n%u",itok.number);

					break;

				default: preerror("unsupported token for #print"); break;

			}

			if(i)exit(e_preprocess);

			break;

		case d_prnex:

			nexttok();

			if(tok==tk_number)printf("%X",itok.number);

			else numexpected();

			break;

		case d_random:

			if(notdoneprestuff==TRUE)doprestuff();

			op(rand());

			break;

		case d_resize:

			resizemem=get_directive_value();

			next=0;

			break;

		case d_resmes:

			nexttok();

			if(tok==tk_string){

				itok.flag=dos_term;

				addtotree("__RESIZEMESSAGE");

				addconsttotree("__resizemessage",TRUE);

			}

			else stringexpected();

			break;

		case d_speed: optimizespeed=1; break;

		case d_stack:

			nexttok();

			if(tok==tk_number){

				longhold=doconstlongmath();

				if(am32==FALSE&&(longhold>0xFEFF||longhold<0))preerror("invalid size for stack");

				else{

					if(comfile!=file_sys)stacksize=longhold;

					else sysstack=longhold;

				}

				next=0;

			}

			else numexpected();

			break;

		case d_start:

			nexttok();

			if(comfile==file_com){

				if(tok==tk_number){

					startptrdata=startptr=doconstlongmath();

					if(startStartup!=0x100)startStartup=startptr;

				}

				else{

					numexpected();

					nexttok();

				}

				next=0;

			}

			else OnlyComFile();

			break;

		case d_8086: case d_8088: chip=0; break;

		case d_80186: chip=1; break;

		case d_80286: chip=2; break;

		case d_80386: chip=3; break;

		case d_80486: chip=4; break;

		case d_80586: chip=5; break;

		case d_80686: chip=6; break;

		case d_80786: chip=7; break;

		case d_atr:

			nexttok();

			if(tok==tk_number){

				sysatr=doconstlongmath();

				next=0;

			}

			else numexpected();

			break;

		case d_name:

			nexttok();

			if(tok==tk_string){

				int i;

				for(i=0;i<8&&string[i]!=0;i++)sysname[i]=string[i];

				for(;i<8;i++)sysname[i]=' ';

			}

			else stringexpected();

			break;

		case d_com:	//список команд для SYS

			listcom=(LISTCOM *)MALLOC(sizeof(LISTCOM)*MAXSYSCOM);

			do{

				nexttok();

				if(tok==tk_id||tok==tk_ID){

					strncpy((listcom+sysnumcom)->name,(char *)string,32);

					(listcom+sysnumcom)->name[32]=0;

					sysnumcom++;

				}

				else{

					idalreadydefined();

					break;

				}

				nexttok();

				if(tok==tk_semicolon)break;

				if(tok!=tk_camma){

					expected(',');

					break;

				}

			}while(sysnumcom<=MAXSYSCOM);

			if(sysnumcom>MAXSYSCOM)preerror("to many commands");

			else if(sysnumcom!=0&&sysnumcom!=MAXSYSCOM)listcom=(LISTCOM *)REALLOC(listcom,sysnumcom*sizeof(LISTCOM));

			break;

		case d_sdp:	//выгрузить динамические процедуры

			next=notdoneprestuff;

			sdp_mode=TRUE;

			usedirectiv=FALSE;

			docalls();

			sdp_mode=FALSE;

			notdoneprestuff=next;

			next=1;

			break;

		case d_warning:

			warning=get_directive_value();

			next=0;

			break;

		case d_ip:

			if(	GetStringAsIt()!=-1)IncludePath((char *)string3);

			break;

		case d_us:	//использовать код STARTUP

			if(comfile==file_com)useStartup=TRUE;

			break;

		case d_suv:	//адрес начала использования под неинициализированные переменные

			nexttok();

			if(comfile==file_com){

				if(tok==tk_number)startStartup=doconstlongmath();

				else{

					numexpected();

					nexttok();

				}

				next=0;

			}

			else OnlyComFile();

			break;

		case d_iav:	//инициализировать все переменные

			notpost=get_directive_value();

			next=0;

			break;

		case d_atex:	//механизм ATEXIT

			atex=TRUE;

			break;

		case d_dseg:	//сегмент данных для rom-bios

			nexttok();

			if(tok==tk_number){

				dataseg=doconstlongmath();

				next=0;

			}

			else numexpected();

			break;

		case d_rsize:	//размер rom-bios

			nexttok();

			if(tok==tk_number){

				romsize=doconstlongmath();

				next=0;

			}

			else numexpected();

			break;

		case d_mdr:	//переносить данные а  опер память

			splitdata=modelmem=get_directive_value();

			next=0;

			break;

		case d_am32:	//32 битная адресация

			nexttok();

			if(tok!=tk_number)numexpected();

			else am32=(unsigned char)(itok.number==0?FALSE:TRUE);

			break;

		case d_undef:	//undef

			oscanlexmode=scanlexmode;

			scanlexmode=DEFLEX2;

			nexttok();

			scanlexmode=oscanlexmode;

//			retoldscanmode(oscanlexmode);

			if(tok==tk_string){

				strcpy(itok.name,(char *)string);

				KillVarOfTree(&treestart);

			}

			else KillVarOfTree(&definestart);

			break;

		case d_stm:	//startuptomain

			startuptomain=TRUE;

			break;

		case d_ib:	//imagebase

			nexttok();

			ImageBase=doconstlongmath();

			next=0;

			break;

		case d_fut:	//fixuptable

			FixUpTable=get_directive_value();

			next=0;

			break;

		case d_fca:	//fastcallapi

			FastCallApi=get_directive_value();

			next=0;

			break;

		case d_dstr:

			dosstring=get_directive_value();

			next=0;

			break;

		case d_cv:

			int v1,v2;

			v1=v2=0;

			cha=cha2;

			inptr=inptr2;

			linenumber=linenum2;

			whitespace(); //пропуск нзначащих символов

			while(isdigit(cha)){

				v1=v1*10+(cha-'0');

				nextchar();

			}

			if(cha!='.')expected('.');

			nextchar();

			i=0;

			while(isdigit(cha)){

				v2=v2*10+(cha-'0');

				nextchar();

				i++;

			}

			while(i<3){

				v2=v2*10;

				i++;

			}

			cha2=cha;

			inptr2=inptr;

			linenum2=linenumber;

			if(v1>=ver1){

				if(v1==ver1&&v2<=ver2)break;

			}

			sprintf((char *)string,"needed compiler version %0u.%03u or best",v1,v2);

			preerror((char *)string);

			break;

		case d_else:

			if(endifcount==-1) preerror("#else without preceeding #if");

			else if((useelse[endifcount]&1)==0)preerror(toelse);

			else useelse[endifcount]&=0xFE;

			if(gotoendif==1) gotoendif=0;

			else gotoendif=1;

			break;

		case d_elif:

			if(endifcount==-1) preerror("#elif without preceeding #if");

			doifdef(d_elif);

			next=0;

			gotoendif=1;

			break;

		case d_wmb: //формирование одного блока под win

			WinMonoBlock=get_directive_value();

			next=0;

			break;

		case d_pragma:

			char *ptr;

			inptr=inptr2;

			cha=cha2;

			FastTok(1);

//			printf("%c %s\n",cha,itok.name);

			i=0;

			while(cha!=13&&cha!=26){

				if(cha=='/'){

					if(input[inptr]=='/'){

						while(cha!=13&&cha!=26)nextchar();

						break;

					}

					if(input[inptr]=='*'){

						while(cha!=26){

							nextchar();

							if(cha=='*'&&input[inptr]=='/'){

								nextchar();

								nextchar();

								break;

							}

							if(cha==13)linenumber++;

						}

						break;

					}

				}

				string[i++]=cha;

				nextchar();

			}

			cha2=cha;

			inptr2=inptr;

			linenum2=linenumber;

			string[i]=0;

			strbtrim((char *)string);

//			printf("%s %s\n",itok.name,string);

			if(strlen((char *)string))ptr=BackString((char *)string);

			else ptr=NULL;

			for(i=0;i<p_end;i++)if(strcmp(itok.name,pragmalist[i])==0)break;

			if(i!=p_li&&ptr==NULL){

				unknownpragma(pragmalist[i]);

				break;

			}

			switch(i){

				case p_op:

					SelectComand(ptr,0);

					break;

				case p_li:

					printf("%s (%u) %s\n",(startfileinfo+currentfileinfo)->filename,linenumber,ptr==NULL?"":ptr);

					break;

				case p_st:

					i=0;

					char c;

					do{

						c=ptr[i++];

					}while(isalnum(c)||c=='_');

					ptr[i]=0;

					if(numdomain==0)domain=(char *)MALLOC(IDLENGTH);

					else domain=(char *)REALLOC(domain,IDLENGTH*(numdomain+1));

					strcpy(domain+numdomain*IDLENGTH,ptr);

					numdomain++;

					break;

				case p_re:

					if(strcmp(ptr,"start")==0){

						scanlexmode=RESLEX;

						input_res();

					}

					else if(strcmp(ptr,"end")==0)

						scanlexmode=STDLEX;

//					retoldscanmode(STDLEX);

					else preerror("for '#pragma resource' used only 'start' or 'end'");

					break;

				case p_db:

#ifdef DEBUGMODE

					if(strcmp(ptr,"start")==0){

						debug=TRUE;

					}

					else if(strcmp(ptr,"end")==0)debug=FALSE;

					else unknownpragma("debug");

#endif

					break;

				case p_pa:

//					printf("ptr=%08X input=%08X tok=%d\n",ptr,input,tok);

					SetNewStr(ptr);

					displaytokerrors=1;

					FastTok(1);

					if(tok!=tk_openbracket){

						expected('(');

						break;

					}

					FastTok(1);

					if(tok==tk_closebracket)strpackcur=strpackdef;

					i=0;

					STACKALIGN *newpar;

					while(tok!=tk_closebracket&&tok!=tk_eof){

						switch(tok){

							case tk_number:

								if(caselong(itok.number)==NUMNUM){

									preerror("bad align size");

									break;

								}

								if(pushpop(i))i=3;

								strpackcur=itok.number;

								break;

							case tk_id:

								if(strcmp(itok.name,"push")==0){

									if(i)unknownpragma("pack(push)");

									i=1;

								}

								else if(strcmp(itok.name,"pop")==0){

									if(i)unknownpragma("pack(pop)");

									i=2;

								}

								else if(strcmp(itok.name,"show")==0){

									printf("%s (%u) Current packing alignment for structure = %d\n",(startfileinfo+currentfileinfo)->filename,linenumber,strpackcur);

									i=3;

								}

								else{

									switch(i){

										case 0:

											unknownpragma("pack");

											break;

										case 1:

											newpar=(STACKALIGN *)MALLOC(sizeof(STACKALIGN));

											newpar->prev=stackalign;

											newpar->size=strpackcur;

											strcpy(newpar->id,itok.name);

											stackalign=newpar;

											i=3;

											break;

										case 2:

											while(stackalign!=NULL&&strcmp(itok.name,stackalign->id)!=0){

												newpar=stackalign->prev;

												free(stackalign);

												stackalign=newpar;

											}

											if(stackalign==NULL){

												strpackcur=strpackdef;

												warpragmapackpop();

												break;

											}

											newpar=stackalign->prev;

											free(stackalign);

											stackalign=newpar;

											strpackcur=(stackalign==NULL?strpackdef:stackalign->size);

											i=3;

											break;

										default:

											unknownpragma("pack");

											break;

									}

								}

						}

						FastTok(1);

						if(tok==tk_camma)FastTok(1);

					}

					pushpop(i);

//					printf("ptr=%08X input=%08X tok=%d\n",ptr,input,tok);

					inptr2=inptr;

					cha2=cha;

					while(ptr==(char*)input)nexttok();

					next=0;

					break;

				case p_idx:

					SetNewStr(ptr);

					inptr2=1;

					InitIdxRegs();

					next=0;

					break;

				default:

					unknownpragma("");

					break;

			}

			if(ptr)free(ptr);

			break;

		case d_inline:

			nexttok();

			if(tok==tk_number){

				if(itok.number>=0&&itok.number<=2)useinline=itok.number;

				else preerror(ido);

			}

			else if(strcmp(itok.name,"AUTO")==0)useinline=2;

			else preerror(ido);

			break;

		default:

			preerror("compiler directive expected");

	}

	if(next)nexttok();

	while(gotoendif){

		if(tok==tk_question){

			if(itok.number==d_endif){

				gotoendif--;

				endifcount--;

//			printf("%s (%u) endif shadow: endifcount=%d\n",(startfileinfo+currentfileinfo)->filename,linenumber,endifcount);

			}

			else if(itok.number==d_ifdef||itok.number==d_ifndef||itok.number==d_if){

				gotoendif++;

				endifcount++;

//			printf("%s (%u) if shadow: endifcount=%d\n",(startfileinfo+currentfileinfo)->filename,linenumber,endifcount);

				CheckNumIF();

				useelse[endifcount]=1;

			}

			else if(itok.number==d_else){

				if((useelse[endifcount]&1)==0)preerror(toelse);

				else useelse[endifcount]&=0xFE;

				if(gotoendif==1&&useelse[endifcount]==0){

					gotoendif=0;

					useelse[endifcount]|=2;

				}

			}

			else if(itok.number==d_elif){

				if(gotoendif==1){

					doifdef(d_elif);

					if(gotoendif==0){

						if((useelse[endifcount]&2)==0){

							useelse[endifcount]|=2;

							break;

						}

						gotoendif=1;

					}

				}

			}

		}

		else if(tok==tk_eof){

			unexpectedeof();

			break;

		}

		if(gotoendif)FindDirectiv();

		else{

			nexttok();

			break;

		}

	}

	usedirectiv=FALSE;

	dirmode=dm_none;

}



void doenum()

{

long counter=0;

unsigned char next;

char holdid[IDLENGTH];

	nexttok();

	if(tok!=tk_openbrace){

		if(tok2!=tk_openbrace)expected('{');

		nexttok();

	}

	do{

		next=1;

		nexttok();

		switch(tok){

			case tk_ID:

			case tk_id:

				strcpy(holdid,(char *)string);

				if(tok2==tk_assign){

					nexttok();

					nexttok();

					CheckMinusNum();

					if(tok==tk_number){

						counter=doconstlongmath();

						next=0;

						postnumflag=0;

					}

					else numexpected();

				}

				addconsttotree(holdid,counter);

				counter++;

				break;

			case tk_closebrace:

				next=0;

				break;

			default:

				idalreadydefined();

				break;

		}

		if(next)nexttok();

	}while(tok==tk_camma);

	if(tok!=tk_closebrace)expected('}');

	nextseminext();

}



int CompConst(int firstval)

{

	int otok=tok;

	nexttok();

	if(tok!=tk_number)unknowncompop();

	switch(otok){

		case tk_equalto:

			if(firstval==itok.number)return TRUE;

			break;

		case tk_notequal:

			if(firstval!=itok.number)return TRUE;

			break;

		case tk_greater:

			if(firstval>itok.number)return TRUE;

			break;

		case tk_less:

			if(firstval<itok.number)return TRUE;

			break;

		case tk_greaterequal:

			if(firstval>=itok.number)return TRUE;

			break;

		case tk_lessequal:

			if(firstval<=itok.number)return TRUE;

			break;

		default: unknowncompop();

	}

	return FALSE;

}



int ifdefconst()

{

	if(optimizespeed==TRUE){

		if(strcmp((char *)string,"speed")==0)return TRUE;

	}

	else if(strcmp((char *)string,"codesize")==0)return TRUE;

	if(strcmp((char *)string,"cpu")==0){

		nexttok();

		return CompConst(chip);

	}

	if(itok2.type==tp_compare){

//		unknowncompop();

		nexttok();

		nexttok();

		return FALSE;

	}

	if(comfile==file_w32&&strcmp((char *)string,"_WIN32")==0)return TRUE;

	if(*(short *)&string[0]==0x5F5F){

		if(comfile==file_w32){

			if(strcmp((char *)string+2,"TLS__")==0)return TRUE;

			if(dllflag&&strcmp((char *)string+2,"DLL__")==0)return TRUE;

			else{

				if(wconsole&&strcmp((char *)string+2,"CONSOLE__")==0)return TRUE;

				else if(strcmp((char *)string+2,"WIN32__")==0)return TRUE;

			}

		}

		if(am32){

			if(strcmp((char *)string+2,"FLAT__")==0)return TRUE;

		}

		else{

			if(strcmp((char *)string+2,"MSDOS__")==0)return TRUE;

			if(modelmem==TINY&&strcmp((char *)string+2,"TINY__")==0)return TRUE;

			if(modelmem==SMALL&&strcmp((char *)string+2,"SMALL__")==0)return TRUE;

		}

		if(comfile==file_d32&&strcmp((char *)string+2,"DOS32__")==0)return TRUE;

		if(comfile==file_com&&strcmp((char *)string+2,"COM__")==0)return TRUE;

		if(comfile==file_sys&&strcmp((char *)string+2,"SYS__")==0)return TRUE;

		if(comfile==file_rom&&strcmp((char *)string+2,"ROM__")==0)return TRUE;

		if(comfile==file_bin&&strcmp((char *)string+2,"BIN32__")==0)return TRUE;

		if(comfile==file_meos&&strcmp((char *)string+2,"MEOS__")==0)return TRUE;

		if((sobj||fobj)&&strcmp((char *)string+2,"OBJ__")==0)return TRUE;

		if(comfile==file_exe){

			if(modelmem==TINY){

				if(strcmp((char *)string+2,"TEXE__")==0)return TRUE;

			}

			else if(strcmp((char *)string+2,"EXE__")==0)return TRUE;

		}

	}

	return 2;

}



void CheckNumIF()

{

	if(endifcount>=NUMIFDEF)preerror("#ifdef/#ifndef - too large enclosure");

}



static int firstincl=FALSE;



void IncludeFile(char *fileincl,int tfind)

{

unsigned char *holdinput;

unsigned int holdinptr,holdendinptr,holdlinenum;

unsigned char holdwarning;

unsigned char holdcha,holdendoffile;

int ofileinfo;

char *ostartline;

int oscanlexmode=scanlexmode;

int opostnumflag;

COM_MOD *ocurmod=cur_mod;



int oendifcount;



	oendifcount=endifcount;

	cur_mod=NULL;

	if(!firstincl){

		firstincl=TRUE;

		if((dbg&1)&&am32==FALSE){

			holdcha=dbgact;

			dbgact=0;

			AddLine();

			dbgact=holdcha;

		}

	}

	scanlexmode=STDLEX;

	opostnumflag=postnumflag;

	holdinput=input;	//сохр некотор переменые

	holdinptr=inptr2;

	holdcha=cha2;

	holdlinenum=linenum2;

	holdendinptr=endinptr;

	holdendoffile=endoffile;

	holdwarning=warning;

	ostartline=startline;

	ofileinfo=currentfileinfo;

	(startfileinfo+currentfileinfo)->stlist=staticlist;

	compilefile(fileincl,tfind);//откомпилировать

	if(endifcount!=oendifcount){

		sprintf((char *)string2,"num if prior %d after %d",oendifcount,endifcount);

		preerror((char *)string2);

	}

//	(startfileinfo+currentfileinfo)->staticlist=staticlist;

	currentfileinfo=ofileinfo;

	staticlist=(startfileinfo+currentfileinfo)->stlist;

	warning=holdwarning;

	endoffile=holdendoffile;//востановить переменые

	endinptr=holdendinptr;

	input=holdinput;

	inptr2=holdinptr;

	cha2=holdcha;

	linenumber=linenum2=holdlinenum;

	startline=ostartline;

//	retoldscanmode(oscanlexmode);

	scanlexmode=oscanlexmode;

	postnumflag=opostnumflag;

	cur_mod=ocurmod;

}



/*-----------------31.05.99 21:39-------------------

 поддержка startup

 --------------------------------------------------*/



int startlabl(char *namelab)

{

//ITOK btok;

//int bb;

//	if(searchtree(&btok,&bb,(unsigned char *)namelab)==FALSE)thisundefined(namelab);

	searchvar(namelab);

	return itok.number;

}



void searchvar(char *name,int err)

{

	strcpy((char *)string,name);

	if(searchtree(&itok,&tok,string)==FALSE&&err)thisundefined(name);

}



void doprestuff()  //инициализация начального кода, like resize mem, jump to main...

{

ITOK oitok;

int otok,otok2;

unsigned int addresshold;

unsigned char ojmp;

char *bstring;

int odbg=dbg;

//сохранить параметры

//	if(FixUp==TRUE||comfile==file_w32)optnumber=FALSE;

	oitok=itok;

	bstring=BackString((char *)string);

	otok=tok;

	ojmp=0xff;

	otok2=tok2;

//	printf("tok=%d inptr=%d\n",tok,inptr);

	if(notdoneprestuff==TRUE){

		if(splitdata)startptrdata=0;

		if(comfile!=file_w32||(dbg&2))dbgact=0;

	}

	if(startuptomain==TRUE&&comfile==file_com){

		if(notdoneprestuff==TRUE){

			outptr=startptr;

			outptrdata=startptrdata;

			if(jumptomain!=CALL_NONE){

				tok=tk_ID;

				strcpy(itok.name,mesmain);

				tobedefined(jumptomain==CALL_NEAR?JMP_NEAR:CALL_SHORT,tk_void);	/* put main on the to be defined stack */

				if(jumptomain==CALL_NEAR )jumploc0();

				else outword(0x00EB);	// JMP short

			}

			endStartup=outptr;

			notdoneprestuff=FALSE;

			goto endp;

		}

		else{

			header=0;	//чтоб не было повторной надписи sphinx

			ojmp=jumptomain;

			jumptomain=CALL_NONE;

			startuptomain=FALSE;

		}

	}

	else if(comsymbios==FALSE){

		outptr=startptr;

		outptrdata=startptrdata;

	}

	notdoneprestuff=FALSE;

	if(comfile==file_meos)outptrdata=outptr=startptr+sizeof(MEOSheader);

	if(sobj){

		outptrdata=outptr=startptrdata=startptr=0;

		goto endp;

	}

	itok.post=1;

	itok.segm=DS;

	itok.number=0;

	itok.flag=0;

	itok.size=0;

	itok.rm=(am32==FALSE?rm_d16:rm_d32);

	tok=(am32==0?tk_wordvar:tk_dwordvar);

	string[0]=0;

	addtotree("__startpostvar");

	itok.rec->count=1;

	if(comfile==file_bin)goto endp;

	else ImageBase=Align(ImageBase,0x10000);

	if(jumptomain==CALL_NONE){

		if(comfile==file_w32||comfile==file_exe||comfile==file_meos)goto endp;

		if(comfile==file_com&&fargc==0&&parsecommandline==0&&atex==0&&use_env==0

				&&clearpost==0&&resizemem==0&&killctrlc==0)goto endp;

	}

	if(comfile==file_sys){

		addconsttotree("__SYSATRIB",sysatr);

		tok=tk_string;

		itok.number=8;

		itok.flag=zero_term;

		strncpy((char *)string,sysname,8);

		addtotree("__SYSNAME");

		if(sysstack!=0){

			addconsttotree("__SYSSTACK",sysstack);

		}

		if(sysnumcom==0)preerror("list command expected");

		else addconsttotree("__SYSNUMCOM",sysnumcom);

	}

	else if(comfile==file_rom){

		resizemem=0;

		addconsttotree("__ROMSIZE",romsize);

		addconsttotree("__DATASEG",dataseg);

		if(modelmem!=SMALL){

			free(outputdata);

			outputdata=output;

		}

	}

	else if(comsymbios==TRUE){

		addresshold=outptr;

		addconsttotree("__STARTPTR",startptr);

		addconsttotree("__STARTVALW",*(unsigned short *)&output[startptr]);

		addconsttotree("__STARTVALB",output[startptr+2]);

	}

	if(comfile==file_w32&&wbss)clearpost=FALSE;

	if(resizemem)addconsttotree("__resizemem",TRUE);

	if(killctrlc)addconsttotree("__ctrl_c",TRUE);

	if(parsecommandline)addconsttotree("__parsecommandline",TRUE);

	if(atex)addconsttotree("__atexit",TRUE);

	if(fargc)addconsttotree("__argc",TRUE);

	if(use_env)addconsttotree("__environ",TRUE);

	if(clearpost)addconsttotree("__clearpost",TRUE);

	if(comfile==file_d32){

		if(useDOS4GW)addconsttotree("__useDOS4GW",TRUE);

		if(dpmistub&&(!usestub)){

			addconsttotree("__DPMIonly",TRUE);

			resizemem=TRUE;

			jumptomain=CALL_NEAR;

		}

	}

	if(jumptomain==CALL_SHORT)addconsttotree("__shortjmp",TRUE);

	else if(jumptomain==CALL_NONE)addconsttotree("__nonejmptomain",TRUE);



	if(dbg){

		if(am32==FALSE){

			if(firstincl==FALSE){

				firstincl=TRUE;

				AddLine();

			}

			dbg=0;

		}

	}

	IncludeFile(namestartupfile,0);

	dbg=odbg;



	if((dbg&1)&&outptr==startptr)KillLastLine();



	if(atex||(comfile==file_w32&&(!dllflag))){

		startexit=startlabl("__startexit");

		if(ojmp==0xff)endStartup=startexit;

	}

	else if(ojmp==0xff)endStartup=outptr;

	if(sobj==FALSE&&fobj==FALSE&&	//new 18.04.07 23:52

			comfile==file_com){

		if(resizemem){

			resizesizeaddress=startlabl("__stackseg");

			stackstartaddress=startlabl("__stackval");

		}

	}

	else if(comfile==file_sys){

		syscom=startlabl("__listcom");

	}

	else if(comfile==file_rom){

		if(modelmem==SMALL){

			stackstartaddress=startlabl("__stackstart");

			dataromstart=startlabl("__startdata");

			dataromsize=startlabl("__sizedata");

		}

		else{

			useStartup=TRUE;

			endStartup=0xfff0;

			startStartup=0;

		}

	}

	if(comsymbios==TRUE){

		output[startptr]=0xE9;

		addresshold=addresshold-(startptr+3); /* inital  */

		*(unsigned short *)&output[startptr+1]=(unsigned short)addresshold;

	}

	if(ojmp!=0xff)jumptomain=ojmp;

endp:

	if(header){

		outprocedure(aabaner,MMBANER);

		alignersize+=MMBANER;

	}

	itok=oitok;

	tok=otok;

	tok2=otok2;

	strcpy((char *)string,bstring);

	free(bstring);

	if(strcmp(itok.name,mesmain)==0)searchtree(&itok,&tok,(unsigned char *)&string);

	if((comfile!=file_d32||dpmistub)&&stubfile!=NULL){

		free(stubfile);

		stubfile=NULL;

	}

	dbgact=0;

}



void KillVarOfTree(idrec **treestart)

{

struct idrec *ptr,*leftptr,*rightptr,*prev;

int cmpresult,ocmpresult=0;

	ptr=*treestart;	//поиск

	while(ptr!=NULL&&(cmpresult=strcmp(ptr->recid,itok.name))!=0){

		prev=ptr;	//родитель

		ocmpresult=cmpresult;	//результат пред сравнения - опр в левой или правой ветви

		if(cmpresult<0)ptr=ptr->left;

		else ptr=ptr->right;

	}

	if(ptr!=NULL){	//найден объект удаления

		if(ptr->newid)free(ptr->newid);	//удалить доп информ.

		leftptr=ptr->left;	//дите

		rightptr=ptr->right;//другое дите

		if(leftptr==NULL&&rightptr==NULL){	//если нет дитей

			if(ocmpresult<0)prev->left=NULL;  //то родитель остался сиротой

			else if(ocmpresult>0)prev->right=NULL;

			else *treestart=NULL;	//удален корень без ветвей

		}

		else if(leftptr==NULL){	//одно дите справа

			if(ocmpresult<0)prev->left=rightptr;	//передать внуков родителю

			else if(ocmpresult>0)prev->right=rightptr;

			else *treestart=rightptr;	//удален корень с одной правой веткой

		}

		else if(rightptr==NULL){	//тоже если дите слева

			if(ocmpresult<0)prev->left=leftptr;

			else if(ocmpresult>0)prev->right=leftptr;

			else *treestart=leftptr;	//удален корень с одной левой веткой

		}

		else{	//если есть оба ребенка

			struct idrec *ostptr,*ptrf;

			if(ocmpresult<0){	//если мы дите слева

				prev->left=leftptr;	//передать левого ребенка

				ostptr=rightptr;    //правого к поиску места

			}

			else if(ocmpresult>0){	//если же мы дите справа

				prev->right=rightptr; //передать правого ребенка

				ostptr=leftptr;       //левого к поиску

			}

			else{                   //если у нас нет родителя

				*treestart=rightptr;   //один наугад становится главным

				ostptr=leftptr;       //другого к поиску

			}

			ptrf=*treestart;	//начало дерева

			while(((cmpresult=strcmp(ptrf->recid,ostptr->recid))<0&&ptrf->left!=NULL)||

	       (cmpresult>0&&ptrf->right!=NULL)){

				if(cmpresult<0)ptrf=ptrf->left;

				else ptrf=ptrf->right;

			}

			if(cmpresult<0)ptrf->left=ostptr;	//строка меньше

			else ptrf->right=ostptr;

		}

		if(ptr->newid)free(ptr->newid);

		if(ptr->sbuf)free(ptr->sbuf);

		free(ptr);

	}

}

/* end of TOKE.C */

