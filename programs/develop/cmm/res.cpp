#define _RES_

#ifdef _WIN32_
#include <windows.h>
#endif

#pragma option -w-pin

#include <fcntl.h>
#include <unistd.h>

#include "tok.h"
#include "port.h"
#include "res.h"

RES *listres;	//таблица ресурсов
int numres=0;	//текущее число ресурсов
int maxres=0;	//максимальное число ресурсов

unsigned short *sortidx;	//масив отсортированых индексов ресурсов
RES *curtres;	//текущая таблица ресурсов

unsigned char *resbuf;
unsigned int cursizeresbuf;
unsigned int curposbuf=0;
unsigned int iconcount=0;	//число иконок
unsigned int cursorcount=0;	//число курсоров
unsigned int numidres;	//число ресурсов с разным id
unsigned int numlangres=0;	//число ресурсов с языками
unsigned int numhlangres=0;	//число узлов с языками
unsigned short langdef=0;
unsigned short numzerotype=0;

_STRINGS_ *strinfo=NULL;
int numstrtbl=0;

struct TUSE{
	unsigned short id;
	unsigned short count;
	char *tname;	//имя типа
}*tuse=NULL;

unsigned int numtyperes=0;	//число типов ресурсов

void AddType(unsigned short type,char *tname=NULL)
{
	if(tuse==NULL){
		tuse=(TUSE *)MALLOC(sizeof(TUSE));
		tuse->id=type;
		tuse->count=1;
		if(type==0)tuse->tname=tname;
		numtyperes=1;
		return;
	}
	for(unsigned int i=0;i<numtyperes;i++){
		if(type==(tuse+i)->id){
			if(type==0&&stricmp(tname,(tuse+i)->tname)!=0)continue;
			(tuse+i)->count++;
			return;
		}
	}
	tuse=(TUSE *)REALLOC(tuse,sizeof(TUSE)*(numtyperes+1));
	(tuse+numtyperes)->id=type;
	(tuse+numtyperes)->count=1;
	if(type==0)(tuse+numtyperes)->tname=tname;
	numtyperes++;
}

static unsigned int idnum;
static char idname[IDLENGTH];
static char resname[IDLENGTH];
static int restok;

void InitBufRes();	//инициализировать буфер для ресурса
void CheckResBuf(unsigned int size);	//проверить и если надо увеличить буфер
void AddWString(unsigned char *name); //добавить строку в ресурс
void AddNumOrd(unsigned char *name);	//добавить ординал/строку
void r_Accelerators();
void r_Dialog();
void r_Icon();
void r_Bitmap();
void r_Menu();
void r_Cursor();
void r_Accelerators();
void r_Font();
void r_Stringtable();
void r_Rcdata();
void GetFileName(char *name);
unsigned char *LoadFileBin(char *name);
unsigned short GetFlag(_STRINGS_ *type,int num);
void r_Language();
void r_Version();
void NewResourse();


void badformat(char *name)
{
char buf[80];
	sprintf(buf,"bad format in '%s'",name);
	preerror(buf);
	while(tok!=tk_endline&&tok!=tk_eof)nexttok();
}

void expectedrescommand()
{
	preerror("expected resourse command");
	while(tok!=tk_endline&&tok!=tk_eof)nexttok();
}

void equalres()
{
	preerror("two resource with equal 'id'");
}

void badico()
{
	preerror("not load binare image");
	while(tok!=tk_endline&&tok!=tk_eof)nexttok();
}

extern int CheckResName(char *name);
/*
void SaveFile(char *name)
{
FILE *inih;
	if((inih=fopen(name,"wb"))!=NULL){
		fwrite(resbuf,1,curposbuf,inih);
		fclose(inih);
	}
} */

void input_res()
{
	scanlexmode=RESLEX;
	nexttok();
	while(tok!=tk_eof){
		while(tok==tk_semicolon||tok==tk_endline)nexttok();
		while(tok==tk_question){
			directive();//обработка директив
			while(tok==tk_semicolon||tok==tk_endline)nexttok();
		}
		if(scanlexmode!=RESLEX||tok==tk_eof)break;
		idname[0]=0;
		resname[0]=0;
		idnum=0;
		restok=-1;
		switch(tok){
			case tk_number:
				idnum=itok.number;
				nexttok();
				break;
			case tk_id:
			case tk_ID:
				strcpy(idname,itok.name);
				strupr(idname);
				nexttok();
				break;
			case tk_rescommand:
				restok=itok.number;
				nexttok();
				break;
			default:
//				printf("line %u: tok=%u\n",linenumber,tok);
				unuseableinput(); break;
		}
		if(restok==-1){
			if(tok==tk_number)restok=CRT_NEWRESOURCE;
			else if(tok!=tk_rescommand){
				if(strlen(itok.name)/*&&tok2==tk_string*/){
					restok=0;
					strcpy(resname,itok.name);
					nexttok();
					NewResourse();
					continue;
				}
				expectedrescommand();
			}
			else{
				restok=itok.number;
				nexttok();
			}
		}
		switch(restok){
			case rc_accelerators:
				r_Accelerators();
				break;
			case rc_dialogex:
			case rc_dialog:
				r_Dialog();
				break;
			case rc_icon:
				r_Icon();
				break;
			case rc_bitmap:
				r_Bitmap();
				break;
			case rc_menuex:
			case rc_menu:
				r_Menu();
				break;
			case rc_cursor:
				r_Cursor();
				break;
			case rc_font:
				r_Font();
				break;
			case rc_stringtable:
				r_Stringtable();
				break;
			case rc_rcdata:
				r_Rcdata();
				break;
			case rc_language:
				r_Language();
				break;
			case rc_versioninfo:
				r_Version();
				break;
			case CRT_NEWRESOURCE:
				restok=itok.number;
				nexttok();
				NewResourse();
				break;
			default:
				expectedrescommand();
				nexttok();
				break;
		}
	}
}

void GetResBlock()
{
	if(numres==0){
		maxres=DRESNUM;
		listres=(RES *)MALLOC(DRESNUM*sizeof(RES));
		memset(listres,0,DRESNUM*sizeof(RES));//очистить таблицу
	}
	else{
		if((numres+1)==maxres){
			listres=(RES *)REALLOC(listres,sizeof(RES)*(maxres+DRESNUM));
			memset(listres+maxres,0,DRESNUM*sizeof(RES));//очистить таблицу
			maxres+=DRESNUM;
		}
	}
	curtres=listres+numres;
	curtres->lang=langdef;
	numres++;
}

