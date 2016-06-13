
#define _TOKA_

#include "tok.h"

#pragma option -w-pin

#ifdef _WIN32_
#include <windows.h>
#endif

#include <unistd.h>
#include <sys/stat.h>

#include "dirlist.h"
#include "id.h"
#include "resname.h"

struct idrec *treestart=NULL;
struct idrec *definestart=NULL;
UNDEFOFF *undefoffstart=NULL;
DLLLIST *listdll=NULL;
struct structteg *tegtree=NULL;	//глобальный срисок тегов
struct structteg *ltegtree=NULL;	//локальный срисок тегов
//struct idrec *lstructlist=NULL;  //список локальных структур
SINFO strinf={NULL};
static int notdef=TRUE;
static char precha;
int scanalltoks=TRUE;


static volatile idrec **DynamicList=NULL;
static int sizeDL;	//размер списка
static volatile int countDP;	//число динамических процедур в списке
static int findofset=FALSE;
#define STEPDL 128;	//шаг увеличения размера списка
ITOK structadr;


char id2[ID2S][9]={
	"ESCHAR","ESBYTE","ESINT","ESWORD","ESLONG","ESDWORD","ESFLOAT","ESQWORD","ESDOUBLE",
	"CSCHAR","CSBYTE","CSINT","CSWORD","CSLONG","CSDWORD","CSFLOAT","CSQWORD","CSDOUBLE",
	"SSCHAR","SSBYTE","SSINT","SSWORD","SSLONG","SSDWORD","SSFLOAT","SSQWORD","SSDOUBLE",
	"DSCHAR","DSBYTE","DSINT","DSWORD","DSLONG","DSDWORD","DSFLOAT","DSQWORD","DSDOUBLE",
	"FSCHAR","FSBYTE","FSINT","FSWORD","FSLONG","FSDWORD","FSFLOAT","FSQWORD","FSDOUBLE",
	"GSCHAR","GSBYTE","GSINT","GSWORD","GSLONG","GSDWORD","GSFLOAT","GSQWORD","GSDOUBLE",};

char regs[2][8][4]={"AX","CX","DX","BX","SP","BP","SI","DI",
                   "EAX","ECX","EDX","EBX","ESP","EBP","ESI","EDI"};
char begs[8][3]={"AL","CL","DL","BL","AH","CH","DH","BH"};
char segs[6][3]={"ES","CS","SS","DS","FS","GS"};

char mon[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
				 "Sep","Oct","Nov","Dec"};

unsigned char cha2;
char skipfind=FALSE;	/* пропуск поиска в глобальном дереве и среди
															  локальных переменных */
static unsigned char savestring3=FALSE;	//разрешить запись в буфер string3
static int posstr3;	//указатель позиции в string3

unsigned int inptr2;
unsigned int linenum2=0;	//если не нуль, то идет обраьотка
char displaytokerrors;		/* flag to display errors, 0 for tok2 scan */
char *bufrm=NULL;
char *startline=NULL;
char *endinput=NULL;
unsigned char skiplocals=FALSE;
int scanlexmode=STDLEX;
unsigned char bytesize=TRUE;

COM_MOD *cur_mod=NULL;

void docals(struct idrec *ptr);
void dostructvar2(int *tok4,ITOK *itok4,struct structteg *tteg,unsigned char *string4);	//разбор структур на переменные и структуры
void dosizeof(ITOK *itok4);	//опр значение sizeof
void ofsstr(int *tok4,ITOK *itok4);
int searchlocals(ITOK *itok4,int *tok4,unsigned char *string4);
unsigned char convert_char();
void tokscan (int *tok4,ITOK *itok4,unsigned char *string4);
int calcnum(int *ctok,ITOK *cstok,char *cstring,long *retval);
int GetDirective(char *str);
int CheckAsmName(char *name);
void tag_massiv(int *tok4,ITOK *itok4,unsigned char *string4);
int CheckResName(char *name);
void GetTypeVar(int *tok4);
int RegToRM(int number,int tok4);
elementteg *FindClassEl(structteg *searcht,unsigned char *string4,int *addofs,
	structteg *subteg);
int AskUseDestr(structteg *searcht);
int SaveStruct(int size,idrec *newrec);
int CheckDefPar(char *name);
int searchtree2(idrec *fptr,ITOK *itok4,int *tok4,unsigned char *string4);

extern void blockerror();
extern void block16_32error();
extern void notstructname();
extern void badtoken();
extern void opb(unsigned long num,unsigned int ofs,unsigned int size);
extern void CorrectOfsBit(int bitofs);
extern int skipstring(int pos,unsigned char term);
extern int skipcomment(int pos);


void retoldscanmode(int mode)
{
	if(mode==STDLEX&&cha2==13&&tok!=tk_endline){
		cha2=cha;
	}
	scanlexmode=mode;
}

void CheckConvertString(char *string4)
{
	if(cha=='o'){
		OemToChar(string4,string4);
		nextchar();
	}
	else if(cha=='w'){
		CharToOem(string4,string4);
		nextchar();
	}
}

void DateToStr(char *buf)
{
//	GetDateFormat(LOCALE_SYSTEM_DEFAULT,DATE_SHORTDATE|LOCALE_NOUSEROVERRIDE,NULL,NULL,buf,80);
	sprintf(buf,"%2d %s %d",timeptr.tm_mday,mon[timeptr.tm_mon],timeptr.tm_year+1900);
}

void compressoffset(ITOK *thetok)
{
	if(thetok->sib==CODE16){
		if(thetok->rm!=rm_d16){
			thetok->rm&=7;
			if(thetok->number==0){
				if(thetok->rm==rm_BP)thetok->rm|=rm_mod01;
			}
			else if((int)thetok->number<128&&(int)thetok->number>=-128)thetok->rm|=rm_mod01;
			else thetok->rm|=rm_mod10;
		}
	}
	else{
		if(thetok->rm!=rm_d32&&thetok->rm!=rm_sib){
			thetok->rm&=7;
			if(thetok->number==0){
				if(thetok->rm==5)thetok->rm|=rm_mod01;
				if(thetok->rm==4&&(thetok->sib&7)==5)thetok->rm|=rm_mod01;
			}
			else if(thetok->number<128&&thetok->number>=-128)thetok->rm|=rm_mod01;
			else thetok->rm|=rm_mod10;
		}
	}
}

int CalcRm16(int base,int idx)
{
int rm;
	rm=0;
	switch(base+idx){
		case 2: rm++;
		case 4: rm++;
		case 6: rm++;
		case 5: rm++;
		case 12: rm++;
		case 11: rm++;
		case 10: rm++;
	}
	return rm;
}

void SRBackBuf(int mode)	//save/restore BackTextBlock
{
static int size=0;
static char *buf;
	if(mode==0){
		if(SizeBackBuf){
			size=SizeBackBuf;
			SizeBackBuf=0;
			buf=BackTextBlock;
		}
	}
	else{
		SizeBackBuf=size;
		size=0;
		BackTextBlock=buf;
	}
}

void ExpandRm(int rm,int sib,int *zoom,int *base,int *idx)
{
int rm0;
int reg1=-1,reg2=-1;
int z=0;
	rm0=rm&0x7;
	if(sib==CODE16||sib==(CODE16+1)){
		switch(rm0){
			case 0:
				reg1=BX;
				reg2=SI;
				break;
			case 1:
				reg1=BX;
				reg2=DI;
				break;
			case 2:
				reg1=BP;
				reg2=SI;
				break;
			case 3:
				reg1=BP;
				reg2=DI;
				break;
			case 4:
				reg2=SI;
				break;
			case 5:
				reg2=DI;
				break;
			case 6:
				if((rm&0xC0)!=0)reg1=BP;
				break;
			case 7:
				reg1=BX;
				break;
		}
	}
	else{
		if(rm0==4){
			reg1=sib&7;
			reg2=(sib>>3)&7;
			if(reg1==5&&(rm&0xc0)==0)reg1=-1;
			if(reg2==4)reg2=-1;
			else z=sib>>6;
		}
		else{
			reg1=rm0;
			if(reg1==5&&(rm&0xc0)==0)reg1=-1;
		}
	}
	*base=reg1;
	*idx=reg2;
	*zoom=z;
}

int CheckZoom(int size)
{
int zoom=0;
	switch(size){
		case 8: zoom++;
		case 4: zoom++;
		case 2: zoom++;
	}
	return zoom;
}

void calcrm(ITOK *itok4,int ttok)//обработка выражения в []
{
int idx,base,razr=0,zoom,rm=0;
long numrm=0,cnum,ocnum;
int nextscan;
ITOK cstok,dstok;
int ctok,sopenb;
unsigned char *pstring;
unsigned int flag;
unsigned int sizevar=1;
unsigned int prevtok=tk_number,operand=tk_plus;
int dsword,dsword2;
	nextchar();
	whitespace();//пропуск незначащих символов
	if(!displaytokerrors){
		for(int i=1;i!=0;){
			FastTok(0,&ctok,&cstok);
			switch(ctok){
				case tk_closeblock: i--; break;
				case tk_openblock: i++; break;
				case tk_eof: i=0; break;
			}
		}
		return;
	}

	dsword2=dsword=itok4->post&POINTER;
	itok4->post&=NOTPOINTER;
	memcpy(&dstok,itok4,sizeof(ITOK));
	ExpandRm(dstok.rm,dstok.sib,&zoom,&base,&idx);
	if((dstok.sib==CODE16||dstok.sib==(CODE16+1))&&dstok.rm!=rm_d16)razr=r16;
	if(am32&&dstok.rm!=rm_d32){
		rm=dstok.rm&0xC0;
		razr=r32;
	}
//	printf("in idx=%d base=%d zoom=%d\n",idx,base,zoom);
	pstring=(unsigned char *)MALLOC(STRLEN);
	sopenb=inptr;
	char bcha=cha;
	SRBackBuf(0);
	flag=f_useidx;//0;//cstok.flag;
	if(bytesize==FALSE){
		sizevar=GetVarSize(ttok);
		if(dsword)sizevar=1;
		if(am32)zoom=CheckZoom(sizevar);
	}
	if(cha=='&'){
		nextchar();
		whitespace();//пропуск незначащих символов
		sizevar=1;
		zoom=0;
	}
	for(;;){
		nextscan=TRUE;
		if(cha=='#'&&dsword)dsword=1;
		tokscan(&ctok,&cstok,pstring);
		whitespace();//пропуск незначащих символов
		if(dsword==1)strcpy(dstok.name,cstok.name);
//		printf("tok=%d num=%d %s\n",ctok,cstok.number,cstok.name);
loopsw:
		switch(ctok){
			case tk_reg:
				if(razr==0)razr=r16;
				else if(razr==r32||sizevar!=1||operand!=tk_plus)goto runblock;	//16.12.04 22:39
				if(cstok.number==BP||cstok.number==BX){
					if(base==-1)base=cstok.number;
					else goto runblock;
				}
				else if(cstok.number==SI||cstok.number==DI){
					if(idx==-1)idx=cstok.number;
					else goto runblock;
				}
				else goto runblock;
				prevtok=tk_reg;
				break;
			case tk_minus:
				if(calcnum(&ctok,&cstok,(char *)pstring,&cnum)==0)goto runblock;
				prevtok=tk_number;
				ocnum=cstok.number;
				goto enumb;
			case tk_number:
				if(operand==tk_mult&&prevtok==tk_reg32){
					if(zoom==0){
						prevtok=tk_number;
						if(razr==0)razr=r32;
						else if(razr==r16)goto runblock;
						zoom=CheckZoom(cstok.number);
						if(zoom)break;
					}
					else goto runblock;
				}
				prevtok=tk_number;
				ocnum=cstok.number;
				if(zoom==0&&razr!=r16&&(idx==-1||base==-1)){
					if((zoom=CheckZoom(cstok.number))!=0){
						if(cha=='*'){
							nextchar();
							tokscan(&ctok,&cstok,pstring);
							if(ctok==tk_reg32){
								if(idx!=-1)base=idx;
								idx=cstok.number;
								prevtok=tk_reg32;
								if(razr==0)razr=r32;
								break;
							}
							if(ctok==tk_number){
								calclongnumber(&ocnum,cstok.number,tk_mult);
								numrm+=ocnum;
								zoom=0;
								break;
							}
							goto runblock;
						}
						zoom=0;
					}
				}
				if(calcnum(&ctok,&cstok,(char *)pstring,&cnum)==0)goto runblock;
enumb:
				flag^=cstok.flag;
				whitespace();//пропуск незначащих символов
				ocnum=cnum;
				numrm+=cnum;
				if(cstok.type==tp_opperand||cstok.type==tp_stopper)nextscan=FALSE;
				else{
					operand=tk_plus;
					goto loopsw;
				}
				break;
			case tk_reg32:
//	printf("prevtok=%d operand=%d ocnum=%d idx=%d base=%d zoom=%d\n",prevtok,operand,ocnum,idx,base,zoom);
				if(razr==0)razr=r32;
				if(razr==r16||(operand!=tk_plus&&operand!=tk_mult)||(idx!=-1&&base!=-1))goto runblock;
				if(sizevar!=1){
					if(idx!=-1)goto runblock;
					idx=cstok.number;
				}
				else if(operand==tk_mult){
					if(prevtok!=tk_number)goto runblock;
					zoom=CheckZoom(ocnum);
					numrm-=ocnum;
					if(idx!=-1)base=idx;
					idx=cstok.number;
				}
				else if(cha=='*'){
					if(idx!=-1)base=idx;
					idx=cstok.number;
				}
				else if(base==-1)base=cstok.number;
				else if(idx==-1)idx=cstok.number;
//					else goto runblock;
				prevtok=tk_reg32;
				break;
			case tk_postnumber:
				if(dsword==0||sizevar!=1)goto runblock;
				if((cstok.post&USED_DIN_VAR)==USED_DIN_VAR){	//динамическая переменная
					if(dstok.rec!=NULL)goto runblock;
					dstok.rec=cstok.rec;
				}
				if(operand==tk_minus)numrm-=cstok.number;
				else numrm+=cstok.number;
				dstok.post|=cstok.post;
				prevtok=tk_number;
				break;
			case tk_undefofs:
				if(dsword==0||sizevar!=1)goto runblock;
				if(operand==tk_minus)numrm-=cstok.number;
				else numrm+=cstok.number;
				dstok.post|=UNDEF_OFSET;
				strcpy(dstok.name,cstok.name);
				flag^=cstok.flag;
				prevtok=tk_number;
				break;
			case tk_rmnumber:
/*				if(strinf.bufstr!=NULL){
					free(strinf.bufstr);
					strinf.bufstr=NULL;
					goto runblock;
				}*/
				if(dsword==0||sizevar!=1)goto runblock;
				if(operand==tk_minus)numrm-=cstok.number;
				else numrm+=cstok.number;
				dstok.post|=cstok.post;
				if(dstok.sib==CODE16){
					if(cstok.rm!=rm_d16){
						switch(cstok.rm&7){
							case 0:
								if(base!=-1||idx!=-1)goto runblock;
								base=BX; idx=SI; break;
							case 1:
								if(base!=-1||idx!=-1)goto runblock;
								base=BX; idx=DI; break;
							case 2:
								if(base!=-1||idx!=-1)goto runblock;
								base=BP; idx=SI; break;
							case 3:
								if(base!=-1||idx!=-1)goto runblock;
								base=BP; idx=DI; break;
							case 4:
								if(idx!=-1)goto runblock;
								idx=SI; break;
							case 5:
								if(idx!=-1)goto runblock;
								idx=DI; break;
							case 6:
								if(idx!=-1)goto runblock;
								base=BP; break;
							case 7:
								if(idx!=-1)goto runblock;
								base=BX; break;
						}
					}
					razr=r16;
				}
				else{
					if(cstok.rm!=rm_d32){
						rm=cstok.rm&0xC0;
						cstok.rm&=7;
						if(base==-1)base=cstok.rm;
						else if(idx==-1){
							idx=base;
							base=cstok.rm;
						}
						else goto runblock;
						if(base==4){
							base=cstok.sib&7;
							idx=(cstok.sib>>3)&7;
							if(base==5&&rm==0)base=-1;
							if(idx==4)idx=-1;
						}
					}
					else dstok.flag|=cstok.flag&f_reloc;
					razr=r32;
				}
				break;
//-----------------28.07.98 13:59-------------------
// var[i]
//--------------------------------------------------
			default:
runblock:
				if(dsword2){
					CharToBackBuf('&');
					if(strinf.bufstr){
						free(strinf.bufstr);
						strinf.bufstr=NULL;
					}
				}
				AddBackBuf(sopenb,bcha);
				if(bufrm){
					if(strcmp(bufrm,"&this;")==0){
						free(bufrm);
						CharToBackBuf(0);
						sprintf((char *)string3,"&%s*%d+this;",BackTextBlock,GetVarSize(ttok));
						bufrm=BackString((char *)string3);
//							puts((char *)string3);
						goto con1;
					}
					else free(bufrm);
				}
				CharToBackBuf(';');
				CharToBackBuf(0);
				bufrm=(char *)REALLOC(BackTextBlock,SizeBackBuf+1);
con1:
				if(cha!=']')blockerror();
				tokscan(&ctok,&cstok,pstring);
				if(itok4->sib>=CODE16)itok4->sib++;
				SRBackBuf(1);
//				if(itok4->post&POINTER)itok4->post=0;//&=NOTPOINTER;
				free(pstring);
				itok4->flag|=flag;
				return;
//-----------------28.07.98 13:59-------------------
// end
//--------------------------------------------------
		}
		if(dsword==1)dsword=0;
		if(nextscan){
			tokscan(&ctok,&cstok,pstring);
			whitespace();//пропуск незначащих символов
		}
		while(ctok==tk_minus){
			if(calcnum(&ctok,&cstok,(char *)pstring,&cnum)==0)goto runblock;
			whitespace();//пропуск незначащих символов
			numrm+=cnum;
			flag^=cstok.flag;
			if(cstok.type!=tp_opperand&&cstok.type!=tp_stopper){
				operand=tk_plus;
				goto loopsw;
			}
		}
		operand=ctok;
//		printf("operand tok=%d flag=%08X %s\n",ctok,cstok.flag,cstok.name);
		if(ctok==tk_closeblock||ctok==tokens||ctok==tk_eof)break;
		if(ctok!=tk_plus&&ctok!=tk_mult)goto runblock;
//		flag|=cstok.flag;
	}
//	printf("out idx=%d base=%d zoom=%d\n",idx,base,zoom);
	SRBackBuf(1);
	if(ctok!=tk_closeblock)expected(']');
	numrm*=sizevar;
	dstok.number+=numrm;
	if(razr==r16){
		if(idx==-1&&base==-1)cstok.rm=6;
		else{
			cstok.rm=CalcRm16(base,idx)|rm_mod10;
			dstok.sib=CODE16;
		}
	}
	else if(razr==0){
		if(dstok.number<65536&&dstok.number>-65535)cstok.rm=6;
		else{
			cstok.rm=5;
			dstok.sib=0;
		}
		if(am32)cstok.rm=5;
	}
	else{
//		printf("razr=%d sib=%08X %s\n",razr,dstok.sib,dstok.name);
		if(dstok.sib>CODE32)dstok.sib-=CODE32;
		else dstok.sib=0;
		if(idx!=-1){
			cstok.rm=4;
			if(idx==4)preerror("ESP cannot be the index register");
			if(base!=-1)dstok.sib=(zoom<<6)+(idx<<3)+base;
			else{
				dstok.sib=(zoom<<6)+(idx<<3)+5;
				rm=(base==5?rm_mod10:0);
			}
		}
		else{
			if(base==4){
				dstok.sib=(4<<3)+4;
				cstok.rm=4;
			}
			else if(base==-1){
				cstok.rm=rm_d32;
			}
			else cstok.rm=base;
		}
		if(base!=-1||rm!=0)cstok.rm|=rm_mod10;	//корекция MOD будет позднее, а сейчас максимум
	}
	dstok.rm=cstok.rm;
	dstok.flag|=flag;
	memcpy(itok4,&dstok,sizeof(ITOK));
	free(pstring);
}

int calcnum(int *ctok,ITOK *cstok,char *cstring,long *retval)
{
long value;
unsigned int flag;
	if(*ctok==tk_minus){
		tokscan(ctok,cstok,(unsigned char *)cstring);
		if(*ctok!=tk_number)return(0);
		cstok->number=-cstok->number;
	}
	else if(*ctok!=tk_number)return(0);
	value=cstok->number;
	flag=cstok->flag;
	for(;;){
int otok;
		tokscan(ctok,cstok,(unsigned char *)cstring);
		otok=*ctok;
		if(otok==tk_closeblock)break;
		tokscan(ctok,cstok,(unsigned char *)cstring);
		if(*ctok!=tk_number){
			if(*ctok==tk_reg32&&(otok==tk_mult||otok==tk_minus)){
				*ctok=(unsigned int)cstok->number;
				*retval=value;
				cstok->flag=flag;
				return 0;
			}
			break;
		}
		if(calclongnumber(&value,cstok->number,otok)==FALSE){
			if(displaytokerrors)blockerror();
			break;
		}
/*		switch(otok){
			case tk_minus: value-=cstok->number; break;
			case tk_plus: value+=cstok->number; break;
			case tk_xor: value^=cstok->number; break;
			case tk_and: value&=cstok->number; break;
			case tk_or: value|=cstok->number; break;
			case tk_mod: value%=cstok->number; break;
			case tk_div: value/=cstok->number; break;
			case tk_mult: value*=cstok->number; break;
			case tk_rr: value>>=cstok->number; break;
			case tk_ll: value<<=cstok->number; break;
			case tk_xorminus: value^=-cstok->number; break;
			case tk_andminus: value&=-cstok->number; break;
			case tk_orminus: value|=-cstok->number; break;
			case tk_modminus: value%=-cstok->number; break;
			case tk_divminus: value/=-cstok->number; break;
			case tk_multminus: value*=-cstok->number; break;
			case tk_rrminus: value>>=-cstok->number; break;
			case tk_llminus: value<<=-cstok->number; break;
			default: if(displaytokerrors)blockerror(); goto end;
		}*/
		flag^=cstok->flag;
	}
//end:
	cstok->flag=flag;
	*retval=value;
	return -1;
}

void CheckReg(int idx,int base,int *reg1,int *reg2,int razr)
{
	if(razr==r32){
		unsigned char lreg[8];
		int i;
		for(i=0;i<8;i++){
			if(i==ESP||i==EBP)lreg[i]=1;
			else{
				if((idx!=-1&&idx==i)||(base!=-1&&base==i))lreg[i]=1;
				else lreg[i]=0;
			}
//			printf("%c",lreg[i]+'0');
		}
		if(lreg[*reg1]==0)lreg[*reg1]=1;
		else{
			for(i=8;i!=0;i--){
				if(lreg[i]==0){
					lreg[i]=1;
					*reg1=i;
					break;
				}
			}
		}
//		printf("\nreg1=%d",*reg1);
		if(lreg[*reg2]!=0){
			for(i=8;i!=0;i--){
				if(lreg[i]==0){
					*reg2=i;
					break;
				}
			}
		}
//		printf(" reg2=%d\n",*reg2);
	}
	else{
		if(base!=-1){
			if(base==*reg1)*reg1=SI;
			else if(base==*reg2)*reg2=DI;
			if(*reg1==*reg2){
				if(*reg1==SI)*reg2=DI;
				else *reg2=SI;
			}
		}
		if(idx!=-1){
			if(idx==SI)*reg1=DI;
			else *reg1=SI;
			*reg2=CX;
		}
	}
}

