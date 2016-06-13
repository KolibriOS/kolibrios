#define _REGOPT_

#include "tok.h"

unsigned char optinitreg=TRUE;
int OptNameIDX(char *name,char *nam,int size);
int getnumber(char *buf,unsigned long *num,int *typenum=NULL);
int convertrazr(int *reg,int razr);

enum{
	t_undef,
	t_const,
	t_string,
	t_id,
};

enum{
	razr_8l,
	razr_8h,
	razr_16,
	razr_32
};

REGISTERSTAT *regstat;

#ifdef OPTVARCONST
void CompareLVIC(BLVIC *bak);
void FreeLVIC(BLVIC *bak);
BLVIC *BakLVIC();
void CopyLVIC(BLVIC *bak);
#endif

void KillRegLec(int reg)
{
	KillVar(regs[0][reg]);
	KillVar(regs[1][reg]);
	if(reg<4){
		KillVar(begs[reg]);
		KillVar(begs[reg+4]);
	}
}

void ClearReg(int reg)
{
	if(optinitreg==FALSE)return;
	(regstat+reg)->type=t_undef;
REGEQVAR *curv,*nextv;
	curv=(regstat+reg)->next;
	while(curv){
		nextv=curv->next;
		free(curv);
		curv=nextv;
	}
	(regstat+reg)->next=NULL;
//	printf("clear reg %d\n",reg);
	KillRegLec(reg);
}

void clearregstat(int regs)
{
	if(optinitreg==FALSE)return;
	for(int i=0;i<8;i++){
		if((regs&(1<<i))==0){
			(regstat+i)->type=t_undef;
REGEQVAR *curv,*nextv;
			curv=(regstat+i)->next;
			while(curv){
				nextv=curv->next;
				free(curv);
				curv=nextv;
			}
			(regstat+i)->next=NULL;
		}
	}
}

void initregstat()
{
	regstat=(REGISTERSTAT *)MALLOC(sizeof(REGISTERSTAT)*8);
	for(int i=0;i<8;i++)(regstat+i)->next=NULL;
	clearregstat();
}

void deinitregstat()
{
	if(regstat){
		clearregstat();
		free(regstat);
	}
	regstat=NULL;
}

REGISTERSTAT *BakRegStat()
{
REGISTERSTAT *bakregstat;
#ifndef OPTVARCONST
	if(optinitreg==FALSE)return NULL;
	bakregstat=(REGISTERSTAT *)MALLOC(sizeof(REGISTERSTAT)*8);
#else
	bakregstat=(REGISTERSTAT *)MALLOC(sizeof(REGISTERSTAT)*8+sizeof(BLVIC *));
	(bakregstat+8)->bakvic=BakLVIC();
#endif
	memcpy(bakregstat,regstat,sizeof(REGISTERSTAT)*8);
REGEQVAR *cur,*news;
	for(int i=0;i<8;i++){
		if((bakregstat+i)->next){
			cur=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
			memcpy(cur,(bakregstat+i)->next,sizeof(REGEQVAR));
			(bakregstat+i)->next=cur;
			while(cur->next){
				news=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
				memcpy(news,cur->next,sizeof(REGEQVAR));
				cur->next=news->next;
			}
		}
	}
	return bakregstat;
}

void CopyRegStat(REGISTERSTAT *bak)
{
#ifndef OPTVARCONST
	if(optinitreg==FALSE||bak==NULL)return;
#else
	if(bak==NULL)return;
	CopyLVIC((bak+8)->bakvic);
#endif
	memcpy(regstat,bak,sizeof(REGISTERSTAT)*8);
REGEQVAR *cur,*news;
	for(int i=0;i<8;i++){
		if((regstat+i)->next){
			cur=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
			memcpy(cur,(regstat+i)->next,sizeof(REGEQVAR));
			(regstat+i)->next=cur;
			while(cur->next){
				news=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
				memcpy(news,cur->next,sizeof(REGEQVAR));
				cur->next=news->next;
			}
		}
	}
}

void ClearRegBak(int reg,REGISTERSTAT *regst)
{
REGEQVAR *curv,*nextv;
	(regst+reg)->type=t_undef;
	curv=(regst+reg)->next;
	while(curv){
		nextv=curv->next;
		free(curv);
		curv=nextv;
	}
	(regst+reg)->next=NULL;
}

void AddRegVar(int reg, int razr,ITOK *itok4)
{
	if(optinitreg==FALSE||(itok4->flag&f_useidx)!=0)return;
REGEQVAR *cur,*news;
	cur=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
	cur->razr=convertrazr(&reg,razr);
	cur->next=NULL;
	strcpy(cur->name,itok4->name);
	if((regstat+reg)->next==NULL)(regstat+reg)->next=cur;
	else{
		news=(regstat+reg)->next;
		while(news->next)news=news->next;
		news->next=cur;
	}
}