int OpenBlock()
{
	while(tok==tk_endline)nexttok();
	if(tok==tk_openbrace||(tok==tk_rescommand&&itok.number==rc_begin)){
		nexttok();
		while(tok==tk_endline)nexttok();
		return TRUE;
	}
	return FALSE;
}

int CloseBlock()
{
	while(tok==tk_endline)nexttok();
	if(tok==tk_closebrace||(tok==tk_rescommand&&itok.number==rc_end)){
		nexttok();
		while(tok==tk_endline)nexttok();
		return TRUE;
	}
	return FALSE;
}

unsigned int GetNumber(int par)
{
unsigned int num=0;
	if(tok==tk_number||(tok==tk_minus&&tok2==tk_number))num=doconstdwordmath();
	else{
		numexpected(par);
		nexttok();
	}
	return num;
}

void GetOrdinal(unsigned char **name,int par)
{
NameOrdinal Temp;
	if(tok==tk_number){
		Temp.ordinal[0]=0xffff;
		Temp.ordinal[1]=(unsigned short)doconstdwordmath();
	}
	else if(tok==tk_string){
		Temp.name=(unsigned char *)BackString((char *)string);
		nexttok();
	}
	else numexpected(par);
	*name=Temp.name;
}

void GetRectangle(unsigned char *buf,int par)
{
int i;
	for(i=0;i<4;){
		*(unsigned short *)&buf[i*2]=(unsigned short)GetNumber(i+par);
		i++;
		if(tok==tk_endline)break;
		expecting(tk_camma);
	}
	if(i!=4)preerror("expecting window rectangle (4 signed integers)");
}

void AddWString(unsigned char *name)
{
unsigned int pos;
//unsigned char c;
	if(name==NULL){
		CheckResBuf(2);
		curposbuf+=2;
	}
	else{
		pos=(strlen((char *)name)+1)*3;
		CheckResBuf(pos);
		pos=MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(char *)name,-1,(wchar_t *)&resbuf[curposbuf],pos);
		curposbuf+=pos*2;
	}
}

void AddNumOrd(unsigned char *name)
{
NameOrdinal Temp;
	Temp.name=name;
	if(Temp.ordinal[0]==0xFFFF){
		CheckResBuf(4);
		*(unsigned short *)&resbuf[curposbuf]=Temp.ordinal[0];
		*(unsigned short *)&resbuf[curposbuf+2]=Temp.ordinal[1];
		curposbuf+=4;
	}
	else AddWString(name);
}

void InitBufRes()
{
	resbuf=(unsigned char *)MALLOC(SIZERESBUF);
	memset(resbuf,0,SIZERESBUF);//очистить таблицу
	curposbuf=0;
	cursizeresbuf=SIZERESBUF;
}

void CheckResBuf(unsigned int size)
{
	while((size+curposbuf)>=cursizeresbuf){
		resbuf=(unsigned char *)REALLOC(resbuf,cursizeresbuf+SIZERESBUF);
		memset(resbuf+cursizeresbuf,0,SIZERESBUF);//очистить таблицу
		cursizeresbuf+=SIZERESBUF;
	}
}

void FreeOrdinal(unsigned char *name)
{
NameOrdinal Temp;
	if(name){
		Temp.name=name;
		if(Temp.ordinal[0]!=0xFFFF)free(name);
	}
}

unsigned short GetFlag(_STRINGS_ *type,int num)
{
	for(int i=0;i<num;i++){
		if(stricmp(itok.name,(type+i)->id)==0){
			return (type+i)->val;
		}
	}
	return 0;
}

void r_Language()
{
unsigned short langsec;
	langdef=(unsigned short)GetNumber(1);
	expecting(tk_camma);
	langsec=(unsigned short)GetNumber(2)*langdef*256;
	langdef|=langsec;
	while(tok!=tk_endline&&tok!=tk_eof)nexttok();
}

void r_Stringtable()
{
#define MAXSTRTABINFO 64
static int maxstrinf=MAXSTRTABINFO;
int num;
int j;
	if(strinfo==NULL)strinfo=(_STRINGS_ *)MALLOC(sizeof(_STRINGS_)*MAXSTRTABINFO);
	while(tok!=tk_endline&&tok!=tk_eof)nexttok();
	if(!OpenBlock())badformat("STRINGTABLE");	//добавить новую
	do{
		num=GetNumber(1);
		for(j=0;j<numstrtbl;j++){
			if(num==(strinfo+j)->val)preerror("String ID is already used");
		}
		(strinfo+numstrtbl)->val=(short)num;
		if(tok==tk_camma)nexttok();
		if(tok!=tk_string)badformat("STRINGTABLE");
		(strinfo+numstrtbl)->id=BackString((char *)string);
		nexttok();
		numstrtbl++;
		if(numstrtbl>=maxstrinf){
			maxstrinf+=MAXSTRTABINFO;
			strinfo=(_STRINGS_ *)REALLOC(strinfo,maxstrinf*sizeof(_STRINGS_));
		}
	}while(!CloseBlock()&&tok!=tk_eof);
}

void CreatStrTabRes()
{
unsigned int i,num,j;
int idnum=0;
int usesec;
	for(i=0;i<65536;){
		idnum++;
		usesec=FALSE;
		for(int ii=0;ii<16;ii++,i++){
			for(j=0;j<numstrtbl;j++){
				if(i==(strinfo+j)->val){
					if(usesec==FALSE){
						usesec=TRUE;
						InitBufRes();
						curposbuf=ii*2;
					}
					char *name=(strinfo+j)->id;
					*(unsigned short *)&resbuf[curposbuf]=(unsigned short)strlen(name);
					curposbuf+=2;
					AddWString((unsigned char *)name);
					free(name);
					curposbuf-=4;
					break;
				}
			}
			curposbuf+=2;
		}
		if(usesec){
			GetResBlock();
			curtres->type=CRT_STRING;
			AddType(CRT_STRING);
			curtres->res=(unsigned char *)REALLOC(resbuf,curposbuf);
			curtres->size=curposbuf;
			curtres->id=idnum;
		}
	}
	free(strinfo);
}