void CheckAllMassiv(char *&buf,int sizeel,SINFO *strc,ITOK *ctok,int reg1,int reg2)
{
int zoom0,idx0,base0,newreg;
int zoom1=0,zoom2=0;
char pv=FALSE;
int razr=r32;
//printf("sib=%X\n",ctok->sib);
	switch(ctok->sib){
		case CODE16+2:
		case CODE32+2: ctok->sib--;
		case CODE16+1:
		case CODE32+1: ctok->sib--;
	}
	if(ctok->sib==CODE16){
		razr=r16;
		if(am32)block16_32error();
	}
	if(buf==NULL&&strc->bufstr==NULL)return;
	ExpandRm(ctok->rm,ctok->sib,&zoom0,&base0,&idx0);
	if(base0==-1&&idx0!=-1&&zoom0==0){
		base0=idx0;
		idx0=-1;
	}
	CheckReg(idx0,base0,&reg1,&reg2,razr);
//	printf("reg1=%d reg2=%d idx0=%d base0=%d zoom0=%d\n",reg1,reg2,idx0,base0,zoom0);
	if(buf!=NULL){
		if(*buf=='&'){
			sizeel=1;
			pv=TRUE;
			ctok->flag&=~f_reloc;
		}
		if((newreg=CheckIDXReg(buf,sizeel,reg1))!=NOINREG){
			if(newreg!=SKIPREG){
				if(razr==r16){
					if(newreg!=BX&&newreg!=DI&&newreg!=SI&&newreg!=BP)goto noopt;
					if(base0!=-1&&(newreg==BX||newreg==BP))goto noopt;
					if(idx0!=-1&&(newreg==DI||newreg==SI))goto noopt;
				}
				waralreadinitreg(regs[am32][reg1],regs[am32][newreg]);
				reg1=newreg;
			}
			free(buf);
			buf=NULL;
		}
		else{
			if(razr==r32){
				if((zoom1=CheckZoom(sizeel))){
					sizeel=1;
					if((newreg=CheckIDXReg(buf,sizeel,reg1))!=NOINREG){
						if(newreg!=SKIPREG){
							waralreadinitreg(regs[1][reg1],regs[1][newreg]);
							reg2=reg1;
							reg1=newreg;
						}
						free(buf);
						buf=NULL;
						goto cont1;
					}
				}
			}
noopt:
			if(razr==r32&&idx0==-1&&base0==-1&&zoom1==0){
ITOK wtok;
				wtok=*ctok;
				newreg=CheckMassiv(buf,sizeel,reg1,&idx0,&base0,&wtok.number);
				*ctok=wtok;
//				newreg=CheckMassiv(buf,sizeel,reg1);
			}
			else newreg=CheckMassiv(buf,sizeel,reg1);
			if(newreg!=-1){
				reg2=reg1;
				reg1=newreg;
			}
		}
	}
	else{
		reg2=reg1;
		reg1=-1;
	}
cont1:
//	printf("reg1=%d reg2=%d idx0=%d base0=%d zoom0=%d\n",reg1,reg2,idx0,base0,zoom0);
	if(strc->bufstr!=NULL){
		sizeel=strc->size;
		if((newreg=CheckIDXReg(strc->bufstr,sizeel,reg2))!=NOINREG){
			if(newreg!=SKIPREG){
				if(razr==r16){
					if(newreg!=BX&&newreg!=DI&&newreg!=SI&&newreg!=BP)goto noopt2;
					if(base0!=-1&&(newreg==BX||newreg==BP))goto noopt2;
					if(idx0!=-1&&(newreg==DI||newreg==SI))goto noopt2;
				}
				waralreadinitreg(regs[am32][reg2],regs[am32][newreg]);
				reg2=newreg;
			}
			free(strc->bufstr);
			strc->bufstr=NULL;
		}
		else{
			if(razr==r32&&zoom1==0){
				if((zoom2=CheckZoom(sizeel))){
					sizeel=1;
					if((newreg=CheckIDXReg(strc->bufstr,sizeel,reg2))!=NOINREG){
						if(newreg!=SKIPREG){
							waralreadinitreg(regs[2][reg1],regs[2][newreg]);
							reg2=newreg;
						}
						free(strc->bufstr);
						strc->bufstr=NULL;
						goto cont2;
					}
				}
			}
noopt2:
			if((newreg=CheckMassiv(strc->bufstr,sizeel,reg2))!=-1)reg2=newreg;
		}
cont2:
		if(reg1==-1){
			reg1=reg2;
			zoom1=zoom2;
			reg2=-1;
			zoom2=0;
		}
	}
	else reg2=-1;
//	printf("reg1=%d reg2=%d idx0=%d base0=%d zoom0=%d\n",reg1,reg2,idx0,base0,zoom0);
	if(base0==-1){
		if(reg1!=-1&&zoom1==0){
			base0=reg1;
			reg1=reg2;
			zoom1=zoom2;
			reg2=-1;
			zoom2=0;
		}
		else if(reg2!=-1&&zoom2==0){
			base0=reg2;
			reg2=-1;
		}
	}
	if(idx0==-1){
		if(zoom1){
			zoom0=zoom1;
			zoom1=0;//zoom2;
			idx0=reg1;
			reg1=reg2;
			reg2=-1;
		}
		else{
			if(reg1!=-1){
				idx0=reg1;
				reg1=reg2;
				zoom1=zoom2;
				reg2=-1;
				zoom2=0;
			}
		}
	}
//	printf("reg1=%d reg2=%d idx0=%d base0=%d zoom0=%d\n",reg1,reg2,idx0,base0,zoom0);
	if(reg2!=-1){
		if(zoom1==0&&zoom2==0){
			op(3);
			op(0xC0+reg1*8+reg2);	//add reg1,reg2
		}
		else{
			if(zoom1==0){
				op(0x8d);	//lea reg1,[reg1+reg2*zoom2]
				op(reg1*8+4);
				op((zoom2<<6)+(reg2<<3)+reg1);
			}
			else{
				op(0x8d);	//lea reg1,[reg2+reg1*zoom1]
				op(reg1*8+4);
				op((zoom1<<6)+(reg1<<3)+reg2);
				zoom1=0;
			}
		}
		ClearReg(reg1);
	}
	if(reg1!=-1){
		if(base0!=-1){
			if(zoom1==0){
				op(3);
				op(0xC0+reg1*8+base0);	//add reg1,base0
			}
			else{
				op(0x8d);	//lea reg1,[base0+reg1*zoom1]
				op(reg1*8+4);
				op((zoom1<<6)+(reg1<<3)+base0);
			}
		}
		else if(zoom1){
			op(0xC1);
			op(0xE0+reg1);	//shl reg2,zoom2
			op(zoom1);
		}
		base0=reg1;
	}
	if(razr==r32){
		ctok->sib=CODE32;
		if(idx0!=-1){
			ctok->rm=4;
			if(base0!=-1){
				ctok->sib=(zoom0<<6)+(idx0<<3)+base0;
				ctok->rm|=rm_mod10;
			}
			else ctok->sib=(zoom0<<6)+(idx0<<3)+5;
		}
		else{
			ctok->rm=base0;
			if(pv==FALSE)ctok->rm|=rm_mod10;
		}
	}
	else{
		if(base0==SI||base0==DI){
			if(idx0==BX||idx0==BP||idx0==-1){
				newreg=idx0;
				idx0=base0;
				base0=newreg;
			}
			else{
				op(3);
				op(0xC0+base0*8+idx0);	//add base0,idx0
				idx0=base0;
				base0=-1;
			}
		}
		if(idx0==BX||idx0==BP){
			if(base0==-1){
				newreg=idx0;
				idx0=base0;
				base0=newreg;
			}
			else{
				op(3);
				op(0xC0+base0*8+idx0);	//add base0,idx0
				idx0=base0;
				base0=-1;
			}
		}
		ctok->rm=CalcRm16(base0,idx0)|rm_mod10;
	}
	if(ctok->post==0&&(ctok->flag&f_reloc)==0)compressoffset(ctok);
}

int CheckMassiv(char *&buf,int sizeel,int treg,int *idx,int *base,long *num)
{
ITOK oitok;
SINFO ostr;
unsigned char *oldinput;
unsigned int oldinptr,oldendinptr;
unsigned char bcha;
int otok;
char *ostring;
char *ofsst;
int retcode=-1;

//07.09.04 22:45
COM_MOD *ocurmod=cur_mod;
	cur_mod=NULL;
/////////////////
	ostring=BackString((char *)string);
	ofsst=BackString(buf);
	free(buf);
	buf=NULL;
	oldinput=input;	//сохр некотор переменые
	oldinptr=inptr2;
	bcha=cha2;
	oldendinptr=endinptr;
	otok=tok;
	oitok=itok;
	ostr=strinf;
	strinf.bufstr=NULL;

	if(idx&&*idx==-1&&*base==-1){
		char *tbuf;
		int i;
		int idx0,base0;
		long numr;
		int operand;
		int startptr,skipvar;
		int firsttok;
		i=0;
		idx0=base0=-1;
		numr=0;
		if(*ofsst=='&')i++;
		tbuf=BackString(ofsst+i);
//		puts(tbuf);
		input=(unsigned char *)tbuf;
		inptr2=1;
		cha2=input[0];
		endinptr=strlen((char *)input);
		operand=tk_plus;
		startptr=0;
		firsttok=tk_eof;
		do{
			skipvar=FALSE;
			getoperand();
			if(operand==tk_plus){
				if(tok==tk_reg32){
					if(itok.number!=treg){
						skipvar=TRUE;
						if(idx0==-1)idx0=itok.number;
						else if(base0==-1)base0=itok.number;
						else skipvar=FALSE;
					}
					else if(firsttok==tk_eof)firsttok=tk_reg32;
				}
				else if(tok==tk_number){
					numr+=itok.number;
					skipvar=TRUE;
				}
				else if(firsttok==tk_eof)firsttok=tok;
			}
			if(operand==tk_minus&&tok==tk_number){
				numr-=itok.number;
				skipvar=TRUE;
			}
			if(skipvar){
				for(;startptr<inptr2-1;startptr++)input[startptr]=' ';
			}
			startptr=inptr2-1;
			nexttok();
			if(tok==tk_plus||tok==tk_minus){
				if(firsttok==tk_eof){
					if(tok==tk_minus)break;
					for(;startptr<inptr2-1;startptr++)input[startptr]=' ';
					startptr=inptr2-1;
					firsttok=tok;
				}
				operand=tok;
			}
			else break;
		}while(1);
//		printf("%s (%u) >%s\n",(startfileinfo+currentfileinfo)->filename,linenumber,tbuf);
		if(tok==tk_semicolon/*&&(idx0!=-1||base0!=-1)*/){
			*idx=idx0;
			*base=base0;
			*num+=numr;
			free(ofsst);
			ofsst=tbuf;
//			printf("idx0=%d base0=%d num=%d\n",idx0,base0,numr);
		}
		else free(tbuf);
	}

	input=(unsigned char *)ofsst;
	inptr2=1;
	cha2=input[0];
	if(cha2=='&'){
		inptr2=2;
		cha2=input[1];
	}
	endinptr=strlen((char *)input);
	getoperand();
	if(am32&&sizeel==1&&tok==tk_reg32&&itok2.type==tp_stopper)retcode=itok.number;
	else{
		warningreg(regs[am32][treg]);
		if((chip==7||chip==8)&&am32==FALSE&&((sizeel>1&&sizeel<6)||sizeel==8||sizeel==9)){
		//избежать обращение к частному регистру
			op(0x31);
			op(0xC0+treg*9);
		}
		ofsst=NULL;
		if(treg==AX)do_e_axmath(0,(am32+1)*2,&ofsst);
		else getintoreg(treg,(am32+1)*2,0,&ofsst);
		IDXToReg((char *)input,sizeel,treg);
	}
	strinf=ostr;
	tok=otok;
	itok=oitok;
	endoffile=0;
	free(input);
	input=oldinput;
	inptr2=oldinptr;
	cur_mod=ocurmod;	//07.09.04 22:45
	cha2=bcha;
	endinptr=oldendinptr;
	otok=0;
	strcpy((char *)string,ostring);
	free(ostring);
	if(sizeel>1)RegMulNum(treg,sizeel,(am32+1)*2,0,&otok,0);
	return retcode;
}

void nextchar()
{
	if(inptr<endinptr){
		cha=input[inptr];
		endoffile=0;
		if(savestring3)string3[posstr3++]=cha;
		inptr++;
		precha=0;
		if(inptr==endinptr&&cur_mod&&displaytokerrors){
			precha=cur_mod->input[cur_mod->inptr];
//			printf("cha=%02X\n",cha);
		}
	}
	else{
		if(cur_mod){
			if(displaytokerrors){
				inptr=cur_mod->inptr;
				input=cur_mod->input;
//				printf("last cha=%02X\n",input[inptr]);

				if(cur_mod->numparamdef)clearregstat();

				endinptr=cur_mod->endinptr;
				linenumber=cur_mod->linenumber;
				currentfileinfo=cur_mod->currentfileinfo;
				COM_MOD *temp=cur_mod;
				cur_mod=cur_mod->next;
				if(temp->freze==FALSE){
					if(temp->paramdef)free(temp->paramdef);
//					if(debug)printf("Free curmod %08X new cur_mod %08X\n",temp,cur_mod);
					free(temp);
				}
//				else if(debug)printf("Freze curmod %08X\n",temp);
				nextchar();
//				printf("%c\n",cha);
				if(!cur_mod)notdef=TRUE;
			}
			else{
				cha=cur_mod->input[cur_mod->inptr2];
				cur_mod->inptr2++;
			}
		}
		else{
			cha=26;
			endoffile++;
			if(endoffile>2)unexpectedeof();
		}
	}
}

void ScanTok2()
{
	inptr2=inptr;
	linenum2=linenumber;
	cha2=cha;
	displaytokerrors=0;
	tokscan(&tok2,&itok2,string2);
}

unsigned int ScanTok3()
{
unsigned int oinptr,oline,otok,rtok;
unsigned char ocha;
ITOK oitok;
	ocha=cha;
	oinptr=inptr;
	oline=linenumber;
	otok=tok;
	oitok=itok;
	do{
		FastTok(1);
		if(tok==tk_id||tok==tk_ID){
			char disp=displaytokerrors;
			displaytokerrors=0;
			strcpy((char *)string3,itok.name);
			searchtree(&itok,&tok,string3);	//NEW 09.06.06 20:29
//			searchtree2(definestart,&itok,&tok,string3);
			displaytokerrors=disp;
		}
	}while(tok==tk_endline);
	rtok=tok;
	tok=otok;
	cha=ocha;
	itok=oitok;
	inptr=oinptr;
	linenumber=oline;
	return rtok;
}

//extern idrec *crec;

void nexttok()
{
#ifdef DEBUGMODE
if(debug)puts("start nexttok");
#endif
	inptr=inptr2;
	linenumber=linenum2;
	cha=cha2;
	displaytokerrors=1;
	tokscan(&tok,&itok,string); //разбор команды
//	printf("input=%08X inptr=%08X tok=%d %s\n",input,inptr,tok,itok.name);
	if(tok==tk_dblcolon&&numblocks){
		skiplocals=TRUE;
		tokscan(&tok,&itok,string); //разбор команды
	}
	ScanTok2();
	if(tok2==tk_dblcolon&&numblocks){
		skiplocals=TRUE;
		tokscan(&tok2,&itok2,string2);
	}
	if(tok2==tk_tilda){
		if(ScanTok3()==tk_number){
			tok2=tk_number;
//			puts("tok2=tk_tilda tok3=tk_number");
		}
	}
	linenumber=linenum2;
	// new !!! 02.04.02 20:22
	if(tok==tk_int&&idasm==TRUE&&tok2==tk_number){
		tok=tk_idasm;
		itok.rm=a_int;
	}
	// new !!! 02.04.02 20:22

#ifdef DEBUGMODE
	if(debug)printdebuginfo();
#endif
	if(tok2==tk_number){
		if(tok==tk_not){
			nexttok();
			if(itok.rm!=tk_float&&itok.rm!=tk_double){
				if(scanlexmode==RESLEX){
					if(itok.number)itok.number=0;
					else itok.number=1;
				}
				else itok.lnumber^=-1;
			}
			else illegalfloat();
			return;
		}
		if(tok==tk_tilda){
//			puts("tok=tk_tilda tok2=tk_number");
			nexttok();
			if(itok.rm!=tk_float&&itok.rm!=tk_double)itok.lnumber=~itok.lnumber;
			else illegalfloat();
			return;
		}
	}
	if(idasm==TRUE&&tok==tk_loop&&tok2!=tk_openbracket){
		tok=tk_idasm;
		itok.rm=a_loop;
		return;
	}
	if(tok==tk_dollar){
		if(itok2.type==tp_opperand||itok2.type==tp_stopper||tok2==tk_dollar){
			tok=tk_number;
			itok.number=outptr;
			if(FixUp)itok.flag|=f_reloc;	//new!!!
			return;
		}

		int qq;	//09.07.08 13:27 new
		if((qq=CheckAsmName(itok2.name))!=-1){
			tok=tk_idasm;
			itok.rm=qq;
			strcpy(itok.name,itok2.name);
			ScanTok2();
		}

	}
	if(tok>=tk_charvar&&tok<=tk_doublevar){
localrec *ptr;
		ptr=itok.locrec;
		switch(itok.type){
			case tp_paramvar:
//			case tp_ppointer:
//			case tp_pfpointer:
				if(ptr->fuse==NOTINITVAR&&tok2==tk_assign&&(!asmparam))warningnotused(itok.name,4);
				goto llq;
			case tp_localvar:
//			case tp_lpointer:
//			case tp_lfpointer:
				if(ptr->fuse==NOTINITVAR&&tok2!=tk_assign){
					if(asmparam)ptr->fuse|=INITVAR;
					else warningusenotintvar(itok.name);
				}
llq:
				ptr->fuse|=(unsigned char)(tok2==tk_assign?INITVAR:USEDVAR);
				break;
		}
#ifdef OPTVARCONST
		if(calcnumber&&CheckConstVar(&itok))tok=tk_number;
#endif
	}
#ifdef OPTVARCONST
	if(tok2>=tk_charvar&&tok2<=tk_doublevar){
		if(calcnumber&&CheckConstVar(&itok2))tok2=tk_number;
	}
#endif
	if(usedirectiv==FALSE&&calcnumber==FALSE)while(tok==tk_question)directive();
//16.08.04 00:23
	if(tok==tk_at&&tok2==tk_macro){
		nexttok();
	}
#ifdef OPTVARCONST
	displaytokerrors=1;
#endif

//	if(crec)printf("num=%08X\n",crec->recnumber);

}

void whitespace() //пропуск нзначащих символов
{
	while(isspace(cha)||cha==255||cha==0){
		if(cha==13){
			linenumber++;
			if((dbg&2)&&displaytokerrors&&notdef)startline=(char*)(input+inptr+1);
			if(scanlexmode==RESLEX||scanlexmode==DEFLEX||scanlexmode==ASMLEX)break;
		}
		nextchar();
	}
}

unsigned char convert_char()
//Returns the value of the current character.  Parses \n and \x00 etc.
//cha equals the last character used.
{
int hold=0;
int i;
unsigned char c;
	if(cha!='\\'){
		if(cha==13){
			linenumber++;
			if((dbg&2)&&displaytokerrors&&notdef)startline=(char*)(input+inptr+1);
		}
		return(cha);
	}
	nextchar();  // move past '\\'
	switch(cha){
		case 'a': return('\a'); 	// what is \a anyway?
		case 'b': return('\b');
		case 'f': return('\f');
		case 'l': return(10);
		case 'n': return(13);
//		case 'p': return('Ь');
		case 'r': return(13);
		case 't': return('\t');
		case 'v': return('\v');
		case 'x':
			for(i=0;i<2;i++){
				c=cha;
				nextchar();
				hold*=16;
				if(isdigit(cha))hold+=cha-'0';
				else if(isxdigit(cha))hold+=(cha&0x5f)-'7';
				else{
//					expectederror("hexadecimal digit");
					if(savestring3)posstr3--;
					inptr--;
					cha=c;
					break;
				}
			}
			return (unsigned char)hold;
		default:
			if(isdigit(cha)){
				hold=cha-'0';
				for(i=0;i<2;i++){
					c=cha;
					nextchar();
					if(isdigit(cha))hold=hold*10+(cha-'0');
					else{
//						expectederror("decimal digit");
						if(savestring3)posstr3--;
						inptr--;
						cha=c;
						break;
					}
				}
				return (unsigned char)hold;
			}
			return cha;
	}
}

void convert_string(unsigned char *str)
{
int k,j,i;
unsigned char c;
unsigned char hold;
	for(k=0,j=0;;k++,j++){
		c=str[k];
		if(c=='\\'){
			c=str[++k];
			switch(c){
				case 'a': hold='\a'; break; 	// what is \a anyway?
				case 'b': hold='\b'; break;
				case 'f': hold='\f'; break;
				case 'l': hold=10; break;
				case 'n': str[j++]=13; hold=10; break;
				case 'r': hold=13; break;
				case 't': hold='\t'; break;
				case 'v': hold='\v'; break;
				case 'x':
					hold=0;
					for(i=0;i<2;i++){
						c=str[++k];
						hold*=(unsigned char)16;
						if(isdigit(c))hold+=(unsigned char)(c-'0');
						else if(isxdigit(c))hold+=(unsigned char)((c&0x5f)-'7');
						else{
							k--;
							break;
						}
					}
					break;
				default:
					hold=c;
					if(isdigit(c)){
						hold-='0';
						for(i=0;i<2;i++){
							c=str[++k];
							if(isdigit(c))hold=(unsigned char)(hold*10+(c-'0'));
							else{
								k--;
								break;
							}
						}
					}
			}
			str[j]=hold;
		}
		else str[j]=c;
		if(c==0)break;
	}
}

int CheckChar2()
{
	if(isalnum(cha)||cha=='_'||cha>=0x80)return TRUE;
	return FALSE;
}

void GetTokString(int *tok4,ITOK *itok4,unsigned char *string4,int useunicode)
{
int strptr=0;
	if(displaytokerrors){
		savestring3=TRUE;
		posstr3=0;
	}
nextstr:
	nextchar();	//строковая константа
	while(cha!='\"'&&!endoffile&&strptr<STRLEN-1){
		string4[strptr++]=convert_char();
		if((char)cha=='n'&&string4[strptr-1]==13){//have to add char 10 for \n value
			string4[strptr++]=10;
		}
		nextchar();
	}
	if(displaytokerrors){
		savestring3=FALSE;
		string3[posstr3-1]=0;
	}
	if(strptr>=(STRLEN-1)&&displaytokerrors)preerror("Maximum String Length Exceeded");
	string4[strptr]=0;
	*tok4=tk_string;
//	itok4->number=strptr;
	itok4->rm=1;	//есть оригинал строки
	if(cha!='\"')expected('\"');
	while(cha!='\"'&&!endoffile)nextchar(); //scan until closing '\"'
	nextchar();
	if(dosstring)itok4->flag=dos_term;
	CheckConvertString((char *)string4);
	switch(cha){
		case 'z':
			itok4->flag=zero_term;
			nextchar();
			break;
		case 'n':
			itok4->flag=no_term;
			nextchar();
			break;
		case '$':
			itok4->flag=dos_term;
			nextchar();
			break;
	}
	CheckConvertString((char *)string4);

//	10.08.04 22:20
	whitespace(); //пропуск незначащих символов
	if(cha=='\"'){
		if(displaytokerrors)savestring3=TRUE;
		goto nextstr;
	}

	if(useunicode&&displaytokerrors){
		char *bak;
		bak=BackString((char *)string4);
		strptr=MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,bak,-1,(wchar_t *)string4,STRLEN)-1;
//		printf("size string=%d\n",strptr);
//		if(itok4->flag==no_term)strptr--;
		strptr*=2;
//		string4[strptr-2]=string4[strptr-1]=0;
		itok4->flag|=s_unicod;
		free(bak);
	}
	itok4->number=strptr;
}

