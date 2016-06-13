#define _SWITCH_

#include "tok.h"

extern int lastcommand;	//последний оператор в блоке

#define MAXCASE 1024

FSWI *swtables;	//таблица информаций об switch
int numswtable=0;	//число блоков в этой таблице
char mesSWITCH[]="SWITCH";
char mesCASE[]="CASE";
int numexpandcase;
int numberbreak=0;

void CheckJmpSW(int line,int endsw,int startsw,int shortjmp,char *mes)
{
int size=startsw-endsw;
	if(shortjmp==FALSE){
		if((unsigned int)size<128)warningjmp(mes,line);
		if(am32==FALSE)*(unsigned short *)&output[endsw-2]=(unsigned short)size;
		else *(unsigned long *)&output[endsw-4]=(unsigned long)size;
	}
	else{
		if((unsigned int)size>127)jumperror(line,mes);
		output[endsw-1]=(unsigned char) size;
	}
}

void CmpRegNum(int tokr,unsigned long value,int reg)
{
	if(tokr!=r8){
		op66(tokr);         //CMP EAX,val
		if(value==0){
			op(0x85);
			op(0xC0+reg*9);	//test AX,AX
		}
		else{
			if(short_ok(value,tokr==r32?TRUE:FALSE)){
				op(0x83);
				op(0xF8+reg);
				op(value);
			}
			else{
				if(reg==0)op(0x3D);
				else{
					op(0x81);
					op(0xF8+reg);
				}
				tokr==r32?outdword(value):outword(value);
			}
		}
	}
	else{                   //CMP AL,val
		if(value==0){
			op(0x84);
			op(0xC0+reg*9);
		}
		else{
			if(reg==0)op(0x3c);
			else{
				op(0x80);
				op(0xF8+reg);
			}
			op(value);
		}
	}
}

int ScanSwitch(int *numcase,ISW *caseinf,COM_MOD *startmod)
{
unsigned char dcha;
int dtok,line,oinptr,i,otok2;
//new
unsigned char *oinput;
int oendinptr;

ITOK otok;
int retcode=TRUE;
	dtok=tok;
	otok=itok;
	otok2=tok2;
	line=linenum2;
	dcha=cha2;
	oinptr=inptr=inptr2;
	cha=cha2;
//new
	oinput=input;
	oendinptr=endinptr;
	if(startmod!=cur_mod){
		COM_MOD *pmod=cur_mod;
		while(pmod->next!=startmod){
			pmod=pmod->next;
		}
		input=pmod->input;
		inptr=pmod->inptr;
		endinptr=pmod->endinptr;
		cha=input[inptr];
		inptr++;
	}

	if(SkipParam()){
		FastTok(0);
		if(tok!=tk_openbrace){
			SwTok(tk_openbrace);
			retcode=FALSE;
		}
		else{
			for(i=1;i!=0;){
				FastTok(1);
				if(tok==tk_question)CheckDir();
				switch(tok){
					case tk_eof:
						unexpectedeof();
						retcode=FALSE;
						i=0;
						break;
					case tk_openbrace: i++; break;
					case tk_closebrace: i--; break;
					case tk_id:
					case tk_case:
					case tk_CASE:
						if(i==1){
							if(stricmp(itok.name,"case")==0){
								inptr2=inptr;
								cha2=cha;
								linenum2=linenumber;
								(caseinf+*numcase)->postcase=(itok.name[0]=='c'?FALSE:TRUE);
								nexttok();
								if(tok==tk_number||(tok==tk_minus&&tok2==tk_number)){
									if(*numcase==MAXCASE){
										preerror("Many to use <case>");
										retcode=FALSE;
									}
									else{
										unsigned long val=doconstlongmath();
										for(int j=0;j<*numcase;j++){
											if(val==(caseinf+j)->value){
												preerror("Duplicate 'case'");
												retcode=FALSE;
												break;
											}
										}
										(caseinf+*numcase)->value=val;
										if(tok==tk_multipoint){
											(caseinf+*numcase)->type=startmulti;
											nexttok();
											val=doconstlongmath();
											numexpandcase+=val-(caseinf+*numcase)->value-1;
											if(val<(caseinf+*numcase)->value){
												preerror("The first value 'case' should be smaller");
												retcode=FALSE;
											}
											*numcase=*numcase+1;
											(caseinf+*numcase)->type=endmulti;
											(caseinf+*numcase)->value=val;
											(caseinf+*numcase)->postcase=(caseinf+*numcase-1)->postcase;
										}
										else (caseinf+*numcase)->type=singlcase;
										*numcase=*numcase+1;
									}
								}
								else{
									numexpected();
									retcode=FALSE;
								}
								inptr=inptr2;
								cha=cha2;
							}
						}
						break;
				}
			}
		}
	}
	if(retcode){	//если не найдено ошибок
		tok=dtok;
		itok=otok;
		linenum2=line;
		cha2=dcha;
		inptr2=oinptr;
		tok2=otok2;
	}
	else{
		inptr2=inptr;
		cha2=cha;
		linenum2=linenumber;
	}
//new
	input=oinput;
	endinptr=oendinptr;

	return retcode;
}