int GetRegVar(ITOK *itok4)
{
	if(optinitreg==FALSE||(itok4->flag&f_useidx)!=0)return 0;
int retval=0;
	for(int i=0;i<8;i++){
REGEQVAR *cur;
		cur=(regstat+i)->next;
		while(cur){
			if(strcmp(itok4->name,cur->name)==0){
				retval|=(1<<i);
				break;
			}
			cur=cur->next;
		}
	}
	return retval;
}

void FreeStat(REGISTERSTAT *bak)
{
#ifndef OPTVARCONST
	if(optinitreg==FALSE||bak==NULL)return;
#else
	if(bak==NULL)return;
	FreeLVIC((bak+8)->bakvic);
#endif
	for(int i=0;i<8;i++)ClearRegBak(i,bak);
	free(bak);
}

void CompareRegStat(REGISTERSTAT *bak)
{
#ifndef OPTVARCONST
	if(optinitreg==FALSE||bak==NULL)return;
#else
	if(bak==NULL)return;
	CompareLVIC((bak+8)->bakvic);
#endif
	for(int i=0;i<8;i++){
		if((regstat+i)->type==t_undef||(regstat+i)->type!=(bak+i)->type||
				((regstat+i)->type==t_id&&strcmp((regstat+i)->id,(bak+i)->id)!=0))ClearRegBak(i,bak);
	}
}

int convertrazr(int *reg,int razr)
{
int nr;
	if(razr==r8){
		if(*reg<4)nr=razr_8l;
		else{
			nr=razr_8h;
			*reg-=4;
		}
	}
	else if(razr==r16)nr=razr_16;
	else nr=razr_32;
	return nr;
}

void ConstToReg(unsigned long num,int reg,int razr)
{
char buf[64];
	if(optinitreg){
		sprintf(buf,"%ld",num);
		IDZToReg(buf,reg,razr);
	}
}

void GenRegToReg(int regd,int regs,int razr)
{
	switch(razr){
		case r8:
			op(0x88);
			op(128+64+regs*8+regd);
			break;
		case r16:
		case r32:
			op66(razr);
			op(0x89);
			op(128+64+regs*8+regd);
			break;
	}
}

int CheckIDZReg(char *name,int reg,int razr)
/*
 name - адрес строки
 */
{
int nr;
int retreg=NOINREG;
int i;
	if(optinitreg==FALSE||razr>r32)return NOINREG;
	nr=convertrazr(&reg,razr);
	for(i=0;i<8;i++){
//printf("%d type=%d razr=%d %s\n",i,(regstat+i)->type,(regstat+i)->razr,(regstat+i)->id);
		if((regstat+i)->type==t_id){
			if((regstat+i)->razr==nr||(razr==r8&&((regstat+i)->razr==razr_8l||
					(regstat+i)->razr==razr_8h))){
				if(strcmp(name,(regstat+i)->id)==0){
					if(i==reg&&nr==(regstat+i)->razr){
						if(/*razr==r8&&*/nr==razr_8h)reg+=4;
						waralreadinit(razr==r8?begs[reg]:regs[razr/4][reg]);
//					printstatreg();
						return SKIPREG;
					}
					retreg=i;
				}
			}
		}
REGEQVAR *cur;
		cur=(regstat+i)->next;
		while(cur){
			if(cur->razr==nr||(razr==r8&&(cur->razr==razr_8l||cur->razr==razr_8h))){
				if(strcmp(name,cur->name)==0){
					if(i==reg&&nr==cur->razr){
						if(nr==razr_8h)reg+=4;
						waralreadinit(razr==r8?begs[reg]:regs[razr/4][reg]);
						return SKIPREG;
					}
					retreg=i;
					break;
				}
			}
			cur=cur->next;
		}
	}
	if(retreg!=NOINREG){
		if(razr==r8){
			if((regstat+retreg)->razr==razr_8h)retreg+=4;
			if(nr==razr_8h)reg+=4;
			waralreadinitreg(begs[reg],begs[retreg]);
		}
		else waralreadinitreg(regs[razr/4][reg],regs[razr/4][retreg]);
	}
	return retreg;
}

void IDZToReg(char *name,int reg,int razr)
/*
 name - адрес строки
 */
{
int nr;
	if(optinitreg==FALSE||name==NULL||razr>r32)return;
	nr=convertrazr(&reg,razr);
	ClearReg(reg);
	(regstat+reg)->type=t_id;
	(regstat+reg)->razr=nr;
	strcpy((regstat+reg)->id,name);
	KillRegLec(reg);
}