void tokscan(int *tok4,ITOK *itok4,unsigned char *string4)
// поиск идентификаторов, директив ...
{
int useme;
unsigned int strptr=0;
char uppercase=1,next=1;
//установки по умолчанию
#ifdef DEBUGMODE
if(debug)printf("start tokscan input=%08X inptr=%08X %c%s\n",input,inptr,cha,input+inptr);
#endif
	if(am32==FALSE){
		itok4->rm=rm_d16;
		itok4->sib=CODE16;
	}
	else{
		itok4->rm=rm_d32;
		itok4->sib=CODE32;
	}
	itok4->segm=DS;
	itok4->lnumber=0;
	itok4->post=0;
	itok4->flag=0;
	itok4->size=0;
	itok4->rec=NULL;
	itok4->type=tp_ucnovn;
	itok4->npointr=0;
	itok4->name[0]=0;
	whitespace(); //пропуск незначащих символов
//	if(displaytokerrors)printf("%c ",cha);
	if(isalpha(cha)||(cha=='_')||(cha>=0x80)){	//идентификатор
		do{
			string4[strptr++]=cha;
			if(islower(cha))uppercase=0;
			nextchar();
		}while((strptr<IDLENGTH)&&(CheckChar2()==TRUE));
		if(strptr>=IDLENGTH){ //длина больше 32
			if(displaytokerrors)preerror("Maximum length for an identifier exceeded");
			while(CheckChar2()==TRUE)nextchar();	//дочитать слово
			strptr=IDLENGTH-1;	//обрезать до 32
		}
		if(cha=='~'&&strptr<IDLENGTH-1){
			string4[strptr++]=cha;
			nextchar();
			if(cha!='('){
				inptr--;
				strptr--;
				cha='~';
			}
		}
		string4[strptr]=0;
		strcpy(itok4->name,(char *)string4);
//		if(displaytokerrors)printf("%s (%u) >%s\n",(startfileinfo+currentfileinfo)->filename,linenumber,itok4->name);
#ifdef DEBUGMODE
		if (debug)printf("ID: '%s' cur_mod=%08X prev cur_mod=%08X\n",itok4->name,cur_mod,cur_mod==NULL?0:cur_mod->next);
#endif
		if(scanlexmode==RESLEX){
			if(stricmp((char *)string4,"not")==0)*tok4=tk_not;
			else if(stricmp((char *)string4,"or")==0)*tok4=tk_or;
			else if(stricmp((char *)string4,"xor")==0)*tok4=tk_xor;
			else if(stricmp((char *)string4,"and")==0)*tok4=tk_and;
			else{
				if(uppercase)*tok4=tk_ID;
				else *tok4=tk_id;
				searchtree(itok4,tok4,string4);
				if(*tok4==tk_id||*tok4==tk_ID){
					if((useme=CheckResName(itok4->name))!=-1){
						*tok4=tk_rescommand;
						itok4->number=useme;
					}
				}
			}
			return;
		}
		if(uppercase){	//верхний регистр
			if(strptr==1&&string4[0]=='L'&&cha=='"'){
				itok4->name[0]=0;
				GetTokString(tok4,itok4,string4,TRUE);
				return;
			}
			*tok4=tk_ID;
			if(string4[1]=='S'&&strptr>=5&&strptr<=8){
				for(useme=0;useme<ID2S;useme++){	//проверка на ESBYTE ...
					if(strcmp((char *)string4,id2[useme])==0){
						*tok4=tk_charvar+useme%DATATYPES;
						itok4->segm=useme/DATATYPES;
						itok4->number=0;
						itok4->post=POINTER;
						goto yesid;
					}
				}
			}
		}
		else *tok4=tk_id;
		if(strptr==2){	//длина 2 символа check for AX, CX, DX, ...
			if((string4[0]&0x5f)=='S'&&(string4[1]&0x5f)=='T'){
				if(cha=='('){
					nextchar();
					if(cha>='0'&&cha<='7'){
						itok4->number=cha-'0';
						nextchar();
						if(cha!=')')expected(')');
						nextchar();
						*tok4=tk_fpust;
					}
				}
				else{
					itok4->number=0;
					*tok4=tk_fpust;
				}
				return;
			}
			if(asmparam||*tok4==tk_ID){
				for(useme=0;useme<8;useme++){
					int i;
					if(asmparam)i=stricmp((char *)string4,regs[0][useme]);
					else i=strcmp((char *)string4,regs[0][useme]);
					if(i==0){
						*tok4=tk_reg;
						itok4->number=useme;
extreg:
						if(cha=='.'){
							tag_massiv(tok4,itok4,string4);
							goto yesid;
						}
						return;
					}
					if(asmparam)i=stricmp((char *)string4,begs[useme]);
					else i=strcmp((char *)string4,begs[useme]);
					if(i==0){
						*tok4=tk_beg;
						itok4->number=useme;
						return;
					}
				}
				if(string4[1]=='S'||(asmparam&&string4[1]=='s')){
					for(useme=0;useme<6;useme++){ // check for CS, SS, ...
						if(string4[0]==segs[useme][0]||(asmparam&&(string4[0]&0x5F)==segs[useme][0])){
							if(cha==':'){
								int oinptr=inptr;
								unsigned char ocha=cha;
								int oline=linenumber;
								nextchar();
/*								switch(cha&(asmparam==FALSE?0xFF:0x5F)){
									case 'E':
									case 'B':
									case 'S':
									case 'D':*/
										tokscan(tok4,itok4,string4);
										if(*tok4>=tk_bits&&*tok4<=tk_doublevar){
											itok4->segm=useme;
											goto yesid;
										}
//								}
								inptr=oinptr;
								cha=ocha;
								linenumber=oline;
							}
							*tok4=tk_seg;
							itok4->number=useme;
							return;
						}
					}
				}
			}
		}
		if(strptr==3&&(string4[0]=='E'||(asmparam&&string4[0]=='e'))){// check for EAX, ECX, EDX, ...
			for(useme=0;useme<8;useme++){
				int i;
				if(asmparam)i=stricmp((char *)&string4[1],regs[0][useme]);
				else i=strcmp((char *)&string4[1],regs[0][useme]);
				if(i==0){
					*tok4=tk_reg32;
					itok4->number=useme;
extreg32:
					if(cha=='.'){
						tag_massiv(tok4,itok4,string4);
						goto yesid;
					}
					if(cha==':'){
						int oinptr=inptr;
						unsigned char ocha=cha;
						int oline=linenumber;
						nextchar();
						tokscan(tok4,itok4,string4);
						if(*tok4==tk_reg32||*tok4==tk_reg){
							itok4->number|=useme*256;
							*tok4=tk_reg64;
							goto yesid;
						}
						inptr=oinptr;
						cha=ocha;
						linenumber=oline;
					}
					return;
				}
			}
		}
		if(asmparam){
			if((string4[1]&0x5f)=='R'){
				if(string4[2]>='0'&&string4[2]<='7'){
					itok4->number=string4[2]-'0';
					switch((string4[0]&0x5f)){
						case 'C': *tok4=tk_controlreg; return;// check for CR0, CR1, ...
						case 'D': *tok4=tk_debugreg; return;//check for DR0, DR1, ...
						case 'T': *tok4=tk_testreg; return;//check for TR0, TR1, ...
						default: itok4->number=0; break;
					}
				}
			}
			else if((*(short *)&string[0]&0x5f5f)==0x4d4d){
				if(string4[2]>='0'&&string4[2]<='7'){
					itok4->number=string4[2]-'0';
					*tok4=tk_mmxreg;
					return;
				}
			}
			else if((*(short *)&string[1]&0x5f5f)==0x4d4d&&(string4[0]&0x5f)=='X'){
				if(string4[3]>='0'&&string4[3]<='7'){
					itok4->number=string4[3]-'0';
					*tok4=tk_xmmreg;
					return;
				}
			}
		}
		if(useasm==FALSE){
			if((useme=FastSearch(id,idofs,2,(char *)string4))!=-1)*tok4=useme;
			else if(idasm==TRUE&&(useme=CheckAsmName((char *)string4))!=-1){
				*tok4=tk_idasm;
				itok4->rm=useme;
				return;
			}
		}
		if(*tok4==tk_id||*tok4==tk_ID){
			if(cur_mod&&cur_mod->numparamdef&&(displaytokerrors||scanalltoks)){//18.03.05 15:12
/*				if(debug&&cur_mod){
					printf("Find %s in %d entries curmod=%08X\n",itok4->name,cur_mod->numparamdef,cur_mod);
					COM_MOD *nmod;
					nmod=cur_mod;
					while(nmod){
						printf("\tinput=%08X inptr=%08X\n",nmod->input,nmod->inptr);
						nmod=nmod->next;
					}
				}*/
				if(CheckDefPar(itok4->name)){
					tokscan(tok4,itok4,string4);
					if(displaytokerrors==0){
						inptr=cur_mod->inptr;
						input=cur_mod->input;
						endinptr=cur_mod->endinptr;
						linenumber=cur_mod->linenumber;
						currentfileinfo=cur_mod->currentfileinfo;
						COM_MOD *temp=cur_mod;
						cur_mod=cur_mod->next;
						free(temp);
						nextchar();
					}
#ifdef DEBUGMODE
					if(debug)printf("1601 tok=%d input=%08X inptr=%08X disp=%d %s %c%40s\n\t%s\n",*tok4,input,inptr,displaytokerrors,itok4->name,cha,input+inptr,""/*input*/);
#endif
					return;
				}
			}
			if(searchlocals(itok4,tok4,string4)==FALSE){//поиск среди локальных меток
			//если ничего не найдено поиск в дереве переменых
				searchtree(itok4,tok4,string4);
				if(*tok4==tk_endline){
					if(scanlexmode!=DEFLEX){
						tokscan(tok4,itok4,string4);
#ifdef DEBUGMODE
						if(debug)printf("1610 tok=%d input=%08X inptr=%08X disp=%d %s %c%40s\n\t%s\n",*tok4,input,inptr,displaytokerrors,itok4->name,cha,input+inptr,""/*input*/);
#endif
						return;
					}
					*tok4=tk_macro;
				}
				if(*tok4==tk_number||*tok4==tk_idasm)return;
				if((*tok4>=tk_bits&&*tok4<=tk_doublevar)||*tok4==tk_pointer){
					if(itok4->rm==rm_d16||itok4->rm==rm_d32){
						if(am32==FALSE){
							itok4->rm=rm_d16;
							itok4->sib=CODE16;
						}
						else{
							itok4->rm=rm_d32;
							itok4->sib=CODE32;
						}
					}
					if(itok4->post==DYNAMIC_POST){	//преобразовать динамическую локальную в локальную
						if(alignword&&*tok4!=tk_charvar&&*tok4!=tk_bytevar){	//выровнять на четный адрес
							switch(*tok4){
								case tk_intvar:
								case tk_wordvar:
									if(postsize%2==1)postsize++;
									break;
								default:
									if(postsize%4!=0)postsize+=4-(postsize%4);
									break;
							}
						}
						idrec *ptr=itok4->rec;
						itok4->post=ptr->recpost=1;
						itok4->number=ptr->recnumber=postsize;
						if(am32==FALSE&&(postsize+itok4->size)>0xFFFFL)tobigpost();
						postsize+=itok4->size;
					}
					else if(itok4->post==DYNAMIC_VAR){
						idrec *ptr=itok4->rec;
						itok4->post=ptr->recpost=USED_DIN_VAR;
					}
				}
			}
			if(tok==tk_reg)goto extreg;
			if(tok==tk_reg32)goto extreg32;
		}
		switch(*tok4){
			int len;
			case tk_dataptr: *tok4=tk_number; itok4->number=outptrdata;
				if(FixUp)itok.flag|=f_reloc;	//new!!!
				break;
			case tk_codeptr: *tok4=tk_number; itok4->number=outptr;
				if(FixUp)itok.flag|=f_reloc;	//new!!!
				break;
			case tk_postptr: *tok4=tk_number; itok4->number=postsize;	break;
			case tk_line: *tok4=tk_number; itok4->number=linenumber; break;
			case tk_file:
				*tok4=tk_string;
				strcpy((char *)string4,(startfileinfo+currentfileinfo)->filename);
				itok4->number=strlen((char *)string4);
				break;
			case tk_structvar:
				struct idrec *ptrs;
				ptrs=itok4->rec;
//				puts(itok4->name);
				dostructvar2(tok4,itok4,(structteg *)ptrs->newid,string4);
				break;
			case tk_sizeof:
				dosizeof(itok4);
				if(itok4->post)*tok4=tk_postnumber;
				else *tok4=tk_number;
				break;
			case tk_signed:
			case tk_unsigned:
			case tk_int:
			case tk_long:
			case tk_short:
				if(itok4->type!=tp_modif){
					GetTypeVar(tok4);
					itok4->type=tp_modif;
				}
				break;
			case tk_ID:
//				printf("%s %s %d %c\n",itok.name,string4,strptr,cha);
				if(strlen(itok4->name)==1&&itok4->name[0]=='L'&&cha=='"'){
					itok4->name[0]=0;
					GetTokString(tok4,itok4,string4,TRUE);
					return;
				}
			case tk_id:
				if(findofset==FALSE){
					struct structteg *tteg;
//					if(displaytokerrors)printf("find teg %s cha=%02X\n",itok4->name,cha);
					if(((tteg=FindTeg(FALSE,itok4->name))!=NULL||
							(tteg=FindTeg(TRUE,itok4->name))!=NULL)&&(cha=='.'||precha=='.')){
//						if(displaytokerrors)puts("finding");
						dostructvar2(tok4,itok4,tteg,string4);
//						printf("tok=%d\n",*tok4);
						if(displaytokerrors&&(*tok4==tk_proc||*tok4==tk_declare||
								*tok4==tk_undefproc)&&(itok4->flag&f_static)==0)unknownobj(itok4->name);
					}
				}
				break;

		}
		if(*tok4==tk_string&&displaytokerrors){
			strcpy((char *)string3,(char *)string4);
			if(itok4->rm==2){
				convert_string(string4);
				itok4->rm--;
			}
			if(dosstring)itok.flag=dos_term;
		}
yesid:
		if((*tok4>=tk_bits&&*tok4<=tk_doublevar)||*tok4==tk_pointer){
			whitespace();//пропуск незначащих символов
			if(cha=='[')calcrm(itok4,*tok4);//обработка выражения в []
		}
		if(itok4->rm!=rm_d16&&*tok4!=tk_proc&&*tok4!=tk_undefproc&&
			*tok4!=tk_apiproc&&*tok4!=tk_declare&&(!(*tok4==tk_pointer&&itok4->type==tk_proc)))
			if(itok4->post==0&&(itok4->flag&f_reloc)==0){  // cannot compress if POST var
				compressoffset(itok4);
			}
		next=0;
	}
	else if(isdigit(cha)){//числа
		inptr--;
		itok4->lnumber=scannumber(&itok4->rm);
		*tok4=tk_number;
		next=0;
	}
	else switch(cha){
		case '\"':
			GetTokString(tok4,itok4,string4,FALSE);
			next=0;
			break;
		case '\'': //символьная константа может иметь более 1 символа
			nextchar();
			next=0;
			if(scanlexmode==RESLEX){
				*tok4=tk_singlquote;
				break;
			}
			*tok4=tk_number;
			itok4->number=0;
			while(cha!='\''&&!endoffile){  // special character
				itok4->lnumber=itok4->lnumber*256+convert_char();
				nextchar();
			}
			if(cha!='\''){
				if(displaytokerrors)expected('\'');
			}
			else nextchar();
			next=0;
			break;
		case '-':
			nextchar();
			switch(cha){
				case '=': *tok4=tk_minusequals; break;	//минус равно
				case '-': *tok4=tk_minusminus; break; 	//минус-минус
				default: *tok4=tk_minus; next = 0; itok4->type=tp_opperand; break;//минус
			}
			break;
		case '+':
			nextchar();
			switch(cha){
				case '=': *tok4=tk_plusequals; break; //плюс равно
				case '+': *tok4=tk_plusplus; break; 	//плюс-плюс
				default: whitespace();	// spaces allowed between
					if(cha=='-')*tok4=tk_minus;  // optimization of + -
					else{
						*tok4=tk_plus; 				 //плюс
						next=0;
					}
					itok4->type=tp_opperand;
					break;
			}
			break;
		case '*':
			nextchar();
			switch(cha){
				case '=': *tok4=tk_multequals; break;
				case '-': *tok4=tk_multminus; break;			 //умножить минус
				default:
					*tok4=tk_mult; 									 //умножить
					next=0;
			}
			itok4->type=tp_opperand;
			break;
		case '/': nextchar();
			switch(cha){
				case '*': nextchar(); //соментарий
					useme=1;
					while(!endoffile&&useme>0){
						whitespace();
						if(cha=='*'){
							nextchar();
							if(cha=='/'){
								nextchar();
								if(cha=='/')nextchar();
								else useme--;
							}
							continue;
						}
						else if(cha=='/'){
							nextchar();
							if(cha=='*')useme++;
							else if(cha=='/'){
								nextchar();
								continue;
							}
							else continue;
						}
						nextchar();
					}
					if(endoffile){
						*tok4=tk_eof;
						if(useme>0&&displaytokerrors)unexpectedeof();
					}
					else tokscan(tok4,itok4,string4);
					break;
				case '/':
					do{
						nextchar();
					}while(!endoffile&&cha!=13);	//строка коментария
					if(endoffile)*tok4=tk_eof;
					else{
						if(scanlexmode==DEFLEX)*tok4=tk_endline;
						else{
							whitespace();
							if(cha==13)nextchar();
							tokscan(tok4,itok4,string4);
						}
					}
					break;
				case '=': *tok4=tk_divequals; nextchar(); return;
				default:
					whitespace();
					if(cha=='-'){
						*tok4=tk_divminus;	//деление
						nextchar();
					}
					else *tok4=tk_div;
					itok4->type=tp_opperand;
					break;
			}
			next=0;
			break;
		case '%':
			nextchar();
			whitespace();
			if(cha=='-')*tok4=tk_modminus;	//остаток от деления
			else{
				*tok4=tk_mod;
				next=0;
			}
			itok4->type=tp_opperand;
			break;
		case '|':
			nextchar();
			switch(cha){
				case '=': *tok4=tk_orequals; break; //OR=
				case '|': *tok4=tk_oror; break; 		//OR-OR
				default:
					whitespace();
					if(cha=='-')*tok4=tk_orminus; 		//OR-
					else{
						*tok4=tk_or;
						if(cha==13)nextchar();					//OR
						next=0;
					}
					itok4->type=tp_opperand;
					break;
			}
			break;
		case '&':
			nextchar();
			switch(cha){
				case '=': *tok4=tk_andequals; break; //AND=
				case '&': *tok4=tk_andand; break;
				default:
					whitespace();
					if(cha=='-')*tok4=tk_andminus;
					else{
						*tok4=tk_and;
						next=0;
					}
					itok4->type=tp_opperand;
					break;
			}
			break;
		case '!':
			nextchar();
			if(cha=='='){
				*tok4=tk_notequal;		//!=
				itok4->type=tp_compare;
			}
			else{
				*tok4=tk_not;
				next=0;
			}
			break;
		case '^':
			nextchar();
			if(cha=='=')*tok4=tk_xorequals;  //XOR=
			else{
				whitespace();
				if(cha=='-')*tok4=tk_xorminus;
				else{
					*tok4=tk_xor;
					next=0;
				}
				itok4->type=tp_opperand;
			}
			break;
		case '=':
			nextchar();
			if(cha=='='){
				*tok4=tk_equalto;  //==
				itok4->type=tp_compare;
			}
			else{
				*tok4=tk_assign;						 //присвоить
				next=0;
			}
			break;
		case '>':
			nextchar();
			switch(cha){
				case '>':
					nextchar();
					if(cha=='=')*tok4=tk_rrequals; //сдвиг вправо с присвоением
					else{
						whitespace();
						if(cha=='-')*tok4 = tk_rrminus;
						else{
							*tok4=tk_rr;							//сдвиг вправо
							next=0;
						}
						itok4->type=tp_opperand;
					}
					break;
				case '<': *tok4=tk_swap; break; 			 //обмен
				case '=': *tok4=tk_greaterequal; itok4->type=tp_compare; break; //больше или равно
				default: *tok4=tk_greater; next=0; itok4->type=tp_compare; break; //больше
			}
			break;
		case '<':
			nextchar();
			switch(cha){
				case '<':
					nextchar();
					if(cha=='=')*tok4=tk_llequals;	 //сдвиг влево с присвоением
					else{
						whitespace();
						if(cha=='-')*tok4=tk_llminus;
						else{
							*tok4=tk_ll;								 //сдвиг влево
							next=0;
						}
						itok4->type=tp_opperand;
					}
					break;
				case '>': *tok4=tk_notequal; itok4->type=tp_compare; break;  //!=
				case '=': *tok4=tk_lessequal; itok4->type=tp_compare; break; //меньше или равно
				default: *tok4=tk_less; next=0; itok4->type=tp_compare; break;//меньше
			}
			break;
		case '#':
			next=0;
			nextchar();
			findofset=TRUE;
			tokscan(tok4,itok4,string4);
			findofset=FALSE;
			if((useme=GetDirective((char *)string4))!=-1){
				itok4->number=useme;
				*tok4=tk_question;
				break;
			}
			if(*tok4==tk_dblcolon){
				skiplocals=TRUE;
				tokscan(tok4,itok4,string4);
			}
/*			char buf[16];
			strncpy(buf,(char *)(input+inptr),15);
			buf[15]=0;*/
//		printf("sib=%08X tok=%d rm=%d post=%d number=%d size=%d segm=%d flag=%08X %s\n",itok4->sib,*tok4,itok4->rm,itok4->post,itok4->number,itok4->size,itok4->segm,itok4->flag,itok4->name/*,buf*/);
//	printf("skiplocals=%d skipfind=%d searchteg=%08X\n",skiplocals,skipfind,searchteg);
			switch(*tok4){
				case tk_structvar:
//					printf("%08X %s\n",itok4->sib,itok4->name);
					if((itok4->sib&1)==0){
						ofsstr(tok4,itok4);
//		if(displaytokerrors)printf("sib=%08X tok=%d rm=%d post=%d number=%d size=%d segm=%d flag=%08X %s\n",itok4->sib,*tok4,itok4->rm,itok4->post,itok4->number,itok4->size,itok4->segm,itok4->flag,itok4->name/*,buf*/);
						break;
					}
					itok4->rm=(am32==0?rm_d16:rm_d32);
				case tk_struct:
					*tok4=tk_number;
					if(bufrm){
						*tok4=tk_rmnumber;
						if(itok4->type==tp_classvar)itok4->flag|=f_useidx;
					}
					break;
				case tk_apiproc:	//!!!16.12.06 23:06
					itok4->rm=tk_undefofs;
					*tok4=tk_apioffset;
					if(FixUp)itok4->flag|=f_reloc;	//new!!! 27.06.05 22:25
					strcpy((char *)string4,itok4->name);
//			printf("sib=%08X tok=%d rm=%d post=%d number=%d size=%d segm=%d %s\n",itok4->sib,*tok4,itok4->rm,itok4->post,itok4->number,itok4->size,itok4->segm,itok4->name/*,buf*/);
					break;
				case tk_proc:
				case tk_interruptproc:
					if(cha=='.')goto structofs;
procofs:
					if(itok4->segm!=NOT_DYNAMIC){
						if(displaytokerrors){
							idrec *ptr=itok4->rec;
							itok4->segm=ptr->recsegm=DYNAMIC_USED;
						}
						itok4->rm=*tok4=tk_undefofs;	//смещение еще не известной метки
						itok4->number=0;
//						if(FixUp)itok4->flag|=f_reloc;	//new!!! 27.06.05 22:25
					}
					else{
						*tok4=tk_number;
						itok4->segm=CS;//(splitdata==FALSE)?DS:CS;
//						if(FixUp)itok4->flag|=f_reloc;
					}
					if(FixUp)itok4->flag|=f_reloc;	//new!!! 27.06.05 22:25
					strcpy((char *)string4,itok4->name);
//			printf("sib=%08X tok=%d rm=%d post=%d number=%d size=%d segm=%d %s\n",itok4->sib,*tok4,itok4->rm,itok4->post,itok4->number,itok4->size,itok4->segm,itok4->name/*,buf*/);
					break;
				case tk_id:
				case tk_ID:
structofs:
					struct structteg *tteg;
					if(((tteg=FindTeg(FALSE,itok4->name))!=NULL||
							(tteg=FindTeg(TRUE,itok4->name))!=NULL)/*&&cha=='.'*/){
						dostructvar2(tok4,itok4,tteg,string4);
//			printf("sib=%08X tok=%d rm=%d post=%d number=%d size=%d %s\n",itok4->sib,*tok4,itok4->rm,itok4->post,itok4->number,itok4->size,itok4->name/*,buf*/);

						if(*tok4==tk_proc)goto procofs;//04.10.04 14:25

						if(*tok4==tk_declare)goto undefofs;	//24.09.04 00:33
//						printf("tok=%d segm=%d %s\n",*tok4,itok4->segm,itok4->name);
						if(strinf.bufstr){
							*tok4=tk_rmnumber;
							if(itok4->type==tp_classvar)itok4->flag|=f_useidx;
						}
						else{
							if(FixUp&&*tok4==tk_proc)itok4->flag|=f_reloc;	//new!!!
							else itok4->flag&=~f_reloc;
//							else itok4->flag&=!f_reloc; 16.08.04 18:51

//30.09.04 20:22
							if(itok4->post){
								*tok4=tk_postnumber;
								itok4->rm=tk_undefofs;
							}
							else
/////

							 *tok4=tk_number;
						}
					}
					else{
undefofs:
						itok4->rm=*tok4=tk_undefofs;	//смещение еще не известной метки
						itok4->number=0;
						if(FixUp)itok4->flag|=f_reloc;	//new!!!
					}
				case tk_number:
				case tk_rmnumber:
					break;
				case tk_reg:
				case tk_reg32:
					itok4->rm=RegToRM(itok4->number,*tok4)/*|rm_mod10*/;
					*tok4=(am32==0?tk_wordvar:tk_dwordvar);//tk_rmnumber;
					itok4->number=0;
					break;
				default:
					if(cha=='.')goto structofs;
					if((*tok4>=tk_bits&&*tok4<=tk_doublevar)||tok==tk_pointer){
#ifdef OPTVARCONST
						if(displaytokerrors)ClearVarByNum(itok4);
#endif
						switch(itok4->type){
							case tp_localvar:
							case tp_paramvar:
localrec *ptr;
								ptr=itok4->locrec;
								ptr->fuse|=(unsigned char)(INITVAR|USEDVAR);
								break;
							case tk_proc:
								itok4->rm=itok4->sib;
								itok4->sib=am32==0?CODE16:CODE32;
								break;
						}
						itok4->size=GetVarSize(*tok4);
						if((itok4->sib==CODE16&&itok4->rm==rm_d16)||
								(itok4->sib==CODE32&&itok4->rm==rm_d32)){
							if(itok4->post==0){
								*tok4=tk_number;
								itok4->segm=DS;
								if(FixUp)itok4->flag|=f_reloc;
								itok4->rm=am32==FALSE?tk_word:tk_dword;
							}
							else{
								*tok4=tk_postnumber;
								itok4->rm=tk_undefofs;
							}
						}
						else{
							*tok4=tk_rmnumber;
							if(itok4->type==tp_classvar)itok4->flag|=f_useidx;
						}
					}
					else{
						itok4->rm=*tok4=tk_undefofs;	//смещение еще не известной метки
						itok4->number=0;
						strcpy((char *)string4,itok4->name);
					}
					break;
			}
			itok4->type=tp_ofs;
//			if(displaytokerrors)printf("name=%s tok=%d post=%d flag=%08X rm=%d\n",itok4->name,*tok4,itok4->post,itok4->flag,itok4->rm);
			break;
		case '(':
			if(*(unsigned short *)&(input[inptr])==
#ifdef _WC_
				')E'
#else
				'E)'
#endif
				||
					(*(unsigned short *)&(input[inptr])==
#ifdef _WC_
				')e'
#else
				'e)'
#endif
					)){
				for(useme=0;useme<8;useme++){
					int i;
					if(asmparam)i=strnicmp((char *)input+inptr+2,regs[0][useme],2);
					else i=strncmp((char *)input+inptr+2,regs[0][useme],2);
					if(i==0){
						inptr+=4;
						nextchar();
						itok4->number=useme;
						if(am32){
							*tok4=tk_reg32;
							goto extreg32;
						}
						*tok4=tk_reg;
						goto extreg;
					}
				}
			}
			*tok4=tk_openbracket;
			break;
		case ':':
			nextchar();
			if(cha!=':'){
				next=0;
				*tok4=tk_colon;
			}
			else *tok4=tk_dblcolon;
			break;
		case ';': itok4->type=tp_stopper; *tok4=tk_semicolon;	break;
		case ')': itok4->type=tp_stopper; *tok4=tk_closebracket; break;
		case '{': *tok4=tk_openbrace; break;
		case '}': itok4->type=tp_stopper; *tok4=tk_closebrace; break;
		case '[': *tok4=tk_openblock; break;
		case ']': itok4->type=tp_stopper; *tok4=tk_closeblock; break;
		case ',': itok4->type=tp_stopper; *tok4=tk_camma; break;
		case '.':
			next=0;
			nextchar();
			if(cha!='.'){
				*tok4=tk_period;
				break;
			}
			nextchar();
			if(cha!='.'){
				badtoken();
				nextchar();
				tokscan(tok4,itok4,string4);
				break;
			}
			nextchar();
			*tok4=tk_multipoint;
			break;
		case '@': *tok4=tk_at; break;
		case '$': *tok4=tk_dollar; break;
		case '?':
			nextchar();
			tokscan(tok4,itok4,string4);
			*tok4=tk_question;
			itok4->number=GetDirective((char *)string4);
			return;
		case '~': *tok4=tk_tilda; break;
		case 26: *tok4=tk_eof; return;
		case 13: *tok4=tk_endline; break;
		case '\\':
			nextchar();
			if(cha==13){
				tokscan(tok4,itok4,string4);
				if(*tok4==tk_endline)tokscan(tok4,itok4,string4);
				return;
			}
		default:
			badtoken();
			nextchar();
			tokscan(tok4,itok4,string4);
			return;
	}
	if(next)nextchar();
#ifdef DEBUGMODE
	if(debug)printf("2172 tok=%d input=%08X inptr=%08X disp=%d %s %X %s\n",*tok4,input,inptr,displaytokerrors,itok4->name,cha,input+inptr);
#endif
}