void domenu(unsigned int exts)
{
unsigned char *name;
unsigned int lastpos;
unsigned long help;
	if(OpenBlock()){
		do{
			if(tok!=tk_rescommand)badformat("MENU");
			lastpos=(exts==TRUE?curposbuf+12:curposbuf);
			CheckResBuf(18);
			restok=itok.number;
			nexttok();
			switch(restok){
				case rc_menuitem:
					if(tok==tk_string){
						name=(unsigned char *)BackString((char *)string);
						nexttok();
						if(tok==tk_camma){
							nexttok();
							if(exts){
								if(tok!=tk_camma)*(unsigned long *)&resbuf[curposbuf+8]=GetNumber(2);
								if(tok==tk_camma){
									nexttok();
									if(tok!=tk_camma)*(unsigned long *)&resbuf[curposbuf]=GetNumber(3);
									if(tok==tk_camma){
										do{
											nexttok();
											if(tok==tk_number)*(unsigned long *)&resbuf[curposbuf+4]|=itok.number;
											else *(unsigned long *)&resbuf[curposbuf+4]|=GetFlag((_STRINGS_ *)&typemenu,NUMMENUPOPUP);
											nexttok();
										}while(tok==tk_or);
									}
								}
							}
							else{
								if(tok!=tk_camma)*(unsigned short *)&resbuf[curposbuf+2]=(unsigned short)GetNumber(2);
								if(tok==tk_camma){
									do{
										nexttok();
										if(tok==tk_number)*(unsigned short *)&resbuf[curposbuf]|=(unsigned short)itok.number;
										else *(unsigned short *)&resbuf[curposbuf]|=GetFlag((_STRINGS_ *)&typemenu,NUMMENUPOPUP);
										nexttok();
									}while(tok==tk_or||tok==tk_camma);
								}
							}
						}
						curposbuf+=(exts==FALSE?4:14);
						AddWString(name);
						if(exts){
							CheckResBuf(2);
							curposbuf=Align(curposbuf,4);
						}
						free(name);
					}
					else if(stricmp(itok.name,"SEPARATOR")==0){
						if(exts){
							*(unsigned long *)&resbuf[curposbuf]=0x800;
							curposbuf+=16;
						}
						else curposbuf+=6;
						nexttok();
					}
					else badformat("MENU");
					break;
				case rc_popup:
					help=0;
					if(tok!=tk_string)badformat("MENU");
					name=(unsigned char *)BackString((char *)string);
					*(unsigned short *)&resbuf[lastpos]=(exts==FALSE?0x10:1);
					nexttok();
					if(tok==tk_camma){
						if(exts){
							nexttok();
							if(tok!=tk_camma)*(unsigned long *)&resbuf[curposbuf+8]=GetNumber(2);
							if(tok==tk_camma){
								nexttok();
								if(tok!=tk_camma)*(unsigned long *)&resbuf[curposbuf]=GetNumber(3);
								if(tok==tk_camma){
									do{
										nexttok();
										if(tok==tk_number)*(unsigned long *)&resbuf[curposbuf+4]|=itok.number;
										else *(unsigned long *)&resbuf[curposbuf+4]|=GetFlag((_STRINGS_ *)&typemenu,NUMMENUPOPUP);
										nexttok();
									}while(tok==tk_or);
								}
								if(tok==tk_camma){
									nexttok();
									help=GetNumber(5);
								}
							}
						}
						else{
							do{
								nexttok();
								if(tok==tk_number)*(unsigned short *)&resbuf[curposbuf]|=(unsigned short)itok.number;
								else *(unsigned short *)&resbuf[curposbuf]|=GetFlag((_STRINGS_ *)&typemenu,NUMMENUPOPUP);
								nexttok();
							}while(tok==tk_or);
						}
					}
					curposbuf+=(exts==FALSE?2:14);
					AddWString(name);
					if(exts){
						CheckResBuf(6);
						curposbuf=Align(curposbuf,4);
						*(unsigned long *)&resbuf[curposbuf]=help;
						curposbuf+=4;
					}
					if(name)free(name);
					domenu(exts);
					break;
				default:
					badformat("MENU");
					break;
			}
		}while(!CloseBlock()&&tok!=tk_eof);
		resbuf[lastpos]|=0x80;
	}
	else badformat("MENU");
}

void r_Menu()
{
unsigned int exts=FALSE;
	if(restok==rc_menuex)exts=TRUE;
	GetResBlock();
	curtres->type=CRT_MENU;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(CRT_MENU);
	InitBufRes();
	while(tok!=tk_endline)nexttok();
	if(tok==tk_rescommand&&itok.number==rc_language){
		unsigned short olang;
		olang=langdef;
		nexttok();
		r_Language();
		curtres->lang=langdef;
		langdef=olang;
	}
	if(exts){
		*(unsigned long *)&resbuf[0]=0x00040001;
		curposbuf=8;
	}
	else curposbuf=4;
	domenu(exts);
	curtres->res=(unsigned char *)REALLOC(resbuf,curposbuf);
	curtres->size=curposbuf;
}

void r_Rcdata()
{
	GetResBlock();
	curtres->type=CRT_RCDATA;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(CRT_RCDATA);
char name[256];
	name[0]=0;
	GetFileName(name);
	if(name[0]!=0)resbuf=LoadFileBin(name);
	else if(tok==tk_string)resbuf=LoadFileBin((char *)string);
	else{
		InitBufRes();
		if(!OpenBlock())badformat("RCDATA");
		do{
			if(tok==tk_number||(tok==tk_minus&&tok2==tk_number)){
				CheckResBuf(2);
				*(unsigned short *)&resbuf[curposbuf]=(unsigned short)GetNumber(0);
				curposbuf+=2;
			}
			else if(tok==tk_string){
				CheckResBuf(itok.number);
				for(int i=0;i<itok.number;i++)resbuf[curposbuf++]=string[i];
				nexttok();
			}
			else if(tok==tk_id||tok==tk_ID){
				CheckResBuf(strlen(itok.name));
				unsigned char c;
				int j=0;
				for(;;){
					c=itok.name[j++];
					if(c==0)break;
					resbuf[curposbuf++]=c;
				}
				nexttok();
			}
			if(tok==tk_rescommand&&itok.number==rc_characteristics){
				nexttok();
				CheckResBuf(4);
				*(unsigned long *)&resbuf[curposbuf]=GetNumber(0);
				curposbuf+=4;
			}
			else badformat("RCDATA");
			if(tok==tk_camma||tok==tk_semicolon)nexttok();
		}while(!CloseBlock()&&tok!=tk_eof);
	}
	curtres->res=(unsigned char *)REALLOC(resbuf,curposbuf);
	curtres->size=curposbuf;
}

void GetVersPar(unsigned char *buf)
{
	nexttok();
	*(unsigned short *)&buf[2]=(unsigned short)GetNumber(1);
	expecting(tk_camma);
	*(unsigned short *)&buf[0]=(unsigned short)GetNumber(2);
	expecting(tk_camma);
	*(unsigned short *)&buf[6]=(unsigned short)GetNumber(3);
	expecting(tk_camma);
	*(unsigned short *)&buf[4]=(unsigned short)GetNumber(4);
}