int GetRegNumber(int reg,unsigned long *num,int razr)
{
int nr;
	nr=convertrazr(&reg,razr);
	if(optinitreg!=FALSE&&razr<=r32&&(regstat+reg)->type==t_id&&
			(regstat+reg)->razr==nr){
		if(isdigit((regstat+reg)->id[0])){
			if(getnumber((regstat+reg)->id,num))return reg;
		}
	}
	return NOINREG;
}

int GetNumberR(int sreg,unsigned long *num,int razr,unsigned long number)
{
int nr;
int reg,rreg=NOINREG;
unsigned long dnum=0xffffffff;
unsigned long nnum;
	if(optinitreg!=FALSE&&razr<=r32){
		for(reg=0;reg<8;reg++){
			if(reg==sreg)continue;
			nr=convertrazr(&reg,razr);
			if((regstat+reg)->type==t_id&&(regstat+reg)->razr==nr){
//				printf("reg=%d %s\n",reg,(regstat+reg)->id);
				if(isdigit((regstat+reg)->id[0])){
					if(getnumber((regstat+reg)->id,num)){
						if(*num==number)return reg;
						if(*num>number&&(*num-number)<dnum){
							dnum=*num-number;
							nnum=*num;
							rreg=reg;
						}
						else if((number-*num)<dnum){
							dnum=number-*num;
							nnum=*num;
							rreg=reg;
						}
					}
				}
			}
		}
		*num=nnum;
	}
	return rreg;
}

int getnumber(char *buf,unsigned long *num,int *typenum)
{
int temp2;
unsigned char *oinput;
unsigned int oinptr,oendinptr;
int retcode=FALSE;
	oinptr=inptr;
	oinput=input;
	oendinptr=endinptr;
	input = (unsigned char*)buf;
	inptr=0;
	endinptr=256;
	*num=scannumber(typenum==NULL?&temp2:typenum);
	if(cha==0)retcode=TRUE;
	inptr=oinptr;
	input=oinput;
	endinptr=oendinptr;
	return retcode;
}

char *GetLecsem(int stop1,int stop2,int type)
{
int oinptr,oinptr2;
char ocha;
int oline;
char nam[SIZEIDREG];
int i,j;
ITOK oitok;
int otok;
char c;
int pinptr;
	switch(tok2){
		case tk_string:
		case tk_proc:
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			return NULL;
	}
	oinptr=inptr;
	oinptr2=inptr=inptr2;
	ocha=cha;
	cha=cha2;
	oline=linenumber;
	otok=tok;
	oitok=itok;
	i=0;
	do{
		pinptr=inptr;
		FastTok(0);
		while(tok==tk_closebracket&&i){
			i--;
			pinptr=inptr;
			FastTok(0);
		}
		if(tok==tk_openbracket)i++;
	}while(tok!=stop1&&tok!=stop2&&tok!=tk_eof&&itok.type!=type);
	if(pinptr)pinptr--;
	i=0;
	if(cha2>' '){
		nam[0]=cha2;
		i++;
	}
	for(j=0;i<SIZEIDREG;i++){
		do{
			c=input[j+oinptr2];
			j++;
		}while(c<=0x20);
		if((oinptr2+j)>pinptr)break;
		nam[i]=c;
	}
	inptr2=oinptr2;
	inptr=oinptr;
	cha=ocha;
	linenumber=oline;
	tok=otok;
	itok=oitok;
	if(i==SIZEIDREG)return NULL;
	c=nam[0];
	if(c==';'||c==','||c==')'||c=='}')return NULL;
	nam[i]=0;
	return BackString(nam);
}

void GetEndLex(int stop1,int stop2,int type)
{
int i=0;
int oscanlexmode;
	if(bufrm){
		free(bufrm);
		bufrm=NULL;
	}
	if(strinf.bufstr){
		free(strinf.bufstr);
		strinf.bufstr=NULL;
	}
	oscanlexmode=scanlexmode;
	scanlexmode=DEFLEX;
	while(tok2!=stop1&&tok2!=stop2&&tok2!=tk_eof&&itok2.type!=type){
		nexttok();
		while(tok==tk_closebracket&&i){
			i--;
			nexttok();
		}
		if(tok==tk_openbracket)i++;
		if(bufrm){
			free(bufrm);
			bufrm=NULL;
		}
		if(strinf.bufstr){
			free(strinf.bufstr);
			strinf.bufstr=NULL;
		}
	}
//	retoldscanmode(oscanlexmode);
	scanlexmode=oscanlexmode;
//	printf("tok2=%d type2=%d\n",tok2,itok2.type);
}