void NewMod(int numipar)
{
COM_MOD *newm;
int oline;
int numpar=0,ns,i,size,nsize;
char *buf=NULL;
	newm=(COM_MOD *)MALLOC(sizeof(COM_MOD));
	newm->numparamdef=numipar;
	newm->freze=FALSE;
	size=0;
	whitespace();
	if(cha=='('){
		nextchar();
		whitespace();
		if(cha!=')'){
			inptr--;
			oline=linenumber;
			for(i=inptr,ns=1;ns>0;i++){	//поиск параметров
				switch(input[i]){
					case '(': ns++; break;
					case ')': ns--; break;
					case ',':
						if(ns==1){
							numpar++;
							nsize=i-inptr;
							if(!buf)buf=(char *)MALLOC(nsize+1);
							else buf=(char *)REALLOC(buf,size+nsize+1);
							strncpy(buf+size,(char *)(input+inptr),nsize);
							buf[size+nsize]=0;
							strbtrim(buf+size);
//							puts(buf+size);
							size+=strlen(buf+size)+1;
							inptr+=nsize+1;
						}
						break;
					case '/':
						i=skipcomment(i);
						break;
					case '"':
					case 0x27:
						i=skipstring(i,input[i]);
						break;
				}
				if((unsigned int)i>=endinptr){
					unexpectedeof();
					break;
				}
			}
			linenumber=oline;
			numpar++;
			nsize=i-inptr;
			if(!buf)buf=(char *)MALLOC(nsize);
			else buf=(char *)REALLOC(buf,size+nsize);
			strncpy(buf+size,(char *)(input+inptr),nsize-1);
			buf[size+nsize-1]=0;
			strbtrim(buf+size);
//							puts(buf+size);
			inptr=i;
		}
		if(numipar>numpar)missingpar(itok.name);
		else if(numipar<numpar)extraparam(itok.name);
		newm->numparamdef=numpar;
	}
	else inptr--;
	newm->input=input;
	newm->inptr2=newm->inptr=inptr;
	newm->endinptr=endinptr;
	newm->linenumber=linenumber;
	newm->currentfileinfo=currentfileinfo;
	newm->paramdef=buf;
	newm->next=cur_mod;
	cur_mod=newm;
//	if(debug)printf("New curmod %08X numpar=%d input=%08X prev curmod=%08X\n",cur_mod,numipar,input,cur_mod->next);
	nextchar();
/*	char *temp=buf;
	for(i=0;i<numpar;i++){
		nsize=strlen(temp)+1;
		puts(temp);
		temp+=nsize;
	}*/
}

void BackMod()
{
	if(cur_mod){
		inptr=cur_mod->inptr;
		inptr2=cur_mod->inptr2;
		input=cur_mod->input;
		endinptr=cur_mod->endinptr;
		linenumber=cur_mod->linenumber;
		currentfileinfo=cur_mod->currentfileinfo;
		cur_mod=cur_mod->next;
		cha2=input[inptr2];
	}
}

char *FindDefPar(char *name,COM_MOD *cmod)
{
char *temp;
int i,numpar,nsize=0;
	numpar=cmod->numparamdef;
	temp=cmod->declareparamdef;
//	if(debug)printf("check for %s\n",name);
	for(i=0;i<numpar;i++){
		nsize=strlen(temp)+1;
//		if(debug)printf("\t%s\n",temp);
		if(strcmp(name,temp)==0){
			temp=cmod->paramdef;
			for(;i>0;i--){
				temp+=strlen(temp)+1;
			}
			return temp;
		}
		temp+=nsize;
	}
	return NULL;
}

int CheckDefPar(char *name)
{
char *temp,*nname;
COM_MOD *fmod;
	fmod=cur_mod;
	nname=name;
	while((temp=FindDefPar(nname,fmod))!=NULL){
		nname=temp;
		if(fmod->next==NULL)break;
		fmod=fmod->next;
	}
	if(fmod==cur_mod&&temp==NULL)return FALSE;
	SetNewStr(nname);
	return TRUE;
}

void SetNewStr(char *name)
{
COM_MOD *fmod;
	fmod=(COM_MOD *)MALLOC(sizeof(COM_MOD));
	fmod->next=cur_mod;
	fmod->numparamdef=0;
	fmod->paramdef=NULL;
	fmod->freze=FALSE;
	fmod->input=input;
	fmod->inptr2=fmod->inptr=inptr-1;
	fmod->endinptr=endinptr;
	fmod->linenumber=linenumber;
	fmod->currentfileinfo=currentfileinfo;
	cur_mod=fmod;
//	if(debug)printf("new curmod %08X prev cur_mod=%08X old input %08X %s\n",cur_mod,cur_mod->next,input,name);
	input=(unsigned char*)(name);
	inptr=1;
	cha=input[0];
	endinptr=strlen(name)+1;
}

int searchtree2(idrec *fptr,ITOK *itok4,int *tok4,unsigned char *string4)
//поиск в дереве переменых
{
struct idrec *ptr;
int cmpresult;
	for(ptr=fptr;ptr!=NULL;){
		if((cmpresult=strcmp(ptr->recid,(char *)string4))==0){
			if(scanlexmode==RESLEX&&ptr->rectok!=tk_number&&ptr->rectok!=tk_string)break;
			itok4->lnumber=ptr->reclnumber;
			itok4->rm=ptr->recrm;
			*tok4=ptr->rectok;
			itok4->post=ptr->recpost;
			itok4->segm=ptr->recsegm;
			itok4->flag|=ptr->flag;
			itok4->size=ptr->recsize;
			itok4->sib=ptr->recsib;
			itok4->rec=ptr;
			itok4->type=ptr->type;
			itok4->npointr=ptr->npointr;
//			if(debug)printf("Find: tok=%d\n",*tok4);
			if(displaytokerrors){
				ptr->count++;
//				printf("%s notdoneprestuff=%d tok=%d segm=%d\n",itok4->name,notdoneprestuff,*tok4,itok4->segm);
				if(notdoneprestuff==2&&*tok4==tk_proc&&itok4->segm==DYNAMIC)AddDynamicList(ptr);
			}
			if(*tok4==tk_macro){
//					printf("tok=%d %s %s\n",*tok4,itok4->name,input);
				if(dirmode==dm_if)return TRUE;
				if(scanlexmode==DEFLEX||scanlexmode==DEFLEX2)*tok4=tk_id;
				else if(displaytokerrors){
					NewMod(itok4->size);
					notdef=FALSE;
					cur_mod->declareparamdef=ptr->newid;
					input=(unsigned char*)(ptr->sbuf);
					inptr=1;
					cha=input[0];
					endinptr=strlen((char *)input);
//					if(debug)printf("New cur_mod tok=%d numpar=%d %s %s\n",*tok4,cur_mod->numparamdef,itok4->name,input);
//					debug=TRUE;
					clearregstat();
					*tok4=tk_endline;
					return TRUE;
				}
			}
			else if(ptr->newid){
				switch(*tok4){
					case tk_string:
						memcpy((char *)string4,ptr->newid,itok4->number);
						if(displaytokerrors&&itok4->rm==1&&ptr->sbuf)strcpy((char *)string3,ptr->sbuf);
						break;
					case tk_structvar:
						itok4->segm=0;
						if(ptr->recpost==DYNAMIC_POST){
							ptr->recpost=itok4->post=1;
							if(alignword){	//выровнять на четный адрес
								if(postsize%2==1)postsize++;
							}
							itok4->number=ptr->recnumber=postsize;
							if(am32==FALSE&&(ptr->recsize+postsize)>0xFFFFL)tobigpost();
							postsize+=ptr->recsize;
						}
						else if(ptr->recpost==DYNAMIC_VAR)itok4->post=ptr->recpost=USED_DIN_VAR;
						return TRUE;
					case tk_proc:
					case tk_apiproc:
					case tk_declare:
					case tk_undefproc:
						strcpy((char *)string4,ptr->newid);
						break;
					default:
						if(scanlexmode==DEFLEX2){
							*tok4=tk_id;
							return TRUE;
						}
						strcpy((char *)string4,ptr->newid);
						if(strcmp((char *)string4,ptr->recid)!=0){	 // see if ID has changed
//							searchtree2(fptr,itok4,tok4,string4);  // search again
							searchtree(itok4,tok4,string4);  // search again
							switch(*tok4){
								case tk_proc:
								case tk_apiproc:
								case tk_declare:
								case tk_undefproc:
									strcpy(itok4->name,ptr->recid);	//имя нужно для undefine
									break;
								default:
									strncpy(itok4->name,(char *)string4,IDLENGTH-1);	//имя нужно для undefine
									break;
							}
							return TRUE;
						}
						break;
				}
			}
			else string4[0]=0;
			strcpy(itok4->name,ptr->recid);	//имя нужно для undefine
			if(displaytokerrors)ptr->count++;
			break;
		}
		else if(cmpresult<0)ptr=ptr->left;
		else ptr=ptr->right;
	}
	if(ptr==NULL)return FALSE;
	else return TRUE;
}

int searchtree(ITOK *itok4,int *tok4,unsigned char *string4)
{
int retval=FALSE;
	if(skipfind==FALSE){
		if(staticlist)retval=searchtree2(staticlist,itok4,tok4,string4);
		if(!retval)retval=searchtree2(treestart,itok4,tok4,string4);
		if(!retval){
			if((retval=searchtree2(definestart,itok4,tok4,string4))==TRUE){
				if(scanlexmode==DEFLEX2)*tok4=tk_id;
			}
		}
	}
	return retval;
}

void AddDynamicList(idrec *ptr)
{
//	printf("Add dinamic list %s %08X seg=%d tok=%d\n",ptr->recid,ptr,ptr->recsegm,ptr->rectok);
	if(DynamicList==NULL){
		sizeDL=STEPDL;
		DynamicList=(volatile idrec **)MALLOC(sizeof(idrec **)*sizeDL);
		countDP=0;
	}
	else if(sizeDL==(countDP+1)){
		sizeDL+=STEPDL;
		DynamicList=(volatile idrec **)REALLOC(DynamicList,sizeof(idrec **)*sizeDL);
	}
	for(int i=0;i<countDP;i++)if(ptr==DynamicList[i])return;
	DynamicList[countDP]=ptr;
	countDP++;
}


void docals(struct idrec *ptr)
/* extract any procedures required from interal library and insert any
	 dynamic procedures that have been called.*/
{
	if(ptr!=NULL){
//		printf("%s\n",ptr->recid);
		tok=ptr->rectok;
		if(sdp_mode==FALSE&&(ptr->flag&f_export)!=0&&tok==tk_proc){
			if(lexport==NULL){
				lexport=(struct listexport *)MALLOC(sizeof(struct listexport));
				lexport->address=ptr->recnumber;
				strcpy(lexport->name,ptr->recid);
				numexport=1;
			}
			else{
				int cmpname,i;
				for(i=0;i<numexport;i++){
					if((cmpname=strcmp(ptr->recid,(lexport+i)->name))<=0)break;
				}
				if(cmpname!=0){
					lexport=(struct listexport *)REALLOC(lexport,sizeof(struct listexport)*(numexport+1));
					if(cmpname<0){
						for(int j=numexport;j>i;j--){
							memcpy(&(lexport+j)->address,&(lexport+j-1)->address,sizeof(struct listexport));
						}
					}
					numexport++;
					(lexport+i)->address=ptr->recnumber;
					strcpy((lexport+i)->name,ptr->recid);
				}
			}
		}
		if(!(ptr->flag&f_extern)){
			if(tok==tk_undefproc){
				strcpy(itok.name,ptr->recid);
				if(sdp_mode){
					int boutptr=outptr;
					int i;
					if((ptr->flag&f_typeproc)==tp_fastcall){
						if((i=includeit(1))!=-1){
							if(updatecall((unsigned int)ptr->recnumber,boutptr,0)>0){
								ptr->rectok=tk_proc;
								ptr->recnumber=boutptr;	// record location placed
								ptr->recrm=i;
								ptr->count++;
							}
						}
					}
					else{
						if((i=includeproc())!=-1){
							if(updatecall((unsigned int)ptr->recnumber,boutptr,0)>0){
								ptr->rectok=tk_proc;
								ptr->recnumber=boutptr;	// record location placed
								ptr->recrm=i;
								ptr->count++;
							}
						}
					}
				}
				else{
					if(updatecall((unsigned int)ptr->recnumber,outptr,0)>0){
						ptr->recnumber=outptr;	// record location placed
						linenumber=ptr->line;
						currentfileinfo=ptr->file;
						if((ptr->flag&f_typeproc)==tp_fastcall){
							if(includeit(1)==-1)thisundefined(itok.name);
						}
						else{
							if(includeproc()==-1)thisundefined(itok.name);
						}
						ptr->rectok=tk_proc;
						ptr->count++;
					}
				}
			}
			else if(tok==tk_proc){
//				printf("%08X %s\n",ptr->recpost,ptr->recid);
				itok.segm=ptr->recsegm;
				if(itok.segm==DYNAMIC_USED){
//				printf("%08X %s\n",ptr->recpost,ptr->recid);
//					if(updatecall((unsigned int)ptr->recnumber,outptr,0)>0){
					itok.number=ptr->recnumber;
					itok.flag=ptr->flag;
					itok.post=ptr->recpost;
					strcpy(itok.name,ptr->recid);
					if(ptr->newid==NULL)string[0]=0;
					else strcpy((char *)string,(char *)ptr->newid);
					itok.rm=ptr->recrm;
					itok.size=ptr->recsize;
					itok.rec=ptr;
					insert_dynamic();
//					}
				}
			}
//			else printf("tok=%d %s\n",tok,ptr->recid);
		}
		docals(ptr ->left);
		docals(ptr ->right);
	}
}

void docalls2()
{
//	puts("start docalls2");
	docals(treestart);
	for(unsigned int i=0;i<totalmodule;i++)docals((startfileinfo+i)->stlist);
//	puts("end docalls2");
}

void docalls()	/* attempt to declare undefs from library and dynamic proc's */
{
int numdinproc;
//	puts("start docalls");
	notdoneprestuff=2;
	docalls2();
	while(DynamicList!=NULL){
		idrec *ptr;
		numdinproc=0;
//		printf("%d dinamic proc\n",countDP);
		for(int i=0;i<countDP;i++){
			ptr=(idrec *)DynamicList[i];
			tok=ptr->rectok;
			itok.segm=ptr->recsegm;
//			printf("%d %08X seg=%d tok=%d %s\n",i+1,ptr,itok.segm,ptr->rectok,ptr->recid);
			if(itok.segm==DYNAMIC_USED){
				itok.number=ptr->recnumber;
				itok.flag=ptr->flag;
				strcpy(itok.name,ptr->recid);
				if(ptr->newid==NULL)string[0]=0;
				else strcpy((char *)string,(char *)ptr->newid);
				itok.rm=ptr->recrm;
				itok.size=ptr->recsize;
				itok.rec=ptr;
				insert_dynamic();
				numdinproc++;
			}
		}
		docalls2();
		if(numdinproc==0)break;
	}
	free(DynamicList);
	DynamicList=NULL;
	countDP=0;
//	puts("end docalls");
}

void CreatParamDestr(idrec *ptrs)
{
	if(am32==FALSE){
		structadr.rm=rm_d16;
		structadr.sib=CODE16;
	}
	else{
		structadr.rm=rm_d32;
		structadr.sib=CODE32;
	}
	structadr.segm=DS;
	structadr.number=ptrs->recnumber;
	structadr.flag=0;
	structadr.size=0;	//addofs
	structadr.rec=NULL;
	structadr.type=tp_ucnovn;
	structadr.npointr=0;
	structadr.post=ptrs->recpost;
	if(ptrs->recsegm==USEDSTR)structadr.sib++;
	if(structadr.post==LOCAL){
		if(ESPloc&&am32){
			structadr.rm=rm_mod10|rm_sib;
			structadr.sib=0x24;
			structadr.number+=addESP;
		}
		else{
			structadr.rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
		}
		structadr.segm=SS;
		compressoffset(&structadr);
	}
}

void CallDestr(idrec *ptr)
{
int oinptr2;
char ocha2;
int oldendinptr=endinptr;
unsigned char odbg;
int otok,otok2;
unsigned char *oinput;
ITOK oitok;	//18.08.04 19:07
	oinptr2=inptr2;
	ocha2=cha2;
	odbg=dbg;
	dbg=0;
	otok=tok;
	otok2=tok2;
	oitok=itok;	//18.08.04 19:07
	oinput=input;
	string[0]=0;
	if(ptr->newid)strcpy((char *)string,ptr->newid);
	itok.type=ptr->type;
	itok.npointr=ptr->npointr;
	itok.rm=ptr->recrm;
	itok.flag=ptr->flag;
	itok.rec=ptr;
	strcpy(itok.name,ptr->recid);
	itok.number=ptr->recnumber;
	tok=ptr->rectok;
	itok.post=ptr->recpost;
	itok.segm=ptr->recsegm;
	itok.sib=ptr->recsib;
	ptr->count++;
	if(itok.segm==DYNAMIC)AddDynamicList(ptr);
	tok2=tk_openbracket;
	input=(unsigned char*)"();";
	inptr2=1;
	cha2='(';
	endinptr=3;
	docommand();
	tok=otok;
	input=oinput;
	inptr2=oinptr2;
	cha2=ocha2;
	tok2=otok2;
	endinptr=oldendinptr;
	dbg=odbg;
	itok=oitok;//18.08.04 19:07
}

elementteg *FindOneDestr(structteg *searcht)
{
elementteg *bazael=searcht->baza,*retrez=NULL;
idrec *ptrs;
int i;
/// new!
	for(i=0;i<searcht->numoper;i++){
		ptrs=(bazael+i)->rec;
		if((bazael+i)->tok==tk_proc&&(ptrs->flag&fs_destructor)!=0)return(bazael+i);
	}
	for(i=0;i<searcht->numoper;i++){
		ptrs=(bazael+i)->rec;
		if((bazael+i)->tok==tk_baseclass
				||(bazael+i)->tok==tk_struct  //new 20.06.05 21:32
				){
			if((retrez=FindOneDestr((structteg *)ptrs))!=NULL)break;
		}
	}
	return retrez;
}

void Destructor(idrec *ptrs)
{
char name[IDLENGTH];
struct elementteg *bazael;
int addofs;
structteg *tteg;
	tteg=(structteg *)ptrs->newid;
	sprintf(name,"%s~",tteg->name);
	if((bazael=FindClassEl(tteg,(unsigned char *)name,&addofs,NULL))==NULL){
		if((bazael=FindOneDestr(tteg))==NULL)preerror("destructor not defined");
	}
	if(ptrs->recsize)CreatParamDestr(ptrs);
	else structadr.sib=THIS_ZEROSIZE;
	CallDestr(bazael->rec);
}

void RunNew(int size)
{
char buf[128];
int oinptr2;
char ocha2;
int oldendinptr=endinptr;
unsigned char odbg;
int otok,otok2;
unsigned char *oinput;
oinptr2=inptr2;
ocha2=cha2;
odbg=dbg;
	sprintf(buf,"__new(%u);}",size);
	dbg=0;
	otok=tok;
	otok2=tok2;
	oinput=input;
	input=(unsigned char*)buf;
	inptr2=1;
	cha2='_';
	tok=tk_openbrace;
	endinptr=strlen(buf);
	doblock();
	tok=otok;
	tok2=otok2;
	input=oinput;
	inptr2=oinptr2;
	cha2=ocha2;
	endinptr=oldendinptr;
	dbg=odbg;
}

void donew()
{
structteg *tteg;
elementteg *bazael;
int addofs=0;
struct idrec *ptr;
	nexttok();
	if((tteg=FindTeg(FALSE))==NULL&&(tteg=FindTeg(TRUE))==NULL){
		tegnotfound();
er:
		while(tok2!=tk_semicolon&&tok!=tk_eof)nexttok();
		return;
	}
//	printf("flag=%08X\n",tteg->flag);
	if((tteg->flag&fs_constructor)==0){
		RunNew(tteg->size);
		goto er;
	}
	bazael=FindClassEl(tteg,(unsigned char *)tteg->name,&addofs,NULL);
	structadr.sib=THIS_NEW;
	structadr.number=tteg->size;
	ptr=bazael->rec;
	string[0]=0;
	if(ptr->newid!=NULL)strcpy((char *)string,ptr->newid);
	itok.type=ptr->type;
	itok.npointr=ptr->npointr;
	itok.rm=ptr->recrm;
	itok.flag=ptr->flag;
	itok.rec=ptr;
	strcpy(itok.name,ptr->recid);
	itok.number=ptr->recnumber;
	tok=ptr->rectok;
	itok.post=ptr->recpost;
	itok.segm=ptr->recsegm;
	itok.sib=ptr->recsib;
	ptr->count++;
	if(itok.segm==DYNAMIC)AddDynamicList(ptr);
//	docommand();
	if(dbg)AddLine();
	int oflag=current_proc_type;
	current_proc_type&=~f_static;
	if(tok==tk_proc)doanyproc();
	else doanyundefproc();
	current_proc_type=oflag;
}

void dodelete()
{
int addofs=0;
int oinptr,oinptr2;
char ocha2;
int oldendinptr=endinptr;
unsigned char odbg;
unsigned char *oinput;
structteg *tteg=NULL;
elementteg *bazael;
char buf[128];
	getoperand();
	if(tok==tk_structvar){
		oinptr=tok2;
//		if((itok.flag&fs_useconstr)!=0){
			if((itok.flag&fs_destructor)!=0){
				Destructor(itok.rec);
			}
			else if(itok.rec->newid&&AskUseDestr((structteg *)itok.rec->newid))Destructor(itok.rec);
//		}
		if(oinptr!=tk_semicolon)nexttok();
	}
	else{
		ITOK wtok;
		char *wbuf;
		wbuf=bufrm;
		bufrm=NULL;
		wtok=itok;
		SINFO wstr;
		wstr=strinf;
		strinf.bufstr=NULL;
		getinto_e_ax(0,tok,&wtok,wbuf,&wstr,(am32+1)*2);
		bufrm=NULL;
		strinf.bufstr=NULL;
		if(tok2!=tk_semicolon){
			nexttok();
			if((tteg=FindTeg(FALSE))==NULL&&(tteg=FindTeg(TRUE))==NULL){
				tegnotfound();
				while(tok!=tk_semicolon&&tok!=tk_eof)nexttok();
				return;
			}
		}
		oinptr2=inptr2;
		ocha2=cha2;
		odbg=dbg;
		dbg=0;
		oinput=input;
		int oflag=current_proc_type;
		current_proc_type&=~f_static;
		if(tteg){
			structadr.number=0;
			if((tteg->flag&fs_destructor)!=0){
				op(0x50);
				addESP+=am32==FALSE?2:4;
				sprintf(buf,"%s~",tteg->name);
				bazael=FindClassEl(tteg,(unsigned char *)buf,&addofs,NULL);

				structadr.sib=THIS_REG;
				structadr.rm=AX;
				structadr.size=addofs;	// ???? 19.08.04 12:54 нигде не используется
				CallDestr(bazael->rec);
				addESP-=am32==FALSE?2:4;
				op(0x58);
			}
			else if(AskUseDestr(tteg)){
				bazael=FindOneDestr(tteg);
				op(0x50);
				addESP+=am32==FALSE?2:4;
				structadr.sib=THIS_REG;
				structadr.rm=AX;
				structadr.size=0;//addofs;
				CallDestr(bazael->rec);
				addESP-=am32==FALSE?2:4;
				op(0x58);
			}
		}
		input=(unsigned char*)"__delete((E)AX);}";
		inptr2=1;
		cha2='_';
		endinptr=strlen((char *)input);
		tok=tk_openbrace;
		doblock();
		input=oinput;
		inptr2=oinptr2;
		cha2=ocha2;
		endinptr=oldendinptr;
		dbg=odbg;
		current_proc_type=oflag;
	}
	nextseminext();
}

int AskUseDestr(structteg *searcht)
{
elementteg *bazael=searcht->baza;
	if((searcht->flag&fs_destructor)!=0)return TRUE;
	for(int i=0;i<searcht->numoper;i++){
//		printf("tok=%d\n",(bazael+i)->tok);
		if((bazael+i)->tok==tk_baseclass
				||(bazael+i)->tok==tk_struct	//new 20.06.05 21:33
				){
			if(AskUseDestr((structteg *)(bazael+i)->rec))return TRUE;
		}
	}
	return FALSE;
}

void AutoDestructor()
{
struct localrec *ptrs;
int calldestruct=FALSE;
int zerosize=FALSE;
treelocalrec *ftlr;
	for(ftlr=tlr;ftlr!=NULL;ftlr=ftlr->next){
		for(ptrs=ftlr->lrec;ptrs!=NULL;ptrs=ptrs->rec.next){
			if(ptrs->rec.rectok==tk_structvar){
				if((ptrs->rec.flag&fs_destructor)!=0||AskUseDestr((structteg *)ptrs->rec.newid)){
					if(!calldestruct){
						calldestruct=TRUE;
						if(ptrs->rec.recsize==0||returntype==tk_void)zerosize=TRUE;
						else{
							op66(r32);
							op(0x50);
							addESP+=4;
						}
					}
					Destructor(&ptrs->rec);
				}
			}
		}
	}
	if(calldestruct&&zerosize==FALSE){
		op66(r32);
		op(0x58);
		addESP-=4;
	}
}

elementteg *FindClassEl(structteg *searcht,unsigned char *string4,int *addofs,
	structteg *subteg)
{
struct elementteg *bazael=searcht->baza;
int numclass=0;
void **listclass;
//11.08.04 22:54
char name[IDLENGTH];
	strcpy(name,searcht->name);

	for(unsigned int i=0;i<searcht->numoper;i++){
		if((bazael+i)->tok==tk_baseclass){
			if(!numclass)listclass=(void **)MALLOC(sizeof(void **));
			else listclass=(void **)REALLOC(listclass,sizeof(void **)*(numclass+1));
			listclass[numclass]=(void *)(bazael+i)->nteg;
			numclass++;
			continue;
		}
		if(strcmp((bazael+i)->name,(char *)string4)==0){
			if(subteg&&subteg!=searcht)continue;
			if(numclass)free(listclass);
//11.08.04 22:54
			strcpy((char *)string4,name);

			return (bazael+i);
		}
	}
	if(numclass){
structteg *ssubteg=subteg;
		for(int i=numclass-1;i>=0;i--){
			structteg *s=(structteg *)listclass[i];
			if(subteg==searcht)ssubteg=s;
			if((bazael=FindClassEl(s,string4,addofs,ssubteg))!=NULL){
				i--;
				while(i>=0){
					s=(structteg *)listclass[i];
					*addofs+=s->size;
					i--;
				}
				free(listclass);
				return bazael;
			}
		}
		free(listclass);
	}
	return NULL;
}

int CallDestructor(structteg *searcht)
{
elementteg *bazael=searcht->baza;
idrec *ptrs;
	for(int i=0;i<searcht->numoper;i++){
		ptrs=(bazael+i)->rec;
		if((searcht->flag&fs_destructor)==0){
			if((bazael+i)->tok==tk_baseclass){
				if(CallDestructor((structteg *)ptrs))return TRUE;
			}
		}
		else if((bazael+i)->tok==tk_proc&&(ptrs->flag&fs_destructor)!=0){
			structadr.sib=THIS_PARAM;
			structadr.number=0;//addofs;
			CallDestr(ptrs);
			return TRUE;
		}
	}
	return FALSE;
}