void GetBlockInfo()
{
int startpos;
	do{
		startpos=curposbuf;
		CheckResBuf(6);
		curposbuf+=6;
		if(stricmp("BLOCK",itok.name)==0){
			nexttok();
			if(tok!=tk_string)stringexpected();
			CheckResBuf((itok.number+1)*2+2);
			AddWString((unsigned char *)string);
			curposbuf=Align(curposbuf,4);
			nexttok();
			if(OpenBlock())GetBlockInfo();
			else badformat("VERSIONINFO");
		}
		else if(stricmp("VALUE",itok.name)==0){
			nexttok();
			if(tok!=tk_string)stringexpected();
			CheckResBuf((itok.number+1)*2+2);
			AddWString((unsigned char *)string);
			curposbuf=Align(curposbuf,4);
			nexttok();
			if(tok2==tk_string){
				expecting(tk_camma);
				CheckResBuf((itok.number+1)*2+2);
				AddWString((unsigned char *)string);
				*(unsigned short *)&resbuf[startpos+4]=1;
				*(unsigned short *)&resbuf[startpos+2]=(unsigned short)itok.number;
				nexttok();
			}
			else{
				do{
					nexttok();
					CheckResBuf(4);
					*(unsigned short *)&resbuf[curposbuf]=(unsigned short)GetNumber(0);
					curposbuf+=2;
					*(unsigned short *)&resbuf[startpos+2]+=2;
				}while(tok==tk_camma);
//				nexttok();
			}
		}
		else badformat("VERSIONINFO");
		*(unsigned short *)&resbuf[startpos]=(unsigned short)(curposbuf-startpos);
		curposbuf=Align(curposbuf,4);
	}while(CloseBlock()==FALSE);
}

void r_Version()
{
	GetResBlock();
	InitBufRes();
	curtres->type=CRT_VERSION;
	curtres->id=1;
	AddType(CRT_VERSION);
	*(unsigned short *)&resbuf[2]=0x34;
//	if(idname[0]==0)stringexpected();
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	curposbuf=6;
	AddWString((unsigned char *)"VS_VERSION_INFO");
	curposbuf=Align(curposbuf,4);
	*(unsigned long *)&resbuf[curposbuf]=0xfeef04bd;
	*(unsigned long *)&resbuf[curposbuf+4]=0x00010000;
	curposbuf+=8;
	while(!OpenBlock()&&tok!=tk_eof){
		switch(GetFlag((_STRINGS_ *)&typeversion,7)){
			case v_fv:
				GetVersPar(resbuf+curposbuf);
				break;
			case v_pv:
				GetVersPar(resbuf+curposbuf+8);
				break;
			case v_ffm:
				nexttok();
				*(unsigned long *)&resbuf[curposbuf+16]=(unsigned long)GetNumber(0);
				break;
			case v_ff:
				nexttok();
				*(unsigned long *)&resbuf[curposbuf+20]=(unsigned long)GetNumber(0);
				break;
			case v_fo:
				nexttok();
				*(unsigned long *)&resbuf[curposbuf+24]=(unsigned long)GetNumber(0);
				break;
			case v_ft:
				nexttok();
				*(unsigned long *)&resbuf[curposbuf+28]=(unsigned long)GetNumber(0);
				break;
			case v_fs:
				nexttok();
				*(unsigned long *)&resbuf[curposbuf+32]=(unsigned long)GetNumber(0);
				break;
			default:
				badformat("VERSIONINFO");
				break;
		}
	}
	curposbuf+=44;
	GetBlockInfo();
	*(unsigned short *)&resbuf[0]=(unsigned short)curposbuf;
	curtres->res=(unsigned char *)REALLOC(resbuf,curposbuf);
	curtres->size=curposbuf;
}