void doswitch()
{
ISW *caseinf;
int numcase=0,reg=AX,mode=0;
unsigned int endsw,defaul=0,tokr,endsw2=0;
int shortjmp=(tok==tk_switch?FALSE:TRUE);
int sline=linenumber;
REGISTERSTAT *bakregstat,*changeregstat;
char *ofsstr=NULL;
//new
COM_MOD *startmod=cur_mod;
unsigned char oinline=useinline;
#ifdef OPTVARCONST
ITOK otok;
int swvar=FALSE;
unsigned long numbervar;
int numrm;
int nonum;
#endif
	useinline=0;
	caseinf=(ISW *)MALLOC(sizeof(ISW)*MAXCASE);	//блок для инфо о case
	numexpandcase=0;
	uptdbr(/*TRUE*/);
	getoperand();
char signflag=0;
	expecting(tk_openbracket);
	switch(tok){
		case tk_intvar:
			signflag=1;
			goto dint;
		case tk_int:
			signflag=1;
		case tk_word:
			getoperand();
		case tk_wordvar:
		case tk_reg:
dint:
			tokr=r16;
			break;
		case tk_charvar:
			signflag=1;
			goto dchar;
		case tk_char:
			signflag=1;
		case tk_byte:
			getoperand();
		case tk_bytevar:
		case tk_beg:
dchar:
			tokr=r8;
			break;
		case tk_long:
			signflag=1;
		case tk_dword:
			getoperand();
			goto dlong;
		case tk_qword:
			getoperand();
			goto qword;
		case tk_floatvar:
		case tk_longvar: signflag=1;
		case tk_dwordvar:
		case tk_reg32:
dlong:
			tokr=r32;
			break;
		case tk_doublevar: signflag=1;
		case tk_qwordvar:
qword:
			tokr=r64;
			break;
		case tk_openbracket:
			nexttok();
			if(tok>=tk_char&&tok<=tk_double){
				tokr=typesize(tok);
				switch(tok){
					case tk_double:
					case tk_float:
					case tk_char:
					case tk_int:
					case tk_long:
						signflag=1;
				}
			}
			nexttok();
			expectingoperand(tk_closebracket);
			break;
		default:
			tokr=(am32+1)*2;
			break;
	}
#ifdef OPTVARCONST
	if(tok>=tk_charvar&&tok<=tk_dwordvar&&tok2==tk_closebracket){
		otok=itok;
		swvar=TRUE;
	}
#endif
	if(ScanSwitch(&numcase,caseinf,startmod)){
int i;
unsigned int sizetab;	//размер таблицы
unsigned long min=0xffffffff,max=0;
unsigned int size0=0,size1;
int reg0=0,svop=0;
long smin=0x7fffffff,smax=-0x7fffffff;
unsigned int size2=0;
unsigned oaddESP=addESP;
		if(numcase>2){
			if(!optimizespeed){
				if((am32==FALSE&&tokr==r32)||(am32&&tokr==r16))size0=numcase;
				else if(tokr==r8)size0=numcase*2;
				if((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&tok2==tk_closebracket)
					reg0=itok.number;	//предполагаемый регистр метода 0
			}
			for(i=0;i<numcase;i++){	//расчет размера по методу 0
				if((caseinf+i)->value>max)max=(caseinf+i)->value;
				if((caseinf+i)->value<min)min=(caseinf+i)->value;
				if((long) (caseinf+i)->value>smax)smax=(caseinf+i)->value;
				if((long) (caseinf+i)->value<smin)smin=(caseinf+i)->value;
				if((!optimizespeed)&&tokr!=r8){
					if((caseinf+i)->value==0)size0+=2;
					else if((caseinf+i)->value<128)size0+=3;
					else{
						size0+=(reg0==0?3:4);
						if(am32)size0+=2;
					}
				}
				if(i!=0){
					if((caseinf+i)->postcase==0)size0+=(am32==FALSE?(chip<3?5:4):6);
					else size0+=2;
				}
			}
			if((unsigned int)(smax-smin)<(max-min)){
				max=smax-smin;
				svop=8;
				min=-smin;
			}
			else{
				smin=min;
				max-=min;
			}
			sizetab=max+1;	//размер таблицы для метода 1
			if(sizetab<0x1000000&&(!(am32==FALSE&&tokr==r32))){
				if(optimizespeed){
					if((unsigned int)(sizetab/numcase)<(unsigned int)(am32==FALSE?3:4)){
						mode=1;
						if(am32==FALSE)reg=BX;
					/* если отношение числа элементов в таблице к числу case менее
					 3 для 16-битного режима и 4 для 32-битного, то берется метод 1 */
					}
				}
				else{	//вычислить размер для оптимизации по размеру
					if(shortjmp)size0+=2;
					else size0+=(am32==FALSE?(chip<3?5:4):6);
					size1=sizetab*(am32==FALSE?2:4);
					size1+=(max<128?3:am32==FALSE?4:6)+(shortjmp==FALSE?(chip<3?5:4):2);
					if(max>127&&reg0==AX)size1--;
					if(min!=0){
						if(min==1)size1++;
						else if(min==2&&(!optimizespeed))size1+=2;
						else if(min<128)size1+=3;
						else{
							size1+=(am32==FALSE?4:6);
							if(reg0==AX)size1--;
						}
					}
					size1+=(am32==FALSE?6:9);
					if(am32){
						if(tokr!=r32)size1+=3;
						if(shortjmp)size1-=2;
					}
					else{
						if(reg0!=BX)size1+=2;
					}
				//выбор метода с меньшим размером
					if(size1<=size0){
						mode=1;
						size0=size1;
						if(am32==FALSE)reg=BX;

					}
				}
			}
		}
		if(numcase>9&&(!optimizespeed)){
// расчет метода 2
			size2=numcase+numexpandcase;
			switch(tokr){
				case r8:
					if(am32)size2=size2*5;
					else size2=size2*3+8;
					break;
				case r16:
					if(am32)size2=size2*6+1;
					else size2=size2*4+8;
					break;
				case r32:
					if(am32)size2=size2*8;
					else size2=size2*6+9;
					break;
			}
			size2+=29;
			//выбор метода с меньшим размером
			if(size2<=size0)mode=2;
		}
//		printf("Num CASE %d Metod 0 size=%d. Metod 1 size=%d. Metod 2 size=%d\n",numcase,size0,size1,size2);
		if(mode==2){
			reg=AX;
			reg0=idxregs[1];
			if((!am32)||(am32&&(!((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&tok2==tk_closebracket)))){
				if(tokr==r8)doalmath(signflag,&ofsstr);
				else do_e_axmath(signflag,tokr,&ofsstr);
			}
			else{
				reg=itok.number;
				if(reg==reg0)reg0=idxregs[0];
				nexttok();
			}
				ClearReg(reg);
				ClearReg(reg0);
			warningreg(regs[am32][reg0]);
		}
		else if((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&tok2==tk_closebracket){
			if(reg==AX){
				reg=itok.number;
				if(mode&&am32)getintoreg_32(reg,r32,0,&ofsstr);
				else nexttok();
				ClearReg(AX);
			}
			else{
				getintoreg_32(BX,r16,0,&ofsstr);
				ClearReg(BX);
				warningreg("BX");
			}
		}
		else{
			if(tokr==r8)doalmath(signflag,&ofsstr);
			else do_e_axmath(signflag,tokr,&ofsstr);
			if(reg!=AX){
				ClearReg(BX);
				if(tokr==r8)outword(0xB4);	//mov ah,0
				if(optimizespeed)outword(0xC389);	//mov bx,ax
				else op(0x93);	//xchg ax,bx
			}
			else{
				ClearReg(AX);
				if(mode&&am32&&tokr!=r32){
					op(0x0F);
					op(tokr==r8?0xB6:0xB7);
					op(0xC0);
				}
			}
		}
		nextexpecting2(tk_openbrace);	//пров на откр скобку
		if(numcase){
			if(mode==1){	//первый метод
				if(min!=0){
					if(min==1)op(0x48+reg-svop);	//dec reg
					else if(min==2&&(!optimizespeed)){
						op(0x48+reg-svop);	//dec reg
						op(0x48+reg-svop);	//dec reg
					}
					else if(min<128){
						op(0x83);
						op(0xE8+reg-svop*5);
						op(min);
					}
					else{
						if(reg==AX)op(0x2D-svop*5);
						else{
							op(0x81);
							op(0xE8+reg-svop*5);
						}
						if(am32)outdword(min);
						else outword(min);
					}
					if(am32)warningreg(regs[1][reg]);
				}
				if(max<128){
					op(0x83);	//cmp reg,max
					op(0xF8+reg);
					op(max);
				}
				else{
					if(reg==AX)op(0x3D);
					else{
						op(0x81);
						op(0xF8+reg);
					}
					if(am32)outdword(max);
					else outword(max);
				}
				if(shortjmp==FALSE){
					if(chip<3){
						outword(0x376);	//jbe 3
						op(0xE9);	//jmp to default
					}
					else outword(0x870F);	//ja default
					outword(0);
					if(am32)outword(0);
				}
				else{
					op(0x77);
					op(0);
				}
				endsw=outptr;	//адрес конца switch или default
				if(am32){
					outword(0x24FF);
					op(0x85+reg*8);
				}
				else outdword(0xA7FFDB01);	//add bx,bx jmp [bx+table]
				AddReloc(CS);
				size1=outptr;
				outword(0);
				if(am32)outword(0);
			}
			else if(mode==0){
				svop=numcase;
				for(;numcase>0;){	//ветвление
					numcase--;
					CmpRegNum(tokr,(caseinf+numcase)->value,reg);
					if((caseinf+numcase)->type==singlcase){
						if(numcase){
							if((caseinf+numcase)->postcase==FALSE){
								if(chip<3){
									outword(0x375);	//jnz 3
									op(0xE9);	//jmp to default
								}
								else outword(0x840F);	//jz default
								outword(0);
								if(am32)outword(0);
							}
							else outword(0x74);
							(caseinf+numcase)->postcase=outptr;
						}
						else{
							if(shortjmp==FALSE){
								if(chip<3){
									outword(0x374);	//jnz 3
									op(0xE9);	//jmp to default
								}
								else outword(0x850F);	//jz default
								outword(0);
								if(am32)outword(0);
							}
							else outword(0x75);
							endsw=outptr;	//адрес конца switch или default
						}
					}
					else{	//case 1...5
						numcase--;
						max=(caseinf+numcase)->value;
						if(numcase!=0){
							if((caseinf+numcase+1)->postcase==FALSE){
								if(max==0){
									if(chip>2){
										outdword(0x860F);
										if(am32)outword(0);
									}
									else{
										outword(am32==FALSE?0x0377:0x577);
										jumploc0();
									}
								}
								else{
									outword(0x77);
									endsw=outptr;
									CmpRegNum(tokr,max,reg);
									if(chip>2){
										outdword(0x830F);
										if(am32)outword(0);
									}
									else{
										outword(am32==FALSE?0x0372:0x572);
										jumploc0();
									}
									output[endsw-1]=(unsigned char)(outptr-endsw);
								}
							}
							else{
								if(max==0)outword(0x76);
								else{
									outword(0x77);
									endsw=outptr;
									CmpRegNum(tokr,max,reg);
									outword(0x73);
									output[endsw-1]=(unsigned char)(outptr-endsw);
								}
							}
							(caseinf+numcase)->postcase=outptr;
						}
						else{
							if(shortjmp==FALSE){
								if((optimizespeed&&chip>2)||(chip>2&&max==0)){
									outdword(0x870F);
									if(am32)outword(0);
									if(max!=0){
										endsw2=outptr;
										CmpRegNum(tokr,max,reg);
										outdword(0x820F);
										if(am32)outword(0);
									}
								}
								else{
									if(max==0) outword(am32==FALSE?0x0376:0x576);
									else{
										outword(0x77);
										endsw=outptr-1;
										CmpRegNum(tokr,max,reg);
										outword(am32==FALSE?0x0373:0x573);
										output[endsw]=(unsigned char)(outptr-endsw-1);
									}
									jumploc0();
								}
							}
							else{
								outword(0x77);
								if(max!=0){
									endsw2=outptr;
									CmpRegNum(tokr,max,reg);
									outword(0x72);
								}
							}
							endsw=outptr;	//адрес конца switch или default
						}
					}
				}
			}
			else if(mode==2){
				if(!am32){
					outdword(0x071E0651);	//push cx,es,ds pop es
					op(0xB9);	//mov CX,numcase
					outword(numcase+numexpandcase);
					op(0xBF);	//mov DI,tableval
					AddReloc(CS);
					size2=outptr;
					outword(0);
					op(0xF2);	//repnz
					switch(tokr){
						case r8: op(0xAE); break;
						case r32: op(0x66);
						case r16: op(0xAF); break;
					}
					outword(0x7407);	//pop es ,jz
					op(shortjmp==FALSE?4:3);
					op(0x59);	//pop cx
					if(shortjmp==FALSE)jumploc0();
					else outword(0xEB);
					endsw=outptr;	//адрес конца switch или default
					op(0xBF);	//mov DI,numcase
					outword(numcase+numexpandcase);
					outdword(0x014FCF29);	//sub di,cx dec di add di,di
					op(0xFF);
					outword(0xC781);	//add di,tableadr
					AddReloc(CS);
					size1=outptr;
					outword(0);
					op(0x59);	//pop cx
					outword(0x25FF);	//jmp [di]
				}
				else{
					op(0x31);
					op(0xC0+reg0*9);	//xor reg0,reg0
					switch(tokr){	//cmp [reg0*size+tableadr],reg
						case r8: op(0x38); op(0x4+reg*8); op(5+reg0*8); break;
						case r32: op(0x39); op(0x4+reg*8); op(0x85+reg0*8); break;
						case r16: op(0x66); op(0x39); op(0x4+reg*8); op(0x45+reg0*8); break;
					}
					AddReloc(CS);
					size2=outptr;
					outdword(0);
					outword(0x0775);	//jne
					outword(0x24FF);	//jmp [reg0*4+tableadr]
					op(0x85+reg0*8);
					AddReloc(CS);
					size1=outptr;
					outdword(0);
					op(0x40+reg0);	//inc reg0
					if((numcase+numexpandcase)>127){	//cmp reg0,numcase
						op(0x81);
						op(0xF8+reg0);
						outdword(numcase+numexpandcase);
					}
					else{
						op(0x83);
						op(0xF8+reg0);
						op(numcase+numexpandcase);
					}
					op(0x72);
					if((numcase+numexpandcase)>127)op(tokr==r16?0xE6:0xE7);
					else op(tokr==r16?0xE9:0xEA);
					if(shortjmp==FALSE)jumploc0();
					else outword(0xEB);
					endsw=outptr;	//адрес конца switch или default
				}
			}
		}
		numcase=0;
		useinline=oinline;
		bakregstat=BakRegStat();
		changeregstat=BakRegStat();
		lastcommand=tk_switch;
		do{
			if(tok==tk_case||tok==tk_CASE||tok==tk_default){     //если case - запомнить позицию и величину
				RestoreStack();
				CompareRegStat(changeregstat);
				switch(lastcommand){
					case tk_goto:
					case tk_GOTO:
					case tk_break:
					case tk_BREAK:
					case tk_return:
					case tk_RETURN:
					case tk_switch:
						CopyRegStat(bakregstat);
#ifdef OPTVARCONST
						nonum=FALSE;
#endif
						addESP=oaddESP;
						break;
					default:
						CopyRegStat(changeregstat);
#ifdef OPTVARCONST
						nonum=TRUE;
#endif
						if(ESPloc&&am32&&oaddESP!=addESP)warESP();
						break;
				}
				if(tok==tk_default){	//тоже для default
					if(mode==0&&svop){
						if(numcase==0)jumploc0();	//default самый первый
						CheckJmpSW(sline,endsw,outptr,shortjmp,mesSWITCH);
						if(endsw2)CheckJmpSW(sline,endsw2,outptr,shortjmp,mesSWITCH);
					}
					if(defaul)preerror("Duplicate 'default'");
					defaul=outptr;
					nexttok();
					expecting(tk_colon);	//пров на : и чтение следующ tok
					continue;
				}
				if(mode==0){
					if(numcase==0&&defaul){	//default самый первый
						if(am32==FALSE)*(unsigned short *)&output[defaul-2]=(unsigned short)(outptr-defaul);
						else *(unsigned long *)&output[defaul-4]=(unsigned long)(outptr-defaul);
					}
					else if(numcase)CheckJmpSW(linenumber,(caseinf+numcase)->postcase,outptr,tok==tk_case?FALSE:TRUE,mesCASE);
				}
				else (caseinf+numcase)->postcase=outptr;
				lastcommand=tok;	//new 12.12.07 14:27
				nexttok();
#ifdef OPTVARCONST
				numrm=itok.rm;
				numbervar=doconstlongmath();
				(caseinf+numcase)->value=numbervar-(mode==1?smin:0);
#else
				(caseinf+numcase)->value=doconstlongmath()-(mode==1?smin:0);
#endif
				numcase++;
				if(tok==tk_multipoint){
					nexttok();
					(caseinf+numcase)->value=doconstlongmath()-(mode==1?smin:0);
					numcase++;
				}
#ifdef OPTVARCONST
				else if(swvar&&nonum==FALSE)Const2Var(&otok,numbervar,numrm);
#endif
				expecting(tk_colon);	//пров на : и чтение следующ tok
				if(tok==tk_closebrace)numcase--;
				continue;//goto checkcase;
			}
			startblock();
			docommand();
			endblock();
		}while(tok!=tk_closebrace);
		RestoreStack();
		if(numberbreak==0){
			switch(lastcommand){
				case tk_break:
					posts--;
					outptr-=(am32==FALSE?3:5);
					break;
				case tk_BREAK:
					posts--;
					outptr-=2;
					break;
			}
		}
		if(defaul&&outptr<defaul){
			if(mode==0&&svop){
				if(numcase==0)jumploc0();	//default самый первый
				CheckJmpSW(sline,endsw,outptr,shortjmp,mesSWITCH);
				if(endsw2)CheckJmpSW(sline,endsw2,outptr,shortjmp,mesSWITCH);
			}
			defaul=outptr;
		}
//		printf("outptr=%08X defaul=%08X\n",outptr,defaul);
		CompareRegStat(changeregstat);
		FreeStat(bakregstat);
		CopyRegStat(changeregstat);
		FreeStat(changeregstat);
		if(numcase){
			if(mode==0){
				if(defaul==0){
					CheckJmpSW(sline,endsw,outptr,shortjmp,mesSWITCH);
					if(endsw2)CheckJmpSW(sline,endsw2,outptr,shortjmp,mesSWITCH);
				}
				free(caseinf);
			}
			else{
				if(defaul==0||defaul>outptr)defaul=outptr;
				CheckJmpSW(sline,endsw,defaul,shortjmp,mesSWITCH);
				caseinf=(ISW *)REALLOC(caseinf,sizeof(ISW)*numcase);
				if(!numswtable)swtables=(FSWI *)MALLOC(sizeof(FSWI));
				else swtables=(FSWI *)REALLOC(swtables,sizeof(FSWI)*(numswtable+1));
				FSWI *swt=swtables+numswtable;
				swt->info=caseinf;
				swt->sizetab=(mode==2?numcase:sizetab)+numexpandcase;
				swt->type=(am32==FALSE?2:4);
				swt->numcase=numcase;
				swt->defal=defaul;
				swt->ptb=size1;
				swt->ptv=size2;
				swt->mode=mode;
				swt->razr=tokr;
				numswtable++;
			}
		}
		else free(caseinf);
		SetBreakLabel();
		SetContinueLabel();
	}
	nexttok();
	lastcommand=tk_switch;
	retproc=FALSE;
}