void CopyTok(int *tok4,ITOK *itok4,idrec *ptr)
{
	itok4->number=ptr->recnumber;
	itok4->rm=ptr->recrm;
	*tok4=ptr->rectok;
	itok4->post=ptr->recpost;
	itok4->segm=ptr->recsegm;
	itok4->flag=ptr->flag;
	itok4->size=ptr->recsize;
	itok4->sib=ptr->recsib;
	itok4->rec=ptr;
	itok4->type=ptr->type;
	itok4->npointr=ptr->npointr;
}

int searchlocals(ITOK *itok4,int *tok4,unsigned char *string4)
//поиск локальных переменых связаного списка
{
	if(skiplocals){
		skiplocals=FALSE;
		return FALSE;
	}
	if(skipfind==LOCAL)return FALSE;
treelocalrec *ftlr;
struct localrec *ptr;
	for(ftlr=tlr;ftlr!=NULL;ftlr=ftlr->next){
		for(ptr=ftlr->lrec;ptr!=NULL;ptr=ptr->rec.next){
//			puts(ptr->rec.recid);
			if(strcmp(ptr->rec.recid,(char *)string4)==0){
				itok4->number=ptr->rec.recnumber;
				*tok4=ptr->rec.rectok;
				itok4->locrec=ptr;
				if(displaytokerrors){
					if(mapfile){
						if(ptr->li.count==0)ptr->li.usedfirst=linenumber;
						else ptr->li.usedlast=linenumber;
						ptr->li.count++;
					}
					if(*tok4==tk_structvar)ptr->rec.count++;
				}
//				printf("type=%d num=%08X %s\n",ptr->rec.type,ptr->rec.recnumber,ptr->rec.recid);
				if(ptr->rec.type==tp_postvar){
					itok4->segm=DS;
					itok4->flag=ptr->rec.flag;
					itok4->size=ptr->rec.recsize;
					itok4->rm=(am32==FALSE?rm_d16:rm_d32);
					itok4->npointr=ptr->rec.npointr;
					itok4->type=tp_localvar;
					itok4->post=1;
					if(*tok4==tk_structvar){
						itok4->rec=&ptr->rec;
						itok4->post=ptr->rec.recpost;
//						itok4->segm=0;
					}
					return TRUE;
				}
				if(ptr->rec.type==tp_gvar){
					itok4->number=0;
					itok4->rm=(am32==FALSE?rm_d16:rm_d32);
					itok4->segm=DS;
					itok4->size=ptr->rec.recsize;
					itok4->npointr=ptr->rec.npointr;
					itok4->type=ptr->rec.recsib;	//????01.09.05 15:58
					itok4->post=USED_DIN_VAR;
					if(ptr->rec.recpost==DYNAMIC_VAR)ptr->rec.recpost=USED_DIN_VAR;
					itok4->rec=&ptr->rec;
					ptr->fuse=USEDVAR;
					if(*tok4==tk_structvar){
						itok4->number=ptr->rec.recnumber;
						itok4->flag|=ptr->rec.flag;
					}
					return TRUE;
				}
				itok4->type=ptr->rec.type;
				if(ptr->rec.type==tp_paramvar||ptr->rec.type==tp_localvar){
					if(ESPloc&&am32){
						itok4->rm=rm_mod10|rm_sib;
						itok4->sib=0x24;
						if(ptr->rec.type==tp_paramvar)itok4->number+=localsize+addESP;
						else itok4->number+=-4+addESP;
					}
					else itok4->rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
					itok4->segm=SS;
					itok4->npointr=ptr->rec.npointr;
					if(ptr->rec.npointr){
						itok4->type=(unsigned short)*tok4-tk_charvar+tk_char;
						*tok4=tk_pointer;
					}
					if(*tok4==tk_structvar){
						itok4->rec=&ptr->rec;
						itok4->post=ptr->rec.recpost;
						itok4->segm=0;
					}
				}
				else if(!(ptr->rec.rectok==tk_locallabel||ptr->rec.rectok==tk_number)){
					internalerror("Bad *tok4 value in searchlocals");
				}
				itok4->size=ptr->rec.recsize;
				itok4->flag=ptr->rec.flag;
				return TRUE;
			}
		}
	}
	if(searchteg){
struct elementteg *bazael;
int addofs=0;
structteg *subteg=NULL;
		if(cha==':'&&cha2==':'){
			nextchar();
			nextchar();
			if((subteg=FindTeg(TRUE,itok4->name))==NULL&&displaytokerrors)undefclass(itok4->name);
			tokscan(tok4,itok4,string4);
			whitespace();
		}
		if(destructor){
			destructor=FALSE;
			strcat((char *)string4,"~");
		}
		if((bazael=FindClassEl(searchteg,string4,&addofs,subteg))!=NULL){
			*tok4=bazael->tok;
			itok4->type=tp_classvar;
//			printf("tok=%d bazael->rec=%08X %s\n",*tok4,bazael->rec,itok4->name);
			if(*tok4==tk_proc||*tok4==tk_pointer){
				idrec *ptr=bazael->rec;
				if(strchr(itok4->name,'@')==NULL){
					strcat(itok4->name,"@");
					strcat(itok4->name,(char *)string4);
				}
//				}
//			printf("tok=%d %s %s\n",*tok4,ptr->recid,ptr->newid);
				if(*tok4==tk_proc&&ptr->newid!=NULL)strcpy((char *)string4,ptr->newid);
				CopyTok(tok4,itok4,bazael->rec);
				structadr.sib=THIS_PARAM;
				structadr.number=0;//addofs;
				if(displaytokerrors){
					ptr->count++;
					if(*tok4==tk_proc){
						if(itok4->segm==DYNAMIC)AddDynamicList(ptr);
					}
				}
				if(*tok4==tk_pointer)goto locvar;
			}
			else if(*tok4==tk_struct){
				*tok4=tk_structvar;
//				printf("teg=%08X\n",bazael->nteg);
				dostructvar2(tok4,itok4,(structteg *)bazael->nteg,string4);
				if(*tok4==tk_structvar){
					itok4->rec=(idrec *)bazael->nteg;
					itok4->number+=(itok4->post==0?0:itok4->rm*((structteg *)bazael->nteg)->size);
					itok4->size=(itok4->post==0?bazael->numel:1)*((structteg *)bazael->nteg)->size;

					itok4->number+=bazael->ofs+addofs;
					if(displaytokerrors){
						if(strinf.bufstr!=NULL)free(strinf.bufstr);
						strinf.bufstr=(char *)MALLOC(7);
						strcpy(strinf.bufstr,"&this;");
						strinf.size=1;
					}
//					itok4->flag|=f_useidx;
//					if(itok4->sib>=CODE16)itok4->sib++;
					return TRUE;

				}
				else if(*tok4==tk_proc){
					structadr.sib=THIS_PARAM;
					structadr.number+=bazael->ofs+addofs;

					if(itok4->sib>=CODE16)itok4->sib++;
					return TRUE;
				}

				goto locvar;;
			}
			else{
				if(bazael->rec){	//static var
					CopyTok(tok4,itok4,bazael->rec);
					return TRUE;
				}

locvar:
				itok4->number+=bazael->ofs+addofs;
				if(displaytokerrors){
					if(bufrm!=NULL)free(bufrm);
					bufrm=(char *)MALLOC(7);
					strcpy(bufrm,"&this;");
				}
//				itok4->flag|=f_useidx;
				if(itok4->sib>=CODE16)itok4->sib++;
//			printf("sib=%08X tok=%d rm=%d post=%d number=%d size=%d %s\n",itok4->sib,*tok4,itok4->rm,itok4->post,itok4->number,itok4->size,string4);
			}
			return TRUE;
		}
	}
	return FALSE;
}

void dostructvar2(int *tok4,ITOK *itok4,struct structteg *tteg,unsigned char *string4)	//разбор структур на переменные и структуры
{
struct elementteg *bazael;
int numel=0;
int usenumstruct=FALSE;
int sopenb;
long cnum;
ITOK cstok;
char *cstring;
char c=0;
int i;
structteg *subteg=NULL;
	structadr=*itok4;
//	bazael=tteg->baza;
	whitespace();//пропуск незначащих символов
	cstring=(char *)MALLOC(STRLEN);
	if(cha=='['){	//[	номер структуры
		usenumstruct=TRUE;
		nextchar();
		sopenb=inptr;
		unsigned char bcha=cha;
		tokscan(&i,&cstok,(unsigned char *)cstring);
		if(bufrm){
			free(bufrm);
			bufrm=NULL;
		}
		if(i==tk_number){	//числовой
			ITOK dstok;
			memcpy(&dstok,itok4,sizeof(ITOK));
			calcnum(&i,&cstok,cstring,&cnum);
			memcpy(itok4,&dstok,sizeof(ITOK));
			if(i!=tk_closeblock)goto notnum;
			numel=cnum;
		}
		else{
notnum:
//new!!!
			if(displaytokerrors){
				if(itok4->segm==USEDSTR)preerror("only once possible use variable an index structure");

//				if(itok4->segm==USEDSTR&&displaytokerrors)preerror("only once possible use variable an index structure");
				itok4->segm=USEDSTR;
				SRBackBuf(0);
				AddBackBuf(sopenb,bcha);
				CharToBackBuf(';');
				CharToBackBuf(0);
				if(cha!=']')blockerror();
				if(strinf.bufstr!=NULL)free(strinf.bufstr);//internalerror("Previous block was not used");
				strinf.bufstr=(char *)REALLOC(BackTextBlock,SizeBackBuf+1);
				SRBackBuf(1);
				nextchar();
//new!!!
			}
			else{
				do{
					tokscan(&i,&cstok,(unsigned char *)cstring);
					if(i==tk_eof)break;
				}while(i!=tk_closeblock);
			}

			c=1;
			strinf.size=tteg->size;
		}
		itok4->flag|=f_useidx;
		whitespace();//пропуск незначащих символов
	}
	if(cha=='.'){
		int fdestr=FALSE;
		nextchar();
		if(cha=='~'){
			nextchar();
			fdestr=TRUE;
		}
		skipfind=LOCAL;
		do{
			tokscan(&i,&cstok,(unsigned char *)cstring);
//			puts(cstok.name);
			if(cha=='.')searchtree2(definestart,&cstok,&i,(unsigned char *)cstring);
		}while(i==tk_endline);
		whitespace();
		if(cha==':'){
			nextchar();
			if(cha!=':')expectederror("::");
			nextchar();
			if((subteg=FindTeg(TRUE,cstok.name))==NULL&&displaytokerrors)undefclass(cstok.name);
			tokscan(&i,&cstok,(unsigned char *)cstring);
			whitespace();
		}
		skipfind=FALSE;
		int addofs=0;
		if(fdestr)strcat(cstok.name,"~");
		if((bazael=FindClassEl(tteg,(unsigned char *)cstok.name,&addofs,subteg))==NULL){
			if(displaytokerrors)unknownstruct(cstok.name,tteg->name);
			*tok4=tk_number;
			free(cstring);
			return;
		}
		free(cstring);
		*tok4=bazael->tok;
		itok4->size=bazael->numel*GetVarSize(*tok4);
		cnum=(c==0?tteg->size*numel:0);
		if((itok4->flag&f_extern)!=0)itok4->number+=((bazael->ofs+cnum)<<16);
		else itok4->number+=bazael->ofs+cnum;
		if(*tok4==tk_struct){
			*tok4=tk_structvar;
			itok4->size=bazael->numel*((structteg *)bazael->nteg)->size;
			dostructvar2(tok4,itok4,(structteg *)bazael->nteg,string4);
			return;
		}
		if((itok4->flag&f_extern)!=0)itok4->number+=addofs<<16;
		else itok4->number+=addofs;
//		printf("tok=%d %s\n",*tok4,bufrm);
		if(*tok4==tk_proc||*tok4==tk_pointer){
			idrec *ptr;//=NULL;
			if(*tok4==tk_proc){
				if(am32==FALSE){
					structadr.rm=rm_d16;
					structadr.sib=CODE16;
				}
				else{
					structadr.rm=rm_d32;
					structadr.sib=CODE32;
				}
				structadr.segm=DS;
				structadr.number=itok4->number;
//				structadr.number=0;
				structadr.flag=0;
				structadr.size=addofs;
				structadr.type=tp_ucnovn;
				structadr.npointr=0;
				structadr.post=itok4->post;
				structadr.rec=(structadr.post==USED_DIN_VAR?itok4->rec:NULL);
				if(itok4->segm==USEDSTR)structadr.sib++;
				if(structadr.post==LOCAL){
					if(ESPloc&&am32){
						structadr.rm=rm_mod10|rm_sib;
						structadr.sib=0x24;	//???
						structadr.number+=addESP;
					}
					else{
						structadr.rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
					}
					structadr.segm=SS;
					compressoffset(&structadr);
					if(tteg->size==0)structadr.sib=THIS_ZEROSIZE;	//14.11.05 13:51
				}
/*				if(strcmp(bazael->name,tteg->name)==0&&itok4->rec){	//вызов конструктора
					ptr=itok4->rec;
//					printf("constructor %08X\n",ptr);
				}*/
			}
//			if(ptr)printf("new address constructor %08X\n",bazael->rec);
			ptr=bazael->rec;
			itok4->type=ptr->type;
			itok4->npointr=ptr->npointr;
			itok4->rm=ptr->recrm;
			itok4->flag=ptr->flag;
			itok4->rec=ptr;
			strcpy(itok4->name,ptr->recid);
			if(*tok4==tk_pointer){
//				puts("Check this code");
				string4[0]=0;
				if(ptr->newid!=NULL)strcpy((char *)string4,ptr->newid);
				*tok4=ptr->rectok;
//!!!
				if(itok4->type==tk_proc&&cha!='('){
					itok4->type=(am32==FALSE?tk_wordvar:tk_dwordvar);
					goto notbit;
				}

				if(itok4->type!=tk_proc)goto notbit;
//				printf("post=%08X %s\n",itok4->post,itok4->name);
				if(itok4->post==LOCAL){
					if(ESPloc&&am32){
						itok4->rm=rm_mod10|rm_sib;
						itok4->sib=0x24;
						itok4->number+=addESP;
					}
					else{
						itok4->sib=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);	//???rm
					}
					itok4->segm=SS;
					itok4->post=FALSE;
				}
				else{
					itok4->sib=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
					itok4->segm=DS;
				}
//				itok4->post=0;
				itok4->flag|=f_useidx;
				return;
			}
			itok4->number=ptr->recnumber;
			*tok4=ptr->rectok;
			itok4->post=ptr->recpost;
			itok4->segm=ptr->recsegm;
			itok4->sib=ptr->recsib;
			itok4->size=ptr->recsize;	//11.08.04 00:24
//11.09.04 00:28
			if(strchr(itok4->name,'@')==NULL){
				strcat(itok4->name,"@");
				strcat(itok4->name,(char *)string4);
			}
			string4[0]=0;
			if(ptr->newid!=NULL)strcpy((char *)string4,ptr->newid);
////////////////////

//			printf("tok=%d num=%d %s\n",*tok4,itok4->number,itok4->name);
			if(displaytokerrors){
				ptr->count++;
				if(*tok4==tk_proc&&itok4->segm==DYNAMIC)AddDynamicList(ptr);
				if(*tok4!=tk_proc&&*tok4!=tk_undefproc&&*tok4!=tk_declare)thisundefined(ptr->recid,FALSE);
			}
			return;
		}
//		if((itok4->flag&f_extern)!=0)itok4->number+=addofs;
//		else itok4->number+=addofs;
		if(*tok4==tk_bits){
			itok4->flag|=f_useidx;
			itok4->number+=bazael->bit.ofs/8;
			if((bazael->bit.ofs%8)==0){
				switch(bazael->bit.siz){
					case 8:
						*tok4=tk_bytevar;
						goto notbit;
					case 16:
						*tok4=tk_wordvar;
						goto notbit;
					case 32:
						*tok4=tk_dwordvar;
						goto notbit;
				}
			}
			itok4->bit.siz=bazael->bit.siz;
			itok4->bit.ofs=bazael->bit.ofs%8;
		}
		else if(bazael->rec){	//static var
			CopyTok(tok4,itok4,bazael->rec);
			return;
		}
notbit:
		itok4->sib=cstok.sib;
		if(itok4->post==LOCAL){
			if(ESPloc&&am32){
				itok4->rm=rm_mod10|rm_sib;
//				printf("sib=%x tok=%d name=%s\n",itok4->sib,*tok4,itok4->name);
				itok4->sib=0x24;
				itok4->number+=addESP;
			}
			else{
				itok4->rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
				if(itok4->segm==USEDSTR&&itok4->sib>=CODE16)itok4->sib++;
			}
			itok4->segm=SS;
			itok4->post=FALSE;
			itok4->flag=f_useidx;
		}
		else{
			itok4->rm=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
			if(itok4->segm==USEDSTR&&itok4->sib>=CODE16)itok4->sib++;
			itok4->segm=DS;
			itok4->flag=FixUp|f_useidx;
		}
		return;
	}
//	itok4->size=(numel==0?bazael->numel:1)*((structteg *)bazael->nteg)->size;
//	itok4->rec=(idrec *)tteg;
	itok4->rm=numel;
	itok4->post=usenumstruct;
	free(cstring);
}

void dosizeof(ITOK *itok4)	//опр значение sizeof
{
struct structteg *tteg;
int i,brase=FALSE;
ITOK cstok;
	whitespace();//пропуск незначащих символов
	itok4->number=0;
	if(cha=='('){
		nextchar();
		brase=TRUE;
	}
//	if((cha!='(')&&(displaytokerrors))expected('(');
//	nextchar();
	cstok.name[0]=0;
//	if(displaytokerrors)puts("start sizeof");
	tokscan(&i,&cstok,string2);
//	if(displaytokerrors)printf("tok=%d findoffset=%d %s\n",i,findofset,cstok.name);
	if(i==tk_dblcolon&&numblocks){
		skiplocals=TRUE;
		tokscan(&i,&cstok,string2);
	}
	if(strcmp(cstok.name,"__CODESIZE")==0){
		itok4->post=CODE_SIZE+am32;
	}
	else if(strcmp(cstok.name,"__DATASIZE")==0){
		itok4->post=DATA_SIZE+am32;
	}
	else if(strcmp(cstok.name,"__POSTSIZE")==0){
		itok4->post=POST_SIZE+am32;
	}
	else if(strcmp(cstok.name,"__STACKSIZE")==0){
		itok4->post=STACK_SIZE+am32;
	}
	else{
		switch(i){
			case tk_char:
			case tk_byte:
			case tk_beg:
				itok4->number=1;
				break;
			case tk_int:
			case tk_word:
			case tk_reg:
			case tk_seg:
				itok4->number=2;
				break;
			case tk_float:
			case tk_reg32:
			case tk_long:
			case tk_dword:
				itok4->number=4;
				break;
			case tk_double:
			case tk_qword:
				itok4->number=8;
				break;
			case tk_string:
				itok4->number=cstok.number;
				if(/*itok4->*/cstok.flag!=no_term)itok4->number++;
				break;
			case tk_bits:
				itok4->number=cstok.bit.siz;
				break;
			case tk_proc:
			case tk_charvar:
			case tk_bytevar:
			case tk_intvar:
			case tk_wordvar:
			case tk_longvar:
			case tk_dwordvar:
			case tk_floatvar:
			case tk_structvar:
			case tk_qwordvar:
			case tk_doublevar:
			case tk_interruptproc:
				if(cstok.size==0)if(displaytokerrors)preerror("unknown size");
				itok4->number=cstok.size;
				break;
			default:
				if((tteg=FindTeg(FALSE,cstok.name))!=NULL||(tteg=FindTeg(TRUE,cstok.name))!=NULL){
					if(cha=='.'){
						dostructvar2(&i,itok4,tteg,string2);
						itok4->number=itok4->size;
						if(bufrm){
							free(bufrm);
							bufrm=NULL;
						}
					}
					else itok4->number=tteg->size;
					itok4->flag=0;
					break;
				}
				if(strcmp("file",(char *)string2)==0){
					displaytokerrors=1;
					tokscan(&i,&cstok,string2);
					if(i==tk_string){
						struct stat statbuf;
						if(stat((char *)string3,&statbuf)!=0)unableopenfile((char *)string3);
						else itok4->number=statbuf.st_size;
					}
					else stringexpected();
				}
				else if(displaytokerrors)preerror("illegal use of sizeof");
				break;
		}
	}
	whitespace();//пропуск незначащих символов
	if(brase){
		if(cha!=')'&&displaytokerrors)expected(')');
		nextchar();
	}
}

void ofsstr(int *tok4,ITOK *itok4)
{
struct structteg *tteg;
int newreg=(idxregs[4]==255?idxregs[0]:idxregs[4]);	//14.06.06 20:06
int poststr;
int flagstr;
	if(itok4->type==tp_classvar){
		tteg=(structteg *)itok4->rec;
		poststr=0;
		flagstr=f_useidx;
	}
	else{
struct idrec *ptrs;
		ptrs=itok4->rec;
		tteg=(structteg *)ptrs->newid;
		poststr=ptrs->recpost;
		flagstr=ptrs->flag;
	}
	if(tteg==NULL){
		if(displaytokerrors){
			unknowntagstruct(itok4->name);
		}
		return;
	}
	if(strinf.bufstr==NULL&&itok4->post==TRUE)itok4->number+=(tteg->size*(itok4->rm));
	itok4->size=tteg->size;
	itok4->sib=(am32==FALSE?CODE16:CODE32);
	char ocha=cha;
	int binptr=inptr;
	if(poststr==LOCAL){
		itok4->rm=rm_mod10;
		if(strinf.bufstr==NULL){
			if(ESPloc&&am32){
				itok4->rm|=rm_sib;
				itok4->sib=0x24;
			}
			else{
				itok4->rm|=(am32==FALSE?rm_BP:rm_EBP);
			}
		}
		else{
			if(displaytokerrors){
				if((newreg=CheckIDXReg(strinf.bufstr,strinf.size,idxregs[0]))!=NOINREG){
					if(newreg==SKIPREG){
						newreg=idxregs[0];
					}
					else{
						if(am32==FALSE&&newreg!=DI)goto noopt1;
						waralreadinitreg(regs[am32][idxregs[0]],regs[am32][newreg]);
					}
					free(strinf.bufstr);
					strinf.bufstr=NULL;
					goto cont1;
				}
				newreg=idxregs[0];
noopt1:
				CheckMassiv(strinf.bufstr,strinf.size,idxregs[0]);
cont1: ;
			}
			if(am32==FALSE)itok4->rm|=CalcRm16(BP,newreg);
			else{
				itok4->sib=(newreg<<3)+(ESPloc?ESP:EBP);
				itok4->rm=4|rm_mod10;
			}
		}
		itok4->segm=SS;
		if(ESPloc&&am32)itok4->number+=addESP;
//		else itok4->number-=localsize;
		*tok4=tk_rmnumber;
		if(itok4->type==tp_classvar)itok4->flag|=f_useidx;
		itok4->post=FALSE;
		compressoffset(itok4);
	}
	else{
		itok4->post=poststr;
		if(strinf.bufstr==NULL){
			itok4->rm=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
			if(itok4->post!=0)*tok4=tk_postnumber;
			else *tok4=tk_number;
		}
		else{
			if(displaytokerrors){
				if((newreg=CheckIDXReg(strinf.bufstr,strinf.size,idxregs[0]))!=NOINREG){
					if(newreg==SKIPREG){
						newreg=idxregs[0];
					}
					else{
						if(am32==FALSE&&newreg!=DI&&newreg!=BX)goto noopt2;
						waralreadinitreg(regs[am32][idxregs[0]],regs[am32][newreg]);
					}
					free(strinf.bufstr);
					strinf.bufstr=NULL;
					goto cont2;
				}
				newreg=(idxregs[4]==255?idxregs[0]:idxregs[4]);	//14.06.06 20:06
noopt2:
				CheckMassiv(strinf.bufstr,strinf.size,(idxregs[4]==255?idxregs[0]:idxregs[4]));
cont2: ;
			}
			itok4->rm=RegToRM(newreg,(am32==FALSE?tk_reg:tk_reg32))|rm_mod10;
			*tok4=tk_rmnumber;
		}
		itok4->segm=DS;
	}
	if(itok4->post==0&&(itok4->flag&f_reloc)==0){  // cannot compress if POST var
		compressoffset(itok4);
	}
//	compressoffset(itok4);
	itok4->flag|=flagstr;
	inptr=binptr;
	cha=ocha;
}

void AddUndefOff(int segm,char *ostring)	//зафиксировать обращение к еще не объявленым именам
/*segm - сегмент откуда идет обращение
 0 - сегмент кода
 1 - сегмент данных
 2 - сегмент кода, но без занесения в таблицу перемещений
 3 - сегмент данных, но без занесения в таблицу перемещений
*/
{
UNDEFOFF *curptr;
	if(undefoffstart==NULL){	//если еще не было неизв меток
		undefoffstart=(UNDEFOFF *)MALLOC(sizeof(UNDEFOFF));
		memset(undefoffstart,0,sizeof(UNDEFOFF));
		strcpy(undefoffstart->name,ostring);
	}
	for(curptr=undefoffstart;;curptr=curptr->next){
		if(strcmp(curptr->name,ostring)==0){	//ранее уже обращались к ней
			//таблица обращений к undef
			if(curptr->pos==NULL)curptr->pos=(IOFS *)MALLOC(sizeof(IOFS)*(curptr->num+1));
			else curptr->pos=(IOFS *)REALLOC(curptr->pos,sizeof(IOFS)*(curptr->num+1));
			(curptr->pos+curptr->num)->ofs=segm==0?outptr:outptrdata;
			(curptr->pos+curptr->num)->dataseg=(unsigned char)segm;
			(curptr->pos+curptr->num)->line=linenumber;
			(curptr->pos+curptr->num)->file=currentfileinfo;
			curptr->num++;
			return;
		}
		if(curptr->next==NULL)break;	//конец списка
	}
	curptr=curptr->next=(UNDEFOFF *)MALLOC(sizeof(UNDEFOFF));	//новая undef
	memset(curptr,0,sizeof(UNDEFOFF));
	strcpy(curptr->name,ostring);
	curptr->num=1;
	curptr->pos=(IOFS *)MALLOC(sizeof(IOFS));
	curptr->pos->ofs=segm==0?outptr:outptrdata;
	curptr->pos->dataseg=(unsigned char)segm;
	curptr->pos->line=linenumber;
	curptr->pos->file=currentfileinfo;
}