void r_Dialog()
{
unsigned char *name=NULL;
unsigned char *font=NULL;
int sizefont=0,i;
NameOrdinal Menu;
NameOrdinal Class;
unsigned int poscount;	//позиция счетчика элементов
unsigned int exts=FALSE;
//unsigned short id;
	Menu.name=NULL;
	Class.name=NULL;
	if(restok==rc_dialogex)exts=TRUE;
	GetResBlock();
	curtres->type=CRT_DIALOG;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(CRT_DIALOG);
	InitBufRes();
	if(exts){
		*(unsigned long *)&resbuf[0]=0xFFFF0001;
		curposbuf=8;
		poscount=16;
		*(unsigned long *)&resbuf[12]=0x80880000;//WS_POPUP|WS_BORDER|WS_SYSMENU;	//установки по умолчанию
	}
	else{
		*(unsigned long *)&resbuf[0]=0x80880000;//WS_POPUP|WS_BORDER|WS_SYSMENU;	//установки по умолчанию
		curposbuf=0;
		poscount=8;
	}
	if(tok2!=tk_camma){	//пропускаем возможные первые два параметра
		nexttok();
		if(tok2!=tk_camma){
			nexttok();
			if(tok2!=tk_camma)badformat("DIALOG");
		}
	}
	GetRectangle(&resbuf[curposbuf+10],1);
	//определить место для IDHelp
	if(tok!=tk_endline&&exts)/**(unsigned long *)&resbuf[curposbuf]=*/GetNumber(0);
	while(!OpenBlock()&&tok!=tk_eof){
		if(tok!=tk_rescommand)expectedrescommand();
		switch(itok.number){
			case rc_style:
				nexttok();
				*(unsigned long *)&resbuf[exts==TRUE?12:0]=GetNumber(1);
				break;
			case rc_caption:
				nexttok();
				if(tok!=tk_string)stringexpected();
				name=(unsigned char *)BackString((char *)string);
				nexttok();
				break;
			case rc_font:
				nexttok();
				sizefont=GetNumber(1);
				expecting(tk_camma);
				if(tok!=tk_string)stringexpected();
				font=(unsigned char *)BackString((char *)string);
				nexttok();
				break;
			case rc_class:
				nexttok();
				GetOrdinal(&Class.name,1);
//				if(tok!=tk_string)stringexpected();
//				Class.name=(unsigned char *)BackString((char *)string);
//				nexttok();
				break;
			case rc_exstyle:
				nexttok();
				*(unsigned long *)&resbuf[exts==TRUE?8:4]=GetNumber(1);
				break;
			case rc_language:
				unsigned short olang;
				olang=langdef;
				nexttok();
				r_Language();
				curtres->lang=langdef;
				langdef=olang;
				break;
			default:
				if(exts&&itok.number==rc_menu){
					nexttok();
					GetOrdinal(&Menu.name,1);
				}
				else badformat("DIALOG");
				break;
		}
		while(tok==tk_endline)nexttok();
	}
//доформировываем диалог
	curposbuf=exts==TRUE?26:18;
	AddNumOrd(Menu.name);
	FreeOrdinal(Menu.name);
	AddNumOrd(Class.name);
	FreeOrdinal(Class.name);
	AddWString(name);
	if(name)free(name);
	if(sizefont){
		resbuf[exts==TRUE?12:0]|=0x40;
		*(unsigned short *)&resbuf[curposbuf]=(unsigned short)sizefont;
		curposbuf+=2;
		if(exts){
				*(unsigned int *)&resbuf[curposbuf]=0x01000000;
				curposbuf+=4;
		}
		AddWString(font);
		free(font);
	}
	while(!CloseBlock()&&tok!=tk_eof){
//	do{
		if(tok!=tk_rescommand)expectedrescommand();
		restok=itok.number;
		nexttok();
		curposbuf=Align(curposbuf,4);
		CheckResBuf(34);
		name=Menu.name=NULL;
		i=1;
		if(exts)curposbuf+=8;
		*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?16:18)]=0XFFFF;
		*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?18:20)]=defdialog[restok].dclass;
		*(unsigned long *)&resbuf[curposbuf]=defdialog[restok].style|0x50000000;
		switch(restok){
			case rc_control:
				GetOrdinal(&Menu.name,1);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				if(exts)*(unsigned int *)&resbuf[curposbuf+12]=(unsigned int)GetNumber(2);
				else *(unsigned short *)&resbuf[curposbuf+16]=(unsigned short)GetNumber(2);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				if(tok==tk_number)*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?18:20)]=(unsigned short)itok.number;
				else{
					if(tok==tk_string)strncpy(itok.name,(char *)string,IDLENGTH);
					i=GetFlag((_STRINGS_ *)&typeclass,6);
					if(!i)name=(unsigned char *)BackString((char *)string);
					else *(unsigned short *)&resbuf[curposbuf+(exts==TRUE?18:20)]=(unsigned short)i;
				}
				nextexpecting2(tk_camma);
				while(tok==tk_endline)nexttok();
				*(unsigned long *)&resbuf[curposbuf]|=GetNumber(4);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				GetRectangle(&resbuf[curposbuf+(exts==TRUE?4:8)],5);
				if(exts&&tok==tk_number)*(unsigned long *)&resbuf[curposbuf-4]=GetNumber(9);
				while(tok!=tk_endline&&tok!=tk_eof&&tok!=tk_rescommand)nexttok();	//пропуск излишних параметров
				break;
			case rc_auto3state:
			case rc_autocheckbox:
			case rc_autoradiobutton:
			case rc_checkbox:
			case rc_ctext:
			case rc_defpushbutton:
			case rc_groupbox:
			case rc_ltext:
			case rc_pushbox:
			case rc_pushbutton:
			case rc_radiobutton:
			case rc_rtext:
			case rc_state3:
				if(tok!=tk_string)stringexpected();
				Menu.name=(unsigned char *)BackString((char *)string);
				nexttok();
				if(tok==tk_camma)nexttok();
				while(tok==tk_endline&&tok!=tk_eof)nexttok();
				i++;
			case rc_combobox:
			case rc_edittext:
			case rc_listbox:
			case rc_scrollbar:
				if(exts)*(unsigned int *)&resbuf[curposbuf+12]=(unsigned int)GetNumber(i);
				else *(unsigned short *)&resbuf[curposbuf+16]=(unsigned short)GetNumber(i);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				GetRectangle(&resbuf[curposbuf+(exts==TRUE?4:8)],i+1);
				if(tok!=tk_endline){
					*(unsigned long *)&resbuf[curposbuf]|=(unsigned long)GetNumber(i+5);
					if(tok==tk_camma){
						nexttok();
						*(unsigned long *)&resbuf[curposbuf+(exts==TRUE?-4:4)]=(unsigned long)GetNumber(i+6);
					}
				}
				if(exts&&tok==tk_number)*(unsigned long *)&resbuf[curposbuf-4]=GetNumber(i+9);
				while(tok!=tk_endline&&tok!=tk_eof&&tok!=tk_rescommand)nexttok();	//пропуск излишних параметров
				break;
			case rc_icon:
				GetOrdinal(&Menu.name,1);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				if(exts)*(unsigned int *)&resbuf[curposbuf+12]=(unsigned int)GetNumber(2);
				else *(unsigned short *)&resbuf[curposbuf+16]=(unsigned short)GetNumber(2);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?4:8)]=(unsigned short)GetNumber(3);
				expecting(tk_camma);
				while(tok==tk_endline)nexttok();
				*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?6:10)]=(unsigned short)GetNumber(4);
				if(tok==tk_camma){
					nexttok();
					*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?8:12)]=(unsigned short)GetNumber(5);
					if(tok==tk_camma){
						nexttok();
						*(unsigned short *)&resbuf[curposbuf+(exts==TRUE?10:14)]=(unsigned short)GetNumber(6);
						if(tok==tk_camma){
							nexttok();
							*(unsigned long *)&resbuf[curposbuf]|=GetNumber(7);
						}
					}
				}
				break;
			default:
				badformat("DIALOG");
				break;
		}
		while(tok==tk_endline)nexttok();
		*(unsigned short *)&resbuf[poscount]=(unsigned short)(*(unsigned short *)&resbuf[poscount]+1);
		curposbuf+=(exts==TRUE?20:22);
		if(name){
			curposbuf-=4;
			AddWString(name);
			free(name);
		}
		if(Menu.name){
			AddNumOrd(Menu.name);
			FreeOrdinal(Menu.name);
		}
		else curposbuf+=2;
		CheckResBuf(6);
		curposbuf+=2;
//	}while(!CloseBlock()&&tok!=tk_eof);
	}
	curtres->res=(unsigned char *)REALLOC(resbuf,curposbuf);
	curtres->size=curposbuf;
}

void GetFileName(char *name)
{
	while(tok!=tk_string&&tok!=tk_endline&&tok!=tk_eof){
		int i;
		for(i=0;i<7;i++){
			if(stricmp(itok.name,typemem[i].id)==0)break;
		}
		if(i==7){
			strcpy(name,itok.name);
			i=strlen(itok.name);
			name[i++]=cha2;
			for(;;){
				cha2=input[inptr2++];
				if(cha2<=0x20)break;
				name[i++]=cha2;
			}
			name[i]=0;
			break;
		}
		nexttok();
	}
}