void IDXToReg(char *name,int size,int reg)
/*
 name - адрес начала
 size - размерность
 */
{
int nr;
char nam[SIZEIDREG];
	if(optinitreg==FALSE)return;
	nr=(am32==0?razr_16:razr_32);
	ClearReg(reg);
	if(OptNameIDX(name,nam,size)==TRUE){
		(regstat+reg)->type=t_id;
		(regstat+reg)->razr=nr;
		strcpy((regstat+reg)->id,nam);
	}
	KillRegLec(reg);
}

int OptNameIDX(char *name,char *nam,int size)
{
int i,j;
int maxlen=SIZEIDREG;
	j=0;
	if(name[j]=='&'){
		size=1;
		j++;
	}
	if(size>1)maxlen-=10;
	for(i=0;;i++){
		if(i==maxlen)return FALSE;
		char c;
		do{
			c=name[j];
			j++;
		}while(c<=0x20&&c!=0);
		if(c==0||c==';')break;
		nam[i]=c;
	}
	nam[i]=0;
	if(size>1)sprintf(nam+i,"*%d",size);
	return TRUE;
}

int CheckIDXReg(char *name,int size,int reg)
/*
 name - адрес начала
 size - размерность
 */
{
int nr;
char nam[SIZEIDREG];
int retreg=NOINREG;
int i;
	if(optinitreg==FALSE)return NOINREG;
	nr=(am32==0?razr_16:razr_32);
	if(OptNameIDX(name,nam,size)==TRUE){
		for(i=0;i<8;i++){
			if((regstat+i)->type==t_id&&(regstat+i)->razr==nr){
				if(strcmp(nam,(regstat+i)->id)==0){
					if(i==reg){
//					printf("%s %s\n",nam,(regstat+i)->id);
		 				waralreadinit(regs[am32][reg]);
						return SKIPREG;
					}
					retreg=i;
				}
			}
REGEQVAR *cur;
			cur=(regstat+i)->next;
			while(cur){
				if(cur->razr==nr&&strcmp(nam,cur->name)==0){
					if(i==reg){
		 				waralreadinit(regs[am32][reg]);
						return SKIPREG;
					}
					retreg=i;
					break;
				}
				cur=cur->next;
			}
		}
	}
	return retreg;
}

int RegToReg(int regd,int regs,int razr)
{
int nr,nrs;
	if(optinitreg==FALSE)return NOINREG;
	nrs=convertrazr(&regs,razr);
	nr=convertrazr(&regd,razr);
	if(razr==r8&&nrs!=nr&&regs==regd)goto noreg;
	if(nrs==nr&&(regstat+regd)->type==(regstat+regs)->type&&
			(regstat+regs)->type!=t_undef&&(regstat+regd)->razr==(regstat+regs)->razr){
		if(strcmp((regstat+regd)->id,(regstat+regs)->id)==0)return SKIPREG;
	}
noreg:
	ClearReg(regd);
	memcpy((regstat+regd),(regstat+regs),sizeof(REGISTERSTAT));
REGEQVAR *cur,*news;
 	if((regstat+regd)->next){
		cur=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
		memcpy(cur,(regstat+regd)->next,sizeof(REGEQVAR));
		(regstat+regd)->next=cur;
		while(cur->next){
			news=(REGEQVAR *)MALLOC(sizeof(REGEQVAR));
			memcpy(news,cur->next,sizeof(REGEQVAR));
			cur->next=news->next;
		}
	}
	(regstat+regd)->razr=nr;
	return NOINREG;;
}

int RegSwapReg(int reg1,int reg2,int razr)
{
int nr,nrs;
REGISTERSTAT temp;
	if(optinitreg==FALSE)return NOINREG;
	nrs=convertrazr(&reg2,razr);
	nr=convertrazr(&reg1,razr);
	if((regstat+reg1)->type&&(regstat+reg1)->razr>nr)ClearReg(reg1);
	if((regstat+reg2)->type&&(regstat+reg2)->razr>nrs)ClearReg(reg2);
	if((regstat+reg1)->razr<razr_16&&nrs>=razr_16)ClearReg(reg1);
	if((regstat+reg2)->razr<razr_16&&nr>=razr_16)ClearReg(reg2);
	if(razr==r8&&nrs!=nr&&reg1==reg2)goto noreg;
	if((regstat+reg1)->type==(regstat+reg2)->type&&
			(regstat+reg2)->type!=r_undef&&(regstat+reg1)->razr==(regstat+reg2)->razr){
		if(strcmp((regstat+reg1)->id,(regstat+reg2)->id)==0)return SKIPREG;
	}
noreg:
	KillRegLec(reg1);
	KillRegLec(reg2);
	memcpy(&temp,(regstat+reg1),sizeof(REGISTERSTAT));
	memcpy((regstat+reg1),(regstat+reg2),sizeof(REGISTERSTAT));
	memcpy((regstat+reg2),&temp,sizeof(REGISTERSTAT));
	return NOINREG;;
}