int CheckUseAsUndef(unsigned char *name)
{
UNDEFOFF *curptr;
int count=0;
	if(undefoffstart==NULL)return 0;	//не было обращений к undef
	for(curptr=undefoffstart;;curptr=curptr->next){
		if(strcmp(curptr->name,(char *)name)==0){	//нашли
			count=curptr->num;
			break;
		}
		if(curptr->next==NULL)break;
	}
	return count;
}

int FindOff(unsigned char *name,int base)	//поиск ссылок на текущее имя
{
/*-----------------13.08.00 23:48-------------------
 просмотреть процедуру при вводе разделения данных и кода
	--------------------------------------------------*/
UNDEFOFF *curptr,*prev;
unsigned char segm;
unsigned int ofs,valofs;
int count=0;
	if(undefoffstart==NULL)return 0;	//не было обращений к undef
	for(curptr=undefoffstart;;curptr=curptr->next){
		if(strcmp(curptr->name,(char *)name)==0){	//нашли
			for(int i=0;i<curptr->num;i++){
				ofs=(curptr->pos+i)->ofs;
				segm=(curptr->pos+i)->dataseg;
				if(base==DS&&dynamic_flag){	//было обращение к динамическим иниц. переменным
					CheckPosts();
					(postbuf+posts)->type=(unsigned short)(am32==0?DIN_VAR:DIN_VAR32);
					(postbuf+posts)->num=(int)itok.rec;
					(postbuf+posts)->loc=ofs;
				}
				else{
					if((segm&1)==0||modelmem==TINY){	//в сегменте кода
						if(base!=VARPOST){
//							if(am32==FALSE)valofs=*(unsigned short *)&output[ofs];
//							else valofs=*(unsigned long *)&output[ofs];
//							valofs+=(base==CS?outptr:outptrdata);
							valofs=(base==CS?outptr:outptrdata);
						}
						else valofs=postsize;
						if(am32==FALSE)*(unsigned short *)&output[ofs]+=(unsigned short)valofs;
						else *(unsigned long *)&output[ofs]+=valofs;
					}
					else{
						if(am32==FALSE)*(unsigned short *)&outputdata[ofs]+=(unsigned short)(base==CS?outptr:outptrdata);
						else *(unsigned long *)&outputdata[ofs]+=(unsigned long)(base==CS?outptr:outptrdata);
					}
					if((FixUp!=FALSE&&(segm&2)==0)/*||base==VARPOST*/){
						CheckPosts();
						(postbuf+posts)->type=(unsigned char)
//								(base==VARPOST?(am32==FALSE?POST_VAR:POST_VAR32):
									((segm&1)==0?(am32==FALSE?FIX_VAR:FIX_VAR32):
									(am32==FALSE?FIX_CODE:FIX_CODE32)
//									)
									);
						(postbuf+posts)->loc=ofs;
						posts++;
					}
					if(base==VARPOST){
						unsigned int i;
						for(i=0;i<posts;i++){
							if(ofs==(postbuf+i)->loc)break;
						}
						if(i==posts){
							CheckPosts();
							posts++;
							(postbuf+i)->loc=ofs;
						}
						(postbuf+i)->type=(unsigned char)(am32==FALSE?POST_VAR:POST_VAR32);
					}
				}
			}
			free(curptr->pos);
			if(undefoffstart->next==NULL)undefoffstart=NULL;
			else if(undefoffstart==curptr)undefoffstart=curptr->next;
			else{
				for(prev=undefoffstart;prev->next!=curptr;prev=prev->next);
				prev->next=curptr->next;
			}
			count=curptr->num;
			free(curptr);
			break;
		}
		if(curptr->next==NULL)break;
	}
	return count;
}

int FindUseName(char *name)	//поиск ссылок на текущее имя
{
UNDEFOFF *curptr;
	if(undefoffstart){
		for(curptr=undefoffstart;;curptr=curptr->next){
			if(strcmp(curptr->name,(char *)name)==0){	//нашли
				return curptr->num;
			}
			if(curptr->next==NULL)break;
		}
	}
	return 0;
}

int GetDirective(char *str)
{
	int i;
	i=FastSearch(dirlist,ofsdir,1,str);
//	printf("i=%d %s\n",i,str);
	if(i!=-1&&i<d_end1&&notdoneprestuff!=TRUE&&displaytokerrors!=0){
		char buf[80];
		sprintf(buf,"Too late to change %s",str);
		preerror(buf);
	}
	return i;
}

int FastSearch(unsigned char *list,short *ofs,int type,char *str)
{
	if((strlen(str)-1)>0){
short offs=-1;
unsigned char c;
		c=str[0];
		switch(type){
			case 0:
				if(c>='A'&&c<='Z')offs=ofs[c-'A'];
				break;
			case 1:
				if(c=='D')offs=ofs[0];
				else if(c>='a'&&c<='z')offs=ofs[c-0x60];
				break;
			case 2:
				if(c=='_')offs=ofs[26];
				else if(c>='a'&&c<='z')offs=ofs[c-'a'+27];
				else offs=ofs[c-'A'];
				break;
			case 3:
				if(c>='a'&&c<='z')offs=ofs[c-0x61];
				break;
		}
//					if(type==0)printf("%s\n",str);
		if(offs!=-1){
			for(unsigned char *ii=(unsigned char *)(list+offs);;ii++){
				short types;
				if((types=*(short *)&*ii)==-1)break;
				ii+=2;
				unsigned char c=*ii;
				int i;
				i=1;
				while(c==str[i]){
					if(c==0)return types;
					ii++;
					i++;
					c=*ii;
				}
				if(c>str[i])break;
				while(*ii!=0)ii++;
			}
		}
	}
	return -1;
}

/*-----------------05.01.00 22:56-------------------
 Работа со структурами
	--------------------------------------------------*/

int GetVarSize(int var)
{
	switch(var){
		case tk_reg:
		case tk_intvar:
		case tk_seg:
		case tk_wordvar: return 2;
		case tk_bits:
		case tk_charvar:
		case tk_beg:
		case tk_bytevar: return 1;
		case tk_pointer:
			if(am32==0)return 2;
		case tk_longvar:
		case tk_dwordvar:
		case tk_reg32:
		case tk_floatvar: return 4;
		case tk_doublevar:
		case tk_qwordvar: return 8;
		default:
			if(am32)return 4;
	}
	return 2;
}

unsigned long getsizebit(int size)
{
unsigned long num;
	nexttok();
	if(tok!=tk_number)numexpected();
	num=doconstdwordmath();
	if(num>(unsigned int)(size*8))preerror("Bit field to large");
	return num;
}

int GetFirstDestr(structteg *searcht)
{
elementteg *bazael=searcht->baza;
idrec *ptrs;
	if((searcht->flag&fs_destructor)!=0)return TRUE;
	for(int i=0;i<searcht->numoper;i++){
		ptrs=(bazael+i)->rec;
		if((bazael+i)->tok==tk_baseclass){
			if(GetFirstDestr((structteg *)ptrs))return TRUE;
		}
	}
	return FALSE;
}

void AddTegToTree(structteg *newteg,int Global)
{
struct structteg *tteg;
int i;
	tteg=(Global==TRUE?tegtree:ltegtree);
	if(tteg==NULL)(Global==TRUE?tegtree:ltegtree)=newteg;
	else{
		while(((i=strcmp(tteg->name,newteg->name))<0&&tteg->left!=NULL)||(i>0&&tteg->right!=NULL))
			tteg=(i<0?tteg->left:tteg->right);
		(i<0?tteg->left:tteg->right)=newteg;
	}
}

int IsClass(structteg *searcht)
{
elementteg *bazael=searcht->baza;
	if((searcht->flag&fs_destructor)!=0)return TRUE;
	for(int i=0;i<searcht->numoper;i++){
		if((bazael+i)->tok==tk_baseclass)return TRUE;
//			if(IsClass((structteg *)(bazael+i)->rec))return TRUE;
		else if((bazael+i)->tok==tk_proc){
			if((bazael+i)->rec&&((bazael+i)->rec->flag&f_classproc))return TRUE;
		}
	}
	return FALSE;
}

struct structteg *CreatTeg(int Global,int useunion,int noname)	//создать новый тег
{
struct structteg *newteg,*tteg;
struct elementteg *bazael;
int ssize=0,numel=0,localtok,size,numt,nameid=FALSE,tsize;
int bitofs=0,bitsize=0,i,type;
int isdestr=FALSE,isbase=0;
int unionsize=0;
	newteg=(struct structteg *)MALLOC(sizeof(struct structteg));
	newteg->left=newteg->right=NULL;
	newteg->baza=NULL;
	newteg->flag=useunion;
	newteg->name[0]=0;
	if(tok==tk_ID||tok==tk_id){
		strcpy(newteg->name,itok.name);
		AddTegToTree(newteg,Global);
		nexttok();
		nameid=TRUE;
	}
	if(tok==tk_colon){
		nexttok();
		do{
			if((tteg=FindTeg(TRUE))==NULL)undefclass(itok.name);
			else{
				size=tteg->size;
				if(numel==0)bazael=(struct elementteg *)MALLOC(sizeof(struct elementteg));
				else bazael=(struct elementteg *)REALLOC(bazael,sizeof(struct elementteg)*(numel+1));
				for(i=0;i<numel;i++){
					if(strcmp((bazael+i)->name,itok.name)==0){
						sprintf((char *)string,"Dublicate base class '%s'",itok.name);
						preerror((char *)string);
					}
				}
				strcpy((bazael+numel)->name,itok.name);
				(bazael+numel)->tok=tk_baseclass;
				(bazael+numel)->ofs=ssize;
				(bazael+numel)->numel=1;
				ssize+=size;
				(bazael+numel)->nteg=tteg;
				numel++;
				isbase++;
			}
			nexttok();
			if(tok==tk_openbrace)break;
			expecting(tk_camma);
		}while(tok!=tk_eof);
		if(useunion)preerror("union cannot have a base type");
	}
	expecting(tk_openbrace);
	while(tok!=tk_closebrace&&tok!=tk_eof){
		int oflag,orm,npointr;
		int utestInitVar=FALSE;
		orm=tokens;
		npointr=oflag=0;
		type=variable;
		if(tok==tk_tilda){
			newteg->flag|=fs_destructor;
			oflag|=fs_destructor;
//			oflag=newteg->flag;
			nexttok();
			isdestr=TRUE;
		}
//13.09.04 00:08
		if(tok==tk_static){
			oflag|=f_static;
			nexttok();
		}
/////////////
		switch(tok){
			case tk_int: orm=tok; localtok=tk_intvar; size=2; break;
			case tk_word: orm=tok; localtok=tk_wordvar; size=2; break;
			case tk_char: orm=tok; localtok=tk_charvar; size=1; break;
			case tk_byte: orm=tok; localtok=tk_bytevar; size=1; break;
			case tk_long: orm=tok; localtok=tk_longvar; size=4; break;
			case tk_dword: orm=tok; localtok=tk_dwordvar; size=4; break;
			case tk_float: orm=tok; localtok=tk_floatvar; size=4; break;
			case tk_qword: orm=tok; localtok=tk_qwordvar; size=8; break;
			case tk_double: orm=tok; localtok=tk_doublevar; size=8; break;
			case tk_union:
				nexttok();
				i=FALSE;
				if(bitofs&&(!useunion)){
					ssize+=(bitofs+7)/8;
					bitofs=0;
				}
				if(tok==tk_openbrace)i=TRUE;
				if((tteg=CreatTeg(Global,TRUE,i))!=NULL){
					if(tok==tk_semicolon){
						if(numel==0)bazael=(struct elementteg *)MALLOC(sizeof(struct elementteg)*tteg->numoper);
						else bazael=(struct elementteg *)REALLOC(bazael,sizeof(struct elementteg)*(numel+tteg->numoper));
						for(i=0;i<tteg->numoper;i++)(tteg->baza+i)->ofs+=ssize;
						memcpy((elementteg *)(bazael+numel),tteg->baza,sizeof(struct elementteg)*tteg->numoper);
						tsize=tteg->size;
						if(useunion==FALSE)ssize+=tsize;
						else if(unionsize<(unsigned int)(tsize))unionsize=tsize;
						numel+=tteg->numoper;
						free(tteg->baza);
						free(tteg);
						size=0;
						nexttok();
						break;
					}
					else{
						tsize=size=tteg->size;
						localtok=tk_struct;
						goto dproc2;
					}
				}
				else declareunion();
				break;
			case tk_struct:
				nexttok();
			case tk_id:
			case tk_ID:
				if(bitofs&&(!useunion)){
					ssize+=(bitofs+7)/8;
					bitofs=0;
				}
				i=0;
				if(tok==tk_openbrace||tok2==tk_openbrace){
					if(tok==tk_openbrace)i=TRUE;
					if((tteg=CreatTeg(Global,FALSE,i))!=NULL){
						if(tok!=tk_semicolon){
							tsize=size=tteg->size;
							localtok=tk_struct;
//							printf("tok=%d %s\n",tok,itok.name);
							goto dproc2;
						}
						else{
							if(numel==0)bazael=(struct elementteg *)MALLOC(sizeof(struct elementteg)*tteg->numoper);
							else bazael=(struct elementteg *)REALLOC(bazael,sizeof(struct elementteg)*(numel+tteg->numoper));
							for(i=0;i<tteg->numoper;i++)(tteg->baza+i)->ofs+=ssize;
							memcpy((elementteg *)(bazael+numel),tteg->baza,sizeof(struct elementteg)*tteg->numoper);
							tsize=tteg->size;
							if(useunion==FALSE)ssize+=tsize;
							else if(unionsize<(unsigned int)(tsize))unionsize=tsize;
							numel+=tteg->numoper;
							free(tteg->baza);
							free(tteg);
							size=0;
							nexttok();
							break;
						}
					}
					datatype_expected();
				}
				if(strcmp(itok.name,newteg->name)==0&&tok2==tk_openbracket){
					if(oflag==0){
						newteg->flag|=fs_constructor;
						oflag|=fs_constructor;
						orm=am32==FALSE?tk_word:tk_dword;
					}
					else if(oflag==fs_destructor){
						strcat(itok.name,"~");
						orm=tk_void;
					}
					goto dproc2;
				}
				if((tteg=FindTeg(FALSE))!=NULL||(tteg=FindTeg(TRUE))!=NULL){
					if(tok2==tk_mult){
						while(tok2==tk_mult)nexttok();
						if(am32){
							localtok=tk_dwordvar;
							size=4;
							orm=tk_dword;
						}
						else{
							localtok=tk_wordvar;
							size=2;
							orm=tk_word;
						}
						warpointerstruct();
					}
					else{
						size=tteg->size;
						localtok=tk_struct;
					}
					goto locvar;
	//				break;
				}
			default:
				skipfind=LOCAL;	//запретить поиск в глобальном и локальном списке
				utestInitVar=TRUE;
				if((i=testInitVar())==FALSE||i==2){	//определение процедуры пока не обрабатываем
					skipfind=FALSE;	//разрешить поиск
					FindEndLex();
					datatype_expected();
					nexttok();
					size=0;
					break;
				}
				oflag|=itok.flag;
				npointr=itok.npointr;
				if(itok.npointr)itok.rm=(am32==TRUE?tk_dword:tk_word);
				orm=itok.rm;
				if(tok2==tk_openbracket&&strcmp(itok.name,newteg->name)==0){
					newteg->flag|=fs_constructor;
					oflag|=fs_constructor;
					if(orm==tokens)orm=am32==FALSE?tk_word:tk_dword;
				}
/*				if(tok==tk_openbracket){
					if(itok.npointr)orm=am32==FALSE?tk_word:tk_dword;
					npointr=0;
					nexttok();
					while(tok==tk_mult){	//указатель на процедуру
						npointr++;
						nexttok();
					}
					type=pointer;
				}*/
				goto dproc2;
		}
		if(size!=0){
locvar:
			do{
				tsize=size;
				skipfind=LOCAL;	//запретить поиск в глобальном и локальном списке
				nexttok();
				if(tok==tk_colon){
					numt=getsizebit(size);
					if(numt==0)numt=size*8-bitofs%(size*8);	//(am32==0?16:32)-bitofs%(am32==0?16:32);
					bitofs+=numt;
					continue;
				}
dproc2:
				if(numel==0)bazael=(struct elementteg *)MALLOC(sizeof(struct elementteg));
				else bazael=(struct elementteg *)REALLOC(bazael,sizeof(struct elementteg)*(numel+1));
				if(tok!=tk_ID&&tok!=tk_id){
					utestInitVar=TRUE;
					if(testInitVar()==FALSE){	//определение процедуры пока не обрабатываем
						idalreadydefined();
					}
					else{
						oflag|=itok.flag;
						npointr=itok.npointr;
						if(itok.npointr)itok.rm=(am32==TRUE?tk_dword:tk_word);
						if(itok.rm!=tokens)orm=itok.rm;
						if(tok==tk_openbracket){
							if(itok.npointr)orm=am32==FALSE?tk_word:tk_dword;
							npointr=0;
							nexttok();
							while(tok==tk_mult){	//указатель на процедуру
								npointr++;
								nexttok();
							}
							type=pointer;
						}
						for(i=0;i<numel;i++){
							if(strcmp((bazael+i)->name,itok.name)==0)idalreadydefined();
						}
						strcpy((bazael+numel)->name,itok.name);
						if(tok2==tk_openbracket&&strcmp(itok.name,newteg->name)==0){
							newteg->flag|=fs_constructor;
							oflag|=fs_constructor;
							if(orm==tokens)orm=am32==FALSE?tk_word:tk_dword;
						}
						nexttok();
						if(tok==tk_openbracket||type==pointer)goto dproc;
						if(npointr){
							idrec *nrec;
							nrec=(bazael+numel)->rec=(idrec *)MALLOC(sizeof(idrec));
							strcpy(nrec->recid,(bazael+numel)->name);//скопир название
							nrec->newid=NULL;
							nrec->npointr=(unsigned short)npointr;
							nrec->flag=oflag;
							nrec->line=linenumber;
							nrec->file=currentfileinfo;
					 		nrec->rectok=(bazael+numel)->tok=tk_pointer;
							nrec->type=(unsigned short)orm;
							tsize=2;
							if(am32||(oflag&f_far))tsize=4;
							goto endelteg;
						}
						else unuseableinput();
					}
				}
				else{
					for(i=0;i<numel;i++){
						if(strcmp((bazael+i)->name,itok.name)==0){
							idalreadydefined();
							FindEndLex();
							break;
						}
					}
					strcpy((bazael+numel)->name,itok.name/*(char *)string*/);
					if(tok2==tk_openbracket&&utestInitVar==FALSE){
						if(tok==tk_id)oflag|=(comfile==file_w32?tp_stdcall:tp_pascal);	//тип проц по умолчанию
						else oflag=tp_fastcall;
					}
//					printf("tok=%d %s\n",tok,itok.name);
					nexttok();
					if(tok==tk_colon){
						numt=getsizebit(size);
						if(numt==0)preerror("Bit fields must contain at least one bit");
						(bazael+numel)->ofs=ssize;
						(bazael+numel)->tok=tk_bits;
						(bazael+numel)->bit.siz=numt;
						(bazael+numel)->bit.ofs=bitofs;
						if(useunion==FALSE)bitofs+=numt;
						else if(numt>bitsize)bitsize=numt;
//						printf("n=%d size=%d %s\n",numel,numt,(bazael+numel)->name);
					}
					else if(tok==tk_openbracket){
dproc:
						idrec *nrec;
						param[0]=0;
						if(type==pointer)expecting(tk_closebracket);
						else{
							if(npointr){
								orm=am32==FALSE?tk_word:tk_dword;
								npointr=0;
							}
						}
						skipfind=FALSE;	//разрешить поиск
						expecting(tk_openbracket);
						if((oflag&f_typeproc)==tp_fastcall)declareparamreg();
						else declareparamstack();
						skipfind=LOCAL;
						nrec=(bazael+numel)->rec=(idrec *)MALLOC(sizeof(idrec));
						strcpy(nrec->recid,(bazael+numel)->name);//скопир название
						nrec->newid=NULL;
//						printf("name=%s param=%s\n",nrec->recid,param);
						if(param[0]!=0)nrec->newid=BackString((char *)param);
						if(orm==tokens)orm=am32==FALSE?tk_word:tk_dword;//tk_void;
						nrec->npointr=(unsigned short)npointr;
						nrec->recrm=orm;
						nrec->flag=oflag;
						nrec->line=linenumber;
						nrec->file=currentfileinfo;
						nrec->count=0;
						nrec->recpost=0;//itok.post;
						nrec->recsize=0;//itok.size;
						nrec->recsib=(am32==TRUE?CODE32:CODE16);//itok.sib;
						nrec->sbuf=NULL;
						if(npointr){
					 		nrec->rectok=(bazael+numel)->tok=tk_pointer;
							nrec->type=tk_proc;
							nrec->recsib=am32==FALSE?rm_d16:rm_d32;
							tsize=2;
							if(am32||(oflag&f_far))tsize=4;
							if(bitofs&&(!useunion)){
								ssize+=(bitofs+7)/8;
								bitofs=0;
							}
							tsize=Align(tsize,strpackcur);	//new 16.03.05 14:21
							nrec->recnumber=(bazael+numel)->ofs=ssize;
							if(useunion==FALSE)ssize+=tsize;
							else if(unionsize<(unsigned int)(tsize))unionsize=tsize;
						}
						else{
					 		(bazael+numel)->tok=tk_proc;
							nrec->rectok=tk_declare;
							nrec->type=tp_ucnovn;
							nrec->recsegm=NOT_DYNAMIC;
							(bazael+numel)->ofs=0;
							nrec->recnumber=secondcallnum++;
							nrec->flag|=f_classproc;
						}
						(bazael+numel)->numel=1;
						nexttok();
						if(tok==tk_openbracket)IsUses(nrec);
//						printf("name=%s post=%08X\n",nrec->recid,nrec->recpost);
					}
					else{
						(bazael+numel)->rec=NULL;
						(bazael+numel)->tok=localtok;
endelteg:
						if(bitofs&&(!useunion)){
							ssize+=(bitofs+7)/8;
							bitofs=0;
						}
						(bazael+numel)->ofs=ssize;
						numt=1;
						if(tok==tk_openblock){//[
							skipfind=FALSE;
							nexttok();
							numt=doconstlongmath();
							expecting(tk_closeblock);
							skipfind=LOCAL;
						}
						(bazael+numel)->numel=numt;
//						printf("%d: unionsize=%d elemsize=%d\n",numel,unionsize,tsize*numt);
						if((oflag&f_static)){
							idrec *nrec;
							nrec=(bazael+numel)->rec=(idrec *)MALLOC(sizeof(idrec));
							strcpy(nrec->recid,(bazael+numel)->name);//скопир название
							nrec->line=linenumber;
							nrec->file=currentfileinfo;
							nrec->count=0;
							nrec->sbuf=NULL;
							nrec->recsize=tsize*numt;
							nrec->recsegm=DS;
							if(localtok==tk_struct){
								nrec->recsib=0;
								nrec->newid=(char *)tteg;
								(bazael+numel)->tok=nrec->rectok=tk_structvar;
								nrec->flag=/*flag|*/tteg->flag;
								if(FixUp)nrec->flag|=f_reloc;
								nrec->recrm=numt;
							}
							else{
								nrec->newid=NULL;
								nrec->npointr=(unsigned short)npointr;
								nrec->recrm=(am32==FALSE?rm_d16:rm_d32);
								if(FixUp)oflag|=f_reloc;
								nrec->flag=oflag;
								nrec->recsib=(am32==TRUE?CODE32:CODE16);//itok.sib;
					 			nrec->rectok=localtok;
								nrec->type=tp_ucnovn;
							}
							if(tok==tk_assign){
								skipfind=FALSE;	//разрешить поиск
								nrec->recpost=0;
								if(localtok==tk_struct){
									if(alignword)alignersize+=AlignCD(DS,2);	//выровнять
									nrec->recnumber=outptrdata;	//адрес начала структуры
									i=initstructvar(tteg,numt);
									if(numt==0){
										numt=i/tteg->size;
										if((i%tteg->size)!=0){
											numt++;
											i=tteg->size-(i%tteg->size);
											for(;i!=0;i--)opd(0);
											i=numt*tteg->size;
										}
										nrec->recrm=numel;
										nrec->recsize=i;
									}
									datasize+=i;
								}
								else{
									if(alignword)alignersize+=AlignCD(DS,tsize);
									nrec->recnumber=outptrdata;
									initglobalvar(orm,numt,tsize,variable);
									datasize+=tsize*numt;
								}
							}
							else{
								nrec->recpost=1;
								if(localtok==tk_struct){
									if(numt==0){
										ZeroMassiv();
										break;
									}
								}
								if(alignword){	//выровнять на четный адрес
									if(postsize%2==1)postsize++;
									if(tsize==4&&postsize%4!=0)postsize+=2;
								}
								nrec->recnumber=postsize;
			 					AddPostData(numt*tteg->size);
							}
						}
						else{
							tsize=Align(tsize*numt,strpackcur);	//new 16.03.05 14:21
							if(useunion==FALSE)ssize+=tsize;
							else if(unionsize<(unsigned int)tsize)unionsize=tsize;
							if(localtok==tk_struct)(bazael+numel)->nteg=tteg;
						}
					}
					if(useunion)bitofs=0;
					numel++;
				}
				newteg->size=ssize+unionsize;
			}while(tok==tk_camma);
			skipfind=FALSE;	//разрешить поиск
			seminext();
		}
	};//while(tok!=tk_closebrace&&tok!=tk_eof);
	if(bitofs&&useunion==FALSE)ssize+=(bitofs+7)/8;
	else if(bitsize&&((unsigned int)((bitsize+7)/8)>unionsize))unionsize=(bitsize+7)/8;
//	printf("bitofs=%d size=%d\n",bitofs,ssize);
	if(isdestr==FALSE&&isbase>1){
		char *buf=(char *)MALLOC(16);
		strcpy(buf,"(){");
		int j;
		for(i=0,j=0;i<isbase;i++){
			if(GetFirstDestr((structteg *)(bazael+i)->nteg))j++;
		}
		if(j>1){
			strcat(buf,"};");
int oflag,oinptr2;
char ocha2;
int oldendinptr=endinptr;
char *oinput;
idrec *nrec;
			oinptr2=inptr2;
			ocha2=cha2;
			oinput=(char*)input;
			string[0]=0;
			newteg->flag|=fs_destructor;
			strcpy(itok.name,newteg->name);
			if(CidOrID()==tk_ID)oflag=tp_fastcall;
			else oflag=(comfile==file_w32?tp_stdcall:tp_pascal);
			bazael=(struct elementteg *)REALLOC(bazael,sizeof(struct elementteg)*(numel+1));
			strcpy((bazael+numel)->name,itok.name);
			strcat((bazael+numel)->name,"~");
			itok.rec=nrec=(bazael+numel)->rec=(idrec *)MALLOC(sizeof(idrec));
			strcpy(nrec->recid,itok.name);//скопир название
			nrec->newid=NULL;
			itok.npointr=nrec->npointr=0;
			itok.rm=nrec->recrm=tk_void;
			itok.flag=nrec->flag=fs_destructor|oflag|f_classproc;
			nrec->line=linenumber;
			nrec->file=currentfileinfo;
			nrec->count=0;
			itok.post=nrec->recpost=0;
			itok.size=nrec->recsize=0;
			itok.sib=nrec->recsib=(am32==TRUE?CODE32:CODE16);
			nrec->sbuf=NULL;
	 		(bazael+numel)->tok=tk_proc;
			tok=nrec->rectok=tk_declare;
			itok.type=nrec->type=tp_ucnovn;
			itok.segm=nrec->recsegm=NOT_DYNAMIC;
			(bazael+numel)->ofs=0;
			itok.number=nrec->recnumber=secondcallnum++;
			(bazael+numel)->numel=1;
			numel++;
			input=(unsigned char*)buf;
			inptr2=1;
			cha2='(';
			endinptr=strlen(buf);
			searchteg=newteg;
			dynamic_proc();
			searchteg=NULL;
			input=(unsigned char*)oinput;
			inptr2=oinptr2;
			cha2=ocha2;
			endinptr=oldendinptr;
		}
		free(buf);
	}
	newteg->size=ssize+unionsize;
	newteg->numoper=numel;
	newteg->baza=bazael;
	nexttok();
	if(noname==FALSE&&nameid==FALSE){
		if(tok==tk_ID||tok==tk_id){
			strcpy(newteg->name,itok.name);
			AddTegToTree(newteg,Global);
		}
		else{
			notstructname();
			if(newteg->baza)free(newteg->baza);
			free(newteg);
			newteg=NULL;
		}
	}
	return newteg;
}