unsigned char *LoadFileBin(char *name)
{
int inico;
unsigned char *bitobr;
	if((inico=open(name,O_BINARY|O_RDONLY))==-1){
		badinfile(name);
		return NULL;
	}
	if((curposbuf=getfilelen(inico))==0){
		badinfile(name);
		close(inico);
		return NULL;
	}
	bitobr=(unsigned char *)MALLOC(curposbuf);
	if((unsigned int)read(inico,bitobr,curposbuf)!=curposbuf){
		errorreadingfile(name);
		close(inico);
		free(bitobr);
		return NULL;
	}
	close(inico);
	nexttok();
	return bitobr;
}

unsigned char *LoadBitmap()
{
//загрузить
unsigned char *bitobr=NULL;
char name[80];
	curposbuf=0;
	name[0]=0;
	GetFileName(name);
	if(name[0]!=0)bitobr=LoadFileBin(name);
	else if(tok==tk_endline){	//нет имени файла
		InitBufRes();
		if(!OpenBlock()){
			badico();
			return NULL;
		}
		do{
			inptr=inptr2;
			cha=cha2;
			if(tok!=tk_singlquote)badico();
			whitespace(); //пропуск незначащих символов
			CheckResBuf(16);
			displaytokerrors=1;
			do{
				unsigned char hold=0;
				for(int i=0;i<2;i++){
					hold*=(unsigned char)16;
					if(isdigit(cha))hold+=(unsigned char)(cha-'0');
					else if(isxdigit(cha))hold+=(unsigned char)((cha&0x5f)-'7');
					else expectederror("hexadecimal digit");
					nextchar();
				}
				resbuf[curposbuf++]=hold;
				whitespace(); //пропуск незначащих символов
			}while(cha!='\''&&cha!=26);
			inptr2=inptr;
			cha2=cha;
			linenum2=linenumber;
			nexttok();
			nexttok();
		}while(!CloseBlock()&&tok!=tk_eof);
		bitobr=(unsigned char *)REALLOC(resbuf,curposbuf);
	}
	else if(tok==tk_string)bitobr=LoadFileBin((char *)string);
	return bitobr;
}

void r_Bitmap()
{
unsigned int size;
unsigned char *bitobr;
	if((bitobr=LoadBitmap())==NULL)return;
	size=curposbuf-14;
	GetResBlock();	//битмар
	curtres->type=CRT_BITMAP;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	curtres->size=size;
	curtres->res=(unsigned char *)MALLOC(size);
	AddType(CRT_BITMAP);
	memcpy(curtres->res,bitobr+14,size);
	free(bitobr);
}

void NewResourse()
{
unsigned char *bitobr=NULL;
char name[256];
	GetResBlock();
	if(resname[0]==0)curtres->type=restok;
	else curtres->tname=BackString(resname);
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(restok,curtres->tname);
	name[0]=0;
	GetFileName(name);
	if(name[0]!=0)bitobr=LoadFileBin(name);
	else if(tok==tk_string)bitobr=LoadFileBin((char *)string);
	else stringexpected();
	if(bitobr!=NULL){
		curtres->res=bitobr;
		curtres->size=curposbuf;
	}
	nexttok();
}

void r_Font()
{
unsigned char *fontobr;
	if((fontobr=LoadBitmap())==NULL)return;
	if((unsigned short)curposbuf==*(unsigned short *)&fontobr[2]){
		GetResBlock();	//фонт
		curtres->type=CRT_FONT;
		if(idname[0]==0)curtres->id=idnum;
		else curtres->name=BackString(idname);
		curtres->size=curposbuf;
		curtres->res=fontobr;
		AddType(CRT_FONT);
	}
}

void r_Icon()
{
unsigned char *icoobr;
unsigned long size;
//загрузить иконку
	if((icoobr=LoadBitmap())==NULL)return;
	GetResBlock();	//группа икон
	curtres->type=CRT_GROUP_ICON;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(CRT_GROUP_ICON);
unsigned int countico=*(unsigned short *)&icoobr[4];	//число иконок
int sizeicohead=sizeof(_ICOHEAD_)+(sizeof(_RESDIR_)*countico);
	curtres->size=sizeicohead;
	curtres->res=(unsigned char *)MALLOC(sizeicohead);
unsigned char *icohead=curtres->res;
unsigned int i;
	for(i=0;i<6;i++)icohead[i]=icoobr[i];	//заголовок
unsigned int ofs=6;
unsigned int ofs2=6;
	for(i=0;i<countico;i++){
		int j;
		for(j=0;j<12;j++)icohead[j+ofs]=icoobr[j+ofs2];	//описание иконки
		iconcount++;
		*(unsigned short *)&icohead[ofs+12]=(unsigned short)iconcount;	//ее номер
		GetResBlock();	//образ иконки
		curtres->type=CRT_ICON;
		curtres->id=iconcount;
		curtres->size=size=*(unsigned long *)&icohead[ofs+8];
		curtres->res=(unsigned char *)MALLOC(size);
		AddType(CRT_ICON);
		unsigned int ofs3=*(unsigned int *)&icoobr[ofs2+12];
		for(j=0;(unsigned int)j<size;j++)curtres->res[j]=icoobr[j+ofs3];
		ofs+=sizeof(_RESDIR_);
		ofs2+=sizeof(_RESDIR_)+2;
	}
	free(icoobr);
}