void KillVar(char *name)
/*-----------------22.04.03 15:51-------------------
 поиск переменной в регистрах и если найдена, то
 убрать инициализацию регистра
--------------------------------------------------*/
{
char *pstr,*nam;
int len;
	if(optinitreg==FALSE)return;
	len=strlen(name);
	for(int i=0;i<8;i++){
		if((regstat+i)->type==t_id){
 			pstr=(regstat+i)->id;
			for(;;){
				if((nam=strstr(pstr,name))!=NULL){
					char c;
					c=nam[len];
					if(c!='_'&&(!isalpha(c))){
						if(nam!=pstr){
							c=nam[-1];
							if(c=='_'||isalpha(c))goto novar;
						}
 						ClearReg(i);
						break;
					}
				}
				else break;
novar:
				pstr=nam+1;
			}
		}
restart:
		if((regstat+i)->next){
REGEQVAR *cur,*prev;
			cur=(regstat+i)->next;
			if(strcmp(cur->name,name)==0){
				(regstat+i)->next=cur->next;
//				printf("kill '%s'\n",name);
				free(cur);
				goto restart;
			}
			prev=cur;
			cur=cur->next;
			while(cur){
				if(strcmp(cur->name,name)==0){
					prev->next=cur->next;
					free(cur);
					cur=prev->next;
				}
				else{
					prev=cur;
					cur=cur->next;
				}
			}
		}
	}
}

/*-----------------16.06.05 23:45-------------------
 Замена переменных константами
	--------------------------------------------------*/
#ifdef OPTVARCONST

BLVIC *mainvic;
unsigned char replasevar=TRUE;
//void updnum(int tok4,long long *num);

#define MAXSIZEVIC 64
int cursizevic=0;
LVIC *listvic=NULL;

void CreateMainLVIC()
{
	mainvic=(BLVIC *)MALLOC(sizeof(BLVIC));
	listvic=mainvic->listvic=(LVIC *)MALLOC(sizeof(LVIC)*MAXSIZEVIC);
	cursizevic=mainvic->sizevic=MAXSIZEVIC;
}

void KillMainLVIC()
{
	free(listvic);
	free(mainvic);
	listvic=NULL;
}

BLVIC *BakLVIC()
{
BLVIC *bakvic;
	if(replasevar==0)return NULL;
	bakvic=(BLVIC *)MALLOC(sizeof(BLVIC));
	bakvic->listvic=(LVIC *)MALLOC(sizeof(LVIC)*cursizevic);
	bakvic->sizevic=cursizevic;
	memcpy(bakvic->listvic,listvic,sizeof(LVIC)*cursizevic);
	return bakvic;
}

void CopyLVIC(BLVIC *bak)
{
	if(bak==NULL)return;
	if(cursizevic<bak->sizevic){
		listvic=(LVIC *)REALLOC(listvic,sizeof(LVIC)*bak->sizevic);
	}
	memcpy(listvic,bak->listvic,sizeof(LVIC)*bak->sizevic);
	if(cursizevic>bak->sizevic){
		for(int i=bak->sizevic;i<cursizevic;i++)(listvic+i)->rec=NULL;
	}
	else cursizevic=bak->sizevic;
}

void FreeLVIC(BLVIC *bak)
{
	if(bak==NULL)return;
	free(bak->listvic);
	free(bak);
}

void CompareLVIC(BLVIC *bak)
{
LVIC *bvic;
	if(bak==NULL)return;
	for(int i=0;i<bak->sizevic;i++){
		if((bak->listvic+i)->rec != NULL){
			bvic=bak->listvic+i;
			int j;
			for(j=0;j<cursizevic;j++){
				if((listvic+j)->rec==bvic->rec){
					if((listvic+j)->lnumber!=bvic->lnumber)bvic->rec=NULL;
					break;
				}
			}
			if(j==cursizevic)bvic->rec=NULL;
		}
	}
}

void ClearLVIC()
{
	memset(listvic,0,cursizevic*sizeof(LVIC));
/*	printf("%s(%d)> Init table LVIC\n",
		startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,
		linenumber);*/
}

