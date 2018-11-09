#define _TOKB_
#include "tok.h"

char badadr[]="Bad outrm value in outaddress();";
char CXandDX[]="CX and DX";
char ECXandEDX[]="ECX and EDX";
char BXandFS[]="BX and FS";
int divexpand=FALSE;
int optnumber=FALSE;
ITOK itok,itok2,ptok;
char *pbuf;
SINFO pstr;

LISTFLOAT *floatnum=NULL;	//список float констант
unsigned int numfloatconst=0;
unsigned int maxnumfloatconst;
#define STEPFLOATCONST 16
unsigned int ofsfloatlist=0;
unsigned char idxregs[5]={ESI,EDI,EBX,EDX,255};

int expandvar();
int speedmul(unsigned long num, int razr);
int leamul32(unsigned long num,int reg,int razr);
void Float2reg32(int reg,int addop=0,int reg1=idxregs[0],int reg2=idxregs[1]);
void NegReg(int razr,int reg);
int RshiftReg(int razr,int reg,int sign);
void do_e_axmath2(int sign,int razr,int expand);
int MulReg(int reg,int razr);
void DivMod(int vop,int sign,int razr,int expand);
void DivNum2(unsigned long num,int razr,int sign);
void DivNum(unsigned long num,int razr,int sign);
void ClearDX(int razr,int sign);
void  doalmath2(int sign);
void num2bits(ITOK *gtok,unsigned long num,int razr);
void reg2bits(ITOK *gtok,int razr);
void cwpointr(ITOK *wtok,char *&wbuf,SINFO *wstr,int *otok,int npointr,int ureg);
int CheckAddOnly();
void optnumadd64(unsigned long long num,int r1,int r2,int vop);
void CallExternProc(char *name);
void getinto_reg(int gtok,ITOK *gstok,char *&gbuf,SINFO *gstr,int razr,int reg);
void intinstack(int addop);

extern void warningoptnum();
extern void segoperror();
extern void segbyteerror();
extern void regmathoperror();
extern void begmathoperror();
extern void negregerror();
extern void regbyteerror();
extern void begworderror();
extern void regshifterror();
extern void regmatherror();
extern void DevideZero();
extern void wordnotoper();

unsigned long long li[]={0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,0x400,
0x800,0x1000,0x2000,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000,0x100000,
0x200000,0x400000,0x800000,0x1000000,0x2000000,0x4000000,0x8000000,0x10000000,
0x20000000,0x40000000,0x80000000,0x100000000LL,0x200000000LL,0x400000000LL
,0x800000000LL,0x1000000000LL,0x2000000000LL,0x4000000000LL,0x8000000000LL
,0x10000000000LL,0x20000000000LL,0x40000000000LL,0x80000000000LL
,0x100000000000LL,0x200000000000LL,0x400000000000LL,0x800000000000LL
,0x1000000000000LL,0x2000000000000LL,0x4000000000000LL,0x8000000000000LL
,0x10000000000000LL,0x20000000000000LL,0x40000000000000LL,0x80000000000000LL
,0x100000000000000LL,0x200000000000000LL,0x400000000000000LL,0x800000000000000LL
,0x1000000000000000LL,0x2000000000000000LL,0x4000000000000000LL,0x8000000000000000LL};

unsigned long leanum[24]={3,5,9,15,25,27,45,81,75,125,135,225,243,405,729,
		375,675,1215,2187,625,1125,2025,3645,6561};

unsigned int numleamul[24][4]={
	{3,0,0,0},{5,0,0,0},{9,0,0,0},{3,5,0,0},{5,5,0,0},{3,9,0,0},{5,9,0,0},
	{9,9,0,0},{5,5,3,0},{5,5,5,0},{9,5,3,0},{9,5,5,0},{9,9,3,0},{9,9,5,0},
	{9,9,9,0},{5,5,5,3},{5,5,9,3},{9,9,5,3},{9,9,9,3},{5,5,5,5},{9,5,5,5},
	{9,9,5,5},{9,9,9,5},{9,9,9,9}};

int RmEqualReg(int reg,int rm,int sib)
{
int rm0;
int reg1=-1,reg2=-1;
	rm0=rm&0x7;
	if(sib==CODE16){
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
				reg1=SI;
				break;
			case 5:
				reg1=DI;
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
		}
		else{
			reg1=rm0;
			if(reg1==5&&(rm&0xc0)==0)reg1=-1;
		}
	}
	if(reg==reg1||reg==reg2)return TRUE;
	return FALSE;
}

void startmul(int razr)
{
	if(razr==r8)outword(0xE08A);	//mov ah,al
	else{
		op66(razr);
		outword(0xD08B);	//mov dx,ax
		warningreg(regs[razr/2-1][2]);
	}
}

void lshiftmul(int num,int razr,int reg=AX)
{
	if(razr==r8){
		if(num==1)outword(0xC002);	//add al,al
		else{
			if(chip==0){
				op(0xB1);	//mov cl,num
				op(num);
				outword(0xE0D2);	//shl al,cl
				warningreg(begs[1]);
			}
			else{
				outword(0xE0C0);	//shl al,num
				op(num);
			}
		}
	}
	else{
		if(num==1){
			op66(razr);
			op(1);
			op(0xC0+reg*9);	//add reg,reg
		}
		else{
			if(chip==0){
				op(0xB1);	//mov cl,num
				op(num);
				op(0xD3);
				op(0xE0+reg);	//shl ax,cl
				warningreg(begs[1]);
			}
			else{
				if(chip>2&&optimizespeed&&leamul32(li[num],reg,razr))return;
				op66(razr);
				op(0xC1);
				op(0xE0+reg);	//shl ax,num
				op(num);
			}
		}
	}
}

void submul(int razr)
{
	if(razr==r8)outword(0xC42A);	//sub al,ah
	else{
		op66(razr);
		outword(0xC22B);	//sub ax,dx
	}
}

void addmul(int razr)
{
	if(razr==r8)outword(0xC402);	//add al,ah
	else{
		op66(razr);
		outword(0xC203);	//add ax,dx
	}
}

int leamul32(unsigned long num,int reg,int razr)
{
	if(num==0)return FALSE;
int vop=0,i=8*reg+4;
	switch(num){
		case 9: vop+=0x40;
		case 5: vop+=0x40;
		case 3: vop+=0x40;
			op66(razr);
			op67(r32);	// AX * n = LEA AX,[EAX*n+?EAX]
			op(0x8d);
			op(reg==BP?i|rm_mod01:i);
			op(vop+9*reg);
			if(reg==BP)op(0);
			return TRUE;
	}
//			if(chip>3&&chip<7)return FALSE;	//избежать AGI для 486,p5,p5mmx
	switch(num){
		case 2: vop=reg; break;
		case 4: vop=0x85; break;
		case 8: vop=0xc5; break;
		default: return FALSE;
	}
	op66(razr);
	op67(r32);	// AX * n = LEA AX,[EAX*n+?EAX]
	op(0x8d);
	if(vop!=0x85&&vop!=0xc5&&reg==BP)i|=rm_mod01;
	op(i);
	op(vop+8*reg);
	if(vop==0x85||vop==0xc5)outdword(0L);
	else if(reg==BP)op(0);
	return TRUE;
}

int speedmul32(unsigned long num,int reg,int razr)
{
int i;
	for(i=3;i<24;i++){
		if(leanum[i]==num){
			leamul32(numleamul[i][0],reg,razr);
			leamul32(numleamul[i][1],reg,razr);
			leamul32(numleamul[i][2],reg,razr);
			leamul32(numleamul[i][3],reg,razr);
			return TRUE;
		}
	}
	for(unsigned int j=1;j<NUMNUM;j++){
		unsigned long v=li[j];
		if((v*3)>num)break;
		for(i=0;i<15;i++){
			unsigned long lm=leanum[i]*v;
			if(lm==num){
				leamul32(numleamul[i][0],reg,razr);
				lshiftmul(j,razr,reg);
				leamul32(numleamul[i][1],reg,razr);
				leamul32(numleamul[i][2],reg,razr);
				return TRUE;
			}
		}
	}
	if(reg==EAX)return speedmul(num,r32);
	return FALSE;
}

int speedmul(unsigned long num, int razr)	//поиск возможности замены умножения на
//сдвиги и сложения
{
int first,second;
	for(unsigned int j=1;j<NUMNUM;j++){
		unsigned long v=li[j];
		if((v-1)>num)break;
		if((num%(v-1))==0){
			second=caselong(num/(v-1));
			if(second!=NUMNUM){
				first=caselong(v);
				if(first!=1){
					startmul(razr);
					lshiftmul(first,razr);
					submul(razr);
					if(second!=0)lshiftmul(second,razr);
					setzeroflag=TRUE;
					return TRUE;
				}
			}
		}
		if((v+1)>num)break;
		if((num%(v+1))==0){
			second=caselong(num/(v+1));
			if(second!=NUMNUM){
				first=caselong(v);
				startmul(razr);
				lshiftmul(first,razr);
				addmul(razr);
				if(second!=0)lshiftmul(second,razr);
				setzeroflag=TRUE;
				return TRUE;
			}
		}
		if(num>10){
			if((v-1)>(num-1))break;
			if(((num-1)%(v-1))==0){
				second=caselong((num-1)/(v-1));
				if(second!=NUMNUM&&second!=1){
					first=caselong(v);
					if(first!=1){
						startmul(razr);
						lshiftmul(first,razr);
						submul(razr);
						lshiftmul(second,razr);
						addmul(razr);
						setzeroflag=TRUE;
						return TRUE;
					}
				}
			}
			if((v+1)>(num-1))break;
			if(((num-1)%(v+1))==0){
				second=caselong((num-1)/(v+1));
				if(second!=NUMNUM){
					first=caselong(v);
					startmul(razr);
					lshiftmul(first,razr);
					addmul(razr);
					lshiftmul(second,razr);
					addmul(razr);
					setzeroflag=TRUE;
					return TRUE;
				}
			}
			if((v-1)>(num+1))break;
			if(((num+1)%(v-1))==0){
				second=caselong((num+1)/(v-1));
				if(second!=NUMNUM){
					first=caselong(v);
					if(first!=1){
						startmul(razr);
						lshiftmul(first,razr);
						submul(razr);
						lshiftmul(second,razr);
						submul(razr);
						setzeroflag=TRUE;
						return TRUE;
					}
				}
			}
			if((v+1)>(num+1))break;
			if(((num+1)%(v+1))==0){
				second=caselong((num+1)/(v+1));
				if(second!=NUMNUM&&second!=1){
					first=caselong(v);
					startmul(razr);
					lshiftmul(first,razr);
					addmul(razr);
					lshiftmul(second,razr);
					submul(razr);
					setzeroflag=TRUE;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void CheckRegForLea(int reg,int *idx,int *base, int *zoom,unsigned long *val,
	unsigned int *rflag,ITOK *posttok)
{
//	printf("in:reg=%02X base=%02X idx=%02X zoom=%02X\n",reg,*base,*idx,*zoom);
	for(;;){
//		printf("tok=%d flag=%08X %s\n",tok2,itok2.flag,itok2.name);
//		printf("tok=%d tok2=%d %s\n",tok,tok2,itok2.name);
		if((tok==tk_plus||tok==tk_minus)&&(tok2==tk_minus||
				(tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_double)||
				(tok2==tk_postnumber&&posttok->post==0)||(tok==tk_plus&&tok2==tk_rmnumber&&(itok2.flag&f_useidx)==0))){
			goto con1;
		}
		if((tok!=tk_plus&&(!(tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_double)))
				||(tok2!=tk_reg32&&tok2!=tk_number)||tok2==tk_rmnumber){
			break;
		}
		if(tok==tk_plus&&tok2==tk_reg32&&(itok2.number==reg||(*idx!=-1&&*base!=-1))){
			break;
		}
		if(tok==tk_mult&&(tok2!=tk_number||*zoom!=-1||*val!=0||*idx!=-1))break;
con1:
		if(tok==tk_minus){
			if(tok2==tk_postnumber||(tok2==tk_number&&(itok2.flag&f_reloc)))break;
			nexttok();
			if(tok==tk_minus){
				if(tok2!=tk_number)break;
				nexttok();
				*val+=itok.number;
			}
			else *val-=itok.number;
			if(tok==tk_number)*rflag^=itok.flag;
			else *posttok=itok;
		}
		else if(tok==tk_plus){
			nexttok();
			if(tok==tk_number){
				*val+=itok.number;
				*rflag^=itok.flag;
			}
			else if(tok==tk_postnumber){
				*val+=itok.number;
				*posttok=itok;
			}
			else if(tok==tk_rmnumber){
				int r1,r2,z;
				ExpandRm(itok.rm,itok.sib,&z,&r1,&r2);
				if(z&&*zoom>0)break;
				if(r1>=0&&r2>=0&&(*idx>=0||*base>=0))break;
				*val+=itok.number;
				if(*zoom<=0)*zoom=z;
				if(*base==-1){
					if(r1>=0){
						*base=r1;
						r1=-1;
					}
					else{
						*base=r2;
						r2=-1;
					}
				}
				if(*idx==-1){
					if(r1>=0)*idx=r1;
					else if(r2>=0)*idx=r2;
				}
			}
			else{
				if(*base==-1)*base=itok.number;
				else *idx=itok.number;
			}
		}
		else if(tok==tk_mult){
			*zoom=0;
			switch(itok2.number){
				case 8: *zoom=*zoom+1;
				case 4: *zoom=*zoom+1;
				case 2: *zoom=*zoom+1;
				case 1:
					*idx=*base;
					*base=-1;
					break;
				case 9: *zoom=*zoom+1;
				case 5: *zoom=*zoom+1;
				case 3:
					*zoom=*zoom+1;
					*idx=*base;
					break;
				default: *zoom=-1;
			}
			if(*zoom==-1)break;
			nexttok();
		}
		else break;
		nexttok();
		if(*base!=-1&&*idx!=-1&&tok2!=tk_number)break;
	}
	if(*zoom==-1&&*base==-1){
		*base=*idx;
		*idx=-1;
	}
//	printf("out:reg=%02X base=%02X idx=%02X zoom=%02X\n",reg,*base,*idx,*zoom);
}

int OutLea(int reg,int idx,int base, int zoom,unsigned long val,
	unsigned int rflag,ITOK *posttok)
{
int mod=rm_mod00;
int q;
	if(zoom==-1)zoom=0;
	if(val!=0){
		if(short_ok(val,TRUE)!=0)mod=rm_mod01;
		else mod=rm_mod10;
	}
	if((rflag&f_reloc)!=0||posttok->post)mod=rm_mod10;
 	if(idx==-1){
		idx=base;
		base=-1;
	}
	if(base==-1){
		if(zoom==0){
			op66(r32);
			op67(r32);
			op(0x8d);
			if(idx==5&&mod==rm_mod00)mod=rm_mod01;
			op((reg*8+idx)|mod);
			if(idx==4)op(idx*9);
		}
		else{
			if(idx==4)return FALSE;
			mod=rm_mod10;
			op66(r32);
			op67(r32);
			op(0x8d);
			op(reg*8+4);
			op(idx*8+5+(zoom<<6));
		}
	}
	else{
		if(idx==4){
			if(base==-1){ idx=-1; base=4;}
			else if(zoom==0){
				q=base;
				base=idx;
				idx=q;
			}
			else return FALSE;
		}
		if(base==5&&mod==rm_mod00){
			if(idx!=-1&&idx!=5){
				if(zoom==0){
					q=base;
					base=idx;
					idx=q;
				}
				else mod=rm_mod01;
			}
			else mod=rm_mod01;
		}
		op66(r32);
		op67(r32);
		op(0x8d);
		op((reg*8+4)|mod);	//sib
		op((zoom<<6)+(idx<<3)+base);
	}
	if(mod==rm_mod01)op(val);
	else if(mod==rm_mod10){
		if(posttok->post)setwordpost(posttok);
		else if((rflag&f_reloc)!=0)AddReloc();
		outdword(val);
	}
	else if(mod==rm_mod00&&base==5)outdword(val);
	return TRUE;
}

int Reg32ToLea2(int reg)	//оптимизация сложения 32-битных регистров в LEA
{
int idx,base,zoom;
unsigned long val;
int otok,otok2,otype2;
unsigned char ocha,next;
unsigned int oinptr,rflag;
int oline;
ITOK oitok;
ITOK posttok;
	if(tok!=tk_minus&&tok!=tk_plus&&tok!=tk_mult)return FALSE;
	if(tok==tk_minus&&(!((tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_double)||tok2==tk_postnumber)))return FALSE;
	if(tok==tk_mult&&(!(tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_double&&(itok2.flag&f_reloc)==0)))return FALSE;
	if(tok==tk_plus&&tok2!=tk_reg32&&tok2!=tk_number&&tok2!=tk_postnumber&&(!(tok2==tk_rmnumber&&(itok2.flag&f_useidx)==0)))return FALSE;
	if(cur_mod)return FALSE;
	posttok.post=0;
	idx=zoom=-1;
	base=reg;
	if(tok==tk_mult){
		zoom=0;
		switch(itok2.number){
			case 8:	zoom++;
			case 4:	zoom++;
			case 2:	zoom++;
			case 1:
				idx=reg;
				base=-1;
				break;
//new!
			case 9:	zoom++;
			case 5:	zoom++;
			case 3:
//				if(!(am32==FALSE&&tok!=tk_plus&&(!(tok2==tk_minus||tok2==tk_number)))){
				if(am32){
					zoom++;
					idx=base=reg;
					break;
				}
// end new!!
			default: return FALSE;
		}
		if(zoom==0){
			zoom=-1;
			base=reg;
			idx=-1;
		}
	}
	val=0;
	rflag=0;
	oinptr=inptr2;
	ocha=cha2;
	otok=tok;
	oitok=itok;
	otok2=tok2;
	otype2=itok2.type;
	oline=linenumber;
	if(tok==tk_mult){
		nexttok();
		nexttok();
	}
	CheckRegForLea(reg,&idx,&base,&zoom,&val,&rflag,&posttok);
	next=0;
	if(idx==-1)next=1;
	if(base==-1)next|=2;
	if(zoom==-1)next|=4;
	if(val==0&&rflag==0&&posttok.post==0)next|=8;
//	printf("next=%d\n",next);
	switch(next){
		case 1:		// idx=-1                  Rb+N
			if(base==ESP&&val==1)goto retfalse;
		case 4: 	// zoom=-1                 Rb+Ri+N
			if(val<2||val>127)goto retfalse;
			break;
		case 2:		// base=-1                 Ri*Z+N
			if(zoom<3){
				if(zoom==2&&val>2){
					if(val<128)goto retfalse;
					break;
				}
				goto retfalse;
			}
		case 0:
		case 8:		// val=0                   Rb+Ri*Z
			break;
		default:
//		case 12:	// val=0   zoom=-1         Rb+Ri
//		case 10:	// val=0   base=-1         Ri*Z
//		case 13:	// val=0   idx=-1  zoom=-1 Rb
//		case 5:		// idx=-1  zoom=-1         Rb+N
//		case 3:		// idx=-1  base=-1         N
//		case 6:		// base=-1 zoom=-1         Ri+N
//		case 11:	// val=0   base=-1 idx=-1  -
//		case 9: 	// val=0   idx=-1          Rb
//		case 14:	// val=0   base=-1 zoom=-1 Ri
//		case 7: 	// idx=-1  base=-1 zoom=-1 N
//		case 15:	// val=0   base=-1 idx=-1  zoom=-1
retfalse:
			inptr2=oinptr;
			cha2=ocha;
			tok=otok;
			itok=oitok;
			tok2=otok2;
			itok2.type=(unsigned short)otype2;
			if(bufrm){
				free(bufrm);
				bufrm=NULL;
			}
			if(strinf.bufstr){
				free(strinf.bufstr);
				strinf.bufstr=NULL;
			}
			endoffile=0;
			linenum2=linenumber=oline;
			return FALSE;
	}
	if(OutLea(reg,idx,base,zoom,val,rflag,&posttok)==FALSE)goto retfalse;
	ClearReg(reg);
//	printf("%s (%u) Combination %d\n",(startfileinfo+currentfileinfo)->filename,linenumber,next);
	return TRUE;
}

int RegEqualToLea(int reg)
{
int idx,base,zoom;
unsigned long val;
int otok,otok2;
unsigned char ocha,next;
unsigned int oinptr,rflag;
ITOK oitok;
ITOK oitok2;
ITOK posttok;
	if(cur_mod)return FALSE;
	oinptr=inptr2;
	ocha=cha2;
	otok=tok;
	oitok=itok;
	otok2=tok2;
	oitok2=oitok;
	base=idx=zoom=-1;
	val=0;
	rflag=0;
	posttok.post=0;
	tok=tk_plus;
	CheckRegForLea(reg,&idx,&base,&zoom,&val,&rflag,&posttok);
	if(tok!=tk_semicolon)goto retfalse;
	if(base==-1)base=reg;
	else if(idx==-1)idx=reg;
	else goto retfalse;
	next=0;
	if(idx==-1)next=1;
	if(base==-1)next|=2;
	if(zoom==-1)next|=4;
	if(val==0&&rflag==0&&posttok.post==0)next|=8;
//	printf("base=%d idx=%d zoom=%d val=%d next=%d\n",base,idx,zoom,val,next);
	if(val==0&&rflag==0&&posttok.post==0)next|=8;
	switch(next){
		case 5:		// idx=-1  zoom=-1         Rb+N
			if(reg==EAX&&(val>127||val<0xffffff80))goto retfalse;
			if(val<3||val>0xfffffffd)goto retfalse;
			if(base==ESP)goto retfalse;
			break;
		case 4: 	// zoom=-1                 Rb+Ri+N
			if(val==1||val==0xffffffff)goto retfalse;
			break;
		case 0:
		case 8:		// val=0                   Rb+Ri*Z
			break;
		default:
//		case 13:	// val=0   idx=-1  zoom=-1 Rb
//		case 3:		// idx=-1  base=-1         N
//		case 6:		// base=-1 zoom=-1         Ri+N
//		case 11:	// val=0   base=-1 idx=-1  -
//		case 9: 	// val=0   idx=-1          Rb
//		case 14:	// val=0   base=-1 zoom=-1 Ri
//		case 7: 	// idx=-1  base=-1 zoom=-1 N
//		case 15:	// val=0   base=-1 idx=-1  zoom=-1
retfalse:
			inptr2=oinptr;
			cha2=ocha;
			tok=otok;
			itok=oitok;
			tok2=otok2;
			itok2=oitok2;
			endoffile=0;
			if(bufrm){
				free(bufrm);
				bufrm=NULL;
			}
			if(strinf.bufstr){
				free(strinf.bufstr);
				strinf.bufstr=NULL;
			}
//	printf("return input=%08X inptr=%08X\n",input,inptr2);
			return FALSE;
	}
//	printf("next=%d\n",next);
	if(OutLea(reg,idx,base,zoom,val,rflag,&posttok)==FALSE)goto retfalse;
	return TRUE;
}

int Reg32ToLea(int reg)	//оптимизация сложения 32-битных регистров в LEA
{
int idx,base,zoom;
unsigned long val;
int otok,otok2,otype2;
unsigned char ocha,next;
unsigned int oinptr,rflag;
int oline;
ITOK oitok;
ITOK posttok;
	if(tok!=tk_reg32&&tok!=tk_number&&tok!=tk_postnumber&&(!(tok==tk_rmnumber&&(itok.flag&f_useidx)==0)))return FALSE;
	if(cur_mod)return FALSE;
	posttok.post=0;
	idx=base=zoom=-1;
	val=0;
	rflag=0;
//	printf("input=%08X inptr=%08X\n",input,inptr2);
	oinptr=inptr2;
	ocha=cha2;
	otok=tok;
	oitok=itok;
	otok2=tok2;
	otype2=itok2.type;
	oline=linenumber;
//	printf("tok=%d type=%d %s\n",tok,itok.type,itok.name);
	if(tok==tk_number){
		if(itok.rm==tk_float||itok.rm==tk_double||itok.rm==tk_qword)return FALSE;
		val=doconstdwordmath();
		rflag=postnumflag;
		if((rflag&f_reloc)&&tok==tk_mult)goto retfalse;
	}
	else if(tok==tk_postnumber){
		posttok=itok;
		tok=tk_number;
		val=doconstdwordmath();
		if(tok==tk_mult)goto retfalse;
	}
	else if(tok==tk_rmnumber){
		ExpandRm(itok.rm,itok.sib,&zoom,&base,&idx);
		val=itok.number;
		nexttok();
	}
	else{
		base=itok.number;
		nexttok();
	}
	if(tok==tk_mult){
		nexttok();
		if(base==-1&&tok==tk_reg32){
			if(itok.number==reg)goto retfalse;
			idx=itok.number;
		}
		else if(base!=-1&&tok==tk_number){
			if((itok.flag&f_reloc)&&(itok.rm==tk_float||itok.rm==tk_double||itok.rm==tk_qword))goto retfalse;
			idx=base;
			base=-1;
			val=itok.number;
		}
		else goto retfalse;
		nexttok();
		zoom=0;
		switch(val){
			case 8: zoom++;
			case 4: zoom++;
			case 2: zoom++;
			case 1: break;
//new!
			case 9: zoom++;
			case 5: zoom++;
			case 3:
//				if(!(am32==FALSE&&tok!=tk_plus&&(!(tok2==tk_minus||tok2==tk_number)))){
				if(am32||((tok==tk_plus||tok==tk_minus)&&(tok2==tk_number||tok2==tk_minus))){
					zoom++;
					base=idx;
					break;
				}
// end new!!
			default:
				goto retfalse;
		}
		if(zoom==0){
			zoom=-1;
			base=idx;
			idx=-1;
		}
		val=0;
	}
	CheckRegForLea(reg,&idx,&base,&zoom,&val,&rflag,&posttok);
	next=0;
	if(idx==-1)next=1;
	if(base==-1)next|=2;
	if(zoom==-1)next|=4;
	if(val==0&&rflag==0&&posttok.post==0)next|=8;
	switch(next){
		case 5:		// idx=-1  zoom=-1         Rb+N
		case 1:		// idx=-1                  Rb+N
			if(base==ESP&&val==1)goto retfalse;
		case 12:	// val=0   zoom=-1         Rb+Ri
			if(base==reg)goto retfalse;
			break;
		case 10:	// val=0   base=-1         Ri*Z
			if(optimizespeed==FALSE||idx==reg)goto retfalse;
			break;
		case 2:		// base=-1                 Ri*Z+N
			if(optimizespeed)break;
			if(zoom<3){
				if(zoom==2&&val>2){
					if(val<128){
						if(idx!=reg)break;
						goto retfalse;
					}
					break;
				}
				if(zoom==1&&val>3&&idx!=reg)break;
				goto retfalse;
			}
		case 0:
		case 4: 	// zoom=-1                 Rb+Ri+N
		case 8:		// val=0                   Rb+Ri*Z
			break;
		default:
//		case 13:	// val=0   idx=-1  zoom=-1 Rb
//		case 3:		// idx=-1  base=-1         N
//		case 6:		// base=-1 zoom=-1         Ri+N
//		case 11:	// val=0   base=-1 idx=-1  -
//		case 9: 	// val=0   idx=-1          Rb
//		case 14:	// val=0   base=-1 zoom=-1 Ri
//		case 7: 	// idx=-1  base=-1 zoom=-1 N
//		case 15:	// val=0   base=-1 idx=-1  zoom=-1
retfalse:
			inptr2=oinptr;
			cha2=ocha;
			tok=otok;
			itok=oitok;
			tok2=otok2;
			itok2.type=(unsigned short)otype2;
			linenum2=linenumber=oline;
			endoffile=0;
			if(bufrm){
				free(bufrm);
				bufrm=NULL;
			}
			if(strinf.bufstr){
				free(strinf.bufstr);
				strinf.bufstr=NULL;
			}
//	printf("return input=%08X inptr=%08X\n",input,inptr2);
			return FALSE;
	}
//	printf("flag=%08X\n",rflag);
	if(OutLea(reg,idx,base,zoom,val,rflag,&posttok)==FALSE)goto retfalse;
	ClearReg(reg);
	return TRUE;
}

void CheckRegForLea16(int reg,int *idx,int *base,unsigned long *val,unsigned int *rflag,ITOK *posttok)
{
int endloop=FALSE;
	for(;;){
		if(tok!=tk_plus&&(!(tok==tk_minus&&((tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_qword&&itok2.rm!=tk_double)||tok2==tk_postnumber))))break;
		if(tok2==tk_postnumber&&(posttok->post||tok==tk_minus))break;
		if(tok2==tk_reg){
			if(itok2.number==reg)endloop=TRUE;
			else if(*base==-1){
				switch(itok2.number){
					case BX:
					case BP:
					case SI:
					case DI:
						*base=itok2.number;
						break;
					default: endloop=TRUE; break;
				}
			}
			else if(*idx==-1){
				switch(itok2.number){
					case BX:
					case BP:
						if(*base==BX||*base==BP)endloop=TRUE;
						else{
							*idx=*base;
							*base=itok2.number;
						}
						break;
					case SI:
					case DI:
						if(*base==SI||*base==DI)endloop=TRUE;
						else{
							*idx=itok2.number;
						}
						break;
					default: endloop=TRUE; break;
				}
			}
			else break;
			if(endloop)break;
			nexttok();
		}
		else if(tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_qword&&itok2.rm!=tk_double){
			if(tok==tk_plus)*val+=itok2.number;
			else *val-=itok2.number;
			nexttok();
			*rflag^=itok.flag;
		}
		else if(tok2==tk_postnumber){
			if(posttok->post)break;
			if(tok==tk_plus)*val+=itok2.number;
			else *val-=itok2.number;
			nexttok();
			*posttok=itok;
		}
		else break;
		nexttok();
	}
}

void OutLea16(int reg,int idx,int base,unsigned int val,int rflag,ITOK *posttok)
{
int mod=rm_mod00;
int rm;
	if(val!=0){
		if(short_ok(val,FALSE)!=0)mod=rm_mod01;
		else mod=rm_mod10;
	}
	if((rflag&f_reloc)!=0||posttok->post)mod=rm_mod10;
	rm=CalcRm16(base,idx);
	op66(r16);
	op67(r16);
	op(0x8D);
	op((rm+reg*8)|mod);
	if(mod==rm_mod01)op(val);
	else if(mod==rm_mod10){
		if(posttok->post)setwordpost(posttok);
		else if((rflag&f_reloc)!=0)AddReloc();
		outword(val);
	}
}

int Reg16ToLea(int reg)	//оптимизация сложения 16-битных регистров в LEA
{
int idx,base;
unsigned long val;
int otok,otok2,otype2;
unsigned char ocha,next;
unsigned int oinptr,rflag;
int oline;
ITOK oitok;
ITOK posttok;
	if(cur_mod)return FALSE;
	posttok.post=0;
//	if(tok!=tk_reg&&tok!=tk_number&&am32!=0&&tok2!=tk_plus)return FALSE;
	if(!((tok==tk_reg||(tok==tk_number&&itok.rm!=tk_float&&itok.rm!=tk_double)&&tok==tk_postnumber)&&tok2==tk_plus))return FALSE;
	idx=base=-1;
	if(tok==tk_reg){
		if(itok.number==reg)return FALSE;
		switch(itok.number){
			case BX:
			case BP:
			case SI:
			case DI:
				base=itok.number;
				break;
			default: return FALSE;
		}
	}
	val=0;
	rflag=0;
	oinptr=inptr2;
	ocha=cha2;
	otok=tok;
	oitok=itok;
	otok2=tok2;
	otype2=itok2.type;
	oline=linenumber;
	if(tok==tk_postnumber){
		posttok=itok;
		tok=tk_number;
	}
	if(tok==tk_number){
		val=doconstdwordmath();
		rflag=postnumflag;
	}
	else nexttok();
	CheckRegForLea16(reg,&idx,&base,&val,&rflag,&posttok);
	next=0;
	if(idx==-1)next=1;
	if(base==-1)next|=2;
	if(val==0&&rflag==0&&posttok.post==0)next|=4;
	switch(next){
		case 0:	//base+idx+num
		case 1:	//base+num
//		case 2:	//idx+num
//		case 3:	//num
		case 4:	//base+idx
//		case 6:	//idx
//		case 7:	//
			break;
		case 5:	//base
			if(am32==0)break;
		default:
retfalse:
			inptr2=oinptr;
			cha2=ocha;
			tok=otok;
			itok=oitok;
			tok2=otok2;
			itok2.type=(unsigned short)otype2;
			linenum2=linenumber=oline;
			endoffile=0;
			if(bufrm){
				free(bufrm);
				bufrm=NULL;
			}
			if(strinf.bufstr){
				free(strinf.bufstr);
				strinf.bufstr=NULL;
			}
			return FALSE;
	}
	OutLea16(reg,idx,base,val,rflag,&posttok);
	return TRUE;
}

int Reg16ToLea2(int reg)	//оптимизация сложения 16-битных регистров в LEA
{
int idx,base;
unsigned long val;
int otok,otok2,otype2;
unsigned char ocha,next;
unsigned int oinptr,rflag;
int oline;
ITOK oitok;
ITOK posttok;
	if(cur_mod)return FALSE;
	posttok.post=0;
	if(tok==tk_plus&&((tok2==tk_reg&&itok2.number!=reg)||(tok2==tk_number&&itok2.rm!=tk_float&&itok2.rm!=tk_double))&&
		(reg==BX||reg==BP||reg==SI||reg==DI)){
		val=0;
		rflag=0;
		oinptr=inptr2;
		ocha=cha2;
		otok=tok;
		oitok=itok;
		otok2=tok2;
		otype2=itok2.type;
		oline=linenumber;
		idx=-1;
		base=reg;
		CheckRegForLea16(reg,&idx,&base,&val,&rflag,&posttok);
		next=0;
		if(idx==-1)next=1;
		if(base==-1)next|=2;
		if(val==0&&rflag==0&&posttok.post==0)next|=4;
		switch(next){
			case 1:	//base+num
				if(val<3&&rflag==0&&posttok.post==0)goto retfalse;
			case 0:	//base+idx+num
//		case 2:	//idx+num
//		case 3:	//num
			case 4:	//base+idx
//		case 6:	//idx
//		case 7:	//
				break;
//			case 5:	//base
		default:
retfalse:
			inptr2=oinptr;
			cha2=ocha;
			tok=otok;
			itok=oitok;
			tok2=otok2;
			itok2.type=(unsigned short)otype2;
			endoffile=0;
			linenum2=linenumber=oline;
			if(bufrm){
				free(bufrm);
				bufrm=NULL;
			}
			if(strinf.bufstr){
				free(strinf.bufstr);
				strinf.bufstr=NULL;
			}
			return FALSE;
		}
		OutLea16(reg,idx,base,val,rflag,&posttok);
		return TRUE;
	}
	return FALSE;
}

int OptimNum()	//оптимизация цифровых операндов
{
int otok,oinptr,deistv,negflag,starttok;
char ocha2;
long long val;
unsigned int flag,oflag;
int plusreloc=0;
	if(optnumber==FALSE||cur_mod)return FALSE;	//оптимизация отключена
	if(tok==tk_minus&&(itok2.flag&f_reloc))return FALSE;
	deistv=0;
	negflag=0;
	starttok=otok=tok;
	oflag=itok.flag;
	oinptr=inptr2;
	ocha2=cha2;
	nexttok();
	flag=itok.flag;
	val=itok.lnumber;
	switch(otok){
		case tk_minus:
			val=-val;
		case tk_plus:
			for(;;){	//оптимизация сложений-вычитаний
				nexttok();
				if((tok!=tk_plus&&tok!=tk_minus)||tok2!=tk_number||
						(tok==tk_minus&&(itok2.flag&f_reloc)&&plusreloc==0)){//не цыфра или другое действие
					tok=otok;
					cha2=ocha2;
					inptr2=oinptr;
					endoffile=0;
					if(deistv==0)break;	//ничего не было оптимизировано
					warningoptnum();
					tok=tk_plus;
					itok.lnumber=val;
					itok.flag|=flag;
					return TRUE;
				}
				deistv=tok;
				nexttok();
				oinptr=inptr2;
				ocha2=cha2;
				if(deistv==tk_minus){
					val-=itok.lnumber;
					if(itok.flag&f_reloc)plusreloc--;
				}
				else{
					val+=itok.lnumber;
					if(itok.flag&f_reloc)plusreloc++;
				}
				flag^=itok.flag;
			}
			break;
		case tk_divminus:
			otok=tk_div;
			goto LL1;
		case tk_multminus:
			otok=tk_mult;
LL1:
			negflag=TRUE;
		case tk_div:
		case tk_mult:
			for(;;){
				nexttok();
				if((tok!=tk_div&&tok!=tk_mult&&tok!=tk_divminus&&tok!=tk_multminus)||
				    tok2!=tk_number||
						(otok!=tok&&((val>itok2.lnumber&&(val%itok2.lnumber)!=0)
						||(val<itok2.lnumber&&(itok2.lnumber%val)!=0)))||
						((flag&f_reloc)&&(itok2.flag&f_reloc))){
					tok=starttok;
					cha2=ocha2;
					inptr2=oinptr;
					endoffile=0;
					if(deistv==0)break;
					warningoptnum();
					tok=otok;
					if(negflag==TRUE)val=-val;
					itok.lnumber=val;
					itok.flag|=flag;
					return TRUE;
				}
				if(tok==tk_divminus){
					tok=tk_div;
					negflag^=TRUE;
				}
				else if(tok==tk_multminus){
					tok=tk_mult;
					negflag^=TRUE;
				}
				deistv=tok;
				nexttok();
				oinptr=inptr2;
				ocha2=cha2;
				if(otok==deistv)val=val*itok.lnumber;
				else{
					if(val>itok.lnumber)val=val/itok.lnumber;
					else if(val<itok.lnumber){
						val=itok.lnumber/val;
						otok=deistv;
					}
					else{
						val=1;
						otok=tk_mult;
					}
				}
				flag|=itok.flag;
			}
			break;
		default:
			tok=starttok;
			cha2=ocha2;
			inptr2=oinptr;
			itok.flag=oflag;
	}
	if(bufrm){
		free(bufrm);
		bufrm=NULL;
	}
	if(strinf.bufstr){
		free(strinf.bufstr);
		strinf.bufstr=NULL;
	}
	return FALSE;
}

int expandvar()
{
	if(divexpand==FALSE)return FALSE;
	for(int i=inptr2-1;;i++){
		switch(input[i]){
			case '"':
			case ';':
			case '*':
			case '|':
			case '^':
			case ',':
			case 0:
			case '&': return FALSE;
			case '/':
				i++;
				if(input[i]=='/'||input[i]=='*')return FALSE;
			case '%': goto en;
		}
	}
en:
	warningexpand();
	return TRUE;
}

void RegMulNum(int reg,unsigned long num,int razr,int sign,int *expand,int flag)
{
int vop,i=0;
	if((flag&f_reloc)!=0)goto num_imul;
	if(razr==r16)num&=0xffff;
	if(chip>2&&(*expand==FALSE)&&optimizespeed&&leamul32(num,reg,razr)){
		setzeroflag=FALSE;
		return;
	}
	switch(num){
		case 0:
			ZeroReg(reg,razr);
			setzeroflag=TRUE;
		case 1: *expand=FALSE; break; /* AX * 1 = AX */
		case 2:  /* AX * 2 = ADD AX,AX */
			if(*expand==TRUE){
				if(optimizespeed==FALSE)goto num_imul;
				if(sign)cwdq(razr);
				else{
					op66(razr);
					outword(0xD231);
				}
			}
	 		op66(razr);
			op(1);
			op(0xC0+9*reg); // ADD reg,reg
			if(*expand==TRUE){
		 		op66(razr);
				outword(0xd283);	//adc dx,0
				op(0);
				ClearReg(DX);
			}
			setzeroflag=TRUE;
			break;
		default:
			vop=caselong(num);
			if(vop!=NUMNUM){
				if(chip<1&&razr==r16){
					if(*expand==TRUE){
						if(optimizespeed==FALSE)goto num_imul;
						op(0xB1);	op(vop); /* MOV CL,num */
						op66(r16);
						if(sign)op(0x99);
						else outword(0xD231);
						outdword(0xd213c001);	//ADD AX,AX ADC DX,DX
						outword(0xfae2);  //LOOP -6
						warningreg(regs[0][2]);
						ClearReg(DX);
					}
					else{
						if(reg==CX)goto num_imul;
						if(vop>3){
							for(;vop!=0;vop--){
								op66(r16);
								op(3);	//add
								op(0xc0+reg*9);
							}
							return;
						}
						op(0xB1);	op(vop); /* MOV CL,num */
						op(0xD3);
						op(0xE0+reg);	//SHL reg,CL
					}
					ConstToReg(vop,CL,r8);
					warningreg(begs[1]);
				}
				else{/* SHL AX,num */
					if(*expand==TRUE){
						if(optimizespeed==FALSE)goto num_imul;
//						op66(razr);
						ClearDX(razr,sign);
/*						if(sign)cwdq(razr);
						else{
							op66(razr);
							outword(0xC031);
						}*/
						if(chip<3&&razr==r16){
							op(0xB1);	op(vop); /* MOV CL,num */
							outdword(0xd213c001);	//ADD AX,AX ADC DX,DX
							outword(0xfae2);  //LOOP -6
							ConstToReg(vop,CL,r8);
							warningreg(begs[1]);
						}
						else{
					 		op66(razr);
							op(0x0f);
							outword(0xC2a4);	//SHLD DX,AX,num
							op(vop);
					 		op66(razr);
							outword(0xE0C1);	//SHL AX,num
							op(vop);
						}
						ClearReg(DX);
						warningreg(regs[razr/2-1][2]);
					}
					else{
					 	op66(razr);
						op(0xc1);
						op(0xe0+reg); // SHL reg,imm8
						op(vop);
						if(cpu<2)cpu=2;
					}
				}
				setzeroflag=TRUE;
				break;
			}
			if(chip<7&&*expand==FALSE&&optimizespeed&&speedmul32(num,reg,razr))break;
num_imul:
			if((razr==r16&&chip<2)||(*expand==TRUE)){
				if(reg==AX){
				 	op66(razr);
					op(0xB9);  /* MOV CX,# */
					if((flag&f_reloc)!=0)AddReloc();
					razr==r16?outword((unsigned int)num):outdword(num);
				 	op66(razr);
					if(sign)outword(0xE9F7); /* IMUL CX */
					else outword(0xE1F7); /* MUL CX */
					ConstToReg(num,CX,razr);
					warningreg(regs[razr/2-1][1]);
				}
				else{
					op66(r16);
					op(0x90+reg);	//XCHG AX,reg
					op66(r16);
					op(reg!=DX?0xBA:0xB9);
					outword(num);	//mov DX,num
					op66(r16);
					op(0xF7);
					op(reg!=DX?0xE2:0xE1);	// mul DX
					op66(r16);
					op(0x90+reg);	//XCHG AX,reg
					warningreg(regs[0][reg!=DX?2:1]);
					ClearReg(DX);
				}
			}
			else{
				if((flag&f_reloc)==0&&short_ok(num,razr/2-1))i=2;	//короткая форма
			 	op66(razr);
				op(0x69+i);	//imul
				op(0xc0+reg*9);
				if(i==2)op(num);
				else{
					if((flag&f_reloc)!=0)AddReloc();
					razr==r16?outword((unsigned int)num):outdword(num);
				}
			}
			setzeroflag=FALSE;
			break;
	}
}

/* --------------- byte, char, word, int math starts ----------------- */

long long CalcNumber(int sign)
{
long long hold;
	switch(sign){
		case 0:
			hold=doconstdwordmath();
			break;
		case 1:
			hold=doconstlongmath();
			break;
		case 2:
			hold=doconstfloatmath();
			break;
		case 3:
			hold=doconstdoublemath();
			break;
		default:
			hold=doconstqwordmath();
			break;
	}
	return hold;
}

int OnlyNumber(int sign)
/*-----------------11.09.00 12:44-------------------
 проверить что строка состоит только из цифр.
--------------------------------------------------*/
{
ITOK oitok=itok;
unsigned char ocha=cha2;
unsigned int oinptr=inptr2;
unsigned int otok2=tok2;
int otype2=itok2.type;
	itok.lnumber=CalcNumber(sign);
//	printf("tok=%d num=%d type=%d\n",tok,itok.number,itok.type);
	if(itok.type==tp_stopper){
		return TRUE;	//только цифры
	}
	cha2=ocha;	//востановить исходное состояние
	inptr2=oinptr;
	tok2=otok2;
	tok=tk_number;
	itok=oitok;
	itok2.type=(unsigned short)otype2;
	if(bufrm){
		free(bufrm);
		bufrm=NULL;
	}
	if(strinf.bufstr){
		free(strinf.bufstr);
		strinf.bufstr=NULL;
	}
	return FALSE;
}

void MultiAssignFloat(int type,int npointr=0)
{
int otok;
ITOK wtok;
char *wbuf;
SINFO wstr;
char *ofsstr=NULL;
	if(tok!=type)illegalfloat();
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	otok=tok;
	nexttok();
//	getoperand();
int numpointr=0;
	nexttok();
	if(tok==tk_float||tok==tk_double)nexttok();
	while(tok==tk_mult){
		nexttok();
		numpointr++;
	}
	if(numpointr>itok.npointr)unuseableinput();
	if(tok2==tk_assign)MultiAssignFloat(type,numpointr);
	else{
		if(tok==tk_pointer)cpointr(BX,numpointr);
		if(tok==tk_floatvar&&tok2==tk_semicolon){
			tok=tk_dwordvar;
			do_e_axmath(1,r32,&ofsstr);
		}
		else doeaxfloatmath(tk_reg32,AX);
	}
	if(otok==tk_pointer){
		if(wtok.type==tk_proc){
			wtok.rm=wtok.sib;
			if(am32)wtok.sib=CODE32;
			else wtok.sib=CODE16;
			compressoffset(&wtok);
		}
		else{
			int razr=typesize(itok.type);
			if(npointr<=wtok.npointr)getpointeradr(&wtok,wbuf,&wstr,npointr-1,razr,BX);
			else unuseableinput();
			wtok=itok;
		}
	}
	if((wtok.rm==rm_d16&&wtok.sib==CODE16)||(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0))){
		op66(r32);
		outseg(&wtok,1);
		op(0xA3); /* MOV [word],AX */
		if(wtok.post==UNDEF_OFSET){
			AddUndefOff(2,wtok.name);
			wtok.post=0;
		}
		if(am32==FALSE)outword(wtok.number);	//????
		else outdword(wtok.number);
	}
	else{
		CheckAllMassiv(wbuf,r32,&wstr,&wtok);
		op66(r32);
		outseg(&wtok,2);
		op(0x89); op(wtok.rm); /* MOV [rmword],reg */
		outaddress(&wtok);
	}
}

int MultiAssign(int razr,int usereg,int npointr)
{
int otok;
ITOK wtok;
char *wbuf;
SINFO wstr;
int sign=0,rettype,nrazr,posiblret,pointr=0;
int hnumber=0;
int ureg;
char *ofsstr=NULL;
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	otok=tok;
	if(tok==tk_bits){
		usereg=EAX;//USEONLY_AX;
		nrazr=r32;
		rettype=tk_dword;
		int i=itok.bit.siz+itok.bit.ofs;
		if(i<9){
			nrazr=r8;
			rettype=tk_byte;
		}
		else if(i<17){
			rettype=tk_word;
			nrazr=r16;
		}
		else if(i>32)nrazr=r64;
	}
	else{
		nrazr=GetVarSize(tok);
		switch(tok){
			case tk_beg:
				ureg=itok.number;
				if(ureg>3)ureg-=4;
				if(usereg>ureg)usereg=ureg;
				break;
			case tk_reg:
			case tk_reg32:
				if(usereg>itok.number)usereg=itok.number;
				break;
		}
//		if(tok==tk_beg)usereg|=USEFIRST4REG;
	}
	posiblret=nrazr;
	if(nrazr>razr&&nrazr<r64)razr=nrazr;
	switch ( razr ) {
		case r8:
			rettype=tk_byte;
			break;
		case r16:
			rettype=tk_word;
			break;
		case r32:
			rettype=tk_dword;
			break;
	}
	nexttok();
int numpointr=0;
	ofsstr=GetLecsem(tk_assign,tk_semicolon);
	nexttok();
	convert_type(&sign,&rettype,&pointr);
	while(tok==tk_mult){
		nexttok();
		numpointr++;
	}
	if(numpointr>itok.npointr){
		unuseableinput();
	}
	ureg=AX;
	if(tok2==tk_assign){
		ureg=hnumber=MultiAssign(razr,usereg,numpointr);
		if(ofsstr){
			free(ofsstr);
			ofsstr=NULL;
		}
	}
	else{
		if(tok==tk_pointer){
			int reg=idxregs[2];
			if(reg==ureg)reg=idxregs[1];
			cpointr(reg,numpointr);
		}
		if(usereg==USEALLREG){
			switch(tok){
				case tk_reg:
				case tk_reg32:
					usereg=itok.number;
					break;
				case tk_beg:
					usereg=(itok.number>3?itok.number-4:itok.number);
					break;
				default:
					usereg=EAX;
					break;
			}
		}
		switch(rettype){
			case tk_char:
			case tk_byte:
				if(tok2==tk_semicolon&&usereg!=0&&usereg<4){
					ureg=hnumber=usereg;
//					if(tok==tk_beg&&itok.number<4)ureg=hnumber=itok.number;
					getintoreg(usereg,r16,sign,&ofsstr);
				}
/*				if(tok2==tk_semicolon&&tok==tk_beg&&usereg<2){
					if((usereg==1&&itok.number<4)||usereg==0){
						ureg=hnumber=itok.number;
						nexttok();
						break;
					}
				}*/
				else doalmath(sign,&ofsstr);
				if(ofsstr)IDZToReg(ofsstr,usereg,r8);
				break;
			case tk_int:
			case tk_word:
				if(tok2==tk_semicolon&&usereg!=0){
					ureg=hnumber=usereg;
//					if(tok==tk_reg)ureg=hnumber=itok.number;
					getintoreg(usereg,r16,sign,&ofsstr);
				}
				else do_e_axmath(sign,r16,&ofsstr);
				if(ofsstr)IDZToReg(ofsstr,usereg,r16);
				break;
			case tk_float:
				doeaxfloatmath(tk_reg32,AX);
				break;
			default:
				if(tok2==tk_semicolon&&usereg!=0&&tok!=tk_floatvar){
					ureg=hnumber=usereg;
//					if(tok==tk_reg32)ureg=hnumber=itok.number;
					getintoreg(usereg,r32,sign,&ofsstr);
				}
				else{
					if(tok==tk_floatvar&&tok2==tk_semicolon)tok=tk_dwordvar;
					do_e_axmath(sign,r32,&ofsstr);
				}
				if(ofsstr)IDZToReg(ofsstr,usereg,r32);
				break;
		}
	}
	if(ofsstr){
		free(ofsstr);
		ofsstr=NULL;
	}
	switch ( nrazr ) {
		case r8:
			posiblret=tk_byte;
			break;
		case r16:
			posiblret=tk_word;
			break;
		case r32:
			posiblret=tk_dword;
			break;
	}
	convert_returnvalue(posiblret,rettype);
	if(otok==tk_pointer)cwpointr(&wtok,wbuf,&wstr,&otok,npointr,ureg);
	switch ( otok ) {
		case tk_intvar:
		case tk_wordvar:
		case tk_longvar:
		case tk_dwordvar:
		case tk_floatvar:
#ifdef OPTVARCONST
			CheckRegToConst(hnumber,&wtok,otok>tk_wordvar?r32:r16);
#endif
			KillVar(wtok.name);
			AddRegVar(hnumber,razr,&wtok);
			if(hnumber==0&&((wtok.rm==rm_d16&&wtok.sib==CODE16)||(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
				op66(nrazr);
				outseg(&wtok,1);
				op(0xA3); /* MOV [word],AX */
				if(wtok.post==UNDEF_OFSET){
					AddUndefOff(2,wtok.name);
					wtok.post=0;
				}
				if(am32==FALSE)outword(wtok.number);	//????
				else outdword(wtok.number);
			}
			else{
				CheckAllMassiv(wbuf,nrazr,&wstr,&wtok);
				op66(nrazr);
				outseg(&wtok,2);
				op(0x89); op(wtok.rm+hnumber*8); /* MOV [rmword],reg */
				outaddress(&wtok);
			}
			break;
		case tk_charvar:
		case tk_bytevar:
#ifdef OPTVARCONST
			CheckRegToConst(hnumber,&wtok,r8);
#endif
			KillVar(wtok.name);
			AddRegVar(hnumber,r8,&wtok);
			if(hnumber==0&&((wtok.rm==rm_d16&&wtok.sib==CODE16)||(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
				outseg(&wtok,1);
				op(0xA2); 		/* MOV [byte],AL */
				if(wtok.post==UNDEF_OFSET){
					AddUndefOff(2,wtok.name);
					wtok.post=0;
				}
				if(am32==FALSE)outword(wtok.number);
				else outdword(wtok.number);
			}
			else{
				CheckAllMassiv(wbuf,1,&wstr,&wtok);
				outseg(&wtok,2);  /* MOV [rmbyte],AL */
				op(0x88);
				op(wtok.rm+hnumber*8);
				outaddress(&wtok);
			}
			break;
		case tk_bits:
			if(razr!=r64){
				op66(nrazr==r32?r32:r16);
				op(0x50);	//push eax
				if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=4;
				reg2bits(&wtok,razr);
				op66(nrazr==r32?r32:r16);
				op(0x58);	//pop eax
			}
			else{
				op66(r32);
				op(0x50);	//push eax
				int siz=wtok.bit.siz;
				op66(r32);
				op(0x50);	//push eax
				wtok.bit.siz=32-wtok.bit.ofs;
				if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=8;
				reg2bits(&wtok,r32);
				op66(r32);
				op(0x58);	//pop eax
				op66(r32);	//shr eax,size
				outword(0xE8C1);
				op(wtok.bit.siz);
				wtok.bit.siz=siz+wtok.bit.ofs-32;
				wtok.bit.ofs=0;
				wtok.number+=4;
				reg2bits(&wtok,r8);
				op66(r32);
				op(0x58);	//pop eax
			}
			break;
		case tk_reg:
		case tk_reg32:
			if(wtok.number!=hnumber){
				if(RegToReg(hnumber,wtok.number,nrazr)==NOINREG){
					op66(nrazr);
					op(0x89);
					op(0xC0+wtok.number+hnumber*8);	//mov reg,AX
				}
				else waralreadinitreg(regs[nrazr/4][wtok.number],regs[nrazr/4][hnumber]);
			}
			break;
		case tk_beg:
			if(razr>r8&&wtok.number>3&&(wtok.number%4)==hnumber)preerror("register AH,BH,CH,DH should be first");
			if(wtok.number!=hnumber){
				if(RegToReg(hnumber,wtok.number,r8)==NOINREG){
					op(0x88);
					op(0xC0+wtok.number+hnumber*8);	//mov beg,AL
				}
				else waralreadinitreg(begs[wtok.number],begs[hnumber]);
			}
			break;
		case tk_seg:
			op(0x8E); 	/* MOV SEG,AX */
			op(0xC0+wtok.number*8+hnumber);
			break;
		default:
			thisundefined(wtok.name);
	}
	return hnumber;
}

int do_d_wordvar(int sign,int razr,int terminater)	//signed or unsigned 16 or 32 bit memory variable
{
unsigned char next=1,getfromAX=0;
unsigned int vop=0,otok,rettype,posiblret;
ITOK wtok;
char *wbuf,*rbuf;
SINFO wstr;
int retrez=0,pointr=0,hnumber=EAX;
int numpointr=0;
char *ofsstr=NULL;
int reg1=idxregs[0],reg2=idxregs[1];
#ifdef OPTVARCONST
int initconst=FALSE;
int operand;
#endif
unsigned int oaddESP=addESP;
//	sign==0?rettype=(razr==r16?tk_word:tk_dword):rettype=(razr==r16?tk_int:tk_long);
	posiblret=rettype=(sign==0?(razr==r16?tk_word:tk_dword):(razr==r16?tk_int:tk_long));
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	otok=tok;
	while(RmEqualReg(hnumber,itok.rm,itok.sib))hnumber++;
	nexttok();
#ifdef OPTVARCONST
	operand=tok;
#endif
	switch(tok){
		case tk_assign:	//=
			if(!((tok2==tk_reg||tok2==tk_reg32)&&ScanTok3()==terminater)){
				ofsstr=GetLecsem(terminater);
			}
			if(ofsstr){
				int retreg;
				if((retreg=CheckIDZReg(ofsstr,AX,razr))!=NOINREG){
					GetEndLex(terminater);
					if(retreg==SKIPREG)retreg=AX;
					if((GetRegVar(&wtok)&(1<<retreg))!=0){
						free(ofsstr);
						if(wbuf)free(wbuf);
						if(wstr.bufstr)free(wstr.bufstr);
						break;
					}
					if(razr==r16)tok=tk_reg;
					else tok=tk_reg32;
					itok.number=retreg;
					if(reg1==itok.number){
						reg1=idxregs[1]; reg2=idxregs[2];
					}
					if(reg2==itok.number){
						reg2=idxregs[2];
					}
					goto regtovar;
				}
			}
			nexttok();
//			printf("tok=%d %s\n",tok,itok.name);
			convert_type(&sign,(int *)&rettype,&pointr);
			while(tok==tk_mult){
				nexttok();
				numpointr++;
			}
			if(numpointr>itok.npointr){
				unuseableinput();
			}
			if(tok2==tk_assign){
				hnumber=MultiAssign(razr,USEALLREG,numpointr);
				if(ofsstr){
					free(ofsstr);
					ofsstr=NULL;
				}
				next=0;
				goto getfromax;
			}
			if(tok==tk_pointer)cpointr(am32==TRUE?EAX:BX,numpointr);
			CheckMinusNum();
//			printf("tok=%d tok2=%d\n",tok,tok2);
			if(itok2.type==tp_opperand){	//сложное выражение
				if(tok==tk_number){	//проверка и суммирование чисел
					if(OnlyNumber(rettype==tk_float?2:sign)){
						next=0;
						itok.flag=(unsigned char)postnumflag;
						if(postnumflag==0)goto loadconst;
						goto numbertovar;
					}
				}
				goto labl1;
			}
			else{
//		 		if(hnumber!=EAX&&(tok==tk_reg||tok==tk_reg32)&&itok.number==EAX)goto labl1;
#ifdef OPTVARCONST
				CheckConstVar3(&tok,&itok,razr);
#endif
				switch(tok){
					case tk_number:
loadconst:
						if((itok.flag&f_reloc)==0){
#ifdef OPTVARCONST
							if(razr==r16)itok.lnumber&=0xffff;
							else itok.lnumber&=0xffffffff;
							if((initconst=Const2Var(&wtok,itok.lnumber,itok.rm))==FALSE){
								waralreadinitvar(wtok.name,itok.number);
								initconst=TRUE;
								break;
							}
#endif
							if(itok.number==0){
		 						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
								op66(razr);
								outseg(&wtok,2);
								op(0x83);
								op(wtok.rm+0x20);
								outaddress(&wtok);
								op(0);
								break;
							}
							if((razr==r32&&itok.number==0xFFFFFFFF)||(razr==r16&&itok.number==0xFFFF)){
		 						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
								op66(razr);
								outseg(&wtok,2);
								op(0x83);
								op(wtok.rm+0x8);
								outaddress(&wtok);
								op(0xFF);
								break;
							}
							if(regoverstack&&razr==r32&&short_ok(itok.number,TRUE)){
								CheckAllMassiv(wbuf,razr,&wstr,&wtok);
								op66(razr);
								op(0x6A);
								op(itok.number);	//push short number
								op66(razr);
								outseg(&wtok,2);
								op(0x8f);
								op(wtok.rm);
								outaddress(&wtok);
								break;
							}
						}
					case tk_postnumber:
					case tk_undefofs:
numbertovar:
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						op(0xC7);	//mov word[],number
						op(wtok.rm);
						outaddress(&wtok);
						if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
						else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
						else if((itok.flag&f_reloc)!=0)AddReloc();
						if(razr==r16){
							if(am32!=FALSE&&tok!=tk_number)dwordvalexpected();
							outword((unsigned int)itok.number);
						}
						else outdword(itok.number);
						break;
					case tk_apioffset:
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						op(0xC7);	//mov word[],number
						op(wtok.rm);
						outaddress(&wtok);
						AddApiToPost(itok.number);
						break;
					case tk_reg32:
//						if(razr==r16)goto labl1;
					case tk_reg:
						if(razr==r32&&tok==tk_reg)goto labl1;
regtovar:
						if((unsigned int)itok.number==0){
							getfromAX=1;
							hnumber=0;
						}
						else{
//							if(wbuf==0&&wstr.bufstr==NULL){
								KillVar(wtok.name);
								AddRegVar(itok.number,razr,&wtok);
								vop++;
//							}
							CheckAllMassiv(wbuf,razr,&wstr,&wtok,reg1,reg2);
							op66(razr);
							outseg(&wtok,2);
							op(0x89);
							op((unsigned int)itok.number*8+wtok.rm);
							outaddress(&wtok);
						}
						break;
					case tk_seg:
						if(razr==r32)goto labl1;
						CheckAllMassiv(wbuf,2,&wstr,&wtok);
						op66(r16);
						outseg(&wtok,2);
						op(0x8C);
						op((unsigned int)itok.number*8+wtok.rm);
						outaddress(&wtok);
						if((unsigned int)itok.number==FS||(unsigned int)itok.number==GS)if(cpu<3)cpu=3;
						break;
					case tk_string:
						CheckAllMassiv(wbuf,1,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						op(0xC7);
						op(wtok.rm);
						outaddress(&wtok);
						if(razr==r16){
							if(am32)dwordvalexpected();
							outword(addpoststring());
						}
						else outdword(addpoststring());
						break;
					case tk_doublevar:
						vop=4;
					case tk_floatvar:
						intinstack(vop);
						getfromAX=0;
						CheckAllMassiv(wbuf,4,&wstr,&wtok);
						outseg(&wtok,2);	//fistp var
						op(razr==r16?0xDF:0xDB);
						op(wtok.rm+0x18);
						outaddress(&wtok);
						if(sign==0)warningretsign();
						fwait3();
						hnumber=EAX;
						break;
					case tk_new:
						donew();
						getfromAX=1;
						clearregstat();
#ifdef OPTVARCONST
						FreeGlobalConst();
#endif
						if(ofsstr){
							free(ofsstr);
							ofsstr=NULL;
						}
						hnumber=0;
						break;
					case tk_delete:
						dodelete();
						terminater=next=0;
						getfromAX=1;
						clearregstat();
#ifdef OPTVARCONST
						FreeGlobalConst();
#endif
						if(ofsstr)free(ofsstr);
						hnumber=0;
						break;
					case tk_longvar:
					case tk_dwordvar:
						if((rettype==tk_long||rettype==tk_dword)&&hnumber&&regoverstack)goto pushvar;
						goto labl1;
					case tk_intvar:
					case tk_wordvar:
						if((rettype==tk_int||rettype==tk_word)&&hnumber&&regoverstack){
pushvar:
							CheckAllMassiv(bufrm,razr,&strinf);
							op66(razr);
							outseg(&itok,2);
							op(0xFF);	// PUSH [dword]
							op(0x30+itok.rm);
							outaddress(&itok);
							CheckAllMassiv(wbuf,razr,&wstr,&wtok);
							op66(razr);
							outseg(&wtok,2);
							op(0x8f);
							op(wtok.rm);
							outaddress(&wtok);
							break;
						}
						goto labl1;
					default:
labl1:
						getfromAX=1;
						if(rettype==tk_char||rettype==tk_byte){
							if(hnumber==0)retrez=doalmath(sign,&ofsstr);
							else{
								retrez=getintoreg(hnumber,razr,sign,&ofsstr);
								posiblret=rettype;
							}
						}
						else if(rettype==tk_int||rettype==tk_word){
							if(hnumber==0)retrez=do_e_axmath(sign,r16,&ofsstr);
							else retrez=getintoreg(hnumber,r16,sign,&ofsstr);
						}
						else if(rettype==tk_float||rettype==tk_double){
							doeaxfloatmath(tk_fpust,AX,rettype==tk_float?0:4);
							getfromAX=0;
							CheckAllMassiv(wbuf,4,&wstr,&wtok);
							outseg(&wtok,2);	//fistp var
							op(razr==r16?0xDF:0xDB);
							op(wtok.rm+0x18);
							outaddress(&wtok);
							if(sign==0)warningretsign();
							fwait3();
							hnumber=EAX;
						}
						else{
							if(hnumber==0)retrez=do_e_axmath(sign,r32,&ofsstr);
							else retrez=getintoreg(hnumber,r32,sign,&ofsstr);
						}
						next=0;
						break;
				}
			}
			if(getfromAX){
getfromax:
#ifdef OPTVARCONST
				initconst=CheckRegToConst(hnumber,&wtok,razr);
#endif
				if(retrez==0)retrez=razr==r16?tk_reg:tk_reg32;
				convert_returnvalue(posiblret,rettype);
				if(addESP!=oaddESP&&am32&&ESPloc&&(wtok.type==tp_paramvar||wtok.type==tp_localvar))wtok.number+=addESP-oaddESP;
				if(wbuf==NULL&&wstr.bufstr==NULL&&hnumber==0&&
						((wtok.rm==rm_d16&&wtok.sib==CODE16)||
						(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
					op66(razr);
					outseg(&wtok,1);
					op(0xA3); // MOV [word],AX
					if(wtok.post==UNDEF_OFSET){
						AddUndefOff(2,wtok.name);
						wtok.post=0;
					}
					if(am32==FALSE)outword(wtok.number);	//????
					else outdword(wtok.number);
				}
				else{
					CheckAllMassiv(wbuf,razr,&wstr,&wtok,reg1,reg2);
//	printf("flag=%08X rm=%d num=%d post=%d sib=%d\n",wtok.flag,wtok.rm,wtok.number,wtok.post,wtok.sib);
					op66(razr);
					outseg(&wtok,2);
					op(0x89); op(wtok.rm+hnumber*8); // MOV [rmword],AX
					outaddress(&wtok);
				}
				if(ofsstr)IDZToReg(ofsstr,hnumber,razr);
				else ClearReg(hnumber);
				KillVar(wtok.name);
				AddRegVar(hnumber,razr,&wtok);
			}
			else{
//				printf("vop=%d %s\n",vop,wtok.name);
				if(vop==0)KillVar(wtok.name);
			}
			if(ofsstr)free(ofsstr);
			break;
		case tk_minusminus: vop=0x8;
		case tk_plusplus:
#ifdef OPTVARCONST
			initconst=UpdVarConst(&wtok,1,tk_byte,tok);
#endif
			CheckAllMassiv(wbuf,razr,&wstr,&wtok);
			op66(razr);
			outseg(&wtok,2);
			op(0xFF); op(vop+wtok.rm);
			outaddress(&wtok);
			KillVar(wtok.name);
			break;
		case tk_cdecl:
		case tk_pascal:
		case tk_fastcall:
		case tk_stdcall:
			vop=tok;
			nexttok();
			if(tok!=tk_openbracket){
				expected('(');
				FindStopTok();
			}
		case tk_openbracket:	//вызов процедуры по адресу в регистре
			param[0]=0;
			int i;
			i=0;
			switch ( vop ) {
				case tk_cdecl:
				case tk_stdcall:
					i=swapparam();
					break;
				case tk_pascal:
					doparams();
					break;
				case tk_fastcall:
					doregparams();
					break;
				default:
					if(comfile==file_w32)swapparam();
					else doparams();
			}
			if(vop!=tk_cdecl)i=0;
			CheckAllMassiv(wbuf,razr,&wstr,&wtok);
			outseg(&wtok,2);
			op(0xFF); op(0x10+wtok.rm);
			outaddress(&wtok);
			clearregstat();
#ifdef OPTVARCONST
			FreeGlobalConst();
#endif
			if(i)CorrectStack(i);
			break;
		case tk_xorequals: vop+=0x08;
		case tk_minusequals: vop+=0x08;
		case tk_andequals: vop+=0x18;
		case tk_orequals: vop+=0x08;
		case tk_plusequals:
			getoperand(am32==TRUE?EAX:BX);
			if(tok==tk_float){
		 		getoperand(am32==TRUE?EAX:BX);
				doeaxfloatmath(tk_reg32,AX);
				goto axtovar;
			}
			if(itok2.type==tp_opperand){
				if(tok==tk_number){
					if(OnlyNumber(sign)){
						next=0;
						otok=tok;
						tok=tk_number;
						goto num;
					}
				}
				retrez=razr==r16?tk_reg:tk_reg32;
				do_e_axmath(sign,razr,&ofsstr);
				if(addESP!=oaddESP&&am32&&ESPloc&&(wtok.type==tp_paramvar||wtok.type==tp_localvar))wtok.number+=addESP-oaddESP;
axtovar:
				CheckAllMassiv(wbuf,razr,&wstr,&wtok);
				op66(razr);
				outseg(&wtok,2);
				op(0x01+vop); op(wtok.rm);	/* ADD [anyword],AX */
				outaddress(&wtok);
				next=0;
			}
			else{
				switch(tok){
					case tk_number:
					case tk_postnumber:
					case tk_undefofs:
num:
#ifdef OPTVARCONST
						if(tok==tk_number&&(itok.flag&f_reloc)==0){
							initconst=UpdVarConst(&wtok,itok.lnumber,itok.rm,operand);
						}
#endif
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						if(tok==tk_number&&(itok.flag&f_reloc)==0&&itok.number==1&&(vop==0||vop==0x28)){
							if(vop)vop=8;
							op(0xFF); op(vop+wtok.rm);
							outaddress(&wtok);
						}
						else if(tok!=tk_undefofs&&tok!=tk_postnumber&&(itok.flag&f_reloc)==0&&
								short_ok(itok.number,razr/2-1)){
							op(0x83);
							op(vop+wtok.rm);
							outaddress(&wtok);
							op((unsigned int)itok.number);
						}
						else{
							op(0x81);
							op(vop+wtok.rm);
							outaddress(&wtok);
							if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
							else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
							else if((itok.flag&f_reloc)!=0)AddReloc();
							razr==r16?outword(itok.number):outdword(itok.number);
						}
						if(next==0)tok=otok;
						break;
					case tk_apioffset:
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						op(0x81);
						op(vop+wtok.rm);
						outaddress(&wtok);
						AddApiToPost(itok.number);
						break;
					case tk_reg32:
						if(razr==r16)goto defxor;
					case tk_reg:
						if(tok==tk_reg&&razr==r32)goto defxor;
#ifdef OPTVARCONST
						initconst=CheckUpdRegToConst(itok.number,&wtok,operand,razr);
#endif
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						op(0x01+vop); op((unsigned int)itok.number*8+wtok.rm);
						outaddress(&wtok);
						break;
					default:
defxor:
						retrez=razr==r16?tk_reg:tk_reg32;
						do_e_axmath(sign,razr,&ofsstr);
						if(addESP!=oaddESP&&am32&&ESPloc&&(wtok.type==tp_paramvar||wtok.type==tp_localvar))wtok.number+=addESP-oaddESP;
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						op66(razr);
						outseg(&wtok,2);
						op(0x01+vop); op(wtok.rm);  /* ADD [anyword],AX */
						outaddress(&wtok);
						next=0;
						break;
				}
			}
//			puts(wtok.name);
			KillVar(wtok.name);
			break;
		case tk_multequals:
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_stopper){
				if(tok==tk_number){
					if(itok.number==1)break;
					if(itok.number==0){
						ZeroReg(hnumber,razr);
						goto getfromax;
					}
#ifdef OPTVARCONST
					if((itok.flag&f_reloc)==0){
						initconst=UpdVarConst(&wtok,itok.lnumber,itok.rm,operand);
					}
#endif
					if(hnumber==0)getinto_e_ax(sign,otok,&wtok,wbuf,&wstr,razr,TRUE);
					else getinto_reg(otok,&wtok,wbuf,&wstr,razr,hnumber);
					vop=0;
					RegMulNum(hnumber,itok.number,razr,sign,(int *)&vop,itok.flag);
					goto getfromax;
				}
			}
			if(hnumber==0)do_e_axmath(sign,razr,&ofsstr);
			else getintoreg(hnumber,razr,sign,&ofsstr);
			if(addESP!=oaddESP&&am32&&ESPloc&&(wtok.type==tp_paramvar||wtok.type==tp_localvar)){
				wtok.number+=addESP-oaddESP;
				oaddESP=addESP;
			}
			CheckAllMassiv(wbuf,razr,&wstr,&wtok);
			op66(razr);
			if(hnumber==0){
				outseg(&wtok,2);
				op(0xF7);	// imul/mul var
				if(sign)op(0x28+wtok.rm);
				else op(0x20+wtok.rm);
			}
			else{
				outseg(&wtok,3);
				outword(0xaf0f);
				op(wtok.rm+hnumber*8);
			}
			outaddress(&wtok);
			next=0;
			KillVar(wtok.name);
			goto getfromax;
		case tk_divequals:
			getoperand(am32==TRUE?EAX:BX);
			hnumber=0;
			if(itok2.type==tp_stopper){
				if(tok==tk_number){
					if(itok.number==1)break;
#ifdef OPTVARCONST
					if((itok.flag&f_reloc)==0){
						initconst=UpdVarConst(&wtok,itok.lnumber,itok.rm,operand);
					}
#endif
					getinto_e_ax(sign,otok,&wtok,wbuf,&wstr,razr,TRUE);
					DivMod(0,sign,razr,0);
					next=0;
					goto getfromax;
				}
				getintoreg_32(CX,razr,sign,&ofsstr);
			}
			else{
				do_e_axmath(sign,razr,&ofsstr);
				if(optimizespeed)outword(0xC88B);	//mov CX,ax
				else op(0x90+ECX);	//xchg ax,Cx
				if(addESP!=oaddESP&&am32&&ESPloc&&(wtok.type==tp_paramvar||wtok.type==tp_localvar)){
					wtok.number+=addESP-oaddESP;
					oaddESP=addESP;
				}
			}
			getinto_e_ax(sign,otok,&wtok,wbuf,&wstr,razr,TRUE);
			ClearDX(razr,sign);
			op66(razr);
			op(0xF7);
			if(sign)op(0xF8+ECX); // IDIV CX
			else op(0xF0+ECX); // DIV CX
			next=0;
			warningreg(regs[razr/2-1][ECX]);
			KillVar(wtok.name);
			goto getfromax;
		case tk_swap:
			KillVar(wtok.name);
			int regdi;
			regdi=TRUE;
			getoperand();
			rbuf=bufrm;
			bufrm=NULL;
			if(am32!=FALSE&&wbuf!=NULL&&wstr.bufstr!=NULL)regdi=FALSE;
			switch(tok){
				case tk_reg32:
					if(razr==r16)swaperror();
				case tk_reg:
					if(tok==tk_reg&&razr==r32)swaperror();
#ifdef OPTVARCONST
					initconst=SwapVarRegConst(itok.number,&wtok,razr);
#endif
					CheckAllMassiv(wbuf,razr,&wstr,&wtok);
					op66(razr);
					outseg(&wtok,2);
					op(0x87);
					op((unsigned int)itok.number*8+wtok.rm);
					outaddress(&wtok);
					ClearReg(itok.number);
					break;
				case tk_qwordvar:
				case tk_longvar:
				case tk_dwordvar:
					if(razr==r16)swaperror();
				case tk_intvar:
				case tk_wordvar:
					if((tok==tk_intvar||tok==tk_wordvar)&&razr==r32)swaperror();
#ifdef OPTVARCONST
					ClearVarByNum(&itok);
#endif
					if(hnumber==0)getinto_e_ax(sign,otok,&wtok,wbuf,&wstr,razr,TRUE);
					else{
						if(regoverstack&&(!((bufrm||strinf.bufstr)&&(wbuf||wstr.bufstr)))){
							CheckAllMassiv(bufrm,razr,&strinf);
							op66(razr);
							outseg(&itok,2);
							op(0xFF);	// PUSH [dword]
							op(0x30+itok.rm);
							outaddress(&itok);
							CheckAllMassiv(wbuf,razr,&wstr,&wtok);
							op66(razr);
							outseg(&wtok,2);
							op(0xFF);	// PUSH [dword]
							op(0x30+wtok.rm);
							outaddress(&wtok);

							CheckAllMassiv(bufrm,razr,&strinf);
							op66(razr);
							outseg(&itok,2);
							op(0x8f);
							op(itok.rm);
							outaddress(&itok);
							CheckAllMassiv(wbuf,razr,&wstr,&wtok);
							op66(razr);
							outseg(&wtok,2);
							op(0x8f);
							op(wtok.rm);
							outaddress(&wtok);

							break;
						}
						getinto_reg(otok,&wtok,wbuf,&wstr,razr,hnumber);
					}
					CheckAllMassiv(rbuf,razr,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
					op66(razr);
					outseg(&itok,2);
					op(0x87);  /* XCHG AX,[wloc] */
					op(itok.rm+hnumber*8);
					outaddress(&itok);
					goto getfromax;
				case tk_seg:
					if(razr==r32)swaperror();
					op66(r16);
					op(0x8C); /* MOV AX,seg */
					op(0xC0+(unsigned int)itok.number*8);
					CheckAllMassiv(wbuf,2,&wstr,&wtok);
					op66(r16);
					outseg(&wtok,2);
					op(0x87);  /* XCHG AX,[wloc] */
					op(wtok.rm);
					outaddress(&wtok);
					op66(r16);
					op(0x8E); /* MOV seg,AX */
					op(0xC0+(unsigned int)itok.number*8);
					break;
				case tk_floatvar:
					if(razr==r16)swaperror();
					if(sign==1){
						CheckAllMassiv(wbuf,4,&wstr,&wtok);
						outseg(&wtok,2);	//fild
						op(0xDB);
						op(wtok.rm);
						outaddress(&wtok);
						CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
						outseg(&itok,2);	//fld val
						op(0xd9);
						op(itok.rm);
						outaddress(&itok);
					}
					else{
						CheckInitBP();
						op66(r32);	//push 0L
						outword(0x6a);
						CheckAllMassiv(wbuf,4,&wstr,&wtok);
						op66(r32);	//push var
						if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=8;
						addESP+=8;
						outseg(&wtok,2);
						op(0xFF);
						op(wtok.rm+0x30);
						outaddress(&wtok);
						fildq_stack();
						CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
						outseg(&itok,2);	//fld val
						op(0xd9);
						op(itok.rm);
						outaddress(&itok);
						RestoreBP();
						if(optimizespeed||am32==0){
							outword(0xC483);
							op(8);
						}
						else{
							op(0x58);	// pop EAX
							op(0x58);	// pop EAX
						}
						addESP-=8;
					}
					outseg(&wtok,2);//fistp var
					op(0xDB);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					outseg(&itok,2);	//fstp val
					op(0xd9);
					op(itok.rm+0x18);
					outaddress(&itok);
					fwait3();
					break;
				default: swaperror(); break;
			}
			break;
		case tk_rrequals:
			vop=8;
			if(sign)vop+=0x10;
		case tk_llequals:
			KillVar(wtok.name);
			getoperand(am32==TRUE?ECX:BX);
			if(itok2.type!=tp_stopper){
				doalmath(0,&ofsstr);		// all shifts unsigned byte
				ClearReg(AX);
				ClearReg(CX);
				outword(0xC188);	// MOV CL,AL
				if(addESP!=oaddESP&&am32&&ESPloc&&(wtok.type==tp_paramvar||wtok.type==tp_localvar))wtok.number+=addESP-oaddESP;
				CheckAllMassiv(wbuf,razr,&wstr,&wtok);
				op66(razr);
				outseg(&wtok,2);
				op(0xD3);	op(0x20+vop+wtok.rm);  // SHL [rmword],CL
				outaddress(&wtok);
				warningreg(begs[1]);
				next=0;
			}
			else if(tok==tk_number){
#ifdef OPTVARCONST
				if((itok.flag&f_reloc)==0){
					initconst=UpdVarConst(&wtok,itok.lnumber,itok.rm,operand);
				}
#endif
				CheckAllMassiv(wbuf,razr,&wstr,&wtok);
				if((unsigned int)itok.number==1){
					op66(razr);
					outseg(&wtok,2);
					op(0xD1); op(0x20+vop+wtok.rm);	/* SHL [rmword],1 */
					outaddress(&wtok);
				}
				else if((unsigned int)itok.number!=0){
					if(chip<2&&razr==r16){
						getintobeg(CL,&ofsstr);
						op66(r16);
						outseg(&wtok,2);
						op(0xD3);	op(0x20+vop+wtok.rm);  /* SHL [rmword],CL */
						outaddress(&wtok);
						warningreg(begs[1]);
						ClearReg(CX);
						next=0;
					}
					else{
						op66(razr);
						outseg(&wtok,2);
						op(0xC1);	op(0x20+vop+wtok.rm);  /* SHL [rmword],imm8 */
						outaddress(&wtok);
						if(cpu<2)cpu=2;
					}
					op((unsigned int)itok.number);
				}
			}
			else{
				if(tok!=tk_beg||(unsigned int)itok.number!=CL){
					getintobeg(CL,&ofsstr);
					warningreg(begs[1]);
					ClearReg(CX);
					next=0;
				}
				CheckAllMassiv(wbuf,razr,&wstr,&wtok);
				op66(razr);
				outseg(&wtok,2);
				op(0xD3);	op(0x20+vop+wtok.rm);  /* SHL [rmword],CL */
				outaddress(&wtok);
			}
			break;
		default: operatorexpected(); break;
	}
#ifdef OPTVARCONST
	if(initconst==FALSE)ClearVarByNum(&wtok);
#endif
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	if(razr==r32&&cpu<3)cpu=3;
	return retrez;
}

int dobytevar(int sign,int terminater)	 // byte, char
{
unsigned char next=1,getfromAX=0;
unsigned int vop=0,otok,rettype,posiblret;
ITOK btok;
char *bbuf,*rbuf;
int retrez=0,pointr=0,hnumber=AL;
SINFO bstr;
int numpointr=0;
char *ofsstr=NULL;
#ifdef OPTVARCONST
int initconst=FALSE;
int operand;
#endif
unsigned int oaddESP=addESP;
	sign==0?posiblret=rettype=tk_byte:posiblret=rettype=tk_char;
	bstr=strinf;
	strinf.bufstr=NULL;
	btok=itok;
	bbuf=bufrm;
	bufrm=NULL;
	otok=tok;
	while(RmEqualReg(hnumber,itok.rm,itok.sib))hnumber++;
	nexttok();
#ifdef OPTVARCONST
	operand=tok;
#endif
	switch(tok){
		case tk_assign:
			if(!((tok2==tk_reg||tok2==tk_reg32||tok2==tk_beg)&&ScanTok3()==terminater)){
				ofsstr=GetLecsem(terminater);
			}
			if(ofsstr){
				int retreg;
				if((retreg=CheckIDZReg(ofsstr,AX,r8))!=NOINREG){
					GetEndLex(terminater);
					tok=tk_beg;
					itok.number=retreg==SKIPREG?AX:retreg;
					goto regtovar;
				}
			}
			nexttok();
			convert_type(&sign,(int *)&rettype,&pointr);
			while(tok==tk_mult){
				nexttok();
				numpointr++;
			}
			if(numpointr>itok.npointr)unuseableinput();
			if(tok2==tk_assign){
				hnumber=MultiAssign(r8,USEALLREG,numpointr);
				if(ofsstr){
					free(ofsstr);
					ofsstr=NULL;
				}
				next=0;
				goto getfromax;
			}
			if(tok==tk_pointer)cpointr(am32==TRUE?EAX:BX,numpointr);
			CheckMinusNum();
			if(itok2.type==tp_opperand){
				if(rettype!=tk_float&&tok==tk_number){	//проверка и суммирование чисел
					if(OnlyNumber(sign)){
						next=0;
						goto numbertovar;
					}
				}
				goto labl1;
			}
			else{
#ifdef OPTVARCONST
				if(tok>=tk_charvar&&tok<=tk_doublevar&&itok.npointr==0){
					if(CheckConstVar(&itok))tok=tk_number;
				}
#endif
				switch(tok){
					case tk_number:
numbertovar:
#ifdef OPTVARCONST
						if((initconst=Const2Var(&btok,itok.lnumber&0Xff,itok.rm))==FALSE){
							waralreadinitvar(btok.name,itok.number);
							initconst=TRUE;
							break;
						}
#endif
						CheckAllMassiv(bbuf,1,&bstr,&btok);
						outseg(&btok,2);
						op(0xC6);
						op(btok.rm);
						outaddress(&btok);
						op((unsigned int)itok.number);
						break;
					case tk_reg32:
					case tk_reg:
						if((unsigned int)itok.number>BX)goto labl1;
					case tk_beg:
regtovar:
						if((unsigned int)itok.number==0){
							getfromAX=1;
							hnumber=0;
						}
						else{
							KillVar(btok.name);
							AddRegVar(itok.number,r8,&btok);
							vop++;
							CheckAllMassiv(bbuf,1,&bstr,&btok);
							outseg(&btok,2);
							op(0x88);
							op((unsigned int)itok.number*8+btok.rm);
							outaddress(&btok);
						}
						break;
					case tk_seg: segbyteerror(); break;
					default:
labl1:
						if(rettype==tk_char||rettype==tk_byte){
							if(hnumber==0)retrez=doalmath(sign,&ofsstr);
							else retrez=getintobeg(hnumber,&ofsstr);
						}
						else if(rettype==tk_int||rettype==tk_word){
							if(hnumber==0)retrez=do_e_axmath(sign,r16,&ofsstr);
							else retrez=getintoreg(hnumber,r16,sign,&ofsstr);
						}
						else if(rettype==tk_float){
							doeaxfloatmath(tk_reg32);
							rettype=tk_long;
							hnumber=0;
						}
						else{
							if(hnumber==0)retrez=do_e_axmath(sign,r32,&ofsstr);
							else retrez=getintoreg(hnumber,r32,sign,&ofsstr);
						}
						getfromAX=1;
						next=0;
						break;
				}
			}
			if(getfromAX){
getfromax:
#ifdef OPTVARCONST
				initconst=CheckRegToConst(hnumber,&btok,r8);
#endif
				if(retrez==0)retrez=tk_reg;
				convert_returnvalue(posiblret,rettype);
				if(addESP!=oaddESP&&am32&&ESPloc&&(btok.type==tp_paramvar||btok.type==tp_localvar))btok.number+=addESP-oaddESP;
				if(bbuf==NULL&&bstr.bufstr==NULL&&hnumber==0&&((btok.rm==rm_d16&&btok.sib==CODE16)||(btok.rm==rm_d32&&(btok.sib==CODE32||btok.sib==0)))){
					outseg(&btok,1);
					op(0xA2); 		// MOV [byte],AL
					if(btok.post==UNDEF_OFSET){
						AddUndefOff(2,btok.name);
						btok.post=0;
					}
					if(am32==FALSE)outword(btok.number);
					else outdword(btok.number);
				}
				else{
					CheckAllMassiv(bbuf,1,&bstr,&btok);
					outseg(&btok,2);  // MOV [rmbyte],AL
					op(0x88);
					op(btok.rm+hnumber*8);
					outaddress(&btok);
				}
				if(ofsstr)IDZToReg(ofsstr,hnumber,r8);
				else ClearReg(hnumber);
				KillVar(btok.name);
				AddRegVar(hnumber,r8,&btok);
			}
			else if(vop)KillVar(btok.name);
			if(ofsstr)free(ofsstr);
			break;
		case tk_multequals:
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_stopper&&tok==tk_number){
				if(itok.number==1)break;
				if(itok.number==0){
					outword(0xB0+hnumber);
					goto getfromax;
				}
#ifdef OPTVARCONST
				if((itok.flag&f_reloc)==0){
					initconst=UpdVarConst(&btok,itok.lnumber,itok.rm,tk_mult);
				}
#endif
			}
			doalmath(sign,&ofsstr);
			hnumber=0;
			if(addESP!=oaddESP&&am32&&ESPloc&&(btok.type==tp_paramvar||btok.type==tp_localvar)){
				btok.number+=addESP-oaddESP;
				oaddESP=addESP;
			}
			CheckAllMassiv(bbuf,1,&bstr,&btok);
			outseg(&btok,2);
			op(0xF6);
			if(sign)op(0x28+btok.rm);
			else op(0x20+btok.rm);
			outaddress(&btok);
			next=0;
			KillVar(btok.name);
			goto getfromax;
		case tk_divequals:
			hnumber=0;
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_stopper){
				if(tok==tk_number){
					if(itok.number==1)break;
#ifdef OPTVARCONST
					if((itok.flag&f_reloc)==0){
						initconst=UpdVarConst(&btok,itok.lnumber,itok.rm,tk_div);
					}
#endif
				}
				getintobeg(CL,&ofsstr);
				warningreg(begs[1]);
			}
			else{
				doalmath(sign,&ofsstr);
				outword(0xC88A);	//mov Cl,al
				if(addESP!=oaddESP&&am32&&ESPloc&&(btok.type==tp_paramvar||btok.type==tp_localvar)){
					btok.number+=addESP-oaddESP;
					oaddESP=addESP;
				}
			}
			if(sign)cbw();
			else xorAHAH();
			getintoal(otok,&btok,bbuf,&bstr);
			warningreg(begs[3]);
			op(0xF6);
			if(sign)op(0xF8+CL); // IDIV CL
			else op(0xF0+CL); // DIV CL
			next=0;
			ClearReg(CX);
			KillVar(btok.name);
			goto getfromax;
		case tk_minusminus: vop=0x8;
		case tk_plusplus:
#ifdef OPTVARCONST
			initconst=UpdVarConst(&btok,1,tk_byte,tok);
#endif
			CheckAllMassiv(bbuf,1,&bstr,&btok);
			outseg(&btok,2);
			op(0xFE);
			op(vop+btok.rm);
			outaddress(&btok);
			KillVar(btok.name);
			break;
		case tk_xorequals: vop+=0x08;
		case tk_minusequals: vop+=0x08;
		case tk_andequals: vop+=0x18;
		case tk_orequals: vop+=0x08;
		case tk_plusequals:
			KillVar(btok.name);
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_opperand){
				if(tok==tk_number){
					if(OnlyNumber(sign)){
						next=0;
						otok=tok;
						tok=tk_number;
						goto num;
					}
				}
				doalmath(sign,&ofsstr);
				if(addESP!=oaddESP&&am32&&ESPloc&&(btok.type==tp_paramvar||btok.type==tp_localvar))btok.number+=addESP-oaddESP;
				CheckAllMassiv(bbuf,1,&bstr,&btok);
				outseg(&btok,2);
				op(vop); op(btok.rm);  // ADD [anybyte],AL
				outaddress(&btok);
				next=0;
				retrez=tk_reg;
			}
			else{
				switch(tok){
					case tk_number:
num:
#ifdef OPTVARCONST
						if((itok.flag&f_reloc)==0){
							initconst=UpdVarConst(&btok,itok.lnumber,itok.rm,operand);
						}
#endif
						CheckAllMassiv(bbuf,1,&bstr,&btok);
						outseg(&btok,2);
						if(itok.number==1&&(vop==0||vop==0x28)){
							if(vop)vop=8;
							op(0xFE);
							op(vop+btok.rm);
							outaddress(&btok);
						}
						else{
							op(0x80);
							op(vop+btok.rm);
							outaddress(&btok);
							op((unsigned int)itok.number);
						}
						if(next==0)tok=otok;
						break;
					case tk_beg:
#ifdef OPTVARCONST
						initconst=CheckUpdRegToConst(itok.number,&btok,operand,r8);
#endif
						CheckAllMassiv(bbuf,1,&bstr,&btok);
						outseg(&btok,2);
						op(vop);
						op((unsigned int)itok.number*8+btok.rm);
						outaddress(&btok);
						break;
					case tk_seg: segbyteerror(); break;
					default:
						retrez=tk_reg;
						doalmath(sign,&ofsstr);
						CheckAllMassiv(bbuf,1,&bstr,&btok);
						outseg(&btok,2);
						op(vop); op(btok.rm);  /* ADD [anybyte],AL */
						outaddress(&btok);
						next=0;
						break;
				}
			}
			break;
		case tk_swap:
			KillVar(btok.name);
			getoperand();
			rbuf=bufrm;
			bufrm=NULL;
			switch(tok){
				case tk_beg:
#ifdef OPTVARCONST
					initconst=SwapVarRegConst(itok.number,&btok,r8);
#endif
					CheckAllMassiv(bbuf,1,&bstr,&btok);
					outseg(&btok,2);
					op(0x86); 	/* XCHG beg,[anybloc] */
					op((unsigned int)itok.number*8+btok.rm);
					outaddress(&btok);
					ClearReg(itok.number>3?itok.number-4:itok.number);
					break;
				case tk_bytevar:
				case tk_charvar:
#ifdef OPTVARCONST
					initconst=SwapVarConst(&itok,&btok);
#endif
					if(hnumber==0)getintoal(otok,&btok,bbuf,&bstr);
					else getinto_reg(otok,&btok,bbuf,&bstr,r8,hnumber);
					CheckAllMassiv(rbuf,1,&strinf,&itok,(am32!=FALSE&&bbuf!=NULL&&bstr.bufstr!=NULL)?BX:DI,DX);
					outseg(&itok,2);
					op(0x86);	 /* XCHG AL,[bloc] */
					op(itok.rm+hnumber*8);
					outaddress(&itok);
					KillVar(itok.name);
					goto getfromax;
				default: swaperror(); break;
			}
			break;
		case tk_rrequals:
			vop=8;
			if(sign)vop+=0x10;
		case tk_llequals:
			KillVar(btok.name);
			getoperand(am32==TRUE?ECX:BX);
			if(itok2.type!=tp_stopper){
				doalmath(0,&ofsstr);		// all shifts unsigned byte
				outword(0xC188);	// MOV CL,AL
				if(addESP!=oaddESP&&am32&&ESPloc&&(btok.type==tp_paramvar||btok.type==tp_localvar))btok.number+=addESP-oaddESP;
				CheckAllMassiv(bbuf,1,&bstr,&btok);
				outseg(&btok,2);
				op(0xD2);	op(0x20+vop+btok.rm);  /* SHL [byte],CL */
				outaddress(&btok);
				warningreg(begs[1]);
				ClearReg(CX);
				ClearReg(AX);
				next=0;
			}
			else if(tok==tk_number){
#ifdef OPTVARCONST
				if((itok.flag&f_reloc)==0){
						initconst=UpdVarConst(&btok,itok.lnumber,itok.rm,operand);
				}
#endif
				if((unsigned int)itok.number==1){
					CheckAllMassiv(bbuf,1,&bstr,&btok);
					outseg(&btok,2);
					op(0xD0);	op(0x20+vop+btok.rm);  /* SHL [byte],1 */
					outaddress(&btok);
				}
				else if((unsigned int)itok.number!=0){
					CheckAllMassiv(bbuf,1,&bstr,&btok);
					if(chip<2){
						getintobeg(CL,&ofsstr);
						outseg(&btok,2);
						op(0xD2);	op(0x20+vop+btok.rm);  /* SHL [byte],CL */
						outaddress(&btok);
						warningreg(begs[1]);
						ClearReg(CX);
						next=0;
					}
					else{
						outseg(&btok,2);
						op(0xC0);	op(0x20+vop+btok.rm);  /* SHL [byte],imm8 */
						outaddress(&btok);
						if(cpu<2)cpu=2;
					}
					op((unsigned int)itok.number);
				}
			}
			else{
				if(tok!=tk_beg||(unsigned int)itok.number!=CL){
					getintobeg(CL,&ofsstr);
					warningreg(begs[1]);
					ClearReg(CX);
					next=0;
				}
				CheckAllMassiv(bbuf,1,&bstr,&btok);
				outseg(&btok,2);
				op(0xD2);	op(0x20+vop+btok.rm);  /* SHL [byte],CL */
				outaddress(&btok);
			}
			break;
		default: operatorexpected(); break;
	}
#ifdef OPTVARCONST
	if(initconst==FALSE)ClearVarByNum(&btok);
#endif
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	return retrez;
}

void  getinto_reg(int gtok,ITOK *gstok,char *&gbuf,SINFO *gstr,int razr,int reg)
{
unsigned int i=0;
int vop=0;
int reg1=SI,reg2=DI;
	switch(gtok){
		case tk_dwordvar:
		case tk_longvar:
			i+=4;
longvar:
			CheckAllMassiv(gbuf,i,gstr,gstok,reg1,reg2);
			op66(razr);
			outseg(gstok,2);
			op(0x8B);
			op(gstok->rm+reg*8);
			outaddress(gstok);
			ClearReg(reg);
			break;
		case tk_intvar:
			vop=8;
		case tk_wordvar:
			i=2;
			goto longvar;
		case tk_bytevar:
		case tk_charvar:
			CheckAllMassiv(gbuf,i,gstr,gstok,reg1,reg2);
			outseg(gstok,2);
			op(0x8A);
			op(gstok->rm+reg*8);
			outaddress(gstok);
			ClearReg(reg);
			break;
	}
}

void  getinto_e_ax(int sign,int gtok,ITOK *gstok,char *&gbuf,SINFO *gstr,int razr,int useAX)
{
unsigned int i=0;
int vop=0;
int reg1=idxregs[0],reg2=idxregs[1];
	if(am32){
		reg1=useAX==FALSE?EAX:idxregs[0];
		reg2=idxregs[3];
	}
//	printf("tok=%u %s\n",gtok,gstok->name);
	switch(gtok){
		case tk_bits:
			vop=gstok->bit.siz+gstok->bit.ofs;
			if(vop<=64)i=r64;
			if(vop<=32)i=r32;
			if(vop<=16)i=r16;
			bits2reg(AX,((unsigned int)razr>i?razr:i));
			break;
		case tk_postnumber:
			op66(razr);
			op(0xB8); /* MOV EAX,# */
			(gstok->flag&f_extern)==0?setwordpost(gstok):setwordext(&gstok->number);
			razr==r16?outword(gstok->number):outdword(gstok->number);
			ClearReg(AX);
			break;
		case tk_rmnumber:
			CheckAllMassiv(gbuf,gstok->size,gstr,gstok,reg1,reg2);
			if(gstok->rm||am32==0){
				op66(razr);
				op67(gstok->sib==CODE16?r16:r32);
				if(gstok->post==0)outseg(gstok,2);
				op(0x8D); op(gstok->rm);
				if(gstok->post!=0&&gstok->post!=UNDEF_OFSET){
					if((gstok->flag&f_extern)==0){
						i=outptr;
						if(am32&&gstok->rm==rm_sib)outptr++;
						setwordpost(gstok);
						outptr=i;
					}
					else setwordext(&gstok->number);
				}
				outaddress(gstok); /* LEA AX,[rm] */
				ClearReg(AX);
			}
			else nexttok();
			break;
		case tk_doublevar:
			vop=4;
		case tk_floatvar:
			CheckInitBP();
			if(cpu<3)cpu=3;
			op66(r32);
			op(0x50);  //push EAX
			if(ESPloc&&am32&&gstok->segm==SS)gstok->number+=4;
			addESP+=4;
			CheckAllMassiv(gbuf,4,gstr,gstok,reg1,reg2);
			outseg(gstok,2);	//fld floatvar
			op(0xd9+vop);
			op(gstok->rm);
			outaddress(gstok);
			fistp_stack();
			op66(r32);
			op(0x58);	//pop EAX
			addESP-=4;
			RestoreBP();
			ClearReg(AX);
			break;
		case tk_qwordvar:
			i=4;
		case tk_dwordvar:
		case tk_longvar:
			i+=4;
longvar:
			if((gstok->rm==rm_d16&&gstok->sib==CODE16)||(gstok->rm==rm_d32&&(gstok->sib==CODE32||gstok->sib==0))){
				op66(razr);
				outseg(gstok,1);
				op(0xA1);
				if(gstok->post==UNDEF_OFSET)AddUndefOff(2,gstok->name);
				if(am32==FALSE)outword((unsigned int)gstok->number);
				else outdword(gstok->number);
			}
			else{
				CheckAllMassiv(gbuf,i,gstr,gstok,reg1,reg2);
				op66(razr);
				outseg(gstok,2);
				op(0x8B);
				op(gstok->rm);
				outaddress(gstok);
			}
			ClearReg(AX);
			break;
		case tk_intvar:
			vop=8;
		case tk_wordvar:
			i=2;
			if(razr==r16)goto longvar;
			i=1;
			if(optimizespeed&&vop==0&&chip>3&&chip<7)goto optpent;
movxx:
			CheckAllMassiv(gbuf,1+i,gstr,gstok,reg1,reg2);
			op66(razr);
			outseg(gstok,3);	/* MOVSX EAX,[charvar] */
			op(0x0F); op(0xB6+vop+i); op(gstok->rm);
			outaddress(gstok);
			ClearReg(AX);
			break;
		case tk_charvar:
			vop=8;
			if(razr==r16){
				if((gstok->rm==rm_d16&&gstok->sib==CODE16)||(gstok->rm==rm_d32&&(gstok->sib==CODE32||gstok->sib==0))){
					outseg(gstok,1);
					op(0xA0);
					if(am32==FALSE)outword(gstok->number);
					else outdword(gstok->number);
				}
				else{
					CheckAllMassiv(gbuf,1,gstr,gstok,reg1,reg2);
					outseg(gstok,2);
					op(0x8A); op(gstok->rm);
					outaddress(gstok);
				}
				cbw();
				ClearReg(AX);
				break;
			}
			goto movxx;
		case tk_bytevar:
optpent:
			if((gstok->rm==rm_d16&&gstok->sib==CODE16)||(gstok->rm==rm_d32&&(gstok->sib==CODE32||gstok->sib==0))){
				ZeroReg(EAX,razr);
				if(i)op66(r16);
				outseg(gstok,1);
				op(0xA0+i);
				if(am32==FALSE)outword(gstok->number);
				else outdword(gstok->number);
				ClearReg(AX);
				break;
			}
			if((chip>=3&&(!optimizespeed))||RmEqualReg(AX,gstok->rm,gstok->sib))goto movxx;
			ZeroReg(EAX,razr);
			CheckAllMassiv(gbuf,1+i,gstr,gstok,SI,reg2);
			if(i)op66(r16);
			outseg(gstok,2);
			op(0x8A+i); op(gstok->rm);
			outaddress(gstok);
			break;
		case tk_beg:
			if(gstok->number==AL&&(!sign)&&razr==r16){
				xorAHAH();
				ClearReg(AX);
				break;
			}
			if(optimizespeed&&chip>3&&chip<7){
				if(sign){
					if(razr==r32)goto movxxr;
					if(gstok->number!=AL){
						op(0x88);
						op(0xC0+gstok->number*8);	//mov al,beg
					}
					if(gstok->number!=AH)outword(0xC488);	//mov ah,al
					outword(0xFCC0);	//sar ah,7
					op(7);
				}
				else{
					if(razr==r32&&(gstok->number==AL||gstok->number==AH)){
						/*if(chip==4)*/goto movxxr;
/*						op(0x88);
						op(0xC1+gstok->number*8);	//mov cl,beg
						xorEAXEAX();
						outword(0xC888);	// mov al,cl
						warningreg(begs[1]);
						break;*/
					}
					if(gstok->number==AH)outdword(0xE430E088);	//mov al,ah xor ah,ah
					else{
						ZeroReg(EAX,razr);
						op(0x88);
						op(0xC0+gstok->number*8);	//mov al,beg
					}
				}
				ClearReg(AX);
				break;
			}
movxxr:
			if(chip>2||razr==r32){
				op66(razr);
				if(sign)outword(0xBE0F);
				else outword(0xB60F);
				op(0xC0+(unsigned int)gstok->number); // MOVxX AX,beg
			}
			else{
				op(0x88);
				op(0xC0+gstok->number*8);
				if(sign)cbw();
				else xorAHAH();
			}
			ClearReg(AX);
			break;
		case tk_reg:
			if(razr==r32){
				if(tok2==tk_openbracket){	//вызов процедуры по адресу в регистре
					reg1=gstok->number;
					nexttok();
					param[0]=0;
					if(comfile==file_w32)swapparam();
					else doparams();
					op66(r16);
					op(0xFF);
					op(0xD0+reg1); 	/* CALL reg with stack params */
#ifdef OPTVARCONST
					FreeGlobalConst();
#endif
					clearregstat();
					break;
				}
				if(optimizespeed&&(!sign)&&chip>3&&chip<7){
					if(gstok->number==AX){
						goto movxr;
/*						op66(r16);
						outword(0xC189);	//mov cx,AX
						xorEAXEAX();
						op66(r16);
						outword(0xC889);	//mov aX,cX
						warningreg(regs[0][1]);*/
					}
					else{
						xorEAXEAX();
						op66(r16);
						op(0x89);
						op(0xC0+gstok->number*8);	//mov ax,reg
					}
				}
				else{
movxr:
					op66(r32);
					op(0x0F);	/* MOVSX or MOVZX EAX,reg */
					if(sign)op(0xBF);
					else op(0xB7);
					op(0xC0+gstok->number);
				}
				RegToReg(AX,gstok->number,razr);
				break;
			}
		case tk_reg32:
			if(tok2==tk_openbracket){	//вызов процедуры по адресу в регистре
				reg1=gstok->number;
				nexttok();
				param[0]=0;
				if(comfile==file_w32)swapparam();
				else doparams();
				op66(razr);
				op(0xFF);
				op(0xD0+reg1); 	/* CALL reg with stack params */
				clearregstat();
#ifdef OPTVARCONST
				FreeGlobalConst();
#endif
				break;
			}
			if(gstok->number!=AX){
				op66(razr);
				op(0x89);
				op(0xC0+gstok->number*8);
				RegToReg(AX,gstok->number,razr);
			}
			break;
		case tk_seg:
			if(razr==r32)op66(r32);
			op(0x8C);	//mov var,SS
			op(0xC0+gstok->number*8);
			ClearReg(AX);
			break;
		case tk_string:
			op66(razr);
			op(0xB8);
			if(razr==r16){
				if(am32)dwordvalexpected();
				outword(addpoststring(CS,gstok->number,gstok->flag));
			}
			else outdword(addpoststring(CS,gstok->number,gstok->flag));
			ClearReg(AX);
			break;
		default: valueexpected();	break;
	}
	if(razr==r32&&cpu<3)cpu=3;
}

int caselong(unsigned long val)
{
int vop=0;
unsigned long num;
	for(;vop<NUMNUM;vop++){
		num=li[vop];
		if(val==num)break;
		if(val<num)return NUMNUM;
	}
	return vop;
}

int caselonglong(unsigned long long val)
{
int vop=0;
unsigned long long num;
	for(;vop<NUMNUM64;vop++){
		num=li[vop];
		if(val==num)break;
		if(val<num)return NUMNUM64;
	}
	return vop;
}

int do_e_axmath(int sign,int razr,char **ofsstr)
{
int negflag=0,next=0;
int expand=FALSE,rettype;
unsigned int i=0;
unsigned long holdnumber=0;
int loop=0,otok;
	rettype=(razr==r16?tk_reg:tk_reg32);
	if(tok==tk_minus){
		if(CheckMinusNum()==FALSE){
			negflag=1;
			getoperand(am32==TRUE?EAX:BX);
		}
	}
	if(uselea){
		if(razr==r32){
			if(Reg32ToLea(EAX)){
				goto contloop;	//оптимизация сложения 32-битных регистров в LEA
			}
		}
		else if(Reg16ToLea(AX)){
			goto contloop;	//оптимизация сложения 32-битных регистров
		}
	}
loopswitch:
	otok=tok;
//	printf("tok=%d %s\n",tok,itok.name);
#ifdef OPTVARCONST
	CheckConstVar3(&tok,&itok,razr);
	if(tok==tk_number)calcnumber=TRUE;
#endif
	switch(tok){
		case tk_number:
			holdnumber=CalcNumber(sign);
			if(loop==0)i=postnumflag;
			else i^=postnumflag;
			loop++;
			if(tok==tk_mult&&(optimizespeed||razr==r32)&&
					(!((tok2==tk_reg32||tok2==tk_reg)&&itok2.number==EAX))&&expandvar()==FALSE){
		 		getoperand(am32==TRUE?EAX:BX);
				next=1;
				goto loopswitch;
			}
			if(tok==tk_plus&&tok2==tk_postnumber){
				nexttok();
				goto loopswitch;
			}
			MovRegNum(razr,i&f_reloc,holdnumber,EAX);
			next=0;
			break;
		case tk_apioffset:
			op66(razr);
			op(0xB8);			/* MOV AX,# */
			AddApiToPost(itok.number);
			nexttok();
			break;
		case tk_postnumber:
		case tk_undefofs:
			if(next==0){
				op66(razr);
				op(0xB8);			/* MOV AX,# */
				next=1;
			}
			if(tok==tk_undefofs){
				AddUndefOff(2,itok.name);
//				AddUndefOff(0,itok.name);	//22.11.04 20:52
//				itok.flag=0;	//new 07.07.04 23:57
			}
			else (itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
			tok=tk_number;
			holdnumber+=doconstdwordmath();
			if(otok!=tk_postnumber){
				if(loop==0)i=postnumflag;
				else i^=postnumflag;
				loop++;
			}
			if(tok==tk_plus&&tok2==tk_postnumber){
				nexttok();
				goto loopswitch;
			}
			if((i&f_reloc)!=0)AddReloc();
			next=0;
			if(razr==r16)outword(holdnumber);
			else outdword(holdnumber);
			ClearReg(AX);
			break;
		case tk_rmnumber:
int reg1,reg2;
			reg1=am32==FALSE?idxregs[0]:EAX;
			reg2=am32==FALSE?idxregs[1]:ECX;
			CheckAllMassiv(bufrm,itok.size,&strinf,&itok,reg1,reg2);
			if(itok.rm||am32==0){
				op66(razr);
				op67(itok.sib==CODE16?r16:r32);
				if((itok.rm&0x3F)==0&&am32&&itok.post==0&&(short_ok(itok.number,TRUE)==FALSE||((itok.rm&rm_mod11)==rm_mod10))){	//add
					outseg(&itok,1);
					op(5);	//add
				}
				else{	//lea
					if(itok.post==0)outseg(&itok,2);
					op(0x8D); op(itok.rm);
				}
//				if(itok.post==0)outseg(&itok,2);
//				op(0x8D); op(itok.rm);
				if(itok.post!=0&&itok.post!=UNDEF_OFSET){
					if((itok.flag&f_extern)==0){
						i=outptr;
						if(am32&&itok.rm==rm_sib)outptr++;
						setwordpost(&itok);
						outptr=i;
					}
					else setwordext(&itok.number);
				}
ITOK oitok;
				oitok=itok;
				tok=tk_number;
				itok.rm=tk_dword;
				oitok.number=doconstdwordmath();
//			oitok.flag|=postnumflag;
				outaddress(&oitok); // LEA AX,[rm]
				ClearReg(AX);
			}
			else nexttok();
			break;
		case tk_at:
		 	getoperand(am32==TRUE?EAX:BX);
			i++;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			if(itok.flag&f_retproc)rettype=(itok.flag&f_retproc)/256+tk_overflowflag-1;
			if((!i)||
					macros(sign!=0?(razr==r16?tk_int:tk_long):(razr==r16?tk_word:tk_dword))==0){
				procdo(sign!=0?(razr==r16?tk_int:tk_long):(razr==r16?tk_word:tk_dword));
			}
			nexttok();
			if(*ofsstr){
				free(*ofsstr);
				*ofsstr=NULL;
			}
			break;
		case tk_new:
			donew();
			clearregstat();
#ifdef OPTVARCONST
			FreeGlobalConst();
#endif
			if(*ofsstr){
				free(*ofsstr);
				*ofsstr=NULL;
			}
			nexttok();
			break;
		default:
			SINFO wstr=strinf;
			ITOK witok=itok;
			char *wbuf=bufrm;
			bufrm=NULL;
			strinf.bufstr=NULL;
			getinto_e_ax(sign,tok,&witok,wbuf,&wstr,razr);
			nexttok(); break;
	}
contloop:
	if(negflag){
		NegReg(razr,EAX);
		setzeroflag=TRUE;
	}
#ifdef OPTVARCONST
	calcnumber=FALSE;
#endif
	if(next)RegMulNum(AX,holdnumber,razr,sign,&expand,i);
//	printf("tok=%d type=%d name=%s\n",tok,itok.type,itok.name);
	if(itok.type!=tp_stopper&&tok!=tk_eof&&itok.type!=tp_compare){
		do_e_axmath2(sign,razr,expand);
		rettype=(razr==r16?tk_reg:tk_reg32);
	}
	return rettype;
}

void do_e_axmath2(int sign,int razr,int expand)
{
int vop,negflag=0,next;
int optnum=FALSE;
unsigned int i;
char *ofsstr=NULL;
unsigned char oaddstack;
	while(itok.type!=tp_stopper&&tok!=tk_eof&&itok.type!=tp_compare){
		next=1;
		vop=0;
		i=0;
// !!! new
#ifdef OPTVARCONST
		CheckConstVar3(&tok2,&itok2,razr);
		if(tok2==tk_number)calcnumber=TRUE;
#endif
		if(uselea&&razr==r32){
			if(Reg32ToLea2(EAX))continue;	//оптимизация сложения 32-битных регистров в LEA
			if(itok.type==tp_stopper||tok==tk_eof||itok.type==tp_compare)break;
		}

		if(tok2==tk_number)optnum=OptimNum();
		int oldtok=tok;
		switch(tok){
			case tk_xor: vop+=0x08;
			case tk_minus: vop+=0x08;
			case tk_and: vop+=0x18;
			case tk_or: vop+=0x08;
			case tk_plus:
			  if(optnum==FALSE)getoperand();
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				switch(tok){
					case tk_number:
						if((itok.flag&f_reloc)==0){
							if(optnumadd(itok.number,0,razr,vop))break;
						}
					case tk_undefofs:
					case tk_postnumber:
						op66(razr);
					  op(0x05+vop);
						if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
						else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
						else if((itok.flag&f_reloc)!=0)AddReloc();
						razr==r16?outword((unsigned int)itok.number):outdword(itok.number);
						setzeroflag=TRUE;
						break;
					case tk_apioffset:
						op66(razr);
					  op(0x05+vop);
						AddApiToPost(itok.number);
						setzeroflag=TRUE;
						break;
					case tk_qwordvar:
					case tk_longvar:
					case tk_dwordvar:
						i=2;
					case tk_intvar:
					case tk_wordvar:
						if(razr==r32&&(tok==tk_intvar||tok==tk_wordvar))goto defxor;
						i+=2;
						CheckAllMassiv(bufrm,i,&strinf);
						op66(razr);
						outseg(&itok,2);
						op(0x03+vop);
						op(itok.rm);
						outaddress(&itok);
						setzeroflag=TRUE;
						break;
					case tk_doublevar:
						i=4;
					case tk_floatvar:
						Float2reg32(EDX,i);
						itok.number=EDX;
						warningreg(regs[1][EDX]);
						goto defreg32;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
						op66(razr);
						op(0x50);	//push AX
						addESP+=razr==r16?2:4;
						oaddstack=addstack;
						addstack=FALSE;
						procdo(razr==r16?(sign==0?tk_word:tk_int):(sign==0?tk_dword:tk_long));
						addstack=oaddstack;
						addESP-=razr==r16?2:4;
						op66(razr);
						op(0x58+EDX);
						itok.number=EDX;
						warningreg(regs[razr/2-1][EDX]);
						if(vop>0x20){
							op66(razr);
							op(0x90+EDX);	//xchg ax,dx
						}
						goto defreg32;
					case tk_bits:
						int vops;
						i=itok.bit.siz+itok.bit.ofs;
						if(i<=64)vops=r64;
						if(i<=32)vops=r32;
						if(i<=16)vops=r16;
						bits2reg(CX,(razr<vops?vops:razr));
						itok.number=CX;
						if(vops==r64)vops=r32;
						warningreg(regs[vops/2-1][ECX]);
						goto defreg32;
					case tk_reg:
						if(razr==r32)goto defxor;
					case tk_reg32:
defreg32:
						op66(razr);
						op(0x01+vop);
						op(0xC0+(unsigned int)itok.number*8);
						setzeroflag=TRUE;
						break;
					case tk_rmnumber:
					case tk_charvar:
					case tk_beg:
					case tk_bytevar:
defxor:
						getintoreg_32(CX,razr,sign,&ofsstr,FALSE);
						op66(razr);
						op(0x01+vop);
						op(0xC8);	/* OPT AX,CX */
						warningreg(regs[razr/2-1][1]);
						next=0;
						setzeroflag=TRUE;
						break;
					default: valueexpected(); break;
				}
				if(expand==TRUE){
					op66(razr);
					op(0x83);
					if(oldtok==tk_plus)outword(0x00d2);	//adc dx,0
					else if(oldtok==tk_minus)outword(0x00da);	//sbb dx,0
					setzeroflag=TRUE;
				}
				break;
			case tk_modminus: negflag=1;
			case tk_mod: negflag=1-negflag; vop=1;
			case tk_divminus: negflag=1-negflag;
			case tk_div:
			  if(optnum==FALSE)getoperand();
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				if(tok==tk_number){
					if(negflag){
						itok.number=-itok.number;
						negflag=0;
					}
				}
				DivMod(vop,sign,razr,expand);
				if(vop==1&&setzeroflag==0){
					op66(razr);
					if(optimizespeed)outword(0xC28B);	//mov ax,dx
					else op(0x92);	//xchg ax,dx
				}
				next=0;
				expand=FALSE;
				break;
			case tk_multminus: negflag=1;
			case tk_mult:
				expand=expandvar();	//возможность расширения разрядности
			  if(optnum==FALSE)getoperand();
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				if(negflag&&tok==tk_number){
					itok.number=-itok.number;
					negflag=0;
				}
				switch(tok){
					case tk_number:
						RegMulNum(AX,itok.number,razr,sign,&expand,itok.flag);
						break;
					case tk_qwordvar:
					case tk_longvar:
					case tk_dwordvar:
						i=2;
					case tk_intvar:
					case tk_wordvar:
						if(razr==r32&&(tok==tk_intvar||tok==tk_wordvar))goto defmul;
						i+=2;
						CheckAllMassiv(bufrm,i,&strinf);
					 	op66(razr);
						outseg(&itok,2);
						op(0xF7);	//imul var
						if(sign)op(0x28+itok.rm);
						else op(0x20+itok.rm);
						outaddress(&itok);
						setzeroflag=FALSE;
						break;
					case tk_doublevar:
						i=4;
					case tk_floatvar:
						Float2reg32(EDX,i);
					 	op66(razr);
						op(0xF7);
						op(0xE8+EDX); // IMUL EDX
						setzeroflag=FALSE;
						warningreg(regs[1][EDX]);
						break;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
						op66(razr);
						op(0x50);	//push AX
						addESP+=razr==r16?2:4;
						oaddstack=addstack;
						addstack=FALSE;
						procdo(razr==r16?(sign==0?tk_word:tk_int):(sign==0?tk_dword:tk_long));
						addstack=oaddstack;
						addESP-=razr==r16?2:4;
						op66(razr);
						op(0x58+EDX);
						itok.number=EDX;
						warningreg(regs[razr/2-1][EDX]);
						goto mulreg32;
					case tk_bits:
						int vops;
						i=itok.bit.siz+itok.bit.ofs;
						if(i<=64)vops=r64;
						if(i<=32)vops=r32;
						if(i<=16)vops=r16;
						bits2reg(CX,(razr<vops?vops:razr));
						itok.number=CX;
						if(vops==r64)vops=r32;
						warningreg(regs[vops/2-1][ECX]);
						goto mulreg32;
					case tk_reg:
						i=itok.number;
						if(razr==r32)goto mulreg;
					case tk_reg32:
mulreg32:
						op66(razr);
						op(0xF7);
						if(sign)op(0xE8+(unsigned int)itok.number);
						else op(0xE0+(unsigned int)itok.number);
						setzeroflag=FALSE;
						break;
					case tk_postnumber:
					case tk_undefofs:
						op66(razr);
						op(0x69);
						op(0xc0);
						if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
						else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
						razr==r16?outword(itok.number):outdword(itok.number);
						setzeroflag=FALSE;
						break;
					case tk_apioffset:
						op66(razr);
						op(0x69);
						op(0xc0);
						AddApiToPost(itok.number);
						setzeroflag=FALSE;
						break;
					case tk_seg:
					case tk_charvar:
					case tk_beg:
					case tk_bytevar:
					case tk_rmnumber:
defmul:
						i=EDX;
mulreg:
						getintoreg_32(i,razr,sign,&ofsstr,FALSE);
					 	op66(razr);
						op(0xF7);
						if(sign)op(0xE8+i);  /* IMUL i */
						else op(0xE0+i); /* MUL i */
						next=0;
						setzeroflag=FALSE;
						warningreg(regs[razr/2-1][2]);
						if(i!=EDX)warningreg(regs[razr/2-1][i]);
						break;
					default: valueexpected();	break;
				}
				break;
			case tk_xorminus: vop+=0x10;
			case tk_andminus: vop+=0x18;
			case tk_orminus: vop+=0x08;
			  getoperand();
				if(tok==tk_number){
					itok.number=-itok.number;
				 	op66(razr);
					op(0x05+vop);
					if((itok.flag&f_reloc)!=0)AddReloc();
					razr==r16?outword((unsigned int)itok.number):outdword(itok.number);
				}
				else{
					getintoreg_32(CX,razr,sign,&ofsstr,FALSE);
					NegReg(razr,ECX);
				 	op66(razr);
					op(0x01+vop);	/* opt AX,CX */
					op(0xC8);
					warningreg(regs[razr/2-1][1]);
					next=0;
				}
				setzeroflag=TRUE;
				break;
			case tk_ll:
			  getoperand();
				if(tok==tk_number){
					if(expand==TRUE){
						if(chip>2||razr==r32){
						 	op66(razr);
							op(0x0f);
							outword(0xC2a4);	//SHLD DX,AX,num
							op(itok.number);
						}
						else{
							if((unsigned int)itok.number==1)outdword(0xd213c001);	//ADD AX,AX ADC DX,DX
							else if((unsigned int)itok.number!=0){
								getintobeg(CL,&ofsstr);
								outdword(0xd213c001);	//ADD AX,AX ADC DX,DX
								outword(0xfae2);  //LOOP -6
								warningreg(begs[1]);
								next=0;
							}
							break;
						}
					}
					lshiftmul(itok.number,razr);
					setzeroflag=TRUE;
				}
				else goto llminus;
				break;
			case tk_llminus:
				tok=tk_minus; 	// do optimization 286+ here later
llminus:
				if(!((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==1)){
					getintobeg(CL,&ofsstr);
			 		warningreg(begs[1]);
				}
				else getoperand();
				next=0;
				if(expand==TRUE){
					if(chip>2||razr==r32){
						op66(razr);
						op(0x0f);
						outword(0xC2a5);	//SHLD DX,AX,CL
					}
					else
						outdword(0xd213c001);	//ADD AX,AX ADC DX,DX
						outword(0xfae2);  //LOOP -6
						break;
				}
				op66(razr);
				outword(0xE0D3);	/* SHL AX,CL */
				setzeroflag=TRUE;
				break;
			case tk_rr:
				if(sign)vop=0x10;
				getoperand();
				if(expand==TRUE){
					if(tok==tk_number){
						if((unsigned int)itok.number==1){
							op66(razr);
							outword(0xead1);	//shr dx,1 ror ax,1
							op66(razr);
							outword(0xc8d1);	//shr dx,1 ror ax,1
							setzeroflag=TRUE;
						}
						else if((unsigned int)itok.number!=0){
							if(chip>2||razr==r32){
						 		op66(razr);
								op(0x0f);
								outword(0xd0ac);	//shrd ax,dx,num
								op(itok.number);
						 		op66(razr);
								op(0xc1); op(0xea+vop);//s?r dx,num
								op(itok.number);
								setzeroflag=TRUE;
							}
							else{
								next=0;
								getintobeg(CL,&ofsstr);
								warningreg(begs[1]);
								op(0xd1); op(0xea+vop);//s?r dx,1
								outdword(0xfae2d8d1);  //rcr ax,1 LOOP -6
								setzeroflag=FALSE;
							}
						}
					}
					else goto rrminus;
				}
				else{
					RshiftReg(razr,EAX,vop);
					setzeroflag=TRUE;
					next=0;
				}
				break;
			case tk_rrminus:
				if(sign)vop=0x10;
				tok=tk_minus;  // do optimization 286+ here later
rrminus:
				if(!((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==1)){
					getintobeg(CL,&ofsstr);
			 		warningreg(begs[1]);
				}
				else getoperand();
				if(expand==TRUE){
					if(chip>2||razr==r32){
			 			op66(razr);
						op(0x0f);
						outword(0xd0ad);	//shrd ax,dx,cl
			 			op66(razr);
						op(0xd3); op(0xea+vop);//s?r dx,num
						setzeroflag=TRUE;
					}
					else{
						op(0xd1); op(0xea+vop);//s?r dx,1
						outdword(0xfae2d8d1);  //rcr ax,1 //LOOP -6
						setzeroflag=FALSE;
					}
				}
				else{
		 			op66(razr);
					op(0xD3);	op(0xE8+vop);  /* SR AX,CL */
					setzeroflag=TRUE;
				}
				next=0;
				break;
			default: operatorexpected(); break;
		}
		calcnumber=FALSE;
		ClearReg(EAX);
		if(negflag){
			NegReg(razr,EAX);
			setzeroflag=TRUE;
			negflag=0;
		}
		if(next)nexttok();
	}
	calcnumber=FALSE;
	ClearReg(EAX);
	if(tok==tk_eof)unexpectedeof();
	if(razr==r32&&cpu<3)cpu=3;
}

void  getintoal(int gtok,ITOK *gstok,char *&gbuf,SINFO *gstr) // AH may also be changed
{
unsigned int i=0;
	switch(gtok){
		case tk_bits:
			int razr;
			i=gstok->bit.siz+gstok->bit.ofs;
			if(i<=64)razr=r64;
			if(i<=32)razr=r32;
			if(i<=16)razr=r16;
			if(i<=8)razr=r8;
			bits2reg(AL,razr);
			break;
		case tk_number:
			op(0xB0); 	/* MOV AL,# */
			op(gstok->number);
			ConstToReg(gstok->number,AL,r8);
			break;
		case tk_rmnumber:
			CheckAllMassiv(gbuf,gstok->size,gstr,gstok);
 			op66(r16);
			op67(gstok->sib==CODE16?r16:r32);
			if(gstok->post==0)outseg(gstok,2);
			op(0x8D);	/* LEA AX,[rm] */
			op(gstok->rm);
			if(gstok->post!=0&&gstok->post!=UNDEF_OFSET){
				if((gstok->flag&f_extern)==0){
					i=outptr;
					if(am32&&gstok->rm==rm_sib)outptr++;
					setwordpost(gstok);
					outptr=i;
				}
				else setwordext(&gstok->number);
			}
			outaddress(gstok);
			ClearReg(AL);
			break;
		case tk_postnumber:
 			op66(r16);
			op(0xB8);	/* MOV AX,# */
			(gstok->flag&f_extern)==0?setwordpost(gstok):setwordext(&gstok->number);
			outword(gstok->number);
			ClearReg(AL);
			break;
		case tk_floatvar:
			if(cpu<3)cpu=3;
			CheckInitBP();
			op66(r32);
			outword(0x6a);  //push 0
			if(ESPloc&&am32&&gstok->segm==SS)gstok->number+=4;
			addESP+=4;
			CheckAllMassiv(gbuf,4,gstr,gstok);
			outseg(gstok,2);	//fld floatvar
			op(0xd9);
			op(gstok->rm);
			outaddress(gstok);
			fistp_stack();
			op66(r32);
			op(0x58);	//pop EAX
			addESP-=4;
			RestoreBP();
			ClearReg(AL);
			break;
		case tk_qwordvar:
			i=4;
		case tk_longvar:
		case tk_dwordvar:
			i+=2;
		case tk_intvar:
		case tk_wordvar:
			i++;
		case tk_bytevar:
		case tk_charvar:
			i++;
			if((gstok->rm==rm_d16&&gstok->sib==CODE16)||(gstok->rm==rm_d32&&(gstok->sib==CODE32||gstok->sib==0))){
				outseg(gstok,1);
				op(0xA0); 	//mov AL,
				if(gstok->post==UNDEF_OFSET)AddUndefOff(2,gstok->name);
				if(am32==FALSE)outword(gstok->number);
				else outdword(gstok->number);
			}
			else{
				CheckAllMassiv(gbuf,i,gstr,gstok);
				outseg(gstok,2);
				op(0x8A);
				op(gstok->rm);
				outaddress(gstok);
			}
			ClearReg(AL);
			break;
		case tk_reg32:
			if(gstok->number>BX){
				i=1;
	 			if(gstok->number!=AX)op66(r32);
			}
			goto beg1;
		case tk_reg:
			if(gstok->number>BX){
				i=1;
	 			if(gstok->number!=AX)op66(r16);
			}
		case tk_beg:
beg1:
			if(gstok->number!=AL){
				op(0x88+i);  //mov [],AL
				op(0xC0+gstok->number*8);
			}
			ClearReg(AL);
			break;
		case tk_seg:
	 		op66(r16);
			op(0x8C); op(0xC0+gstok->number*8); break;
			ClearReg(AL);
		default: bytevalexpected(0); break;
	}
}

int doalmath(int sign,char **ofsstr)
{
int negflag=0,i=0;
int rettype=tk_beg;
	if(tok==tk_minus){
		if(CheckMinusNum()==FALSE){
			negflag=1;
			getoperand(am32==TRUE?EAX:BX);
		}
	}
#ifdef OPTVARCONST
	if(tok>=tk_charvar&&tok<=tk_doublevar&&itok.npointr==0){
		if(CheckConstVar(&itok)){
			tok=tk_number;
			calcnumber=TRUE;
		}
	}
#endif
	switch(tok){
		case tk_number:
			op(0xB0);	//mov AL,num
			i=CalcNumber(sign);
			op(i);
			ConstToReg(i,AL,r8);
			break;
		case tk_at:
		 	getoperand(am32==TRUE?EAX:BX);
			i++;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			if(itok.flag&f_retproc)rettype=(itok.flag&f_retproc)/256+tk_overflowflag-1;
			if((!i)||macros(sign!=0?tk_char:tk_byte)==0)procdo(sign!=0?tk_char:tk_byte);
			nexttok();
			if(*ofsstr){
				free(*ofsstr);
				*ofsstr=NULL;
			}
			break;
		default:
			SINFO bstr=strinf;
			strinf.bufstr=NULL;
			ITOK btok;
			char *bbuf;
			btok=itok;
			bbuf=bufrm;
			bufrm=NULL;
			getintoal(tok,&btok,bbuf,&bstr); nexttok(); break;
	}
#ifdef OPTVARCONST
	calcnumber=FALSE;
#endif
	if(negflag){
		if(optimizespeed&&(chip==5||chip==6))outdword(0xC0FEFF34);	//xor AL,-1 AL++
		else outword(0xD8F6);// NEG AL
		setzeroflag=TRUE;
	}
	if(itok.type!=tp_stopper&&tok!=tk_eof&&itok.type!=tp_compare){
		doalmath2(sign);
		rettype=tk_beg;
	}
	return rettype;
}

void  doalmath2(int sign)
{
int vop,i,next;
int expand=FALSE,optnum=FALSE;
int negflag=0;
char *ofsstr=NULL;
	while(itok.type!=tp_stopper&&tok!=tk_eof&&itok.type!=tp_compare){
		vop=0;
		i=0;
		next=1;
#ifdef OPTVARCONST
		if(tok2>=tk_charvar&&tok2<=tk_doublevar&&itok2.npointr==0){
			if(CheckConstVar(&itok2)){
				tok2=tk_number;
				calcnumber=TRUE;
			}
		}
#endif
		if(tok2==tk_number)optnum=OptimNum();
		int oldtok=tok;
		switch(tok){
			case tk_xor: vop+=0x08;
			case tk_minus: vop+=0x08;
			case tk_and: vop+=0x18;
			case tk_or: vop+=0x08;
			case tk_plus:
			  if(optnum==FALSE)getoperand();
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				switch(tok){
					case tk_number:
						if(itok.number==0&&oldtok!=tk_and)break;
						if(itok.number==255&&oldtok==tk_and)break;
					  op(0x04+vop);
						op((unsigned int)itok.number); /* OPT AL,num */
						break;
					case tk_rmnumber:
						CheckAllMassiv(bufrm,itok.size,&strinf);
				 		op66(r16);
						op67(itok.sib==CODE16?r16:r32);
						if(itok.post==0)outseg(&itok,2);
						op(0x8D); /* LEA CX,[rm] */
						op(0x8+itok.rm);
						if(itok.post!=0&&itok.post!=UNDEF_OFSET){
							if((itok.flag&f_extern)==0){
								unsigned int ooutptr=outptr;
								if(am32&&itok.rm==rm_sib)outptr++;
								setwordpost(&itok);
								outptr=ooutptr;
							}
							else setwordext(&itok.number);
						}
						outaddress(&itok);
						op(vop); /* OPT AL,CL */
						op(0xC8);
						warningreg(regs[0][1]);
						break;
					case tk_postnumber:
				 		op66(r16);
						op(0x05+vop); /* OPT AX,# */
						(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
						outword((unsigned int)itok.number);
						break;
					case tk_qwordvar:
						i=4;
					case tk_longvar:
					case tk_dwordvar:
						i+=2;
					case tk_intvar:
					case tk_wordvar:
						i++;
					case tk_charvar:
					case tk_bytevar:
						i++;
						CheckAllMassiv(bufrm,i,&strinf);
						outseg(&itok,2);
						op(0x02+vop);	op(itok.rm);
						outaddress(&itok);
						break;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
						op66(r16);
						op(0x50);	//push AX
						addESP+=2;
unsigned char oaddstack;
						oaddstack=addstack;
						addstack=FALSE;
						procdo(sign!=0?tk_char:tk_byte);
						addstack=oaddstack;
						addESP-=2;
						op66(r16);
						op(0x58+CX);
						itok.number=CX;
						warningreg(regs[0][CX]);
						if(vop>0x20){
							op66(r16);
							op(0x90+CX);	//xchg ax,Cx
						}
						goto defbeg;
					case tk_bits:
						int razr;
						i=itok.bit.siz+itok.bit.ofs;
						if(i<=64)razr=r64;
						if(i<=32)razr=r32;
						if(i<=16)razr=r16;
						if(i<=8)razr=r8;
						bits2reg(CL,razr);
						itok.number=CL;
						if(razr==r64)razr=r32;
				 		warningreg(razr==r8?begs[1]:(regs[razr/2-1][1]));
						goto defbeg;
					case tk_doublevar:
						i=4;
					case tk_floatvar:
						Float2reg32(EAX,i);
						itok.number=0;
					case tk_beg:
defbeg:
						op(vop);
						op(0xC0+(unsigned int)itok.number*8);
						break;
					case tk_reg32:
					case tk_reg:
						if((unsigned int)itok.number>BX){
					 		op66(r16);
							op(0x89);	/* MOV CX,reg */
							warningreg(regs[0][1]);
						op(0xC1+(unsigned int)itok.number*8); /* MOV instr */
						itok.number=CL;
						}
						goto defbeg;
					default: valueexpected(); break;
				}
				setzeroflag=TRUE;
				if(expand==TRUE){
					if(oldtok==tk_plus){
						outword(0xd480);	//ADC AH,0
						op(0);
					}
					else if(oldtok==tk_minus){
						outword(0xdc80);	//SBB AH,0
						op(0);
					}
					setzeroflag=FALSE;
				}
				break;
			case tk_modminus: negflag=1;
			case tk_mod: negflag=1-negflag; vop=1;
			case tk_divminus: negflag=1-negflag;
			case tk_div:
			  if(optnum==FALSE)getoperand();
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				if(tok==tk_number){
					if(negflag){
						itok.number=-itok.number;
						negflag=0;
					}
					itok.number&=255;
					if(vop){	//%
						if(itok.number==0)DevideZero();
						if(caselong(itok.number)!=NUMNUM){
							op(0x24);	/* AND AL,number-1 */
							op((unsigned int)itok.number-1);
							setzeroflag=TRUE;
						}
						else{
							if(expand==FALSE){
								if(sign)cbw();
								else xorAHAH();
							}
							op(0xB1); op((unsigned int)itok.number); /* MOV CL,# */
							if(sign)outword(0xF9F6);	/* IDIV CL */
							else outword(0xF1F6); /* DIV CL */
							outword(0xE088);// MOV AL,AH
							setzeroflag=FALSE;
					 		warningreg(begs[1]);
							break;
						}
					}
					else{
						switch((unsigned int)itok.number){
							case 0:
								DevideZero();
								break;
							case 1:	break;
							case 2:
								op(0xd0+expand);
								if(sign)op(0xf8);
								else op(0xE8);// SHR AL,1
								setzeroflag=TRUE;
								break;
							default:
								vop=caselong(itok.number);
								if(vop!=NUMNUM){
									if(chip<2){
										op(0xB1);	op(vop); /* MOV CL,num */
										op(0xd2+expand);
										if(sign)op(0xF8); // SAR AL,CL
										else op(0xE8); // SHR AL,CL
							 			warningreg(begs[1]);
									}
									else{
										op(0xc0+expand);
										if(sign)op(0xF8); /* SAR AL,num */
										else op(0xE8); /* SHR AL,num */
										op(vop);
										if(cpu<2)cpu=2;
									}
									setzeroflag=TRUE;
								}
								else{
									if(expand==FALSE){
										if(optimizespeed&&(itok.flag&f_reloc)==0&&sign==0){	//for signed needed new algoritm
											//замена деления умножением
											itok.number=256/(unsigned int)itok.number+1;
											if(chip>4){
												xorAHAH();
											 	op66(r16);
												outword(0xC06B);	//imul AX,num
												op(itok.number);
												warningreg(regs[0][2]);
											}
											else{
												op(0xB2);	//mov DL,num
												op(itok.number);
												outword(0xE2F6);	//mul DL
												warningreg(begs[2]);
											}
											outword(0xE088);	//mov AL.AH
											setzeroflag=FALSE;
											break;
										}
										if(sign)cbw();
										else xorAHAH();
									}
									op(0xB1);  /* MOV CL,# */
									op((unsigned int)itok.number);
									if(sign)outword(0xF9F6);  /* IDIV CL */
									else outword(0xF1F6); /* DIV CL */
									setzeroflag=FALSE;
						 			warningreg(begs[1]);
								}
						}
					}
				}
				else{
					case tk_doublevar:
						i=4;
					if(tok==tk_floatvar){
						Float2reg32(ECX,i);
						itok.number=ECX;
						tok=tk_beg;
						sign=1;
					}
					if(expand==FALSE){
						if(sign)cbw();
						else xorAHAH();
					}
					switch(tok){
						case tk_rmnumber:
						case tk_postnumber:
							getintoreg_32(CX,r16,sign,&ofsstr,FALSE);
							if(sign)outword(0xF9F6);  // IDIV CL
							else outword(0xF1F6);	// DIV CL
							setzeroflag=FALSE;
					 		warningreg(regs[0][1]);
							break;
						case tk_qwordvar:
							i=4;
						case tk_longvar:
						case tk_dwordvar:
							i+=2;
						case tk_intvar:
						case tk_wordvar:
							i++;
						case tk_charvar:
						case tk_bytevar:
							i++;
							CheckAllMassiv(bufrm,i,&strinf);
							outseg(&itok,2);
							op(0xF6);
							if(sign)op(0x38+itok.rm);
							else op(0x30+itok.rm);
							outaddress(&itok);
							setzeroflag=FALSE;
							break;
						case tk_bits:
							int razr;
							i=itok.bit.siz+itok.bit.ofs;
							if(i<=64)razr=r64;
							if(i<=32)razr=r32;
							if(i<=16)razr=r16;
							if(i<=8)razr=r8;
							bits2reg(CL,razr);
							itok.number=CL;
							if(razr==r64)razr=r32;
					 		warningreg(razr==r8?begs[1]:(regs[razr/2-1][1]));
							goto defdiv;
						case tk_ID:
						case tk_id:
						case tk_proc:
						case tk_apiproc:
						case tk_undefproc:
						case tk_declare:
							op66(r16);
							op(0x50);	//push AX
							addESP+=2;
unsigned char oaddstack;
							oaddstack=addstack;
							addstack=FALSE;
							procdo(sign!=0?tk_char:tk_byte);
							addstack=oaddstack;
							addESP-=2;
							op66(r16);
							op(0x58+CX);
							itok.number=CX;
							warningreg(regs[0][CX]);
							op66(r16);
							op(0x90+CX);	//xchg ax,cx
						case tk_beg:
defdiv:
							op(0xF6);
							if(sign)op(0xF8+(unsigned int)itok.number);
							else op(0xF0+(unsigned int)itok.number);
							setzeroflag=FALSE;
							break;
						case tk_reg32:
						case tk_reg:
							if((unsigned int)itok.number>BX){
						 		op66(r16);
								op(0x89);  /* MOV CX,reg */
						 		warningreg(regs[0][1]);
								op(0xC1+(unsigned int)itok.number*8); /* MOV instr */
								itok.number=CL;
							}
							goto defdiv;
						default: valueexpected();	break;
					}
					if(vop)outword(0xE088);// MOV AL,AH
				}
				expand=FALSE;
				break;
			case tk_multminus: negflag=1;
			case tk_mult:
				expand=expandvar();
			  if(optnum==FALSE)getoperand();
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				switch(tok){
					case tk_number:
						if(negflag){
							itok.number=-itok.number;
							negflag=0;
						}
						itok.number&=255;
						switch((unsigned int)itok.number){
							case 0: /* AL * 0 = MOV AL,0 */
								outword(0x00B0);
							case 1:
								expand=FALSE;
								setzeroflag=FALSE;
								break; /* AL * 1 = AL */
							case 2:
								if(expand==TRUE){
									if(sign)cbw();
									else xorAHAH();
								}
								outword(0xC000+expand); // AL * 2 = ADD AL,AL
								setzeroflag=TRUE;
								break;
							default:
								vop=caselong(itok.number);
								if(vop!=NUMNUM){
									if(chip<1){
										if(expand==TRUE){
											if(optimizespeed==FALSE&&sign==FALSE)goto num_imul;
											if(sign)cbw();
											else xorAHAH();
										}
								 		op(0xB1); op(vop); /* MOV CL,num */
										outword(0xE0D2+expand);// SHL AL,CL
							 			warningreg(begs[1]);
									}
									else{
										if(expand==TRUE){
											if(optimizespeed==FALSE&&sign==FALSE)goto num_imul;
											if(sign)cbw();
											else xorAHAH();
									 		op66(r16);
										}
										outword(0xe0c0+expand);	//SHL AX/L,num
										op(vop);
										if(cpu<1)cpu=1;
									}
									setzeroflag=TRUE;
								}
								else if(expand==FALSE&&optimizespeed!=FALSE&&
										speedmul(itok.number,r8)!=FALSE);
								else{
num_imul:
									op(0xB1);  /* MOV CL,# */
									op((unsigned int)itok.number);
									if(sign)outword(0xE9F6);// IMUL CL
									else outword(0xE1F6);	// MUL CL
									setzeroflag=FALSE;
							 		warningreg(begs[1]);
								}
						}
						break;
					case tk_rmnumber:
					case tk_postnumber:
						getintoreg_32(CX,r16,sign,&ofsstr,FALSE);
						if(sign)outword(0xE9F6); // IMUL CL
						else outword(0xE1F6);	// MUL CL
						setzeroflag=FALSE;
				 		warningreg(regs[0][1]);
						break;
					case tk_doublevar:
						i=4;
					case tk_floatvar:
						Float2reg32(ECX,i);
						itok.number=ECX;
						outword(0xE9F6); // IMUL CL
						setzeroflag=FALSE;
				 		warningreg(begs[1]);
						break;
					case tk_qwordvar:
						i=4;
					case tk_longvar:
					case tk_dwordvar:
						i+=2;
					case tk_intvar:
					case tk_wordvar:
						i++;
					case tk_charvar:
					case tk_bytevar:
						i++;
						CheckAllMassiv(bufrm,i,&strinf);
						outseg(&itok,2);
						op(0xF6);
						if(sign)op(0x28+itok.rm);
						else op(0x20+itok.rm);
						outaddress(&itok);
						setzeroflag=FALSE;
						break;
					case tk_bits:
						int razr;
						i=itok.bit.siz+itok.bit.ofs;
						if(i<=64)razr=r64;
						if(i<=32)razr=r32;
						if(i<=16)razr=r16;
						if(i<=8)razr=r8;
						bits2reg(CL,razr);
						itok.number=CL;
						if(razr==r64)razr=r32;
				 		warningreg(razr==r8?begs[1]:(regs[razr/2-1][1]));
						goto defmul;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
						op66(r16);
						op(0x50);	//push AX
						addESP+=2;
unsigned char oaddstack;
						oaddstack=addstack;
						addstack=FALSE;
						procdo(sign!=0?tk_char:tk_byte);
						addstack=oaddstack;
						addESP-=2;
						op66(r16);
						op(0x58+DX);
						itok.number=DX;
						warningreg(regs[0][DX]);
					case tk_beg:
defmul:
						op(0xF6);
						if(sign)op(0xE8+(unsigned int)itok.number);
						else op(0xE0+(unsigned int)itok.number);
						setzeroflag=FALSE;
						break;
					case tk_reg32:
					case tk_reg:
						if((unsigned int)itok.number>BX){
					 		op66(r16);
							op(0x89);  /* MOV CX,reg */
					 		warningreg(regs[0][1]);
							op(0xC1+(unsigned int)itok.number*8); /* MOV instr */
							itok.number=CL;
						}
						goto defmul;
					default: valueexpected();	break;
				}
				break;
			case tk_xorminus: vop+=0x10;
			case tk_andminus: vop+=0x18;
			case tk_orminus: vop+=0x08;
			  getoperand();
				if(tok==tk_number){
					itok.number=-itok.number;
					op(0x04+vop);
					op((unsigned int)itok.number);
				}
				else{
					getintobeg(CL,&ofsstr);
					if(optimizespeed&&(chip==5||chip==6)){
						op(0x80);
						outdword(0xC1FEFFE1);	//and CL,-1 inc CL
					}
					else outword(0xD9F6);  // NEG CL
					op(0x00+vop);
					op(0xC8);	/* opt AL,CL */
			 		warningreg(begs[1]);
					next=0;
				}
				setzeroflag=TRUE;
				break;
			case tk_rr:
				vop=8;
				if(sign)vop+=0x10;
			case tk_ll:
			  getoperand();
				if(tok==tk_number){
					if((unsigned int)itok.number==1){
						if(expand==TRUE)op66(r16);
						op(0xD0+expand); op(0xE0+vop);  /* SR AL,1 */
					}
					else if((unsigned int)itok.number!=0){
						if(chip<2) goto llminus;
						else{
							if(expand==TRUE)op66(r16);
							op(0xC0+expand); op(0xE0+vop);	/* SR AL,imm8 */
							op((unsigned int)itok.number);
							if(cpu<2)cpu=2;
						}
					}
					setzeroflag=TRUE;
				}
				else goto llminus;
				break;
			case tk_rrminus:
				vop=8;
				if(sign)vop+=0x10;
			case tk_llminus:
				tok=tk_minus; 	// need 286+ opt some time
llminus:
				if(!((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==1)){
					getintobeg(CL,&ofsstr);
			 		warningreg(begs[1]);
				}
				else getoperand();
				if(expand==TRUE)op66(r16);
				op(0xD2+expand); op(0xE0+vop);
				setzeroflag=TRUE;
				next=0;
				break;
			default: operatorexpected(); break;
		}
		if(negflag){
			if(optimizespeed&&(chip==5||chip==6))outdword(0xC0FEFF34);	//xor AL,-1 AL++
			else outword(0xD8F6);// NEG AL
			negflag=0;
			setzeroflag=TRUE;
		}
		if(next)nexttok();
		ClearReg(AX);
	}
#ifdef OPTVARCONST
	calcnumber=FALSE;
#endif
	if(tok==tk_eof)unexpectedeof();
}

/* =============== doreg_32(), dobeg(), doseg() ===============*/

int doreg_32(int reg,int razr,int terminater)
{
unsigned char next=1;
int vop=0,sign=0;
int i;
int reg1=idxregs[0],reg2=idxregs[1];
unsigned long ii;
int rettype=tk_reg;
int rrettype,pointr=0;
int numpointr=0;
char *ofsstr=NULL;
	if(reg==reg1){
		reg1=idxregs[1];
		reg2=idxregs[2];
	}
	if(reg==reg2)reg2=idxregs[2];
	if(reg==ESP)RestoreStack();
	rrettype=razr==r16?tk_word:tk_dword;
	nexttok();
	switch(tok){
		case tk_assign://=
			if(am32)idxregs[4]=reg;
			if(!((tok2==tk_reg||tok2==tk_reg32||tok2==tk_beg)&&ScanTok3()==terminater)){
				ofsstr=GetLecsem(terminater);
			}
			else{
				nexttok();
				switch(tok){
					case tk_beg:
						i=r8;
						break;
					case tk_reg:
						i=r16;
						break;
					case tk_reg32:
						i=r32;
						break;
				}
				if(i!=razr||RegToReg(reg,itok.number,i)==NOINREG)goto nn1;
				waralreadinitreg(regs[razr/4][itok.number],regs[razr/4][reg]);
				if(am32)idxregs[4]=255;
				break;
			}
			if(ofsstr){
				int retreg;
				if((retreg=CheckIDZReg(ofsstr,reg,razr))!=NOINREG){
					GetEndLex(terminater);
					if(razr==r16)tok=tk_reg;
					else tok=tk_reg32;
					itok.number=retreg==SKIPREG?reg:retreg;
					goto nn1;
				}
			}
			nexttok();
			convert_type(&sign,&rrettype,&pointr,am32==TRUE?reg:BX);
			if(rrettype==tk_float||rrettype==tk_double){
				if(am32)idxregs[4]=255;
				doeaxfloatmath(tk_reg32,reg,rrettype==tk_float?0:4);
				next=0;
				if(ofsstr){
					IDZToReg(ofsstr,reg,razr);
					free(ofsstr);
				}
				break;
			}
			while(tok==tk_mult){
				nexttok();
				numpointr++;
			}
			if(numpointr>itok.npointr)unuseableinput();
			if(tok2==tk_assign){
				int hnumber=MultiAssign(razr,USEALLREG,numpointr);
//				puts("end MultAssign");
				if(ofsstr){
					free(ofsstr);
					ofsstr=NULL;
				}
				if(reg!=hnumber){
					op66(razr);
					op(0x89);
					op(0xC0+reg+hnumber*8);	//mov reg,AX
				}
				next=0;
/*				if(ofsstr){
					IDZToReg(ofsstr,reg,razr);
					free(ofsstr);
				}*/
				if(am32)idxregs[4]=255;
				break;
			}
//			printf("tok=%d %s\n",tok,itok.name);
			if(tok==tk_pointer)cpointr(am32==TRUE?reg:BX,numpointr);
			if(tok==tk_new||tok==tk_delete){
				if(tok==tk_new)donew();
				else{
					dodelete();
					terminater=next=0;
				}
				if(am32)idxregs[4]=255;
				if(reg!=AX){
					GenRegToReg(reg,AX,(am32+1)*2);
				}
				clearregstat();
#ifdef OPTVARCONST
				FreeGlobalConst();
#endif
				if(ofsstr){
					free(ofsstr);
					ofsstr=NULL;
				}
				break;
			}
nn1:
			if(am32)idxregs[4]=255;
			if(reg==AX){
				if(rrettype==tk_char||rrettype==tk_byte)rettype=doalmath(sign,&ofsstr);
				else if(rrettype==tk_int||rrettype==tk_word)rettype=do_e_axmath(sign,r16,&ofsstr);
				else rettype=do_e_axmath(sign,r32,&ofsstr);
				convert_returnvalue(razr==r16?tk_word:tk_dword,rrettype);
			}
			else{
				if(rrettype==tk_char||rrettype==tk_byte&&reg<=BX){
					rettype=getintobeg(reg,&ofsstr);
					if(itok.type!=tp_stopper&&tok!=tk_eof){
						dobegmath(reg);
						rettype=tk_beg;
					}
					op66(razr);
					op(0x0F);
					if(!sign)op(0xB6);
					else op(0xBE);
					op(0xC0+reg*9);
				}
				else{
					if(rrettype==tk_int||rrettype==tk_word)next=r16;
					else next=r32;
					rettype=getintoreg(reg,next,sign,&ofsstr);
					if(next==r16&&razr==r32){
						op66(r32);
						op(0x0F);
						if(!sign)op(0xB7);
						else op(0xBF);
						op(0xC0+reg*9);
					}
				}
			}
			next=0;
			if(ofsstr){
				IDZToReg(ofsstr,reg,razr);
				free(ofsstr);
			}
			break;
		case tk_plusplus: op66(razr); op(0x40+reg);
			ClearReg(reg);
			break;
		case tk_minusminus: op66(razr); op(0x48+reg);
			ClearReg(reg);
			break;
		case tk_cdecl:
		case tk_pascal:
		case tk_fastcall:
		case tk_stdcall:
			vop=tok;
			nexttok();
			if(tok!=tk_openbracket){
				expected('(');
				FindStopTok();
			}
		case tk_openbracket:	//вызов процедуры по адресу в регистре
			param[0]=0;
			i=0;
			switch ( vop ) {
				case tk_cdecl:
				case tk_stdcall:
					i=swapparam();
					break;
				case tk_pascal:
					doparams();
					break;
				case tk_fastcall:
					doregparams();
					break;
				default:
					if(comfile==file_w32)swapparam();
					else doparams();
			}
			if(vop!=tk_cdecl)i=0;
			op66(razr);
			op(0xFF);
			op(0xD0+reg); 	/* CALL reg with stack params */
			if(i)CorrectStack(i);
			clearregstat();
#ifdef OPTVARCONST
			FreeGlobalConst();
#endif
			break;
		case tk_swap:
			getoperand(reg==BX?SI:BX);
			switch(tok){
				case tk_qwordvar:
				case tk_longvar:
				case tk_dwordvar:
					if(razr==r16)swaperror();
					i=4;
					goto swapint;
			 	case tk_intvar:
				case tk_wordvar:
					i=2;
					if(razr==r32)swaperror();
swapint:
					ClearReg(reg);
					CheckAllMassiv(bufrm,i,&strinf,&itok,reg1,reg2);
					op66(razr);
			 		outseg(&itok,2);
					op(0x87);
					op(reg*8+itok.rm);
					outaddress(&itok);
					break;
				case tk_reg32:
					if(razr==r16)swaperror();
					goto swapreg;
				case tk_reg:
					if(razr==r32)swaperror();
swapreg:
					if(reg!=(int)itok.number){
						if(RegSwapReg(reg,itok.number,razr)==NOINREG){;
							op66(razr);
							if(reg==AX)op(0x90+(unsigned int)itok.number);
							else if((unsigned int)itok.number==AX)op(0x90+reg);
							else{
								op(0x87);
								op(0xC0+(unsigned int)itok.number+reg*8);
							}
						}
						else waralreadinitreg(regs[razr/4][reg],regs[razr/4][itok.number]);
					}
					break;
				default: swaperror(); break;
			}
			break;
		case tk_xorequals: vop+=0x08;
		case tk_minusequals: vop+=0x08;
		case tk_andequals: vop+=0x18;
		case tk_orequals: vop+=0x08;
		case tk_plusequals:
			ClearReg(reg);
			if(am32&&uselea&&tok==tk_plusequals){
				if(RegEqualToLea(reg)){
					next=0;
					break;
				}
			}
			if(CheckAddOnly()){
				inptr2--;
				cha2=' ';
				if(tok==tk_plusequals)tok=tk_plus;
				else tok=tk_minus;
				if(reg==EAX)do_e_axmath2(0,razr,0);
				else doregmath_32(reg,razr,0,&ofsstr);
				next=0;
				break;
			}
			getoperand(reg==BX?SI:BX);
			if(itok2.type==tp_opperand&&tok!=tk_number&&tok!=tk_undefofs&&tok!=tk_postnumber)goto defadd;
			CheckMinusNum();
			idrec *rrec;
			int opost;
			i=tok;
			switch(tok){
				case tk_postnumber:
				case tk_undefofs:
					ii=itok.number;
					rrec=itok.rec;
					opost=itok.post;
					char uname[IDLENGTH];
					strcpy(uname,itok.name);
					if(itok.flag&f_extern)goto addnum;
					tok=tk_number;
				case tk_number:
					ii=doconstdwordmath();
					next=0;
					if(itok.type==tp_opperand){
						if(reg==EAX){
							sign=ECX;
							if((tok2==tk_reg||tok2==tk_reg32)&&itok2.number==ECX)sign=EDX;
							warningreg(regs[razr/2-1][sign]);
						}
						if(i==tk_postnumber||i==tk_undefofs){
							op66(razr);
							op(0xB8+reg);	// MOV reg,#
							if(i==tk_postnumber)(postnumflag&f_extern)==0?setwordpost(&itok):setwordext((long *)&ii);
							else{
								if((postnumflag&f_reloc)!=0)AddReloc();
								if(i==tk_undefofs)AddUndefOff(2,uname);
							}
							razr==r16?outword(ii):outdword(ii);
						}
						else MovRegNum(razr,postnumflag&f_reloc,ii,sign);
						if(sign==EAX)do_e_axmath2(0,razr,0);
						else doregmath_32(sign,razr,0,&ofsstr);
						itok.number=sign;
						goto addreg;
					}
					if((postnumflag&f_reloc)==0&&i!=tk_undefofs&&i!=tk_postnumber&&optnumadd(ii,reg,razr,vop))break;
addnum:
					op66(razr);
				  if(reg==AX)op(0x05+vop);
					else{
						op(0x81);
						op(0xC0+vop+reg);
					}
					itok.rec=rrec;
					itok.post=opost;
					if(i==tk_postnumber)(postnumflag&f_extern)==0?setwordpost(&itok):setwordext((long *)&ii);
					else{
						if((postnumflag&f_reloc)!=0)AddReloc();
						if(i==tk_undefofs)AddUndefOff(2,uname);
					}
					razr==r16?outword(ii):outdword(ii);
					break;
				case tk_qwordvar:
				case tk_longvar:
				case tk_dwordvar:
					i=4;
					goto wordadd;
				case tk_intvar:
				case tk_wordvar:
					i=2;
					if(razr==r32)valueexpected();
wordadd:
					CheckAllMassiv(bufrm,i,&strinf,&itok,reg1,reg2);
					op66(razr);
					outseg(&itok,2);
					op(0x03+vop);
					op(reg*8+itok.rm);
					outaddress(&itok);
					break;
				case tk_reg:
					if(razr==r32)valueexpected();
				case tk_reg32:
addreg:
					op66(razr);
					op(0x01+vop);
					op(0xC0+reg+(unsigned int)itok.number*8);
					break;
				case tk_ID:
				case tk_id:
				case tk_proc:
				case tk_apiproc:
				case tk_undefproc:
				case tk_declare:
unsigned char oaddstack;
					if(reg==EAX){
						op66(razr);
						op(0x50);	//push AX
						addESP+=razr==r16?2:4;
						warningreg(regs[razr/2-1][EDX]);
						oaddstack=addstack;
						addstack=FALSE;
					}
					procdo(razr==r16?tk_word:tk_dword);
					if(itok2.type==tp_opperand){
						nexttok();
						do_e_axmath2(0,razr,0);
						next=0;
					}
					if(reg==EAX){
						addstack=oaddstack;
						addESP-=razr==r16?2:4;
						op66(razr);
						op(0x58+EDX);	//pop dx
						if(vop>0x20){
							op66(razr);
							op(0x90+EDX);	//xchg ax,dx
						}
						op66(razr);
						op(0x01+vop);
						op(0xc0+EDX*8);	//add ax,dx
					}
					else{
						op66(razr);
						op(0x01+vop);
						op(0xc0+reg);	//add reg,ax
					}
					break;
				case tk_seg:
					if(razr==r32)valueexpected();
				case tk_bytevar:
				case tk_charvar:
				case tk_beg:
defadd:
					if(reg==AX){
						getintoreg_32(ECX,razr,sign,&ofsstr);
						doregmath_32(ECX,razr,sign,&ofsstr);
						sign=CX;	//sign исп как пром регистр
					}
					else{
						do_e_axmath(0,razr,&ofsstr);
						sign=EAX;
					}
					warningreg(regs[razr/2-1][sign]);
					op66(razr);// OPT reg32,ECX
					op(0x01+vop);
					op(0xC0+reg+sign*8);
					next=0;
					break;
				default: valueexpected(); break;
			}
			break;
		case tk_rrequals: vop+=0x08;
		case tk_llequals:
			ClearReg(reg);
			getoperand(am32==TRUE?ECX:reg==BX?SI:BX);
			CheckMinusNum();
			if(tok==tk_number){
				ii=doconstlongmath();
				next=0;
				if(itok.type==tp_opperand){
					if(reg==ECX)regshifterror();
					op(0xB0+CL); op(ii);	//mov CL,num
					dobegmath(CL);
					warningreg(begs[1]);
					ConstToReg(ii,CL,r8);
					goto shiftcl;
				}
				if(ii==1){
					op66(razr);
					op(0xD1); op(0xE0+reg+vop);
				}  /* SHL reg,1 */
				else if(ii!=0){
					if(chip<2&&razr==r16){
						op(0xB0+CL); op(ii);	//mov CL,num
						warningreg(begs[1]);
						ConstToReg(ii,CL,r8);
						goto shiftcl;
					}
					else{
						op66(razr);
						op(0xC1); op(0xE0+reg+vop);	/* SHL reg,imm8 */
						op(ii);
						if(cpu<2)cpu=2;
					}
				}
			}
			else if(reg!=CX){
				if(!(itok2.type==tp_stopper&&(tok==tk_beg||tok==reg||tok==tk_reg32)&&itok.number==CL)){
					getintobeg(CL,&ofsstr);
					dobegmath(CL);
					warningreg(begs[1]);
					ClearReg(CL);
					next=0;
				}
shiftcl:
				op66(razr);
				op(0xD3); op(0xE0+vop+reg);	/* SHL AX,CL */
			}
			else regshifterror();
			break;
		case tk_multequals:
			ClearReg(reg);
			getoperand(reg==BX?SI:BX);
			CheckMinusNum();
			if(tok==tk_number){
				ii=doconstlongmath();
				next=0;
				if(itok.type==tp_opperand){
					if(reg==EAX)sign=ECX;
					MovRegNum(razr,postnumflag&f_reloc,ii,sign);
					if(sign==EAX)do_e_axmath2(0,razr,0);
					else doregmath_32(ECX,razr,0,&ofsstr);
					ConstToReg(ii,sign,razr);
					goto mulreg;
				}
				i=0;
				RegMulNum(reg,ii,razr,0,&i,itok.flag);
			}
			else{
				if(itok2.type==tp_stopper)next=(unsigned char)MulReg(reg,razr);
				else{
					if(reg==AX){
						getintoreg_32(ECX,razr,sign,&ofsstr);
						doregmath_32(ECX,razr,sign,&ofsstr);
						sign=CX;	//sign исп как пром регистр
					}
					else{
						do_e_axmath(0,razr,&ofsstr);
						sign=EAX;
					}
mulreg:
					warningreg(regs[razr/2-1][sign]);
					ClearReg(sign);
					op66(razr);
					outword(0xAF0F);
					op(0xC0+reg*8+sign);
					next=0;
				}
			}
			break;
		case tk_divequals:
			getoperand(reg==BX?SI:BX);
			ClearReg(reg);
			CheckMinusNum();
			if(tok==tk_number){
				ii=doconstlongmath();
				next=0;
				if(itok.type==tp_opperand){
					op66(razr);
					op(0x50+reg);	//push reg
					addESP+=razr==r16?2:4;
					MovRegNum(razr,postnumflag&f_reloc,ii,EAX);
					do_e_axmath2(0,razr,0);
					ClearReg(AX);
					goto divreg;
				}
				if((vop=caselong(ii))!=NUMNUM&&(!(reg==ECX&&chip<2))){
					if(vop!=0){
						if(chip<2&&razr==r16){
							op(0xB1); op(vop); /* MOV CL,num */
							op(0xD3);
							op(0xE8+reg); // SHR reg,CL
							warningreg(begs[1]);
							ClearReg(CX);
						}
						else{
							op66(razr);
							op(0xC1);
							op(0xE8+reg); // SHR reg,num
		 					op(vop);
						}
					}
				}
				else{
					if(reg!=EAX){
						op66(razr);
						op(0x90+reg);	//xchg reg,AX
					}
					DivNum(ii,razr,0);
					if(reg!=EAX){
						op66(razr);
						op(0x90+reg);	//xchg reg,AX
						warningreg(regs[razr/2-1][EAX]);
						ClearReg(AX);
					}
				}
			}
			else if(itok2.type==tp_stopper){
				if(reg!=EAX){
					op66(razr);
					op(0x90+reg);	//xchg reg,AX
				}
				DivMod(0,0,razr,0);
				next=0;
				if(reg!=EAX){
					op66(razr);
					op(0x90+reg);	//xchg reg,AX
					warningreg(regs[razr/2-1][EAX]);
					ClearReg(AX);
				}
			}
			else{
				op66(razr);
				op(0x50+reg);	//push reg
				addESP+=razr==r16?2:4;
				do_e_axmath(0,razr,&ofsstr);
divreg:
				op66(razr);
				sign=reg;
				if(reg==EAX){
					sign=ECX;
					warningreg(regs[razr/2-1][ECX]);
					ClearReg(CX);
				}
				addESP-=razr==r16?2:4;
				op(0x58+sign);	//pop sign
				op66(razr);
				op(0x90+sign);	//xchg AX,sign
				op66(razr);
				op(0xF7);
				op(0xF0+sign); // DIV reg
				op66(razr);
				if(reg!=EAX){
					if(optimizespeed){
						op(0x89);
						op(0xC0+reg);	//mov reg,AX
					}
					else op(0x90+reg);	//xchg AX,sign
				}
				warningreg(regs[razr/2-1][EAX]);
				ClearReg(AX);
				next=0;
			}
			break;
		default: operatorexpected(); break;
	}
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	if(razr==r32&&cpu<3)cpu=3;
//	puts("return doreg_32");
	return rettype;
}

int optnumadd(unsigned long num,int reg,int razr,int vop)
{
int nrazr=0;
	if(num==0){
		if(vop==0x20){	//&=
			ZeroReg(reg,razr);
			setzeroflag=TRUE;
		}
		return TRUE;		//+= -= |= ^=
	}
	if(vop==8){	//|=
		if(num<65536&&razr==r32)nrazr=razr=r16;
		if((unsigned short)num<256&&razr==r16&&reg<4){
			if(reg==AX)op(0x0C);
			else{
				op(0x80);
				op(0xc8+reg);
			}
			op(num);
			return TRUE;
		}
		if(nrazr==r16){
			op66(r16);
			if(reg==AX)op(0x0D);
			else{
				op(0x81);
				op(0xc8+reg);
			}
			outword(num);
			return TRUE;
		}
	}
	if(num==1){
		if(vop==0x28){	//-=
			op66(razr);
			op(0x48+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
		if(vop==0){	//+=
			op66(razr);
			op(0x40+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
	}
	if(!optimizespeed&&num==2&&((razr==r16&&am32==FALSE)||(razr==r32&&am32))){
		if(vop==0x28){	//-=
			op(0x48+reg);
			op(0x48+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
		if(vop==0){	//+=
			op66(razr);
			op(0x40+reg);
			op66(razr);
			op(0x40+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
	}
	if((razr==r16&&(unsigned short)num==0xffff)||
			(razr==r32&&num==0xffffffff)){
		if(vop==0x28){	//-=
			op66(razr);
			op(0x40+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
		if(vop==0){	//+=
			op66(razr);
			op(0x48+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
		if(vop==0x20)return TRUE;	//&=
		if(vop==0x30){	//^=
			if(optimizespeed&&(chip==5||chip==6))return FALSE;
			op66(razr);
			op(0xF7);
			op(0xD0+reg);	//Not reg
			setzeroflag=FALSE;
			return TRUE;
		}
	}
	if(vop==0x20){	//&=
		if(num>=0xFFFF0000&&razr==r32)nrazr=razr=r16;
		if(razr==r16&&(unsigned short)num>=0xFF00&&reg<4){
			if(reg==AL)op(0x24);
			else{
				op(128);
				op(0xE0+reg);
			}
			op(num);
			return TRUE;
		}
		if(nrazr==r16){
			op66(r16);
			if(reg==AX)op(0x25);
			else{
				op(129);
				op(0xE0+reg);
			}
			outword(num);
			return TRUE;
		}
	}
	if(!optimizespeed&&(razr==r16&&(unsigned short)num==0xfffe&&am32==FALSE)||
			(razr==r32&&num==0xfffffffe&&am32)){
		if(vop==0x28){	//-=
			op(0x40+reg);
			op(0x40+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
		if(vop==0){	//+=
			op(0x48+reg);
			op(0x48+reg);
			setzeroflag=TRUE;
			return TRUE;
		}
	}
	if(short_ok(num,razr/2-1)){
		op66(razr);
		op(0x83);
		op(0xC0+vop+reg);
		op(num);
		setzeroflag=TRUE;
		return TRUE;
	}
	return FALSE;
}

int dobeg(int beg,int terminater)
{
unsigned char next=1;
int vop=0,i=0,sign=0;
int rettype=tk_beg,pointr=0;;
int rrettype=tk_byte;
int numpointr=0;
char *ofsstr=NULL;
	nexttok();
	switch(tok){
		case tk_assign:
			if(!((tok2==tk_reg||tok2==tk_reg32||tok2==tk_beg)&&ScanTok3()==terminater)){
				ofsstr=GetLecsem(terminater);
			}
			else{
				nexttok();
				if(RegToReg(beg,itok.number,r8)==NOINREG)goto nn1;
				waralreadinitreg(begs[itok.number],begs[beg]);
				break;
			}
			if(ofsstr){
				int retreg;
				if((retreg=CheckIDZReg(ofsstr,beg,r8))!=NOINREG){
					GetEndLex(terminater);
					tok=tk_beg;
					itok.number=retreg==SKIPREG?beg:retreg;
					goto nn1;
				}
			}
			nexttok();
			convert_type(&sign,&rrettype,&pointr,am32==TRUE?(beg>3?beg-4:beg):BX);
			while(tok==tk_mult){
				nexttok();
				numpointr++;
			}
			if(numpointr>itok.npointr)unuseableinput();
			if(tok2==tk_assign){
				int hnumber=MultiAssign(r8,(beg>3?beg-4:beg),numpointr);
				if(ofsstr){
					free(ofsstr);
					ofsstr=NULL;
				}
				if(beg!=hnumber){
					op(0x88);
					op(0xC0+beg+hnumber*8);	//mov beg,AL
				}
				next=0;
				if(ofsstr){
					IDZToReg(ofsstr,beg,r8);
					free(ofsstr);
				}
				break;
			}
			if(tok==tk_pointer)cpointr(am32==TRUE?(beg>3?beg-4:beg):BX,numpointr);
nn1:
			if(beg==AL){
				if(rrettype==tk_char||rrettype==tk_byte)rettype=doalmath(sign,&ofsstr);
				else if(rrettype==tk_int||rrettype==tk_word)rettype=do_e_axmath(sign,r16,&ofsstr);
				else rettype=do_e_axmath(sign,r32,&ofsstr);
				next=0;
			}
			else{
				if(rrettype==tk_char||rrettype==tk_byte||beg>BL){
					rettype=getintobeg(beg,&ofsstr);
					if(itok.type!=tp_stopper&&tok!=tk_eof){
						dobegmath(beg);
						rettype=tk_beg;
					}
				}
				else{
					if(rrettype==tk_int||rrettype==tk_word)next=r16;
					else next=r32;
					rettype=getintoreg(beg,next,sign,&ofsstr);
				}
				next=0;
			}
			if(ofsstr){
				IDZToReg(ofsstr,beg,r8);
				free(ofsstr);
			}
			break;
		case tk_plusplus: op(0xFE); op(0xC0+beg);
			ClearReg(beg>3?beg%4:beg);
			break;
		case tk_minusminus: op(0xFE); op(0xC8+beg);
			ClearReg(beg>3?beg%4:beg);
			break;
		case tk_swap:
			getoperand(beg==BL||beg==BH?SI:BX);
			switch(tok){
				case tk_charvar:
				case tk_bytevar:
					CheckAllMassiv(bufrm,1,&strinf);
				  outseg(&itok,2);
					op(0x86);
					op(beg*8+itok.rm);
					outaddress(&itok);
					KillVar(itok.name);
					ClearReg(beg>3?beg-4:beg);
					break;
				case tk_beg:
					if(beg!=(int)itok.number){
						if(RegSwapReg(beg,itok.number,r8)==NOINREG){
							op(0x86);
							op(0xC0+(unsigned int)itok.number+beg*8);
						}
						else waralreadinitreg(begs[beg],begs[itok.number]);
					}
					break;
				default: swaperror(); break;
			}
			break;
		case tk_xorequals: vop+=0x08;
		case tk_minusequals: vop+=0x08;
		case tk_andequals: vop+=0x18;
		case tk_orequals: vop+=0x08;
		case tk_plusequals:
			ClearReg(beg>3?beg%4:beg);
			if(CheckAddOnly()){
				inptr2--;
				cha2=' ';
				if(tok==tk_plusequals)tok=tk_plus;
				else tok=tk_minus;
				if(beg==AL)doalmath2(0);
				else dobegmath(beg);
				next=0;
				break;
			}
			getoperand(beg==BL||beg==BH?SI:BX);
			if(itok2.type==tp_opperand&&tok!=tk_number){
				if(beg==AL){
					getintobeg(CL,&ofsstr);
					dobegmath(CL);
					sign=CL;	//sign исп как пром регистр
				}
				else{
					doalmath(0,&ofsstr);
					sign=EAX;
				}
				warningreg(begs[sign]);
				op(0x00+vop);
				op(0xC0+beg+sign*8);
				next=0;
				break;
			}
			switch(tok){
				case tk_number:
					i=doconstlongmath();
					next=0;
					if(i==0&&(vop==0||vop==0x28||vop==8))break;
					if(i==255&&vop==0x20)break;
					if(beg==AL)op(0x04+vop);
					else{
						op(0x80);
						op(0xC0+vop+beg);
					}
					op(i);
					break;
				case tk_qwordvar:
					i+=4;
				case tk_longvar:
				case tk_dwordvar:
					i+=2;
				case tk_wordvar:
				case tk_intvar:
					i++;
				case tk_charvar:
				case tk_bytevar:
					i++;
					CheckAllMassiv(bufrm,i,&strinf);
					outseg(&itok,2);
					op(0x02+vop);
					op(beg*8+itok.rm);
					outaddress(&itok);
					break;
				case tk_beg:
					op(0x00+vop);
					op(0xC0+beg+(unsigned int)itok.number*8);
					break;
				case tk_proc:
				case tk_apiproc:
				case tk_undefproc:
				case tk_declare:
				case tk_ID:
				case tk_id:
					op66(r16);
					i=beg<4?beg:beg-4;
					op(0x50+i);
					addESP+=2;
					warningreg(regs[0][i]);
					procdo(tk_byte);
					if(itok2.type==tp_opperand){
						nexttok();
						doalmath2(0);
						next=0;
					}
					addESP-=2;
					op66(r16);
					op(0x58+(i!=AX?i:CX));
					if(i!=AX){
						op(0x00+vop);
						op(0xc0+beg);
					}
					else{
						if(vop>0x20){
							op(0x86);
							op(0xC0+CL+beg);	//xchg al,cl
						}
						op(0x00+vop);
						op(0xc0+CL*8+beg);
					}
					break;
				case tk_reg:
					if((unsigned int)itok.number<BX){
						op(0x00+vop);
						op(0xC0+beg+(unsigned int)itok.number*8);
					}
					else begworderror();
					break;
				case tk_seg: begworderror(); break;
				default: valueexpected();	break;
			}
			break;
		case tk_rrequals: vop+=0x08;
		case tk_llequals:
			getoperand(am32==TRUE?ECX:BX);
			ClearReg(beg>3?beg%4:beg);
			if(itok2.type==tp_stopper&&tok==tk_number){
				if((unsigned int)itok.number==1){
					op(0xD0);	op(0xE0+beg+vop);
				}	/* SHL beg,1 */
				else if((unsigned int)itok.number!=0){
					if(chip<2)goto shiftbeg;
					else{
						op(0xc0);
						op(0xe0+beg+vop);
						op((unsigned int)itok.number);
					}
				}
			}
			else{
shiftbeg:
				if(beg!=CL){
					if(itok2.type==tp_stopper&&tok==tk_beg&&itok.number==CL){
						op(0xD2); op(0xE0+vop+beg); 	/* SHL beg,CL */
					}
					else{
						ClearReg(CL);
						getintobeg(CL,&ofsstr);
						dobegmath(CL);
						next=0;
						warningreg(begs[1]);
						op(0xD2); op(0xE0+vop+beg); 	/* SHL beg,CL */
					}
				}
				else regshifterror();
			}
			break;
		default: operatorexpected(); break;
	}
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	return rettype;
}

void doseg(int seg)
{
unsigned char next=1;
int numpointr=0;
char *ofsstr=NULL;
	if(seg==CS)preerror("CS not used for destention");
	if(seg==FS||seg==GS)if(cpu<3)cpu=3;
	if(seg==SS)RestoreStack();
	nexttok();
	KillVar(segs[seg]);
	if(tok==tk_assign){
		nexttok();
		while(tok==tk_mult){
			nexttok();
			numpointr++;
		}
		if(numpointr>itok.npointr)unuseableinput();
		if(tok2==tk_assign){
			itok.number=MultiAssign(r16,USEALLREG,numpointr);
			if(ofsstr){
				free(ofsstr);
				ofsstr=NULL;
			}
			goto getfromreg;
		}
		if(tok==tk_pointer)cpointr(am32==TRUE?EAX:BX,numpointr);
		if(itok2.type!=tp_opperand){
			switch(tok){
				case tk_reg:
getfromreg:
					op(0x8E);
					op(0xC0+seg*8+(unsigned int)itok.number);	/* MOV seg,reg */
					break;
				case tk_intvar:
				case tk_wordvar:
				case tk_longvar:
				case tk_dwordvar:
				case tk_qwordvar:
					CheckAllMassiv(bufrm,2,&strinf);
					op66(r16);
					outseg(&itok,2);
					op(0x8E);
					op(seg*8+itok.rm);
					outaddress(&itok);	 /* MOV seg,[wordvar] */
					break;
				case tk_seg:
					if(optimizespeed==FALSE||(regoverstack&&chip>2)){
						PushSeg((unsigned int)itok.number);
						PopSeg(seg);
						break;
					}
					goto segax;
				case tk_number:
					if(regoverstack&&chip>1){
//						op66(r16);
						if(short_ok(itok.number)&&(itok.flag&f_reloc)==0){
							op(0x6A);
							op(itok.number);
						}
						else{
							op(0x68);
							if((itok.flag&f_reloc)!=0)AddReloc();
							if(am32)outdword(itok.number);
							else outword(itok.number);
						}
						PopSeg(seg);
						if(cpu<2)cpu=2;
						break;
					}
					goto segax;
				default: goto segax;
			}
		}
		else{
segax:
			do_e_axmath(0,r16,&ofsstr);
			op(0x8E); 	/* MOV SEG,AX */
			op(0xC0+seg*8);
			next=0;
		}
	}
	else if(tok==tk_swap){
	 	getoperand();
		switch(tok){
			case tk_intvar:
			case tk_wordvar:
				KillVar(itok.name);
				op66(r16);
				op(0x8C);
				op(0xC0+seg*8); /* MOV AX,SEG */
				CheckAllMassiv(bufrm,2,&strinf);
				op66(r16);
				outseg(&itok,2);
				op(0x87);
				op(itok.rm);
				outaddress(&itok);	/* XCHG AX,[word] */
				op66(r16);
				op(0x8E);
				op(0xC0+seg*8);	/* MOV seg,AX */
				break;
			case tk_seg:
				KillVar(segs[itok.number]);
				PushSeg(seg);
				PushSeg(itok.number);
				PopSeg(seg);
				PopSeg(itok.number);
				break;
			default: preerror("Only int, word variables valid for segment register ><");
				break;
		}
	}
	else segoperror();
	if(next)nextseminext();
	else seminext();
}

void PushSeg(int seg)
{
	switch(seg){
		case DS: op(0x1E); break;
		case CS: op(0x0E); break;
		case SS: op(0x16); break;
		case ES: op(0x06); break;
		case FS: outword(0xA00F); if(cpu<3)cpu=3; break;
		case GS: outword(0xA80F); if(cpu<3)cpu=3; break;
	}
}

void PopSeg(int seg)
{
	switch(seg){
		case DS: op(0x1F); break;
		case SS: op(0x17); break;
		case ES: op(0x07); break;
		case FS: outword(0xA10F); if(cpu<3)cpu=3;  break;
		case GS: outword(0xA90F); if(cpu<3)cpu=3;  break;
	}
}

// =============== doregmath_32(), dobegmath() ===============

void doregmath_32(int reg,int razr,int sign,char **ofsstr,int fdiv)  // math done is on all regs except AX
// all other registers preserved
{
int vop,i,optnum,negflag=FALSE;
	while(itok.type!=tp_stopper&&tok!=tk_eof){
		if(negflag){
			NegReg(razr,reg);
			negflag=FALSE;
		}
		i=vop=0;
		optnum=FALSE;
#ifdef OPTVARCONST
		CheckConstVar3(&tok2,&itok2,razr);
		if(tok2==tk_number)calcnumber=TRUE;
#endif
		if(uselea){
			if(razr==r32){
				if(Reg32ToLea2(reg))continue;	//оптимизация сложения 32-битных регистров в LEA
			}
			else if(Reg16ToLea2(reg))continue;
			if(itok.type==tp_stopper||tok==tk_eof)break;
		}
		if(tok2==tk_number)optnum=OptimNum();
		switch(tok){
			case tk_xor: vop+=0x08;
			case tk_minus: vop+=0x08;
			case tk_and: vop+=0x18;
			case tk_or: vop+=0x08;
			case tk_plus:
			  if(optnum==FALSE)getoperand(reg==BX?SI:BX);
				else tok=tk_number;
				switch(tok){
					case tk_number:
						if((itok.flag&f_reloc)==0&&optnumadd(itok.number,reg,razr,vop))break;
					case tk_postnumber:
					case tk_undefofs:
						op66(razr);
					  op(0x81);
						op(0xC0+vop+reg);
						if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
						else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
						else if((itok.flag&f_reloc)!=0)AddReloc();
						razr==r16?outword((unsigned int)itok.number):outdword(itok.number);
						break;
					case tk_apioffset:
						op66(razr);
					  op(0x81);
						op(0xC0+vop+reg);
						AddApiToPost(itok.number);
						break;
					case tk_doublevar:
						i=4;
					case tk_floatvar:
						Float2reg32(EAX,i);
						itok.number=EAX;
						warningreg(regs[1][EAX]);
						goto defreg32;
					case tk_bits:
						int vops,reg2s;
						i=itok.bit.siz+itok.bit.ofs;
						if(i<=64)vops=r64;
						if(i<=32)vops=r32;
						if(i<=16)vops=r16;
						if(vops<razr)vops=razr;
						reg2s=CX;
						if(reg==CX)reg2s=DI;
						bits2reg(reg2s,vops);
						if(vops==r64)vops=r32;
						warningreg(regs[vops/2-1][reg2s]);
						itok.number=reg2s;
						goto defreg32;
					case tk_reg:
						if(razr==r32){
							op66(razr);
							outword(0xB70F);
							if(itok.number==reg){
								op(0xC0+reg);
								itok.number=EAX;
							}
							else op(0xC0+itok.number*9);
							warningreg(regs[razr/2-1][itok.number]);
						}
					case tk_reg32:
defreg32:
						op66(razr);
						op(0x01+vop);
						op(0xC0+reg+(unsigned int)itok.number*8);
						break;
					case tk_qwordvar:
					case tk_longvar:
					case tk_dwordvar:
						i=4;
						goto wordvar;
					case tk_intvar:
					case tk_wordvar:
						if(razr==r32)goto addchar;
						i=2;
wordvar:
						CheckAllMassiv(bufrm,i,&strinf);
						op66(razr);
						outseg(&itok,2);
						op(0x03+vop);
						op(reg*8+itok.rm);
						outaddress(&itok);
						break;
					case tk_charvar:
					case tk_beg:
					case tk_bytevar:
					case tk_seg:
					case tk_rmnumber:
addchar:
						SINFO wstr;
						ITOK wtok;
						char *wbuf;
						wstr=strinf;
						strinf.bufstr=NULL;
						wbuf=bufrm;
						bufrm=NULL;
						wtok=itok;
						getinto_e_ax(sign,tok,&wtok,wbuf,&wstr,razr);
						goto addax;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
unsigned char oaddstack;
						if(!(comfile==file_w32&&(reg==EBX||reg==EDI||reg==ESI))){
							op66(razr);
							op(0x50+reg);	//push AX
						}
						addESP+=razr==r16?2:4;
						oaddstack=addstack;
						addstack=FALSE;
						procdo(razr==r16?(sign==0?tk_word:tk_int):(sign==0?tk_dword:tk_long));
						addstack=oaddstack;
						addESP-=razr==r16?2:4;
						if(!(comfile==file_w32&&(reg==EBX||reg==EDI||reg==ESI))){
							op66(razr);
							op(0x58+reg);
						}
addax:
						op66(razr);
						op(0x01+vop);
						op(0xc0+reg);
						warningreg(regs[razr/2-1][0]);
						break;
					default: valueexpected(); break;
				}
				break;
			case tk_xorminus: vop+=0x10;
			case tk_andminus: vop+=0x18;
			case tk_orminus: vop+=0x08;
			  getoperand(reg==BX?SI:BX);
				switch(tok){
					case tk_number:
						itok.number=-itok.number;
						op66(razr);
						op(0x81);
						op(0xC0+vop+reg);
						if((itok.flag&f_reloc)!=0)AddReloc();
						razr==r16?outword((unsigned int)itok.number):outdword(itok.number);
						break;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
						procdo(razr==r16?tk_word:tk_dword);
						i=EAX;
						if(*ofsstr!=NULL){
							free(*ofsstr);
							*ofsstr=NULL;
						}
						goto defxormin;
					default:
						if(reg==ECX){
							i=EAX;
							SINFO wstr;
							wstr=strinf;
							strinf.bufstr=NULL;
							ITOK wtok;
							char *wbuf;
							wbuf=bufrm;
							bufrm=NULL;
							wtok=itok;
							getinto_e_ax(sign,tok,&wtok,wbuf,&wstr,razr);
						}
						else{
							i=ECX;
							getintoreg_32(CX,razr,sign,ofsstr,FALSE);
						}
defxormin:
						NegReg(razr,i);
					 	op66(razr);
						op(vop+1);	/* opt AX,CX */
						op(0xC0+i*8+reg);
						warningreg(regs[razr/2-1][i]);
						if(i==ECX)continue;
						break;
				}
				break;
			case tk_rrminus:
				if(reg==ECX){
					regmathoperror();
					break;
				}
				tok=tk_minus;
				getintobeg(CL,ofsstr);
				op66(razr);
				op(0xD3);
				op(0xE8+reg);	// SHL xXX,CL
				warningreg(begs[1]);
				continue;
			case tk_rr:
			  getoperand(am32==TRUE?ECX:(reg==BX?SI:BX));
				if(RshiftReg(razr,reg,sign)==FALSE)regmathoperror();
				else continue;
				break;
			case tk_llminus:
				if(reg==ECX){
					regmathoperror();
					break;
				}
				tok=tk_minus;
				goto llshift;
			case tk_ll:
			  getoperand(am32==TRUE?ECX:(reg==BX?SI:BX));
				if(tok==tk_number){
					if(chip<2&&reg==CX){
						regmathoperror();
						break;
					}
					lshiftmul(itok.number,razr,reg);
				}
				else if(reg!=ECX&&(tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==CL){
					op66(razr);
					op(0xD3);
					op(0xE0+reg);	// SHL xXX,CL
				}
				else if(reg!=ECX){
llshift:
					getintobeg(CL,ofsstr);
					op66(razr);
					op(0xD3);
					op(0xE0+reg);	// SHL xXX,CL
					warningreg(begs[1]);
					continue;
				}
				else regmathoperror();
				break;
			case tk_multminus: negflag=TRUE;
			case tk_mult:
			  if(optnum==FALSE)getoperand(reg==BX?SI:BX);
				else tok=tk_number;
				if(negflag&&tok==tk_number){
					itok.number=-itok.number;
					negflag=FALSE;
				}
				if(MulReg(reg,razr)==0)continue;
				break;
			case tk_modminus: negflag=1;
			case tk_mod:
				vop=1;
				goto divcalc;
			case tk_divminus: negflag=1;
			case tk_div:
divcalc:
			  if(optnum==FALSE)getoperand(reg==BX?SI:BX);
				else tok=tk_number;
				if(negflag&&tok==tk_number){
					itok.number=-itok.number;
					negflag=FALSE;
				}
				if(tok==tk_number&&(i=caselong(itok.number))!=NUMNUM){
					if(vop){	//mod
						itok.number--;
						if(itok.number==0){
							ZeroReg(reg,razr);
						}
						else if(short_ok(itok.number,razr/2-1)){
							op66(razr);
							op(0x83);
							op(0xE0+reg);	//and reg,num-1
							op(itok.number);
						}
						else{
							op66(razr);
							op(0x81);
							op(0xE0+reg);	//and reg,num-1
							razr==r16?outword((unsigned int)itok.number):outdword(itok.number);
						}
					}
					else{
						if(i!=0){
							if(chip<2&&razr==r16){
								if(reg==CX)regmathoperror();
								op(0xB1); op(i); /* MOV CL,num */
								op(0xD3);
								op(0xE8+reg); // SHR reg,CL
								warningreg(begs[1]);
							}
							else{
								op66(razr);
								op(0xC1);
								op(0xE8+reg); // SHR reg,num
		 						op(i);
							}
						}
					}
					break;
				}
				if(fdiv!=1){
					op66(razr);
					op(0x90+reg);	//xchg AX,reg
				}
				DivMod(vop,sign,razr,0);
				if(itok.type!=tp_stopper){
					if(vop==1){
						op66(razr);
						if(optimizespeed)outword(0xC28B);	//mov ax,dx
						else op(0x92);	//xchg ax,dx
					}
					do_e_axmath2(0,razr,0);
				}
				else if(vop==1){
					if(reg!=EDX){
						op66(razr);
						op(0x89);
						op(128+64+EDX*8+reg);
					}
					warningreg(regs[razr/2-1][EAX]);
					continue;
				}
				op66(razr);
				if(optimizespeed){
					op(0x89);
					op(128+64+EAX*8+reg);
				}
				else op(0x90+reg);	//xchg AX,reg
				warningreg(regs[razr/2-1][EAX]);
				continue;
			default: operatorexpected(); break;
		}
		calcnumber=FALSE;
		ClearReg(reg);
		nexttok();
	}
	calcnumber=FALSE;
	ClearReg(reg);
	if(razr==r32&&cpu<3)cpu=3;
}

void  dobegmath(int beg)  // math done is on all begs except AL
// all other registers preserved
{
int vop,i,optnum=FALSE,negflag=FALSE;
	while(itok.type!=tp_stopper&&tok!=tk_eof){
		vop=0;
		i=0;
#ifdef OPTVARCONST
		if(tok2>=tk_charvar&&tok2<=tk_doublevar&&itok2.npointr==0){
			if(CheckConstVar(&itok2)){
				tok2=tk_number;
				calcnumber=TRUE;
			}
		}
#endif
		if(tok2==tk_number)optnum=OptimNum();
		int oldtok=tok;
		switch(tok){
			case tk_xor: vop+=0x08;
			case tk_minus: vop+=0x08;
			case tk_and: vop+=0x18;
			case tk_or: vop+=0x08;
			case tk_plus:
			  if(optnum==FALSE)getoperand(beg==BL||beg==BH?SI:BX);
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				switch(tok){
					case tk_number:
						if(itok.number==0&&oldtok!=tk_and)break;
						else if(itok.number==1){
							if(oldtok==tk_plus){
								op(0xFE);
								op(0xC0+beg);
								break;
							}
							if(oldtok==tk_minus){
								op(0xFE);
								op(0xC8+beg);
								break;
							}
						}
						else if((unsigned char)itok.number==0xff){
							if(oldtok==tk_minus){
								op(0xFE);
								op(0xC0+beg);
								break;
							}
							if(oldtok==tk_plus){
								op(0xFE);
								op(0xC8+beg);
								break;
							}
						}
					  op(0x80);
						op(0xC0+vop+beg);
						op((unsigned int)itok.number);
						break;
					case tk_bits:
						int vops,reg2s;
						i=itok.bit.siz+itok.bit.ofs;
						if(i<=64)vops=r64;
						if(i<=32)vops=r32;
						if(i<=16)vops=r16;
						if(i<=8)vops=r8;
						reg2s=CL;
						if(beg==CL)reg2s=BL;
						bits2reg(reg2s,vops);
						if(vops==r64)vops=r32;
						warningreg(vops==r8?begs[reg2s]:regs[vops/2-1][reg2s]);
						itok.number=reg2s;
					case tk_beg:
						op(0x00+vop);
						op(0xC0+beg+(unsigned int)itok.number*8);
						break;
					case tk_qwordvar:
						i=4;
					case tk_longvar:
					case tk_dwordvar:
						i+=2;
					case tk_intvar:
					case tk_wordvar:
						i++;
					case tk_bytevar:
					case tk_charvar:
						i++;
						CheckAllMassiv(bufrm,i,&strinf);
						outseg(&itok,2);
						op(0x02+vop);
						op(beg*8+itok.rm);
						outaddress(&itok);
						break;
					case tk_postnumber:
					case tk_reg:
					case tk_rmnumber: begworderror();	break;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:// begcallerror(); break;
					case tk_declare:
						procdo(tk_byte);
						if(beg!=AL){
							op(0x00+vop);
							op(0xc0+beg);
						}
						break;
					default: valueexpected(); break;
				}
				break;
			case tk_xorminus: vop+=0x10;
			case tk_andminus: vop+=0x18;
			case tk_orminus: vop+=0x08;
			  getoperand(beg==BL||beg==BH?SI:BX);
				if(tok==tk_number){
					itok.number=-itok.number;
					op(0x80);
					op(0xC0+vop +beg);
					op((unsigned int)itok.number);
				}
				else negregerror();
				break;
			case tk_rr:
				vop=8;
			case tk_ll:
				nexttok();
				if(tok==tk_number){
					if((unsigned int)itok.number==1){
						if(vop==0){
							op(2);
							op(0xC0+9*beg); // ADD reg,reg
						}
						else{
							op(0xD0); op(0xE8+beg);  /* SHR reg,1 */
						}
					}
					else if((unsigned int)itok.number!=0){
						if(chip<2)begmathoperror();
						else{
							op(0xc0);
							op(0xe0+beg+vop); // SHL reg,imm8
							op((unsigned int)itok.number);
							if(cpu<2)cpu=2;
						}
					}
				}
				else if(tok==tk_beg&&itok.number==CL&&beg!=CL){
					op(0xD2);
					op(0xE0+beg+vop);	// SHL xXX,CL
				}
				else begmathoperror();
				break;
			case tk_multminus: negflag=TRUE;
			case tk_mult:
			  if(optnum==FALSE)getoperand(beg==BL||beg==BH?SI:BX);
				else{
					tok=tk_number;
					optnum=FALSE;
				}
				if(negflag&&tok==tk_number){
					itok.number=-itok.number;
					negflag=FALSE;
				}
				switch(tok){
					case tk_number:
						itok.number&=255;
						switch((unsigned int)itok.number){
							case 0: // beg * 0 = MOV beg,0
								outword(0x00B0+beg);
							case 1: break; //beg*1=beg
							case 2:
								op(0);
								op(0xC0+9*beg); // beg * 2 = ADD beg,beg
								break;
							default:
								vop=caselong(itok.number);
								if(vop!=NUMNUM){
									if(chip<1)begmathoperror();
									else{
										op(0xc0);
										op(0xe0+beg);	//SHL beg,num
										op(vop);
										if(cpu<1)cpu=1;
									}
								}
								else begmathoperror();
						}
						break;
					default: begmathoperror(); break;
				}
				break;
			case tk_divminus:
			case tk_modminus:
			case tk_div:
			case tk_mod:
			case tk_rrminus:
			case tk_llminus:
				begmathoperror();	break;
			default: operatorexpected(); break;
		}
#ifdef OPTVARCONST
		calcnumber=FALSE;
#endif
		nexttok();
		ClearReg(beg%4);
	}
}

// ============= getintoreg_32(), getintobeg() ============

int getintoreg_32(int reg,int razr,int sign,char **ofsstr,int useloop)	// get into word reg (except AX) with enum
{
int negflag=0,next=1,i=0;
int swap=0,oflag=0;
unsigned long holdnumber=0;
int reg1=idxregs[0],reg2=idxregs[1];
int rettype=tk_reg;
int loop=0,otok;
//				printf("line %d tok=%d %s\n",linenumber,tok,itok.name);
	if(!am32){
		if(reg==idxregs[1]||reg==idxregs[2]){
			reg1=reg;
			if(reg==idxregs[1])reg2=idxregs[0];
		}
	}
	else{
		reg1=reg;
		if(reg==idxregs[1])reg2=idxregs[0];
	}
	if(tok==tk_minus){
		if(CheckMinusNum()==FALSE){
			negflag=1;
			getoperand(am32==FALSE?BX:reg);
		}
	}
loopswitch:

	if(uselea){
		if(razr==r32){
			if(cpu<3)cpu=3;
			if(Reg32ToLea(reg)){
				return tk_reg;
			}
		}
		else
			if(Reg16ToLea(reg)){
				return tk_reg;
			}
	}
loopswitch1:

#ifdef OPTVARCONST
	CheckConstVar3(&tok,&itok,razr);
	if(tok==tk_number)calcnumber=TRUE;
#endif
	otok=tok;
//	printf("reg=%d tok=%d\n",reg,tok);
	switch(tok){
		case tk_number:
			if(useloop==FALSE)MovRegNum(razr,itok.flag&f_reloc,itok.number,reg);
			else{
				holdnumber=CalcNumber(sign);
				if(loop==0)oflag=postnumflag;
				else oflag^=postnumflag;
				loop++;
				if(tok==tk_mult){
					nexttok();
					swap=1;
					goto loopswitch1;
				}
				if(tok==tk_plus&&tok2==tk_postnumber){
					nexttok();
//		 		getoperand(am32==TRUE?EAX:BX);
					goto loopswitch1;
				}
				MovRegNum(razr,oflag&f_reloc,holdnumber,reg);
				next=0;
			}
			break;
		case tk_postnumber:
		case tk_undefofs:
			if(!swap){
				op66(razr);
				op(0xB8+reg);			/* MOV AX,# */
				swap=1;
			}
			if(tok==tk_undefofs){
				AddUndefOff(2,itok.name);
//				AddUndefOff(0,itok.name);
//				itok.flag=0;	//new 07.07.04 23:57
			}
			else (itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
			if(useloop==FALSE)holdnumber=itok.number;
			else{
				tok=tk_number;
				holdnumber+=doconstdwordmath();
				if(otok!=tk_postnumber){
					if(loop==0)oflag=postnumflag;
					else oflag^=postnumflag;
					loop++;
				}
				if(tok==tk_plus&&tok2==tk_postnumber){
					nexttok();
					goto loopswitch1;
				}
				swap=0;
				next=0;
			}
			if(useloop&&(oflag&f_reloc))AddReloc();
			if(razr==r16)outword(holdnumber);
			else outdword(holdnumber);
			ClearReg(reg);
			break;
		case tk_apioffset:
			op66(razr);
			op(0xB8+reg);			/* MOV AX,# */
			AddApiToPost(itok.number);
			ClearReg(reg);
			break;
		case tk_rmnumber:
			CheckAllMassiv(bufrm,itok.size,&strinf,&itok,reg1,reg2);
			if((am32&&itok.rm==reg&&reg!=EBP&&reg!=ESP)||(am32==0&&reg==itok.rm&&
					(reg==SI||reg==DI||reg==BX)))break;
			op66(razr);
			op67(itok.sib==CODE16?r16:r32);
			if(itok.post==0)outseg(&itok,2);
			op(0x8D);			// LEA reg,[rm]
			op(reg*8+itok.rm);
			if(itok.post!=0&&itok.post!=UNDEF_OFSET){
				if((itok.flag&f_extern)==0){
					unsigned int ooutptr=outptr;
					if(am32&&itok.rm==rm_sib)outptr++;
					setwordpost(&itok);
					outptr=ooutptr;
				}
				else setwordext(&itok.number);
			}
			if(useloop==FALSE)outaddress(&itok);
			else{
ITOK oitok;
				oitok=itok;
				tok=tk_number;
				itok.rm=tk_dword;
				oitok.number=doconstdwordmath();
				outaddress(&oitok);
				next=0;
			}
			ClearReg(reg);
			break;
		case tk_qwordvar:
			i=4;
		case tk_longvar:
		case tk_dwordvar:
			i+=4;
dwordvar:
			CheckAllMassiv(bufrm,i,&strinf,&itok,reg1,reg2);
	 		op66(razr);
			outseg(&itok,2);
			op(0x8B);
			op(reg*8+itok.rm);
			outaddress(&itok);
			ClearReg(reg);
			break;
		case tk_intvar:
		case tk_wordvar:
			i=2;
			if(razr==r16)goto dwordvar;
			CheckAllMassiv(bufrm,2,&strinf,&itok,reg1,reg2);
			if(tok==tk_wordvar&&optimizespeed&&chip>3&&chip<7&&RmEqualReg(reg,itok.rm,itok.sib)==FALSE){
				ZeroReg(reg,r32);
	 			op66(r16);
				outseg(&itok,2);	//mov reg,var
				op(0x8B);
			}
			else{
		 		op66(r32);
				outseg(&itok,3);	//movxx reg,var
				op(0x0F); op(tok==tk_wordvar?0xB7:0xBF);
			}
			op(reg*8+itok.rm);
			outaddress(&itok);
			ClearReg(reg);
			break;
		case tk_doublevar:
			i=4;
		case tk_floatvar:
			Float2reg32(reg,i,reg1,reg2);
			ClearReg(reg);
			break;
		case tk_bytevar:
		case tk_charvar:
			if(chip>2||razr==r32){
				CheckAllMassiv(bufrm,1,&strinf,&itok,reg1,reg2);
				if(reg<=EBX&&tok==tk_bytevar&&optimizespeed&&chip>3&&chip<7&&RmEqualReg(reg,itok.rm,itok.sib)==FALSE){
					ZeroReg(reg,razr);
					outseg(&itok,2);
					op(0x8A);
				}
				else{
					op66(razr);
					outseg(&itok,3);
					op(0xf);
					if(tok==tk_bytevar)op(0xb6);
					else op(0xbe);
				}
				op(reg*8+itok.rm); // MOVZX regL,[byte]
				outaddress(&itok);
				ClearReg(reg);
				break;
			}
			if(reg<=BX){
				if(reg==AX&&itok.rm==rm_d16&&itok.sib==CODE16){
					outseg(&itok,1);
					op(0xA0); // MOV AL,[byte]
					outword((unsigned int)itok.number);
				}
				else{
					CheckAllMassiv(bufrm,1,&strinf,&itok,reg1,reg2);
					outseg(&itok,2);
					op(0x8A);
					op(reg*8+itok.rm); // MOV regL,[byte]
					outaddress(&itok);
				}
				ClearReg(reg);
				op(0x30); op(0xC0+(reg+4)*9); // XOR regH,regH
			}
			else regbyteerror();
			break;
		case tk_reg:
			if(razr==r32){
				if(tok2==tk_openbracket){	//вызов процедуры по адресу в регистре
					reg1=itok.number;
					nexttok();
					param[0]=0;
					if(comfile==file_w32)swapparam();
					else doparams();
					op66(r16);
					op(0xFF);
					op(0xD0+reg1); 	/* CALL reg with stack params */
					itok.number=0;
					clearregstat();
#ifdef OPTVARCONST
					FreeGlobalConst();
#endif
				}
				if(optimizespeed&&chip>3&&chip<7&&reg!=(int)itok.number){
					ZeroReg(reg,r32);
					op(0x89);
					op(0xC0+reg+(unsigned int)itok.number*8);
				}
				else{
					op66(r32);
					outword(0xB70F);
					op(0xC0+reg*8+(unsigned int)itok.number);
				}
				RegToReg(reg,itok.number,r32);
				break;
			}
		case tk_reg32:
			if(tok2==tk_openbracket){	//вызов процедуры по адресу в регистре
				reg1=itok.number;
				reg2=tok==tk_reg32?r32:r16;
				nexttok();
				param[0]=0;
				if(comfile==file_w32)swapparam();
				else doparams();
				op66(reg2);
				op(0xFF);
				op(0xD0+reg1); 	/* CALL reg with stack params */
				itok.number=0;
				clearregstat();
#ifdef OPTVARCONST
				FreeGlobalConst();
#endif
			}
			if(reg!=(int)itok.number){
				op66(razr);
				op(0x89);
				op(0xC0+reg+(unsigned int)itok.number*8);
				RegToReg(reg,itok.number,r32);
			}
			break;
		case tk_bits:
			int vops;
			i=itok.bit.siz+itok.bit.ofs;
			if(i<=64)vops=r64;
			if(i<=32)vops=r32;
			if(i<=16)vops=r16;
			bits2reg(reg,(razr<vops?vops:razr));
			break;
		case tk_seg:
			op66(razr);
			op(0x8C);
			op(0xC0+reg+(unsigned int)itok.number*8);
			ClearReg(reg);
			break;
		case tk_beg:
			if(chip>2||razr==r32){
				if(optimizespeed&&chip>3&&chip<7&&reg<4&&reg!=(int)(itok.number%4)){
					ZeroReg(reg,razr);
					op(0x88);
					op(0xC0+reg+(unsigned int)itok.number*8); // MOV regL,beg
				}
				else{
					op66(razr);
					outword(0xb60f);
					op(0xC0+reg*8+(unsigned int)itok.number); // MOVZX regL,beg
				}
			}
			else if(reg>BX)regbyteerror();
			else{
				op(0x88); op(0xC0+reg+(unsigned int)itok.number*8); // MOV regL,beg
				op(0x30); op(0xC0+(reg+4)*9);	// XOR regH,regH
			}
			ClearReg(reg);
			break;
		case tk_at:
			getoperand(am32==FALSE?BX:reg);
			i++;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			if(itok.flag&f_retproc)rettype=(itok.flag&f_retproc)/256+tk_overflowflag-1;
			if((!i)||macros(razr==r16?tk_word:tk_dword)==0)procdo(razr==r16?tk_word:tk_dword);
			itok.number=0;
			tok=(razr==r16?tk_reg:tk_reg32);
			if(*ofsstr!=NULL){
				free(*ofsstr);
				ofsstr=NULL;
			}
//		printf("tok=%d num=%d tok2=%d\n",tok,itok.number,tok2);
			goto loopswitch;
//			break;
		case tk_string:
			op66(razr);
			op(0xB8+reg);
			if(razr==r16){
				if(am32)dwordvalexpected();
				outword(addpoststring());
			}
			else outdword(addpoststring());
			ClearReg(reg);
			break;
		default: valueexpected();	break;
	}
#ifdef OPTVARCONST
	calcnumber=FALSE;
#endif
	if(negflag)NegReg(razr,reg);
	if(swap){
		negflag=0;
		RegMulNum(reg,holdnumber,razr,0,&negflag,oflag);
		ClearReg(reg);
	}
	if(next)nexttok();
//		printf("tok=%d num=%d tok2=%d\n",tok,itok.number,tok2);
	return rettype;
}

int getintobeg(int beg,char **ofsstr)	// get into beg (CL,DL,BL not others) with enum
{
int negflag=0,i=0;
int rettype=tk_beg;
	if(tok==tk_minus){
		if(CheckMinusNum()==FALSE){
			negflag=1;
			getoperand(am32==TRUE?(beg>3?beg-4:beg):BX);
		}
	}
#ifdef OPTVARCONST
	if(tok>=tk_charvar&&tok<=tk_doublevar&&itok.npointr==0){
//		printf("type1=%d type2=%d\n",itok.type,itok2.type);
		if(CheckConstVar(&itok)){
			tok=tk_number;
			calcnumber=TRUE;
		}
	}
#endif
	switch(tok){
		case tk_number:
			op(0xB0+beg);
			i=(int)doconstlongmath();
			op(i);
			ConstToReg(i,beg,r8);
			break;
		case tk_rmnumber:
			CheckAllMassiv(bufrm,itok.size,&strinf);
			op66(r16);
			op67(itok.sib==CODE16?r16:r32);
			if(itok.post==0)outseg(&itok,2);
			op(0x8D);				// LEA reg,[rm]
			op(beg*8+itok.rm);
			if(itok.post!=0&&itok.post!=UNDEF_OFSET){
				if((itok.flag&f_extern)==0){
					unsigned int ooutptr=outptr;
					if(am32&&itok.rm==rm_sib)outptr++;
					setwordpost(&itok);
					outptr=ooutptr;
				}
				else setwordext(&itok.number);
			}
			outaddress(&itok);
			ClearReg(beg%4);
			break;
		case tk_postnumber:
			op66(r16);
			op(0xB8+beg);
			(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
			outword((unsigned int)itok.number);
			nexttok();
			ClearReg(beg%4);
			break;
		case tk_qwordvar:
			i=4;
		case tk_longvar:
		case tk_dwordvar:
			i+=2;
		case tk_intvar:
		case tk_wordvar:
			i++;
		case tk_charvar:
		case tk_bytevar:
			i++;
			CheckAllMassiv(bufrm,i,&strinf);
			outseg(&itok,2);
			op(0x8A);
			op(beg*8+itok.rm);
			outaddress(&itok);
			nexttok();
			ClearReg(beg%4);
			break;
		case tk_doublevar:
			i=4;
		case tk_floatvar:
			Float2reg32((beg<4?beg:beg-4),i);
			if(beg>3){
				op(0x88);
				op(0xC0+beg+8*(beg-4));
			}
			nexttok();
			ClearReg(beg%4);
			break;
		case tk_bits:
			int vops;
			i=itok.bit.siz+itok.bit.ofs;
			if(i<=64)vops=r64;
			if(i<=32)vops=r32;
			if(i<=16)vops=r16;
			if(i<=8)vops=r8;
			if(beg>BX&&vops!=r8){
				op66(vops==r64?r32:vops);
				op(0x50);
				if(ESPloc&&am32&&itok.segm==SS)itok.number+=4;
				addESP+=vops==r16?2:4;
				bits2reg(AX,vops);
				op(0x88);
				op(0xC0+beg);
				op66(vops==r64?r32:vops);
				addESP-=vops==r16?2:4;
				op(0x58);
			}
			else bits2reg(beg,vops);
			nexttok();
			break;
		case tk_beg:
			if(beg!=(int)itok.number){
				op(0x88);
				op(0xC0+beg+(unsigned int)itok.number*8);
				RegToReg(beg,itok.number,r8);
			}
			nexttok();
			break;
		case tk_reg32:
		case tk_reg:
			if(beg<BX){
				if(beg!=(int)itok.number){
					op66(r16);
					op(0x89);
					op(0xC0+beg+(unsigned int)itok.number*8);
					RegToReg(beg,itok.number,r8);
				}
			}
			else begworderror();
			nexttok();
			break;
		case tk_seg:
			op66(r16);
			op(0x8C);
			op(0xC0+beg+(unsigned int)itok.number*8);
			nexttok();
			ClearReg(beg%4);
			break;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			if(itok.flag&f_retproc)rettype=(itok.flag&f_retproc)/256+tk_overflowflag-1;
			procdo(tk_byte);
			op(0x88);
			op(0xc0+beg);
			nexttok();
			RegToReg(beg,AL,r8);
			if(*ofsstr){
				free(*ofsstr);
				*ofsstr=NULL;
			}
			break;
		default: valueexpected();	nexttok(); return 0;
	}
#ifdef OPTVARCONST
	calcnumber=FALSE;
#endif
	if(negflag){
		if(optimizespeed&&(chip==5||chip==6)){
			op(0x80);
			op(0xF0+beg);
			op(0xff);
			op(0xFE);
			op(0xC0+beg);
		}
		else{
			op(0xF6);
			op(0xD8+beg); 	// NEG beg
		}
		ClearReg(beg%4);
	}
	return rettype;
}

void  outaddress(ITOK *outtok)
{
int rm=outtok->rm;
	if(outtok->sib==CODE16){
		if(rm==rm_d16){
			if(outtok->post==UNDEF_OFSET){
				AddUndefOff(2,outtok->name);
				outtok->post=0;
			}
			outword(outtok->number);
		}
		else{
			rm&=rm_mod11;
			if(rm==rm_mod11)internalerror(badadr);
			else if(rm==rm_mod10){
				if(outtok->post==UNDEF_OFSET){
					AddUndefOff(2,outtok->name);
					outtok->post=0;
				}
				outword(outtok->number);
			}
			else if(rm!=rm_mod00)op(outtok->number);
		}
	}
	else{
		if(rm==rm_d32){
			if(outtok->post==UNDEF_OFSET){
				AddUndefOff(2,outtok->name);
				outtok->post=0;
			}
			outdword(outtok->number);
		}
		else{
			if((rm&7)==rm_sib){
				op(outtok->sib);
				if(rm==4&&(outtok->sib&7)==5){
					if(outtok->post==UNDEF_OFSET){
						AddUndefOff(2,outtok->name);
						outtok->post=0;
					}
					outdword(outtok->number);
				}
			}
			rm&=rm_mod11;
			if(rm==rm_mod11)internalerror(badadr);
			else if(rm==rm_mod10){
				if(outtok->post==UNDEF_OFSET){
					AddUndefOff(2,outtok->name);
					outtok->post=0;
				}
				outdword(outtok->number);
			}
			else if(rm==rm_mod01)op(outtok->number);
		}
	}
}

/*-----------------05.01.00 23:37-------------------
 Обработка float
 операции с переменными типа float
 --------------------------------------------------*/
int dofloatvar(int addop,int retrez,int terminater)
{
unsigned char next=1, getfromEAX=0;
unsigned int vop=0,rettype=tk_float,posiblret=tk_float,sign=0;
char *wbuf,*rbuf;
ITOK wtok;
SINFO wstr;
int pointr=0;
int razr,i=0;
int type=tk_floatvar;
	if(addop){
		rettype=posiblret=tk_double;
		razr=8;
		type=tk_doublevar;
	}
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	KillVar(itok.name);
char *ofsstr=NULL;
	nexttok();
	switch(tok){
		case tk_assign:	//=
			getoperand(am32==TRUE?EAX:BX);
			convert_type((int *)&sign,(int *)&rettype,&pointr);
			if(tok2==tk_assign){
				MultiAssignFloat(type);
				next=0;
				goto getfromeax;
			}
			CheckMinusNum();
			if(itok2.type==tp_opperand){	//составное
				if(tok==tk_number){	//проверка и суммирование чисел
					if(OnlyNumber(rettype==tk_float?2:3)){
						next=0;
						itok.flag=(unsigned char)postnumflag;
						itok.rm=rettype;
						goto numbertovar;
					}
				}
				getfromEAX=1;
				if(rettype==tk_char||rettype==tk_byte)doalmath(sign,&ofsstr);
				else if(rettype==tk_int||rettype==tk_word)do_e_axmath(sign,r16,&ofsstr);
				else if(rettype==tk_long||rettype==tk_dword)do_e_axmath(sign,r32,&ofsstr);
				else goto labl1;
			}
			else{
				switch(tok){
					case tk_number:
numbertovar:
						if(rettype==tk_float){
							if(itok.rm==tk_double)itok.fnumber=itok.dnumber;
							else if(itok.rm!=tk_float){
								float temp=itok.number;
								*(float *)&itok.number=temp;
							}
						}
						else{
							if(itok.rm==tk_float)itok.dnumber=itok.fnumber;
							else if(itok.rm!=tk_double)itok.dnumber=itok.lnumber;
						}
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						for(i=0;i<2;i++){
							op66(r32);
		 					if((itok.flag&f_reloc)==0&&itok.number==0){
								outseg(&wtok,2);
								op(0x83);
								op(wtok.rm+0x20);
								outaddress(&wtok);
								op(0);
							}
							else if(regoverstack&&short_ok(itok.number,TRUE)&&(itok.flag&f_reloc)==0){
								op(0x6A);
								op(itok.number);	//push short number
								op66(r32);
								outseg(&wtok,2);
								op(0x8f);
								op(wtok.rm);
								outaddress(&wtok);
							}
							else{
								outseg(&wtok,2);
								op(0xC7);	//mov word[],number
								op(wtok.rm);
								outaddress(&wtok);
								if((itok.flag&f_reloc)!=0)AddReloc();
								outdword(itok.number);
							}
							if(i==1||rettype==tk_float)break;
							itok.lnumber>>=32;
							wtok.number+=4;
							compressoffset(&wtok);
						}
						break;
					case tk_doublevar:
					case tk_floatvar:
						if(tok!=type)goto labl1;
						ITOK w1tok;
						char *w1buf;
						w1buf=bufrm;
						bufrm=NULL;
						w1tok=itok;
						SINFO w1str;
						w1str=strinf;
						strinf.bufstr=NULL;
						getinto_e_ax(0,tok==tk_floatvar?tk_dwordvar:tk_qwordvar,&w1tok,w1buf,&w1str,r32);
						if(tok==tk_doublevar){
							if((wtok.rm==rm_d16&&wtok.sib==CODE16)||(wtok.rm==rm_d32&&wtok.sib==CODE32)){
								op66(r32);
								outseg(&wtok,1);
								op(0xA3);	// MOV [dword],EAX
								if(am32)outdword(wtok.number);
								else outword(wtok.number);
							}
							else{
								CheckAllMassiv(wbuf,4,&wstr,&wtok);
								op66(r32);
								outseg(&wtok,2);
								op(0x89); op(wtok.rm); // MOV [rmdword],EAX
								outaddress(&wtok);
							}
							itok.number+=4;
							compressoffset(&itok);
							wtok.number+=4;
							compressoffset(&wtok);
							ITOK w1tok;
							char *w1buf;
							w1buf=bufrm;
							bufrm=NULL;
							w1tok=itok;
							SINFO w1str;
							w1str=strinf;
							strinf.bufstr=NULL;
							getinto_e_ax(0,tk_qwordvar,&w1tok,w1buf,&w1str,r32);
						}
						getfromEAX=1;
						break;
					default:
labl1:
						if((i=doeaxfloatmath(tk_fpust))==tk_fpust||i==tk_double){
							getfromEAX=0;
							if(retrez==tk_floatvar||retrez==tk_doublevar){
								CheckAllMassiv(wbuf,razr,&wstr,&wtok);
								outseg(&wtok,2);
								op(0xd9+addop);
								op(wtok.rm+0x18);
								outaddress(&wtok);
								fwait3();
							}
							else retrez=tk_fpust;
						}
						else getfromEAX=1;
						next=0;
				}
			}
			if(getfromEAX){
getfromeax:
				retrez=tk_reg32;
				convert_returnvalue(posiblret,rettype);
				if((wtok.rm==rm_d16&&wtok.sib==CODE16)||(wtok.rm==rm_d32&&wtok.sib==CODE32)){
					op66(r32);
					outseg(&wtok,1);
					op(0xA3);	// MOV [dword],EAX
					if(am32)outdword(wtok.number);
					else outword(wtok.number);
				}
				else{
					CheckAllMassiv(wbuf,4,&wstr,&wtok);
					op66(r32);
					outseg(&wtok,2);
					op(0x89); op(wtok.rm); // MOV [rmdword],EAX
					outaddress(&wtok);
				}
			}
			break;
		case tk_minusminus:
			vop=0x28;
		case tk_plusplus:
incvar:
			outword(0xe8d9);	//fld1
			CheckAllMassiv(wbuf,razr,&wstr,&wtok);
			outseg(&wtok,2);	//fadd var
			op(0xd8+addop);
			op(wtok.rm+vop);
			outaddress(&wtok);
			if(retrez==tk_floatvar||retrez==tk_doublevar){
				outseg(&wtok,2);	//fstp var
				op(0xd9+addop);
				op(wtok.rm+0x18);
				outaddress(&wtok);
				fwait3();
			}
			break;
		case tk_divequals: vop+=0x10;
		case tk_minusequals: vop+=0x20;
		case tk_multequals: vop+=8;
		case tk_plusequals:
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_stopper){
				if(tok==tk_number){
					if((itok.rm==tk_float&&itok.fnumber==1.0)||
							(itok.rm==tk_double&&itok.dnumber==1.0)||
							(itok.rm!=tk_float&&itok.rm!=tk_double&&itok.lnumber==1)){
						if(vop==0||vop==0x28)goto incvar;
						break;
					}
					if((itok.rm==tk_float&&itok.fnumber==0.0)||
							(itok.rm==tk_double&&itok.dnumber==0.0)||itok.lnumber==0){
						switch(vop){
							case 0x38:
								DevideZero();
								break;
							case 8:
								outword(0xEED9);	//FLDZ
					 			if(retrez==tk_floatvar||retrez==tk_doublevar){
									CheckAllMassiv(wbuf,razr,&wstr,&wtok);
									outseg(&wtok,2);	//fstp var
									op(0xd9+addop);
									op(wtok.rm+0x18);
									outaddress(&wtok);
									fwait3();
								}
								break;
						}
						break;
					}
				}
			}
			if(itok2.type==tp_opperand){
				doeaxfloatmath(tk_fpust);
endequals:
				next=0;
endequals1:
				CheckAllMassiv(wbuf,razr,&wstr,&wtok);
				outseg(&wtok,2);	//fsubr var
				op(0xd8+addop);
				op(wtok.rm+vop);
				outaddress(&wtok);
				if(retrez==tk_floatvar||retrez==tk_doublevar){
					outseg(&wtok,2);	//fstp var
					op(0xd9+addop);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					fwait3();
				}
			}
			else{
				switch(tok){
					case tk_number:
						if(rettype==tk_float){
							if(itok.rm==tk_double)itok.fnumber=itok.dnumber;
							else if(itok.rm!=tk_float){
								float temp=itok.number;
								*(float *)&itok.number=temp;
							}
						}
						else{
							if(itok.rm==tk_float)itok.dnumber=itok.fnumber;
							else if(itok.rm!=tk_double)itok.dnumber=itok.lnumber;
						}

						if(vop==0x38){	// div 22.12.05 22:10
							vop=8;	//mult
							if(itok.rm==tk_float)itok.fnumber=1/itok.fnumber;
							else itok.dnumber=1/itok.dnumber;
						}

						op66(r32);
						op(0xD9+addop);
						op((am32==FALSE?0x06:0x05));	//fld
						AddFloatConst(itok.lnumber,rettype);
						outword(0);
						if(am32)outword(0);
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						outseg(&wtok,2);	//fsubr var
						op(0xd8+addop);
						op(wtok.rm+vop);
						outaddress(&wtok);
						if(retrez==tk_floatvar||retrez==tk_doublevar){
							outseg(&wtok,2);	//fstp var
							op(0xd9+addop);
							op(wtok.rm+0x18);
							outaddress(&wtok);
							fwait3();
						}
						break;
					case tk_longvar:
						sign=2;
					case tk_floatvar:
						CheckAllMassiv(bufrm,4,&strinf);
						outseg(&itok,2);	//fld val or fild val
						op(0xd9+sign);
						op(itok.rm);
						outaddress(&itok);
						goto endequals1;
					case tk_qwordvar:
						sign=2;
						i=0x28;
					case tk_doublevar:
						CheckAllMassiv(bufrm,8,&strinf);
						outseg(&itok,2);	//fldq val or fild val
						op(0xdd+sign);
						op(itok.rm+i);
						outaddress(&itok);
						goto endequals1;
					case tk_dwordvar:
						CheckInitBP();
						op66(r32); 		//push 0L
						outword(0x6a);
						if(ESPloc&&am32&&itok.segm==SS)itok.number+=4;
						addESP+=4;
						CheckAllMassiv(bufrm,4,&strinf);
						op66(r32);		//push var
						outseg(&itok,2);
						op(0xFF);
						op(itok.rm+0x30);
						outaddress(&itok);
						if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=4;
						addESP+=4;
						fildq_stack();
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						outseg(&wtok,2);	//fsubr var
						op(0xd8+addop);
						op(wtok.rm+vop);
						outaddress(&wtok);
						if(optimizespeed||am32==0){
							outword(0xC483);
							op(8);
						}
						else{
							op(0x58);	// pop EAX
							op(0x58);	// pop EAX
						}
						addESP-=8;
						if(retrez==tk_floatvar||retrez==tk_doublevar){
							outseg(&wtok,2);	//fstp var
							op(0xd9+addop);
							op(wtok.rm+0x18);
							outaddress(&wtok);
							fwait3();
						}
						RestoreBP();
						break;
					case tk_reg32:	//добавить обработку интерпритации float, long
						CheckInitBP();
						op66(r32);	//push 0L
						outword(0x6a);
						op66(r32);  //push reg32
						op(0x50+(unsigned int)itok.number);
						if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=8;
						addESP+=8;
						fildq_stack();
						CheckAllMassiv(wbuf,razr,&wstr,&wtok);
						outseg(&wtok,2);	//fsubr var
						op(0xd8+addop);
						op(wtok.rm+vop);
						outaddress(&wtok);
						if(optimizespeed||am32==0){
							outword(0xC483);
							op(8);
						}
						else{
							op(0x58);	// pop EAX
							op(0x58);	// pop EAX
						}
						addESP-=8;
						if(retrez==tk_floatvar||retrez==tk_doublevar){
							outseg(&wtok,2);	//fstp var
							op(0xd9+addop);
							op(wtok.rm+0x18);
							outaddress(&wtok);
							fwait3();
						}
						RestoreBP();
						break;
					default:
						if(doeaxfloatmath(tk_fpust)==tk_assign){
							CheckInitBP();
							op66(r32);  //push EAX
							op(0x50);
							op(0xD9+addop);
							fld_stack(4+localsize);
							fwait3();
							RestoreBP();
							op66(r32);
							op(0x58);
						}
						goto endequals;
				}
			}
			break;
		case tk_swap:
			int regdi;
			regdi=TRUE;
			getoperand();
			rbuf=bufrm;
			bufrm=NULL;
			if(am32!=FALSE&&wbuf!=NULL&&wstr.bufstr!=NULL)regdi=FALSE;
			switch(tok){
				case tk_reg32:	//добавить обработку интерпритации float, long
					CheckInitBP();
					op66(r32);   		//push 0L
					outword(0x6a);
					op66(r32);     //push reg32
					op(0x50+(unsigned int)itok.number);
					if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=8;
					addESP+=8;
					fildq_stack();
					CheckAllMassiv(wbuf,razr,&wstr,&wtok);
					outseg(&wtok,2);	//fld val
					op(0xd9+addop);
					op(wtok.rm);
					outaddress(&wtok);
					op(0xdb);
					if(am32){
						if(optimizespeed)outword(0x241c);	//fistp ssdword[bp-8]/ssdword[esp]
						else{
							outword(0x245C);
							op(4);
						}
					}
					else{
						int dob;
						dob=8;
						if(!optimizespeed)dob=4;
						if(short_ok(localsize+dob,FALSE)==0){
							op(0x9E);
							outword(-dob-localsize);
						}
						else{
							op(0x5E);
							op(-dob-localsize);
						}
					}
					outseg(&wtok,2);//fstp val
					op(0xd9+addop);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					fwait3();
					op66(r32);     // pop reg32
				  op(0x58+(unsigned int)itok.number);
					if(!optimizespeed){
						op66(r32);     // pop reg32
					  op(0x58+(unsigned int)itok.number);
					}
					else{
						outword(0xC483);
						op(4);
					}
					addESP-=8;
					RestoreBP();
					break;
				case tk_longvar:
					CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
					outseg(&itok,2);	//fild
					op(0xDB);
					op(itok.rm);
					outaddress(&itok);
					CheckAllMassiv(wbuf,razr,&wstr,&wtok);
					outseg(&wtok,2);	//fld val
					op(0xd9+addop);
					op(wtok.rm);
					outaddress(&wtok);
					outseg(&itok,2);//fistp var
					op(0xDB);
					op(itok.rm+0x18);
					outaddress(&itok);
					outseg(&wtok,2);	//fstp val
					op(0xd9+addop);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					fwait3();
					break;
				case tk_qwordvar:
					CheckAllMassiv(rbuf,8,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
					outseg(&itok,2);	//fildq val
					op(0xdd);
					op(itok.rm+0x28);
					outaddress(&itok);
					CheckAllMassiv(wbuf,razr,&wstr,&wtok);
					outseg(&wtok,2);	//fld val
					op(0xd9+addop);
					op(wtok.rm);
					outaddress(&wtok);
					outseg(&itok,2);	//fistp val
					op(0xdf);
					op(itok.rm+0x38);
					outaddress(&itok);
					outseg(&wtok,2);	//fstp val
					op(0xd9+addop);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					fwait3();
					break;
				case tk_dwordvar:
					CheckInitBP();
					op66(r32);	//push 0L
					outword(0x6a);
					if(ESPloc&&am32&&itok.segm==SS)itok.number+=4;
					addESP+=4;
					CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
					op66(r32);	//push var
					outseg(&itok,2);
					op(0xFF);
					op(itok.rm+0x30);
					outaddress(&itok);
					if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=4;
					addESP+=4;
					fildq_stack();
					CheckAllMassiv(wbuf,razr,&wstr,&wtok);
					outseg(&wtok,2);	//fld val
					op(0xd9+addop);
					op(wtok.rm);
					outaddress(&wtok);
					if(optimizespeed||am32==FALSE){
						outword(0xC483);
						op(8);
					}
					else{
						op(0x58);	// pop EAX
						op(0x58);	// pop EAX
					}
					addESP-=8;
					outseg(&itok,2);//fistp var
					op(0xDB);
					op(itok.rm+0x18);
					outaddress(&itok);
					outseg(&wtok,2);	//fstp val
					op(0xd9+addop);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					fwait3();
					RestoreBP();
					break;
				case tk_floatvar:
					if(rettype==tk_double){
						CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
						outseg(&itok,2);	//fld val
						op(0xd9);
						op(itok.rm);
						outaddress(&itok);
						CheckAllMassiv(wbuf,8,&wstr,&wtok);
						outseg(&wtok,2);	//fstp val
						op(0xdd);
						op(wtok.rm);
						outaddress(&wtok);
						outseg(&itok,2);	//fstp val
						op(0xd9);
						op(itok.rm+0x18);
						outaddress(&itok);
						outseg(&wtok,2);	//fstp val
						op(0xdd);
						op(wtok.rm+0x18);
						outaddress(&wtok);
						fwait3();
					}
					else{
						getinto_e_ax(0,tk_dwordvar,&wtok,wbuf,&wstr,r32);
						CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
						op66(r32);
						outseg(&itok,2);// XCHG EAX,[dword]
						op(0x87); op(itok.rm);
						outaddress(&itok);
						goto getfromeax;
					}
					break;
				case tk_doublevar:
					if(rettype==tk_float){
						CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
						outseg(&itok,2);	//fldq val
						op(0xdd);
						op(itok.rm);
						outaddress(&itok);
						CheckAllMassiv(wbuf,8,&wstr,&wtok);
						outseg(&wtok,2);	//fld val
						op(0xd9);
						op(wtok.rm);
						outaddress(&wtok);
						outseg(&itok,2);	//fstp val
						op(0xdd);
						op(itok.rm+0x18);
						outaddress(&itok);
						outseg(&wtok,2);	//fstp val
						op(0xd9);
						op(wtok.rm+0x18);
						outaddress(&wtok);
						fwait3();
					}
					else{
						getinto_e_ax(0,tk_dwordvar,&wtok,wbuf,&wstr,r32);
						CheckAllMassiv(rbuf,4,&strinf,&itok,regdi==FALSE?idxregs[2]:idxregs[1],idxregs[3]);
						op66(r32);
						outseg(&itok,2);// XCHG EAX,[dword]
						op(0x87); op(itok.rm);
						outaddress(&itok);
						goto getfromeax;
					}
					break;
				case tk_fpust:
					if(itok.number>6)fpustdestroed();
					CheckAllMassiv(wbuf,razr,&wstr,&wtok);
					outseg(&wtok,2);	//fld val
					op(0xd9+addop);
					op(wtok.rm);
					outaddress(&wtok);
					op(0xD9);
					op(0xC8+itok.number+1);
					outseg(&wtok,2);	//fstp val
					op(0xd9+addop);
					op(wtok.rm+0x18);
					outaddress(&wtok);
					break;
				default: swaperror(); break;
			}
			break;
		default: operatorexpected(); break;
	}
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	if(cpu<3)cpu=3;
	return retrez;
}

void fildq2_stack(int size)
{
	op(0xdf);
	if(am32)outword(0x242c);	//fildq ssdword[bp-8]/ssdword[esp]
	else{
		if(short_ok(size,FALSE)==0){
			op(0xAE);
			outword(-size);
		}
		else{
			op(0x6E);
			op(-size);
		}
	}
}

void fildq_stack()
{
	fildq2_stack(localsize+8);
}

void fistp2_stack(int size)
{
	if(am32)outword(0x241c);	//fistp ssdword[ebp+2]/ssdword[esp]
	else{
		if(short_ok(size,FALSE)==0){
			op(0x9E);
			outword(-size);
		}
		else{
			op(0x5E);
			op(-size);
		}
	}
}

void fistp_stack(int addop)
{
	op(0xDB+addop);
	fistp2_stack(localsize+4+addop);
}

void fld_stack(int size)
{
	if(am32)outword(0x2404);	//fld ssdword[ebp+2]/ssdword[esp]
	else{
		if(short_ok(size,FALSE)==0){
			op(0x86);
			outword(-size);
		}
		else{
			op(0x46);
			op(-size);
		}
	}
}

void FloatToNumer(int addop)
//конвертация float в число
{
	CheckInitBP();
	op(0xD9+addop);
	fld_stack(4+localsize+addop);
	fistp_stack();
	RestoreBP();
	fwait3();
	op66(r32);
	op(0x58);	//pop reg
	if(cpu<3)cpu=3;
}

void Float2reg32(int reg,int addop,int reg1,int reg2)
{
	CheckInitBP();
	op66(r32);
	op(0x50);  //push AX
	if(ESPloc&&am32&&itok.segm==SS)itok.number+=4;
	addESP+=4;
	CheckAllMassiv(bufrm,4+addop,&strinf,&itok,reg1,reg2);
	outseg(&itok,2);	//fld floatvar
	op(0xd9+addop);
	op(itok.rm);
	outaddress(&itok);
	fistp_stack();
	fwait3();
	op66(r32);
	op(0x58+reg);	//pop reg
	addESP-=4;
	if(cpu<3)cpu=3;
	RestoreBP();
}

void  byteinstack(int *numstak,int *nums)
{
	CheckInitBP();
	CheckAllMassiv(bufrm,1,&strinf);
	op66(r16);
	if(tok==tk_bytevar&&optimizespeed&&chip>3&&chip<7){
		outword(0xC031);	//xor ax,ax
		outseg(&itok,2);
		op(0x8A);
	}
	else{
		outseg(&itok,3);
		op(0xf);
		if(tok==tk_bytevar)op(0xb6);
		else op(0xbe);
	}
	op(itok.rm);
	outaddress(&itok);
	op66(r16);
	outword(0xDF50);	//push ax
	*numstak+=2;
	addESP+=2;
	fld_stack(*nums+*numstak);
}

void  reginstack(int *numstak,int *nums)
{
	CheckInitBP();
	op66(tok==tk_reg32?r32:r16);
	if(tok==tk_beg){
		if(optimizespeed&&chip>3&&chip<7&&itok.number!=AL&&itok.number!=AH){
			xorAXAX();
			op(0x88);
			op(0xC0+itok.number*8);	//mov al,beg
		}
		else if(itok.number==AL)xorAHAH();
		else{
			outword(0xB60F);	/* MOVZX AX,beg */
			op(0xC0+itok.number);
		}
		itok.number=0;
	}
	else outword(0x6a);  //push 0
	op66(tok==tk_reg32?r32:r16);
	op(0x50+itok.number);	//push AX
	if(tok==tk_reg){
		op(0xDB);
		*numstak+=4;
		addESP+=4;
		fld_stack(*nums+*numstak);
	}
	else if(tok==tk_beg){
		op(0xdf);
		*numstak+=2;
		addESP+=2;
		fld_stack(*nums+*numstak);
	}
	else{
		*numstak+=8;
		addESP+=8;
		fildq2_stack(*nums+*numstak);
	}
}

void wordinstack(int *numstak,int *nums)
{
	CheckInitBP();
	op66(tok==tk_wordvar?r16:r32);
	outword(0x6a);  //push 0
	if(ESPloc&&am32&&itok.segm==SS)itok.number+=4;
	CheckAllMassiv(bufrm,tok==tk_wordvar?2:4,&strinf);
	op66(tok==tk_wordvar?r16:r32);
	outseg(&itok,2); //push var
	op(0xFF);
	op(itok.rm+0x30);
	outaddress(&itok);
	if(tok==tk_wordvar){
		op(0xDB);
		addESP+=4;
		*numstak+=4;
		fld_stack(*nums+*numstak);
	}
	else{
		addESP+=8;
		*numstak+=8;
		fildq2_stack(*nums+*numstak);
	}
}

void intinstack(int addop)
{
	CheckAllMassiv(bufrm,tok==tk_intvar?2:4+addop,&strinf);
	outseg(&itok,2);	//fild intvar
	if(tok==tk_intvar||tok==tk_qwordvar)op(0xDF);
	else if(tok==tk_floatvar||tok==tk_doublevar)op(0xd9+addop);
	else op(0xDB);
	op(itok.rm+(tok==tk_qwordvar?0x28:0));
	outaddress(&itok);
}

int doeaxfloatmath(int itreturn,int reg,int addop)
{
int vop,negflag=0,next,numstak=0;
static int nums=0;
int i=0;
long long lnumber;
int ols;
	ols=localsize;
	nums+=localsize;
	next=1;
	if(tok==tk_minus){
		if(CheckMinusNum()==FALSE){
			negflag=1;
			getoperand(am32==TRUE?EAX:BX);
		}
	}
	if(tok==tk_number){
//		printf("rm=%d %e %e %08X\n",itok.rm,itok.fnumber,itok.dnumber,itok.number);
		if(addop==0/*itok.rm==tk_float*/){
			itok.number=doconstfloatmath();
			itok.rm=tk_float;
		}
		else{
			itok.lnumber=doconstdoublemath();
			itok.rm=tk_double;
		}
		next=0;
		if(itok.type==tp_stopper){
//			if(itok.rm==tk_float&&addop==4)itok.dnumber=itok.fnumber;
//			if(itok.rm==tk_double&&addop==0)itok.fnumber=itok.dnumber;
			if(itreturn==tk_stackstart){
				if(itok.rm==tk_double){
					lnumber=itok.lnumber;
					itok.lnumber=lnumber>>32;
				}
				for(i=0;i<2;i++){
					op66(r32);
					if(short_ok(itok.number,TRUE)){	//PUSH number
						op(0x6A);
						op(itok.number);
					}
					else{
						op(0x68);
						outdword(itok.number);
					}
					if(itok.rm!=tk_double)break;
					if(i==1)break;
					itok.number=lnumber;
				}
				return itreturn;
			}
			if(itreturn==tk_reg32){
				MovRegNum(r32,0,itok.number,reg);
				return itreturn;
			}
			if(itreturn==tk_reg64){
				MovRegNum(r32,0,itok.number,reg&255);
				MovRegNum(r32,0,itok.lnumber>>=32,reg/256);
				return itreturn;
			}
		}
	}
	if(itreturn==tk_stackstart&&(!am32)){
		op66(r32);
		op(0x50);	//push eax
		if(addop){
			op66(r32);
			op(0x50);	//push eax
		}
		op(0x55);       //push bp
		outword(0xe589);//mov bp,sp
		if(initBP>1)initBP++;
		localsize=-6-addop;
	}
	if(next==0)goto casenumber;
	switch(tok){
		case tk_number:
casenumber:
//		printf("rm=%d %e %e %08X\n",itok.rm,itok.fnumber,itok.dnumber,itok.number);
			if((itok.rm==tk_float&&itok.fnumber==1.0)||
					(itok.rm==tk_double&&itok.dnumber==1.0)||
					(itok.rm!=tk_float&&itok.rm!=tk_double&&itok.lnumber==1)){
				outword(0xE8D9);	//FLD1
				break;
			}
			if((itok.rm==tk_float&&itok.fnumber==0.0)||
					(itok.rm==tk_double&&itok.dnumber==0.0)||itok.lnumber==0){
				outword(0xEED9);	//FLDZ
				break;
			}
			op(0xD9+(itok.rm==tk_float?0:4));
			op((am32==FALSE?0x06:0x05));	//fld
			AddFloatConst(itok.lnumber,itok.rm);
			outword(0);
			if(am32)outword(0);
			break;
		case tk_at:
			getoperand(am32==TRUE?EAX:BX);
			macros(tk_fpust);
			break;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_declare:
		case tk_undefproc:
int onums;
			onums=nums;
			nums=-(int)localsize;
			vop=procdo(tk_fpust);	//возвр из процедур
			nums=onums;
			if(tok2==tk_semicolon&&vop!=tk_fpust&&vop!=tk_double){
				nexttok();
				return tk_assign;
			}
			break;
		case tk_qwordvar:
		case tk_doublevar:
			i=4;
		case tk_floatvar:
		case tk_longvar:
		case tk_intvar:
			intinstack(i);
			break;
		case tk_dwordvar:
		case tk_wordvar:
			wordinstack(&numstak,&nums);
			break;
		case tk_charvar:
		case tk_bytevar:
			byteinstack(&numstak,&nums);
			break;
		case tk_beg:
		case tk_reg32:
		case tk_reg:
			reginstack(&numstak,&nums);
			break;
		case tk_fpust:
			if(itok.number){
				op(0xD9);
				op(0xC8+itok.number);
			}
			break;
		default: valueexpected();	break;
	}
	if(negflag){
		outword(0xe0d9);	//fchs
		negflag=0;
	}
	if(next==1)nexttok();
	while(itok.type!=tp_stopper&&tok!=tk_eof&&itok.type!=tp_compare){
		next=1;
		vop=0;
		i=0;
		switch(tok){
			case tk_multminus: negflag=1; goto mult;
			case tk_divminus: negflag=1;
			case tk_div: vop+=0x10;
			case tk_minus: vop+=0x20;
mult:
			case tk_mult: vop+=8;
			case tk_plus:
		 		getoperand();
				switch(tok){
					case tk_number:
						if(itok.rm!=tk_float&&itok.rm!=tk_double){
							itok.dnumber=itok.lnumber;
							itok.rm=tk_double;
						}
						if((itok.rm==tk_float&&itok.fnumber==1.0)||
								(itok.rm==tk_double&&itok.dnumber==1.0)){
num1:
							if(vop==0||vop==0x28){	// + -
								outword(0xE8D9);	//FLD1
								op(0xDE);
								op(0xC1+vop);
							}
							break;
						}
						op(0xd8+(itok.rm==tk_float?0:4));

						if(vop==0x38){	// div 22.12.05 22:10
							vop=8;	//mult
							if(itok.rm==tk_float)itok.fnumber=1/itok.fnumber;
							else itok.dnumber=1/itok.dnumber;
						}
						if(/*vop==0x38||*/vop==0x28)vop-=8;

						op((am32==FALSE?0x06:0x05)+vop);
						AddFloatConst(itok.lnumber,itok.rm);
						outword(0);
						if(am32)outword(0);
						break;
					case tk_doublevar:
						i=4;
					case tk_floatvar:
						if(vop==0x38||vop==0x28)vop-=8;
						CheckAllMassiv(bufrm,4+i,&strinf);
						outseg(&itok,2);	//fsubr var
						op(0xd8+i);
						op(itok.rm+vop);
						outaddress(&itok);
						break;
					case tk_longvar:
					case tk_intvar:
						CheckAllMassiv(bufrm,4,&strinf);
						outseg(&itok,2);	//fsubr var
						op(tok==tk_intvar?0xDE:0xDA);
						if(vop==0x38||vop==0x28)vop-=8;
						op(itok.rm+vop);
						outaddress(&itok);
						break;
					case tk_qwordvar:
						intinstack(4);
						op(0xde);
						op(0xC1+vop);//fsubpr st1
						break;
					case tk_dwordvar:
					case tk_wordvar:
						wordinstack(&numstak,&nums);
						op(0xde);
						op(0xC1+vop);//fsubpr st1
						break;
					case tk_beg:
					case tk_reg32:
					case tk_reg:
						reginstack(&numstak,&nums);
						op(0xde);
						op(0xC1+vop);//fsubpr st1
						break;
					case tk_charvar:
					case tk_bytevar:
						byteinstack(&numstak,&nums);
						op(0xde);
						op(0xC1+vop);//fsubpr st1
						break;
					case tk_fpust:
						if(vop==0x38||vop==0x28)vop-=8;
						op(0xd8);
						op(0xC0+vop+itok.number);//fsubpr st
						break;
					default: valueexpected(); break;
				}
				break;
			default: illegalfloat(); break;
		}
		if(negflag){
			outword(0xe0d9);	//fchs
			negflag=0;
		}
		if(next)nexttok();
	}
	if(tok==tk_eof)unexpectedeof();
	if(itreturn==tk_stackstart){
		if(!am32){
			op(0xD9+addop);
			outword(0x025e);	//fstp ssdword[bp+2]
			numstak-=2;
		}
		else{
			if(numstak<4){
				op66(r32);
				op(0x50);	//push eax
				addESP+=4;
			}
			else numstak-=4;
			if(addop){
				if(numstak<4){
					op66(r32);
					op(0x50);	//push eax
					addESP+=4;
				}
				else numstak-=4;
			}
			op(0xD9+addop);
			if(numstak==0)outword(0x241C);	//fstp ssdword[esp]
			else{
				outword(0x245C);	//fstp ssdword[esp+numstak]
				op(numstak);
			}
		}
		fwait3();
	}
	else if(itreturn==tk_reg32||itreturn==tk_reg64){
		i=4;
		if(itreturn==tk_reg64)i=8;
		if(numstak<i){
			op66(r32);
			op(0x50);	//push eax
			if(itreturn==tk_reg64){
				op66(r32);
				op(0x50);	//push eax
			}
			numstak+=i;
			addESP+=i;
		}
		op(itreturn==tk_reg32?0xDB:0xDF);
//		op(itreturn==tk_reg32?0xD9:0xDB);
		fistp2_stack(localsize+numstak);
		fwait3();
		if(itreturn==tk_reg64){
			op66(r32);
			op(0x58);	//pop EAX
			op66(r32);
			op(0x58+EDX);	//pop EDX
		}
		else{
			op66(r32);
			op(0x58+reg);	//pop reg
		}
		numstak-=i;
		addESP-=i;
	}
	if(localsize!=(unsigned int)ols){
		localsize=ols;
		Leave();
		initBP--;
	}
	else if(numstak){
		if(numstak<128){
			outword(0xC483);
			op(numstak);
		}
		else{
			outword(0xC481);
			outword(numstak);
		}
		addESP-=numstak;
	}
	RestoreBP();
	nums-=localsize;
	return itreturn;
}

void float2stack(int num)
{
	outword(0xC0DD);
	if(tok!=tk_fpust)outword(0xf7d9);	//fincstp
//	if(num>6)fpustdestroed();
//	outword(0xf6d9);	//fdecstp
	doeaxfloatmath(tk_fpust);
	if(num){
		op(0xD9);
		op(0xC8+num);
	}
}

void dofloatstack(int num)
{
int vop=0;
	nexttok();
	switch(tok){
		case tk_assign:	//=
			getoperand(am32==TRUE?EAX:BX);
			float2stack(num);
			break;
		case tk_divequals: vop+=0x10;
		case tk_minusequals: vop+=0x20;
		case tk_multequals: vop+=8;
		case tk_plusequals:
			getoperand(am32==TRUE?EAX:BX);
			float2stack(0);
//			doeaxfloatmath(tk_fpust);
			op(0xdc);
			op(0xC0+vop+num);//fsubpr st
			break;
		case tk_swap:
			getoperand(am32==TRUE?EAX:BX);
			switch(tok){
				case tk_fpust:
					op(0xD9);
					op(0xC8+num);
					op(0xD9);
					op(0xC8+itok.number);
					op(0xD9);
					op(0xC8+num);
					break;
				case tk_doublevar:
					vop=4;
				case tk_floatvar:
					CheckAllMassiv(bufrm,4,&strinf);
					outseg(&itok,2);	//fld val
					op(0xd9+vop);
					op(itok.rm);
					outaddress(&itok);
					op(0xD9);
					op(0xC8+num+1);
					outseg(&itok,2);	//fstp val
					op(0xd9+vop);
					op(itok.rm+0x18);
					outaddress(&itok);
					break;
				default:
					swaperror();
					break;
			}
			nexttok();
			break;
		default:
			illegalfloat(); break;
	}
}

void RestoreBP()
{
	if(am32==0&&initBP==2){
		Leave();
		initBP=0;
	}
}

void CheckInitBP()
{
	if(am32==0&&initBP==0){
		op(0x55);       //push bp
		outword(0xe589);//mov bp,sp
		initBP=2;
	}
}

void AddReloc(int segm)
{
	if(FixUp!=FALSE){
		CheckPosts();
		if(segm==DS){
			(postbuf+posts)->type=(unsigned short)(am32==FALSE?FIX_VAR:FIX_VAR32);
			(postbuf+posts)->loc=outptrdata;
		}
		else{
			(postbuf+posts)->type=(unsigned short)(am32==FALSE?FIX_CODE:FIX_CODE32);
			(postbuf+posts)->loc=outptr;
		}
		posts++;
	}
	postnumflag&=(~f_reloc);
}

void fwait3()
{
	if(chip<4&&tok2!=tk_floatvar&&tok2!=tk_doublevar&&tok2!=tk_float&&tok2!=tk_double)op(0x9B);
}

void AddFloatConst(long long fnumber,int type)
{
unsigned int i;
unsigned int ofs;
	for(i=0;i<numfloatconst;i++){
		if(type==(floatnum+i)->type){
			ofs=(floatnum+i)->ofs;
			if(type==tk_float){
				if(*(long *)&fnumber==(floatnum+i)->num[0])goto endp;
			}
			else if(*(double *)&fnumber==(floatnum+i)->dnum)goto endp;
		}
	}
	if(numfloatconst==0){
		floatnum=(LISTFLOAT *)MALLOC(sizeof(LISTFLOAT)*STEPFLOATCONST);
		maxnumfloatconst=STEPFLOATCONST;
//		memset(floatnum,0,sizeof(LISTFLOAT)*STEPFLOATCONST);
	}
	else if((numfloatconst+1)==maxnumfloatconst){
		floatnum=(LISTFLOAT *)REALLOC(floatnum,(maxnumfloatconst+STEPFLOATCONST)*sizeof(LISTFLOAT));
		maxnumfloatconst+=STEPFLOATCONST;
	}
	numfloatconst++;
	(floatnum+i)->type=type;
	ofs=(floatnum+i)->ofs=ofsfloatlist;
	if(type==tk_float)(floatnum+i)->num[0]=*(unsigned long *)&fnumber;
	else (floatnum+i)->dnum=*(double *)&fnumber;
	ofsfloatlist+=(type==tk_float?4:8);
endp:
	CheckPosts();
	(postbuf+posts)->type=POST_FLOATNUM;
	(postbuf+posts)->loc=outptrdata;
	(postbuf+posts)->num=ofs;
	posts++;
}

void setwordext(long *id)
{
	CheckPosts();
	(postbuf+posts)->type=EXT_VAR;
	(postbuf+posts)->loc=outptr;
	(postbuf+posts)->num=*id&0xFFFF;	//id номер внешней переменной
	*id>>=16;	//ее значение
	posts++;
}

void setwordpost(ITOK *stok)						/* for post word num setting */
{
	CheckPosts();
	if(stok->post==USED_DIN_VAR){
		(postbuf+posts)->type=(unsigned short)(am32==0?DIN_VAR:DIN_VAR32);
//			printf("Add tok=%d %s\n",stok->rec->rectok,stok->rec->recid);
//		printf("rec=%08X\n",stok->rec);
		if(stok->rec->rectok==tk_structvar&&stok->rec->recsib==tp_gvar){
			(postbuf+posts)->num=(int)stok->rec;//02.09.05 17:11 ->right;
		}
		else (postbuf+posts)->num=(int)stok->rec;
	}
	else if(stok->post>=CODE_SIZE&&stok->post<=STACK_SIZE32)(postbuf+posts)->type=stok->post;
	else (postbuf+posts)->type=(unsigned short)(am32==FALSE?POST_VAR:POST_VAR32);
	(postbuf+posts)->loc=outptr;
	posts++;
}

void MovRegNum(int razr,int relocf,unsigned long number,int reg)
{
unsigned long num;
int nreg;
	op66(razr);
	if(relocf==0){
		if(number!=0&&GetRegNumber(reg,&num,razr)==reg){
			ConstToReg(number,reg,razr);
			if(num==number)return;
			else if(num<number){
				num=number-num;
				if(num==1){
					op(0x40+reg);	//inc reg
					return;
				}
				if(num==2){
					op(0x40+reg);	//inc reg
					op66(razr);
					op(0x40+reg);	//inc reg
					return;
				}
				if(razr==r32&&short_ok(num,TRUE)){
					op(0x83);
					op(0xC0+reg);
					op(num);
					return;
				}
			}
			else{
				num=num-number;
				if(num==1){
					op(0x48+reg);	//dec reg
					return;
				}
				if(num==2){
					op(0x48+reg);	//dec reg
					op66(razr);
					op(0x48+reg);	//dec reg
					return;
				}
				if(razr==r32&&short_ok(num,TRUE)){
					op(0x83);
					op(0xE8+reg);
					op(num);
					return;
				}
			}
		}
		if((nreg=GetNumberR(reg,&num,razr,number))!=NOINREG){
			ConstToReg(number,reg,razr);
			if(num==number){
				op(0x89);
				op(128+64+nreg*8+reg);
				return;
			}
			else if((number-num)==1){
				op(0x89);
				op(128+64+nreg*8+reg);
				op66(razr);
				op(0x40+reg);	//inc reg
				return;
			}
			else{
				if((num-number)==1){
					op(0x89);
					op(128+64+nreg*8+reg);
					op66(razr);
					op(0x48+reg);	//dec reg
					return;
				}
			}
		}
		ConstToReg(number,reg,razr);
		if(number==0){
			op(0x31); op(0xC0+reg*9);
		}  // XOR reg,reg
		else if((long)number==-1&&razr==r32){
			op(0x83);	//or reg,-1
			op(0xC8+reg);
			op(0xFF);
		}
		else if(number==1&&razr==r32){
			op(0x31); op(0xC0+reg*9);	//xor reg,reg
			op66(razr);
			op(0x40+reg);	//inc reg
		}
		else if(regoverstack&&razr==r32&&short_ok(number,TRUE)){
			op(0x6A);
			op(number);	//push short number
			op66(razr);
		  op(0x58+reg);
		}
		else goto stdmov;
	}
	else{
stdmov:
		op(0xB8+reg);	// MOV reg,#
		if(relocf)AddReloc();
		razr==r16?outword(number):outdword(number);
		ClearReg(reg);
	}
}

void NegReg(int razr,int reg)
{
	op66(razr);
	if(optimizespeed&&(chip==5||chip==6)){
		op(0x83);	//and reg,-1
		op(0xF0+reg);
		op(0xFF);
		op66(razr);
		op(0x40+reg);	//inc reg
	}
	else{
		op(0xF7);
		op(0xD8+reg);  // NEG reg
	}
	ClearReg(reg);
}

int RshiftReg(int razr,int reg,int sign)
{
char *ofsstr=NULL;
	if(tok==tk_number){
		if((unsigned int)itok.number==1){
			op66(razr);
			op(0xD1); op(0xE8+reg+sign);  // SHR reg,1
		}
		else if((unsigned int)itok.number!=0){
			if(chip<2&&razr==r16){
				if(reg==ECX)return FALSE;
				goto rrshift;
			}
			else{
				op66(razr);
				op(0xc1);
				op(0xe8+reg+sign); // SHR reg,imm8
				op((unsigned int)itok.number);
				if(cpu<2)cpu=2;
			}
		}
		nexttok();
	}
	else if(reg==ECX)return FALSE;
	else if((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==CL){
		nexttok();
		goto rshift;
	}
	else{
rrshift:
		getintobeg(CL,&ofsstr);
		warningreg(begs[1]);
rshift:
		op66(razr);
		op(0xD3);
		op(0xE8+reg+sign);	// SHL xXX,CL
	}
	return TRUE;
}

int CheckMinusNum()
{
	if(tok==tk_minus&&tok2==tk_number){
		nexttok();
		if(itok.rm==tk_float)itok.number|=0x80000000;
		else if(itok.rm==tk_double)itok.lnumber|=0x8000000000000000LL;
		else itok.lnumber=-itok.lnumber;
		return TRUE;
	}
	return FALSE;
}

int MulReg(int reg,int razr)
{
int next=1,i=0;
int ii=0;
char *ofsstr=NULL;
	switch(tok){
		case tk_number:
			RegMulNum(reg,itok.number,razr,0,&i,itok.flag);
			break;
		case tk_postnumber:
		case tk_undefofs:
			if(chip<2&&razr==r16)regmathoperror();
			op66(razr);
			op(0x69);
			op(0xc0+reg*9);
			if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
			else AddUndefOff(0,itok.name);
			razr==r16?outword(itok.number):outdword(itok.number);
			break;
		case tk_apioffset:
			if(chip<2&&razr==r16)regmathoperror();
			op66(razr);
			op(0x69);
			op(0xc0+reg*9);
			AddApiToPost(itok.number);
			break;
		case tk_doublevar:
			ii=4;
		case tk_floatvar:
			if(reg!=EDX)i=EDX;
			Float2reg32(i,ii);
		 	op66(razr);
			outword(0xAF0F);
			op(0xC0+reg*8+i); // IMUL reg,EDX/EAX
			warningreg(regs[1][i]);
			break;
		case tk_qwordvar:
			i=4;
		case tk_longvar:
		case tk_dwordvar:
			i+=4;
			goto mulword;
		case tk_intvar:
		case tk_wordvar:
			if(chip<2)regmathoperror();
			if(razr==r32)goto defmul1;
			i=2;
mulword:
			CheckAllMassiv(bufrm,i,&strinf);
			op66(razr);
			outseg(&itok,3);
			outword(0xAF0F);
			op(reg*8+itok.rm);
			outaddress(&itok);
			break;
		case tk_bits:
			int vops,reg2s;
			i=itok.bit.siz+itok.bit.ofs;
			if(i<=64)vops=r64;
			if(i<=32)vops=r32;
			if(i<=16)vops=r16;
			if(vops<razr)vops=razr;
			reg2s=CX;
			if(reg==CX)reg2s=DI;
			bits2reg(reg2s,vops);
			if(vops==r64)vops=r32;
			warningreg(regs[vops/2-1][reg2s]);
			itok.number=reg2s;
			goto defreg32;
		case tk_reg:
			if(chip<2)regmathoperror();
			if(razr==r32){
				op66(r32);
				outword(0xB70F);
				if(itok.number==reg){
					op(0xC0+reg);
					itok.number=EAX;
				}
				else op(0xC0+itok.number*9);
				warningreg(regs[razr/2-1][itok.number]);
			}
		case tk_reg32:
defreg32:
			op66(razr);
			outword(0xAF0F);
			op(0xC0+reg*8+(unsigned int)itok.number);
			break;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			procdo(razr==r16?tk_word:tk_dword);
			i=EAX;
			goto defmul;
		case tk_seg:
		case tk_charvar:
		case tk_beg:
		case tk_bytevar:
		case tk_rmnumber:
defmul1:
			i=EDX;
			if(reg==EDX)i=ECX;
			getintoreg_32(i,razr,0,&ofsstr,FALSE);
defmul:
		 	op66(razr);
			outword(0xAF0F);
			op(0xC0+reg*8+i);
			warningreg(regs[razr/2-1][2]);
			if(i!=EDX)warningreg(regs[razr/2-1][i]);
			if(i!=EAX)next=0;
			break;
		default: regmathoperror(); break;
	}
	return next;
}

void DivMod(int vop,int sign,int razr,int expand)
{
int i,next;
char *ofsstr=NULL;
	if(tok==tk_bits){
		i=itok.bit.siz+itok.bit.ofs;
		if(i<=64)next=r64;
		if(i<=32)next=r32;
		if(i<=16)next=r16;
		bits2reg(CX,(razr<next?next:razr));
		itok.number=CX;
		if(next==r64)next=r32;
		ClearReg(CX);
		warningreg(regs[next/2-1][ECX]);
		tok=(razr==r16?tk_reg:tk_reg32);
	}
	i=0;
	next=1;
	if(tok==tk_number){
		if(razr==r16)itok.number&=0xffff;
		if(vop){	//%
			if((itok.flag&f_reloc)!=0)goto defal;
			if(itok.number==0)DevideZero();
			if(caselong(itok.number)!=NUMNUM){
				itok.number--;
				if(itok.number==0)ZeroReg(EAX,razr);
 				else if(short_ok(itok.number,razr/2-1)){
					op66(razr);
					outword(0xE083);
					op(itok.number);
				}
				else{
					op66(razr);
					op(0x25); // AND EAX,number-1
					razr==r16?outword((unsigned int)itok.number):outdword(itok.number);
				}
				setzeroflag=TRUE;
			}
			else{
defal:
				if(!expand)ClearDX(razr,sign);
				DivNum2(itok.number,razr,sign);
				setzeroflag=FALSE;
			}
		}
		else{	//деление
			if((itok.flag&f_reloc)!=0)goto divin;
			switch(itok.number){
				case 0:
					DevideZero();
					break;
				case 1: break;
				case 2:
					if(expand==TRUE){
						op66(razr);
						if((chip>2||razr==r32)&&chip!=5&&chip!=6){
							outdword(0x01d0ac0f);	//shrd ax,dx,1
							setzeroflag=TRUE;
						}
						else{
							outword(0xead1);	//shr dx,1 rcr ax,1
							op66(razr);
							outword(0xd8d1);
							setzeroflag=FALSE;
						}
						ClearReg(AX);
						ClearReg(DX);
						break;
					}
					op66(razr);
				  if(sign)outword(0xF8D1);// SAR AX,1
					else outword(0xE8D1);	// SHR AX,1
					setzeroflag=TRUE;
					ClearReg(AX);
					break;
				default:
					vop=caselong(itok.number);
					if(vop!=NUMNUM){
						if(expand==TRUE){
							if(chip>2||razr==r32){
								op66(razr);
								op(0x0f);
								outword(0xd0ac);	//shrd ax,dx,vop
								op(vop);
								setzeroflag=TRUE;
								ClearReg(AX);
								ClearReg(DX);
							}
							else{
								if(optimizespeed==FALSE)goto divin;
								op(0xB1); op(vop); /* MOV CL,num */
								if(sign)outword(0xFad1); // SAR AX,1
								else outword(0xEad1); // SHR AX,1
								outdword(0xfae2d8d1);  //rcr ax,1  LOOP -6
								warningreg(begs[1]);
								setzeroflag=FALSE;
								ClearReg(AX);
								ConstToReg(vop,CL,r8);
							}
						}
						else{
							if(chip<2&&razr==r16){
								op(0xB1); op(vop); /* MOV CL,num */
								if(sign)outword(0xF8D3); // SAR AX,CL
								else outword(0xE8D3); // SHR AX,CL
								warningreg(begs[1]);
								ClearReg(AX);
								ConstToReg(vop,CL,r8);
							}
							else{
								op66(razr);
								if(sign)outword(0xF8C1); // SAR AX,num
								else outword(0xE8C1); // SHR AX,num
					 			op(vop);
								ClearReg(AX);
							}
							setzeroflag=TRUE;
						}
					}
					else{
						if(expand==FALSE)DivNum(itok.number,razr,sign);
						else{
divin:

							if(!expand)ClearDX(razr,sign);
							DivNum2(itok.number,razr,sign);
						}
					}
			}
		}
	}
	else{
		if(tok==tk_doublevar){
			i=4;
			tok=tk_floatvar;
		}
		if(tok==tk_floatvar){
			Float2reg32(ECX,i);
			warningreg(regs[razr/2-1][1]);
			itok.number=ECX;
			tok=(razr==r16?tk_reg:tk_reg32);
			sign=1;
		}
		switch(tok){
			case tk_qwordvar:
				i=4;
			case tk_longvar:
			case tk_dwordvar:
				i+=2;
			case tk_intvar:
			case tk_wordvar:
				if(razr==r32&&(tok==tk_intvar||tok==tk_wordvar))goto defdiv;
				i+=2;
				CheckAllMassiv(bufrm,i,&strinf);
				if(expand==FALSE)ClearDX(razr,sign);
		 		op66(razr);
				outseg(&itok,2);
				op(0xF7);
				if(sign)op(0x38+itok.rm); /* IDIV word ptr [#] */
				else op(0x30+itok.rm); /* DIV word ptr [#] */
				outaddress(&itok);
				ClearReg(AX);
				break;
			case tk_reg:
				if(razr==r32){
					i=itok.number;
					getintoreg_32(i,r32,0,&ofsstr,FALSE);
					if(expand==FALSE)ClearDX(razr,sign);
					op66(r32);
					op(0xF7);
					if(sign)op(0xF8+i);  /* IDIV ECX */
					else op(0xF0+i); /* DIV ECX */
					next=0;
					warningreg(regs[1][2]);
				ClearReg(AX);
				ClearReg(CX);
					break;
				}
			case tk_reg32:
				if(expand==FALSE)ClearDX(razr,sign);
		 		op66(razr);
				op(0xF7);
				if(sign)op(0xF8+(unsigned int)itok.number);
				else op(0xF0+(unsigned int)itok.number);
				ClearReg(AX);
				break;
			case tk_ID:
			case tk_id:
			case tk_proc:
			case tk_apiproc:
			case tk_undefproc:
			case tk_declare:
				op66(razr);
				op(0x50);	//push AX
				addESP+=razr==r16?2:4;
unsigned char oaddstack;
				oaddstack=addstack;
				addstack=FALSE;
				procdo(razr==r16?(sign==0?tk_word:tk_int):(sign==0?tk_dword:tk_long));
				addstack=oaddstack;
				addESP-=razr==r16?2:4;
				op66(razr);
				op(0x90+ECX);	//xchg AX,CX
				op66(razr);
				op(0x58);	//pop AX
				if(expand==FALSE)ClearDX(razr,sign);
				op66(razr);
				op(0xF7);
				if(sign)op(0xF8+ECX);  /* IDIV ECX */
				else op(0xF0+ECX); /* DIV ECX */
				warningreg(regs[razr/2-1][ECX]);
				break;
			case tk_undefofs:
			case tk_seg:
			case tk_charvar:
			case tk_beg:
			case tk_bytevar:
			case tk_rmnumber:
			case tk_postnumber:
			case tk_apioffset:
defdiv:
				getintoreg_32(CX,razr,0,&ofsstr,FALSE);
				if(expand==FALSE)ClearDX(razr,sign);
		 		op66(razr);
				if(sign)outword(0xF9F7);  /* IDIV CX */
				else outword(0xF1F7); /* DIV CX */
				next=0;
				warningreg(regs[razr/2-1][ECX]);
				ClearReg(CX);
				ClearReg(AX);
				break;
			default: valueexpected();	break;
		}
		setzeroflag=FALSE;
/*		if(vop){
	 		op66(razr);
			if(optimizespeed)outword(0xC28B);	//mov ax,dx
			else op(0x92);	//xchg ax,dx
		}*/
	}
	if(next)nexttok();
}

void DivNum(unsigned long num,int razr,int sign)
{
/*int i;
unsigned long num2;
	if(num<65536&&optimizespeed&&(itok.flag&f_reloc)==0&&sign==0){	//for signed needed new algoritm
		if(razr==r16&&chip>2){
			num2=65536/(unsigned int)num+1;
			if((65535/num2)!=num)goto stddiv;
			op66(r32);
			op(0x25);
			outdword(0xffff);	//and EAX,ffff
			if(short_ok(num2,FALSE))i=2;	//короткая форма
		 	op66(r32);
			op(0x69+i);	//imul EAX,num
			op(0xc0);
			if(i==2)op(num2);
			else outdword(num2);
		 	op66(r32);
			outword(0xE8C1);
			op(0x10);	//shr EAX,16
			setzeroflag=TRUE;
		}
		else{
			if(razr==r32)num=(unsigned long)0xFFFFFFFFL/num+1;
			else num=65536/(unsigned int)num+1;
			op66(razr);
			op(0xBA);	//mov DX,num
			if(razr==r16)outword(num);
			else outdword(num);
			op66(razr);
			outword(0xE2F7);	//mul DX
			op66(razr);
			outword(0xD089);  //mov AX,DX
			setzeroflag=FALSE;
			warningreg(regs[razr/2-1][2]);
		}
		return;
	}
stddiv:*/
	ClearDX(razr,sign);
	DivNum2(num,razr,sign);
}

void ClearDX(int razr,int sign)
{
	if(sign)cwdq(razr);
	else{
		op66(razr);
		outword(0xD231);
	}
	warningreg(regs[razr/2-1][EDX]);
	ClearReg(DX);
}

void DivNum2(unsigned long num,int razr,int sign)
{
	MovRegNum(razr,itok.flag&f_reloc,num,ECX);
	op66(razr);
	if(sign)outword(0xF9F7);  /* IDIV CX */
	else outword(0xF1F7); /* DIV CX */
	warningreg(regs[razr/2-1][ECX]);
	ClearReg(CX);
	ClearReg(AX);
}

int getintoreg(int reg,int razr,int sign,char **ofsstr)
{
ITOK oitok,oitok2;
int oline,oendinptr;
int oinptr,otok,otok2;
unsigned char *oinput;
unsigned char ocha;
int useeax=FALSE;
int operand=tk_plus;
int rettype=tk_reg;
char *obuf;
SINFO ostr;
int i=0;
int j=0;
int onlynum=FALSE;
COM_MOD *bmod;
	switch(tok){
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			break;
		default:
//			if(cur_mod)break;	//10.08.04 22:50 из-за define прекратить
			obuf=bufrm;
			bufrm=NULL;
			ostr=strinf;
			strinf.bufstr=NULL;
			oitok=itok;
			oitok2=itok2;
			otok=tok;
			otok2=tok2;
			oline=linenum2;
			oinptr=inptr2;
			oinput=input;
			oendinptr=endinptr;
			bmod=cur_mod;
			while(bmod){
				bmod->freze=TRUE;
				bmod=bmod->next;
			}
			bmod=cur_mod;
			ocha=cha2;
//			printf("input=%08X inptr=%08X %s\n",input,inptr2,input+inptr2);
			if(tok==tk_number)onlynum=TRUE;
			while((!useeax)&&itok.type!=tp_stopper&&tok!=tk_eof){
				nexttok();
				if(itok.type==tp_stopper)break;
				if(itok.type==tp_opperand)operand=tok;
				else{
					i++;
					if(bufrm){						free(bufrm);						bufrm=NULL;					}
					if(strinf.bufstr)free(strinf.bufstr);
					switch(operand){
						case tk_div:
						case tk_mod:
						case tk_divminus:
						case tk_modminus:
							if(j==0)j=1;
							if((tok==tk_reg||tok==tk_reg32)&&itok.number==reg)useeax=TRUE;
							if(j==1){
								if(tok==tk_number){
									if(onlynum==FALSE&&caselong(itok.number)==NUMNUM)j++;
								}
								else{
									j++;
									onlynum=FALSE;
								}
							}
							break;
					}
				}
			}
			if(bmod!=cur_mod){
				if(bmod&&bmod->freze){
					cur_mod=bmod;
					while(bmod){
						bmod->freze=FALSE;
						bmod=bmod->next;
					}
				}
				else{
					do{
						COM_MOD *temp=cur_mod;
						cur_mod=cur_mod->next;
//						printf("bmod=%08X cur_mod=%08X\n",bmod,cur_mod);
						if(temp->paramdef)free(temp->paramdef);
						free(temp);
					}while(bmod!=cur_mod);
					while(bmod){
						bmod->freze=FALSE;
						bmod=bmod->next;
					}
				}
				input=oinput;
			}
			endinptr=oendinptr;
			itok=oitok;
			itok2=oitok2;
			tok=otok;
			tok2=otok2;
			linenum2=oline;
			inptr2=oinptr;
			cha2=ocha;
			endoffile=0;
//			printf("input=%08X inptr=%08X %s\n",input,inptr2,input+inptr2);
//			if(bufrm) { free(bufrm); bufrm=NULL; }
//			if(strinf.bufstr)free(strinf.bufstr);
			bufrm=obuf;
			strinf=ostr;
			break;
	}
	if(useeax){
		op66(razr);
		op(0x50);
		addESP+=razr==r16?2:4;
		do_e_axmath(0,razr,ofsstr);
		op66(razr);
		if(optimizespeed){
			op(0x89);
			op(0xC0+reg);
		}
		else op(0x90+reg);
		op66(razr);
		addESP-=razr==r16?2:4;
		op(0x58);
	}
	else{
		if(i==1&&j>=2){
			i=EAX;
			j=1;
		}
		else i=reg;
		rettype=getintoreg_32(i,razr,sign,ofsstr);
		if(itok.type!=tp_stopper&&itok.type!=tp_compare&&tok!=tk_eof){
			doregmath_32(reg,razr,sign,ofsstr,j);
			rettype=tk_reg;
		}
	}
	return rettype;
}

void dobits()
{
ITOK wtok;
char *wbuf;
SINFO wstr;
int razr,i,sign=0,posiblret,pointr=0;
unsigned int rettype;
int numpointr=0;
char *ofsstr=NULL;
	posiblret=rettype=tk_dword;
	razr=r32;
	i=itok.bit.siz+itok.bit.ofs;
	if(i<9){
		razr=r8;
		posiblret=rettype=tk_byte;
	}
	else if(i<17){
		posiblret=rettype=tk_word;
		razr=r16;
	}
	else if(i>32)razr=r64;
	if(tok2==tk_assign){
		wstr=strinf;
		strinf.bufstr=NULL;
		wtok=itok;
		wbuf=bufrm;
		bufrm=NULL;
		nexttok();
		nexttok();
		convert_type(&sign,(int *)&rettype,&pointr);
		while(tok==tk_mult){
			nexttok();
			numpointr++;
		}
		if(numpointr>itok.npointr)unuseableinput();
		if(tok2==tk_assign){
			MultiAssign(razr,EAX,numpointr);
			goto axtobit;
		}
		if(tok==tk_pointer)cpointr(am32==TRUE?EAX:BX,numpointr);
		CheckMinusNum();
		if(tok==tk_number){
			if(itok2.type==tp_opperand){	//сложное выражение
				if(!OnlyNumber(0))goto labl1;
				itok.flag=(unsigned char)postnumflag;
			}
			else{
				unsigned long num=itok.number;
				nexttok();
				itok.number=num;
			}
			CheckAllMassiv(wbuf,razr,&wstr,&wtok);
			if(razr!=r64)num2bits(&wtok,itok.number,razr);
			else{
				int siz=wtok.bit.siz;
				wtok.bit.siz=32-wtok.bit.ofs;
				num2bits(&wtok,itok.number,r32);
				wtok.bit.siz=siz+wtok.bit.ofs-32;
				wtok.bit.ofs=0;
				itok.number=itok.number>>(32-wtok.bit.siz);
				wtok.number+=4;
				num2bits(&wtok,itok.number,r8);
			}
		}
		else{
labl1:
			if(rettype==tk_char||rettype==tk_byte)doalmath(sign,&ofsstr);
			else if(rettype==tk_int||rettype==tk_word)do_e_axmath(sign,r16,&ofsstr);
			else if(rettype==tk_float)doeaxfloatmath(tk_reg32,AX);
			else do_e_axmath(sign,r32,&ofsstr);
			convert_returnvalue(posiblret,rettype);
			CheckAllMassiv(wbuf,razr,&wstr,&wtok);
axtobit:
			if(razr!=r64)reg2bits(&wtok,razr);
			else{
				int siz=wtok.bit.siz;
				op66(r32);
				op(0x50);	//push eax
				if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=4;
				addESP+=4;
				wtok.bit.siz=32-wtok.bit.ofs;
				reg2bits(&wtok,r32);
				op66(r32);
				op(0x58);	//pop eax
				addESP-=4;
				op66(r32);	//shr eax,size
				outword(0xE8C1);
				op(wtok.bit.siz);
				wtok.bit.siz=siz+wtok.bit.ofs-32;
				wtok.bit.ofs=0;
				wtok.number+=4;
				reg2bits(&wtok,r8);
			}
			return;
		}
	}
	else{
		bits2reg(AX,razr);
		wstr=strinf;
		strinf.bufstr=NULL;
		wtok=itok;
		wbuf=bufrm;
		bufrm=NULL;
		switch(razr){
			case r8:
				dobeg(AL);
				break;
			case r16:
			case r32:
				doreg_32(AX,razr);
				break;
			case r64:
				doreg_32(AX,r32);
				break;
		}
		goto axtobit;
	}
	seminext();
}

void bits2reg(int reg,int razr)
{
int i,j,skip66=FALSE;
	i=itok.bit.siz+itok.bit.ofs;
	j=~GetBitMask(itok.bit.ofs,itok.bit.siz);
	ITOK wtok;
	char *wbuf;
	wbuf=bufrm;
	bufrm=NULL;
	wtok=itok;
	SINFO wstr;
	wstr=strinf;
	strinf.bufstr=NULL;
	switch(razr){
		case r8:
			if(reg==AL){
				getintoal(tk_bytevar,&wtok,wbuf,&wstr);
				if(i!=8){
					op(4+0x20);	//and al,j
					op(j);
				}
			}
			else{
				CheckAllMassiv(bufrm,1,&strinf);
				outseg(&itok,2);
				op(0x8A);
				op(reg*8+itok.rm);
				outaddress(&itok);
				if(i!=8){
					op(128);
					op(128+64+reg+0x20);
					op(j);
				}
			}
			if(itok.bit.ofs){	//shr al,ofs
				if(itok.bit.ofs==1){
					op(0xD0);
					op(0xE8+reg);
				}
				else{
					op(0xC0);
					op(0xE8+reg);
					op(itok.bit.ofs);
				}
			}
			break;
		case r16:
		case r32:
			if(reg==AX){
				getinto_e_ax(0,(razr==r16?tk_wordvar:tk_dwordvar),&wtok,wbuf,&wstr,razr);
				if(razr==r16&&i!=16){
					op66(razr);	//and (e)ax,j
					op(4+1+0x20);
					outword(j);
				}
				else if(razr==r32&&i!=32){
					op66(razr);	//and (e)ax,j
					if(short_ok(j,TRUE)){
						op(128+2+1);
						op(128+64+0x20);
						op((int)j);
					}
					else{
						op(4+1+0x20);
						outdword(j);
					}
				}
				if(j<65536&&razr==r32)skip66=TRUE;
				if(itok.bit.ofs){	//shr (e)ax,ofs
					if(!skip66)op66(razr);
					if(itok.bit.ofs==1)outword(0xE8D1);
					else{
						outword(0xE8C1);
						op(itok.bit.ofs);
					}
				}
			}
			else{
int reg1=idxregs[0],reg2=idxregs[1];
				if(!am32){
					if(reg==idxregs[2]||reg==idxregs[1]){
						reg1=reg;
						if(reg==idxregs[1])reg2=idxregs[0];
					}
				}
				else{
					reg1=reg;
					if(reg==idxregs[1])reg2=idxregs[0];
				}
				CheckAllMassiv(bufrm,razr==r32?4:2,&strinf,&itok,reg1,reg2);
		 		op66(razr);
				outseg(&itok,2);
				op(0x8B);
				op(reg*8+itok.rm);
				outaddress(&itok);
				if((razr==r16&&i!=16)||(razr==r32&&i!=32)){
					op66(razr);	//and reg,j
					if(short_ok(j,razr==r16?FALSE:TRUE)){
						op(128+2+1);
						op(128+64+reg+0x20);
						op(j);
					}
					else{
						op(128+1);
						op(128+64+reg+0x20);
						if(razr==r16)outword(j);
						else outdword(j);
					}
				}
				if(itok.bit.ofs){	//shr reg,ofs
					op66(razr);
					if(itok.bit.ofs==1){
						op(0xD1);
						op(0xE8+reg);
					}
					else{
						op(0xC1);
						op(0xE8+reg);
						op(itok.bit.ofs);
					}
				}
			}
			break;
		case r64:
			if(reg==AX){
				getinto_e_ax(0,tk_dwordvar,&wtok,wbuf,&wstr,r32);
				itok.number+=4;
				outseg(&itok,2);
				op(0x8B);
				op(DX*8+itok.rm);
				outaddress(&itok);
				if(itok.bit.siz!=32){
					op(128);
					op(128+64+DL+0x20);
					op(li[itok.bit.siz+itok.bit.ofs-32]-1);
				}
				op66(r32);
				outword(0xAC0F);	//shrd edx,eax,ofs
				op(0xC2);
				op(itok.bit.ofs);
				itok.number-=4;
				warningreg(regs[1][EDX]);
			}
			else{
				int reg1=DX;
				if(reg==DX)reg1=CX;
				CheckAllMassiv(bufrm,4,&strinf,&itok);
		 		op66(r32);
				outseg(&itok,2);
				op(0x8B);
				op(reg*8+itok.rm);
				outaddress(&itok);
				itok.number+=4;
				outseg(&itok,2);
				op(0x8B);
				op(reg1*8+itok.rm);
				outaddress(&itok);
				if(itok.bit.siz!=32){
					op(128+(reg1<4?0:3));
					op(128+64+reg1+0x20);
					op(li[itok.bit.siz+itok.bit.ofs-32]-1);
				}
				itok.number-=4;
				op66(r32);
				outword(0xAC0F);	//shrd edx,eax,ofs
				op(0xc0+reg1+reg*8);
				op(itok.bit.ofs);
				warningreg(regs[1][reg1]);
			}
			ClearReg(DX);
			break;
	}
	ClearReg(AX);
	ClearReg(reg);
}

void num2bits(ITOK *gtok,unsigned long num,int razr)
{
unsigned int j,mask;
	mask=GetBitMask(gtok->bit.ofs,gtok->bit.siz);
	j=li[gtok->bit.siz]-1;
	if((num&j)!=j){
		if(razr!=r8){
			op66(razr); 	//and bits,mask
			outseg(gtok,2);
			if((postnumflag&f_reloc)==0&&short_ok(mask,razr/2-1)){
				op(128+2+1);
				op(gtok->rm+0x20);
				outaddress(gtok);
				op(mask);
			}
			else{
				op(128+1);
				op(gtok->rm+0x20);
				outaddress(gtok);
				if((postnumflag&f_reloc)!=0)AddReloc();
				if(razr==r16)outword(mask);
				else outdword(mask);
			}
		}
		else{
			outseg(gtok,2); 	//and bits,mask
			op(128);
			op(gtok->rm+0x20);
			outaddress(gtok);
			op(mask);
		}
	}
	num=(num&j)<<gtok->bit.ofs; 	//or bits,mask
	if(num<65536&&razr==r32)razr=r16;
	if(num<256&&razr==r16)razr=r8;
	if(razr!=r8){
		op66(razr);
		outseg(gtok,2);
		if((postnumflag&f_reloc)==0&&short_ok(num,razr/2-1)){
			op(128+2+1);
			op(gtok->rm+8);
			outaddress(gtok);
			op(num);
		}
		else{
			op(128+1);
			op(gtok->rm+8);
			outaddress(gtok);
			if((postnumflag&f_reloc)!=0)AddReloc();
			if(razr==r16)outword(num);
			else outdword(num);
		}
	}
	else{
		if((unsigned char)num!=0){
			outseg(gtok,2);
			op(128);
			op(gtok->rm+8);
			outaddress(gtok);
			op(num);
		}
	}
}

void reg2bits(ITOK *gtok,int razr)
{
int i,j,mask;
	j=li[gtok->bit.siz]-1;
	mask=GetBitMask(gtok->bit.ofs,gtok->bit.siz);
	i=gtok->bit.ofs+gtok->bit.siz;
	switch(razr){
		case r8:
			if(i!=8){
				op(4+0x20);	//and al,size
				op(j);
			}
			outseg(gtok,2);	//and bits,mask
			op(128);
			op(gtok->rm+0x20);
			outaddress(gtok);
			op(mask);
			if(gtok->bit.ofs)lshiftmul(gtok->bit.ofs,razr);
			outseg(gtok,2);
			op(8);
			op(gtok->rm);
			outaddress(gtok);
			break;
		case r16:
		case r32:
			if(razr==r16&&i!=16){
				op66(razr);	//and (e)ax,size
				op(4+1+0x20);
				outword(j);
			}
			else if(razr==r32&&i!=32){
				op66(razr);	//and (e)ax,size
				if(short_ok(j,TRUE)){
					op(128+2+1);
					op(128+64+0x20);
					op((int)j);
				}
				else{
					op(4+1+0x20);
					outdword(j);
				}
			}
			op66(razr);	//and bits,mask
			outseg(gtok,2);
			if(short_ok(mask,razr/2-1)){
				op(128+2+1);
				op(gtok->rm+0x20);
				outaddress(gtok);
				op(mask);
			}
			else{
				op(128+1);
				op(gtok->rm+0x20);
				outaddress(gtok);
				if(razr==r16)outword(mask);
				else outdword(mask);
			}
			if(gtok->bit.ofs)lshiftmul(gtok->bit.ofs,razr);
			op66(razr);
			outseg(gtok,2);
			op(1+8);
			op(gtok->rm);
			outaddress(gtok);
			break;
	}
}

void getoperand(int reg)
{
unsigned int numpointr=0;
	nexttok();
	while(tok==tk_mult){
		nexttok();
		numpointr++;
	}
	if(numpointr>itok.npointr){
		unuseableinput();
	}
	if(tok==tk_pointer){
		cpointr(reg,numpointr);
	}
	CheckMinusNum();
}

void cpointr(int reg,int numpointr)
{
	if(itok.type==tk_proc){
		if(tok2==tk_openbracket){
			tok=tk_proc;
		}
		else{
			itok.rm=itok.sib;
			if(am32){
				itok.sib=CODE32;
				tok=tk_dwordvar;
			}
			else{
				itok.sib=CODE16;
				tok=tk_wordvar;
			}
			compressoffset(&itok);
		}
		return;
	}
	int razr=typesize(itok.type);
	if(numpointr==itok.npointr){
		getpointeradr(&itok,bufrm,&strinf,numpointr-1,razr,reg);
		if(itok.type>=tk_char&&itok.type<=tk_float)tok=tk_charvar+itok.type-tk_char;
		else tok=(am32==FALSE?tk_wordvar:tk_dwordvar);
	}
	else if(numpointr<itok.npointr){
		if(numpointr)getpointeradr(&itok,bufrm,&strinf,numpointr-1,razr,reg);
		tok=(am32==FALSE?tk_wordvar:tk_dwordvar);
	}
	else unuseableinput();
}

void cwpointr(ITOK *wtok,char *&wbuf,SINFO *wstr,int *otok,int npointr,int ureg)
{
	if(wtok->type==tk_proc){
		wtok->rm=wtok->sib;
		if(am32){
			wtok->sib=CODE32;
			*otok=tk_dwordvar;
		}
		else{
			wtok->sib=CODE16;
			*otok=tk_wordvar;
		}
		compressoffset(wtok);
	}
	else{
		int razr=typesize(wtok->type);
		int reg=idxregs[2];
		if(reg==ureg)reg=idxregs[1];
		if(npointr==wtok->npointr){
			getpointeradr(wtok,wbuf,wstr,npointr-1,razr,reg);
			if(wtok->type>=tk_char&&wtok->type<=tk_float)*otok=tk_charvar+wtok->type-tk_char;
			else *otok=(am32==FALSE?tk_wordvar:tk_dwordvar);
		}
		else if(npointr<wtok->npointr){
			*otok=(am32==FALSE?tk_wordvar:tk_dwordvar);
			if(npointr)getpointeradr(wtok,wbuf,wstr,npointr-1,razr,reg);
			else return;
		}
		else unuseableinput();
		memcpy(wtok,&itok,sizeof(ITOK));
	}
}

int CheckAddOnly()
{
ITOK oitok,oitok2;
int oline;
int oinptr,otok,otok2;
unsigned char ocha;
char *obuf;
SINFO ostr;
int retval=TRUE;
int j=3;
int changesign=0;
	if(tok==tk_minusequals)changesign++;
	else if(tok!=tk_plusequals)return FALSE;
	if(itok2.type==tp_stopper)return FALSE;
newloop:
	obuf=bufrm;
	bufrm=NULL;
	ostr=strinf;
	strinf.bufstr=NULL;
	oitok=itok;
	oitok2=itok2;
	otok=tok;
	otok2=tok2;
	oline=linenum2;
	oinptr=inptr2;
	ocha=cha2;
	while(tok2==tk_minus||tok2==tk_mult)nexttok();
	while(itok.type!=tp_stopper&&tok!=tk_eof){
		nexttok();
		switch(tok){
			case tk_ID:
			case tk_id:
			case tk_proc:
			case tk_apiproc:
			case tk_undefproc:
			case tk_declare:
				retval=FALSE;
				itok.type=tp_stopper;
				break;
		}
		if(itok.type==tp_stopper)break;
		if(itok.type==tp_opperand){
			if(tok!=tk_plus&&tok!=tk_minus){
				retval=FALSE;
				break;
			}
			else if(changesign==2){
				int i=inptr2-1;
				char c;
				do{
					i--;
					c=input[i];
					i--;
				}while(c!='-'&&c!='+');
				i++;
				if(c=='-')c='+';
				else c='-';
				input[i]=c;
			}
			while(itok2.type==tp_opperand&&tok!=tk_eof)nexttok();
		}
		else{
			if(tok!=tk_number&&tok!=tk_postnumber&&tok!=tk_undefofs){
				if(j>1)j=0;
				if(bufrm){					free(bufrm);					bufrm=NULL;				}
				if(strinf.bufstr)free(strinf.bufstr);
			}
			else if(j>1)j--;
		}
	}
	itok=oitok;
	itok2=oitok2;
	tok=otok;
	tok2=otok2;
	linenum2=oline;
	inptr2=oinptr;
	cha2=ocha;
	endoffile=0;
	bufrm=obuf;
	strinf=ostr;
	if(j==1)retval=FALSE;
	else if(changesign==1){
		changesign++;
		goto newloop;
	}
	return retval;
}

int doqwordvar(int terminater)	//64 bit memory variable
{
unsigned char next=1,getfromAX=0;
unsigned int vop=0,otok,rettype;
int sign,i;
ITOK wtok;
char *wbuf,*rbuf;
SINFO wstr;
int retrez=0,pointr=0;
int numpointr=0;
int reg;
char *ofsstr=NULL;
int reg1=idxregs[0],reg2=idxregs[1];
	rettype=tk_qword;
	sign=0;
	wstr=strinf;
	strinf.bufstr=NULL;
	wtok=itok;
	wbuf=bufrm;
	bufrm=NULL;
	otok=tok;
	nexttok();
	switch(tok){
		case tk_assign:	//=
			nexttok();
			convert_type(&sign,(int *)&rettype,&pointr);
			while(tok==tk_mult){
				nexttok();
				numpointr++;
			}
			if(numpointr>itok.npointr){
				unuseableinput();
			}
			CheckMinusNum();
			if(itok2.type==tp_opperand){	//сложное выражение
				if(tok==tk_number){	//проверка и суммирование чисел
					switch(rettype){
						case tk_float: sign=2; break;
						case tk_double: sign=3; break;
						case tk_qword: sign=4; break;
					}
					if(OnlyNumber(sign)){
						next=0;
						itok.flag=(unsigned char)postnumflag;
						goto numbertovar;
					}
				}
				goto labl1;
			}
			else{
				switch(tok){
					case tk_number:
						if((itok.flag&f_reloc)==0){
							if(itok.lnumber==0){
		 						CheckAllMassiv(wbuf,8,&wstr,&wtok);
								for(i=0;i<2;i++){
									op66(r32);
									outseg(&wtok,2);
									op(0x83);
									op(wtok.rm+0x20);
									outaddress(&wtok);
									op(0);
									if(i==1)break;
									wtok.number+=4;
									compressoffset(&wtok);
								}
								break;
							}
							if(itok.lnumber==0xFFFFFFFFFFFFFFFFLL){
		 						CheckAllMassiv(wbuf,8,&wstr,&wtok);
								for(i=0;i<2;i++){
									op66(r32);
									outseg(&wtok,2);
									op(0x83);
									op(wtok.rm+0x8);
									outaddress(&wtok);
									op(0xFF);
									if(i==1)break;
									wtok.number+=4;
									compressoffset(&wtok);
								}
								break;
							}
						}
numbertovar:
						CheckAllMassiv(wbuf,8,&wstr,&wtok);
						for(i=0;i<2;i++){
							op66(r32);
							if(regoverstack&&short_ok(itok.number,TRUE)&&(itok.flag&f_reloc)==0){
								op(0x6A);
								op(itok.number);	//push short number
								op66(r32);
								outseg(&wtok,2);
								op(0x8f);
								op(wtok.rm);
								outaddress(&wtok);
							}
							else{
								outseg(&wtok,2);
								op(0xC7);	//mov word[],number
								op(wtok.rm);
								outaddress(&wtok);
								if((itok.flag&f_reloc)!=0)AddReloc();
								outdword(itok.number);
							}
							if(i==1)break;
							itok.lnumber>>=32;
							wtok.number+=4;
							compressoffset(&wtok);
						}
						break;
					case tk_apioffset:
					case tk_postnumber:
					case tk_undefofs:
						CheckAllMassiv(wbuf,8,&wstr,&wtok);
						op66(r32);
						outseg(&wtok,2);
						op(0xC7);	//mov word[],number
						op(wtok.rm);
						outaddress(&wtok);
						if(tok==tk_apioffset)AddApiToPost(itok.number);
						else{
							if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
							else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
//						else if((itok.flag&f_reloc)!=0)AddReloc();
							outdword(itok.number);
						}
						wtok.number+=4;
						compressoffset(&wtok);
						op66(r32);
						outseg(&wtok,2);
						op(0x83);
						op(wtok.rm+0x20);
						outaddress(&wtok);
						op(0);
						break;
					case tk_reg64:
						goto getfromreg;
					case tk_reg32:
						if(itok.number==AX&&wbuf==NULL&&wstr.bufstr==NULL&&
							((wtok.rm==rm_d16&&wtok.sib==CODE16)||
							(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
							op66(r32);
							outseg(&wtok,1);
							op(0xA3); // MOV [word],AX
							if(wtok.post==UNDEF_OFSET){
								AddUndefOff(2,wtok.name);
								wtok.post=0;
							}
							if(am32==FALSE)outword(wtok.number);
							else outdword(wtok.number);
						}
						else{
							CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
							op66(r32);
							outseg(&wtok,2);
							op(0x89);
							op((unsigned int)itok.number*8+wtok.rm);
							outaddress(&wtok);
						}
						wtok.number+=4;
						compressoffset(&wtok);
						op66(r32);
						outseg(&wtok,2);
						op(0x83);
						op(wtok.rm+0x20);
						outaddress(&wtok);
						op(0);
						break;
					case tk_string:
						CheckAllMassiv(wbuf,8,&wstr,&wtok);
						op66(r32);
						outseg(&wtok,2);
						op(0xC7);
						op(wtok.rm);
						outaddress(&wtok);
						outdword(addpoststring());
						wtok.number+=4;
						compressoffset(&wtok);
						op66(r32);
						outseg(&wtok,2);
						op(0x83);
						op(wtok.rm+0x20);
						outaddress(&wtok);
						op(0);
						break;
		 			default:
labl1:
						reg=EAX|(EDX*256);
						getintoreg64(reg);
						doregmath64(reg);
						getfromAX=1;
						next=0;
						break;
				}
			}
			if(getfromAX){
getfromax:
				itok.number=EAX|(EDX*256);
getfromreg:
				reg=itok.number&255;
				for(i=0;i<2;i++){
					if(reg==AX&&wbuf==NULL&&wstr.bufstr==NULL&&
						((wtok.rm==rm_d16&&wtok.sib==CODE16)||
						(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
						op66(r32);
						outseg(&wtok,1);
						op(0xA3); // MOV [word],AX
						if(wtok.post==UNDEF_OFSET){
							AddUndefOff(2,wtok.name);
							wtok.post=0;
						}
						if(am32==FALSE)outword(wtok.number);
						else outdword(wtok.number);
					}
					else{
						CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
						op66(r32);
						outseg(&wtok,2);
						op(0x89);
						op((unsigned int)reg*8+wtok.rm);
						outaddress(&wtok);
					}
					if(i==1)break;
					wtok.number+=4;
					compressoffset(&wtok);
					reg=itok.number/256;
				}
				warningreg(regs[1][EAX]);
				warningreg(regs[1][EDX]);
				ClearReg(AX);
				ClearReg(DX);
				retrez=tk_reg64;
			}
			break;
		case tk_minusminus: vop=0x8;
		case tk_plusplus:
			CheckAllMassiv(wbuf,8,&wstr,&wtok);
			op66(r32);
			outseg(&wtok,2);
incdec:
			op(0xFF); op(vop+wtok.rm);
			outaddress(&wtok);
			wtok.number+=4;
			compressoffset(&wtok);
			CheckAllMassiv(wbuf,8,&wstr,&wtok);
			op66(r32);
			outseg(&wtok,2);
			op(0x83); op(0x10+vop+wtok.rm);
			outaddress(&wtok);
			op(0);
			break;
		case tk_xorequals: vop+=0x08;
		case tk_minusequals: vop+=0x08;
		case tk_andequals: vop+=0x18;
		case tk_orequals: vop+=0x08;
		case tk_plusequals:
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_opperand){
				if(tok==tk_number){
					if(OnlyNumber(4)){
						next=0;
						otok=tok;
						tok=tk_number;
						goto num;
					}
				}
				goto defxor;
			}
			else{
				switch(tok){
					case tk_number:
					case tk_postnumber:
					case tk_undefofs:
					case tk_apioffset:
num:
						for(i=0;i<2;i++){
							CheckAllMassiv(wbuf,8,&wstr,&wtok);
							op66(r32);
							outseg(&wtok,2);
							if(tok==tk_number&&(itok.flag&f_reloc)==0&&(vop==0||vop==0x28)){
								if(i==0&&itok.lnumber==1){
									if(vop)vop=8;
									if(next==0)tok=otok;
									goto incdec;
								}
								if(itok.number==1){
									op(0xFF); op((vop!=0?8:0)+wtok.rm);
									outaddress(&wtok);
									goto conl;
								}
							}
							if(i==1){
								if(vop==0)vop+=0x10;
								else if(vop==0x28)vop-=0x10;
							}
							if(tok!=tk_apioffset&&tok!=tk_undefofs&&tok!=tk_postnumber&&(itok.flag&f_reloc)==0&&
								short_ok(itok.number,TRUE)){
								op(0x83);
								op(vop+wtok.rm);
								outaddress(&wtok);
								op((unsigned int)itok.number);
							}
							else{
								op(0x81);
								op(vop+wtok.rm);
								outaddress(&wtok);
								if(tok==tk_apioffset)AddApiToPost(itok.number);
								else{
									if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
									else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
									else if((itok.flag&f_reloc)!=0)AddReloc();
									outdword(itok.number);
								}
							}
conl:
							wtok.number+=4;
							compressoffset(&wtok);
							itok.lnumber>>=32;
						}
						if(next==0)tok=otok;
						break;
					case tk_reg64:
						reg=itok.number&255;
						for(i=0;i<2;i++){
							if(i==1){
								if(vop==0)vop+=0x10;
								else if(vop==0x28)vop-=0x10;
							}
							CheckAllMassiv(wbuf,8,&wstr,&wtok);
							op66(r32);
							outseg(&wtok,2);
							op(0x01+vop); op((unsigned int)reg*8+wtok.rm);
							outaddress(&wtok);
							if(i==1)break;
							wtok.number+=4;
							compressoffset(&wtok);
							reg=itok.number/256;
						}
						break;
					default:
defxor:
						reg=EAX|(EDX*256);
						getintoreg64(reg);
						doregmath64(reg);
						CheckAllMassiv(wbuf,8,&wstr,&wtok);
						reg=EAX;
						for(i=0;i<2;i++){
							op66(r32);
							op(0x01+vop);
							op(wtok.rm+reg*8);
							outaddress(&wtok);
							if(i==1)break;
							if(vop==0)vop=0x10;
							if(vop==0x28)vop=0x18;
							reg=EDX;
							wtok.number+=4;
							compressoffset(&wtok);
						}
						next=0;
						retrez=tk_reg64;
						warningreg(regs[1][EAX]);
						warningreg(regs[1][EDX]);
						break;
				}
			}
			break;
		case tk_multequals:
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_stopper&&tok==tk_number&&(itok.flag&f_reloc)==0){
				if(itok.lnumber==1)break;
				if(itok.lnumber==0){
					ZeroReg(EAX,r32);
					ZeroReg(EDX,r32);
					goto getfromax;
				}
				if((i=caselong(itok.number))!=NUMNUM){
					if(wbuf==NULL&&wstr.bufstr==NULL&&
							((wtok.rm==rm_d16&&wtok.sib==CODE16)||
							(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
						op66(r32);
						outseg(&wtok,1);
						op(0xA1); // MOV EAX,[dword]
						if(wtok.post==UNDEF_OFSET){
							AddUndefOff(2,wtok.name);
							wtok.post=0;
						}
						if(am32==FALSE)outword(wtok.number);
						else outdword(wtok.number);
					}
					else{
						CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
						op66(r32);
						outseg(&wtok,2);
						op(0x8B);
						op(wtok.rm);
						outaddress(&wtok);
					}
					wtok.number+=4;
					compressoffset(&wtok);
					ClearReg(AX);
					CheckAllMassiv(wbuf,8,&wstr,&wtok);
					op66(r32);
					outseg(&wtok,3);
					op(0x0F);
					op(0xA4+vop);
					op(wtok.rm);  // SHLD [rmword],CL
					outaddress(&wtok);
					op(i);
					wtok.number-=4;
					compressoffset(&wtok);
					CheckAllMassiv(wbuf,8,&wstr,&wtok);
					op66(r32);
					outseg(&wtok,2);
					op(0xC1);
					op(0x20+wtok.rm);
					outaddress(&wtok);
					op(i);
					break;
				}
			}
			CheckAllMassiv(wbuf,8,&wstr,&wtok);
			wtok.number+=4;
			compressoffset(&wtok);
			for(i=0;i<2;i++){
				op66(r32);
				outseg(&wtok,2);
				op(0xFF);	op(0x30+wtok.rm);
				outaddress(&wtok);
				if(i==1)break;
				wtok.number-=4;
				compressoffset(&wtok);
			}
			reg=ECX|(EAX*256);
			getintoreg64(reg);
			doregmath64(reg);
			CallExternProc("__llmul");
			next=0;
			goto getfromax;
		case tk_divequals:
			getoperand(am32==TRUE?EAX:BX);
			if(itok2.type==tp_stopper&&tok==tk_number&&(itok.flag&f_reloc)==0){
				if(itok.number==0){
					DevideZero();
					break;
				}
				if(itok.number==1)break;
				if((i=caselong(itok.number))!=NUMNUM){
					wtok.number+=4;
					compressoffset(&wtok);
					if(wbuf==NULL&&wstr.bufstr==NULL&&
							((wtok.rm==rm_d16&&wtok.sib==CODE16)||
							(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
						op66(r32);
						outseg(&wtok,1);
						op(0xA1); // MOV EAX,[dword]
						if(wtok.post==UNDEF_OFSET){
							AddUndefOff(2,wtok.name);
							wtok.post=0;
						}
						if(am32==FALSE)outword(wtok.number);
						else outdword(wtok.number);
					}
					else{
						CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
						op66(r32);
						outseg(&wtok,2);
						op(0x8B);
						op(wtok.rm);
						outaddress(&wtok);
					}
					wtok.number-=4;
					compressoffset(&wtok);
					ClearReg(AX);
					CheckAllMassiv(wbuf,8,&wstr,&wtok);
					op66(r32);
					outseg(&wtok,3);
					op(0x0F);
					op(0xAC);
					op(wtok.rm);  // SHLD [rmword],CL
					outaddress(&wtok);
					op(i);
					wtok.number+=4;
					compressoffset(&wtok);
					CheckAllMassiv(wbuf,8,&wstr,&wtok);
					op66(r32);
					outseg(&wtok,2);
					op(0xC1);
					op(0x28+wtok.rm);
					outaddress(&wtok);
					op(i);
					break;
				}
				unsigned long number;
				number=itok.lnumber>>32;
				for(i=0;i<2;i++){
					op66(r32);
					if((itok.flag&f_reloc)==0&&short_ok(number,1)){
						op(0x6A);
						op(number);
					}
					else{
						op(0x68);
						if(i==0&&(itok.flag&f_reloc)!=0)AddReloc();
						outdword(number);
					}
					if(i==1)break;
					number=itok.number;
				}
				goto divcont;
			}
			reg=EAX|(EDX*256);
			getintoreg64(reg);
			doregmath64(reg);
			op66(r32);
			op(0x50+EDX);
			op66(r32);
			op(0x50+EAX);
			next=0;
divcont:
			addESP+=8;
			if(ESPloc&&am32&&wtok.segm==SS)wtok.number+=8;
			reg=EAX;
			for(i=0;i<2;i++){
				if(reg==AX&&wbuf==NULL&&wstr.bufstr==NULL&&
					((wtok.rm==rm_d16&&wtok.sib==CODE16)||
					(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
					op66(r32);
					outseg(&wtok,1);
					op(0xA1);
					if(wtok.post==UNDEF_OFSET){
						AddUndefOff(2,wtok.name);
					}
					if(am32==FALSE)outword(wtok.number);
					else outdword(wtok.number);
				}
				else{
					CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
					op66(r32);
					outseg(&wtok,2);
					op(0x8B);
					op(reg*8+wtok.rm);
					outaddress(&wtok);
				}
				if(i==1)break;
				wtok.number+=4;
				compressoffset(&wtok);
				reg=EDX;
			}
			CallExternProc("__lludiv");
			addESP-=8;
			wtok.number-=4;
			compressoffset(&wtok);
			goto getfromax;
		case tk_swap:
			int regdi;
			regdi=TRUE;
			getoperand();
			rbuf=bufrm;
			bufrm=NULL;
			if(am32!=FALSE&&wbuf!=NULL&&wstr.bufstr!=NULL)regdi=FALSE;
			switch(tok){
				case tk_reg64:
					CheckAllMassiv(wbuf,8,&wstr,&wtok);
					reg=itok.number&255;
					for(i=0;i<2;i++){
						op66(r32);
						outseg(&wtok,2);
						op(0x87);
						op(reg*8+wtok.rm);
						outaddress(&wtok);
						ClearReg(reg);
						if(i==1)break;
						reg=itok.number/256;
						wtok.number+=4;
						compressoffset(&wtok);
					}
					break;
				case tk_qwordvar:
					for(i=0;i<2;i++){
						getinto_e_ax(0,tk_dwordvar,&wtok,wbuf,&wstr,r32,TRUE);
						CheckAllMassiv(rbuf,8,&strinf,&itok,regdi==FALSE?BX:DI,DX);
						op66(r32);
						outseg(&itok,2);
						op(0x87);  // XCHG AX,[wloc]
						op(itok.rm);
						outaddress(&itok);
						KillVar(itok.name);
						if(wbuf==NULL&&wstr.bufstr==NULL&&
								((wtok.rm==rm_d16&&wtok.sib==CODE16)||
								(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
							op66(r32);
							outseg(&wtok,1);
							op(0xA3); /* MOV [word],AX */
							if(wtok.post==UNDEF_OFSET){
								AddUndefOff(2,wtok.name);
								wtok.post=0;
							}
							if(am32==FALSE)outword(wtok.number);	//????
							else outdword(wtok.number);
						}
						else{
							CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
							op66(r32);
							outseg(&wtok,2);
							op(0x89); op(wtok.rm); /* MOV [rmword],AX */
							outaddress(&wtok);
						}
						if(i==1)break;
						itok.number+=4;
						compressoffset(&itok);
						wtok.number+=4;
						compressoffset(&wtok);
					}
					warningreg(regs[1][EAX]);
					ClearReg(EAX);
					break;
				default: swaperror(); break;
			}
			break;
		case tk_rrequals:
			vop=8;
			wtok.number+=4;
			compressoffset(&wtok);
		case tk_llequals:
			getoperand(am32==TRUE?ECX:BX);
			if(itok2.type!=tp_stopper){
				getintobeg(CL,&ofsstr);
				ClearReg(CX);
				warningreg(begs[1]);
				next=0;
				i=1;
			}
			else if(tok==tk_number){
				i=0;
			}
			else{
				if(tok!=tk_beg||(unsigned int)itok.number!=CL){
					getintobeg(CL,&ofsstr);
					ClearReg(CX);
					warningreg(begs[1]);
					next=0;
				}
				i=1;
			}
			if(wbuf==NULL&&wstr.bufstr==NULL&&
				((wtok.rm==rm_d16&&wtok.sib==CODE16)||
				(wtok.rm==rm_d32&&(wtok.sib==CODE32||wtok.sib==0)))){
				op66(r32);
				outseg(&wtok,1);
				op(0xA1); // MOV EAX,[dword]
				if(wtok.post==UNDEF_OFSET){
					AddUndefOff(2,wtok.name);
					wtok.post=0;
				}
				if(am32==FALSE)outword(wtok.number);
				else outdword(wtok.number);
			}
			else{
				CheckAllMassiv(wbuf,8,&wstr,&wtok,reg1,reg2);
				op66(r32);
				outseg(&wtok,2);
				op(0x8B);
				op(wtok.rm);
				outaddress(&wtok);
			}
			if(vop)wtok.number-=4;
			else wtok.number+=4;
			compressoffset(&wtok);
			ClearReg(AX);
			warningreg(regs[1][EAX]);
			CheckAllMassiv(wbuf,8,&wstr,&wtok);
			op66(r32);
			outseg(&wtok,3);
			op(0x0F);
			op(0xA4+vop+i);
			op(wtok.rm);  // SHLD [rmword],CL
			outaddress(&wtok);
			if(i==0)op((unsigned int)itok.number);
			if(vop)wtok.number+=4;
			else wtok.number-=4;
			compressoffset(&wtok);
			CheckAllMassiv(wbuf,8,&wstr,&wtok);
			op66(r32);
			outseg(&wtok,2);
			op(i==0?0xC1:0xD3);
			op(0x20+vop+wtok.rm);
			outaddress(&wtok);
			if(i==0)op(itok.number);
			break;
		default: operatorexpected(); break;
	}
	KillVar(wtok.name);
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	if(cpu<3)cpu=3;
	return retrez;
}

void Select2FreeReg(int r1,int r2,int *reg1,int *reg2)
{
	*reg1=idxregs[0];
	*reg2=idxregs[1];
	if(r1==idxregs[0]){
		if(r2==idxregs[1]){
			*reg1=idxregs[2];
			*reg2=idxregs[3];
		}
		else{
			*reg1=idxregs[1];
			if(r2==idxregs[2])*reg2=idxregs[3];
			else *reg2=idxregs[2];
		}
	}
	if(r1==idxregs[1]){
		if(r2==idxregs[0]){
			*reg1=idxregs[2];
			*reg2=idxregs[3];
		}
		else{
			*reg1=idxregs[0];
			if(r2==idxregs[2])*reg2=idxregs[3];
			else *reg2=idxregs[2];
		}
	}
}

void doreg64(int reg,int terminater)
{
unsigned char next=1;
int vop=0,sign=0;
int i;
int reg1,reg2;
unsigned long long ii;
int r1,r2;
char *ofsstr=NULL;
int rettype;
int pointr=0;
int numpointr=0;
	rettype=tk_qword;
	r1=reg&255;
	r2=reg/256;
	Select2FreeReg(r1,r2,&reg1,&reg2);
	if(r1==ESP||r2==ESP)RestoreStack();
	nexttok();
	switch(tok){
		case tk_assign://=
			nexttok();
			/*-----------------31.08.05 18:39-------------------

			--------------------------------------------------*/
			convert_type(&sign,(int *)&rettype,&pointr);
			while(tok==tk_mult){
				nexttok();
				numpointr++;
			}
			if(numpointr>itok.npointr){
				unuseableinput();
			}
			CheckMinusNum();
			if(tok==tk_number){	//проверка и суммирование чисел
				switch(rettype){
					case tk_float: sign=2; break;
					case tk_double: sign=3; break;
					case tk_qword: sign=4; break;
				}
				if(OnlyNumber(sign)){
					MovRegNum(r32,postnumflag&f_reloc,itok.number,r1);
					MovRegNum(r32,0,itok.lnumber>>32,r2);
					next=0;
					break;
				}
			}
			if(rettype==tk_float||rettype==tk_double){
				doeaxfloatmath(tk_stackstart,0,rettype==tk_float?0:4);
				op66(r32);
				op(0x58+r1);
				op66(r32);
				if(rettype==tk_float){
					op(0x31); op(0xC0+r2*9);
				}
				else op(0x58+r2);
				next=0;
				break;
			}
			/*-----------------31.08.05 18:39-------------------

			--------------------------------------------------*/
			getintoreg64(reg);
			doregmath64(reg);
			next=0;
			break;
		case tk_plusplus: op66(r32); op(0x40+r1);
			op66(r32);
			op(0x83);
			op(0xD0+r2);
			op(0);
			break;
		case tk_minusminus: op66(r32); op(0x48+r1);
			op66(r32);
			op(0x83);
			op(0xD8+r2);
			op(0);
			break;
		case tk_swap:
			getoperand(reg1);
			switch(tok){
				case tk_qwordvar:
					reg=r1;
					for(i=0;i<2;i++){
						CheckAllMassiv(bufrm,8,&strinf,&itok,reg1,reg2);
						op66(r32);
				 		outseg(&itok,2);
						op(0x87);
						op(reg*8+itok.rm);
						outaddress(&itok);
						itok.number+=4;
						compressoffset(&itok);
						reg=r2;
					}
					break;
				case tk_reg64:
					reg=r1;
					vop=itok.number;
					itok.number&=255;
					for(i=0;i<2;i++){
						if(reg!=(int)itok.number){
							if(RegSwapReg(reg,itok.number,r32)==NOINREG){;
								op66(r32);
								if(reg==AX)op(0x90+(unsigned int)itok.number);
								else if((unsigned int)itok.number==AX)op(0x90+reg);
								else{
									op(0x87);
									op(0xC0+(unsigned int)itok.number+reg*8);
								}
							}
							else waralreadinitreg(regs[1][reg],regs[1][itok.number]);
						}
						reg=r2;
						itok.number=vop/256;
					}
					break;
				default: swaperror(); break;
			}
			break;
		case tk_xorequals: vop+=0x08;
		case tk_minusequals: vop+=0x08;
		case tk_andequals: vop+=0x18;
		case tk_orequals: vop+=0x08;
		case tk_plusequals:
			if(CheckAddOnly()){
				inptr2--;
				cha2=' ';
				if(tok==tk_plusequals)tok=tk_plus;
				else tok=tk_minus;
				doregmath64(reg);
				next=0;
				break;
			}
			getoperand(reg1);
			if(itok2.type==tp_opperand&&tok!=tk_number&&tok!=tk_undefofs&&
				tok!=tk_postnumber)goto defadd;
			CheckMinusNum();
			idrec *rrec;
			int opost;
			i=tok;
			switch(tok){
				case tk_postnumber:
				case tk_undefofs:
					ii=itok.number;
					rrec=itok.rec;
					opost=itok.post;
					char uname[IDLENGTH];
					strcpy(uname,itok.name);
					if(itok.flag&f_extern)goto addnum;
					tok=tk_number;
				case tk_number:
					ii=doconstqwordmath();
					next=0;
					if(itok.type==tp_opperand){
						sign=reg1|(reg2*256);
						if(i==tk_postnumber||i==tk_undefofs){
							op66(r32);
							op(0xB8+r1);	// MOV reg,#
							if(i==tk_postnumber)(postnumflag&f_extern)==0?setwordpost(&itok):setwordext((long *)&ii);
							else{
								if((postnumflag&f_reloc)!=0)AddReloc();
								if(i==tk_undefofs)AddUndefOff(2,uname);
							}
							outdword(ii);
							ZeroReg(r2,r32);
						}
						else{
							MovRegNum(r32,postnumflag&f_reloc,ii,reg1);
							MovRegNum(r32,postnumflag&f_reloc,ii>>=32,reg2);
						}
						doregmath64(sign);
						itok.number=sign;
						goto addreg;
					}
					if((postnumflag&f_reloc)==0&&i!=tk_undefofs&&i!=tk_postnumber){
						optnumadd64(ii,r1,r2,vop);
						break;
					}
addnum:
					op66(r32);
				  if(r1==AX)op(0x05+vop);
					else{
						op(0x81);
						op(0xC0+vop+r1);
					}
					itok.rec=rrec;
					itok.post=opost;
					if(i==tk_postnumber)(postnumflag&f_extern)==0?setwordpost(&itok):setwordext((long *)&ii);
					else{
						if((postnumflag&f_reloc)!=0)AddReloc();
						if(i==tk_undefofs)AddUndefOff(2,uname);
					}
					outdword(ii);
					ii>>=32;
					if(vop==0)vop=0x10;
					else if(vop==0x28)vop=0x18;
					op66(r32);
				  if(r2==AX)op(0x05+vop);
					else{
						op(0x81);
						op(0xC0+vop+r2);
					}
					outdword(ii);
					break;
				case tk_longvar:
				case tk_dwordvar:
					CheckAllMassiv(bufrm,4,&strinf,&itok,reg1,reg2);
					op66(r32);
					outseg(&itok,2);
					op(0x03+vop);
					op(r1*8+itok.rm);
					outaddress(&itok);
					if(vop==0x20){	//&=
						ZeroReg(r2,r32);
					}
					else{
						if(vop==0||vop==0x28){
							if(vop)vop=8;
							op66(r32);
							op(0x83);
							op(0xD0+vop+r2);
							op(0);
						}
					}
					break;
				case tk_qword:
					CheckAllMassiv(bufrm,4,&strinf,&itok,reg1,reg2);
					reg=r1;
					for(i=0;i<2;i++){
						op66(r32);
						outseg(&itok,2);
						op(0x03+vop);
						op(reg*8+itok.rm);
						outaddress(&itok);
						if(i==1)break;
						reg=r2;
						itok.number+=4;
						compressoffset(&itok);
					}
					break;
				case tk_reg64:
addreg:
					reg=r1;
					reg2=itok.number&255;
					for(i=0;i<2;i++){
						op66(r32);
						op(0x01+vop);
						op(0xC0+reg+reg2*8);
						if(i==1)break;
						if(vop==0)vop=0x10;
						if(vop==0x28)vop=0x18;
						reg2=itok.number/256;
						reg=r2;
					}
					break;
				case tk_reg32:
					op66(r32);
					op(0x01+vop);
					op(0xC0+reg+(unsigned int)itok.number*8);
					if(vop==0x20){	//&=
						ZeroReg(r2,r32);
					}
					else{
						if(vop==0||vop==0x28){
							if(vop)vop=8;
							op66(r32);
							op(0x83);
							op(0xD0+vop+r2);
							op(0);
						}
					}
					break;
				case tk_ID:
				case tk_id:
				case tk_proc:
				case tk_apiproc:
				case tk_undefproc:
				case tk_declare:
unsigned char oaddstack;
					oaddstack=addstack;
					if(r1==EAX||r2==EAX){
						op66(r32);
						op(0x50);	//push AX
						warningreg(regs[1][EAX]);
						addESP+=4;
						ClearReg(EAX);
						addstack=FALSE;
					}
					if(r1==EDX||r2==EDX){
						op66(r32);
						op(0x50+EDX);	//push DX
						addESP+=4;
						warningreg(regs[1][EDX]);
						ClearReg(EDX);
						addstack=FALSE;
					}
					procdo(tk_qword);
					addstack=oaddstack;
					if(itok2.type==tp_opperand){
						nexttok();
						doregmath64(EAX|(EDX*256));
						next=0;
					}
					if(r1==EDX||r2==EDX){
						op66(r32);
						op(0x89);
						op(0xC0+reg2+EDX*8);	//mov reg,EDX
						op66(r32);
						op(0x58+EDX);	//pop dx
						addESP-=4;
					}
					else reg2=EDX;
					if(r1==EAX||r2==EAX){
						op66(r32);
						op(0x89);
						op(0xC0+reg1+EAX*8);	//mov reg,EAX
						op66(r32);
						op(0x58);	//pop ax
						addESP-=4;
					}
					else reg1=EAX;
					op66(r32);
					op(0x01+vop);
					op(0xc0+r1+reg1*8);	//add reg,ax
					if(vop==0)vop=0x10;
					if(vop==0x28)vop=0x18;
					op66(r32);
					op(0x01+vop);
					op(0xc0+r2+reg2*8);	//add reg,ax
					break;
				case tk_bytevar:
				case tk_charvar:
				case tk_beg:
				case tk_reg:
				case tk_intvar:
				case tk_wordvar:
defadd:
					sign=reg1|(reg2*256);
					getintoreg64(sign);
					doregmath64(sign);
					warningreg(regs[1][reg1]);
					warningreg(regs[1][reg2]);
					ClearReg(reg1);
					ClearReg(reg2);
					op66(r32);
					op(0x01+vop);
					op(0xc0+r1+reg1*8);	//add reg,ax
					if(vop==0)vop=0x10;
					if(vop==0x28)vop=0x18;
					op66(r32);
					op(0x01+vop);
					op(0xc0+r2+reg2*8);	//add reg,ax
					next=0;
					break;
				default: valueexpected(); break;
			}
			break;
		case tk_rrequals: vop+=0x08;
		case tk_llequals:
			getoperand(reg1);
			CheckMinusNum();
			if(tok==tk_number){
				ii=doconstqwordmath();
				next=0;
				if(itok.type==tp_opperand){
					if(r1==ECX||r2==ECX)regshifterror();
					op(0xB0+CL); op(ii);	//mov CL,num
					ConstToReg(ii,CL,r8);
					dobegmath(CL);
					warningreg(begs[1]);
					ClearReg(CL);
					goto shiftcl;
				}
				if(vop){
					reg=r1;
					r1=r2;
					r2=reg;
				}
				if(ii<32){
					op66(r32);
					op(0x0F);
					op(0xA4+vop);
					op(0xC0+r2+r1*8);
					op(ii);
					op66(r32);
					if(ii==1){
						op(0xD1); op(0xE0+r1+vop);
					}
					else{
						op(0xC1);
						op(0xE0+r1+vop);	//shl ax,num
						op(ii);
					}
				}
				else{
					op66(r32);
					op(0x89);
					op(0xC0+r2+r1*8);
					ii-=32;
					if(ii!=0){
						op66(r32);
						if(ii==1){
							op(0xD1);
							op(0xE0+vop+r1);	//shr ax,1
						}
						else{
							op(0xC1);
							op(0xE0+r2+vop);	//shl ax,num
							op(ii);
						}
					}
					ZeroReg(r1,r32);
				}
			}
			else if(reg!=CX){
				if(!(itok2.type==tp_stopper&&(tok==tk_beg||tok==reg||tok==tk_reg32)&&itok.number==CL)){
					getintobeg(CL,&ofsstr);
					dobegmath(CL);
					warningreg(begs[1]);
					ClearReg(CL);
					next=0;
				}
shiftcl:
				op66(r32);
				op(0x0F);
				op(0xA5+vop);
				if(vop){
					reg=r1;
					r1=r2;
					r2=reg;
				}
				op(0xC0+r2+r1*8);
				op66(r32);
				op(0xD3);
				op(0xE0+vop+r1);	// SHL xXX,CL
			}
			else regshifterror();
			break;
		case tk_multequals:
			getoperand(reg1);
			CheckMinusNum();
			if(tok==tk_number){
				ii=doconstqwordmath();
				next=0;
				if(itok.type==tp_opperand){
					op66(r32);
					op(0x50+r2);
					op66(r32);
					op(0x50+r1);
					addESP+=8;
					MovRegNum(r32,postnumflag&f_reloc,ii,reg1);
					MovRegNum(r32,0,ii>>32,reg2);
					ConstToReg(ii,reg1,r32);
					ConstToReg(ii>>32,reg2,r32);
					doregmath64(reg1|(reg2*256));
					goto mul;
				}
				i=0;
				if((postnumflag&f_reloc)==0){
					if(ii==0){
						ZeroReg(r1,r32);
						ZeroReg(r2,r32);
						break;
					}
					if(ii==1)break;
					if(ii==2){
				 		op66(r32);
						op(1);
						op(0xC0+9*r1); // ADD r1,r1
				 		op66(r32);
						op(0x83);
						op(0xC2+r2*8);	//adc r2,0
						op(0);
						break;
					}
					if((i=caselonglong(ii))!=NUMNUM64){
						if(i<32){
							op66(r32);
							outword(0xA40F);
							op(0xC0+r2+r1*8);
							op(i);
							op66(r32);
							op(0xC1);
							op(0xE0+r1);	//shl ax,num
							op(i);
						}
						else{
							op66(r32);
							op(0x89);
							op(0xC0+r2+r1*8);
							i-=32;
							if(i!=0){
								op66(r32);
								if(i==1){
									op(1);
									op(0xC0+r2*9);	//add reg,reg
								}
								else{
									op(0xC1);
									op(0xE0+r2);	//shl ax,num
									op(i);
								}
							}
							ZeroReg(r1,r32);
						}
						break;
					}
				}
				op66(r32);
				op(0x50+r2);
				op66(r32);
				op(0x50+r1);
				addESP+=8;
				MovRegNum(r32,postnumflag&f_reloc,ii,ECX);
				MovRegNum(r32,0,ii>>32,EAX);
				goto mul;
			}
			op66(r32);
			op(0x50+r2);
			op66(r32);
			op(0x50+r1);
			addESP+=8;
			reg=ECX|(EAX*256);
			warningreg(regs[1][ECX]);
			getintoreg64(reg);
			doregmath64(reg);
mul:
			CallExternProc("__llmul");
			addESP-=8;
endmul:
			ClearReg(EAX);
			warningreg(regs[1][EAX]);
			ClearReg(EDX);
			warningreg(regs[1][EDX]);
			next=0;
			if(r1!=EAX){
				if(r1==EDX){
					if(r2==EAX){
						op66(r32);
						op(0x90+EDX);
						break;
					}
					op66(r32);
					op(0x89);
					op(0xC0+r2+EDX*8);	//mov reg,EDX
					op66(r32);
					op(0x89);
					op(0xC0+r1+EAX*8);	//mov reg,EAX
					break;
				}
				op66(r32);
				op(0x89);
				op(0xC0+r1+EAX*8);	//mov reg,EAX
			}
			if(r2!=EDX){
				op66(r32);
				op(0x89);
				op(0xC0+r2+EDX*8);	//mov reg,EDX
			}
			break;
		case tk_divequals:
			getoperand(reg1);
			CheckMinusNum();
			if(tok==tk_number){
				ii=doconstqwordmath();
				next=0;
				if(itok.type==tp_opperand){
					MovRegNum(r32,postnumflag&f_reloc,ii,reg1);
					MovRegNum(r32,0,ii>>32,reg2);
					ConstToReg(ii,reg1,r32);
					ConstToReg(ii>>32,reg2,r32);
					doregmath64(reg1|(reg2*256));
					op66(r32);
					op(0x50+reg2);
					op66(r32);
					op(0x50+reg1);
					addESP+=8;
					warningreg(regs[1][reg1]);
					warningreg(regs[1][reg2]);
					goto divcont;
				}
				if((postnumflag&f_reloc)==0){
					if(ii==0){
						DevideZero();
						break;
					}
					if(ii==1)break;
					if((i=caselonglong(ii))!=NUMNUM64){
						if(i<32){
							op66(r32);
							outword(0xAC0F);
							op(0xC0+r1+r2*8);
							op(i);
							op66(r32);
							op(0xc1);
							op(0xe8+r2); // SHR reg,imm8
							op(i);
						}
						else{
							op66(r32);
							op(0x89);
							op(0xC0+r1+r2*8);
							i-=32;
							if(i!=0){
								op66(r32);
								if(i==1){
									op(0xD1);
									op(0xE8+r1);	//shr ax,1
								}
								else{
									op(0xC1);
									op(0xE8+r1);	//shr ax,num
									op(i);
								}
							}
							ZeroReg(r2,r32);
						}
						break;
					}
				}
				unsigned long number;
				number=ii>>32;
				for(i=0;i<2;i++){
					op66(r32);
					if((postnumflag&f_reloc)==0&&short_ok(number,1)){
						op(0x6A);
						op(number);
					}
					else{
						op(0x68);
						if(i==0&&(postnumflag&f_reloc)!=0)AddReloc();
						outdword(number);
					}
					if(i==1)break;
					number=ii;
				}
				addESP+=8;
				goto divcont;
			}
			reg=reg1|(reg2*256);
			getintoreg64(reg);
			doregmath64(reg);
			op66(r32);
			op(0x50+reg2);
			op66(r32);
			op(0x50+reg1);
			addESP+=8;
			warningreg(regs[1][reg1]);
			warningreg(regs[1][reg2]);
			next=0;
divcont:
			if(r1!=EAX){
				if(r2==EAX){
					if(r1==EDX){
						op66(r32);
						op(0x90+EDX);
						goto sdiv;
					}
					op66(r32);
					op(0x89);
					op(0xC0+EDX+r2*8);	//mov EDX,r2
					op66(r32);
					op(0x89);
					op(0xC0+EAX+r1*8);	//mov EAX,r1
					goto sdiv;
				}
				op66(r32);
				op(0x89);
				op(0xC0+EAX+r1*8);	//mov EAX,r1
			}
			if(r2!=EDX){
				op66(r32);
				op(0x89);
				op(0xC0+EDX+r2*8);	//mov EDX,r2
			}
sdiv:
			CallExternProc("__lludiv");
			addESP-=8;
			goto endmul;
		default: operatorexpected(); break;
	}
	ClearReg(r1);
	ClearReg(r2);
	if(next)nexttok();
	if(terminater==tk_semicolon)seminext();
	if(cpu<3)cpu=3;
}

void optnumadd64(unsigned long long num,int r1,int r2,int vop)
{
int i,reg;
	if(num==0){
		if(vop==0x20){	//&=
			reg=r1;
			for(i=0;i<2;i++){
				ZeroReg(reg,r32);
				reg=r2;
			}
			setzeroflag=TRUE;
		}
		return;		//+= -= |= ^=
	}
	if(num==1){
		if(vop==0x28){	//-=
			op66(r32);
			op(0x48+r1);
			op66(r32);
			op(0x83);
			op(0xD8+r2);
			op(0);
			setzeroflag=TRUE;
			return;
		}
		if(vop==0){	//+=
			op66(r32);
			op(0x40+r1);
			op66(r32);
			op(0x83);
			op(0xD0+r2);
			op(0);
			setzeroflag=TRUE;
			return;
		}
	}
	if(vop==8){	//|=
		if(num<65536){
			if((unsigned short)num<256&&r1<4){
				if(r1==AX)op(0x0C);
				else{
					op(0x80);
					op(0xc8+r1);
				}
				op(num);
				return;
			}
			op66(r16);
			if(r1==AX)op(0x0D);
			else{
				op(0x81);
				op(0xc8+r1);
			}
			outword(num);
			return;
		}
		if(num<0x100000000LL){
			op66(r32);
			if(r1==AX)op(0x0D);
			else{
				op(0x81);
				op(0xc8+r1);
			}
			outdword(num);
			return;
		}
	}
	if(vop==0x20){	//&=
		if(num>=0xFFFFFFFF00000000LL){
			if((unsigned long)num>=0xFFFF0000){
				if((unsigned short)num>=0xFF00&&r1<4){
					if(r1==AL)op(0x24);
					else{
						op(128);
						op(0xE0+r1);
					}
					op(num);
					return;
				}
				op66(r16);
				if(r1==AX)op(0x25);
				else{
					op(129);
					op(0xE0+r1);
				}
				outword(num);
				return;
			}
			op66(r32);
			if(r1==AX)op(0x25);
			else{
				op(129);
				op(0xE0+r1);
			}
			outdword(num);
			return;
		}
	}
	reg=r1;
	int j=0;
	for(i=0;i<2;i++){
		if((unsigned long)num==0)j++;
		if(optnumadd(num,reg,r32,vop)==FALSE){
			op66(r32);
			if(short_ok((unsigned long)num,TRUE)){
				op(0x83);
				op(0xC0+vop+reg);
				op(num);
			}
			else{
				if(reg==EAX)op(5+vop);
				else{
				  op(0x81);
					op(0xC0+vop+reg);
				}
				outdword(num);
			}
		}
		num>>=32;
		if(j==0&&(vop==0||vop==0x28)){
			if(vop)vop=8;
			op66(r32);
			if(short_ok((unsigned long)num,TRUE)){
				op(0x83);
				op(0xD0+vop+r2);
				op(num);
			}
			else{
				if(r2==EAX)op(0x15+vop);
				else{
				  op(0x81);
					op(0xD3+vop+r2);
				}
				outdword(num);
			}
			break;
		}
		reg=r2;
	}
	setzeroflag=TRUE;
}

void doregmath64(int reg)
{
int vop,i,optnum,negflag=FALSE;
int r1,r2,next=1;
int reg1,reg2;
char *ofsstr=NULL;
	r1=reg&255;
	r2=reg/256;
	Select2FreeReg(r1,r2,&reg1,&reg2);
	while(itok.type!=tp_stopper&&tok!=tk_eof){
		if(negflag){
			op66(r32);
			op(0xF7);
			op(0xD8+r2);  // NEG reg
			op66(r32);
			op(0xF7);
			op(0xD8+r1);  // NEG reg
			op66(r32);
			op(0x83);
			op(0xD8+r2);
			op(0);
			ClearReg(r1);
			ClearReg(r2);
			negflag=FALSE;
		}
		vop=0;
		next=1;
		optnum=FALSE;
		if(tok2==tk_number)optnum=OptimNum();
		switch(tok){
			case tk_xor: vop+=0x08;
			case tk_minus: vop+=0x08;
			case tk_and: vop+=0x18;
			case tk_or: vop+=0x08;
			case tk_plus:
			  if(optnum==FALSE)getoperand(reg1);
				else tok=tk_number;
				switch(tok){
					case tk_number:
						if((itok.flag&f_reloc)==0){
							optnumadd64(itok.lnumber,r1,r2,vop);
							break;
						}
					case tk_postnumber:
					case tk_undefofs:
					case tk_apioffset:
						op66(r32);
					  op(0x81);
						op(0xC0+vop+r1);
						if(tok==tk_apioffset)AddApiToPost(itok.number);
						else{
							if(tok==tk_postnumber)(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
							else if(tok==tk_undefofs)AddUndefOff(0,itok.name);
							else if((itok.flag&f_reloc)!=0)AddReloc();
							outdword(itok.number);
						}
						if(vop==0x20){	//&=
							ZeroReg(r2,r32);
						}
						else{
							if(vop==0||vop==0x28){
								if(vop)vop=8;
								op66(r32);
								op(0x83);
								op(0xD0+vop+r2);
								op(0);
							}
						}
						break;
					case tk_rmnumber:
					case tk_charvar:
					case tk_beg:
					case tk_bytevar:
						getintoreg_32(reg2,r32,0,&ofsstr,FALSE);
						op66(r32);
						op(0x01+vop);
						op(0xC0+r1+reg2*8);	/* OPT AX,CX */
						if(vop==0x20){	//&=
							ZeroReg(r2,r32);
						}
						else{
							if(vop==0||vop==0x28){
								if(vop)vop=8;
								op66(r32);
								op(0x83);
								op(0xD0+vop+r2);
								op(0);
							}
						}
						warningreg(regs[r32/2-1][reg2]);
						next=0;
						break;
		  		case tk_reg:
						op66(r32);
						outword(0xB70F);
						if(itok.number==r1||itok.number==r2)itok.number=reg1;
						op(0xC0+itok.number*9);
						warningreg(regs[1][itok.number]);
					case tk_reg32:
defreg32:
						op66(r32);
						op(0x01+vop);
						op(0xC0+r1+(unsigned int)itok.number*8);
						if(vop==0x20){	//&=
							ZeroReg(r2,r32);
						}
						else{
							if(vop==0||vop==0x28){
								if(vop)vop=8;
								op66(r32);
								op(0x83);
								op(0xD0+vop+r2);
								op(0);
							}
						}
						break;
					case tk_reg64:
						int reg2;
						reg=r1;
						reg2=itok.number&255;
						for(i=0;i<2;i++){
							op66(r32);
							op(0x01+vop);
							op(0xC0+reg+reg2*8);
							if(i==1)break;
							if(vop==0)vop=0x10;
							if(vop==0x28)vop=0x18;
							reg2=itok.number/256;
							reg=r2;
						}
						break;
					case tk_longvar:
					case tk_dwordvar:
						CheckAllMassiv(bufrm,4,&strinf);
						op66(r32);
						outseg(&itok,2);
						op(0x03+vop);
						op(r1*8+itok.rm);
						outaddress(&itok);
						if(vop==0x20){	//&=
							ZeroReg(r2,r32);
						}
						else{
							if(vop==0||vop==0x28){
								if(vop)vop=8;
								op66(r32);
								op(0x83);
								op(0xD0+vop+r2);
								op(0);
							}
						}
						break;
					case tk_qwordvar:
						reg=r1;
						for(i=0;i<2;i++){
							CheckAllMassiv(bufrm,8,&strinf);
							op66(r32);
							outseg(&itok,2);
							op(0x03+vop);
							op(reg*8+itok.rm);
							outaddress(&itok);
							if(i==1)break;
							if(vop==0)vop=0x10;
							if(vop==0x28)vop=0x18;
							itok.number+=4;
							compressoffset(&itok);
							reg=r2;
						}
						break;
					case tk_ID:
					case tk_id:
					case tk_proc:
					case tk_apiproc:
					case tk_undefproc:
					case tk_declare:
						procdo(tk_qword);
						op66(r32);
						op(0x01+vop);
						op(0xc0+r1);
						warningreg(regs[1][0]);
						if(vop==0)vop=0x10;
						if(vop==0x28)vop=0x18;
						op66(r32);
						op(0x01+vop);
						op(0xc0+r2+EDX*8);
						warningreg(regs[1][EDX]);
						break;
					default: valueexpected(); break;
				}
				break;
			case tk_rrminus:
				if(r1==ECX||r2==ECX){
					regshifterror();
					break;
				}
				tok=tk_minus;
				goto rshift2;
			case tk_rr:
			  getoperand(ECX);
				if(tok==tk_number){
					op66(r32);
					outword(0xAC0F);
					op(0xC0+r1+r2*8);
					op(itok.number);
					op66(r32);
					if((unsigned int)itok.number==1){
						op(0xD1); op(0xE8+r2);  // SHR reg,1
					}
					else if((unsigned int)itok.number!=0){
						op(0xc1);
						op(0xe8+r2); // SHR reg,imm8
						op((unsigned int)itok.number);
					}
				}
				else if(r1==ECX||r2==ECX)regshifterror();
				else if((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==CL)goto rshift;
				else{
rshift2:
					getintobeg(CL,&ofsstr);
					next=0;
					warningreg(begs[1]);
rshift:
					op66(r32);
					outword(0xAD0F);
					op(0xC0+r1+r2*8);
					op66(r32);
					op(0xD3);
					op(0xE8+r2);	// SHL xXX,CL
				}
				break;
			case tk_llminus:
				if(r1==ECX||r2==ECX){
					regshifterror();
					break;
				}
				tok=tk_minus;
				goto llshift;
			case tk_ll:
			  getoperand(ECX);
				if(tok==tk_number){
					op66(r32);
					outword(0xA40F);
					op(0xC0+r2+r1*8);
					op(itok.number);
					op66(r32);
					if((unsigned int)itok.number==1){
						op(1);
						op(0xC0+r1*9);	//add reg,reg
					}
					else{
						op(0xC1);
						op(0xE0+r1);	//shl ax,num
						op(itok.number);
					}
				}
				else if(r1==ECX||r2==ECX)regshifterror();
				else if((tok==tk_beg||tok==tk_reg||tok==tk_reg32)&&itok.number==CL)goto lshift;
				else{
llshift:
					getintobeg(CL,&ofsstr);
					next=0;
					warningreg(begs[1]);
lshift:
					op66(r32);
					outword(0xA50F);
					op(0xC0+r2+r1*8);
					op66(r32);
					op(0xD3);
					op(0xE0+r1);	// SHL xXX,CL
				}
				break;
			case tk_multminus: negflag=TRUE;
			case tk_mult:
			  getoperand(reg1);
				if(negflag&&tok==tk_number){
					itok.lnumber=-itok.lnumber;
					negflag=FALSE;
				}
				if(tok==tk_number&&((itok.number&f_reloc)==0)){
					if(itok.lnumber==0){
						ZeroReg(r1,r32);
						ZeroReg(r2,r32);
						break;
					}
					if(itok.lnumber==1)break;
					if(itok.lnumber==2){
				 		op66(r32);
						op(1);
						op(0xC0+9*r1); // ADD r1,r1
				 		op66(r32);
						op(0x83);
						op(0xC2+r2*8);	//adc r2,0
						op(0);
						break;
					}
					if((i=caselonglong(itok.lnumber))!=NUMNUM64){
						if(i<32){
							op66(r32);
							outword(0xA40F);
							op(0xC0+r2+r1*8);
							op(i);
							op66(r32);
							op(0xC1);
							op(0xE0+r1);	//shl ax,num
							op(i);
						}
						else{
							op66(r32);
							op(0x89);
							op(0xC0+r2+r1*8);
							i-=32;
							if(i!=0){
								op66(r32);
								if(i==1){
									op(1);
									op(0xC0+r2*9);	//add reg,reg
								}
								else{
									op(0xC1);
									op(0xE0+r2);	//shl ax,num
									op(i);
								}
							}
							ZeroReg(r1,r32);
						}
						break;
					}
					op66(r32);
					op(0x50+r2);
					op66(r32);
					op(0x50+r1);
					addESP+=8;
					MovRegNum(r32,postnumflag&f_reloc,itok.number,ECX);
					MovRegNum(r32,0,itok.lnumber>>32,EAX);
					goto mul;
				}
				//вызов процедуры __llmul
				op66(r32);
				op(0x50+r2);
				op66(r32);
				op(0x50+r1);
				addESP+=8;
				reg=ECX|(EAX*256);
				getintoreg64(reg);
//				doregmath64(reg);
				next=0;
mul:
				CallExternProc("__llmul");
				addESP-=8;
endmul:
				if(r1!=EAX){
					if(r1==EDX){
						if(r2==EAX){
							op66(r32);
							op(0x90+EDX);
							break;
						}
						op66(r32);
						op(0x89);
						op(0xC0+r2+EDX*8);	//mov reg,EDX
						op66(r32);
						op(0x89);
						op(0xC0+r1+EAX*8);	//mov reg,EAX
						break;
					}
					op66(r32);
					op(0x89);
					op(0xC0+r1+EAX*8);	//mov reg,EAX
				}
				if(r2!=EDX){
					op66(r32);
					op(0x89);
					op(0xC0+r2+EDX*8);	//mov reg,EDX
				}
				break;
			case tk_modminus: negflag=TRUE;
			case tk_mod:
				vop=1;
				goto divcalc;;
			case tk_divminus: negflag=TRUE;
			case tk_div:
divcalc:
			  getoperand(reg1);
				if(negflag&&tok==tk_number){
					itok.lnumber=-itok.lnumber;
					negflag=FALSE;
				}
				if(tok==tk_number&&((itok.flag&f_reloc)==0)){
					if(itok.lnumber==0){
						DevideZero();
						break;
					}
					if(itok.lnumber==1){
						if(vop){	//mod
							ZeroReg(r1,r32);
							ZeroReg(r2,r32);
						}
						break;
					}
					if((i=caselonglong(itok.lnumber))!=NUMNUM64){
						if(vop){	//mod
							optnumadd64(itok.lnumber-1,r1,r2,0x20);
						}
						else{
							if(i<32){
								op66(r32);
								outword(0xAC0F);
								op(0xC0+r1+r2*8);
								op(i);
								op66(r32);
								op(0xc1);
								op(0xe8+r2); // SHR reg,imm8
								op(i);
							}
							else{
								op66(r32);
								op(0x89);
								op(0xC0+r1+r2*8);
								i-=32;
								if(i!=0){
									op66(r32);
									if(i==1){
										op(0xD1);
										op(0xE8+r1);	//shr ax,1
									}
									else{
										op(0xC1);
										op(0xE8+r1);	//shr ax,num
										op(i);
									}
								}
								ZeroReg(r2,r32);
							}
						}
						break;
					}
					unsigned long number;
					number=itok.lnumber>>32;
					for(i=0;i<2;i++){
						op66(r32);
						if((itok.flag&f_reloc)==0&&short_ok(number,1)){
							op(0x6A);
							op(number);
						}
						else{
							op(0x68);
							if(i==0&&(itok.flag&f_reloc)!=0)AddReloc();
							outdword(number);
						}
						if(i==1)break;
						number=itok.number;
					}
					addESP+=8;
					goto divcont;
				}
				reg=reg1|(reg2*256);
				getintoreg64(reg);
				op66(r32);
				op(0x50+reg2);
				op66(r32);
				op(0x50+reg1);
				addESP+=8;
				next=0;
divcont:
		 		if(r1!=EAX){
					if(r2==EAX){
						if(r1==EDX){
							op66(r32);
							op(0x90+EDX);
							goto sdiv;
						}
						op66(r32);
						op(0x89);
						op(0xC0+EDX+r2*8);	//mov EDX,r2
						op66(r32);
						op(0x89);
						op(0xC0+EAX+r1*8);	//mov EAX,r1
						goto sdiv;
					}
					op66(r32);
					op(0x89);
					op(0xC0+EAX+r1*8);	//mov EAX,r1
				}
				if(r2!=EDX){
					op66(r32);
					op(0x89);
					op(0xC0+EDX+r2*8);	//mov EDX,r2
				}
sdiv:
				CallExternProc((char*)(vop==0?"__lludiv":"__llumod"));
				addESP-=8;
				goto endmul;
			default: operatorexpected(); break;
		}
		if(next)nexttok();
	}
	ClearReg(r1);
	ClearReg(r2);
	if(cpu<3)cpu=3;
}

void getintoreg64(int reg)
{
int negflag=0,next=1,i=0;
unsigned long long holdnumber=0;
int r1,r2;
int reg1,reg2;
	r1=reg&255;
	r2=reg/256;
	Select2FreeReg(r1,r2,&reg1,&reg2);
	if(tok==tk_minus){
		if(CheckMinusNum()==FALSE){
			negflag=1;
			getoperand(am32==FALSE?BX:r1);
		}
	}
	switch(tok){
		case tk_number:
			holdnumber=CalcNumber(4);
			MovRegNum(r32,postnumflag&f_reloc,holdnumber,r1);
			MovRegNum(r32,0,holdnumber>>32,r2);
			next=0;
			break;
		case tk_postnumber:
		case tk_undefofs:
		case tk_apioffset:
			op66(r32);
			op(0xB8+r1);			/* MOV AX,# */
			if(tok==tk_apioffset)AddApiToPost(itok.number);
			else{
				if(tok==tk_undefofs)AddUndefOff(0,itok.name);
				else (itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
				tok=tk_number;
				holdnumber+=doconstdwordmath();
				outdword(holdnumber);
				MovRegNum(r32,0,holdnumber>>32,r2);
				next=0;
			}
			ClearReg(r1);
			ClearReg(r2);
			break;
		case tk_rmnumber:
			CheckAllMassiv(bufrm,8,&strinf,&itok,reg1,reg2);
			if(!(am32&&itok.rm==r1&&r1!=EBP&&r1!=ESP)){
				op66(r32);
				op67(itok.sib==CODE16?r16:r32);
				if(itok.post==0)outseg(&itok,2);
				op(0x8D);			// LEA reg,[rm]
				op(r1*8+itok.rm);
				if(itok.post!=0&&itok.post!=UNDEF_OFSET){
					if((itok.flag&f_extern)==0){
						unsigned int ooutptr=outptr;
						if(am32&&itok.rm==rm_sib)outptr++;
						setwordpost(&itok);
						outptr=ooutptr;
					}
					else setwordext(&itok.number);
				}
				outaddress(&itok);
				ClearReg(r1);
			}
			ZeroReg(r2,r32);
			break;
		case tk_qwordvar:
			reg=r1;
			for(i=0;i<2;i++){
				if(reg==EAX&&((itok.rm==rm_d16&&itok.sib==CODE16)||(itok.rm==rm_d32&&(itok.sib==CODE32||itok.sib==0)))){
			 		op66(r32);
					outseg(&itok,1);
					op(0xA1);
					if(itok.post==UNDEF_OFSET)AddUndefOff(2,itok.name);
					if(am32==FALSE)outword((unsigned int)itok.number);
					else outdword(itok.number);
				}
				else{
					CheckAllMassiv(bufrm,8,&strinf,&itok,reg1,reg2);
			 		op66(r32);
					outseg(&itok,2);
					op(0x8B);
					op(reg*8+itok.rm);
					outaddress(&itok);
				}
				ClearReg(reg);
				if(i==1)break;
				itok.number+=4;
				compressoffset(&itok);
				reg=r2;
			}
			break;
		case tk_longvar:
		case tk_dwordvar:
			if(reg==EAX&&((itok.rm==rm_d16&&itok.sib==CODE16)||(itok.rm==rm_d32&&(itok.sib==CODE32||itok.sib==0)))){
		 		op66(r32);
				outseg(&itok,1);
				op(0xA1);
				if(itok.post==UNDEF_OFSET)AddUndefOff(2,itok.name);
				if(am32==FALSE)outword((unsigned int)itok.number);
				else outdword(itok.number);
			}
			else{
				CheckAllMassiv(bufrm,4,&strinf,&itok,reg1,reg2);
		 		op66(r32);
				outseg(&itok,2);
				op(0x8B);
				op(r1*8+itok.rm);
				outaddress(&itok);
			}
			ZeroReg(r2,r32);
			ClearReg(r1);
			break;
		case tk_intvar:
		case tk_wordvar:
			CheckAllMassiv(bufrm,2,&strinf,&itok,reg1,reg2);
			if(tok==tk_wordvar&&optimizespeed&&chip>3&&chip<7&&RmEqualReg(reg,itok.rm,itok.sib)==FALSE){
				ZeroReg(r1,r32);
	 			op66(r16);
				outseg(&itok,2);	//mov reg,var
				op(0x8B);
			}
			else{
		 		op66(r32);
				outseg(&itok,3);	//movxx reg,var
				op(0x0F); op(tok==tk_wordvar?0xB7:0xBF);
			}
			op(r1*8+itok.rm);
			outaddress(&itok);
			ClearReg(r1);
			ZeroReg(r2,r32);
			break;
		case tk_bytevar:
		case tk_charvar:
			CheckAllMassiv(bufrm,1,&strinf,&itok,reg1,reg2);
			if(reg<=EBX&&tok==tk_bytevar&&optimizespeed&&chip>3&&chip<7&&RmEqualReg(reg,itok.rm,itok.sib)==FALSE){
				ZeroReg(r1,r32);
				outseg(&itok,2);
				op(0x8A);
			}
			else{
				op66(r32);
				outseg(&itok,3);
				op(0xf);
				if(tok==tk_bytevar)op(0xb6);
				else op(0xbe);
			}
			op(r1*8+itok.rm); // MOVZX regL,[byte]
			outaddress(&itok);
			ClearReg(r1);
			ZeroReg(r2,r32);
			break;
		case tk_reg:
			if(tok2==tk_openbracket){	//вызов процедуры по адресу в регистре
				reg1=itok.number;
				nexttok();
				param[0]=0;
				if(comfile==file_w32)swapparam();
				else doparams();
				op66(r16);
				op(0xFF);
				op(0xD0+reg1); 	/* CALL reg with stack params */
				itok.number=0;
				clearregstat();
#ifdef OPTVARCONST
				FreeGlobalConst();
#endif
			}
			if(optimizespeed&&chip>3&&chip<7&&reg!=(int)itok.number){
				ZeroReg(r1,r32);
				op(0x89);
				op(0xC0+r1+(unsigned int)itok.number*8);
			}
			else{
				op66(r32);
				outword(0xB70F);
				op(0xC0+r1*8+(unsigned int)itok.number);
			}
			RegToReg(r1,itok.number,r32);
			ZeroReg(r2,r32);
			break;
		case tk_reg32:
			if(tok2==tk_openbracket){	//вызов процедуры по адресу в регистре
				reg1=itok.number;
				nexttok();
				param[0]=0;
				if(comfile==file_w32)swapparam();
				else doparams();
				op66(r32);
				op(0xFF);
				op(0xD0+reg1); 	/* CALL reg with stack params */
				itok.number=0;
				clearregstat();
#ifdef OPTVARCONST
				FreeGlobalConst();
#endif
			}
			if(r1!=(int)itok.number){
				op66(r32);
				op(0x89);
				op(0xC0+r1+(unsigned int)itok.number*8);
				RegToReg(r1,itok.number,r32);
			}
			ZeroReg(r2,r32);
			break;
		case tk_reg64:
			reg1=itok.number&255;
			reg2=itok.number/256;
movreg64:
			if(r1==reg2){
				if(r2==reg1){
					op66(r32);
					if(r1==AX)op(0x90+r2);
					else if(r2==AX)op(0x90+r1);
					else{
						op(0x87);
						op(0xC0+r1+r2*8);
					}
					break;
				}
				int temp;
				temp=r2;
				r2=r1;
				r1=temp;
				temp=reg2;
				reg2=reg1;
				reg1=temp;
			}
			if(r2==reg1){
				int temp;
				temp=r2;
				r2=r1;
				r1=temp;
				temp=reg2;
				reg2=reg1;
				reg1=temp;
			}
			if(r1!=reg1){
				op66(r32);
				op(0x89);
				op(0xC0+r1+reg1*8);
				RegToReg(r1,reg1,r32);
			}
			if(r2!=reg2){
				op66(r32);
				op(0x89);
				op(0xC0+r2+reg2*8);
				RegToReg(r2,reg2,r32);
			}
			break;
		case tk_bits:
			int vops;
			i=itok.bit.siz+itok.bit.ofs;
			if(i<=64)vops=r64;
			if(i<=32)vops=r32;
			if(i<=16)vops=r16;
			bits2reg(r1,(r32<vops?vops:r32));
			ZeroReg(r2,r32);
			break;
		case tk_seg:
			op66(r32);
			op(0x8C);
			op(0xC0+r1+(unsigned int)itok.number*8);
			ClearReg(r1);
			ZeroReg(r2,r32);
			break;
		case tk_beg:
			if(optimizespeed&&chip>3&&chip<7&&reg<4&&reg!=(int)(itok.number%4)){
				ZeroReg(r1,r32);
				op(0x88);
				op(0xC0+r1+(unsigned int)itok.number*8); // MOV regL,beg
			}
			else{
				op66(r32);
				outword(0xb60f);
				op(0xC0+r1*8+(unsigned int)itok.number); // MOVZX regL,beg
			}
			ClearReg(reg);
			ZeroReg(r2,r32);
			break;
		case tk_at:
			getoperand(am32==FALSE?BX:reg);
			i++;
		case tk_ID:
		case tk_id:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			if((!i)||macros(tk_qword)==0)procdo(tk_qword);
			reg1=EAX; reg2=EDX;
			goto movreg64;
		case tk_string:
			op66(r32);
			op(0xB8+r1);
			outdword(addpoststring());
			ClearReg(r1);
			ZeroReg(r2,r32);
			break;
		default: valueexpected();	break;
	}
	if(negflag){
		op66(r32);
		op(0xF7);
		op(0xD8+r2);  // NEG reg
		op66(r32);
		op(0xF7);
		op(0xD8+r1);  // NEG reg
		op66(r32);
		op(0x83);
		op(0xD8+r2);
		op(0);
		ClearReg(r1);
		ClearReg(r2);
	}
	if(next)nexttok();
}

void CallExternProc(char *name)
{
ITOK itok4;
int tok4=tk_id;
char string4[256];
struct idrec *ptrs;
	memset(&itok4,0,sizeof(ITOK));
	strcpy(string4,name);
	searchtree(&itok4,&tok4,(unsigned char *)string4);
	ptrs=itok4.rec;
	switch(tok4){
		case tk_id:
			tok4=tok;
			itok4=itok;
			strcpy((char *)itok.name,string4);
			string[0]=0;
			itok.flag=tp_stdcall;
			tok=tk_undefproc;
			itok.number=secondcallnum;
			itok.segm=NOT_DYNAMIC;
			itok.rm=tk_qword;
			itok.post=0;
			addtotree(itok.name);
			addacall(secondcallnum++,CALL_NEAR);
			tok=tok4;
			itok=itok4;
			callloc0();
			break;
		case tk_declare:
			ptrs->rectok=tk_undefproc;
		case tk_undefproc:
			addacall(itok4.number,(unsigned char)(am32!=FALSE?CALL_32:CALL_NEAR));
			callloc0();		/* produce CALL [#] */
			break;
		case tk_proc:
			if(itok4.segm==DYNAMIC)itok4.segm=ptrs->recsegm=DYNAMIC_USED;
			if(itok4.segm<NOT_DYNAMIC){	//динамическая процедура
				addacall(itok4.number,(unsigned char)(am32!=FALSE?CALL_32:CALL_NEAR));
				callloc0();
			}
			else{
				callloc(itok4.number);
			}
			break;
		default:
			sprintf(string4,"'%s' already used",name);
			preerror(string4);
			break;
	}
}
/* end of TOKB.C */