void r_Cursor()
{
unsigned char *curobr;
unsigned long size;
//загрузить курсор
	if((curobr=LoadBitmap())==NULL)return;
	GetResBlock();	//группа курсоров
	curtres->type=CRT_GROUP_CURSOR;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(CRT_GROUP_CURSOR);
unsigned int countcur=*(unsigned short *)&curobr[4];	//число курсоров в файле
int sizecurhead=sizeof(_ICOHEAD_)+(sizeof(_CURDIR_)*countcur);
	curtres->size=sizecurhead;
	curtres->res=(unsigned char *)MALLOC(sizecurhead);
unsigned char *curhead=curtres->res;
unsigned int i;
	for(i=0;i<6;i++)curhead[i]=curobr[i];	//заголовок
unsigned int ofs=6;
unsigned int ofs2=6;
	for(i=0;i<countcur;i++){
		cursorcount++;
		*(unsigned short *)&curhead[ofs]=curobr[ofs2];
		*(unsigned short *)&curhead[ofs+2]=curobr[ofs2+1];
		*(unsigned long *)&curhead[ofs+4]=0x10001;
		*(unsigned short *)&curhead[ofs+12]=(unsigned short)cursorcount;	//ее номер
		GetResBlock();	//образ курсора
		curtres->type=CRT_CURSOR;
		curtres->id=cursorcount;
		curtres->size=size=*(unsigned long *)&curhead[ofs+8]=*(unsigned long *)&curobr[ofs2+8]+4;
		curtres->res=(unsigned char *)MALLOC(size);
		AddType(CRT_CURSOR);
		unsigned int ofs3=*(unsigned int *)&curobr[ofs2+12];
		*(unsigned short *)&curtres->res[0]=*(unsigned short *)&curobr[ofs2+4];
		*(unsigned short *)&curtres->res[2]=*(unsigned short *)&curobr[ofs2+6];
		size-=4;
		for(int j=0;(unsigned int)j<size;j++)curtres->res[j+4]=curobr[j+ofs3];
		ofs+=sizeof(_CURDIR_);
		ofs2+=sizeof(_CURDIR_)+2;
	}
	free(curobr);
}

void r_Accelerators()
{
	GetResBlock();
	curtres->type=CRT_ACCELERATOR;
	if(idname[0]==0)curtres->id=idnum;
	else curtres->name=BackString(idname);
	AddType(CRT_ACCELERATOR);
	InitBufRes();
	if(OpenBlock()){
		do{
			CheckResBuf(8);
			if(tok==tk_string){
				unsigned char c;
				c=string[0];
				if(c=='^'){
					c=string[1];
					if(c<0x40)preerror("Unsolved symbol for Contrl");//error
					c-=0x40;
				}
				*(unsigned short *)&resbuf[curposbuf+2]=(unsigned short)c;
				nexttok();
			}
			else{
				*(unsigned short *)&resbuf[curposbuf+2]=(unsigned short)GetNumber(1);
			}
			expecting(tk_camma);
			*(unsigned short *)&resbuf[curposbuf+4]=(unsigned short)GetNumber(2);
			if(tok==tk_camma){
				nexttok();
				if(tok==tk_number)*(unsigned short *)&resbuf[curposbuf]|=(unsigned short)itok.number;
				*(unsigned short *)&resbuf[curposbuf]|=GetFlag((_STRINGS_ *)&typeacceler,5);//GetAccerFlag();
				nexttok();
				while(tok!=tk_endline){
					if(tok==tk_camma)nexttok();
					if(tok==tk_number)*(unsigned short *)&resbuf[curposbuf]|=(unsigned short)itok.number;
					*(unsigned short *)&resbuf[curposbuf]|=GetFlag((_STRINGS_ *)&typeacceler,5);//GetAccerFlag();
					nexttok();
				}
			}
			curposbuf+=8;
		}while(!CloseBlock()&&tok!=tk_eof);
		resbuf[curposbuf-8]|=0x80;
	}
	else badformat("ACCELERATORS");
	curtres->res=(unsigned char *)REALLOC(resbuf,curposbuf);
	curtres->size=curposbuf;
}

void SortRes()
{
int i,j,k;
int sortpos=0;	//позиция в списке сортировки
	for(i=0;i<numtyperes;i++){
		for(j=i+1;j<numtyperes;j++){
			TUSE buf;
			if((tuse+i)->id>(tuse+j)->id){
				buf.id=(tuse+i)->id;
				buf.count=(tuse+i)->count;
				buf.tname=(tuse+i)->tname;
				(tuse+i)->id=(tuse+j)->id;
				(tuse+i)->count=(tuse+j)->count;
				(tuse+i)->tname=(tuse+j)->tname;
				(tuse+j)->count=buf.count;
				(tuse+j)->id=buf.id;
				(tuse+j)->tname=buf.tname;
			}
			if((tuse+i)->id==(tuse+j)->id){
				if(stricmp((tuse+i)->tname,(tuse+j)->tname)>0){
					buf.count=(tuse+i)->count;
					buf.tname=(tuse+i)->tname;
					(tuse+i)->count=(tuse+j)->count;
					(tuse+i)->tname=(tuse+j)->tname;
					(tuse+j)->count=buf.count;
					(tuse+j)->tname=buf.tname;
				}
			}
		}
		if((tuse+i)->id==0)numzerotype++;
	}

	sortidx=(unsigned short *)MALLOC(sizeof(unsigned short)*numres);
	for(i=0;i<numtyperes;i++){
		for(k=0,j=0;j<numres;j++){
			if((tuse+i)->id==(listres+j)->type){
				if((listres+j)->lang){
					numlangres++;
					numhlangres++;
				}
				if((tuse+i)->count==1){
					sortidx[sortpos++]=(unsigned short)j;
					break;
				}
				else{
					if(k==0)sortidx[sortpos++]=(unsigned short)j;
					else{
						int m=k;	//число элементов с данным типом
						int n=sortpos-k;
						do{
							m--;
							if((listres+j)->name==NULL&&(listres+sortidx[m+n])->name==NULL){
								if((listres+j)->id>=(listres+sortidx[m+n])->id){//новый больше
									if((listres+j)->id==(listres+sortidx[m+n])->id){	//равны
										numidres--;
										numlangres++;
										if((listres+j)->lang==0){
											numlangres++;
											numhlangres++;
										}
										if((listres+j)->lang==(listres+sortidx[m+n])->lang)equalres();
										if((listres+j)->lang>(listres+sortidx[m+n])->lang){	//у нового язык старше
											sortidx[n+m+1]=(unsigned short)j;	//добавить в конец
											break;
										}
									}
									else{
										sortidx[n+m+1]=(unsigned short)j;	//добавить в конец
										break;
									}
								}
								sortidx[n+m+1]=sortidx[n+m];	//сдвинуть
							}
							else if((listres+j)->name==NULL&&(listres+sortidx[m+n])->name!=NULL){
								sortidx[n+m+1]=(unsigned short)j;
								break;
							}
							else if((listres+j)->name!=NULL&&(listres+sortidx[m+n])->name==NULL){
								sortidx[n+m+1]=sortidx[n+m];
							}
							else{
								int cmp;
								if((cmp=strcmp((listres+j)->name,(listres+sortidx[m+n])->name))>=0){
									if(cmp==0){
										numidres--;
										numlangres++;
										if((listres+j)->lang==0){
											numlangres++;
											numhlangres++;
										}
//										numhlangres--;
										if((listres+j)->lang==(listres+sortidx[m+n])->lang)equalres();
										if((listres+j)->lang>(listres+sortidx[m+n])->lang){
											sortidx[n+m+1]=(unsigned short)j;
											break;
										}
									}
									else{
										sortidx[n+m+1]=(unsigned short)j;
										break;
									}
								}
								sortidx[n+m+1]=sortidx[n+m];
							}
							if(m==0)sortidx[sortpos-k]=(unsigned short)j;
						}while(m>0);
						sortpos++;
					}
				}
				k++;
			}
		}
	}
/*
	for(i=0;i<numres;i++){
		printf("type=%-5uid=%-10u type=%-5uid=%u\n",(listres+sortidx[i])->type,(listres+sortidx[i])->id,
			(listres+i)->type,(listres+i)->id);
	}
	*/
}