int Const2Var(ITOK *itok4,long long num,int typenum)
{
int i;
int freevic=-1;
	if(replasevar&&itok4->rec&&(itok4->flag&f_useidx)==0){
		if((typenum==tk_word||typenum==tk_dword)&&num<256)typenum=tk_byte;
		for(i=0;i<cursizevic;i++){
			if((listvic+i)->rec==itok4->rec)break;
			if(freevic==-1&&(listvic+i)->rec==NULL)freevic=i;
		}
		if(i==cursizevic){
			if(freevic==-1){
				cursizevic+=MAXSIZEVIC;
				listvic=(LVIC *)REALLOC(listvic,sizeof(LVIC)*cursizevic);
				for(freevic=cursizevic-1;freevic>i;freevic--)(listvic+freevic)->rec=NULL;
			}
			else i=freevic;
		}
		else if((listvic+i)->contype==typenum&&(listvic+i)->lnumber==num)return FALSE;
		(listvic+i)->rec=itok4->rec;
		(listvic+i)->contype=typenum;
		(listvic+i)->lnumber=num;
		switch(itok4->type){
			case tp_localvar:
			case tp_paramvar:
				(listvic+i)->typevar=tp_localvar;
				break;
			case tp_postvar:
			case tp_gvar:
				(listvic+i)->typevar=tp_gvar;
				break;
			default:
				(listvic+i)->typevar=itok4->type;
				break;
		}
/*		printf("%s(%d)> %s var '%s' by num=%u type=%d\n",
			startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,
			linenumber,
			freevic==-1?"Update":"Add",
			itok4->name,
			(listvic+i)->number,
			typenum);*/
	}
	return TRUE;
}

void Const2VarRec(LVIC *varconst)
{
int i;
int freevic=-1;
	if(replasevar==0)return;
	for(i=0;i<cursizevic;i++){
		if((listvic+i)->rec==varconst->rec)break;
		if(freevic==-1&&(listvic+i)->rec==NULL)freevic=i;
	}
	if(i==cursizevic){
		if(freevic==-1){
			cursizevic+=MAXSIZEVIC;
			listvic=(LVIC *)REALLOC(listvic,sizeof(LVIC)*cursizevic);
			for(freevic=cursizevic-1;freevic>i;freevic--)(listvic+freevic)->rec=NULL;
		}
		else i=freevic;
	}
	(listvic+i)->rec=varconst->rec;
	(listvic+i)->contype=varconst->contype;
	(listvic+i)->lnumber=varconst->lnumber;
	switch(varconst->rec->type){
		case tp_localvar:
		case tp_paramvar:
			(listvic+i)->typevar=tp_localvar;
			break;
		case tp_postvar:
		case tp_gvar:
			(listvic+i)->typevar=tp_gvar;
			break;
		default:
			(listvic+i)->typevar=varconst->rec->type;
			break;
	}
}

void ClearVarByNum(ITOK *itok4)
{
	if(replasevar&&itok4->rec&&(itok4->flag&f_useidx)==0){
		for(int i=0;i<cursizevic;i++){
			if((listvic+i)->rec==itok4->rec){
				(listvic+i)->rec=NULL;
/*				printf("%s(%d)> Clear by constant var '%s'\n",
					startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,
					linenumber,
					itok4->name);*/
				break;
			}
		}
	}
}

int UpdVarConst(ITOK *itok4,long long num,int typenum,int operand)
{
int retcode=FALSE;
int i;
	if(replasevar==0||itok4->rec==NULL||(itok4->flag&f_useidx)!=0)return TRUE;
	for(i=0;i<cursizevic;i++){
		if((listvic+i)->rec==itok4->rec){
//			printf("%s(%d)> Modif var '%s' by num=%u ",startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,linenumber,itok4->name,(listvic+i)->number);
			switch(operand){
				case tk_plusequals:
				case tk_plusplus: operand=tk_plus; break;
				case tk_minusequals:
				case tk_minusminus: operand=tk_minus; break;
				case tk_xorequals: operand=tk_xor; break;
				case tk_andequals: operand=tk_and; break;
				case tk_orequals: operand=tk_or; break;
				case tk_multequals: operand=tk_mult; break;
				case tk_divequals: operand=tk_div; break;
				case tk_rrequals: operand=tk_rr; break;
				case tk_llequals: operand=tk_ll; break;
				case tk_numsign:
					if((listvic+i)->contype==tk_float)(listvic+i)->number|=0x80000000;
					else if((listvic+i)->contype==tk_double)(listvic+i)->lnumber|=0x8000000000000000LL; 
					else (listvic+i)->lnumber=-(listvic+i)->lnumber;
					return TRUE;;
				case tk_not:
					(listvic+i)->lnumber=~(listvic+i)->lnumber;
					return TRUE;;
			}
			switch((listvic+i)->contype){
				case tk_char:
				case tk_int:
				case tk_long:
					retcode=calclongnumber(&(listvic+i)->number,num,operand);
					break;
				case tk_byte:
				case tk_word:
				case tk_dword:
					retcode=calcdwordnumber((unsigned long *)&(listvic+i)->number,num,operand);
					break;
				case tk_qword:
					retcode=calcqwordnumber((unsigned long long *)&(listvic+i)->lnumber,num,operand);
					break;
				case tk_float:
					retcode=calcfloatnumber(&(listvic+i)->fnumber,*(float *) &num,operand);
					break;
				case tk_double:
					retcode=calcdoublenumber(&(listvic+i)->dnumber,*(double *) &num,operand);
					break;
			}
/*			if(retcode)printf("new num=%u\n",(listvic+i)->number);
			else{
				puts("error");
				(listvic+i)->rec=NULL;
			}                       */
			break;
		}
	}
	return TRUE;
}