struct structteg * FindTeg(int Global,char *name)	//найти тег
{
struct structteg *tteg;
int i;
char *tn;
	if((tn=strchr(name,'@'))!=NULL)*tn=0;
	tteg=(Global==TRUE?tegtree:ltegtree);
	while(tteg&&(i=strcmp(tteg->name,name))!=0)tteg=(i<0?tteg->left:tteg->right);
	return tteg;
}

unsigned int SaveVal(unsigned long long val,int type)
{
unsigned int loop=0;
	switch(type){
		case tk_bytevar:
		case tk_charvar:
			opd(val);
			break;
		case tk_wordvar:
		case tk_intvar:
			outwordd(val);
			loop++;
			break;
		case tk_dwordvar:
		case tk_longvar:
		case tk_floatvar:
			outdwordd(val);
			loop+=3;
			break;
		case tk_qwordvar:
		case tk_doublevar:
			outqwordd(val);
			loop+=7;
			break;
		case tk_pointer:
			if(am32){
				outdwordd(val);
				loop+=3;
			}
			else{
				outwordd(val);
				loop++;
			}
			break;
		case tk_proc:
			loop--;
			break;
		default: internalerror("Bad type variable");
	}
	return loop;
}

void FillTeg(unsigned long long val,unsigned int numel,struct structteg *tteg)
/*-----------------03.10.99 00:20-------------------
 заполнить структуру одинаковыми величинами
 --------------------------------------------------*/
{
struct elementteg *elem=tteg->baza;
int bitofs=0;
unsigned int startstruct;
	if(dbg&2)AddDataLine(1/*,variable*/);
	for(unsigned int j=0;j<numel;j++){
		startstruct=outptrdata;
		for(unsigned int c=0;c<tteg->numoper;c++){
			if((elem+c)->ofs)outptrdata=startstruct+(elem+c)->ofs;
			if(splitdata==FALSE)outptr=outptrdata;
			int type=(elem+c)->tok;
			if(type==tk_bits){
				opb(val,(elem+c)->bit.ofs,(elem+c)->bit.siz);
				bitofs=(elem+c)->bit.ofs+(elem+c)->bit.siz;
			}
			else{
				if(bitofs){
					CorrectOfsBit(bitofs);
					bitofs=0;
				}
				for(unsigned int i=0;i<(elem+c)->numel;i++){
					if(type==tk_struct||type==tk_baseclass){
						FillTeg(val,(elem+c)->numel,(struct structteg *)(elem+c)->nteg);
						break;
					}
					SaveVal(val,type);
				}
			}
		}
	}
	if(bitofs)CorrectOfsBit(bitofs);
}