int MakeRes(unsigned long ofsres,LISTRELOC **listrel)
{
int i,j;
unsigned long nextofs;
unsigned int startlang,ofsback;
unsigned int startofsdata,startdata;
int numrel=0;
RES *curres;
LISTRELOC *listr=NULL;
	linenumber=0;
	InitBufRes();
	numidres=numres;
	SortRes();
//создать корневой уровень
	*(unsigned short *)&resbuf[12]=(unsigned short)numzerotype;
	*(unsigned short *)&resbuf[14]=(unsigned short)(numtyperes-numzerotype);
	curposbuf=16;
	nextofs=numtyperes*8+16;
//расчет размеров уровней
	startlang=curposbuf+numtyperes*24+numidres*8;
	startofsdata=startlang+numhlangres*16+numlangres*8;
	startdata=startofsdata+numres*16;
	CheckResBuf(startdata-curposbuf);
	for(i=0;i<numtyperes;i++){
		if((tuse+i)->id==0){
			*(unsigned long *)&resbuf[curposbuf]=startdata|0x80000000;
			curres=listres+sortidx[i];
			int len=strlen(curres->tname);
			CheckResBuf(startdata-curposbuf+len*2+4);
			*(unsigned short *)&resbuf[startdata]=(unsigned short)len;
			unsigned char c;
			len=0;
			startdata+=2;
			for(;;){
				c=curres->tname[len++];
				if(c==0)break;
				resbuf[startdata]=c;
				startdata+=2;
			}
			free(curres->tname);
		}
		else *(unsigned long *)&resbuf[curposbuf]=(tuse+i)->id;
		*(unsigned long *)&resbuf[curposbuf+4]=nextofs|0x80000000;
		nextofs+=(tuse+i)->count*8+16;
		curposbuf+=8;
	}
//расчет размеров уровней
/*	startlang=curposbuf+numtyperes*16+numidres*8;
	startofsdata=startlang+numhlangres*16+numlangres*8;
	startdata=startofsdata+numres*16;
	CheckResBuf(startdata-curposbuf);*/
//создать уровень имен и языка
	curres=listres+sortidx[0];
	for(j=0;j<numres;){
		ofsback=curposbuf+12;
		curposbuf+=16;
		unsigned int type=curres->type;
		while((unsigned int)curres->type==type){
			int k=j;
			if(curres->name){	//добавить имя
				if(j<(numres-1)&&type==(unsigned int)(listres+sortidx[j+1])->type&&
					(listres+sortidx[j+1])->name!=NULL)
					while(strcmp(curres->name,(listres+sortidx[j+1])->name)==0)j++;
				*(unsigned short *)&resbuf[ofsback]=(unsigned short)(*(unsigned short *)&resbuf[ofsback]+1);
				*(unsigned long *)&resbuf[curposbuf]=startdata|0x80000000;
				int len=strlen(curres->name);
				CheckResBuf(startdata-curposbuf+len*2+4);
				*(unsigned short *)&resbuf[startdata]=(unsigned short)len;
				unsigned char c;
				len=0;
				startdata+=2;
				for(;;){
					c=curres->name[len++];
					if(c==0)break;
					resbuf[startdata]=c;
					startdata+=2;
				}
//				startdata=Align(startdata,4);
				free(curres->name);
			}
			else{	//добавить id
				if(j<(numres-1)&&type==(unsigned int)(listres+sortidx[j+1])->type)
					while(curres->id==(listres+sortidx[j+1])->id)j++;
				*(unsigned short *)&resbuf[ofsback+2]=(unsigned short)(*(unsigned short *)&resbuf[ofsback+2]+1);
				*(unsigned long *)&resbuf[curposbuf]=curres->id;
			}
			curposbuf+=4;
			if(j!=k){	//несколько имен с разными языками
				*(unsigned long *)&resbuf[curposbuf]=startlang|0x80000000;
				*(unsigned long *)&resbuf[startlang+12]=j-k+1;
				startlang+=16;
				for(;k<=j;k++){
					*(unsigned long *)&resbuf[startlang]=(listres+sortidx[k])->lang;
					*(unsigned long *)&resbuf[startlang+4]=startofsdata;
					startlang+=8;
					startofsdata+=16;
				}
			}
			else{
				if(curres->lang){//указан язык
					*(unsigned long *)&resbuf[curposbuf]=startlang|0x80000000;
					resbuf[startlang+14]=1;
					startlang+=16;
					*(unsigned long *)&resbuf[startlang]=curres->lang;
					*(unsigned long *)&resbuf[startlang+4]=startofsdata;
					startlang+=8;
				}
				else *(unsigned long *)&resbuf[curposbuf]=startofsdata;
				startofsdata+=16;
			}
			curposbuf+=4;
			j++;
			if(j==numres)break;
			curres=listres+sortidx[j];
		}
	}
	curposbuf=startlang;
//создать уровень смещений данных
	for(i=0;i<numres;i++){
		startdata=Align(startdata,4);
		curres=listres+sortidx[i];
		*(unsigned long *)&resbuf[curposbuf]=startdata+ofsres;
		if(!numrel)listr=(LISTRELOC *)MALLOC(sizeof(LISTRELOC));
		else listr=(LISTRELOC *)REALLOC(listr,(numrel+1)*sizeof(LISTRELOC));
		(listr+numrel)->val=curposbuf;
		numrel++;
		curposbuf+=4;
		*(unsigned long *)&resbuf[curposbuf]=curres->size;
		curposbuf+=12;
		CheckResBuf(startdata-curposbuf+curres->size);
		memcpy(resbuf+startdata,curres->res,curres->size);
		free(curres->res);
		startdata+=curres->size;
	}
	curposbuf=startdata;
	nextofs=Align(curposbuf,FILEALIGN);
	CheckResBuf(nextofs-curposbuf);
//	SaveFile("resorse");
	*listrel=listr;
	return numrel;
}