void FreeGlobalConst()
{
	if(replasevar==0)return;
	for(int i=0;i<cursizevic;i++){
		if((listvic+i)->rec!=NULL&&(listvic+i)->typevar!=tp_localvar){
			(listvic+i)->rec=NULL;
/*			printf("%s(%d)> Clear global constant var\n",
					startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,
					linenumber);*/
		}
	}
}

int CheckRegToConst(int reg,ITOK *itok4,int razr)
{
unsigned long num;
int typenum;
int nr;
	nr=convertrazr(&reg,razr);
	if(replasevar&&optinitreg!=FALSE&&razr<=r32&&(regstat+reg)->type==t_id&&
			(regstat+reg)->razr>=nr){
		if(isdigit((regstat+reg)->id[0])){
			if(getnumber((regstat+reg)->id,&num,&typenum)){
				Const2Var(itok4,num,typenum);
				return TRUE;
			}
		}
	}
	return FALSE;
}

int CheckUpdRegToConst(int reg,ITOK *itok4,int operand,int razr)
{
unsigned long num;
int typenum;
int nr;
	nr=convertrazr(&reg,razr);
	if(replasevar&&optinitreg!=FALSE&&razr<=r32&&(regstat+reg)->type==t_id&&
			(regstat+reg)->razr>=nr){
		if(isdigit((regstat+reg)->id[0])){
			if(getnumber((regstat+reg)->id,&num,&typenum))return UpdVarConst(itok4,num,typenum,operand);
		}
	}
	return FALSE;
}

int SwapVarConst(ITOK *itok2,ITOK *itok4)
{
int i;
int fir=-1,sec=-1;
LVIC tempvic;
	if(replasevar==0)return TRUE;
	if(itok4->rec==NULL||(itok4->flag&f_useidx)!=0)sec=-2;
	if(itok2->rec==NULL||(itok2->flag&f_useidx)!=0){
		if(sec==-2)return TRUE;
		fir=-2;
	}
	for(i=0;i<cursizevic;i++){
		if(sec==-1&&(listvic+i)->rec==itok4->rec)sec=i;
		else if(fir==-1&&(listvic+i)->rec==itok2->rec)fir=i;
	}
	if(sec==-2)sec=-1;
	if(fir==-2)fir=-1;
	if(sec!=-1&&fir!=-1){
		memcpy(&tempvic,listvic+fir,sizeof(LVIC));
		memcpy(listvic+fir,listvic+sec,sizeof(LVIC));
		memcpy(listvic+sec,&tempvic,sizeof(LVIC));
	}
	else if(sec==-1&&fir!=-1)(listvic+fir)->rec=itok4->rec;
	else if(fir==-1&&sec!=-1)(listvic+sec)->rec=itok2->rec;
	return TRUE;
}