unsigned int Fill2Teg(unsigned int numel,struct structteg *tteg)
/*-----------------03.10.99 00:20-------------------
 заполнить структуру величинами
 --------------------------------------------------*/
{
unsigned long long hold;
struct elementteg *elem=tteg->baza;
unsigned int tnumel=0;	//номер елемента одного типа
unsigned int ttype=0;	//номер элемента структуры
unsigned int nums=0;	//номер копии структуры
unsigned int loop=0;	//заполненый размер
int type=tokens;
int bitofs=0;
unsigned int startstruct=outptrdata;
	scanalltoks=FALSE;
	for(;;){
		hold=0;
		if(tnumel==(elem+ttype)->numel||type==tk_bits){
			tnumel=0;
			ttype++;
			if(ttype==tteg->numoper){
				ttype=0;
				nums++;
				startstruct=outptrdata;
				if(nums==numel){
					scanalltoks=TRUE;
					return loop;
				}
			}
			loop=(elem+ttype)->ofs;
			outptrdata=startstruct+loop;
			if(splitdata==FALSE)outptr=outptrdata;
			loop+=tteg->size*nums;
		}
		tnumel++;
		type=(elem+ttype)->tok;
//		puts((elem+ttype)->name);
		if(type==tk_struct||type==tk_baseclass){
			tnumel=(elem+ttype)->numel;
			loop+=Fill2Teg((elem+ttype)->numel,(struct structteg *)(elem+ttype)->nteg);
			if(tok==tk_closebrace)break;
			continue;
		}
		if(type==tk_proc)continue;
		if((elem+ttype)->rec&&((elem+ttype)->rec->flag&f_static))continue;
		if(dbg&2)AddDataLine((char)GetVarSize(type)/*,variable*/);
loopsw:
		int htok=tok;
//		printf("tok=%d\n",tok);
		switch(tok){
			case tk_camma:
				hold=0;
				goto strl3;
			case tk_postnumber:
				(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
				itok.flag=0;
//					goto strl1;
			case tk_undefofs:
//					AddUndefOff(1,itok.name);
//strl1:
				tok=tk_number;
				if(htok==tk_undefofs)AddUndefOff(3,itok.name);
				hold+=doconstdwordmath();
				if(tok==tk_plus&&tok2==tk_postnumber&&htok!=tk_undefofs){
					nexttok();
					goto loopsw;
				}
				goto strl2;
			case tk_minus:
			case tk_number:
				if(type==tk_bytevar||type==tk_wordvar||type==tk_dwordvar||type==tk_bits||type==tk_pointer){
					hold=doconstdwordmath();
				}
				else if(type==tk_charvar||type==tk_intvar||type==tk_longvar){
					hold=doconstlongmath();
				}
				else if(type==tk_floatvar)hold=doconstfloatmath();
				else if(type==tk_doublevar)hold=doconstdoublemath();
				else if(type==tk_qwordvar)hold=doconstqwordmath();
//				printf("tok=%d num=%08X\n",tok,hold);
strl2:
				if((postnumflag&f_reloc)!=0)AddReloc();
strl3:
				if(type==tk_bits){
					opb(hold,(elem+ttype)->bit.ofs,(elem+ttype)->bit.siz);
					bitofs=(elem+ttype)->bit.ofs+(elem+ttype)->bit.siz;
				}
				else{
					if(bitofs){
						CorrectOfsBit(bitofs);
						loop+=(bitofs+7)/8;
						bitofs=0;
					}
					loop+=SaveVal(hold,type);
				}
				break;
			case tk_string:
				if(type==tk_bytevar||type==tk_charvar){
					unsigned int i;
					if((unsigned int)itok.number<(elem+ttype)->numel)i=itok.number;
					else i=(elem+ttype)->numel;
					tnumel=0;
					for(;tnumel<i;tnumel++){
						opd(string[tnumel]);
					}
					if(tnumel<(elem+ttype)->numel){
						switch(itok.flag&3){
							case zero_term:
								if(itok.flag&s_unicod)opd(0);
								opd(0);
								tnumel++;
								break;
							case dos_term:
								if(itok.flag&s_unicod)opd(0);
								opd('$');
								tnumel++;
								break;
						}
						while(tnumel<(elem+ttype)->numel){
							opd(aligner);
							tnumel++;
						}
					}
					loop+=tnumel-1;
					nexttok();
					break;
				}
				if(am32==FALSE){
					outwordd(addpoststring());
					if(type==tk_longvar||type==tk_dwordvar){
						loop+=2;
						outword(0);
					}
					loop++;
				}
				else{
					if(type==tk_intvar||type==tk_wordvar)dwordvalexpected();
					outdwordd(addpoststring());
					loop+=3;
				}
				nexttok();
				break;
			default:
				numexpected();
				nexttok();
				break;
		}
		loop++;
		if(tok==tk_closebrace)break;
//		printf("tok=%d\n",tok);
		expecting(tk_camma);
	}
	scanalltoks=TRUE;
	if(bitofs){
		CorrectOfsBit(bitofs);
		loop+=(bitofs+7)/8;
	}
	return loop;
}

unsigned int initstructvar(structteg *tteg,int numel)
{
unsigned int loop=0;
	nexttok();
	switch(tok){	//заполнить величинами
		case tk_minus:
		case tk_number:
			if(numel==0)ZeroMassiv();
			FillTeg(doconstqwordmath(),numel,tteg);
			loop=numel*tteg->size;
			break;
		case tk_from:	//считать файл с данными
			nexttok();
			loop=dofrom();
			for(;loop<tteg->size*numel;loop++)opd(aligner);
			nexttok();
			break;
		case tk_extract:	//считать фрагмент файла с данными
			nexttok();
			loop=doextract();
			for(;loop<tteg->size*numel;loop++)opd(aligner);
			break;
		case tk_openbrace:	//массив данных
			nexttok();
			loop=Fill2Teg(numel,tteg);
			for(;loop<tteg->size*numel;loop++)opd(aligner);
			if(tok!=tk_closebrace){
				preerror("extra parameter at initialization of structure");
				while(tok!=tk_closebrace&&tok!=tk_eof)nexttok();
			}
			nexttok();
			break;
		default:
//			printf("tok=%d\n",tok);
			numexpected(); nexttok(); break;
	}
	return loop;
}

void InitStruct2(unsigned int flag,structteg *tteg)	//инициализировать глобальную структуру
{
struct idrec *newrec=NULL,*ptr;
int numel,count;
char done=0,dynamic;
unsigned int loop;
	while(tok!=tk_eof&&done==0){
		loop=0;
//		printf("3 tok=%d %s\n",tok,itok.name);
		switch(tok){
			case tk_id:
			case tk_ID:	//инициализировать структуру
//выделить память под новую структ
				newrec=(struct idrec *)MALLOC(sizeof(struct idrec));

//				if(strcmp(itok.name,"ccchrg")==0)printf("rec=%08X teg=%08X size=%X %s\n",newrec,tteg,sizeof(idrec),itok.name);

				ptr=((flag&f_static)==0?treestart:staticlist);	//начало дерева
				if(ptr==NULL)((flag&f_static)==0?treestart:staticlist)=newrec;//начало дерева
				else{	//поиск строки в дереве
					while(((numel=strcmp(ptr->recid,itok.name))<0&&ptr->left!=NULL)||(numel>0&&ptr->right!=NULL)){
						ptr=(numel<0?ptr->left:ptr->right);
					}
					(numel<0?ptr->left:ptr->right)=newrec;	//строка меньше
				}
				newrec->recsib=0;
				strcpy(newrec->recid,itok.name);//скопир название
				newrec->newid=(char *)tteg;
				newrec->left=NULL;
				newrec->right=NULL;
				newrec->rectok=tk_structvar;
				newrec->flag=flag|tteg->flag;
				newrec->line=linenumber;
				newrec->file=currentfileinfo;
				newrec->type=tp_gvar;
				if(FixUp)newrec->flag|=f_reloc;
				numel=1;
				nexttok();
				if(tok==tk_openblock){//[
					nexttok();
					if(tok!=tk_closeblock)numel=doconstlongmath();	//число элементов
					else numel=0;
					expecting(tk_closeblock);//]
				}
				newrec->recrm=numel;
				newrec->recsize=numel*tteg->size;
				dynamic=FALSE;
				if(tok==tk_assign||notpost==TRUE){//=
					if(useStartup==TRUE&&tok!=tk_assign&&numel!=0){
						if(SaveStruct(numel*tteg->size,newrec)==TRUE)break;
					}
					if(alignword&&(!dynamic_flag))alignersize+=AlignCD(DS,2);	//выровнять
//					NotPostUnion();
					itok.rec=newrec;
					if((count=FindOff((unsigned char *)newrec->recid,DS))==0){
						if(dynamic_flag)dynamic=DYNAMIC_VAR;
					}
					else if(dynamic_flag)dynamic=USED_DIN_VAR;
					newrec->recnumber=(dynamic==0?outptrdata:0);	//адрес начала структуры
					newrec->recpost=dynamic;
					if(notpost==TRUE&&tok!=tk_assign){
						if(numel==0)ZeroMassiv();
						for(;loop<tteg->size*numel;loop++)opd(aligner);
						datasize+=loop;
						break;
					}
					if(dynamic)newrec->sbuf=dynamic_var();
					else{
						loop=initstructvar(tteg,numel);
						if(numel==0){
							numel=loop/tteg->size;
							if((loop%tteg->size)!=0){
								numel++;
								int i=tteg->size-(loop%tteg->size);
								for(;i!=0;i--)opd(0);
								loop=numel*tteg->size;
							}
							newrec->recrm=numel;
							newrec->recsize=loop;
						}
						datasize+=loop;
					}
				}
				else{
					if(numel==0){
						ZeroMassiv();
						break;
					}
					if(CheckUseAsUndef((unsigned char *)newrec->recid)==0&&dynamic_flag)dynamic=TRUE;
					switch(tok){	//неинициализированные
						default: expected(';');
						case tk_semicolon: done=1;//	;
						case tk_camma:	 //, post global type
//							long longpostsize;
							if(useStartup==TRUE){
								if(SaveStruct(numel*tteg->size,newrec)==TRUE){
									nexttok();
									break;
								}
							}
							newrec->recpost=dynamic+1;
							loop=numel*tteg->size;
							if((flag&f_extern)==0&&dynamic==0){
								if(alignword){	//выровнять на четный адрес
									if(postsize%2==1)postsize++;
								}
								newrec->recnumber=postsize;
							}
							else newrec->recnumber=externnum++;
							count=FindOff((unsigned char *)newrec->recid,VARPOST);
							if((flag&f_extern)==0&&dynamic==0)
									/*-----------------10.09.02 23:21-------------------
									 этот вызов должен быть после FindOff
										--------------------------------------------------*/
									AddPostData(loop);
							nexttok();
							break;
					}
				}
				newrec->count=count;
				break;
			case tk_proc:
			case tk_qwordvar:
			case tk_doublevar:
			case tk_floatvar:
			case tk_dwordvar:
			case tk_longvar:
			case tk_charvar:
			case tk_intvar:
			case tk_bytevar:
			case tk_wordvar: idalreadydefined(); nexttok(); break;
			default:
				if(newrec)expected(';');
				else errstruct();
				FindStopTok();
			case tk_semicolon: done=1;
			case tk_camma: nexttok(); break;
		}
	}
	dopoststrings();
}

void InitStruct()	//инициализировать глобальную структуру
{
struct structteg *tteg;
unsigned int flag;
//if(debug)puts("start init struct");
	flag=itok.flag;
	if(fstatic){
		flag|=f_static;
		fstatic=FALSE;
	}
	if(tok==tk_struct)nexttok();
	else if(tok2!=tk_id&&tok2!=tk_ID){
		if(tok2==tk_dblcolon){
			itok.flag=f_classproc;
			itok.rm=am32==FALSE?tk_word:tk_dword;//tk_void;
			itok.npointr=0;
			doclassproc(0/*comfile==file_w32?tk_stdcall:tk_pascal 18.10.05 10:52*/);
			return;
		}
		notstructname();
	}
	tteg=FindTeg(TRUE);
	if(tteg==NULL){
		if(tok==tk_openbrace||tok2==tk_openbrace||tok==tk_colon||tok2==tk_colon)
				tteg=CreatTeg(TRUE);	//найти или создать тег
		else{
			while(tok!=tk_semicolon&&tok!=tk_eof)nexttok();
			tegnotfound();
			return;
		}
	}
	else{
		if(tok2==tk_openbrace)idalreadydefined();
		nexttok();
	}
	InitStruct2(flag,tteg);
}

unsigned long LocalStruct2(int flag,int *localline,int binptr,char bcha,structteg *tteg)	//инициализировать локальную структуру
{
int numel,first=FALSE;
struct localrec *newrec;
unsigned long size=0;
	skipfind=TRUE;	//запретить искать в глобальном дереве
	do{
		if(first!=FALSE){
			binptr=inptr2;
			bcha=cha2;
			nexttok();
		}
		first=TRUE;
		if(tok!=tk_ID&&tok!=tk_id)idalreadydefined();
		else{	//инициализировать структуру
			numel=1;
			newrec=addlocalvar((char *)string,tk_structvar,localsize);
			newrec->rec.newid=(char *)tteg;
			newrec->rec.flag=tteg->flag;
			newrec->rec.type=tp_localvar;
			nexttok();
			if(tok==tk_openblock){//[
				skipfind=FALSE;
				nexttok();
				numel=doconstlongmath();	//число элементов
				skipfind=TRUE;
				expecting(tk_closeblock);//]
			}
			newrec->rec.recrm=numel;
			size=numel*tteg->size;
			newrec->rec.recsize=size;
			if(flag&f_static){
//				if(bcha==0)not_union_static();
				if(tok==tk_assign){
//					newrec->rec.rectok=tk_structvar;
					newrec->rec.recnumber=0;
					newrec->rec.recpost=DYNAMIC_VAR;
//					newrec->rec.recsize=tteg->size;
//					newrec->rec.recrm=numel;
					newrec->rec.line=linenumber;
					newrec->rec.file=currentfileinfo;
					newrec->rec.npointr=0;
					newrec->rec.sbuf=dynamic_var();
					newrec->rec.recsib=newrec->rec.type=tp_gvar;
				}
				else{
					newrec->rec.recpost=TRUE;
					if(alignword){	//выровнять на четный адрес
						if(postsize%2==1)postsize++;
					}
					newrec->rec.recnumber=postsize;
					AddPostData(size);
				}
				size=0;
			}
			else{
				size=Align(size,(am32==FALSE?2:4));
				newrec->rec.recpost=LOCAL;
				newrec->rec.recnumber=-localsize-size;
//				localsize+=size;
				if(tok==tk_assign){
					if(*localline==0)*localline=linenumber;
					AddBackBuf(binptr,bcha);
				}
			}
		}
		localsize+=size;
	}while(tok==tk_camma);
	skipfind=FALSE;	//запретить искать в глобальном дереве
//	localsize+=size;
	itok.name[0]=0;
	seminext();
	return size;
}

unsigned long LocalStruct(int flag,int *localline)	//инициализировать локальную структуру
{
struct structteg *tteg;
int binptr;
char bcha;
structteg *osearchteg;
	osearchteg=searchteg;
	searchteg=NULL;
	skipfind=TRUE;	//запретить искать в глобальном дереве
	if(tok==tk_struct)nexttok();
	binptr=inptr2;
	bcha=cha2;
	if((tteg=FindTeg(FALSE))==NULL&&(tteg=FindTeg(TRUE))==NULL){
		skipfind=FALSE;
		if(tok==tk_openbrace||tok2==tk_openbrace){
			tteg=CreatTeg(FALSE);	//найти или создать тег
		}
		else{
			while(tok!=tk_semicolon&&tok!=tk_eof)nexttok();
			tegnotfound();
			searchteg=osearchteg;
			return 0;
		}
	}
	else{
		searchteg=osearchteg;
		if(tok2==tk_openbrace){
			idalreadydefined();
			SkipBlock2();
			return 0;
		}
		nexttok();
		if(tok!=tk_id&&tok!=tk_ID){
			notstructname();
			FindStopTok();
			return 0;
		}
	}
	searchteg=osearchteg;
	return LocalStruct2(flag,localline,binptr,bcha,tteg);
}

void dostruct()
{
struct structteg *tteg;
int numel;
int usenumstr;
SINFO rstr;
char *ofsst=NULL;
int oneloop=FALSE;
unsigned long hnum;
unsigned int num,sized=0,sign=FALSE;
unsigned long adr;
int localstr=FALSE;
int poststr;
int flagstr;
struct idrec *ptrs;
	if(tok2==tk_assign){
		switch(ScanTok3()){
			case tk_proc:
			case tk_apiproc:
			case tk_undefproc:
			case tk_declare:
				ptrs=itok.rec;
				if(ptrs->recpost==LOCAL){
					if(ESPloc&&am32){
						itok.rm=rm_mod10|rm_sib;
						itok.sib=0x24;
						itok.number+=addESP;
					}
					else{
						itok.rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
					}
					itok.segm=SS;
					itok.post=0;
					compressoffset(&itok);
				}
				else{
					itok.segm=DS;
					itok.rm=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
					itok.post=ptrs->recpost;
				}
				itok.sib=(am32==FALSE?CODE16:CODE32);
				itok.flag=ptrs->flag;
				switch(itok.size){
					case 1:
						tok=tk_bytevar;
						dobytevar(0);
						break;
					case 2:
					case 3:
						tok=tk_wordvar;
						do_d_wordvar(0,r16);
						break;
					case 4:
					case 5:
					case 6:
					case 7:
						tok=tk_dwordvar;
						do_d_wordvar(0,r32);
						break;
					default:
						tok=tk_qwordvar;
						doqwordvar();
						break;
				}
				return;
		}
	}
	adr=itok.number;
	numel=itok.rm;	//число структур
	usenumstr=itok.post;	//было указание номера структуры
	if(itok.type==tp_classvar){
		tteg=(structteg *)itok.rec;
		poststr=0;
		flagstr=f_useidx;
	}
	else{
		ptrs=itok.rec;
		tteg=(structteg *)ptrs->newid;
		if(ptrs->recpost==LOCAL)localstr=TRUE;
		poststr=ptrs->recpost;
		flagstr=ptrs->flag;
	}
	rstr=strinf;
	strinf.bufstr=NULL;
	num=itok.size;
//	if(bufrm)printf("num=%d %s\n",adr,bufrm);
	nexttok();
	if(tok==tk_assign){
		getoperand();
int starts=0;	//смещение заполнения в структуре
		if(tok==tk_int||tok==tk_word){
			sized=2;
			getoperand();
		}
		else if(tok==tk_long||tok==tk_dword){
			sized=4;
			getoperand();
		}
		else if(tok==tk_char||tok==tk_byte){
			sized=1;
			getoperand();
		}
		switch(tok){
			case tk_charvar:
			case tk_bytevar:
			case tk_intvar:
			case tk_wordvar:
			case tk_longvar:
			case tk_dwordvar:
				int razr,retreg;
				ITOK wtok;
				char *wbuf;
				wbuf=bufrm;
				bufrm=NULL;
				wtok=itok;
				SINFO wstr;
				wstr=strinf;
				strinf.bufstr=NULL;
				if((retreg=CheckIDZReg(itok.name,AX,razr=GetVarSize(tok)))!=NOINREG){
					GenRegToReg(AX,retreg,razr);
					IDZToReg(itok.name,AX,razr);
					if(bufrm){
						free(bufrm);
						bufrm=NULL;
					}
					if(strinf.bufstr){
						free(strinf.bufstr);
						strinf.bufstr=NULL;
					}
					if(sized){
						switch(razr){
							case r8: tok=tk_beg; break;
							case r16: tok=tk_reg; break;
							case r32: tok=tk_reg32; break;
						}
						itok.number=0;
						goto convr;
					}
					break;
				}
				IDZToReg(itok.name,AX,razr);
				if(sized==0){
					switch(tok){
						case tk_charvar:
						case tk_bytevar:
							getintoal(tok,&wtok,wbuf,&wstr);
							sized=1;
							break;
						case tk_intvar:
							sign=TRUE;
						case tk_wordvar:
							getinto_e_ax(sign,tok,&wtok,wbuf,&wstr,r16);
							sized=2;
							break;
						case tk_longvar:
							sign=TRUE;
						case tk_dwordvar:
							getinto_e_ax(sign,tok,&wtok,wbuf,&wstr,r32);
							sized=4;
							break;
					}
				}
				else{
convr:
					switch(sized){
						case 1:
							getintoal(tok,&wtok,wbuf,&wstr);
							break;
						case 2:
							getinto_e_ax(0,tok,&wtok,wbuf,&wstr,r16);
							break;
						case 4:
							getinto_e_ax(0,tok,&wtok,wbuf,&wstr,r32);
							break;
					}
				}
				if(usenumstr!=FALSE){
//					num/=ptrs->recrm;	//указан номер структуры - значит не все
					num=tteg->size;
					if(strinf.bufstr==NULL)starts=num*numel;
				}
				num/=sized;
				goto fillstr;
			case tk_minus:
				nexttok();
				if(tok!=tk_number){
					errstruct();
					break;
				}
				itok.number=-itok.number;
			case tk_number:
				if(sized==0){
					sized=4;
					if(itok.number<65536)sized=2;
					if(itok.number<256)sized=1;
				}
				if(usenumstr!=FALSE){
//					num/=ptrs->recrm;	//указан номер структуры - значит не все
					num=tteg->size;
					if(strinf.bufstr==NULL)starts=num*numel;
				}
				if(strinf.bufstr==NULL){
					if(optimizespeed==TRUE){
						if(sized==1&&(num%2)==0){
							sized=2;
							itok.number=(itok.number&255)|((itok.number<<8)&0xFF00);
						}
						if(chip>2&&sized==2&&(num%4)==0){
							sized=4;
							itok.number=(itok.number&0xFFFF)|(itok.number<<16);
						}
					}
					else{
						if(am32==FALSE){
							if(sized==1&&((num<7&&(num%2)==0)||(itok.number==0&&num==8))){
								sized=2;
								itok.number=(itok.number&255)|((itok.number<<8)&0xFF00);
							}
						}
						else if(chip>2&&sized!=4&&(num%4)==0&&itok.number==0)sized=4;
					}
				}
				num/=sized;
				if(num==1){
					oneloop++;
					hnum=itok.number;
				}
				else if(num==2&&sized==2&&chip>2){
					oneloop++;
					hnum=(itok.number&0xFFFF)|(itok.number<<16);
					sized=4;
					num=1;
				}
				else if(num==4&&sized==1&&chip>2){
					oneloop++;
					itok.number&=255;
					hnum=itok.number|(itok.number<<8)|(itok.number<<16)|(itok.number<<24);
					sized=4;
					num=1;
				}
				else{
					if(sized==1){
						if(itok.number==0)outword(0x00B0);
						else{
							op(0xB0);
							op(itok.number);
						}
						ConstToReg(itok.number,AX,r8);
					}
					else MovRegNum(sized,0,itok.number,EAX);
				}
fillstr:
//				itok.number=ptrs->recnumber;
				itok.number=adr;
				if(rstr.bufstr==NULL)itok.number+=starts;
				itok.sib=(am32==FALSE?CODE16:CODE32);
				if(localstr){
					if(comfile!=file_w32&&oneloop==FALSE){
						pushss();
						popes();
					}
					if(ESPloc&&am32){
						itok.rm=rm_mod10|rm_sib;
						itok.sib=0x24;
						itok.number+=addESP;
					}
					else{
						itok.rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
					}
					itok.segm=SS;
					tok=tk_rmnumber;
					itok.post=0;
					itok.size=tteg->size;
					compressoffset(&itok);
					if(rstr.bufstr!=NULL)CheckAllMassiv(bufrm,2,&rstr,&itok,EDI,EDX);
				}
				else{
					if(comfile!=file_w32&&oneloop==FALSE){
						pushds();
						popes();
					}
					itok.post=poststr;
					itok.segm=DS;
					itok.rm=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
					itok.rec=NULL;//ptrs;
					if(rstr.bufstr==NULL){
						if(itok.post!=0)tok=tk_postnumber;
						else tok=tk_number;
					}
					else{
//						printf("bufrm=%s rstr=%s\n",bufrm,rstr.bufstr);
						CheckAllMassiv(bufrm,2,&rstr,&itok,EDI,EDX);
//						puts("end rm");
						tok=tk_rmnumber;
						itok.size=tteg->size;		// ???? trap
					}
				}
				itok.flag=flagstr;
				if(oneloop){
					switch(sized){
						case 1:
							outseg(&itok,2);
							op(0xC6);
							op(itok.rm);
							outaddress(&itok);
							op(hnum);
							break;
						case 2:
						case 4:
							op66(sized);
							if(hnum==0){
								outseg(&itok,2);
								op(0x83);
								op(itok.rm+0x20);
								outaddress(&itok);
								op(0);
								break;
							}
							if((sized==4&&hnum==0xFFFFFFFF)||(sized==2&&hnum==0xFFFF)){
								outseg(&itok,2);
								op(0x83);
								op(itok.rm+0x8);
								outaddress(&itok);
								op(0xFF);
								break;
							}
							if(optimizespeed==0&&sized==4&&short_ok(hnum,TRUE)){
								op(0x6A);
								op(hnum);	//push short number
								op66(sized);
								outseg(&itok,2);
								op(0x8f);
								op(itok.rm);
								outaddress(&itok);
								break;
							}
							outseg(&itok,2);
							op(0xC7);	//mov word[],number
							op(itok.rm);
							outaddress(&itok);
							if(sized==2)outword(hnum);
							else outdword(hnum);
							break;
					}
				}
				else{
					warningreg(regs[am32][DI]);
					getintoreg_32(DI,(am32+1)*2,0,&ofsst);
					if(num<=2||(num<=4&&(!optimizespeed))){
						for(unsigned int i=0;i<num;i++){
							if(sized==1)stosb();
							else if(sized==2)stosw();
							else stosd();
						}
					}
					else{
						MovRegNum((am32+1)*2,0,num,ECX);
						warningreg(regs[am32][CX]);
						ConstToReg(0,ECX,(am32+1)*2);
						op(0xF3);  // REPZ
						if(sized==1)stosb();
						else if(sized==2)stosw();
						else stosd();
					}
				}
				nexttok();
				break;
			case tk_structvar:
struct structteg *tteg2;
int numel2;
int usenumstr2;
unsigned int num2;
int localstr2;
int poststr2;
int flagstr2;
//	adr=itok.number;

				localstr2=FALSE;
				numel2=itok.rm;	//число структур
				usenumstr2=itok.post;	//было указание номера структуры
				if(itok.type==tp_classvar){
					tteg2=(structteg *)itok.rec;
					poststr2=0;
					flagstr2=f_useidx;
				}
				else{
struct idrec *ptrs;
					ptrs=itok.rec;
					tteg2=(structteg *)ptrs->newid;
					if(ptrs->recpost==LOCAL)localstr2=TRUE;
					poststr2=ptrs->recpost;
					flagstr2=ptrs->flag;
				}
				num2=itok.size;
//				ptrs2=itok.rec;
//				tteg2=(structteg *)ptrs2->newid;
//				numel2=itok.rm;	//число структур
//				num2=tteg2->size;
				if(usenumstr2!=FALSE){
					num2=tteg->size;
					if(strinf.bufstr==NULL)starts=num2*numel2;
				}
//				if(itok.post==FALSE)num2*=ptrs2->recrm;	//не указан номер структуры - значит все
//				else if(strinf.bufstr==NULL)starts=num2*numel2;
				if(strinf.bufstr==NULL)itok.number+=starts;
				itok.sib=(am32==FALSE?CODE16:CODE32);
				if(localstr2){
					itok.rm=rm_mod10;
					if(strinf.bufstr==NULL){
						if(ESPloc&&am32){
							itok.rm|=rm_sib;
							itok.sib=0x24;
						}
						else{
							itok.rm|=(am32==FALSE?rm_BP:rm_EBP);
						}
					}
					else{
						CheckAllMassiv(bufrm,2,&strinf);
						if(am32==FALSE)itok.rm|=rm_BPSI;
						else{
							itok.sib=(ESPloc?0x34:0x35);
							itok.rm=4|rm_mod10;
						}
					}
					itok.segm=SS;
					if(ESPloc&&am32)itok.number+=addESP;
//					else itok.number-=localsize;
					tok=tk_rmnumber;
					itok.size=tteg2->size;
					itok.post=0;
					compressoffset(&itok);
				}
				else{
					itok.post=poststr2;
					itok.rm=(am32==FALSE?rm_d16:rm_d32);	//22.11.04 09:22//установки по умолчанию
					if(strinf.bufstr==NULL){
						if(itok.post==TRUE)tok=tk_postnumber;
						else tok=tk_number;
					}
					else{
						CheckAllMassiv(bufrm,2,&strinf);
						itok.rm=rm_mod10|(am32==FALSE?rm_SI:6);
						tok=tk_rmnumber;
						itok.size=tteg2->size;
					}
					itok.segm=DS;
				}
///!new
				itok.flag=flagstr2;
				warningreg(regs[am32][SI]);
///!new
//				printf("tok=%d rm=%08X %s",tok,itok.rm,itok.name);
				getintoreg_32(SI,(am32+1)*2,0,&ofsst);
				ClearReg(SI);
				num=tteg->size;
				starts=0;
				if(usenumstr==FALSE)num*=tteg->size;	//ptrs->recrm;	//не указан номер структуры - значит все
				else if(rstr.bufstr==NULL)starts=num*numel;
				itok.number=adr;//ptrs->recnumber;
				if(rstr.bufstr==NULL)itok.number+=starts;
				itok.sib=(am32==FALSE?CODE16:CODE32);
				if(localstr){
//				if(ptrs->recpost==LOCAL){
					if(ESPloc&&am32){
						itok.rm=rm_mod10|rm_sib;
						itok.sib=0x24;
						itok.number+=addESP;
					}
					else{
						itok.rm=rm_mod10|(am32==FALSE?rm_BP:rm_EBP);
					}
					itok.segm=SS;
					tok=tk_rmnumber;
					itok.size=tteg->size;
					itok.post=0;
					compressoffset(&itok);
					if(rstr.bufstr!=NULL)CheckAllMassiv(bufrm,2,&rstr,&itok,EDI,EDX);
				}
				else{
					itok.post=poststr;//ptrs->recpost;
					itok.segm=DS;
					itok.rm=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
					if(rstr.bufstr==NULL){
						if(itok.post==TRUE)tok=tk_postnumber;
						else tok=tk_number;
					}
					else{
						CheckAllMassiv(bufrm,2,&rstr,&itok,EDI,EDX);
						tok=tk_rmnumber;
						itok.size=tteg->size;
					}
				}
///!new
				itok.flag=flagstr;//ptrs->flag;
				warningreg(regs[am32][DI]);
				if(num2>num)warnsize();
				ClearReg(DI);
///!new
				usenumstr2=DS;
				if(poststr2==LOCAL)usenumstr2=SS;
				getintoreg_32(DI,(am32+1)*2,0,&ofsst);
				num=num2;
				sized=1;
				if(optimizespeed){
					if((num%2)==0)sized=2;
					if(chip>2&&(num%4)==0)sized=4;
				}
				else{
					if(am32==FALSE){
						if((num%2)==0)sized=2;
					}
					else{
						if(chip>2&&(num%4)==0)sized=4;
					}
				}
				num/=sized;
///!new
				if(num<=2||(num<=4&&(!optimizespeed&&usenumstr2!=SS))){
					for(unsigned int i=0;i<num;i++){
						if(usenumstr2==SS)op(0x36);
						if(sized==1)movsb();
						else if(sized==2)movsw();
						else movsd();
					}
				}
				else{
///!new
					MovRegNum((am32+1)*2,0,num,ECX);
					warningreg(regs[am32][CX]);
					ConstToReg(0,ECX,(am32+1)*2);
					if(comfile!=file_w32){
						if(poststr)pushss();
//						if(ptrs->recpost==LOCAL)pushss();
						else pushds();
						popes();
					}
					op(0xF3);  // REPZ
					if(usenumstr2==SS)op(0x36);
					if(sized==1)movsb();
					else if(sized==2)movsw();
					else movsd();
				}
				break;
			default: errstruct();
		}
	}
	else{
		nexttok();
		errstruct();
	}
	if(ofsst)free(ofsst);
}

int SaveStruct(int size,idrec *newrec)
{
int i=0;
	if((startStartup+size)<endStartup){
		if(alignword){	//выровнять на четный адрес
			if(startStartup%2==1)i=1;
		}
		if((startStartup+size+i)<=endStartup){
			startStartup+=i;
			newrec->recnumber=startStartup;	//адрес начала структуры
			newrec->recpost=FALSE;
			startStartup+=size;
			return TRUE;
		}
	}
	return FALSE;
}

int CheckAsmName(char *name)
{
char buf[IDLENGTH];
	strcpy(buf,name);
	strupr(buf);
	return FastSearch((unsigned char *)asmMnem,ofsmnem,0,buf);
}

void FastTok(int mode,int *tok4,ITOK *itok4)
{
int useme;
int next=1;
	*tok4=tokens;
	itok4->type=tp_ucnovn;
	whitespace(); //пропуск незначащих символов
	if(isalpha(cha)||(cha=='_')||cha>=0x80){	//идентификатор
		if(mode==1){
			int i=0;
			do{
				itok4->name[i++]=cha;
				nextchar();
			}while(CheckChar2()==TRUE&&i<IDLENGTH);
			if(i==IDLENGTH)i--;
			itok4->name[i]=0;
		}
		while(CheckChar2()==TRUE)nextchar();	//дочитать слово
		*tok4=tk_id;
		return;
	}
	if(isdigit(cha)){//числа
		if(mode==1){
			inptr--;
			itok4->lnumber=scannumber(&itok4->rm);
		}
		else{
			do{
				nextchar();
			}while(isdigit(cha));
		}
		*tok4=tk_number;
		return;
	}
	else switch(cha){
		case '\"':
			nextchar();	//строковая константа
			while(cha!='\"'&&!endoffile){
				convert_char();
				nextchar();
			}
			*tok4=tk_string;
			break;
		case '\'': //символьная константа может иметь более 1 символа
			nextchar();
			while(cha!='\''&&!endoffile){  // special character
				convert_char();
				nextchar();
			}
			break;
		case '/': nextchar();
			switch(cha){
				case '*': nextchar(); //соментарий
					useme=1;
					if(mode==2)itok4->number=inptr-2;
					while(!endoffile&&useme>0){
						whitespace();
						if(cha=='*'){
							nextchar();
							if(cha=='/')useme--;
							else continue;
						}
						else if(cha=='/'){
							nextchar();
							if(cha=='*')useme++;
							else continue;
						}
						nextchar();
					}
					if(mode==2)*tok4=tk_comment2;
					else FastTok(mode,tok4,itok4);
					if(endoffile)*tok4=tk_eof;
					return;
				case '/':
					if(mode==2)itok4->number=inptr-2;
					do{
						nextchar();
					}while(!endoffile&&cha!=13);	//строка коментария
					if(endoffile)*tok4=tk_eof;
					if(mode==2)*tok4=tk_comment1;
					else FastTok(mode,tok4,itok4);
					return;
			}
			break;
		case '#':
		case '?':
			nextchar();
//			tokscan(&tok,&itok,string);
			FastTok(1,tok4,itok4);
//			if((itok.number=GetDirective((char *)string))!=-1){
			if(*tok4==tk_id&&(itok4->number=GetDirective(itok4->name))!=-1){
				*tok4=tk_question;
			}
			return;
		case ';': *tok4=tk_semicolon;	break;
		case '[': *tok4=tk_openblock; break;
		case ']': *tok4=tk_closeblock; break;
		case '(': *tok4=tk_openbracket; break;
		case ')': *tok4=tk_closebracket; break;
		case '{': *tok4=tk_openbrace; break;
		case '}': *tok4=tk_closebrace; break;
		case ',': *tok4=tk_camma; break;
		case ':': *tok4=tk_colon; break;
		case 26: *tok4=tk_eof; return;
		case 13: *tok4=tk_endline; break;
		case '\\':
			nextchar();
			if(cha==13){
				FastTok(mode,tok4,itok4);
				if(tok==tk_endline)FastTok(mode,tok4,itok4);
				return;
			}
			break;
		case '!':
			nextchar();
			if(cha=='='){
				*tok4=tk_notequal;		//!=
				itok4->type=tp_compare;
			}
			else{
				*tok4=tk_not;
				next=0;
			}
			break;
		case '=':
			nextchar();
			if(cha=='='){
				*tok4=tk_equalto;  //==
				itok4->type=tp_compare;
			}
			else{
				*tok4=tk_assign;						 //присвоить
				next=0;
			}
			break;
		case '>':
			nextchar();
			switch(cha){
				case '>':
					nextchar();
					if(cha=='=')*tok4=tk_rrequals; //сдвиг вправо с присвоением
					else{
						*tok4=tk_rr;							//сдвиг вправо
						next=0;
						itok4->type=tp_opperand;
					}
					break;
				case '<': *tok4=tk_swap; break; 			 //обмен
				case '=': *tok4=tk_greaterequal; itok4->type=tp_compare; break; //больше или равно
				default: *tok4=tk_greater; next=0; itok4->type=tp_compare; break; //больше
			}
			break;
		case '<':
			nextchar();
			switch(cha){
				case '<':
					nextchar();
					if(cha=='=')*tok4=tk_llequals;	 //сдвиг влево с присвоением
					else{
						*tok4=tk_ll;								 //сдвиг влево
						next=0;
						itok4->type=tp_opperand;
					}
					break;
				case '>': *tok4=tk_notequal; itok4->type=tp_compare; break;  //!=
				case '=': *tok4=tk_lessequal; itok4->type=tp_compare; break; //меньше или равно
				default: *tok4=tk_less; next=0; itok4->type=tp_compare; break;//меньше
			}
			break;
	}
	if(next)nextchar();
}

void FindDirectiv()
//ускоренный поиск директивы
{
	inptr=inptr2;
	cha=cha2;
	linenumber=linenum2;
	do{
		FastTok(0);
	}while(tok!=tk_question&&tok!=tk_eof);
	inptr2=inptr;
	cha2=cha;
	linenum2=linenumber;
}

unsigned long long scannumber(int *rm)
{
int useme=0,binnum=0;
unsigned char c;
unsigned long long number=0;
	*rm=tokens;
	for(int i=inptr;;i++){
		c=input[i];
		unsigned char cc=c&(unsigned char)0x5f;
		if(cc>='A'&&cc<='F'){
			if(cc=='B'&&binnum==1)binnum++;
			else binnum=3;
			useme=3;
		}
		else if(c<'0'||c>'9'){
			if(useme==0&&c=='.'&&input[i+1]!='.'){	//float
				binnum=3;
				useme++;
				*rm=tk_double;
				*(double *)&number=atof((char *)&input[inptr]);
//				printf("float=%f %08X\n",*(float *)&number,*(long *)&number);
				inptr=i+1;
				do{
					nextchar();
				}while(isdigit(cha));
				if((cha&0x5f)!='E')break;
				nextchar();
				if(isdigit(cha)||cha=='-'){
					do{
						nextchar();
					}while(isdigit(cha));
				}
				break;
			}
			else if(cc=='H'){	//hex
				useme=2;
				binnum=3;
				break;
			}
			break;
		}
		else if(c<'2'&&(binnum==0||binnum==2))binnum++;
	}
	if(binnum==2)goto cbinnum;
	if(useme!=1){
		nextchar();
		if(useme==2)goto hexnum;
		if(cha=='0'){
			nextchar();
			switch(cha&0x5f){
				case 'X':	 // hexadecimal number
					nextchar();
hexnum:
					while(isxdigit(cha)){
						number*=16;
						if(isdigit(cha))number+=cha-'0';
						else number+=(cha&0x5f)-'7';
						nextchar();
					}
					if(useme==2)nextchar();
					useme++;
					break;
				case 'B': 	// binary number
cbinnum:
					nextchar();
					while(cha=='0'||cha=='1'){
						number=number*2+(cha-'0');
						nextchar();
					}
					if(binnum==2)nextchar();
					else useme++;
					break;
				case 'O': 	// octal number
					nextchar();
					while(cha>='0'&&cha<='7'){
						number=number*8+(cha-'0');
						nextchar();
					}
					useme++;
					break;
			}
		}
		if(useme==0||useme==3){ 	// decimal number
			while(isdigit(cha)){
				number=number*10+(cha-'0');
				nextchar();
			}
		}
	}
	c=(unsigned char)(cha&0x5F);
	if(c=='I'){
		if(input[inptr]=='6'&&input[inptr+1]=='4'){
			inptr+=2;
			nextchar();
			*rm=tk_qword;
		}
	}
	else if(c=='L'||c=='U'||c=='F'||c=='Q'){
		if(c=='L')*rm=tk_dword;
		if(c=='F'){
			*rm=tk_float;
			*(float *)&number=*(double *)&number;
		}
		if(c=='Q')*rm=tk_qword;
		nextchar();
		c=(unsigned char)(cha&0x5F);
		if(c=='L'||c=='U'){
			if(c=='L')*rm=tk_dword;
			nextchar();
		}
	}
	if(*rm==tokens){
		if(number<256)*rm=tk_byte;
		else if(number<65536)*rm=tk_word;
		else if(number<0x100000000LL)*rm=tk_dword;
		else *rm=tk_qword;
	}
	return number;
}

void tag_massiv(int *tok4,ITOK *itok4,unsigned char *string4)
{
struct structteg *tteg;
int number,tok,rm;
	tok=*tok4;
	number=itok4->number;
	nextchar();
//14.09.04 18:52
	do{
		FastTok(1,tok4,itok4);
		strcpy((char *)string4,itok4->name);
		searchtree2(definestart,itok4,tok4,string4);
	}while(*tok4==tk_endline);
///////////////////
//	printf("tok=%d %s\n",*tok4,itok4->name);
	char *tn;
	if((tn=strchr(itok4->name,'@'))!=NULL)*tn=0;
	if((tteg=FindTeg(FALSE,itok4->name))!=NULL||(tteg=FindTeg(TRUE,itok4->name))!=NULL){
//	printf("tok=%d %s\n",*tok4,itok4->name);
		itok4->number=0;
		dostructvar2(tok4,itok4,tteg,string4);
		rm=RegToRM(number,tok);
//	printf("tok=%d rm=%d %s\n",*tok4,itok4->rm,itok4->name);
		if(*tok4==tk_pointer&&itok4->type==tk_proc){
			itok4->sib=rm|rm_mod10;
			itok4->flag&=~f_reloc;
		}
		else if(*tok4!=tk_proc&&*tok4!=tk_declare/*&&(itok4->flag&f_static)==0 эффект добавления регристра*/){
			itok4->rm=rm_mod10|rm;
			itok4->flag&=~f_reloc;
		}
		if(*tok4==tk_proc||*tok4==tk_declare||*tok4==tk_undefproc){
			structadr.sib=THIS_REG;
			structadr.rm=number;
		}
		else if(*tok4==tk_id||*tok4==tk_ID||*tok4==tk_structvar){
			*tok4=tk_rmnumber;
			itok4->segm=(splitdata==FALSE)?DS:CS;
			itok4->post=0;
		}
		if(bufrm&&strcmp(bufrm,"&this;")==0){
			free(bufrm);
			bufrm=NULL;
		}
//	printf("tok=%d rm=%d post=%d %s\n",*tok4,itok4->rm,itok4->post,itok4->name);
//	if(strinf.bufstr)puts(strinf.bufstr);
	}
	else if(displaytokerrors){
		unknowntagstruct(itok4->name);
		*tok4=tk_number;
	}
}

int RegToRM(int number,int tok4)
{
int rm;
	if(displaytokerrors){
		if(am32==FALSE&&tok4==tk_reg32)regBXDISIBPexpected();
		else if(am32&&tok4==tk_reg)reg32expected();
	}
	if(am32==FALSE){
		switch(number){
			case BX:
				rm=rm_BX;
				break;
			case DI:
				rm=rm_DI;
				break;
			case SI:
				rm=rm_SI;
				break;
			case BP:
				rm=rm_BP;
				break;
			default:
				if(displaytokerrors)regBXDISIBPexpected();
				break;
		}
	}
	else rm=number;
	return rm;
}

int CheckResName(char *name)
{
char buf[IDLENGTH];
	strcpy(buf,name);
	strlwr(buf);
	return FastSearch((unsigned char *)resMnem,ofsres,3,buf);
}

int GetTokVar2()
{
ITOK cstok;
int i;
unsigned char ocha=cha;
unsigned int oinptr=inptr;
int oline=linenumber;
char odisplay=displaytokerrors;
	displaytokerrors=0;
	tokscan(&i,&cstok,string2);
	displaytokerrors=odisplay;
	switch(i){
		case tk_number:
			cha=ocha;
			inptr=oinptr;
			linenumber=oline;
		case tk_int:
		case tk_char:
		case tk_long:
		case tk_short:
		case tk_byte:
		case tk_word:
		case tk_dword:
		case tk_signed:
		case tk_unsigned:
		return i;
	}
	cha=ocha;
	inptr=oinptr;
	linenumber=oline;
	return 0;
}

void GetTypeVar(int *tok4)
{
static int use=0;
int i;
	if(use)return;
	use=1;
	i=GetTokVar2();
	if(*tok4==tk_signed){
		if(i==tk_char||i==tk_int||i==tk_long){
			*tok4=i;
			i=GetTokVar2();
			if(*tok4==tk_long){
				if(i==tk_long)*tok4=tk_qword;
				else if(i!=tk_int&&i!=0&&displaytokerrors)unknowntype();
			}
			if(*tok4==tk_int&&am32)*tok4=tk_long;
		}
		else if(i==tk_short){
			i=GetTokVar2();
			*tok4=tk_int;
			if(i!=0&&i!=tk_int&&displaytokerrors)unknowntype();
		}
		else if(i==0)*tok4=(am32==0?tk_int:tk_long);
		else if(displaytokerrors)unknowntype();
	}
	else if(*tok4==tk_unsigned){
		switch(i){
			case tk_char:
				*tok4=tk_byte;
				break;
			case tk_int:
				*tok4=(am32==0?tk_word:tk_dword);
				break;
			case tk_long:
				*tok4=tk_dword;
				i=GetTokVar2();
				if(i==tk_long)*tok4=tk_qword;
				else if(i!=tk_int&&i!=0&&displaytokerrors)unknowntype();
				break;
			case tk_short:
				*tok4=tk_word;
				i=GetTokVar2();
				if(i!=0&&i!=tk_int&&displaytokerrors)unknowntype();
				break;
			case 0:
				*tok4=(am32==0?tk_word:tk_dword);
				break;
			default:
				if(displaytokerrors)unknowntype();
		}
	}
	else if(i==tk_int&&(*tok4==tk_long||*tok4==tk_short))*tok4=i;
	else if(*tok4==tk_short)*tok4=tk_int;
	else if(*tok4==tk_int&&(!(idasm==TRUE&&i==tk_number))&&am32)*tok4=tk_long;
	else if(i==tk_long&& *tok4==tk_long)*tok4=tk_qword;
	use=0;
}

int CheckDef()
{
int otok;
	if(skipfind!=LOCAL)return FALSE;
	switch(tok2){
		case tk_semicolon:
		case tk_assign:
		case tk_openblock:
		case tk_openbracket:
		case tk_camma:
		case tk_openbrace:
			return FALSE;
	}
	otok=tok;
	searchtree2(definestart,&itok,&tok,string);
	if(tok==otok)return FALSE;
	return TRUE;
}

/* end of TOKA.C */