int SwapVarRegConst(int reg,ITOK *itok4,int razr)
{
int i;
int freevic=-1;
unsigned long num;
int typenum;
int nr;
int numinreg=FALSE;
	nr=convertrazr(&reg,razr);
	if(replasevar&&itok4->rec&&(itok4->flag&f_useidx)==0&&optinitreg!=FALSE&&razr<=r32&&
			(regstat+reg)->type==t_id&&(regstat+reg)->razr>=nr){
		for(i=0;i<cursizevic;i++){
			if((listvic+i)->rec==itok4->rec)break;
			if(freevic==-1&&(listvic+i)->rec==NULL)freevic=i;
		}
		if(isdigit((regstat+reg)->id[0])){
			if(getnumber((regstat+reg)->id,&num,&typenum)){	//цифра в регистре
				if(i!=cursizevic)ConstToReg((listvic+i)->lnumber,reg,razr);	//новую в регистр
				else{
					if(freevic==-1){
						cursizevic+=MAXSIZEVIC;
						listvic=(LVIC *)REALLOC(listvic,sizeof(LVIC)*cursizevic);
						for(freevic=cursizevic-1;freevic>i;freevic--)(listvic+freevic)->rec=NULL;
					}
					else i=freevic;
				}
				(listvic+i)->rec=itok4->rec;	//цифру в переменную
				(listvic+i)->contype=typenum;
				(listvic+i)->lnumber=num;
				switch(itok4->type){
					case tp_localvar:
					case tp_paramvar:
						(listvic+i)->typevar=tp_localvar;
						break;
					case tp_postvar:
					case tp_gvar:
						(listvic+i)->typevar=tp_gvar;
						break;
					default:
						(listvic+i)->typevar=itok4->type;
						break;
				}
/*				printf("%s(%d)> %s var '%s' by num=%u type=%d\n",
					startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,
					linenumber,
					freevic==-1?"Update":"Add",
					itok4->name,
					(listvic+i)->number,
					typenum);*/
			}
		}
		else{	//нет цифры в регистре
			if(i!=cursizevic){	//цифра в переменной
				ConstToReg((listvic+i)->lnumber,reg,razr);	//теперь в регистре
				(listvic+i)->rec=NULL;	//а впеременной уже нет
			}
		}
	}
	return TRUE;
}

int CheckConstVar(ITOK *itok4)
{
	if(replasevar==FALSE||itok4->rec==NULL||(itok4->flag&f_useidx)!=0)return FALSE;
	if(itok4->rec&&(itok4->flag&f_useidx)==0){
		for(int i=0;i<cursizevic;i++){
			if((listvic+i)->rec==itok4->rec){
				itok4->lnumber=(listvic+i)->lnumber;
				itok4->rm=(listvic+i)->contype;
				itok4->flag=0;
				warreplasevar(itok4->name);
				return TRUE;

			}
		}
	}
	return FALSE;
}

int CheckConstVar2(ITOK *itok4,long long *num,int *typenum)
{
//	if(replasevar==FALSE||itok4->rec==NULL||(itok4->flag&f_useidx)!=0)return FALSE;
	if(replasevar&&itok4->rec&&(itok4->flag&f_useidx)==0){
		for(int i=0;i<cursizevic;i++){
			if((listvic+i)->rec==itok4->rec){
				*num=(listvic+i)->lnumber;
				*typenum=(listvic+i)->contype;
//				warreplasevar(itok4->name);
				return TRUE;

			}
		}
	}
	return FALSE;
}

/*
void updnum(int tok4,long long *num)
{
	switch(tok4){
		case tk_charvar:
		case tk_bytevar:
			*num=*num&0xff;
			break;
		case tk_intvar:
		case tk_wordvar:
			*num=*num&0xffff;
			break;
		case tk_longvar:
		case tk_dwordvar:
			*num=*num&0xffffffff;
			break;

	}
}
 */

void CheckConstVar3(int *tok4,ITOK *itok4,int razr)
{
long long lnum;
int rm;
	if(replasevar==FALSE||itok4->rec==NULL||(itok4->flag&f_useidx)!=0)return;
	if(*tok4>=tk_charvar&&*tok4<=tk_doublevar){
		if((optimizespeed||itok4->type==tp_gvar||itok4->type==tp_postvar||
				itok4->number>127||(int)(itok4->number)<-127)&&CheckConstVar(itok4)){
			switch(*tok4){
				case tk_charvar:
				case tk_bytevar:
					itok4->lnumber=itok4->lnumber&0xff;
					break;
				case tk_intvar:
				case tk_wordvar:
					itok4->lnumber=itok4->lnumber&0xffff;
					break;
				case tk_longvar:
				case tk_dwordvar:
					itok4->lnumber=itok4->lnumber&0xffffffff;
					break;

			}
//			updnum(*tok4,&itok4->lnumber);
//			printf("tok=%d num=%d %s\n",*tok4,itok.number,itok.name);
			*tok4=tk_number;
		}
		else{
			if(CheckConstVar2(itok4,&lnum,&rm)){
				if(short_ok(lnum,razr==r32?TRUE:FALSE)){
					itok4->lnumber=lnum;
					itok4->rm=rm;
					*tok4=tk_number;
					itok4->flag=0;
					warreplasevar(itok4->name);
				}
			}
		}
	}
}

#endif
