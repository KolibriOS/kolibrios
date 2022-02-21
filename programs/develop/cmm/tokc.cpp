#define _TOKC_

#include <fcntl.h>	 /* O_ constant definitions */
#include <unistd.h>	 
#include "tok.h"

void GetFileTime(int fd,struct ftime *buf);

#define MAXIF 32

//idrec *crec=NULL;

char invaliddecrem[]="invalid token to be decremented";
char mesLOOPNZ[]="LOOPNZ";
char mesIF[]="IF";
char mesELSE[]="ELSE";
char mesWHILE[]="WHILE";
char mesFOR[]="FOR";
char mesRETURN[]="RETURN";
int tok,tok2; /* token holders current, next */
unsigned char string[STRLEN],string2[STRLEN+20],
  string3[STRLEN];	//содержит не преобразованную строку
unsigned int posts=0; 		 /* number of post entrys */
postinfo *postbuf;
unsigned int maxposts=MAXPOSTS;
unsigned int secondcallnum=1;  /* # of different second calls and labels */
unsigned int externnum=0;
unsigned long postsize=0;			 /* total size of all post vars */
unsigned int poststrptr=0;	/* post string index in output */
unsigned int outptrsize=MAXDATA;	//размер выходного буфера для кода
unsigned int outdatasize=MAXDATA;	//размер выходного буфера для данных
long runfilesize;
int error=0;		 /* current error number holder */
unsigned char dos1=0,dos2=0;		/* DOS version required for program execution */
unsigned char cpu=0;	/* CPU required, 2 for 80286, 3 for 80386, ... */
unsigned int paramsize=0;  /* byte size of all procedure parameters */
unsigned int localsize=0;  /* byte size of all procedure local variables */
unsigned int procedure_start=0; /* address of start of procedure */
unsigned int current_proc_type;	 /* current procedure type */
int returntype; 				 /* return type, (void, byte, word, ...) */
unsigned char warning;
/*+++++++++++++++++++++++ Установки по умолчанию +++++++++++++++++++++++*/
unsigned char am32 = FALSE; 		      // режим 32 битной адресации
unsigned char comfile = file_com; 		// output file format
unsigned char optimizespeed = 1;			// optimize for size or speed flag
unsigned char alignword = 1;					// flag whether or not to align words
unsigned char aligner = 0;						// value used to align words
unsigned char header = 1; 						// output SPHINX C-- Ver1 Ver2 header
unsigned char chip = 0; 							// CPU optimization (286,386,486,...)
unsigned char killctrlc = 0;					// add disable CTRL-C code in header
unsigned int	stacksize = 2048; 			// stack size (2048 default)
unsigned char splitdata=FALSE;	      //отделить данные от кода
unsigned char AlignCycle=FALSE;       //выравнивать начала циклов
/*+++++++++++++++++++ end of flexable compiler options ++++++++++++++++++++*/
unsigned char notdoneprestuff = TRUE; // flag if initial stuff has been entered
unsigned int datasize=0,alignersize=0;	/* size of data and other */
unsigned int outptr=0x100,outptrdata=0x100; 			/* ptr to output */
unsigned char *output;
unsigned char *outputdata=NULL;
unsigned int linenumber=0;
unsigned char dynamic_flag=0;	//флаг обработки динамических элементов

unsigned char *input; 	 /* dynamic input buffer */
unsigned int endinptr;		 /* end index of input array */
unsigned int inptr; 		 /* index in input buffer */
unsigned char cha;		 /* pipe byte for token production */
char endoffile; 		 /* end of input file flag */
unsigned char insertmode=FALSE;
unsigned int numblocks=0;	//номер вложенного блока
treelocalrec *tlr=NULL;	//цепочка локальных блоков
treelocalrec *btlr=NULL;	//цепочка использованых локальных блоков
RETLIST *listreturn=NULL;
unsigned int numreturn=0;
idrec *staticlist;
unsigned char stat_reg[8];	//таблица занятости регистров

int sizestack=0;	//размер не компенсированных параметров функций
unsigned char addstack=TRUE;

extern char shorterr[];
extern unsigned long long li[];

/*-----------------01.05.98 19:22-------------------
 дополнительные переменные для реализации BREAK CONTINUE
--------------------------------------------------*/
#define MAXIN 100	//максимальная вложеность циклов
unsigned int numbr=0;	//счетчик общего числа циклов LOOP DO-WHILE...
unsigned int listbr[MAXIN];	//таблица номеров циклов
unsigned int usebr[MAXIN];	//использовано break
unsigned int useco[MAXIN];	//использовано continue
unsigned int curbr=0,curco=0;	//текущаявложеность циклов
unsigned int startStartup=0x100;
unsigned int endStartup=0;
unsigned char useStartup=FALSE;
unsigned char notpost=FALSE;
int retproc;
int lastcommand;	//последний оператор в блоке
unsigned char FastCallApi=TRUE;	//разрешить быстрый вызов API процедур
unsigned char FixUp=FALSE;	//Делать ли таблицу перемещений
unsigned char AlignProc=FALSE;
//------- работа с union ---------------------------------------------
char param[256];	//буфер для параметров процедуры
char *BackTextBlock;	//буфер для перенесенного текста
int SizeBackBuf=0,MaxSizeBackBuf;
struct FILEINFO *startfileinfo=NULL;
unsigned int totalmodule=0;
unsigned int currentfileinfo;
unsigned char setzeroflag;	//операция меняет zero flag
unsigned char notunreach=FALSE;
unsigned int initBP=0;
int inlineflag=0;  // flag for disabling entry and exit codes production
unsigned char fstatic=FALSE;

unsigned long addESP=0;	//добавка стека
unsigned char blockproc=FALSE;	//идетразборка блока функции

unsigned int   updatelocalvar(char *str,int tok,unsigned int num);
void setuprm();
void doloop(unsigned int typeb);							 /* both short and long loops */
void doBREAK(unsigned char typeb);
void doCONTINUE(unsigned char typeb);
void dowhile(unsigned int typeb);
void MakeContinue(unsigned char typeb);
void dofor(unsigned int typeb);
void dodo();
void globalvar(); 	/* both initialized and unitialized combined */
void doswitch();
void CalcRegPar(int reg,int def,char *&ofsstr);
void JXorJMP();
int loadinputfile(char *inpfile);
int SaveStartUp(int size,char *var_name);
void LoadData(unsigned int size,int filehandle);
void SetNewTok(int type,int typev);
void doreturn(int type=tokens);		/* do return(...); */
void notnegit(int notneg);
void insertcode();		 // force code procedure at specified location
void interruptproc();
void dobigif();
void doif(void);
void doasmblock();
void declareextern();
unsigned long dounion(int,int);
void RunBackText();
int FindDublString(int segm,unsigned int len,int term);
void *liststring=NULL;	//цепочка информационных блоков о строках
void GetNameLabel(int type,int num);
void CheckPosts();
SAVEPAR *SRparam(int save,SAVEPAR *par);	//save or restore global param compiler
void AddRetList(int pos,int line,int type);
void CheckRealizable();
void declare_procedure(int oflag,int orm,int npointr);
void labelindata();
void AddRegistr(int razr,int reg);
void ClearRegister();
int GetRegister(int mode=0);
void RegAddNum(int reg);
void dowhilefast(unsigned int typeb);
int getrazr(int type);
void RestoreSaveReg();
void killlocals(/*int endp=TRUE*/);
void leaveproc();
int IsSaveReg();
void CorrectParamVar();

extern void ManyLogicCompare();
extern void maxdataerror();
extern void CompareOr();
extern void dynamiclabelerror();
extern void retvoid();
extern int numberbreak;

SAVEREG savereg;
SAVEREG *psavereg=&savereg;

int loadfile(char *filename,int firstflag)
{
	int hold;
	for(int i=0;i<=numfindpath;i++){
		char *path = findpath[(firstflag==0?i:numfindpath-i)]; // FIXME! (нужно выяснить, почему path может быть равен "\0")
		if(path && strlen(path)) {
			sprintf((char *)string2,"%s%s", path, filename);
		} else {
			strcpy((char *)string2, filename);
		}
#ifndef _WIN32_
		for(char* p=(char *)string2; *p; ++p) if(*p=='\\') *p='/';
#endif
		if((hold=loadinputfile((char *)string2))!=-2)break;
		if(firstflag==2||(firstflag==0&&(i+1)==numfindpath))break;
	}
	if(hold==-2){
		unableopenfile(filename); //сообщение о ошибке
		exit(e_cannotopeninput);	//завершить работу если не смогли загрузить файл
	}
	return hold;
}

void compilefile(char *filename,int firstflag)
{
int hold;

	hold=loadfile(filename,firstflag);
	if(hold==1||hold==-1)return;
	if(strcmp(filename,"startup.h--")==0)startupfile=currentfileinfo;

	inptr=0;
	endoffile=0;
	startline=(char*)input;
	endinput=startline+endinptr;
	warning=gwarning;
	nextchar();
	cha2=cha; //символ из буфера
	inptr2=inptr;	//запомн указатель на след символ
	linenum2=1;   //номер строки
	{	//проверка на файл ресурсов и его обработка
		char *a;
		if((a=strrchr(filename,'.'))!=NULL){
			if(stricmp(a,".rc")==0){
				input_res();
				free(input);
				return;
			}
		}
	}

	nexttok();    //опр тип первого и второго токена
	while(tok!=tk_eof){	//цикл пока не кончится файл
		while(tok==tk_question){
			directive();//обработка директив
			if(tok==tk_semicolon)nexttok();
		}
		usedirectiv=FALSE;
		if(notdoneprestuff==TRUE)doprestuff();//startup
		switch(tok){
			case tk_ID:
			case tk_id:
				if(FindTeg(TRUE)!=NULL){
					InitStruct();
					break;
				}
				if(tok2==tk_colon){
					labelindata();
					break;
				}
			case tk_far:
			case tk_cdecl:
			case tk_pascal:
			case tk_stdcall:
			case tk_fastcall:
			case tk_declare:
			case tk_undefproc:
			case tk_float:
			case tk_long:
			case tk_dword:
			case tk_word:
			case tk_byte:
			case tk_char:
			case tk_int:
			case tk_void:
			case tk_export:
			case tk_qword:
			case tk_double:
			case tk_fpust:
				if((hold=testInitVar())==FALSE)define_procedure();
				else if(hold==TRUE)globalvar();
				break;
			case tk_struct: InitStruct(); break;
			case tk_interrupt: interruptproc(); break;
			case tk_at: insertcode(); break;	//вставка регистровой процедуры
			case tk_colon:
				nexttok();
				dynamic_flag=2;
				break;// опр динамической  процедуры
			case tk_inline:
				if(testInitVar()){
					preerror("Bad header dynamic function");
					nexttok();
				}
				dynamic_proc();
				break;
			case tk_static:
				fstatic=2;
				nexttok();
				break;
			case tk_enum: doenum(); break;
			case tk_from: nexttok(); dofrom(); nextseminext(); break;
			case tk_extract: nexttok(); doextract(); seminext(); break;
			case tk_loop:
			case tk_while:
			case tk_do:
			case tk_else:
			case tk_ELSE:
			case tk_if:
			case tk_IF:
			case tk_interruptproc:
			case tk_proc:
			case tk_charvar:
			case tk_intvar:
			case tk_bytevar:
			case tk_longvar:
			case tk_dwordvar:
			case tk_floatvar:
			case tk_qwordvar:
			case tk_doublevar:
			case tk_wordvar: idalreadydefined(); break;
			case tk_reg32:
			case tk_debugreg:
			case tk_controlreg:
			case tk_testreg:
			case tk_reg:
			case tk_seg:
			case tk_beg:
			case tk_reg64:
				preerror("register name cannot be used as an identifier");
				nexttok();
			case tk_eof: break;
			case tk_locallabel: internalerror("local label token found outside function block."); break;
			case tk_extern: declareextern(); break;
			case tk_union: dynamic_flag=0; dounion(TRUE,fstatic==0?0:f_static); break;
			case tk_semicolon: nexttok(); break;
			case tk_asm:
				if(tok2==tk_openbrace)doasmblock();
				else doasm();
				break;
			case tk_idasm: doasm(TRUE); break;
			case tk_dollar:	doasm(FALSE); break;
			default: unuseableinput();
/*				while(itok.type==tp_stopper&&tok!=tk_eof)*/nexttok();
				break;
		}
		if(fstatic)fstatic--;
		else if(dynamic_flag)dynamic_flag--;
	}
	(startfileinfo+currentfileinfo)->stlist=staticlist;
	free(input);
}

/* ------------------- output procedures start ------------------- */
int CheckCodeSize()
//проверка размера буфера для кода
{
	if(!am32){
		maxoutputerror();
		return FALSE;
	}
	outptrsize+=MAXDATA;
	output=(unsigned char *)REALLOC(output,outptrsize);
	if(splitdata==FALSE)outputdata=output;
	return TRUE;
}

int CheckDataSize()
//проверка размера буфера для кода
{
	if(!am32){
		maxoutputerror();
		return FALSE;
	}
	outdatasize+=MAXDATA;
	outputdata=(unsigned char *)REALLOC(outputdata,outdatasize);
	return TRUE;
}

void  op(int byte)
{
	if(outptr>=outptrsize&&CheckCodeSize()==FALSE)return;
	output[outptr++]=(unsigned char)byte;
	if(splitdata==FALSE)outptrdata=outptr;
	retproc=FALSE;
}

void  opd(int byte)
{
	if(splitdata==FALSE){
		if(outptr>=outptrsize&&CheckCodeSize()==FALSE)return;
		output[outptr++]=(unsigned char)byte;
		outptrdata=outptr;
	}
	else{
		if(outptrdata>=outdatasize&&CheckDataSize()==FALSE)return;
		outputdata[outptrdata++]=(unsigned char)byte;
	}
}

void CorrectOfsBit(int bitofs)
{
	bitofs=(bitofs+7)/8;
	if(splitdata)outptrdata+=bitofs;
	else{
		outptr+=bitofs;
		outptrdata=outptr;
	}
}

long GetBitMask(int ofs,int size)
{
	return (~((li[size]-1)<<ofs));
}

void opb(unsigned long num,unsigned int ofs,unsigned int size)
{
int s;
//проверить выход за границы блока памяти
	s=(ofs+size+7)/8;
	if(splitdata==FALSE){
		if((outptr+s+8)>=outptrsize&&CheckCodeSize()==FALSE)return;
	}
	else{
		if((outptrdata+s+8)>=outdatasize&&CheckDataSize()==FALSE)return;
	}
	if(size!=32)num=num&(li[size]-1);
	s=outptrdata+ofs/8;
	ofs=ofs%8;
	*(long *)&outputdata[s]&=GetBitMask(ofs,size);
	*(long *)&outputdata[s]|=(num<<ofs);
//	printf("ofs=%Xh mask=%X value=%X\n",s,GetBitMask(ofs,size),(num<<ofs));
	if((ofs+size)>32){
		*(long *)&outputdata[s+4]&=GetBitMask(0,ofs+size-32);
		*(long *)&outputdata[s+4]|=(num>>(64-ofs-size));
//		printf("continue ofs=%Xh mask=%X value=%X\n",s+4,GetBitMask(0,ofs+size-32),num>>(64-ofs-size));
	}
}

void  outword(unsigned int num)
{
	op(num);
	op(num/256);
}

void  outwordd(unsigned int num)
{
	opd(num);
	opd(num/256);
}

void  outdword(unsigned long num)
{
	outword((unsigned int)(num&0xFFFFL));
	outword((unsigned int)(num/0x10000L));
}

void  outdwordd(unsigned long num)
{
	outwordd((unsigned int)(num&0xFFFFL));
	outwordd((unsigned int)(num/0x10000L));
}

void  outqword(unsigned long long num)
{
	outdword((unsigned long)(num&0xFFFFFFFFL));
	outdword((unsigned long)(num/0x100000000LL));
}

void  outqwordd(unsigned long long num)
{
	outdwordd((unsigned long)(num&0xFFFFFFFFL));
	outdwordd((unsigned long)(num/0x100000000LL));
}

void  doasmblock()
{
	nexttok();
	useasm=TRUE;
	expecting(tk_openbrace);
	for(;;){
		if(tok==tk_closebrace)break;
		if(tok==tk_eof){
			unexpectedeof();
			break;
		}
		lastcommand=tok;
		if(dbg)AddLine();
		doasm(TRUE);
	}
	useasm=FALSE;
	nexttok();
}

void doblock()
{
	expecting(tk_openbrace);
	doblock2();
/*	for(;;){
		if(tok==tk_closebrace)break;
		if(tok==tk_eof){
			unexpectedeof();
			break;
		}
		docommand();
	}
	RestoreStack();*/
}

void doblock2()
{
	for(;;){
		if(tok==tk_closebrace)break;
		if(tok==tk_eof){
			unexpectedeof();
			break;
		}
		docommand();
	}
	if(numblocks==1&&addstack&&sizestack&&localsize&&am32&&ESPloc&&IsSaveReg()==FALSE){
		localsize+=sizestack;
		sizestack=0;
	}
	else RestoreStack();
}

void gotodo()
{
	nexttok();
	if(gotol(0))nexttok();
	seminext();
}

void GOTOdo()
{
	nexttok();
	if(GOTO())nexttok();
	seminext();
}

void docommand()		 /* do a single command */
{
unsigned int useflag;
	useflag=0;
	if(dbg)AddLine();
//loops:
	lastcommand=tok;
//	printf("tok=%d %s\n",tok,itok.name);
	switch(tok){
		case tk_ID: useflag++;
		case tk_id:
			if((useflag=doid((char)useflag,tk_void))!=tokens){
				nextseminext();
				if(useflag==tk_fpust)preerror("function returned parametr in FPU stack");
			}
			else if(tok!=tk_closebrace)docommand();
			break;
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
			if(doanyundefproc()!=tokens)nextseminext();
			else if(tok!=tk_closebrace)docommand();
			break;
		case tk_proc:
			doanyproc();
			nextseminext();
			break;
		case tk_interruptproc:
			outword(0x0E9C);	//pushf //push cs
			useflag=itok.post;
			callloc(itok.number);
			nexttok();
			expecting(tk_openbracket);
			expecting(tk_closebracket);
#ifdef OPTVARCONST
			FreeGlobalConst();
#endif
			seminext();
			clearregstat(useflag);
			break;
		case tk_bits:
			dobits();
			break;
		case tk_charvar: useflag=1;
		case tk_bytevar:
			dobytevar(useflag);
			break;
		case tk_intvar: useflag=1;
		case tk_wordvar:
			do_d_wordvar(useflag,r16);
			break;
		case tk_longvar: useflag=1;
		case tk_dwordvar:
			do_d_wordvar(useflag,r32);
			break;
		case tk_doublevar:
			useflag=4;
		case tk_floatvar:
			dofloatvar(useflag,tk_floatvar,tk_semicolon);
			break;
		case tk_qwordvar:
			doqwordvar();
			break;
		case tk_fpust:
			dofloatstack(itok.number);
			break;
		case tk_structvar:
			dostruct();
			break;
		case tk_pointer:
			dopointer();
			break;
		case tk_mult:
			dovalpointer();
			break;
		case tk_RETURN:
		case tk_return:
			RestoreStack();
#ifdef OPTVARCONST
			ClearLVIC();
#endif
			doreturn(tok);
			CheckRealizable();
			break;
		case tk_at:
			nexttok();
			if(tok2==tk_colon){
				LLabel();
				if(tok!=tk_closebrace)docommand();
			}
			else if(macros(tk_void)!=0)nextseminext();
			break;
		case tk_if: RestoreStack(); doif();	break;
		case tk_IF: RestoreStack(); dobigif(); break;
		case tk_loopnz:
		case tk_LOOPNZ:
		case tk_loop: RestoreStack(); doloop(tok);	break;
		case tk_while:
		case tk_WHILE: RestoreStack(); dowhilefast(tok); break;
		case tk_do: RestoreStack(); dodo();	break;
		case tk_for:
		case tk_FOR: RestoreStack(); dofor(tok); break;
		case tk_reg32: doreg_32((unsigned int)itok.number,r32); break;
		case tk_reg: doreg_32((unsigned int)itok.number,r16); break;
		case tk_beg: dobeg((unsigned int)itok.number); break;
		case tk_reg64: doreg64(itok.number); break;
		case tk_seg: doseg((unsigned int)itok.number); break;
		case tk_openbrace:
			startblock();
			doblock();
			nexttok();
			endblock();
			break;
		case tk_from: nexttok(); dofrom(); nextseminext();	break;
		case tk_extract: nexttok(); doextract(); seminext(); break;
		case tk_minus: useflag=8;
		case tk_not:
			notnegit(useflag);
			nextseminext();
			break;
		case tk_locallabel: RestoreStack(); define_locallabel(); break;
		case tk_camma:
		case tk_semicolon: nexttok();	break;
		case tk_else:
			preerror("else without preceeding if or IF");
			nexttok();
			break;
		case tk_ELSE:
			preerror("ELSE without preceeding IF or if");
			nexttok();
			break;
		case tk_eof: unexpectedeof(); break;
		case tk_void:
		case tk_long:
		case tk_dword:
		case tk_word:
		case tk_byte:
		case tk_int:
		case tk_char:
			preerror("cannot declare variables within function { } block");
			nexttok();
			break;
		case tk_GOTO:
			RestoreStack();
			GOTOdo();
			CheckRealizable();
			break;
		case tk_goto:
			RestoreStack();
			gotodo();
			CheckRealizable();
			break;
		case tk_BREAK:
			RestoreStack();
			doBREAK(BREAK_SHORT);
			CheckRealizable();
			break;
		case tk_break:
			RestoreStack();
			doBREAK((unsigned char)(am32==FALSE?BREAK_NEAR:BREAK_32));
			CheckRealizable();
			break;
		case tk_CONTINUE:
			RestoreStack();
			doCONTINUE(CONTINUE_SHORT);
			CheckRealizable();
			break;
		case tk_continue:
			RestoreStack();
			doCONTINUE((unsigned char)(am32==FALSE?CONTINUE_NEAR:CONTINUE_32));
			CheckRealizable();
			break;
		case tk_asm:
			if(tok2==tk_openbrace)doasmblock();
			else doasm();
			break;
		case tk_idasm:
			useflag=TRUE;
		case tk_dollar:
			doasm(useflag);
			break;
		case tk_SWITCH:
		case tk_switch: RestoreStack(); doswitch(); break;
		case tk_delete: dodelete(); break;
		case tk_new: donew(); seminext(); break;
		case tk_question:
//			calcnumber=FALSE;
			while(tok==tk_question)directive();
			break;
/*		case tk_openbracket:
			nexttok();
			nexttok();
			expectingoperand(tk_closebracket);
			goto loops;*/
		default: unuseableinput(); break;
	}
	notunreach=FALSE;
}

void doBREAK(unsigned char typeb)
{
	if(curbr==0)preerror("'BREAK' or 'break' use only in loop, do-while..");
	else MakeBreak(typeb);
	nextseminext();
}

void doCONTINUE(unsigned char typeb)
{
	if(curco==0)preerror("'CONTINUE' or 'continue' use only in loop, do-while..");
	else MakeContinue(typeb);
	nextseminext();
}

void MakeBreak(unsigned char typeb)
{
unsigned int nbr=0;
	if(tok2==tk_number){
		nexttok();
		nbr=itok.number;
		if(nbr>=curbr)preerror("'BREAK' or 'break' on incorrect number skip cycle");
	}
	numberbreak=nbr;
	nbr=curbr-1-nbr;
	if(usebr[nbr]==0){
		GetNameLabel(tk_break,nbr);
		addlocalvar((char *)string2,tk_locallabel,secondcallnum,TRUE);
		usebr[nbr]=secondcallnum;
		secondcallnum++;
	}
	addacall(usebr[nbr],typeb);
	if(typeb==BREAK_SHORT)outword(0x00EB); 	// JMP SHORT
	else jumploc0();
}

void MakeContinue(unsigned char typeb)
{
unsigned int nbr=0;
	if(tok2==tk_number){
		nexttok();
		nbr=itok.number;
		if(nbr>=curco)preerror("'CONTINUE' or 'continue' on incorrect number skip cycle");
	}
	nbr=curco-1-nbr;
	if(useco[nbr]==0){
		GetNameLabel(tk_continue,nbr);
//		printf("%s nbr=%d\n",(char *)string2,nbr);
		addlocalvar((char *)string2,tk_locallabel,secondcallnum,TRUE);
		useco[nbr]=secondcallnum;
		secondcallnum++;
	}
	addacall(useco[nbr],typeb);
	if(typeb==CONTINUE_SHORT)outword(0x00EB); 	// JMP SHORT
	else jumploc0();
}

int CheckExitProc()
{
	if(strcmp(itok.name,"EXIT")==0||strcmp(itok.name,"ABORT")==0)return TRUE;
	return FALSE;
}

void LLabel()
{
localrec *ptr;
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	RestoreStack();
	clearregstat();
	switch(tok){
		case tk_id:
		case tk_ID:
			FindOff((unsigned char *)itok.name,CS);
			ptr=addlocalvar(itok.name,tk_number,outptr,TRUE);
			if(FixUp)ptr->rec.flag=f_reloc;
			break;
		case tk_undefproc:
			ptr=addlocalvar(itok.name,tk_number,outptr,TRUE);//добавить в локальный список
			if(FixUp)ptr->rec.flag=f_reloc;
			updatecall((unsigned int)itok.number,outptr,procedure_start);//обработать ранние обращения
			break;
		default:
			preerror("error declaretion local label");
			break;
	}
	nexttok();
	nexttok();
}

void AddApiToPost(unsigned int num)
{
	CheckPosts();
	(postbuf+posts)->type=CALL_32I;
	(postbuf+posts)->loc=outptr;
	(postbuf+posts)->num=num;
	posts++;
	outdword(0);
}

/* ---------------------- Procedure Calling Starts -------------------- */

int doanyundefproc(int jumpsend)
{
unsigned int cnum,snum;
int returnvalue;
int regs;
char fname[IDLENGTH];
	if(tok2==tk_colon){	// if a label
#ifdef OPTVARCONST
		ClearLVIC();
#endif
		RestoreStack();
		clearregstat();
		if(CidOrID()==tk_ID){//local label that has been used, but not placed
			localrec *ptr=addlocalvar(itok.name,tk_number,outptr,TRUE);
			if(FixUp)ptr->rec.flag=f_reloc;
			updatecall((unsigned int)itok.number,outptr,procedure_start);//обработать ранние обращения
		}
		else{	//глобальная метка
			tok=tk_proc;
			itok.number=outptr;
			string[0]=0;
			updatecall((unsigned int)updatetree(),(unsigned int)itok.number,0);
		}
		nexttok();	// move past id
		nexttok();	// move past :
		return(tokens);
	}
	if(tok2==tk_openbracket){
		strcpy(fname,itok.name);
		if(tok==tk_declare){	//сменить статус процедуры с объявленой на неизвестную
			tok=tk_undefproc;
			updatetree();
			if(itok.flag&f_classproc)AddUndefClassProc();
		}
		cnum=(unsigned int)itok.number;
		regs=itok.post;
		returnvalue=itok.rm;
		unsigned int tproc=itok.flag;
		unsigned char apiproc=FALSE;
		unsigned int oaddESP=addESP;
		int sizestack=-1;
		if(tok==tk_apiproc){
			apiproc=TRUE;
			sizestack=itok.size;	//размер стека под параметры
		}
#ifdef OPTVARCONST
		if(tproc&f_useidx)ClearLVIC();
		else FreeGlobalConst();
#endif
		int exitproc=CheckExitProc();
		snum=initparamproc();
		if(sizestack!=-1){
			if(snum>(unsigned int)sizestack)extraparam(fname);
			else if(snum<(unsigned int)sizestack)missingpar(fname);
		}
		if((tproc&f_typeproc)!=tp_cdecl){
			snum=0;
			addESP=oaddESP;
		}
		if(FastCallApi==TRUE&&apiproc!=FALSE){
			if(jumpsend)outword(0x25ff);
			else outword(0x15FF);
			AddApiToPost(cnum);
		}
		else{
			addacall(cnum,(unsigned char)((tproc&f_extern)!=0?CALL_EXT:(am32!=FALSE?CALL_32:CALL_NEAR)));
			if(jumpsend)jumploc0();
			else callloc0();		/* produce CALL [#] */
		}
		clearregstat(regs);
		if(snum!=0&&jumpsend==FALSE)CorrectStack(snum);
		retproc=exitproc;
		return(returnvalue);
	}
	thisundefined(itok.name);
	nexttok();
	return(tk_long);
}

void CorrectStack(unsigned int num)
{
	if(addstack){
		sizestack+=num;
//		printf("%s(%d)> Add %d bytes stacks.\n",startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,linenumber,num);
	}
	else{
		if(short_ok(num)){
			outword(0xC483);
			op(num);
		}
		else{
			outword(0xC481);
			if(am32==FALSE)outword(num);
			else outdword(num);
		}
		addESP-=num;
	}
}

unsigned int initparamproc()
{
unsigned int typep=itok.flag,snum=0;
ITOK ostructadr=structadr;
	strcpy(param,(char *)string);
	nexttok();
	switch(typep&f_typeproc){
		case tp_cdecl:
		case tp_stdcall:
			snum=swapparam();
			break;
		case tp_pascal:
			doparams();
			break;
		case tp_fastcall:
			doregparams();
			break;
	}
//	if(crec)printf("after doparams num=%08X\n",crec->recnumber);
	if((typep&f_classproc)&&(!(typep&f_static))){
		if((current_proc_type&f_static)&&structadr.sib==THIS_PARAM                            )return snum;
		structadr=ostructadr;
		if(structadr.sib==THIS_PARAM){
			if(structadr.number==0){
				op(0xff);
				if(ESPloc&&am32){
					int num;
					num=localsize+addESP+4;
					if(short_ok(num,TRUE)){
						outword(0x2474);
						op(num);
					}
					else{
						outword(0x24B4);
						outdword(num);
					}
				}
				else outword(am32==FALSE?0x0476:0x0875);//push[ebp+4]
			}
			else{
				int reg=GetRegister(1);
				op(0x8B);
				if(ESPloc&&am32){
					int num;
					num=localsize+addESP+4;
					if(short_ok(num,TRUE)){
						op(4+reg*8+64);
						op(0x24);
						op(num);
					}
					else{
						op(4+reg*8+128);
						op(0x24);
						outdword(num);
					}
				}
				else{
					op((am32==FALSE?6:5)+reg*8+64);
					op((am32==FALSE?4:8));//mov ESI,[ebp+4]
				}
				RegAddNum(reg);
				op(0x50+reg);	//push reg
				warningreg(regs[am32][reg]);
			}
		}
		else if(structadr.sib==THIS_REG){
			if(structadr.number/*size*/!=0){
				int reg=GetRegister(1);
				if(reg==structadr.rm)RegAddNum(reg);
				else{
					if(am32==FALSE){
						switch(structadr.rm){
							case BX:
								structadr.rm=7;
								break;
							case DI:
								structadr.rm=5;
								break;
							case SI:
								structadr.rm=4;
								break;
							case BP:
								structadr.rm=6;
								break;
							default:
								regBXDISIBPexpected();
						}
						structadr.sib=CODE16;
					}
					else structadr.sib=CODE32;
					structadr.rm|=(structadr.number<128?rm_mod01:rm_mod10);
					op(0x8d);	//lea reg [reg2+num]
					op(structadr.rm+reg*8);
					outaddress(&structadr);
				}
				op(0x50+reg);
				warningreg(regs[am32][reg]);
			}
			else op(0x50+structadr.rm);
		}
		else if(structadr.sib==THIS_NEW){
			RunNew(structadr.number);
			op(0x50);
		}
		else if(structadr.sib==THIS_ZEROSIZE){
			outword(0x6a);	//push 0
		}
		else{
//			printf("post=%d\n",structadr.post);
			if(structadr.post==LOCAL){
				int reg=GetRegister(1);
				structadr.post=0;
				outseg(&structadr,2);
				op(0x8d);
				op(structadr.rm+reg*8);
				outaddress(&structadr);
				op(0x50+reg);
				warningreg(regs[am32][reg]);
			}
			else{
				if(strinf.bufstr){
					int reg=GetRegister(1);
					int newreg;
					if((newreg=CheckIDXReg(strinf.bufstr,strinf.size,reg))!=NOINREG){
						if(newreg!=SKIPREG){
							if(am32==FALSE&&newreg!=BX&&newreg!=DI&&newreg!=SI&&newreg!=BP)goto noopt;
							waralreadinitreg(regs[am32][reg],regs[am32][newreg]);
							reg=newreg;
						}
						free(strinf.bufstr);
						goto cont1;
					}
noopt:
					if(newreg=CheckMassiv(strinf.bufstr,strinf.size,reg)!=-1)reg=newreg;
cont1:
					strinf.bufstr=NULL;
					RegAddNum(reg);
					op(0x50+reg);
				}
				else{
					op(0x68);
					if(structadr.post/*&&structadr.post!=USED_DIN_VAR*/)setwordpost(&structadr);
					else if(FixUp)AddReloc();
					if(am32==FALSE)outword((unsigned int)structadr.number);
					else outdword(structadr.number);
				}
			}
		}
		snum+=(am32==FALSE?2:4);
		addESP+=(am32==FALSE?2:4);
	}
	if(typep&f_far)op(0x0e);	//push cs
	return snum;
}

int doanyproc(int jumpsend)
{
unsigned int cloc,snum;
int returnvalue,dynamicindex;
int regs;
	if(tok2==tk_colon){
		preerror("dublication global label");
		nexttok();
		return 0;
	}
	cloc=(unsigned int)itok.number;	 /* get address or handle */
	returnvalue=itok.rm;
	regs=itok.post;
//	printf("regs=%08X name=%s\n",regs,itok.name);
	int flag=itok.flag;
	if(itok.npointr)dopointerproc();
	else{
		if((itok.flag&f_inline)!=0&&(useinline==TRUE||(useinline==2&&optimizespeed))){
			if(macros(tk_void)!=0)return(returnvalue);
		}
		dynamicindex=itok.segm;

//	printf("%s %08X seg=%d\n",rec->recid/*itok.name*/,itok.flag,itok.segm);
		if(itok.segm==DYNAMIC){
			itok.segm=DYNAMIC_USED;
			updatetree();
		}
		unsigned int oaddESP=addESP;
		snum=initparamproc();
		if((flag&f_typeproc)!=tp_cdecl){
			snum=0;
			addESP=oaddESP;
		}
		if(dynamicindex<NOT_DYNAMIC){	//динамическая процедура
			addacall(cloc,(unsigned char)(am32!=FALSE?CALL_32:CALL_NEAR));
			if(jumpsend)jumploc0();
			else{
				callloc0();
				if(snum!=0)CorrectStack(snum);
			}
		}
		else{
			if(jumpsend)jumploc(cloc);
			else{
				callloc(cloc);
				if(snum!=0)CorrectStack(snum);
			}
		}
	}
#ifdef OPTVARCONST
	if(flag&f_useidx)ClearLVIC();
	else FreeGlobalConst();
#endif
	clearregstat(regs);
	return(returnvalue);
}

int  doid(char uppercase,int expectedreturn)
{
int cnum;
	if(tok2==tk_colon){	// if a label
#ifdef OPTVARCONST
		ClearLVIC();
#endif
		RestoreStack();
		clearregstat();
		cnum=FindOff((unsigned char *)itok.name,CS);
		if(uppercase){
			localrec *ptr=addlocalvar(itok.name,tk_number,outptr,TRUE);
			if(FixUp)ptr->rec.flag=f_reloc;
		}
		else{
			tok=tk_proc;
			itok.rm=tk_void;
			itok.number=outptr;
			itok.segm=NOT_DYNAMIC;
			itok.flag=0;
			string[0]=0;
			itok.type=tp_ucnovn;
			addtotree(itok.name);
			itok.rec->count=cnum;
		}
		nexttok();	// move past id
		nexttok();	// move past :
		return(tokens);
	}
	if(tok2==tk_openbracket){
		if((cnum=CheckMacros())!=tokens)return cnum;
		tobedefined(am32==FALSE?CALL_NEAR:CALL_32,expectedreturn);
		cnum=posts-1;
		param[0]=0;
		int flag=itok.flag;
		int exitproc=CheckExitProc();
		unsigned int oaddESP=addESP;
		if(itok.flag==tp_stdcall){
			nexttok();
			swapparam();
		}
		else{
			nexttok();
			if(uppercase)doregparams();
			else doparams();
		}
		(postbuf+cnum)->loc=outptr+1;
		callloc0();			/* produce CALL [#] */
		clearregstat();
		addESP=oaddESP;
#ifdef OPTVARCONST
		if(flag&f_useidx)ClearLVIC();
		else FreeGlobalConst();
#endif
		retproc=exitproc;
		return(expectedreturn);
	}
	thisundefined(itok.name);
	return(tk_long);
}

int typesize(int vartype) // возвращает размер в байтах кода возврата
{
	switch(vartype){
		case tk_char:
		case tk_byte:  return(1);
		case tk_int:
		case tk_word:  return(2);
		case tk_float:
		case tk_dword:
		case tk_long:  return(4);
		case tk_double:
		case tk_qword: return 8;
	}
	return(0);
}

void FpuSt2Number()
{
	op66(r32);  //push EAX
	op(0x50);
	CheckInitBP();
	fistp_stack();
	RestoreBP();
	fwait3();
	op66(r32);
	op(0x58);	//pop EAX
	if(cpu<3)cpu=3;
}

void FpuSt2QNumber()
{
	op66(r32);  //push EAX
	op(0x50);
	op66(r32);  //push EAX
	op(0x50);
	CheckInitBP();
	fistp_stack(4);
	RestoreBP();
	fwait3();
	op66(r32);
	op(0x58+EDX);	//pop EAX
	op66(r32);
	op(0x58);	//pop EAX
	if(cpu<3)cpu=3;
}

void fwait3_4()
{
	if(chip<4)op(0x9B);
}

void  convert_returnvalue(int expectedreturn,int actualreturn)
{
	if(expectedreturn==tk_void)return; //17.09.05 17:52
	if(actualreturn==tk_void/*||expectedreturn==tk_void*/){
		retvoid();
		return;
	}
	switch(expectedreturn){
		case tk_byte:
		case tk_char:
		case tk_word:
		case tk_int:
		  if(actualreturn==tk_float||actualreturn==tk_double){
				op66(r32);
				op(0x50);
				FloatToNumer(actualreturn==tk_float?0:4);
			}
			else if(actualreturn==tk_fpust)FpuSt2Number();
			else if(expectedreturn==tk_word||expectedreturn==tk_int){
				if(actualreturn==tk_char)cbw();
				else if(actualreturn==tk_byte)xorAHAH();
			}
			break;
		case tk_long:
		case tk_dword:
			switch(actualreturn){
				case tk_char:
					op66(r32);
					op(0x0F); outword(0xC0BE);	//MOVSX EAX,AL
					break;
				case tk_byte: 	/* MOVZX EAX,AL */
					op66(r32);
					op(0x0F); outword(0xC0B6);
					break;
				case tk_word:  /* MOVZX EAX,AX */
					op66(r32);
					op(0x0F); outword(0xC0B7);
					break;
				case tk_int:	/* MOVSX EAX,AX */
					op66(r32);
					op(0x0F); outword(0xC0BF);
					break;
				case tk_double:
				case tk_fpust:
					FpuSt2Number();
					break;
//				case tk_double:
				case tk_float:
					op66(r32);
					op(0x50);
					FloatToNumer(/*actualreturn==tk_float?0:4*/);
					break;
			}
			if(cpu<3)cpu=3;
			break;
		case tk_qword:
			switch(actualreturn){
				case tk_char:
					op66(r32);
					op(0x0F); outword(0xC0BE);	//MOVSX EAX,AL
					cwdq(r32);
					break;
				case tk_byte: 	/* MOVZX EAX,AL */
					op66(r32);
					op(0x0F); outword(0xC0B6);
					ZeroReg(EDX,r32);
					break;
				case tk_word:  /* MOVZX EAX,AX */
					op66(r32);
					op(0x0F); outword(0xC0B7);
				case tk_dword:
					ZeroReg(EDX,r32);
					break;
				case tk_int:	/* MOVSX EAX,AX */
					op66(r32);
					op(0x0F); outword(0xC0BF);
				case tk_long:
					cwdq(r32);
					break;
				case tk_fpust:
				case tk_double:
					FpuSt2QNumber();
					break;
				case tk_float:
					op66(r32);
					op(0x50);
					FloatToNumer(actualreturn==tk_float?0:4);
					cwdq(r32);
					break;
			}
			if(cpu<3)cpu=3;
			break;
		case tk_fpust:
			if(tok2==tk_semicolon)break;
			switch(actualreturn){
				case tk_char:
					CheckInitBP();
					cbw();
					op66(r32);
					outword(0xDF50);	//push EAX
					goto endfxld;	//fild ss[bp-4]/[esp]
				case tk_byte:
					CheckInitBP();
					xorAHAH();
					op66(r32);
					outword(0xDF50);	//push EAX
					goto endfxld;	//fild ss[bp-4]/[esp]
				case tk_word:
					CheckInitBP();
					op66(r16);
					outword(0x6A);  //push 0
					op66(r16);
					outword(0xDB50);	//push AX
					goto endfxld;	//fild ss[bp-4]/[esp]
				case tk_int:
					CheckInitBP();
					op66(r32);
					outword(0xDF50);	//push eax
					goto endfxld;	//fild ss[bp-4]/[esp]
				case tk_dword:
					CheckInitBP();
					op66(r32); 		//push 0L
					outword(0x6A);
					op66(r32);
					op(0x50);	//push EAX
					fildq_stack();
					RestoreBP();
					op66(r32);
					op(0x58);	//pop eax
					op66(r32);
					op(0x58);	//pop eax
					break;
				case tk_long:
					CheckInitBP();
					op66(r32);
					outword(0xDB50);	//push EAX
					goto endfxld;	//fild ss[bp-4]/[esp]
				case tk_float:
					CheckInitBP();
					op66(r32);
					outword(0xd950);	//push EAX
endfxld:
					fld_stack(4+localsize);
					RestoreBP();
					op66(r32);
					op(0x58);	//pop eax
					break;
				case tk_qword:
//				case tk_double:
					CheckInitBP();
					op66(r32);
					op(0x50+EDX);	//push EDX
					op66(r32);
					op(0x50);	//push EAX
/*					if(actualreturn==tk_double){
						op(0xDD);
						fld_stack(8+localsize);
					}
					else*/ fildq_stack();
					RestoreBP();
					op66(r32);
					op(0x58);	//pop eax
					op66(r32);
					op(0x58);	//pop eax
					break;
			}
			if(cpu<3)cpu=3;
			break;
		default:
//			printf("expectedreturn=%d %s %d\n",expectedreturn,(startfileinfo+currentfileinfo)->filename,linenumber);
			break;
	}
}

int  procdo(int expectedreturn)
{
int actualreturn;
char idflag=0;
	switch(tok){
		case tk_ID:	idflag++;
		case tk_id:
		 	actualreturn=doid(idflag,expectedreturn);
			break;
		case tk_proc:
			actualreturn=doanyproc();
			break;
		case tk_apiproc:
		case tk_undefproc:
		case tk_declare:
//			if((actualreturn=doanyundefproc())==tk_void)actualreturn=expectedreturn;
			actualreturn=doanyundefproc(); //17.09.05 17:56
			break;
		default: internalerror("Bad tok in procdo();"); break;
	}
	convert_returnvalue(expectedreturn,actualreturn);
	return actualreturn;
}

/* +++++++++++++++++++++++ loops and ifs start ++++++++++++++++++++++++ */

void endcmpfloat()
{
	fwait3();
	outword(0xE0DF);//fstsw ax
	op(0x9E);
	RestoreBP();
}

int  outcmp(int swapped,int ctok,ITOK *cstok,char *&cbuf,SINFO *cstr,int ctok2,ITOK *cstok2,char *&cbuf2,SINFO *cstr2,int typet)
{
unsigned char err=0;
int typef=0;
int vop=0;
long long lnumber;
unsigned int ofs;
int i,reg,reg1;
	if(typet<r16)typet=r16;
	switch(ctok){
		case tk_reg64:
			reg=cstok->number/256;
			switch(ctok2){
				case tk_reg64:
					reg1=cstok2->number/256;
					for(i=0;i<2;i++){
						op66(r32);
					  op(0x39);	//cmp reg,reg
						op(0xC0+reg+reg1*8);
						if(i==1)break;
						outword(0x75);
						ofs=outptr;
						reg=cstok->number&255;
						reg1=cstok2->number&255;
					}
					output[ofs-1]=outptr-ofs;
					break;
				case tk_number:
				case tk_postnumber:
				case tk_undefofs:
					lnumber=cstok2->lnumber>>32;
					for(i=0;i<2;i++){
						op66(r32);
					//проверка на возможность более короткого кода
						if((cstok2->flag&f_reloc)==0&&ctok2!=tk_postnumber&&ctok2!=tk_undefofs&&
							short_ok(lnumber,TRUE)){
							if(!lnumber){
								op(0x85);	//test reg,reg
								op(0xc0+reg*9);
							}
							else{
								op(0x83);	//cmp reg,
								op(0xF8+reg);
								op(lnumber);
							}
						}
						else{
							if(reg==AX)op(0x3D);
							else{
								op(0x81);
								op(0xF8+reg);
							}
							if(i==1){
								if(ctok2==tk_postnumber)(cstok2->flag&f_extern)==0?setwordpost(cstok2):setwordext(&cstok2->number);
								else if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
								if(ctok2==tk_undefofs)AddUndefOff(2,cstok2->name);
							}
							outdword(cstok2->number);
						}
						if(i==1)break;
						outword(0x75);
						ofs=outptr;
						reg=cstok->number&255;
					}
					output[ofs-1]=outptr-ofs;
					break;
				default:
					if(swapped)err=1;
					else return(outcmp(1,ctok2,cstok2,cbuf2,cstr2,ctok,cstok,cbuf,cstr,typet));
					break;
			}
			break;
		case tk_reg32:
		case tk_reg:
			switch(ctok2){
				case tk_reg:
				case tk_reg32:
					if(ctok!=ctok2)err=1;
					else{
						op66(typet);
					  op(0x39);	//cmp reg,reg
						op(0xC0+(unsigned int)cstok->number+(unsigned int)cstok2->number*8);
					}
					break;
				case tk_number:
					if(cstok2->number==0&&(cstok2->flag&f_reloc)==0){
		 				op66(typet);
						op(0x85);	//test reg,reg
						op(0xc0+(unsigned int)cstok->number*9);
						break;
					}
				case tk_postnumber:
				case tk_undefofs:
					op66(typet);
					//проверка на возможность более короткого кода
					if((cstok2->flag&f_reloc)==0&&ctok2!=tk_postnumber&&ctok2!=tk_undefofs&&
							short_ok(cstok2->number,ctok==tk_reg?FALSE:TRUE)){
						op(0x83);	//cmp reg,
						op(0xF8+(unsigned int)cstok->number);
						op(cstok2->number);
						break;
					}
					if(cstok->number==AX)op(0x3D);
					else{
						op(0x81);
						op(0xF8+(unsigned int)cstok->number);
					}
					if(ctok2==tk_postnumber)(cstok2->flag&f_extern)==0?setwordpost(cstok2):setwordext(&cstok2->number);
					else if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
					if(ctok2==tk_undefofs)AddUndefOff(2,cstok2->name);
					if(ctok==tk_reg)outword((unsigned int)cstok2->number);
					else outdword(cstok2->number);
					break;
				default:
					if(swapped)err=1;
					else return(outcmp(1,ctok2,cstok2,cbuf2,cstr2,ctok,cstok,cbuf,cstr,typet));
					break;
			}
			break;
		case tk_qwordvar:
			cstok->number+=4;
			compressoffset(cstok);
			switch(ctok2){
				case tk_postnumber:
				case tk_number:
				case tk_undefofs:
					lnumber=cstok2->lnumber>>32;
					CheckAllMassiv(cbuf,8,cstr,cstok);
					for(i=0;i<2;i++){
						op66(r32);
						outseg(cstok,2);
					//проверка на возможность более короткого кода
						if((cstok2->flag&f_reloc)==0&&ctok2!=tk_postnumber&&ctok2!=tk_undefofs&&
								short_ok(lnumber,1)){
							op(0x83);
							op(0x38+cstok->rm);
							outaddress(cstok);
							op(lnumber);
						}
						else{
							op(0x81);
							op(0x38+cstok->rm);
							outaddress(cstok);
							if(i==1){
								if(ctok2==tk_postnumber)(cstok2->flag&f_extern)==0?setwordpost(cstok2):setwordext(&cstok2->number);
								else if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
								if(ctok2==tk_undefofs)AddUndefOff(2,cstok2->name);
							}
							outdword(lnumber);
						}
						if(i==1)break;
						outword(0x75);
						ofs=outptr;
						cstok->number-=4;
						compressoffset(cstok);
						lnumber=cstok2->lnumber;
					}
					output[ofs-1]=outptr-ofs;
					break;
				case tk_reg64:
					CheckAllMassiv(cbuf,8,cstr,cstok);
					reg=cstok2->number/256;
					for(i=0;i<2;i++){
						op66(r32);
						outseg(cstok,2);
						op(0x39); /* CMP [word],AX */
						op(cstok->rm+reg*8);
						outaddress(cstok);
						if(i==1)break;
						reg=cstok2->number&255;
						outword(0x75);
						ofs=outptr;
						cstok->number-=4;
						compressoffset(cstok);
					}
					output[ofs-1]=outptr-ofs;
					break;
				default:
					i=EAX|(EDX*256);
					getintoreg64(i);
					doregmath64(i);
					CheckAllMassiv(cbuf,8,cstr,cstok);
					reg=EDX;
					for(i=0;i<2;i++){
						op66(r32);
						outseg(cstok,2);
						op(0x39); /* CMP [word],AX */
						op(cstok->rm+reg*8);
						outaddress(cstok);
						if(i==1)break;
						reg=EAX;
						outword(0x75);
						ofs=outptr;
						cstok->number-=4;
						compressoffset(cstok);
					}
					output[ofs-1]=outptr-ofs;
					break;
			}
			break;
		case tk_intvar:
		case tk_wordvar:
			if(swapped&&typet==r32)typet=r16;
		case tk_longvar:
		case tk_dwordvar:
			switch(ctok2){
				case tk_reg32:
				case tk_reg:
					CheckAllMassiv(cbuf,typet,cstr,cstok);
					op66(typet);
				  outseg(cstok,2);
					op(0x39);
					op((unsigned int)cstok2->number*8+cstok->rm);
					outaddress(cstok);
					break;
				case tk_postnumber:
				case tk_number:
				case tk_undefofs:
					CheckAllMassiv(cbuf,typet,cstr,cstok);
					op66(typet);
					outseg(cstok,2);
					//проверка на возможность более короткого кода
					if((cstok2->flag&f_reloc)==0&&ctok2!=tk_postnumber&&ctok2!=tk_undefofs&&
							short_ok(cstok2->number,typet/2-1)){
						op(0x83);
						op(0x38+cstok->rm);
						outaddress(cstok);
						op(cstok2->number);
						break;
					}
					op(0x81);
					op(0x38+cstok->rm);
					outaddress(cstok);
					if(ctok2==tk_postnumber)(cstok2->flag&f_extern)==0?setwordpost(cstok2):setwordext(&cstok2->number);
					else if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
					if(ctok2==tk_undefofs)AddUndefOff(2,cstok2->name);
					if(typet==r16)outword((unsigned int)cstok2->number);
					else outdword(cstok2->number);
					break;
				case tk_charvar:
				case tk_intvar:
					getinto_e_ax(1,ctok2,cstok2,cbuf2,cstr2,typet);
					CheckAllMassiv(cbuf,typet,cstr,cstok);
					op66(typet);
					outseg(cstok,2);
					op(0x39);	/* CMP [word],AX */
					op(cstok->rm);
					outaddress(cstok);
					break;
				default:
					getinto_e_ax(0,ctok2,cstok2,cbuf2,cstr2,typet);
//					ClearReg(AX);
					CheckAllMassiv(cbuf,typet,cstr,cstok);
					op66(typet);
					outseg(cstok,2);
					op(0x39); /* CMP [word],AX */
					op(cstok->rm);
					outaddress(cstok);
					break;
			}
			break;
		case tk_number:
			if(ctok2==tk_postnumber){
				op(0xB8); 	 /* MOV AX,# */
				if((cstok->flag&f_reloc)!=0)AddReloc(cstok->segm);
				if(am32==FALSE)outword((unsigned int)cstok->number);
				else outdword(cstok->number);
				op(0x3D);		 /* CMP AX,# */
				(cstok2->flag&f_extern)==0?setwordpost(cstok2):setwordext(&cstok2->number);
				if(am32==FALSE)outword((unsigned int)cstok2->number);
				else outdword(cstok2->number);
			}
			if(ctok2==tk_number){
				if(cstok2->rm!=tk_float&&cstok->rm!=tk_float){
					if((unsigned long)cstok2->number<256&&(unsigned long)cstok->number<256){
						op(0xB0);	//mov al,number
						op(cstok->number);
						op(0x3C);	//cmp Al,number
						op(cstok2->number);
					}
					else if((unsigned long)cstok2->number<65536&&               cstok->number<65536){
						op66(r16);
						op(0xB8); /* MOV AX,# */
						if((cstok->flag&f_reloc)!=0)AddReloc(cstok->segm);
						outword((unsigned int)cstok->number);
						op66(r16);
						op(0x3D);	/* CMP AX,# */
						if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
						outword((unsigned int)cstok2->number);
					}
					else{
						op66(r32);
						op(0xB8); /* MOV AX,# */
						if((cstok->flag&f_reloc)!=0)AddReloc(cstok->segm);
						outdword(cstok->number);
						op66(r32);
						op(0x3D);	/* CMP AX,# */
						if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
						outdword(cstok2->number);
					}
				}
				else{
					op(0x55);       //push bp
					outword(0xe589);//mov bp,sp
					op66(r32);
					if(short_ok(cstok->number,TRUE)){	//push num
						op(0x6A);
						op(cstok->number);
					}
					else{
						op(0x68);
						outdword(cstok->number);
					}
					op66(r32);
					if(short_ok(cstok2->number,TRUE)){	//push num
						op(0x6A);
						op(cstok2->number);
					}
					else{
						op(0x68);
						outdword(cstok2->number);
					}
					outword(am32==FALSE?0x46d9:0x45d9);
					op(0xfC);//fld ssdword[bp-4]
					op(0xD8);
					outword(0xF85e - am32);	//fcomp [bp-8]
					endcmpfloat();
				}
			}
			else{
				if(swapped)err=1;
				else return(outcmp(1,ctok2,cstok2,cbuf2,cstr2,ctok,cstok,cbuf,cstr,typet));
			}
			break;
		case tk_postnumber:
			if(ctok2==tk_number||ctok2==tk_postnumber){
				op(0xB8); /* MOV AX,# */
				(cstok->flag&f_extern)==0?setwordpost(cstok):setwordext(&cstok->number);
				if(am32==FALSE)outword((unsigned int)cstok->number);
				else outdword(cstok->number);
				op(0x3D);	/* CMP AX,# */
				if(ctok2==tk_postnumber)(cstok2->flag&f_extern)==0?setwordpost(cstok2):setwordext(&cstok2->number);
				else if((cstok2->flag&f_reloc)!=0)AddReloc(cstok2->segm);
				if(am32==FALSE)outword((unsigned int)cstok2->number);
				else outdword(cstok2->number);
			}
			else{
				if(swapped)err=1;
				else return(outcmp(1,ctok2,cstok2,cbuf2,cstr2,ctok,cstok,cbuf,cstr,typet));
			}
			break;
		case tk_charvar:
		case tk_bytevar:
			switch(ctok2){
				case tk_number:
					CheckAllMassiv(cbuf,1,cstr,cstok);
					outseg(cstok,2);
					op(0x80);  /* CMP [byte],# */
					op(0x38+cstok->rm);
					outaddress(cstok);
					op((unsigned int)cstok2->number);
					break;
				case tk_reg:
				case tk_reg32:
					if(cstok2->number>3)goto defchar;
				case tk_beg:
					CheckAllMassiv(cbuf,1,cstr,cstok);
					outseg(cstok,2);
					op(0x38);	 /* CMP [byte],beg */
					op((unsigned int)cstok2->number*8+cstok->rm);
					outaddress(cstok);
					break;
				default:
defchar:
					getintoal(ctok2,cstok2,cbuf2,cstr2);
					CheckAllMassiv(cbuf,1,cstr,cstok);
					outseg(cstok,2);
					op(0x38); 	/* CMP [byte],AL */
					op(cstok->rm);
					outaddress(cstok);
					break;
			}
			break;
		case tk_beg:
			switch(ctok2){
				case tk_number:
					if(cstok2->number==0){
						op(0x84);	//test beg,beg
						op(0xc0+(unsigned int)cstok->number*9);
						break;
					}
					if((unsigned int)cstok->number==AL)op(0x3C);
					else{
						op(0x80);
						op(0xF8+(unsigned int)cstok->number);
					}
					op((unsigned int)cstok2->number);
					break;
				case tk_beg:
					op(0x38);
					op(0xC0+(unsigned int)cstok->number+(unsigned int)cstok2->number*8);
					break;
				case tk_reg:
					if((unsigned int)cstok2->number<=BX){	/* CMP beg,beg */
						op(0x38);
						op(0xC0+(unsigned int)cstok->number+(unsigned int)cstok2->number*8);
					}
					else{
						op66(r16);
						op(0x89);					/* MOV AX,reg */
						op(0xC0+(unsigned int)cstok2->number*8);
						op(0x38); 			/* CMP beg,AL */
						op(0xC0+(unsigned int)cstok->number);
					}
					break;
				default:
					if(swapped)err=1;
					else return(outcmp(1,ctok2,cstok2,cbuf2,cstr2,ctok,cstok,cbuf,cstr,typet));
				break;
			}
			break;
		case tk_doublevar:
			vop=4;
			goto cont_float;
		case tk_fpust:
			typef++;
			if(cstok->type==tp_modif)typef++;
			else{
				if(cstok->number!=0){
					op(0xd9);	//fld st(x)
					op(0xC0+cstok->number);
					typef++;
				}
			}
		case tk_floatvar:
cont_float:
			switch(ctok2){
				case tk_beg:
					CheckInitBP();
					switch(cstok2->rm){
						case tk_char:
						case tk_int:
						case tk_long:
							outword(0xBE0F);	/* MOVSX AX,beg */
							op(0xC0+(unsigned int)cstok2->number);
							break;
						default:
							if((optimizespeed&&chip>3&&chip<7)||cstok2->number==AL){
								xorAHAH();
								if(cstok2->number!=AL){
									op(0x88);
									op(0xC0+cstok2->number*8);	//mov al,beg
								}
							}
							else{
								outword(0xB60F);	// MOVZX AX,beg
								op(0xC0+(unsigned int)cstok2->number);
							}
							break;
					}
					outword(0xDF50);	//push AX
					fld_stack(2+localsize);
					if(typef==2)outword(0xD9DE);	//FCOMPP
					else if(typef==1)outword(0xD9D8);	//FCOMP
					else{
 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					op(0x58);	// pop EAX
					endcmpfloat();
					swapped=1;
					break;
				case tk_reg:
					CheckInitBP();
					op66(r32);
					switch(cstok2->rm){
						case tk_char:
						case tk_int:
						case tk_long:
							outword(0xBF0F);	/* MOVSX EAX,reg */
							break;
						default:
							if(optimizespeed&&chip>3&&chip<7&&cstok2->number!=AX){
								outword(0xC031);	// xor EAX,EAX
								op66(r16);
								op(0x8B);
							}
							else outword(0xB70F);	// MOVZX EAX,reg
							break;
					}
					op(0xC0+(unsigned int)cstok2->number);
					op66(r32);
					outword(0xDB50);	//push EAX
					fld_stack(4+localsize);
					if(typef==2)outword(0xD9DE);	//FCOMPP
					else if(typef==1)outword(0xD9D8);	//FCOMP
					else{
 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					op66(r32);
					op(0x58);	// pop EAX
					endcmpfloat();
					swapped=1;
					break;
				case tk_reg32:
					CheckInitBP();
					if(cstok2->rm==tk_float){
						op66(r32);
						op(0x50+(unsigned int)cstok2->number);	//push reg32
						op(0xd9);
						fld_stack(4+localsize);
						if(typef==2)outword(0xD9DE);	//FCOMPP
						else if(typef==1)outword(0xD9D8);	//FCOMP
						else{
	 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
							CheckAllMassiv(cbuf,4+vop,cstr,cstok);
							outseg(cstok,2);	//fcomp var
							op(0xd8+vop);
							op(cstok->rm+0x18);
							outaddress(cstok);
						}
						op66(r32);
						op(0x58);	// pop EAX
					}
					else{
						if(cstok2->rm!=tk_char&&cstok2->rm!=tk_int&&cstok2->rm!=tk_long){
							typet=tk_word;
							op66(r32);
							outword(0x6a);	//$push 0
	 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
						}
						op66(r32);
						op(0x50+cstok2->number); //push reg32
						if(typet!=r16)fildq_stack();
						else{
							op(0xdb);
							fld_stack(4+localsize);
						}
						if(typef==2)outword(0xD9DE);	//FCOMPP
						else if(typef==1)outword(0xD9D8);	//FCOMP
						else{
	 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
							CheckAllMassiv(cbuf,4+vop,cstr,cstok);
							outseg(cstok,2);	//fcomp var
							op(0xd8+vop);
							op(cstok->rm+0x18);
							outaddress(cstok);
						}
						if(typet!=r16){
							if(optimizespeed||am32==FALSE){
								outword(0xC483);
								op(8);
							}
							else{
								op(0x58);	// pop EAX
								op(0x58);	// pop EAX
							}
						}
						else{
							op66(r32);
							op(0x58);	// pop EAX
						}
					}
					endcmpfloat();
					swapped=1;
					break;
				case tk_charvar:
					CheckAllMassiv(cbuf2,1,cstr2,cstok2);
					outseg(cstok2,3);	/* MOVSX AX,[charvar] */
					outword(0xBE0F); op(cstok2->rm);
					outaddress(cstok2);
					CheckInitBP();
					outword(0xdf50);	//push ax
					fld_stack(2+localsize);
					if(typef==2)outword(0xD9DE);	//FCOMPP
					else if(typef==1)outword(0xD9D8);	//FCOMP
					else{
 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					op(0x58);	// pop EAX
					endcmpfloat();
					swapped=1;
					break;
				case tk_intvar:
					CheckAllMassiv(cbuf2,2,cstr2,cstok2);
					outseg(cstok2,2);
					if(typef){
						op(0xDE);	//ficomp var2
						op(cstok2->rm+0x08+typef*8);
						outaddress(cstok2);
					}
					else{
						op(0xdf);	//fild var
						op(cstok2->rm);
						outaddress(cstok2);
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
						swapped=1;
					}
					fwait3_4();
					outword(0xE0DF);//fstsw ax
					op(0x9E);
					break;
				case tk_bytevar:
					CheckAllMassiv(cbuf2,1,cstr2,cstok2);
					if(optimizespeed&&chip>3&&chip<7){
						outword(0xC031);
						outseg(cstok2,2);
						op(0x8A);
					}
					else{
						outseg(cstok2,3);
						outword(0xB60F);
					}
					op(cstok2->rm); // MOVZX regL,[byte]
					outaddress(cstok2);
					CheckInitBP();
					outword(0xDF50);	//push ax
					fld_stack(2+localsize);
					if(typef==2)outword(0xD9DE);	//FCOMPP
					else if(typef==1)outword(0xD9D8);	//FCOMP
					else{
 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					op(0x58);	// pop EAX
					endcmpfloat();
					swapped=1;
					break;
				case tk_wordvar:
					CheckInitBP();
					op66(r16);
					outword(0x6a);  //push 0
					CheckAllMassiv(cbuf2,2,cstr2,cstok2);
					op66(r16);
					outseg(cstok2,2); //push var
					op(0xFF);
					op(cstok2->rm+0x30);
					outaddress(cstok2);
					op(0xDB);
					fld_stack(4+localsize);
					if(typef==2)outword(0xD9DE);	//FCOMPP
					else if(typef==1)outword(0xD9D8);	//FCOMP
					else{
 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=4;
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					op66(r32);
					op(0x58);	// pop EAX
					endcmpfloat();
					swapped=1;
					break;
				case tk_dwordvar:
					CheckInitBP();
					op66(r32);	//push 0L
					outword(0x6a);
					CheckAllMassiv(cbuf2,4,cstr2,cstok2);
					op66(r32);	//push var
					outseg(cstok2,2);
					op(0xFF);
					op(cstok2->rm+0x30);
					outaddress(cstok2);
					fildq_stack();
					if(typef==2)outword(0xD9DE);	//FCOMPP
					else if(typef==1)outword(0xD9D8);	//FCOMP
					else{
 						if(ESPloc&&am32&&cstok->segm==SS)cstok->number+=8;
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					if(optimizespeed||am32==FALSE){
						outword(0xC483);
						op(8);
					}
					else{
						op(0x58);	// pop EAX
						op(0x58);	// pop EAX
					}
					endcmpfloat();
					swapped=1;
					break;
				case tk_longvar:
					CheckAllMassiv(cbuf2,4,cstr2,cstok2);
					outseg(cstok2,2);
					if(typef){
						op(0xDA);	//ficomp var2
						op(cstok2->rm+0x08+typef*8);
						outaddress(cstok2);
					}
					else{
						op(0xdb);	//fild var
						op(cstok2->rm);
						outaddress(cstok2);
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
						swapped=1;
					}
					endcmpfloat();
					break;
				case tk_number:
					if(!typef){
						CheckAllMassiv(cbuf,4,cstr,cstok);
						outseg(cstok,2);	//fld val
						op(0xd9);
						op(cstok->rm);
						outaddress(cstok);
					}
					if(cstok2->rm!=tk_double&&cstok2->rm!=tk_float){
						cstok2->dnumber=cstok2->lnumber;
						cstok2->rm=tk_double;
					}
					else if(vop==4&&cstok2->rm==tk_float){
						cstok2->dnumber=cstok2->fnumber;
						cstok2->rm=tk_double;
					}
					if(am32&&(cstok2->rm==tk_float&&cstok2->fnumber==0.0)||
					(cstok2->rm==tk_double&&cstok2->dnumber==0.0)||cstok2->lnumber==0){
						outword(0xe4d9);	//ftst
						if(typef!=1)outword(0xC0DD);	//ffree
					}
					else{
						op66(r32);
						int rm;
						rm=(am32==FALSE?0x1eD8:0x1DD8);
						if(typef==1)rm-=0x800;
						if(cstok2->rm==tk_double||vop==4)rm+=4;
						outword(rm);	//fcom(p)
						AddFloatConst(cstok2->lnumber,cstok2->rm);
						outword(0);
						if(am32)outword(0);
					}
					endcmpfloat();
					break;
				case tk_floatvar:
					if(!typef){
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fld val
						op(0xd9+vop);
						op(cstok->rm);
						outaddress(cstok);
					}
					CheckAllMassiv(cbuf2,4,cstr2,cstok2);
					outseg(cstok2,2);	//fcomp var
					op(0xd8);
					op(cstok2->rm+(typef==1?0x10:0x18));
					outaddress(cstok2);
					endcmpfloat();
					break;
				case tk_doublevar:
					if(!typef){
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fld val
						op(0xd9+vop);
						op(cstok->rm);
						outaddress(cstok);
					}
					CheckAllMassiv(cbuf2,8,cstr2,cstok2);
					outseg(cstok2,2);	//fcomp var
					op(0xd8+4);
					op(cstok2->rm+(typef==1?0x10:0x18));
					outaddress(cstok2);
					endcmpfloat();
					break;
				case tk_fpust:
					if(typef==2){	//fcomp
						if(chip>=7){	//fcomip
							op(0xDF);
							op(0xF0+cstok2->number+1);
							swapped=1;
							break;
						}
						op(0xD8);
						op(0xD8+cstok2->number+1);
					}
					else if(typef==1){	//fcom
						if(chip>=7){	//fcomi
							op(0xDB);
							op(0xF0+cstok2->number);
							swapped=1;
							break;
						}
						op(0xD8);
						op(0xD0+cstok2->number);
					}
					else{
						if(cstok2->type!=tp_modif){
							op(0xd9);	//fld st(x)
							op(0xC0+cstok2->number);
						}
						CheckAllMassiv(cbuf,4+vop,cstr,cstok);
						outseg(cstok,2);	//fcomp var
						op(0xd8+vop);
						op(cstok->rm+0x18);
						outaddress(cstok);
					}
					endcmpfloat();
					swapped=1;
					break;
				default:
					err=1;
					break;
			}
			break;
		default: err=1; break;
	}
	if(err)preerror("unable to create comparison, check restrictions");
	return(swapped);
}

int CheckCompareTok(int reg)
{
int comparetok=tk_equalto;
	if(itok.type==tp_compare)comparetok=tok;
	else unknowncompop();
	getoperand(reg);
	return comparetok;
}

int typenumber(int vtok)
{
	switch(vtok){
		case tk_char:
		case tk_beg:
		case tk_byte:
			return tk_byte;
		case tk_int:
		case tk_reg:
		case tk_word:
			return tk_word;
		case tk_reg64:
		case tk_qword:
			return tk_qword;
		case tk_floatvar:
		case tk_float:
			return tk_float;
		case tk_doublevar:
		case tk_double:
			return tk_double;
	}
	return tk_dword;
}

#ifdef OPTVARCONST
int constructcompare(int invertflag,unsigned int startloc,LVIC *comconst)
#else
int constructcompare(int invertflag,unsigned int startloc)
#endif
/* build cmp for IF, if and do {} while */
{
int comparetok=0,jumptype,vartype=tokens,notflag=FALSE;
int ittok,ittok2=tokens,type2=tokens;//,retcompare=tokens;
int razr=r_undef;
char *ibuf,*ibuf2;
ITOK htok,htok2;
SINFO hstr,hstr2;
int preg=(am32==TRUE?EAX:SI);
int use_cxz=0;
char *ofsstr=NULL,*ofsstr2=NULL;
int usereg=-1;
int usereg2=-1;

//////04.10.04 13:45
int bracket=0;
unsigned char oinline=useinline;
	useinline=0;
	do{
		nexttok();
//		printf("tok=%d tok2=%d\n",tok,tok2);
		if(tok==tk_openbracket)bracket++;
		if(tok==tk_not)notflag=(notflag+1)%2;
	}while(tok2==tk_openbracket||tok2==tk_not);
	if(bracket==0)expected('(');
/////////////////

	setzeroflag=FALSE;
	ofsstr=GetLecsem(tk_closebracket,tk_eof,tp_compare);
	getoperand();	//NEW 04.10.04 13:45
	if(tok==tk_openbracket){
		bracket++;
		getoperand();
	}
	if(tok==tk_not){
		notflag=(notflag+1)%2;
		getoperand();
	}
	switch(tok){
		case tk_closebracket:
			useinline=oinline;
			getoperand();
			return voidcompr;
		case tk_asm:
			if(tok2==tk_openbrace){
				nexttok();
				type2=tok;
			}
		case tk_dollar:
			nexttok();
		case tk_idasm:
			if(stricmp(itok.name,"test")==0){
				if(iTest(1)==FALSE)InvOperComp();
				ittok=0x75;
				if(type2==tk_openbrace)expecting(tk_closebrace);
				goto endcomp;
			}
			else preerror("Only 'TEST' possible use in compare");
			break;
		case tk_qword:
		case tk_double:
			razr+=4;
		case tk_beg:
		case tk_reg32:
		case tk_reg: vartype=tok; break;
		case tk_float:
		case tk_long:
		case tk_dword:
			razr+=2;
		case tk_int:
		case tk_word:
			razr++;
		case tk_char:
		case tk_byte:
			razr++;
			vartype=tok;
			getoperand();
			if(tok==tk_closebracket&&bracket>1){
				bracket--;
				getoperand();
			}
			break;
		case tk_undefproc:
		case tk_declare:
//			if(itok.rm==tk_void)itok.rm=(am32==FALSE?tk_word:tk_dword);
		case tk_proc:
		case tk_apiproc:
			vartype=itok.rm;
			if(vartype==tokens)vartype=(am32==FALSE?tk_word:tk_dword);
			else if(vartype==tk_void&&(itok.flag&f_retproc)==0){
				retvoid();
				vartype=itok.rm=(am32==FALSE?tk_word:tk_dword);
			}
			if(ofsstr){
				free(ofsstr);
				ofsstr=NULL;
			}
			break;
		case tk_bytevar: vartype=tk_byte;	break;
		case tk_charvar: vartype=tk_char; break;
		case tk_intvar: vartype=tk_int; break;
		case tk_wordvar: vartype=tk_word; break;
		case tk_dwordvar: vartype=tk_dword; break;
		case tk_longvar: vartype=tk_long; break;
		case tk_floatvar: vartype=tk_float; break;
		case tk_qwordvar: vartype=tk_qword; break;
		case tk_fpust:
		case tk_doublevar: vartype=tk_double; break;
		case tk_bits:
			int i;
			i=itok.bit.ofs+itok.bit.siz;
			if(i<=64){
				vartype=tk_dword;
				razr=r64;
			}
			if(i<=32){
				vartype=tk_dword;
				razr=r32;
			}
			if(i<=16){
				vartype=tk_word;
				razr=r16;
			}
			if(i<=8){
				vartype=tk_byte;
				razr=r8;
			}
			break;
		case tk_at:
			if(ofsstr){
				free(ofsstr);
				ofsstr=NULL;
			}
			nexttok();
			if(itok.flag&f_retproc){
				comparetok=(itok.flag&f_retproc)/256+tk_overflowflag-1;
//				notflag=(notflag==FALSE?TRUE:FALSE);
				vartype=tk_ID;
				ittok2=macros(vartype);
				if(tok2==tk_closebracket){
					nexttok();
					ittok=0x70+comparetok-tk_overflowflag;
					goto endcomp;
				}
				if(ittok2==0){
					vartype=itok.rm;
					break;
				}
				goto mac1;
			}
			if((itok.flag&f_typeproc)==tp_fastcall&&itok.segm!=NOT_DYNAMIC)vartype=itok.rm;
			else vartype=tk_ID;
			if((ittok2=macros(vartype))==0){
				vartype=itok.rm;
				break;
			}
mac1:
			vartype=ittok2;
			switch(vartype){
			 	case tk_byte:
				case tk_char:	tok=tk_beg; break;
				case tk_int:
				case tk_word:	tok=tk_reg; break;
				case tk_float:
				case tk_dword:
				case tk_long:	tok=tk_reg32; break;
				case tk_double:
				case tk_qword: tok=tk_reg64; break;
				default:
					preerror("Macro has a return type of void");
					tok=tk_reg;
					vartype=tk_word;
					break;
			}
			itok.number=AX;  // or AL or EAX
			break;
		default:
			if((tok>=tk_overflowflag)&&(tok<=tk_plusflag)){
				ittok=0x70+tok-tk_overflowflag;
				nexttok();
				if(tok!=tk_closebracket&&(tok==tk_oror||tok==tk_andand||tok==tk_notequal||tok==tk_equalto)){
					int oper;
					int oper2;
					oper=tok;
					nexttok();
					ittok^=notflag;
					notflag=0;
					if(tok==tk_not){
						notflag=TRUE;
						nexttok();
					}
					if((tok>=tk_overflowflag)&&(tok<=tk_plusflag)){
						ittok-=0x70;
						ittok2=tok-tk_overflowflag;
						ittok2^=notflag;
						notflag=0;
						nexttok();
						switch(oper){
							case tk_oror:
								if((ittok==2&&ittok2==4)||(ittok==4&&ittok2==2))ittok=6;
								else if(ittok==4&&(ittok2==8||ittok2==0))ittok=(ittok|ittok2)+1;
								else unknowncompop();
								break;
							case tk_andand:
								if((ittok==3&&ittok2==5)||(ittok==5&&ittok2==3))ittok=7;
								else if(ittok==5&&(ittok2==8||ittok2==0))ittok=(ittok|ittok2)+1;
								else unknowncompop();
								break;
							case tk_notequal:
								if((ittok==8&&ittok2==0)||(ittok==0&&ittok2==8))ittok=12;
								else unknowncompop();
								break;
							case tk_equalto:
								if((ittok==8&&ittok2==0)||(ittok==0&&ittok2==8))ittok=13;
								else unknowncompop();
								break;
						}
						if(tok!=tk_closebracket&&(tok==tk_notequal||tok==tk_equalto)){
							oper2=tok;
							nexttok();
							if(tok==tk_not){
								notflag=TRUE;
								nexttok();
							}
							if((tok>=tk_overflowflag)&&(tok<=tk_plusflag)){
								ittok2=tok-tk_overflowflag;
								ittok2^=notflag;
								notflag=0;
								nexttok();
								if(oper2==tk_notequal){
									if(oper==tk_oror&&((ittok==5&&ittok2==8)||(ittok==13&&ittok2==0)))ittok=14;
									else unknowncompop();
								}
								else{
									if(oper==tk_andand&&((ittok==6&&ittok2==8)||(ittok==14&&ittok2==0)))ittok=15;
									else unknowncompop();
								}
							}
							else unknowncompop();
						}
					}
					else unknowncompop();
					ittok+=0x70;
				}
				goto endcomp;
			}
			vartype=(am32==FALSE?tk_word:tk_dword);
			break;
	}
	CheckMinusNum();
	if(itok2.type!=tp_compare&&tok2!=tk_closebracket){	//сложный операнд
		if(ofsstr){
			int retreg;
			razr=getrazr(vartype);
			if((retreg=CheckIDZReg(ofsstr,AX,razr))!=NOINREG){
				GetEndLex(tk_closebracket,tk_semicolon,tp_compare);
				nexttok();
				if(razr==r16)ittok=tk_reg;
				else if(razr==r32)ittok=tk_reg32;
				else ittok=tk_beg;
				usereg=htok.number=retreg==SKIPREG?AX:retreg;
				goto nn1;
			}
		}
		comparetok=0;//используется временно не посмыслу
		ittok=tok;
		htok=itok;
		ibuf=NULL;
		hstr.bufstr=NULL;
		ittok2=tok2;
		preg=BX;
		switch(tok2){
			case tk_assign:
			case tk_plusplus:
			case tk_minusminus:
			case tk_divequals:
			case tk_minusequals:
			case tk_multequals:
			case tk_plusequals:
				switch(tok){
					case tk_charvar: comparetok=1;
					case tk_bytevar:
						if((comparetok=dobytevar(comparetok,0))==tk_reg||comparetok==tk_beg){
							usereg=htok.number=AX;
							ittok=tk_beg;
						}
						break;
					case tk_intvar: comparetok=1;
					case tk_wordvar:
						if((comparetok=do_d_wordvar(comparetok,r16,0))==tk_reg){
							usereg=htok.number=AX;
							ittok=tk_reg;
						}
						break;
					case tk_longvar: comparetok=1;
					case tk_dwordvar:
						if((comparetok=do_d_wordvar(comparetok,r32,0))==tk_reg32){
							usereg=htok.number=AX;
							ittok=tk_reg32;
						}
						break;
					case tk_floatvar:
						if(dofloatvar(0,tk_fpust,0)==tk_fpust){
							ittok=tk_fpust;
							htok.type=tp_modif;
						}
						break;
					case tk_qwordvar:
						if((comparetok=doqwordvar(0))==tk_reg64){
							ittok=tk_reg64;
							usereg=htok.number=EAX|(EDX*256);
						}
						break;
					case tk_reg64:
						usereg=itok.number;
						getintoreg64(itok.number);
						doregmath64(itok.number);
						comparetok=tk_reg64;
						break;
					case tk_reg32:
						usereg=itok.number;
						comparetok=doreg_32((unsigned int)itok.number,r32,0);
//						printf("comparetok=%d\n",comparetok);
						break;
					case tk_reg:
						usereg=itok.number;
						comparetok=doreg_32((unsigned int)itok.number,r16,0);
						break;
					case tk_beg:
						usereg=itok.number;
						comparetok=dobeg((unsigned int)itok.number,0);
						break;
					default: InvOperComp(); break;
				}
				if(ittok<tk_floatvar&&(!(comparetok>=tk_overflowflag&&comparetok<=tk_plusflag))){
					if(ittok2!=tk_assign&&ittok2!=tk_divequals&&ittok2!=tk_multequals){
						if(tok==tk_closebracket){
							ittok=0x75;
							goto endcomp;
						}
						if(tok2==tk_number&&itok2.number==0){
							if((ittok2==tk_plusplus||ittok2==tk_minusminus)&&
									tok!=tk_notequal&&tok!=tk_equalto)break;
							comparetok=CheckCompareTok(BX);
							nexttok();
							goto createopcode;
						}
					}
				}
				break;
			default:
				switch(vartype){
					case tk_int: comparetok=do_e_axmath(1,r16,&ofsstr); ittok=tk_reg; break;
					case tk_reg:
					case tk_word: comparetok=do_e_axmath(0,r16,&ofsstr); ittok=tk_reg; break;
					case tk_char: comparetok=doalmath(1,&ofsstr); ittok=tk_beg; break;
					case tk_beg:
					case tk_byte: comparetok=doalmath(0,&ofsstr); ittok=tk_beg; break;
					case tk_long: comparetok=do_e_axmath(1,r32,&ofsstr); ittok=tk_reg32; break;
					case tk_reg32:
					case tk_dword:
						comparetok=do_e_axmath(0,r32,&ofsstr);
						ittok=tk_reg32;
						break;
					case tk_qword:
						usereg=htok.number=EAX|(EDX*256);
						getintoreg64(usereg);
						doregmath64(usereg);
						comparetok=ittok=tk_reg64;
						break;
					case tk_float:
						doeaxfloatmath(tk_fpust);
						ittok=tk_fpust;
						htok.type=tp_modif;
						break;
					default:
						if(itok.flag&f_retproc){
							comparetok=(itok.flag&f_retproc)/256+tk_overflowflag-1;
//							printf("tok=%d flag=%08X comparetok=%u %s\n",tok,itok.flag,comparetok,itok.name);
//							notflag=(notflag==FALSE?TRUE:FALSE);
							switch(tok){
								case tk_undefproc:
								case tk_declare:
								case tk_apiproc:
									doanyundefproc();
									break;
								case tk_proc:
									doanyproc();
									break;
							}
							nexttok();
							if(tok!=tk_closebracket){
								retvoid();
								do{
									nexttok();
								}while(tok!=tk_closebracket);
							}
						}
						else{
							internalerror("Bad vartype value in constructcompare();");
							tok=tk_reg; break;
						}
				}
				if(ittok!=tk_reg64)usereg=htok.number=AX; 			// same value as AL and EAX
		}
		RestoreStack();
	}
	else{
#ifdef OPTVARCONST
		CheckConstVar3(&tok,&itok,razr);
#endif
		if(tok>=tk_charvar&&tok<=tk_doublevar){
			switch(vartype){
				case tk_int: tok=tk_intvar; break;
				case tk_word: tok=tk_wordvar; break;
				case tk_char: tok=tk_charvar; break;
				case tk_byte: tok=tk_bytevar; break;
				case tk_long: tok=tk_longvar; break;
				case tk_dword: tok=tk_dwordvar; break;
				case tk_float: tok=tk_floatvar; break;
				case tk_qword: tok=tk_qwordvar; break;
				case tk_double: tok=tk_doublevar; break;
			}
		}
		else if(tok==tk_number){
			if(tok2==tk_closebracket){
				invertflag=(itok.number==0?zerocompr:voidcompr);
				nexttok();
				getoperand();
				useinline=oinline;
				return invertflag;
			}
			if(itok.rm==tk_float)vartype=tk_float;
		}
		else if(tok==tk_bits){
			bits2reg(AX,razr);
			switch(razr){
				case r64:
				case r32:
					tok=tk_reg32;
					break;
				case r16:
					tok=tk_reg;
					break;
				case r8:
					tok=tk_beg;
					break;
			}
			itok.number=0;
		}
		if(tok==tk_beg||tok==tk_reg||tok==tk_reg32)itok.rm=vartype;	//тип содержимого в reg32
		ittok=tok;
		htok=itok;
		ibuf=bufrm;
		bufrm=NULL;
		hstr=strinf;
		strinf.bufstr=NULL;
		nexttok();
	}
nn1:
	if(razr==r_undef){
		switch(vartype){
			case tk_qword:
			case tk_double:
				razr+=4;
			case tk_long:
			case tk_dword:
			case tk_float:
			case tk_reg32:
				razr+=2;
			case tk_int:
			case tk_word:
			case tk_reg:
				razr++;
			case tk_char:
			case tk_byte:
			case tk_beg:
				razr++;
		}
	}
	if(tok!=tk_closebracket){	//сравнение
		ofsstr2=GetLecsem(tk_closebracket);
		comparetok=CheckCompareTok(preg);
		if(tok>=tk_char&&tok<=tk_double){
			type2=tok;
			if(ofsstr2)free(ofsstr2);
			ofsstr2=GetLecsem(tk_closebracket);
			getoperand(preg);
		}
		if(tok==tk_minus){
			if(CheckMinusNum()==FALSE){
				preerror("only negative of constants valid within compairsons");
				nexttok();
			}
		}
#ifdef OPTVARCONST
		CheckConstVar3(&tok,&itok,razr);
#endif
		if(tok==tk_number){
			switch(vartype){
				case tk_long:
				case tk_int:
				case tk_char:
					htok2.number=doconstlongmath();
					itok.flag=(unsigned char)postnumflag;
					break;
				case tk_dword:
				case tk_reg32:
				case tk_beg:
				case tk_reg:
				case tk_word:
				case tk_byte:
					htok2.number=doconstdwordmath();
					itok.flag=(unsigned char)postnumflag;
					break;
				case tk_float:
					htok2.number=doconstfloatmath();
					break;
				case tk_reg64:
				case tk_qword:
					htok2.lnumber=doconstqwordmath();
					itok.flag=(unsigned char)postnumflag;
					break;
				case tk_double:
					htok2.lnumber=doconstdoublemath();
					break;
			}
			htok2.rm=typenumber(vartype);
			ittok2=tk_number;
			htok2.flag=itok.flag;
		}
		else{
			if(ittok>=tk_charvar&&ittok<=tk_doublevar&&(tok==tk_proc||tok==tk_id||
					tok==tk_undefproc||tok==tk_declare||tok==tk_apiproc||tok==tk_ID||
					itok2.type==tp_opperand||(tok>=tk_charvar&&tok<=tk_doublevar))){
				if(ofsstr2){
					int retreg;
					razr=getrazr(vartype);
					if((retreg=CheckIDZReg(ofsstr2,AX,razr))!=NOINREG){
						GetEndLex(tk_closebracket);
						usereg2=retreg==SKIPREG?AX:retreg;
						if(razr==r16)ittok2=tk_reg;
						else if(razr==r32)ittok2=tk_reg32;
						else ittok2=tk_beg;
						htok2.number=usereg2;
						nexttok();
						goto en2;
					}
				}
				int sign=0;
				switch(ittok){
					case tk_charvar: sign=1;
					case tk_bytevar:
						doalmath(sign,&ofsstr2);
						usereg2=htok2.number=AX;
						ittok2=tk_beg;
						break;
					case tk_intvar: sign=1;
					case tk_wordvar:
						do_e_axmath(sign,r16,&ofsstr2);
						usereg2=htok2.number=AX;
						ittok2=tk_reg;
						break;
					case tk_longvar: sign=1;
					case tk_dwordvar:
						do_e_axmath(sign,r32,&ofsstr2);
						usereg2=htok2.number=AX;
						ittok2=tk_reg32;
						break;
					case tk_floatvar:
						doeaxfloatmath(tk_fpust);
						ittok2=tk_fpust;
						htok2.type=tp_modif;
						htok2.number=0;
						ClearReg(AX);
						break;
					case tk_doublevar:
						doeaxfloatmath(tk_fpust,0,4);
						ittok2=tk_fpust;
						htok2.type=tp_modif;
						htok2.number=0;
						ClearReg(AX);
						break;
					case tk_qwordvar:
						usereg2=htok2.number=EAX|(EDX*256);
						getintoreg64(usereg2);
						doregmath64(usereg2);
						ittok2=tk_reg64;
						ClearReg(AX);
						ClearReg(DX);
						break;
				}
			}
			else{
				if(tok==tk_bits){
					int i=itok.bit.ofs+itok.bit.siz;
					int vops;
					if(i<=64)vops=r64;
					if(i<=32)vops=r32;
					if(i<=16)vops=r16;
					if(i<=8)vops=r8;
					if(vops<razr)vops=razr;
					i=AX;
					if((ittok==tk_reg32||ittok==tk_reg||ittok==tk_beg)&&htok.number==0)i=CX;
					bits2reg(i,vops);
					switch(razr){
						case r64:
						case r32:
							tok=tk_reg32;
							break;
						case r16:
							tok=tk_reg;
							break;
						case r8:
							tok=tk_beg;
							break;
					}
					itok.number=i;
					ClearReg(i);
				}
				switch(tok){
					case tk_beg:
					case tk_reg:
					case tk_reg32:
						itok.rm=type2;	//тип содержимого в reg32
						if((ittok==tk_reg32||ittok==tk_reg||ittok==tk_beg)&&
								htok.number==itok.number)preerror("Comparison two identical registers");
						break;
				}
				int next=TRUE;
				int sign=0;
				if(ittok==tk_reg32){
					if(ofsstr2){
						int retreg;
						int treg;
						treg=(htok.number==0?DX:AX);
						razr=r32;
						if((retreg=CheckIDZReg(ofsstr2,treg,r32))!=NOINREG){
							if(retreg==SKIPREG)retreg=treg;
							if(retreg!=htok.number){
								GetEndLex(tk_closebracket);
								ittok2=tk_reg32;
								htok2.number=usereg2=retreg;
								nexttok();
								goto en2;
							}
						}
					}
					switch(tok){
						case tk_intvar:
							sign=1;
						case tk_wordvar:
							if(htok.number!=0){
								do_e_axmath(sign,r32,&ofsstr2);
								usereg2=itok.number=0;
							}
							else{
								getintoreg_32(DX,r32,sign,&ofsstr2);
								usereg2=itok.number=DX;
							}
				 			warningreg(regs[1][itok.number]);
							next=FALSE;
							ittok2=tk_reg32;
							htok2=itok;
					}
				}
				if(next){
					ittok2=tok;
					htok2=itok;
					ibuf2=bufrm;
					bufrm=NULL;
					hstr2=strinf;
					strinf.bufstr=NULL;
					nexttok();
				}
			}
			RestoreStack();
		}
	}
	else{	// !=0
		if((comparetok>=tk_overflowflag)&&(comparetok<=tk_plusflag)){
			ittok=0x70+comparetok-tk_overflowflag;
			goto endcomp;
		}
		htok2.rm=typenumber(vartype);
		comparetok=tk_notequal;
		ittok2=tk_number;
		htok2.number=0;
		htok2.flag=0;
	}
	if(ittok2==tk_number&&htok2.number==0&&(htok2.flag&f_reloc)==0){
		if(setzeroflag){
			if(comparetok==tk_notequal){
				ittok=0x75;
				goto endcomp;
			}
			if(comparetok==tk_equalto){
				ittok=0x74;
				goto endcomp;
			}
		}
		if(htok.number==CX&&optimizespeed==0){
			if(ittok==tk_reg||ittok==tk_reg32){
				if(comparetok==tk_notequal)use_cxz=notflag==0?cxnzcompr:cxzcompr;
				else if(comparetok==tk_equalto)use_cxz=notflag==TRUE?cxnzcompr:cxzcompr;
			}
		}
	}
en2:
	if(ittok>=tk_charvar&&ittok<=tk_floatvar){
		if(ofsstr){
			int retreg;
			razr=getrazr(vartype);
			if((retreg=CheckIDZReg(ofsstr,AX,razr))!=NOINREG){
				usereg=retreg==SKIPREG?AX:retreg;
				if(!((ittok2==tk_reg||ittok2==tk_reg32||ittok2==tk_beg)&&usereg==htok2.number)){
					if(razr==r16)ittok=tk_reg;
					else if(razr==r32)ittok=tk_reg32;
					else ittok=tk_beg;
					htok.number=usereg;
					if(ibuf){
						free(ibuf);
						ibuf=NULL;
					}
					if(hstr.bufstr){
						free(hstr.bufstr);
						hstr.bufstr=NULL;
					}
				}
				else{
					usereg=-1;
				}
			}
			else{
				if(ittok==tk_floatvar)ClearReg(AX);
				else if(ittok>=tk_intvar&&ittok<tk_floatvar){
					switch(ittok2){
						case tk_reg:
						case tk_reg32:
						case tk_number:
						case tk_postnumber:
						case tk_undefofs:
							break;
						default:
							usereg2=0;
							break;
					}
				}
				else{
					switch(ittok2){
						case tk_reg:
						case tk_reg32:
							ClearReg(AX);
						case tk_number:
						case tk_beg:
							break;
						default:
							usereg2=0;
							break;
					}
				}
			}
		}
	}
	if(ittok==tk_number&&htok.flag==0&&ittok2==tk_number&&htok2.flag==0){
		invertflag=(htok.number!=htok2.number?zerocompr:voidcompr);
		useinline=oinline;
		return invertflag;
	}
	if((ittok==tk_number||ittok==tk_postnumber)&&(ittok2==tk_number||ittok2==tk_postnumber))usereg=0;
#ifdef OPTVARCONST
	if(comconst){
		if(comparetok==tk_equalto||comparetok==tk_notequal){
			if(ittok>=tk_charvar&&ittok<=tk_doublevar&&ittok2==tk_number&&
				(htok2.flag&f_reloc)==0&&htok.rec&&(htok.flag&f_useidx)==0){
				comconst->rec=htok.rec;
				comconst->lnumber=htok2.lnumber;
				comconst->contype=htok2.rm;
			}
			else if(ittok2>=tk_charvar&&ittok2<=tk_doublevar&&ittok==tk_number&&
				(htok.flag&f_reloc)==0&&htok2.rec&&(htok2.flag&f_useidx)==0){
				comconst->rec=htok2.rec;
				comconst->lnumber=htok.lnumber;
				comconst->contype=htok.rm;
			}
			comconst->typevar=comparetok;
			if(notflag)comconst->typevar=(comparetok==tk_equalto?tk_notequal:tk_equalto);
		}
	}
#endif
	if(outcmp(0,ittok,&htok,ibuf,&hstr,ittok2,&htok2,ibuf2,&hstr2,razr)){
		switch(comparetok){
			case tk_less:	comparetok=tk_greater; break;
			case tk_lessequal: comparetok=tk_greaterequal; break;
			case tk_greater: comparetok=tk_less; break;
			case tk_greaterequal: comparetok=tk_lessequal; break;
		}
	}
createopcode:
	jumptype=0;
	if(vartype==tk_char||vartype==tk_int||vartype==tk_long)jumptype=1;
	switch(comparetok){
		case tk_equalto: ittok=0x74; break;
		case tk_notequal: ittok=0x75; break;
		case tk_greater:
			ittok=(jumptype==0?0x77:0x7F);
			break;
		case tk_less:
			ittok=(jumptype==0?0x72:0x7C);
			break;
		case tk_greaterequal:
			ittok=(jumptype==0?0x73:0x7D);
			break;
		case tk_lessequal:
			ittok=(jumptype==0?0x76:0x7E);
			break;
		default: unknowncompop(); break;
	}
endcomp:
	if(ofsstr){
		if(usereg!=-1)IDZToReg(ofsstr,usereg,razr);
		free(ofsstr);
	}
	if(ofsstr2){
//		printf("usereg2=%08X %s\n",usereg2,ofsstr2);
		if(usereg2!=-1)IDZToReg(ofsstr2,usereg2,razr);
		free(ofsstr2);
	}
	if(invertflag==2)invertflag=((outptr+2-startloc)>128?1:0);
	ittok^=invertflag;
	ittok^=notflag;
	op(ittok);  /* output instruction code */
	expecting(tk_closebracket);
	useinline=oinline;
	return invertflag|use_cxz;
}

#ifdef OPTVARCONST
ICOMP *compare(int type,unsigned int *numcomp,REGISTERSTAT **bakreg,REGISTERSTAT **changereg,LVIC *comconst)
#else
ICOMP *compare(int type,unsigned int *numcomp,REGISTERSTAT **bakreg,REGISTERSTAT **changereg)
#endif
{
unsigned int i;
ICOMP *icomp;
int j=0;
int ifline=linenumber;
int ptok=tk_oror;
int rcompr;
int useor=FALSE;
REGISTERSTAT *bakregstat=NULL,*changeregstat=NULL;
	if(am32)j=2;
	icomp=(ICOMP *)MALLOC(sizeof(ICOMP)*MAXIF);	//блок для инфо о сравнениях
	i=0;

	do{
#ifdef OPTVARCONST
		if((rcompr=constructcompare(0,outptr,comconst))==voidcompr||rcompr==zerocompr)i=1;
#else
		if((rcompr=constructcompare(0,outptr))==voidcompr||rcompr==zerocompr)i=1;
#endif
		if(i){
			if(rcompr==voidcompr&&ptok==tk_andand){
				i=0;
				ptok=tok;
			}
			continue;
		}
		op(0x03+j);
		if(tok!=tk_oror){
			JXorJMP();
			if(am32!=FALSE)outword(0);
			outword(0);
		}
		(icomp+*numcomp)->loc=outptr;
		(icomp+*numcomp)->type=tok;
//		(icomp+*numcomp)->use_cxz=rcompr&0xFC;
		(*numcomp)++ ;
		if(*numcomp==MAXIF){
			ManyLogicCompare();
			free(icomp);
			return NULL;
		}
		ptok=tok;
/*		if(tok!=tk_andand&&tok!=tk_oror&&bakregstat==NULL){
			bakregstat=BakRegStat();
			changeregstat=BakRegStat();
		}*/
	}while(tok==tk_oror||tok==tk_andand);
	if(tok==tk_closebracket)nexttok();
	for(i=0;i<*numcomp;i++){
		unsigned long temp=outptr-(icomp+i)->loc;
		if((icomp+i)->type==tk_oror){
#ifdef OPTVARCONST
			if(comconst)comconst->rec=NULL;
#endif
			if(temp>127)CompareOr();
			output[(icomp+i)->loc-1]=(unsigned char)temp;
			clearregstat();
			useor=TRUE;
		}
		else if(chip>2){
			if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-2]=(unsigned short)temp;
			else *(unsigned long *)&output[(icomp+i)->loc-4]=(unsigned long)temp;
		}
	}

	if(bakregstat==NULL){
		bakregstat=BakRegStat();
		changeregstat=BakRegStat();
	}
	if(type==tk_if&&rcompr!=zerocompr){
		if(rcompr==voidcompr)warcompeqconst();
		if(tok==tk_return||tok==tk_RETURN){
			if(tok2==tk_semicolon||(tok2==tk_openbracket&&ScanTok3()==tk_closebracket)){
				if(insertmode||(!optimizespeed)){
					if(tok==tk_return||tok==tk_RETURN)goto merge_if;
				}
			}
			startblock();
			doreturn(tok);
			endblock();
			int di;
			if(rcompr==voidcompr)di=0;
			else di=am32==FALSE?2:4;
			for(unsigned int i=0;i<*numcomp;i++){
				if((icomp+i)->type!=tk_oror){
					if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-di]=(unsigned short)(outptr-(icomp+i)->loc);
					else *(unsigned long *)&output[(icomp+i)->loc-di]=(unsigned long)(outptr-(icomp+i)->loc);
				}
			}
			if((outptr-icomp->loc)<=127)warningjmp(mesIF,ifline);
			if(tok==tk_else||tok==tk_ELSE){
				notunreach=TRUE;
				nexttok();
				docommand();
			}
			free(icomp);
			return NULL;
		}
		if(tok==tk_break||tok==tk_BREAK||tok==tk_continue||tok==tk_CONTINUE||tok==tk_goto||tok==tk_GOTO){
merge_if:
			if(rcompr==voidcompr)goto endp;
			if(chip<3){
				for(i=0;i<*numcomp;i++){
					if((icomp+i)->type==tk_oror)output[(icomp+i)->loc-1]=(unsigned char)(output[(icomp+i)->loc-1]-3-j);
					else{
						if((icomp+i)->type!=tk_andand)output[(icomp+i)->loc-5-j]=(unsigned char)(output[(icomp+i)->loc-5-j]^1);
						else{
							if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-2]=(unsigned short)(outptr-(icomp+i)->loc);
							else *(unsigned long *)&output[(icomp+i)->loc-4]=(unsigned long)(outptr-(icomp+i)->loc);
						}
					}
				}
				outptr-=3+j;
			}
			else{
				for(i=0;i<*numcomp;i++){
					if((icomp+i)->type==tk_oror)output[(icomp+i)->loc-1]=(unsigned char)(output[(icomp+i)->loc-1]-2-j);
					else{
						if((icomp+i)->type!=tk_andand){
							output[(icomp+i)->loc-4-j]=(unsigned char)(output[(icomp+i)->loc-3-j]-0x10);
							output[(icomp+i)->loc-3-j]=(unsigned char)(3+j);
						}
						else{
							if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-2]=(unsigned short)(outptr-(icomp+i)->loc+1);
							else *(unsigned long *)&output[(icomp+i)->loc-4]=(unsigned long)(outptr-(icomp+i)->loc+1);
						}
					}
				}
				outptr-=2+j;
				if(cpu<3)cpu=3;
			}
			int delta=0;	//new 07.06.06 20:27

			if(tok==tk_break||tok==tk_BREAK){
				if(useor==FALSE){
					int ooutptr=outptr-2;
					int otok=tok;
					i=(unsigned char)(output[ooutptr]^1);
					outptr--;
					if(tok==tk_break)i+=0x10;
					else outptr--;
					doBREAK((unsigned char)(tok==tk_BREAK?BREAK_SHORT:(am32==FALSE?BREAK_NEAR:BREAK_32)));
					if(otok==tk_break){
						output[ooutptr]=0x0f;
						output[ooutptr+1]=i;
						delta=-1;
					}
					else{
						output[ooutptr]=i;
						delta=(am32==TRUE?-5:-3);
					}
				}
				else{
					if(tok==tk_BREAK){
						output[outptr-1]-=(am32==TRUE?3:1);
					}
					doBREAK((unsigned char)(tok==tk_BREAK?BREAK_SHORT:(am32==FALSE?BREAK_NEAR:BREAK_32)));
				}
			}
			else if(tok==tk_return||tok==tk_RETURN){
//new 07.06.06 21:12
				if(useor==FALSE){
					int ooutptr=outptr-2;
					int otok=tok;
					i=(unsigned char)(output[ooutptr]^1);
					outptr--;
					if(tok==tk_return){
						i+=0x10;
					}
					else outptr--;
					AddRetList(outptr+1,linenumber,tok);
					if(tok2==tk_openbracket){
						nexttok();
						nexttok();
					}
					if(otok==tk_return){
						output[ooutptr]=0x0f;
						output[ooutptr+1]=i;
						delta=-1;
					}
					else{
						output[ooutptr]=i;
						delta=(am32==TRUE?-5:-3);
					}
				}
				else{
					if(tok==tk_RETURN)output[outptr-1]-=(am32==TRUE?3:1);
					AddRetList(outptr+1,linenumber,tok);
					if(tok==tk_return&&tok2==tk_openbracket){
						nexttok();
						nexttok();
					}
				}
				nextseminext();
				clearregstat();
#ifdef OPTVARCONST
				ClearLVIC();
#endif
			}
			else if(tok==tk_goto||tok==tk_GOTO){
				int ooutptr=outptr;
				if(useor==FALSE){
					if(tok==tk_GOTO){
						outptr-=2;
						i=(unsigned char)(output[outptr]^1);
						GOTOdo();
						output[outptr-2]=i;
						delta=(am32==0?-3:-5);
					}
					else{
						int otok2=tok2;
						gotodo();
						if(output[ooutptr]==0xEB){	//был короткий переход
							outptr=ooutptr-2;
							op(output[outptr]^1);
							op(output[ooutptr+1]+2);
							delta=(am32==0?-3:-5);
						}
						else if(am32&&otok2==tk_number){
							outptr=ooutptr-2;
							i=(unsigned char)((output[outptr]^1)+0x10);
							op(0x0f);
							op(i);
							if(output[outptr]==0xE9)outdword(*(unsigned long *)&output[ooutptr+1]+1);
							else outdword(*(unsigned short *)&output[ooutptr+2]);
							delta=-1;
						}
					}
				}
				else{	// useor
					if(tok==tk_goto)gotodo();
					else GOTOdo();
					if(output[ooutptr]==0xEB){	//был короткий переход
						output[ooutptr-1]-=(am32==TRUE?3:1);
					}
				}
			}
			else{
				if(useor==FALSE){
					int ooutptr=outptr-2;
					int otok=tok;
					i=(unsigned char)(output[ooutptr]^1);
					outptr--;
					if(tok==tk_continue)i+=0x10;
					else outptr--;
					doCONTINUE((unsigned char)(tok==tk_CONTINUE?CONTINUE_SHORT:(am32==FALSE?CONTINUE_NEAR:CONTINUE_32)));
					if(otok==tk_continue){
						output[ooutptr]=0x0f;
						output[ooutptr+1]=i;
						delta=-1;
					}
					else{
						output[ooutptr]=i;
						delta=(am32==TRUE?-5:-3);
					}
				}
				else{
					if(tok==tk_CONTINUE){
						output[outptr-1]-=(am32==TRUE?3:1);
					}
					doCONTINUE((unsigned char)(tok==tk_CONTINUE?CONTINUE_SHORT:(am32==FALSE?CONTINUE_NEAR:CONTINUE_32)));
				}
			}
			for(i=0;i<*numcomp;i++){
//				if((icomp+i)->type==tk_oror)output[(icomp+i)->loc-1]+=delta;
//				else
				if((icomp+i)->type==tk_andand){
					if(am32==FALSE)*(signed short *)&output[(icomp+i)->loc-2]+=delta;
					else *(signed long *)&output[(icomp+i)->loc-4]+=delta;
				}
			}
			if(tok==tk_else||tok==tk_ELSE){
				notunreach=TRUE;
				nexttok();
				docommand();
			}
			free(icomp);
			return NULL;
		}
	}
endp:
	if(type!=tk_for){
		startblock();
		if(rcompr==zerocompr)warcompneqconst();
		if(tok==tk_openbrace){
			if(rcompr==zerocompr){
				cha=cha2;
				inptr=inptr2;
				SkipBlock();
				inptr2=inptr;
				cha2=cha;
				linenum2=linenumber;
				nexttok();
			}
			else{
#ifdef OPTVARCONST
				if(comconst&&comconst->rec&&comconst->typevar==tk_equalto){
					Const2VarRec(comconst);
				}
#endif
				doblock();
				nexttok();
			}
		}
		else{
			if(rcompr==zerocompr){
				do{
					nexttok();
				}while(tok!=tk_semicolon&&tok!=tk_eof);
			}
			else{
#ifdef OPTVARCONST
				if(comconst&&comconst->rec&&comconst->typevar==tk_equalto){
					Const2VarRec(comconst);
				}
#endif
				docommand();
			}
		}
		endblock();
		RestoreStack();
	}
	if(bakreg)*bakreg=bakregstat;
	if(changereg)*changereg=changeregstat;
	return icomp;
}

void opt_if_else_stop(unsigned int newptr)
{
unsigned int ooutptr,ooutptrdata;
unsigned char instr;
	dbgact++;
	ooutptr=outptr;
	ooutptrdata=outptrdata;
	outptr=newptr;
	instr=output[outptr];
	docommand();
	if(output[newptr]==0xEB){
		signed char ofs=output[newptr+1];
		if(output[newptr-1]==0x0F&&instr>=0x80&&instr<0x90){
			ofs--;
			if(am32)ofs-=(signed char)2;
		}
		if(am32)*(long *)&output[newptr+1]=ofs;
		else*(short *)&output[newptr+1]=ofs;
	}
	if(am32&&output[newptr]==0x66&&output[newptr+1]==0xE9){
		signed short ofs=(signed short)(*(short *)&output[newptr+2]-1);
		*(long *)&output[newptr+1]=ofs;
	}
	output[newptr]=instr;
	outptr=ooutptr;
	outptrdata=ooutptrdata;
	dbgact--;
}

void doif()
{
unsigned int startloc,elseline,numcomp=0,ifline;
ICOMP *icomp;
REGISTERSTAT *bakregstat=NULL,*changeregstat=NULL;
unsigned int oaddESP=addESP;
	ifline=linenumber;
#ifdef OPTVARCONST
LVIC comconst;
	comconst.rec=NULL;
	icomp=compare(tk_if,&numcomp,&bakregstat,&changeregstat,&comconst);
#else
	icomp=compare(tk_if,&numcomp,&bakregstat,&changeregstat);
#endif
//	i=CheckStopBlock();
	/*-----------------19.08.99 22:35-------------------
	 Убирать else можно только после первого if
	 После else if в следующий else убирать нельзя
		--------------------------------------------------*/
	if(icomp!=NULL){
		elseline=linenumber;
unsigned long temp;
unsigned int j=0;
unsigned int otok=tok;
unsigned int oinptr=inptr2;
unsigned char ocha=cha2;
unsigned int oline=linenumber;
		if(tok==tk_else||tok==tk_ELSE){
			if(dbg)AddLine();
			j=(am32==FALSE?3:5);
			if(tok2==tk_goto||tok2==tk_break||tok2==tk_continue||//поглотить их
					tok2==tk_RETURN||tok2==tk_return||tok2==tk_GOTO||tok2==tk_BREAK||tok2==tk_CONTINUE){
				nexttok();
				switch(tok){
					case tk_GOTO: otok=tk_goto; break;
					case tk_BREAK: otok=tk_break; break;
					case tk_CONTINUE: otok=tk_continue; break;
					case tk_return:
					case tk_RETURN:
						if(tok2==tk_semicolon||(tok2==tk_openbracket&&
								ScanTok3()==tk_closebracket)){
	 						startblock();
							otok=tk_return;
							break;
						}
						tok=otok;	//невозможно оптимизировать
						inptr2=oinptr;
						cha2=ocha;
						linenumber=oline;
						goto nooptim;
					default:otok=tok; break;
				}
				oinptr=inptr2;
				ocha=cha2;
				oline=linenumber;
				for(unsigned int i=0;i<numcomp;i++){
					if((icomp+i)->type!=tk_oror){
						notunreach=TRUE;
						tok=otok;
						inptr2=oinptr;
						cha2=ocha;
						linenumber=oline;
						opt_if_else_stop((icomp+i)->loc-j);
					}
				}
				if(otok==tk_return)endblock();
				if((outptr+j-icomp->loc)<=127)warningjmp(mesIF,ifline);
				free(icomp);
				retproc=FALSE;
				lastcommand=tk_if;
				return;
			}
nooptim:
			if(tok==tk_ELSE)j=2;
		}
		notunreach=TRUE;
		for(unsigned int i=0;i<numcomp;i++){
			if((icomp+i)->type!=tk_oror){
				if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-2]=(unsigned short)(outptr+j-(icomp+i)->loc);
				else *(unsigned long *)&output[(icomp+i)->loc-4]=(unsigned long)(outptr+j-(icomp+i)->loc);
			}
		}
		if((outptr+j-icomp->loc)<=127)warningjmp(mesIF,ifline);
		switch(lastcommand){
			case tk_return:
			case tk_RETURN:
			case tk_goto:
			case tk_GOTO:
			case tk_break:
			case tk_BREAK:
			case tk_continue:
			case tk_CONTINUE:
				addESP=oaddESP;
				break;
		}
		if(retproc)CopyRegStat(bakregstat);
		else{
			switch(lastcommand){
				case tk_return:
				case tk_RETURN:
				case tk_goto:
				case tk_GOTO:
				case tk_break:
				case tk_BREAK:
				case tk_continue:
				case tk_CONTINUE:
					CopyRegStat(bakregstat);
					break;
				default:
					CompareRegStat(changeregstat);
					break;
			}
		}
//		printf("lastcommand=%d\n",lastcommand);
//		CompareRegStat(changeregstat);
		if(tok==tk_else/*&&i==FALSE*/){
			addESP=oaddESP;
			RestoreStack();
			CopyRegStat(bakregstat);
			jumploc0();
			startloc=outptr;
			getoperand();
#ifdef OPTVARCONST
			if(tok!=tk_if&&tok!=tk_IF&&comconst.rec&&comconst.typevar==tk_notequal){
				Const2VarRec(&comconst);
			}
#endif
			startblock();
			if(tok==tk_return||tok==tk_RETURN){
				if(dbg)AddLine();
				doreturn(tok);
			}
			else docommand();
			endblock();
			RestoreStack();
			temp=outptr-startloc;
			if(temp<=127)warningjmp(mesELSE,elseline);
			if(am32==FALSE)*(unsigned short *)&output[startloc-2]=(unsigned short)temp;
			else *(unsigned long *)&output[startloc-4]=temp;
			CompareRegStat(changeregstat);
		}
		else if(tok==tk_ELSE/*&&i==FALSE*/){
			addESP=oaddESP;
			RestoreStack();
			CopyRegStat(bakregstat);
			outword(0x00EB);
			startloc=outptr;
			getoperand();
#ifdef OPTVARCONST
			if(tok!=tk_if&&tok!=tk_IF&&comconst.rec&&comconst.typevar==tk_notequal){
				Const2VarRec(&comconst);
			}
#endif
			startblock();
			if(tok==tk_return||tok==tk_RETURN){
				if(dbg)AddLine();
				doreturn(tok);
			}
			else docommand();
			endblock();
			RestoreStack();
			temp=outptr-startloc;
			if(temp>127)jumperror(elseline,mesELSE);
			output[startloc-1]=(unsigned char)temp;
			CompareRegStat(changeregstat);
		}
/*		else{
		if(i!=FALSE&&(tok==tk_else||tok==tk_ELSE))nexttok();
		}*/
		free(icomp);
		CopyRegStat(changeregstat);
	}
	FreeStat(bakregstat);
	FreeStat(changeregstat);
	retproc=FALSE;
	lastcommand=tk_if;
}

#ifdef OPTVARCONST
ICOMP *bigcompare(int type,unsigned int *numcomp,REGISTERSTAT **bakreg,REGISTERSTAT **changereg,LVIC *comconst)
#else
ICOMP *bigcompare(int type,unsigned int *numcomp,REGISTERSTAT **bakreg,REGISTERSTAT **changereg)
#endif
{
unsigned int ifline;
ICOMP *icomp;
unsigned int i=0;
int j=0;
int ptok=tk_oror;
int rcompr;
int useor=FALSE;
REGISTERSTAT *bakregstat=NULL,*changeregstat=NULL;
	if(am32!=FALSE)j=2;
	icomp=(ICOMP *)MALLOC(sizeof(ICOMP)*MAXIF);	//блок для инфо о сравнениях
	ifline=linenumber;
	do{
#ifdef OPTVARCONST
		if((rcompr=constructcompare(1,outptr,comconst))==voidcompr||rcompr==zerocompr)i=1;
#else
		if((rcompr=constructcompare(1,outptr))==voidcompr||rcompr==zerocompr)i=1;
#endif
		(icomp+*numcomp)->use_cxz=rcompr&0xFC;
		if(i){
			if(rcompr==voidcompr&&ptok==tk_andand){
				i=0;
				ptok=tok;
			}
			continue;
		}
		op(0x00);
		(icomp+*numcomp)->loc=outptr;
		(icomp+*numcomp)->type=tok;
		(*numcomp)++;
		if(*numcomp==MAXIF){
			ManyLogicCompare();
			free(icomp);
			return NULL;
		}
		ptok=tok;
/*		if(tok==tk_andand&&bakregstat==NULL){
			bakregstat=BakRegStat();
			changeregstat=BakRegStat();
		}*/
	}while(tok==tk_oror||tok==tk_andand);
	if(tok==tk_closebracket)nexttok();
	for(i=0;i<*numcomp;i++){
		if(outptr-(icomp+i)->loc>127)CompareOr();
		output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
		if((icomp+i)->type==tk_oror){
#ifdef OPTVARCONST
			if(comconst)comconst->rec=NULL;
#endif
			output[(icomp+i)->loc-2]=(unsigned char)(output[(icomp+i)->loc-2]^1);
			clearregstat();
			useor=TRUE;
		}
	}
	if(bakregstat==NULL){
		bakregstat=BakRegStat();
		changeregstat=BakRegStat();
	}
	if(type==tk_IF&&rcompr!=zerocompr){
		if(rcompr==voidcompr)warcompeqconst();
		if(tok==tk_return||tok==tk_RETURN){
			if(tok2==tk_semicolon||(tok2==tk_openbracket&&ScanTok3()==tk_closebracket)){
				if(insertmode||(!optimizespeed)){
					if(tok==tk_RETURN)goto merge_if;
					else if(chip>2&&(insertmode||(paramsize&&
							(current_proc_type&f_typeproc)!=tp_cdecl)))goto merge_if2;
				}
			}
			startblock();
			doreturn(tok);
			endblock();
			for(unsigned int i=0;i<*numcomp;i++){
				if((icomp+i)->type!=tk_oror){
					if((outptr-(icomp+i)->loc)>127)jumperror(ifline,mesIF);
					output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
				}
			}
			if(tok==tk_else||tok==tk_ELSE){
	 			notunreach=TRUE;
				getoperand();
 				docommand();
			}
			free(icomp);
			return NULL;
		}
		if(tok==tk_BREAK||tok==tk_CONTINUE||tok==tk_GOTO){
merge_if:
			int otok=tok;
			for(i=0;i<*numcomp;i++){
				unsigned char oldcode;
				if(((icomp+i)->type==tk_oror)||((i+1)==*numcomp)){
					outptr=(icomp+i)->loc-2;
					oldcode=output[outptr];
					if(tok==tk_BREAK)MakeBreak(BREAK_SHORT);
					else if(tok==tk_RETURN){
						AddRetList(outptr+1,ifline,tk_RETURN);
						clearregstat();
#ifdef OPTVARCONST
						ClearLVIC();
#endif
					}
					else if(tok==tk_GOTO){
						if((i+1)!=*numcomp){
							int optr,ocha,oline;
							optr=inptr2;
							ocha=cha2;
							oline=linenum2;
							GOTOdo();
							if(outptr!=(icomp+i)->loc)jumperror(oline,"GOTO");
							inptr2=optr;
							cha2=(unsigned char)ocha;
							linenum2=oline;
							tok=tk_GOTO;
						}
						else{
							getoperand();
							CheckIP();
							if(tok==tk_number||tok==tk_interruptproc||tok==tk_proc){
								long hnumber;
								if(tok==tk_number)hnumber=doconstdwordmath();
								else{
									hnumber=itok.number;
									nexttok();
								}
								long loc=hnumber-outptr-2;
								if(loc>-129&&loc<128){
									op((unsigned char)(oldcode^1));
									op(loc);
								}
								else{
									loc-=3;
									outptr++;
									op(am32==0?3:5);
									op(0xE9);
									if(am32==FALSE)outword(loc);
									else{
										loc-=2;
										outdword(loc);
									}
								}
								seminext();
								break;
							}
							else if(GOTO())nexttok();
							seminext();
						}
					}
					else MakeContinue(CONTINUE_SHORT);
					if((i+1)==*numcomp)oldcode^=1;
					output[outptr-2]=oldcode;
				}
			}
			if(tok==tk_RETURN&&tok2==tk_openbracket){
				nexttok();
				nexttok();
			}
			if(otok!=tk_GOTO&&rcompr!=voidcompr)nextseminext();

			if(tok==tk_else||tok==tk_ELSE){
	 			notunreach=TRUE;
				getoperand();
 				docommand();
			}
			free(icomp);
			return NULL;
		}
		if((tok==tk_break||tok==tk_continue||tok==tk_goto)&&chip>2){
merge_if2:

//			printf("%s (%u) %s %s tok=%d\n",(startfileinfo+currentfileinfo)->filename,linenumber,itok.name,string,tok);

			if(*numcomp==1&&(!(tok==tk_goto&&(tok2==tk_reg||tok2==tk_reg32)))){
				outptr-=2;
				i=(output[outptr]^1)+0x10;
				op(0x0F);
			}
			if(tok==tk_break)doBREAK((unsigned char)(am32==FALSE?BREAK_NEAR:BREAK_32));
			else if(tok==tk_return){
				AddRetList(outptr+1,ifline,tk_return);
				if(tok2==tk_openbracket){
					nexttok();
					nexttok();
				}
				nextseminext();
				clearregstat();
#ifdef OPTVARCONST
				ClearLVIC();
#endif
			}
			else if(tok==tk_goto){
				nexttok();
				CheckIP();
				if((tok==tk_number&&*numcomp==1)||tok==tk_interruptproc||tok==tk_proc){
					if(tok==tk_proc&&tok2==tk_openbracket)doanyproc(TRUE);
					else{
						long hnumber;
						if(tok==tk_number)hnumber=doconstdwordmath();
						else{
							hnumber=itok.number;
							nexttok();
						}
						long loc=hnumber-outptr-2;
						if(loc>-130&&loc<127){
							outptr--;
							op((unsigned char)(i-0x10));
							op(loc+1);
							seminext();
							goto c1;
						}
						else{
							loc--;
							op(0xE9);
							if(am32==FALSE)outword(loc);
							else{
								loc-=2;
								outdword(loc);
							}
						}
					}
				}
				else{
					if(tok==tk_reg||tok==tk_reg32){
						i=outptr;
						if(gotol(0))nexttok();
						i=outptr-i;
						output[outptr-i-1]=(unsigned char)i;
						goto c1;
					}
					if(gotol(0))nexttok();
				}
				seminext();
			}
			else doCONTINUE((unsigned char)(am32==FALSE?CONTINUE_NEAR:CONTINUE_32));
			if(*numcomp==1)output[outptr-3-j]=(unsigned char)i;
c1:
			if(cpu<3)cpu=3;
			if(tok==tk_else||tok==tk_ELSE){
	 			notunreach=TRUE;
				getoperand();
				docommand();
			}
			if(*numcomp!=1){
				for(unsigned int i=0;i<*numcomp;i++){
					if((icomp+i)->type!=tk_oror){
						if((outptr-(icomp+i)->loc)>127)jumperror(ifline,mesIF);
						output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
					}
				}
			}
			free(icomp);
			return NULL;
		}
	}
	if((icomp+(*numcomp-1))->use_cxz==cxnzcompr){
		outptr-=4;
		outword(0xE3);
		(icomp+(*numcomp-1))->loc=outptr;
		for(i=(*numcomp-1);i!=0;i--){
			if((icomp+i-1)->type==tk_oror)output[(icomp+i-1)->loc-1]-=(unsigned char)2;
		}
	}
	if(type!=tk_FOR){
		startblock();
		if(rcompr==zerocompr)warcompneqconst();
		if(tok==tk_openbrace){
			if(rcompr==zerocompr){
				cha=cha2;
				inptr=inptr2;
				SkipBlock();
				inptr2=inptr;
				cha2=cha;
				linenum2=linenumber;
				nexttok();
			}
			else{
#ifdef OPTVARCONST
				if(comconst&&comconst->rec&&comconst->typevar==tk_equalto)Const2VarRec(comconst);
#endif
				doblock();
				nexttok();
			}
		}
		else{
			if(rcompr==zerocompr){
				do{
					nexttok();
				}while(tok!=tk_semicolon&&tok!=tk_eof);
			}
			else{
#ifdef OPTVARCONST
				if(comconst&&comconst->rec&&comconst->typevar==tk_equalto)Const2VarRec(comconst);
#endif
				docommand();
			}
		}
		endblock();
		RestoreStack();
	}
	if(bakreg)*bakreg=bakregstat;
	if(changereg)*changereg=changeregstat;
	return icomp;
}

void dobigif()
{
unsigned int ifline,numcomp=0,j=0;
ICOMP *icomp;
int ic;
unsigned int oaddESP=addESP;
REGISTERSTAT *bakregstat=NULL,*changeregstat=NULL;
	ifline=linenumber;
#ifdef OPTVARCONST
LVIC comconst;
	comconst.rec=NULL;
	icomp=bigcompare(tk_IF,&numcomp,&bakregstat,&changeregstat,&comconst);
#else
	icomp=bigcompare(tk_IF,&numcomp,&bakregstat,&changeregstat);
#endif
	if(icomp!=NULL){
		unsigned int elseline;
		elseline=linenumber;
		if(tok==tk_else)j=(am32==FALSE?3:5);
		else if(tok==tk_ELSE)j=2;
		notunreach=TRUE;
		if(dbg)AddLine();
		for(unsigned int i=0;i<numcomp;i++){
			if((icomp+i)->type!=tk_oror){
				if((outptr+j-(icomp+i)->loc)>127)jumperror(ifline,mesIF);
				output[(icomp+i)->loc-1]=(unsigned char)(outptr+j-(icomp+i)->loc);
			}
		}
		switch(lastcommand){
			case tk_return:
			case tk_RETURN:
			case tk_goto:
			case tk_GOTO:
			case tk_break:
			case tk_BREAK:
			case tk_continue:
			case tk_CONTINUE:
				addESP=oaddESP;
				break;
		}
		if(retproc)CopyRegStat(bakregstat);
		else{
			switch(lastcommand){
				case tk_return:
				case tk_RETURN:
				case tk_goto:
				case tk_GOTO:
				case tk_break:
				case tk_BREAK:
				case tk_continue:
				case tk_CONTINUE:
					CopyRegStat(bakregstat);
					break;
				default:
					CompareRegStat(changeregstat);
					break;
			}
//			CompareRegStat(changeregstat);
		}
		if(tok==tk_else/*&&i==FALSE*/){
			addESP=oaddESP;
			RestoreStack();
			CopyRegStat(bakregstat);
			jumploc0();
			ic = outptr;
			getoperand();
#ifdef OPTVARCONST
			if(tok!=tk_if&&tok!=tk_IF&&comconst.rec&&comconst.typevar==tk_notequal)Const2VarRec(&comconst);
#endif
			startblock();
			if(tok==tk_return||tok==tk_RETURN){
				if(dbg)AddLine();
				doreturn(tok);
			}
			else docommand();
			endblock();
			RestoreStack();
			if((outptr-ic)<=127)warningjmp(mesELSE,elseline);
			if(am32==FALSE)*(unsigned short *)&output[ic-2]=(unsigned short)(outptr-ic);
			else *(unsigned long *)&output[ic-4]=(unsigned long)(outptr-ic);
			CompareRegStat(changeregstat);
		}
		else if(tok==tk_ELSE/*&&ic==FALSE*/){
			addESP=oaddESP;
			RestoreStack();
			CopyRegStat(bakregstat);
			outword(0x00EB);
			ic=outptr;
			getoperand();
#ifdef OPTVARCONST
			if(tok!=tk_if&&tok!=tk_IF&&comconst.rec&&comconst.typevar==tk_notequal)Const2VarRec(&comconst);
#endif
			startblock();
			if(tok==tk_return||tok==tk_RETURN){
				if(dbg)AddLine();
				doreturn(tok);
			}
			else docommand();
			endblock();
			RestoreStack();
			if((outptr-ic)>127)jumperror(elseline,mesELSE);
			output[ic-1]=(unsigned char)(outptr-ic);
			CompareRegStat(changeregstat);
		}
		free(icomp);
		CopyRegStat(changeregstat);
	}
	FreeStat(bakregstat);
	FreeStat(changeregstat);
	retproc=FALSE;
	lastcommand=tk_IF;
}

void JXorJMP()
{
	if(chip<3){
		op(0xE9);
	}
	else{
		unsigned char i;
		outptr-=2;
		i=(unsigned char)((output[outptr]^1)+0x10);
		op(0x0F); op(i);
		if(cpu<3)cpu=3;
	}
}

void dowhile(unsigned int typeb)
{
unsigned int ifline,conloc,numcomp=0;
ICOMP *icomp;
REGISTERSTAT *bakregstat=NULL,*changeregstat=NULL;
	uptdbr();
	if(AlignCycle)AlignCD(CS,aligncycle);
	conloc=outptr;
	ifline=linenumber;
	if(typeb==tk_while)
#ifdef OPTVARCONST
			icomp=compare(typeb,&numcomp,&bakregstat,&changeregstat,NULL);
#else
			icomp=compare(typeb,&numcomp,&bakregstat,&changeregstat);
#endif
	else
#ifdef OPTVARCONST
			icomp=bigcompare(typeb,&numcomp,&bakregstat,&changeregstat,NULL);
#else
			icomp=bigcompare(typeb,&numcomp,&bakregstat,&changeregstat);
#endif
	SetContinueLabel();
	jumploc(conloc);
	if(icomp!=NULL){
		if(typeb==tk_WHILE){
			for(unsigned int i=0;i<numcomp;i++){
				if((icomp+i)->type!=tk_oror){
					if((outptr-(icomp+i)->loc)>127)jumperror(ifline,mesWHILE);
					output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
				}
			}
		}
		else{
			for(unsigned int i=0;i<numcomp;i++){
				if((icomp+i)->type!=tk_oror){
					if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-2]=(unsigned short)(outptr-(icomp+i)->loc);
					else *(unsigned long *)&output[(icomp+i)->loc-4]=(unsigned long)(outptr-(icomp+i)->loc);
				}
			}
			if((outptr-icomp->loc)<=127)warningjmp(mesWHILE,ifline);
		}
		free(icomp);
	}
	if(retproc){
		if(bakregstat)CopyRegStat(bakregstat);
	}
	else if(changeregstat)CompareRegStat(changeregstat);
	if(changeregstat)CopyRegStat(changeregstat);
	SetBreakLabel();
	if(usebr[curbr]!=0)clearregstat();
	FreeStat(bakregstat);
	FreeStat(changeregstat);
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	lastcommand=tk_do;
}

void dowhilefast(unsigned int typeb)
{
unsigned int numcomp=0;
unsigned int startloc,i;
ICOMP *icomp;
ITOK oitok,ostructadr;
SINFO ostr;
unsigned int oldinptr;
int otok,otok2;
char *ostring,*obufrm;
int blinenum,olinenum,oinptr;
char *ostartline,*bstartline;
char ocha,bcha;
int jmptocompr=0;
int rcompr;
unsigned char *oinput;
unsigned int oaddESP=addESP;
	ocha=cha2;
	oinptr=inptr2;
	olinenum=linenumber;
	ostartline=startline;
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	do{
		nexttok();
		if(tok2==tk_openbracket)nexttok();
		if(tok2==tk_closebracket){
			nexttok();
			jmptocompr=1;
			break;
		}
		if(tok2==tk_number){
			nexttok();
			if(tok2==tk_closebracket){
				if(itok.number!=0)jmptocompr=1;
				else jmptocompr=2;
				nexttok();
				break;
			}
		}
		nexttok();	//необходимо для избежания предупреждения о неинициализированной переменной
		cha=cha2;
		inptr=inptr2;
		SkipParam();
		inptr2=inptr;
		cha2=cha;
		linenum2=linenumber;
		nexttok();
		if(typeb==tk_while&&tok==tk_oror&&optimizespeed==0){
			cha2=ocha;
			inptr2=oinptr;
			linenumber=linenum2=olinenum;
			startline=ostartline;
			dowhile(typeb);
			if(ESPloc&&am32&&oaddESP!=addESP)warESP();
			return;
		}
		if(bufrm){
			free(bufrm);
			bufrm=NULL;
		}
		if(strinf.bufstr){
			free(strinf.bufstr);
			strinf.bufstr=NULL;
		}
	}while(tok==tk_andand||tok==tk_oror);
	while(tok==tk_closebracket)nexttok();
	if(!jmptocompr){
		if(typeb==tk_WHILE)outword(0x00EB); 	// JMP SHORT
		else jumploc0();
	}
	i=outptr;
	if(AlignCycle)AlignCD(CS,aligncycle);
	startloc=outptr;
	uptdbr();
	if(tok!=tk_openbrace){
		if(jmptocompr==2){
			do{
				nexttok();
			}while(tok!=tk_semicolon&&tok!=tk_eof);
		}
		else docommand();
	}
	else{
		if(jmptocompr==2){
			cha=cha2;
			inptr=inptr2;
			SkipBlock();
			inptr2=inptr;
			cha2=cha;
			linenum2=linenumber;
			nexttok();
		}
		else{
			startblock();
			doblock();
			nexttok();
			endblock();
		}
	}
	RestoreStack();
	if(!jmptocompr){
		if(typeb==tk_WHILE){
			if(i!=outptr){
			 	if((outptr-i)>127)jumperror(olinenum,mesWHILE);
				output[i-1]=(unsigned char)(outptr-i);
			}
			else{
				if(dbg&1)KillLastLine();
				outptr-=2;
				startloc-=2;
			}
		}
		else{
			if(i!=outptr){
				if((outptr-i)<=127)warningjmp(mesWHILE,olinenum);
				if(am32) *(unsigned long *)&output[i-4]=(unsigned long)(outptr-i);
				else *(unsigned short *)&output[i-2]=(unsigned short)(outptr-i);
			}
			else{
				i=3;
				if(am32)i+=2;
				if(dbg){
					KillLastLine();
					KillLastLine();
				}
				outptr-=i;
				startloc-=i;
			}
		}
	}
	SetContinueLabel();
	clearregstat();    //06.09.04 21:56
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	icomp=(ICOMP *)MALLOC(sizeof(ICOMP)*MAXIF);	//блок для инфо о сравнениях
//	oitok2=itok2;

	ostring=BackString((char *)string);
	oldinptr=inptr2;
	oinput=input;
	bcha=cha2;
	otok=tok;
	otok2=tok2;
	oitok=itok;
	ostructadr=structadr;
	ostr=strinf;
	strinf.bufstr=NULL;
	obufrm=bufrm;
	int oldendinptr=endinptr;
COM_MOD *tempcurmod=cur_mod;
	if(cur_mod)BackMod();

	bufrm=NULL;
	blinenum=linenum2;
	inptr2=oinptr;
	cha2=ocha;
	linenumber=linenum2=olinenum;
	bstartline=startline;
	startline=ostartline;

	if(dbg)AddLine();
	int ptok=tk_oror;
	do{
		i=0;
#ifdef OPTVARCONST
		if((rcompr=constructcompare(2,startloc,NULL))==voidcompr||rcompr==zerocompr)i=1;
#else
		if((rcompr=constructcompare(2,startloc))==voidcompr||rcompr==zerocompr)i=1;
#endif
		if(i){
			if(ptok==tk_andand){
//				i=0;
				ptok=tok;
			}
			continue;
		}
		if((rcompr&1)){;
			op(0x03);
			if(tok!=tk_andand){
				JXorJMP();
				if(am32==FALSE)outword(startloc-(outptr+2));
				else outdword(startloc-(outptr+4));
			}
		}
		else{
			op(startloc-(outptr+1)); 		 /* the small jump */
			if(tok==tk_andand)output[outptr-2]=(unsigned char)(output[outptr-2]^1);
		}
		(icomp+numcomp)->loc=outptr;
		(icomp+numcomp)->type=tok;
		numcomp++;
		if(numcomp==MAXIF){
			ManyLogicCompare();
			break;
		}
		ptok=tok;
	}while(tok==tk_oror||tok==tk_andand);
	if(jmptocompr==1)jumploc(startloc);

	startline=bstartline;
	strinf=ostr;
	inptr2=oldinptr;
	input=oinput;
	endinptr=oldendinptr;
	cha2=bcha;
	tok=otok;
	itok=oitok;
	structadr=ostructadr;
	bufrm=obufrm;
	tok2=otok2;
	cur_mod=tempcurmod;
	strcpy((char *)string,ostring);
	free(ostring);
	linenumber=linenum2=blinenum;
	itok2=oitok;

	for(i=0;i<numcomp;i++){
		if((icomp+i)->type==tk_andand){
			if(outptr-(icomp+i)->loc>127)CompareOr();
			output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
		}
	}
	if(rcompr==cxzcompr){
		outptr-=4;
		op(0xE3);
		op(startloc-outptr-1);
		for(i=(numcomp-1);i!=0;i--){
			if((icomp+i-1)->type==tk_andand)output[(icomp+i-1)->loc-1]-=(unsigned char)2;
		}
	}
	SetBreakLabel();
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	free(icomp);
	if(ESPloc&&am32&&oaddESP!=addESP)warESP();
	lastcommand=tk_do;
}

void dodo()
{
unsigned int startloc,numcomp=0,i=0;
ICOMP *icomp;
int ptok=tk_oror;
int rcompr;
unsigned int oaddESP=addESP;
	nexttok();
	if(AlignCycle)AlignCD(CS,aligncycle);
	startloc=outptr;
	uptdbr();
	if(dbg&1)KillLastLine();
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	docommand();
	SetContinueLabel();
	if(dbg)AddLine();
	if(tok!=tk_while)preerror("'while' expected following 'do'");
	icomp=(ICOMP *)MALLOC(sizeof(ICOMP)*MAXIF);	//блок для инфо о сравнениях
	do{
#ifdef OPTVARCONST
		if((rcompr=constructcompare(2,startloc,NULL))==voidcompr||rcompr==zerocompr)i=1;
#else
		if((rcompr=constructcompare(2,startloc))==voidcompr||rcompr==zerocompr)i=1;
#endif
		if(i){
			if(ptok==tk_andand){
				i=0;
				ptok=tok;
			}
			continue;
		}
		if((rcompr&1)){;
			op(0x03);
			if(tok!=tk_andand){
				JXorJMP();
				if(am32==FALSE)outword(startloc-(outptr+2));
				else outdword(startloc-(outptr+4));
			}
		}
		else{
			op(startloc-(outptr+1)); 		 /* the small jump */
			if(tok==tk_andand)output[outptr-2]=(unsigned char)(output[outptr-2]^1);
		}
		(icomp+numcomp)->loc=outptr;
		(icomp+numcomp)->type=tok;
		numcomp++;
		if(numcomp==MAXIF){
			ManyLogicCompare();
			goto end;
		}
		ptok=tok;
	}while(tok==tk_oror||tok==tk_andand);
	if(i)jumploc(startloc);
	if(tok==tk_closebracket)nexttok();
	for(i=0;i<numcomp;i++){
		if((icomp+i)->type==tk_andand){
			if(outptr-(icomp+i)->loc>127)CompareOr();
			output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
		}
	}
	if(rcompr==cxzcompr){
		outptr-=4;
		op(0xE3);
		op(startloc-outptr-1);
		for(i=(numcomp-1);i!=0;i--){
			if((icomp+i-1)->type==tk_andand)output[(icomp+i-1)->loc-1]-=(unsigned char)2;
		}
	}
	seminext();
	SetBreakLabel();
	if(usebr[curbr]!=0||useco[curco]!=0)clearregstat();
end:
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	free(icomp);
	if(ESPloc&&am32&&oaddESP!=addESP)warESP();
	lastcommand=tk_do;
}

void dofor(unsigned int typeb)
{
unsigned int ifline,conloc,blinenum,numcomp=0;
unsigned char bcha;
unsigned char COMPARE=FALSE,modif=FALSE;
int i;
unsigned char *buf;
unsigned int oaddESP=addESP;
ICOMP *icomp=NULL;
REGISTERSTAT *bakregstat=NULL,*changeregstat=NULL;
	ifline=linenumber;
//	printf("start for, curco=%u curbr=%u\n",curco,curbr);
	uptdbr();
	nexttok();
	i=inptr2;
	bcha=cha2;
	expecting(tk_openbracket);	//пров на откр скобку
	if(tok!=tk_semicolon){	//ЕСТЬ ПРЕДВАРИТЕЛЬНЫЕ УСТАНОВКИ
		for(;;){	//записать их в буфер
			AddBackBuf(i,bcha);
			if(tok==tk_semicolon)break;
			if(tok!=tk_camma){
				expecting(tk_semicolon);
				break;
			}
			i=inptr2;
			bcha=cha2;
			nexttok();
		}
		if(bufrm){
			free(bufrm);
			bufrm=NULL;
		}
		if(strinf.bufstr){
			free(strinf.bufstr);
			strinf.bufstr=NULL;
		}
		CharToBackBuf('}');
		CharToBackBuf(0);
		RunBackText();	//выполнить его
	}
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	bcha=cha2;
	i=inptr2;
	nexttok();
	if(AlignCycle)AlignCD(CS,aligncycle);
	conloc=outptr;	//запомнить точку начала цикла

	if(tok!=tk_semicolon){	//если есть условие
		if(tok!=tk_openbracket){	//если условие начинается не с (
			CharToBackBuf('(');	//добавить ее
			COMPARE=TRUE;	//и флаг установить
		}
		AddBackBuf(i,bcha);	//запомнить условие
		if(tok!=tk_semicolon)expected(';');
		SizeBackBuf--;
		if(COMPARE)CharToBackBuf(')');	//если надо, закрыть скобку
		CharToBackBuf(0);
		int oendinptr=endinptr;
		endinptr=SizeBackBuf-1;//strlen(BackTextBlock);
		i=inptr2;
		buf=input;
		bcha=cha2;
		input=(unsigned char *)BackTextBlock;
		SizeBackBuf=0;
		inptr2=1;
		cha2='(';
		if(typeb==tk_for)
#ifdef OPTVARCONST
				icomp=compare(typeb,&numcomp,&bakregstat,&changeregstat,NULL);
#else
				icomp=compare(typeb,&numcomp,&bakregstat,&changeregstat);
#endif
		else
#ifdef OPTVARCONST
				icomp=bigcompare(typeb,&numcomp,&bakregstat,&changeregstat,NULL);
#else
				icomp=bigcompare(typeb,&numcomp,&bakregstat,&changeregstat);
#endif
		free(input);
		input=buf;
		inptr2=i;
		cha2=bcha;
		endinptr=oendinptr;
		COMPARE=TRUE;
		nexttok();
	}
	else{
		i=inptr2;
		bcha=cha2;
		nexttok();
	}

	if(tok!=tk_closebracket){	//есть модификация
		modif=TRUE;
		while(tok!=tk_closebracket){
			AddBackBuf(i,bcha);
			if(cha==')'||cha==26){
				CharToBackBuf(';');
				nextchar();
				cha2=cha;
				inptr2=inptr;
				break;
			}
			if(bufrm){
				free(bufrm);
				bufrm=NULL;
			}
			if(strinf.bufstr){
				free(strinf.bufstr);
				strinf.bufstr=NULL;
			}
			i=inptr2;
			bcha=cha2;
			nexttok();
		}
		CharToBackBuf('}');
		CharToBackBuf(0);
		buf=(unsigned char *)REALLOC(BackTextBlock,SizeBackBuf);
		SizeBackBuf=0;
	}

	blinenum=linenumber;
	nexttok();
///////////////////
	if(tok==tk_openbrace){
		if(COMPARE&&(icomp+numcomp)->use_cxz==zerocompr){
			warcompneqconst();
			cha=cha2;
			inptr=inptr2;
			SkipBlock();
			inptr2=inptr;
			cha2=cha;
			linenum2=linenumber;
			nexttok();
		}
		else{
			startblock();
			doblock();
			nexttok();
			endblock();
		}
	}
	else{
		if(COMPARE&&(icomp+numcomp)->use_cxz==zerocompr){
			warcompneqconst();
			do{
				nexttok();
			}while(tok!=tk_semicolon&&tok!=tk_eof);
		}
		else docommand();
	}

	RestoreStack();
	SetContinueLabel();
//	puts((char *)string2);
//	printf("end for, curco=%u curbr=%u\n",curco,curbr);
	if(modif){
		unsigned int oldlinenum=linenum2;
		ITOK oitok;
		oitok=itok2;
		BackTextBlock=(char *)buf;
		linenum2=blinenum;
		RunBackText();
		linenumber=linenum2=oldlinenum;
		itok2=oitok;
	}

	if(COMPARE==FALSE||(COMPARE&&(icomp+numcomp)->use_cxz!=zerocompr)){
		if(COMPARE&&(icomp+numcomp)->use_cxz==voidcompr)warcompeqconst();
		jumploc(conloc);//JMP на начало цикла
	}

	if(COMPARE){
		for(unsigned int i=0;i<numcomp;i++){
			if((icomp+i)->type!=tk_oror){
				if(typeb==tk_FOR){
					if((outptr-(icomp+i)->loc)>127)jumperror(ifline,mesFOR);
					output[(icomp+i)->loc-1]=(unsigned char)(outptr-(icomp+i)->loc);
				}
				else{
					if((outptr-(icomp+i)->loc)<=127)warningjmp(mesFOR,ifline);
					if(am32==FALSE)*(unsigned short *)&output[(icomp+i)->loc-2]=(unsigned short)(outptr-(icomp+i)->loc);
					else *(unsigned long *)&output[(icomp+i)->loc-4]=(unsigned long)(outptr-(icomp+i)->loc);
				}
			}
		}
		free(icomp);
	}
	if(retproc){
		if(bakregstat)CopyRegStat(bakregstat);
	}
	else if(changeregstat)CompareRegStat(changeregstat);
	if(changeregstat)CopyRegStat(changeregstat);
	SetBreakLabel();
	if(usebr[curbr]!=0||useco[curco]!=0)clearregstat();
	FreeStat(bakregstat);
	FreeStat(changeregstat);
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	if(ESPloc&&am32&&oaddESP!=addESP)warESP();
	lastcommand=tk_for;
}

void decit(int dectok,ITOK *detok,char *&decbuf,SINFO *dstr)
// outputs code to decrement the given variable one.
{
int vop=0,i=0,razr=r16;
	switch(dectok){
		case tk_dwordvar:
		case tk_longvar:
			CheckAllMassiv(decbuf,4,dstr,detok);
			op66(r32);
			if(cpu<3)cpu=3;
			vop=1;
			goto l2;
		case tk_wordvar:
		case tk_intvar: vop=1;
			op66(r16);
			i=1;
		case tk_bytevar:
		case tk_charvar:
			i++;
			CheckAllMassiv(decbuf,i,dstr,detok);
			if(vop!=0)op66(r16);
l2:
			outseg(detok,2);
			op(0xFE + vop);
			op(0x08+detok->rm);
			outaddress(detok);
			break;
		case tk_reg32:
			if(cpu<3)cpu=3;
			razr=r32;
		case tk_reg:
			op66(razr);
			op(0x48+detok->number);
			break;
		case tk_beg:
			op(0xFE);
			op(0xC8+detok->number);
			break;
		default:
			preerror(invaliddecrem);
			break;
	}
}

void uptdbr(/*int usesw*/)
{
	listbr[curbr]=numbr;	//номер этого цикла
	usebr[curbr]=0;
	curbr++;	//число вложений
	numbr++;	//всего циклов
//	if(!usesw){
		useco[curco]=0;
		curco++;
//	}
	if(curbr==MAXIN)preerror("to many inside bloks");
}

void doloop(unsigned int typeb) 							// both short and long loops
{
unsigned int startloc,startloc2;
int looptok;
ITOK lootok;
char *loopbuf;
signed char delta;
SINFO lstr;
int j=0,sline=linenumber;
unsigned int oaddESP=addESP;
	nexttok();
//	printf("tok=%u name=%s bufrm=%s\n",tok,itok.name,bufrm);
	expecting(tk_openbracket);
	looptok=tok;
	lootok=itok;
	loopbuf=bufrm;
	bufrm=NULL;
	lstr=strinf;
	strinf.bufstr=NULL;
//	printf("bufrm=%s strinf=%s\n",loopbuf,lstr.bufstr);
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	uptdbr();
	if(dbg&1)KillLastLine();
	if((tok<tk_charvar||tok>tk_dwordvar)&&tok!=tk_reg&&tok!=tk_reg32
			&&tok!=tk_beg&&tok!=tk_closebracket)
		preerror(invaliddecrem);
	if(tok!=tk_closebracket){
		if(typeb!=tk_loop){
			if(dbg)AddLine();
			int vop=tok==tk_reg?r16:r32;
			if(typeb==tk_LOOPNZ&&((tok==tk_reg&&itok.number==CX)||
					(tok==tk_reg32&&itok.number==ECX))&&(!(optimizespeed&&chip>3&&chip<7))){
				op67(vop);
				outword(0xE3);
			}
			else{
				if(tok==tk_reg||tok==tk_reg32||tok==tk_beg){
					if(tok==tk_beg)op(0x84);
					else{
						op66(vop);
						op(0x85);
					}
					op(0xc0+(unsigned int)itok.number*9);
				}
				else{
					ITOK htok2;
					htok2.number=0;
					htok2.rm=(am32==FALSE?rm_d16:rm_d32);
					htok2.segm=DS;
					htok2.post=0;
					htok2.sib=(am32==FALSE?CODE16:CODE32);
					htok2.flag=0;
					int razr=r_undef;
					switch(tok){
						case tk_longvar:
						case tk_dwordvar:
							razr=2;
						case tk_intvar:
						case tk_wordvar:
							razr++;
						case tk_charvar:
						case tk_bytevar:
							razr++;
					}
					outcmp(0,tok,&itok,bufrm,&strinf,tk_number,&htok2,loopbuf,&lstr,razr);
				}
				if(typeb==tk_LOOPNZ)outword(0x74);
				else{
					if(chip<3){
						outword(0x0375);	// JNZ past jump up
						op(0xE9);
					}
					else{
						outword(0x840F);
					}
					outword(0);
					if(am32!=FALSE)outword(0);
				}
			}
		}
		nexttok();	//то что уменьшается
	}
	expecting(tk_closebracket);
	startloc2=outptr;
	if(AlignCycle)AlignCD(CS,aligncycle);
	startloc=outptr;
	docommand();
	RestoreStack();
	SetContinueLabel();
	if(looptok!=tk_closebracket){
		if(((outptr-startloc)<=(127-3))&&(chip<3||optimizespeed==0)&&
		   ((looptok==tk_reg&&lootok.number==CX)||(looptok==tk_reg32&&lootok.number==ECX))){
			delta=(char)(startloc-(outptr+2));
			if(op67(looptok==tk_reg?r16:r32)!=FALSE)delta--;
			op(0xE2);
			op(delta);	/* LOOP 'delta' */
		}
		else{
			decit(looptok,&lootok,loopbuf,&lstr);
			if((outptr-startloc)>(unsigned int)(127-2-j)){	 /* long jump */
				if(chip<3){
					outword(0x0374);	// JZ past jump up
					op(0xE9);
				}  /* JMP top of loop */
				else{
					outword(0x850F);
					if(cpu<3)cpu=3;
				}
				if(am32==FALSE)outword(startloc-(outptr+2));
				else outdword(startloc-(outptr+4));
			}
			else{
				op(0x75);   // short jump
				op(startloc-(outptr+1));
			} /* JNZ 'delta' */
		}
	}
	else jumploc(startloc);//JMP на начало цикла
	if(typeb!=tk_loop){
		looptok=outptr-startloc2;
		if(typeb==tk_LOOPNZ){
			if(looptok>127)jumperror(sline,mesLOOPNZ);
			output[startloc2-1]=(unsigned char)looptok;
		}
		else{
			if(looptok<=127)warningjmp(mesLOOPNZ,sline);
			if(am32==FALSE)*(unsigned short *)&output[startloc2-2]=(unsigned short)looptok;
			else *(unsigned long *)&output[startloc2-4]=(unsigned long)looptok;
		}
	}
	SetBreakLabel();
	if(usebr[curbr]!=0||useco[curco]!=0||typeb!=tk_loop)clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	if(ESPloc&&am32&&oaddESP!=addESP)warESP();
	lastcommand=tk_loop;
}

void GetNameLabel(int type,int num)
{
	sprintf((char *)string2,type==tk_break?"BREAK%04X":"CONTINUE%04X",listbr[num]);
}

void SetBreakLabel()
{
	curbr--;
	if(usebr[curbr]!=0){
		GetNameLabel(tk_break,curbr);
		updatecall((unsigned int)updatelocalvar((char *)string2,tk_number,outptr),outptr,procedure_start);
//		clearregstat();
	}
}

void SetContinueLabel()
{
	curco--;
	if(useco[curco]!=0){
		GetNameLabel(tk_continue,curco);
		updatecall((unsigned int)updatelocalvar((char *)string2,tk_number,outptr),outptr,procedure_start);
	}
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void SaveDataVal(unsigned int ssize,unsigned long long val)
{
	switch(ssize){
		case 1: opd(val);	break;
		case 2: outwordd(val); break;
		case 4: outdwordd(val); break;
		case 8: outqwordd(val); break;
	}
}

long AddVarString()
{
long loop=0;
int term;
	do{
		term=itok.flag;
		for(int i=0;i<itok.number;i++){	//ввести строку
			opd(string[i]);
			loop++;
		}
		nexttok();
	}while(tok==tk_string);
	switch(term&3){
		case zero_term:
			if(term&s_unicod)opd(0);
			opd(0);
			loop++;
			break;
		case dos_term:
			if(term&s_unicod)opd(0);
			opd('$');
			loop++;
			break;
	}
	return loop;
}

long initglobalvar(int type,long elements,long ssize,char typev)
{
long loop;
long long i=0;
int htok;
char name[IDLENGTH];
	nexttok();
	loop=0;
	if(dbg&2)AddDataLine((tok==tk_string&&typev!=pointer?(char)3:(char)ssize));
loopsw:
	htok=tok;
	switch(tok){	//заполнить величинами
		case tk_apioffset: AddApiToPost(itok.number); nexttok(); break;
		case tk_postnumber:
			(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
			itok.flag=0;
			goto cn1;
		case tk_undefofs:
			strcpy(name,itok.name);
//			AddUndefOff(1,itok.name);
cn1:
			tok=tk_number;
		case tk_minus:
		case tk_number:
		  if(type==tk_byte||type==tk_word||type==tk_dword)i+=doconstdwordmath();
			else if(type==tk_float)i=doconstfloatmath();
			else if(type==tk_double)i=doconstdoublemath();
			else if(type==tk_qword)i+=doconstqwordmath();
			else i+=doconstlongmath();
			if(tok==tk_plus&&tok2==tk_postnumber&&htok!=tk_undefofs){
				nexttok();
				goto loopsw;
			}
			if(elements!=0){
				for(;loop<elements;loop++){
				 	if(postnumflag&f_reloc)AddReloc();
				 	if(htok==tk_undefofs)AddUndefOff(3,name);
					SaveDataVal(ssize,i);
				}
			}
			loop=loop*ssize;
			break;
		case tk_string:
			if(typev==pointer){
				loop=(am32==FALSE?2:4);
				i=addpoststring(DS);
				if(am32==FALSE)outwordd(i);
				else outdwordd(i);
				nexttok();
			}
			else{
				loop=AddVarString();
				if(elements!=0){
					for(;loop<ssize*elements;loop++){//дополнить 0 если короткая
						opd(aligner);
					}
				}
			}
			break;
		case tk_from:	//считать файл с данными
			nexttok();
			loop=dofrom();
			if(elements!=0){
				for(;loop<ssize*elements;loop++)opd(aligner);
			}
			nexttok();
			break;
		case tk_extract:	//считать фрагмент файла с данными
			nexttok();
			loop=doextract();
			if(elements!=0){
				for(;loop<ssize*elements;loop++)opd(aligner);
			}
			break;
		case tk_openbrace:	//массив данных
			nexttok();
			while(tok!=tk_closebrace){
				htok=tok;
				if(typev==pointer){
					if(tok==tk_string){
						i=addpoststring(DS);
						nexttok();
					}
					else if(tok==tk_number||tok==tk_minus||tok==tk_undefofs){
						if(tok==tk_undefofs){
							tok=tk_number;
							strcpy(name,itok.name);
//							AddUndefOff(1,itok.name);
						}
//						else if((itok.flag&f_reloc)!=0)AddReloc();
						i=doconstdwordmath();
					}
					else{
						numexpected();
						nexttok();
					}
				 	if(postnumflag&f_reloc)AddReloc();
				 	if(htok==tk_undefofs)AddUndefOff(3,name);
					if(am32==FALSE)outwordd(i);
					else outdwordd(i);
				}
   	    else{
					switch(tok){
						case tk_apioffset: AddApiToPost(itok.number); nexttok(); break;
						case tk_string:
							loop+=AddVarString()/ssize-1;
							break;
						case tk_postnumber:
							(itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
							itok.flag=0;
							goto cn2;
						case tk_undefofs:
							strcpy(name,itok.name);
//							AddUndefOff(1,itok.name);
cn2:
							tok=tk_number;
						case tk_number:
						case tk_minus:
						  if(type==tk_byte||type==tk_word||type==tk_dword)i=doconstdwordmath();
							else if(type==tk_float)i=doconstfloatmath();
							else if(type==tk_double)i=doconstdoublemath();
							else if(type==tk_qword)i=doconstqwordmath();
							else i=doconstlongmath();
//							if((postnumflag&f_reloc)!=0)AddReloc();
						 	if((postnumflag&f_reloc)!=0)AddReloc();
						 	if(htok==tk_undefofs)AddUndefOff(3,name);
							SaveDataVal(ssize,i);
							break;
						default:
							numexpected();
							nexttok();
							break;
					}
				}
				loop++;
				if(tok==tk_closebrace)break;
				expecting(tk_camma);
			}
			if(elements!=0){
				for(;loop<elements;loop++)SaveDataVal(ssize,aligner);
			}
			loop=loop*ssize;
			nexttok();
			break;
		default:
//			printf("tok=%d\n",tok);
			numexpected(); nexttok(); break;
	}
	return loop;
}

void AddPostData(unsigned int loop)
{
	if(dynamic_flag==0){
		unsigned int longpostsize=loop+postsize;
		if(am32==FALSE&&longpostsize>0xFFFFL)tobigpost();
		else postsize=longpostsize;
	}
}

void labelindata()
{
//idrec *varrec;
	FindOff((unsigned char *)itok.name,DS);
	tok=tk_number;
	itok.number=outptrdata;
	itok.segm=DS;
	string[0]=0;
	if(FixUp)itok.flag=f_reloc;
	/*varrec=*/addtotree(itok.name);
	/*varrec->count=*/FindOff((unsigned char *)itok.name,DS);
	nexttok();
	nexttok();
}

void globalvar()	 /* both initialized and unitialized combined */
{
long size,loop,i,elements,ssize;
char done=0,typev;
char var_name[IDLENGTH];
int type=itok.rm,typebak;	//тип переменной
unsigned int flag,fflag=itok.flag,dynamic;
unsigned int npointr=itok.npointr;
int count;
idrec *varrec;
	size=typesize(type);	//размер переменной
	if(FixUp)fflag|=f_reloc;
	typebak=type;
	while(tok!=tk_eof&&done==0){
		int nnpointr=0;
		typev=variable;
		ssize=size;
		flag=fflag;
		type=typebak;
//		printf("type=%d\n",type);
		if(tok==tk_far){
			flag|=f_far;
			nexttok();
		}
		while(tok==tk_mult){	//указатель
			npointr++;
			nexttok();
		}
		if(tok==tk_openbracket){
			nexttok();
			while(tok==tk_mult){	//указатель на процедуру
				nnpointr++;
				nexttok();
			}
		}
		if(npointr){
			if((flag&f_far)!=0||am32!=FALSE)ssize=4;
			else ssize=2;
			typev=pointer;
			type=am32==FALSE?tk_word:tk_dword;
		}
//			printf("tok=%d %s\n",tok,itok.name);
		switch(tok){
			case tk_id:
			case tk_ID:
				if(tok2==tk_openbracket||nnpointr){
					if(npointr)type=am32==FALSE?tk_word:tk_dword;
					declare_procedure(flag,type,nnpointr);
					break;
				}
				strcpy(var_name,itok.name);	//имя переменной
				elements=1;
				nexttok();
				if(tok==tk_openblock){	//[
					nexttok();
					if(tok==tk_closeblock){//неизвестное число элементов
						elements=0;
						nexttok();
					}
					else{
				 		CheckMinusNum();
						if(tok!=tk_number){
							numexpected();
							nexttok();
						}
						else{
							elements=doconstlongmath();	//число элементов
							expecting(tk_closeblock);
						}
					}
				}
				if(type==tk_void){
					preerror("type 'void' not use for declare variables");
					break;
				}
				dynamic=FALSE;
				if(tok==tk_assign||(notpost==TRUE&&dynamic_flag==0)){	//= инициализированая переменная
					if((flag&f_extern))preerror("extern variable do not initialize at declare");
					i=tok;
					itok.type=tp_gvar;// 11.07.05 21:56 tp_ucnovn;
					SetNewTok(type,typev);
					if(useStartup==TRUE&&i!=tk_assign&&SaveStartUp(size*elements,var_name)!=FALSE){
						if(elements==0)ZeroMassiv();	//ошибка
						tok=i;
						break;
					}
					if(alignword&&ssize&&(!dynamic_flag))alignersize+=AlignCD(DS,ssize);
					itok.number=dynamic_flag==0?outptrdata:0;
					itok.segm=(comfile==file_rom&&modelmem==TINY?CS:DS);
					itok.flag=flag;
//					itok.post=dynamic;
					itok.size=elements*ssize;
					itok.rm=(am32==FALSE?rm_d16:rm_d32);
					itok.npointr=(unsigned short)npointr;

					varrec=addtotree(var_name);
					if((count=FindOff((unsigned char *)var_name,DS))==0){
						if(dynamic_flag)dynamic=DYNAMIC_VAR;
					}
					else if(dynamic_flag)dynamic=USED_DIN_VAR;
					if(i!=tk_assign){
						if(elements==0)ZeroMassiv();
						else if(notpost==TRUE){
							for(loop=0;loop<elements;loop++)SaveDataVal(ssize,aligner);
							loop=loop*ssize;
							varrec->recsize=loop;
							datasize+=loop;
						}
						tok=i;
						break;
					}
					if(dynamic){
						varrec->sbuf=dynamic_var();
						varrec->recpost=dynamic;
					}
					else{
						loop=initglobalvar(type,elements,ssize,typev);
						varrec->recsize=loop;
						datasize+=loop;
					}
					varrec->count=count;
				}
				else{
					if(elements==0){
						ZeroMassiv();
						break;
					}
					if(CheckUseAsUndef((unsigned char *)var_name)==0&&dynamic_flag)dynamic=TRUE;
					switch(tok){	//неинициализированные
						default: expected(';');
						case tk_semicolon: done=1;//	;
						case tk_camma:	 //, post global type
							itok.type=tp_postvar;//11.07.05 21:57 tp_ucnovn;
							SetNewTok(type,typev);
							if((flag&f_extern)==0&&useStartup==TRUE&&dynamic==0){
								if(SaveStartUp(ssize*elements,var_name)!=FALSE){
									nexttok();
									break;
								}
							}
							if((flag&f_extern)==0&&alignword&&dynamic==0){	//выровнять на четный адрес
								if(ssize==2){
									if(postsize%2==1)postsize++;
								}
								else if(ssize==4&&postsize%4!=0)postsize+=4-(postsize%4);
							}
							count=FindOff((unsigned char *)var_name,VARPOST);
							itok.post=dynamic+1;
							itok.segm=DS;
							loop=elements*ssize;
							itok.number=(flag&f_extern)==0?postsize:externnum++;
							itok.flag=flag;
							itok.size=loop;
							itok.rm=(am32==FALSE?rm_d16:rm_d32);
							itok.npointr=(unsigned short)npointr;
							varrec=addtotree(var_name);
							varrec->count=count;
							if((flag&f_extern)==0)AddPostData(loop);
							nexttok();
							break;
					}
				}
				break;
			case tk_undefproc:
				if(tok2==tk_openbracket||nnpointr){
					if(npointr)type=am32==FALSE?tk_word:tk_dword;
					declare_procedure(flag,type,nnpointr);
					break;
				}
			case tk_proc:
			case tk_floatvar:
			case tk_dwordvar:
			case tk_longvar:
			case tk_charvar:
			case tk_intvar:
			case tk_bytevar:
			case tk_pointer:
			case tk_wordvar: idalreadydefined(); nexttok(); break;
			default: expected(';');
			case tk_semicolon: done=1;
			case tk_camma: nexttok(); break;
		}
		npointr=0;
	}
	dopoststrings();
}

void SetNewTok(int type,int typev)
{
	switch(typev){
		case variable:
			if(type>=tk_char&&type<=tk_double)tok=type+(tk_charvar-tk_char);
			break;
		case pointer:
//			if(type>=tk_void&&type<=tk_float){
				tok=tk_pointer;
				itok.type=(unsigned short)type;
//			}
			break;
	}
}

int SaveStartUp(int size,char *var_name)
{
int i=0;
	if((startStartup+size)<=endStartup){
		if(alignword){	//выровнять на четный адрес
			if(size==2){
				if(startStartup%2==1)i=1;
			}
			else if(size==4&&startStartup%4!=0)i=4-(startStartup%4);
		}
		if((startStartup+size+i)<=endStartup){
			startStartup+=i;
			itok.number=startStartup;
			itok.segm=DS;
			itok.flag=0;
			itok.post=0;
			itok.rm=(am32==FALSE?rm_d16:rm_d32);
			itok.type=tp_ucnovn;
			addtotree(var_name);
			startStartup+=size;
			return TRUE;
		}
	}
	return FALSE;
}

/* ======= старт заголовка процедуры ======== */

void setuprm()
{
	itok.rm=returntype=(itok.rm==tokens?am32==FALSE?tk_word:tk_dword:itok.rm);
	if(itok.npointr)itok.rm=returntype=(am32==FALSE?tk_word:tk_dword);
}

void eaxToFloat(int reg=AX)
{
int next=1;
	CheckMinusNum();
	if(itok2.type==tp_opperand){	//составное
		doeaxfloatmath(tk_reg32,reg);
		next=0;
	}
	else{
		switch(tok){
			case tk_number:
				if(itok.rm==tk_double)itok.fnumber=itok.dnumber;
				else if(itok.rm!=tk_float){
					float temp=itok.number;
					*(float *)&itok.number=temp;
				}
				op66(r32);
				op(0xb8+reg);	// MOV EAX,#
				outdword(itok.number);
				break;
			case tk_floatvar:
				if(reg==AX&&itok.rm==rm_d16&&itok.sib==CODE16){
					op66(r32);
					outseg(&itok,1);
					op(0xA1);
					outword((unsigned int)itok.number);
				}
				else{
					CheckAllMassiv(bufrm,4,&strinf);
					op66(r32);
					outseg(&itok,2);
					op(0x8B);
					op(itok.rm+reg*8);
					outaddress(&itok);
				}
				break;
			default:
				if(doeaxfloatmath(tk_reg32,reg)!=tk_reg32&&reg!=AX){
					if(!am32)Leave();
					op66(r32);
					op(0x89);
					op(0xc0+reg);
				}
				next=0;
		}
	}
	if(next)nexttok();
}

void CalcRegPar(int reg,int def,char **ofsstr)
{
char signflag=0;
int razr;
unsigned char oinline=useinline;
	useinline=0;
//	if(*ofsstr)puts(*ofsstr);
	if(tok!=tk_camma){
		if(tok==tk_openbracket){
			nexttok();
			if(tok>=tk_char&&tok<=tk_double)def=tok;
			nexttok();
			expectingoperand(tk_closebracket);
		}
		if(tok>=tk_char&&tok<=tk_double){
			def=tok;
			getoperand();
		}
		if(tok==tk_string){
  		op(0xB8+reg);
			if(am32==FALSE)outword(addpoststring());
			else outdword(addpoststring());
			nexttok();
			razr=(am32==FALSE?r16:r32);
		}
		else if(tok==tk_floatvar)eaxToFloat(reg);
		else{
			switch(def){
				case tk_int: signflag=1;
				case tk_word:
					if(reg==AX)do_e_axmath(signflag,r16,ofsstr);
					else getintoreg(reg,r16,signflag,ofsstr);
					razr=r16;
					break;
				case tk_char: signflag=1;
				case tk_byte:
					if(reg==AX)doalmath(signflag,ofsstr);
					else{
						getintobeg(reg,ofsstr);
						dobegmath(reg);
					}
					razr=r8;
					break;
				case tk_long: signflag=1;
				case tk_dword:
					if(reg==AX)do_e_axmath(signflag,r32,ofsstr);
					else getintoreg(reg,r32,signflag,ofsstr);
					razr=r32;
					break;
				case tk_float:
					eaxToFloat(reg);
					razr=r32;
					break;
				case tk_qword:
					getintoreg64(reg);
					doregmath64(reg);
					razr=r64;
					break;
				case tokens:
					razr=(am32==FALSE?r16:r32);
					if(reg==AX)do_e_axmath(0,razr,ofsstr);
					else getintoreg(reg,razr,signflag,ofsstr);
					break;
			}
		}
		AddRegistr(razr,reg);
//					printf("%08X\n",reg);
	}
	useinline=oinline;
}

int GetTypeParam(char c)
{
	switch(c){
		case 'B': return tk_byte;
		case 'W': return tk_word;
		case 'D':	return tk_dword;
		case 'C': return tk_char;
		case 'I': return tk_int;
		case 'L': return tk_long;
		case 'F': return tk_float;
		case 'A': return tk_multipoint;
		case 'Q': return tk_qword;
		case 'E': return tk_double;
		case 'S': return tk_fpust;
		case 'T': return tk_struct;
		case 'U': return tokens;
	default:
			extraparam();
//			printf("%c\n",c);
			return 0;
	}
}

void doregparams()
{
int i=0;
char *ofsstr=NULL;
int razr;
int retreg;
	ClearRegister();
	if(tok!=tk_openbracket)expected('(');
	while(tok2==tk_camma){
		nexttok();
		i++;
	}
	ofsstr=GetLecsem(tk_camma,tk_closebracket);
	getoperand();
	if(tok!=tk_closebracket){
		if(strlen(param)!=0){
		int def;
			char *oparam=BackString(param);
			for(;;){
				while(tok==tk_camma){
					if(ofsstr)free(ofsstr);
					ofsstr=GetLecsem(tk_camma,tk_closebracket);
					getoperand();
				}
				if((def=GetTypeParam(oparam[i++]))==0){
					nexttok();
					break;
				}
				if(def==tk_qword){
					int c1,c2;
					c1=oparam[i++]-0x30;
					c2=oparam[i++]-0x30;
					retreg=c1|(c2*256);
//					printf("%08X\n",retreg);
				}
				else retreg=oparam[i++]-0x30;
				if(ofsstr){
					if((razr=getrazr(def))!=r64){
						int retr;
						if((retr=CheckIDZReg(ofsstr,retreg,razr))!=NOINREG){
							GetEndLex(tk_camma,tk_closebracket);
							if(retr!=SKIPREG)GenRegToReg(retreg,retr,razr);
							nexttok();
							goto endparam;
						}
					}
				}
				if(def==tk_fpust)float2stack(retreg);
				else CalcRegPar(retreg,def,&ofsstr);
endparam:
				if(ofsstr&&razr!=r64){
					IDZToReg(ofsstr,retreg,razr);
					free(ofsstr);
					ofsstr=NULL;
				}
				if(tok!=tk_camma){
					if(tok!=tk_closebracket)expected(')');
					if(oparam[i]!=0)missingpar();
					break;
				}
			}
			free(oparam);
		}
		else{
char regpar[6]={AX,BX,CX,DX,DI,SI};
			for(;i<6;i++){
				if(tok==tk_camma){
					if(ofsstr)free(ofsstr);
					ofsstr=GetLecsem(tk_camma,tk_closebracket);
					getoperand();
					if(tok==tk_camma)continue;
				}
				retreg=regpar[i];
				if(ofsstr&&tok!=tk_camma){
					razr=(am32==FALSE?r16:r32);
					int retr;
					if((retr=CheckIDZReg(ofsstr,retreg,razr))!=NOINREG){
						GetEndLex(tk_camma,tk_closebracket);
						if(retr!=SKIPREG)GenRegToReg(retreg,retr,razr);
						nexttok();
						goto endparam1;
					}
				}
				CalcRegPar(retreg,tokens,&ofsstr);
endparam1:
				if(ofsstr){
					IDZToReg(ofsstr,retreg,razr);
					free(ofsstr);
					ofsstr=NULL;
				}
				if(tok!=tk_camma){
					if(tok!=tk_closebracket)expected(')');
					break;
				}
			}
		}
		setzeroflag=FALSE;
	}
	if(ofsstr)free(ofsstr);
}

int CheckUses()
{
int i;
int regs=0;
int bracket=FALSE;
	memset((SAVEREG *)psavereg,0,sizeof(SAVEREG));
	if(tok==tk_openbracket){
		if(stricmp(itok2.name,"uses")==0){
			bracket=TRUE;
			nexttok();
		}
		else return 0;
	}
	if(stricmp(itok.name,"uses")==0){
		nexttok();
		while(tok==tk_reg32||tok==tk_reg||tok==tk_beg){
			i=r32;
			switch(tok){
				case tk_beg:
					if(itok.number>3)itok.number-=4;
					i=(am32+1)*2;
					break;
				case tk_reg:
					if(!am32)i=r16;
					break;
			}
			regs=regs|(1<<itok.number);
			psavereg->reg[itok.number]=i;
			i=1;
			if((am32&&i==r16)||(am32==0&&i==r32))i=2;
			psavereg->size+=i;
			nexttok();
			if(tok==tk_camma)nexttok();
		}
		if(strcmp(itok.name,"allregs")==0){
			psavereg->size=1;
			psavereg->all=1;
			regs=dEAX|dEBX|dECX|dEDX|dEBP|dEDI|dESI;
			nexttok();
		}
	}
	if(bracket)expecting(tk_closebracket);
	return regs;
}

void Enter()
{
	if(ESPloc==FALSE||am32==FALSE){
		op(0x55);       //push bp
		outword(0xe589);//mov bp,sp
	}
}

void setproc(int defflag)
{
char *bstring;
unsigned char oinline,ooptimizespeed;
ITOK otok;
unsigned int i;
int regs=0;
unsigned char oESPloc=ESPloc;
int startline=linenumber;
unsigned int oregidx;

	clearregstat();
	addESP=inlineflag=0;
	oinline=useinline;
	ooptimizespeed=optimizespeed;
	oregidx=*(unsigned int *)&idxregs;
	current_proc_type=itok.flag;

	if(itok.flag&f_extern)notexternfun();	//new 18.04.07 12:20
//	printf("%s tok=%d flag=%08X\n",itok.name,tok,itok.flag);

	tok=lastcommand=tk_proc;
	itok.number=outptr;
	if(itok.segm<NOT_DYNAMIC)itok.segm=DYNAMIC_SET;
	setuprm();
//	printf("rm=%d %s\n",itok.rm,itok.name);
	if(defflag){	//ранее уже были вызовы
//		updatecall(updatetree(),(unsigned int)itok.number,0);
		regs=itok.post;
		if(updatecall(updatetree(),(unsigned int)itok.number,0)==-1&&
					strcmp(itok.name,mesmain)==0/*&&jumptomain!=CALL_NONE*/){
			itok.number=outptr=outptr-(jumptomain==CALL_SHORT?2:am32==FALSE?3:5);
			if(dbg){
				KillLastLine();
				KillLastLine();
				AddLine();
			}
			updatetree();
		}
 	}
	else{	//иначе добавить в дерево
		string[0]=0;
		itok.type=tp_ucnovn;
		addtotree(itok.name);
	}
//	puts(itok.name);
	if((i=FindOff((unsigned char *)itok.name,CS))!=0){
		itok.rec->count=i;
	}
	bstring=BackString((char *)string);
	otok=itok;
	if(strcmp(mesmain,otok.name)==0){
		if(startuptomain==TRUE&&comfile==file_com)doprestuff();
		if(numdomain!=0){
			for(i=0;i<numdomain;i++){
				strcpy(itok.name,domain+i*IDLENGTH);
				tok=tk_ID;
				searchvar(itok.name,0);
				switch(tok){
					case tk_ID:
					case tk_id:
						tobedefined(am32==FALSE?CALL_NEAR:CALL_32,tk_void);
						(postbuf+posts-1)->loc=outptr+1;
						callloc0();			/* produce CALL [#] */
						break;
					case tk_declare:
						tok=tk_undefproc;
						updatetree();
					case tk_undefproc:
						addacall(itok.number,(unsigned char)((itok.flag&f_extern)!=0?CALL_EXT:(am32!=FALSE?CALL_32:CALL_NEAR)));
						callloc0();		/* produce CALL [#] */
						break;
					case tk_proc:
						if(itok.flag&f_far)op(0x0e);	//push cs
						if(itok.segm<NOT_DYNAMIC){
							addacall(itok.number,(unsigned char)(am32!=FALSE?CALL_32:CALL_NEAR));
							itok.number=0;
						}
						callloc(itok.number);
						break;
					default:
						preerror("impossible type for startup function");
						break;
				}
			}
			numdomain=0;
			free(domain);
		}
	}
	nextexpecting2(tk_openbracket);
	startblock();
	if(searchteg&&(!(current_proc_type&f_static))&&(current_proc_type&f_typeproc)!=tp_pascal)AddThis();
	if(tok!=tk_closebracket){
		if((current_proc_type&f_typeproc)!=tp_fastcall){
			declareparams();
			if(otok.type==tp_declare&&defflag&&strcmp(bstring,param)!=0){
//					printf("old=%s new=%s\n",bstring,param);
					redeclare(otok.name);
			}
//			if(bstring[0]!=0&&bstring[0]!='A'&&strcmp(bstring,param)!=0)redeclare();
		}
		else{
			declareparamreg();
		}
		if(bstring)free(bstring);
		bstring=BackString((char *)param);
	}
	else if((current_proc_type&f_typeproc)!=tp_fastcall&&bstring[0]!=0){
		if(bstring[0]!='V'&&bstring[0]!='A'){
			redeclare(otok.name);
//			puts(bstring);
		}
	}

	strcpy((char *)string,bstring);
	itok=otok;
	tok=tk_proc;
	updatetree();

	if(searchteg&&(!(current_proc_type&f_static))&&(current_proc_type&f_typeproc)==tp_pascal)AddThis();
	nexttok();
	blockproc=TRUE;
//	printf("tok=%d\n",tok);
	if(tok==tk_inline){
		inlineflag=1;
		nexttok();
	}
	else if(paramsize)Enter();	// PUSH BP  MOV BP,SP
	if(tok==tk_colon&&(current_proc_type&fs_constructor)!=0){
		do{
			AddBackBuf(inptr2,cha2);
		}while(tok==tk_camma);
		nexttok();
	}
	regs|=CheckUses();
	CorrectParamVar();
	if(tok!=tk_openbrace)declarelocals(0,inlineflag);
#ifdef OPTVARCONST
	ClearLVIC();
#endif
//	numblocks++;	//на этом месте для ранего определения ::var
	expecting(tk_openbrace);
	declarelocals(1,inlineflag);
	retproc=FALSE;
	if(paramsize||localsize/*||(lstructlist!=NULL)*/){
		initBP=1;
		if(inlineflag)warninline();
	}

#ifdef __NEWLEX__
	doblockstart(otok.name);
#endif
	doblock2();							// do proc
	if(current_proc_type&fs_destructor){
elementteg *bazael=searchteg->baza;
		for(i=0;i<searchteg->numoper;i++){
			if((bazael+i)->tok==tk_baseclass)CallDestructor((structteg *)(bazael+i)->rec);
		}
	}
	i=numreturn;
	setreturn();
	if(inlineflag==0&&retproc==FALSE&&(i||(lastcommand!=tk_goto&&lastcommand!=tk_GOTO)))leaveproc();
	endblock();
	initBP=0;
	strcpy((char *)string,bstring);
	itok=otok;
	tok=tk_proc;
	itok.size=outptr-procedure_start;
	itok.rec->recpost=regs;
//	printf("reg=%08X set=%s\n",regs,itok.name);
#ifdef OPTVARCONST
	if(inlineflag)itok.flag|=f_useidx;
#endif
	updatetree();
	if(mapfile)mapfun(startline);
	i=linenumber;
	linenumber=startline;
	killlocals();
	linenumber=i;
	free(bstring);
	optimizespeed=ooptimizespeed;
	useinline=oinline;
	*(unsigned int *)&idxregs=oregidx;
	if(searchteg)searchteg=NULL;
	blockproc=FALSE;
	ESPloc=oESPloc;
	nexttok();
}

void insertproc(/*int sizepar*/)
{
unsigned char oinline,ooptimizespeed;
struct treelocalrec  *otlr;
struct structteg *olteglist;
//struct idrec     *olstructlist;
struct idrec *rec;
unsigned int oparamsize;
unsigned int olocalsize;
unsigned char oinsertmode;
unsigned int onumblocks;	//номер вложенного блока
unsigned int osizestack;
RETLIST *olistreturn;
unsigned int onumreturn;
int oinlineflag;
SAVEREG *osavr;
unsigned int oaddESP=addESP;
	addESP=0;
	clearregstat();
	osavr=psavereg;
	psavereg=(SAVEREG*)MALLOC(sizeof(SAVEREG));
	oinsertmode=insertmode;
	insertmode=TRUE;	//флаг режима вставки
	oinline=useinline;
	ooptimizespeed=optimizespeed;
	current_proc_type=itok.flag;
	rec=itok.rec;
	otlr=tlr;
	olteglist=ltegtree;
//	olstructlist=lstructlist;
	oinlineflag=inlineflag;
	osizestack=sizestack;
	sizestack=0;
	tlr=NULL;
	ltegtree=NULL;
//	lstructlist=0;
	oparamsize=paramsize;
	olocalsize=localsize;
	onumblocks=numblocks;
	olistreturn=listreturn;
	onumreturn=numreturn;
	paramsize=0;
	localsize=0;
	numblocks=0;	//номер вложенного блока
	listreturn=NULL;
	numreturn=0;
	inlineflag=0;
	nextexpecting2(tk_openbracket);
	if(tok!=tk_closebracket){
		if((current_proc_type&f_typeproc)!=tp_fastcall)declareparams();
		else declareparamreg();
	}
	CorrectParamVar();
	nexttok();
	if(tok==tk_inline){
		nexttok();
		inlineflag=1;
		expecting(tk_openbrace);
	}
	else{
		if((current_proc_type&f_typeproc)!=tp_fastcall&&paramsize>0)Enter();	// PUSH BP  MOV BP,SP
		rec->recpost|=CheckUses();
		if(tok!=tk_openbrace)declarelocals(0);
		expecting(tk_openbrace);
		declarelocals(1);
	}
	retproc=FALSE;
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	startblock();
	doblock2();
	sizestack=osizestack;						// do proc
	setreturn();
	endblock();
	RestoreSaveReg();
	if(inlineflag==0&&(localsize||paramsize)&&(ESPloc==FALSE||am32==FALSE))Leave();
	killlocals(/*0*/);
	optimizespeed=ooptimizespeed;
	useinline=oinline;
	paramsize=oparamsize;
	localsize=olocalsize;
	tlr=otlr;
	ltegtree=olteglist;
//	lstructlist=olstructlist;
	insertmode=oinsertmode;
	numblocks=onumblocks;
	listreturn=olistreturn;
	numreturn=onumreturn;
	inlineflag=oinlineflag;
	free(psavereg);
	psavereg=osavr;
	addESP=oaddESP;
//	nexttok();
//	printf("tok=%d\n",tok);
}

void AddTypeVar(int type,int pos)
{
	switch(type){
		case tk_char:
			param[pos]='C';
			break;
		case tk_byte:
			param[pos]='B';
			break;
		case tk_int:
			param[pos]='I';
			break;
		case tk_word:
			param[pos]='W';
			break;
		case tk_long:
			param[pos]='L';
			break;
		case tk_dword:
			param[pos]='D';
			break;
		case tk_float:
			param[pos]='F';
			break;
		case tk_qword:
			param[pos]='Q';
			break;
		case tk_double:
			param[pos]='E';
			break;

	}
}

void declareparamreg() /* declare procedure parameters */
{
int i=0,num=1;
unsigned char j=0;
	do{
		if(tok>=tk_char&&tok<=tk_double){
			AddTypeVar(tok,i++);
			nexttok();
			j=1;
		}
		else{
			switch(tok){
				case tk_beg:
					if(j==0)param[i++]='B';
					param[i++]=(char)(itok.number+0x30);
					j=0;
					break;
				case tk_reg:
					if(j==0)param[i++]='W';
					param[i++]=(char)(itok.number+0x30);
					j=0;
					break;
				case tk_reg32:
					if(j==0)param[i++]='D';
					param[i++]=(char)(itok.number+0x30);
					j=0;
					break;
				case tk_reg64:
					param[i++]='Q';
					param[i++]=(char)((itok.number&255)+0x30);
					param[i++]=(char)((itok.number/256)+0x30);
					j=0;
					break;
				case tk_fpust:
					param[i++]='S';
					param[i++]=(char)(itok.number+0x30);
					j=0;
					break;
				case tk_closebracket:
					if(j==0)goto endproc;
				default: edpip(num);
			}
			nexttok();
			if(tok==tk_camma){
				if(j!=0){
					edpip(num);
					param[i++]='0';
					j=0;
				}
				nexttok();
			}
		}
		num++;
	}while(tok!=tk_closebracket);
endproc:
	if(j!=0){
		edpip(num-1);
		param[i++]='0';
	}
	param[i]=0;
}

void AddMultiPoint(int pos)
{
	param[pos]='A';
	nexttok();
	if(tok!=tk_closebracket)SwTok(tk_closebracket);
}

void declareparamstack() /* declare procedure parameters */
{
int i=0,num=1;
unsigned char j=0;
/*
 1 - объявлен тип
 2 - была запятая
 3 - была точка с запятой
 4 -
 5 - был идентификатор
 6 - тип void
 */
int typevar=tk_multipoint;
structteg *tteg=NULL;
int size;
	do{
		if(tok==tk_struct)nexttok();
		if(tok>=tk_char&&tok<=tk_double){
			if(j>3||j==1)edpip(num);
			j=1;
			typevar=tok;
			nexttok();
		}
		else if((tteg=FindTeg(TRUE))!=NULL){
			size=Align(tteg->size,(am32+1)*2);
//			printf("%s size=%u\n",itok.name,size);
			j=1;
			typevar=tk_struct;
			nexttok();
		}
		while(tok==tk_mult){
			param[i++]='*';
			nexttok();
		}
		if(tok==tk_camma){
			if(j==1){
				if(typevar==tk_struct)i+=sprintf(&param[i],"T%u",size);
				else AddTypeVar(typevar,i++);
			}
			else if(j!=5)edpip(num);
			j=2;
			num++;
		}
		else if(tok==tk_semicolon){
			if(j==1){
				if(typevar==tk_struct)i+=sprintf(&param[i],"T%u",size);
				else AddTypeVar(typevar,i++);
			}
			else if(j!=5)edpip(num);
			j=3;
			num++;
		}
		else if(tok==tk_multipoint){
			AddMultiPoint(i++);
			break;
		}
		else if(tok==tk_closebracket){
			if(j==0)param[i++]='A';
			else if(j==1){
				if(typevar==tk_struct)i+=sprintf(&param[i],"T%u",size);
				else AddTypeVar(typevar,i++);
			}
			else if(j<4)edpip(num);
			break;
		}
		else if(tok==tk_void){
			if(j!=0)edpip(num);
			param[i++]='V';
			j=6;
		}
		else if(tok==tk_beg||tok==tk_reg||tok==tk_reg32||tok==tk_reg64||tok==tk_fpust){
			if(j==1){
				if(typevar==tk_struct)i+=sprintf(&param[i],"T%u",size);
				else AddTypeVar(typevar,i++);
			}
			else if(j<4){
				switch(tok){
					case tk_beg: param[i++]='B'; break;
					case tk_reg: param[i++]='W'; break;
					case tk_reg32: param[i++]='D'; break;
					case tk_fpust: param[i++]='S'; break;
					case tk_reg64:
						param[i++]='Q';
						param[i++]=(char)((itok.number&255)+0x30);
						itok.number/=256;
						break;
				}
			}
			else edpip(num);
			param[i++]=(char)(itok.number+0x30);
			j=5;
		}
		else{
			if(j==1||j==2){
				if(typevar==tk_struct)i+=sprintf(&param[i],"T%u",size);
				else AddTypeVar(typevar,i++);
			}
			else edpip(num);
			j=5;
		}
		nexttok();
	}while(tok!=tk_eof);
	param[i]=0;
//	puts(param);
}

void CorrectParamVar()
{
unsigned int addnum;
int fspin;
struct localrec *ptr;
	if(paramsize==0)return;
	if(insertmode)addnum=0;	//ret-address
	else addnum=2;
	if(ESPloc==FALSE||am32==FALSE)addnum+=2;	//EBP
	if((current_proc_type&f_far))addnum+=2;// move over seg on stack
	if(am32)addnum*=2;
	if((current_proc_type&f_typeproc)==tp_cdecl||(current_proc_type&f_typeproc)==tp_stdcall)fspin=FALSE;
	else fspin=TRUE;
treelocalrec *ntlr=tlr;
	while(ntlr&&ntlr->level>1)ntlr=ntlr->next;
	for(ptr=ntlr->lrec;;ptr=ptr->rec.next){
		if(ptr->rec.type==tp_paramvar){
			if(fspin)ptr->rec.recnumber=paramsize-ptr->rec.recnumber-Align(ptr->rec.recsize,am32==FALSE?2:4);
			ptr->rec.recnumber+=addnum;
		}
		if(ptr->rec.next==NULL)break;
	}
}

void declareparams()									 /* declare procedure parameters */
{
int paramtok,sparamtok,i=0,type;
char flag=33;
int numpointr;
localrec *lrec;
structteg *tteg=NULL,*nteg;
	do{
		if(flag!=33)nexttok();
		flag=1;
		if(tok==tk_multipoint){
			AddMultiPoint(i++);
			break;
		}
		if(tok==tk_void){
			param[i++]='V';
			nexttok();
			if(tok!=tk_closebracket)SwTok(tk_closebracket);
			break;
		}
		if(tok>=tk_char&&tok<=tk_double){
			type=tok;
			SetNewTok(tok,variable);
			paramtok=tok;
		}
		else if(tok==tk_beg||tok==tk_reg||tok==tk_reg32||tok==tk_reg64||tok==tk_fpust)flag=3;
		else{
			if(tok==tk_struct)nexttok();
			if((tteg=FindTeg(TRUE))!=NULL)paramtok=tk_struct;
			else{
				datatype_expected();
				flag=0;
				nexttok();
			}
		}
		if(flag){
			do{
				numpointr=0;
				skipfind=TRUE;
				if(flag!=3)nexttok();
				while(tok==tk_mult){
					nexttok();
					numpointr++;
					param[i++]='*';
				}
				if(tok==tk_id||tok==tk_ID){	//проверить на типы определенные через define
					skipfind=FALSE;
					int otok=tok;
					searchtree(&ptok,&otok,string);
					if(otok>=tk_char&&otok<=tk_double){
						tok=otok;
						itok=ptok;
					}
					skipfind=TRUE;
				}
				if(tok==tk_struct){
					nexttok();
					if((tteg=FindTeg(TRUE))!=NULL){
						paramtok=tk_struct;
						nexttok();
					}
					else{
						edpip();
						nexttok();
						goto endparam1;
					}
				}
				else {
					if((nteg=FindTeg(TRUE))!=NULL){
						tteg=nteg;
						paramtok=tk_struct;
						nexttok();
					}
					else{
						if(tok>=tk_char&&tok<=tk_double){
							type=tok;
							SetNewTok(tok,variable);
							paramtok=tok;
							nexttok();
							while(tok==tk_mult){
								nexttok();
								numpointr++;
								param[i++]='*';
							}
							flag=2;
						}
						skipfind=FALSE;
						if(tok==tk_beg||tok==tk_reg||tok==tk_reg32||tok==tk_reg64||tok==tk_fpust){
							if(flag==2)AddTypeVar(type,i++);
							else{
								switch(tok){
									case tk_beg: param[i++]='B'; break;
									case tk_reg: param[i++]='W'; break;
									case tk_reg32: param[i++]='D'; break;
									case tk_fpust: param[i++]='S'; break;
									case tk_reg64:
										param[i++]='Q';
										param[i++]=(char)((itok.number&255)+0x30);
										itok.number/=256;
										break;
								}
							}
							param[i++]=(unsigned char)(itok.number+0x30);
							flag=1;
							goto endparam1;
						}
					}
				}
				sparamtok=paramtok;
				if(tok==tk_multipoint){
					AddMultiPoint(i++);
					break;
				}
				if(tok!=tk_ID&&tok!=tk_id){
					idalreadydefined();
					break;
				}
				int lsize;
				if(am32)lsize=4;
				else{
					lsize=2;
					if(sparamtok>=tk_longvar&&sparamtok<=tk_floatvar&&numpointr==0)
						 lsize+=2;//add 2 more bytes
				}
				if((sparamtok==tk_qwordvar||sparamtok==tk_doublevar)&&numpointr==0)lsize=8;
				AddTypeVar(type,i++);
				if(sparamtok==tk_struct){
//idrec *newrec,*trec;
					lsize=Align(tteg->size,(am32+1)*2);
					i+=sprintf(&param[i],"T%u",lsize);
//					newrec=(struct idrec *)MALLOC(sizeof(struct idrec));
//					if(lstructlist==NULL)lstructlist=newrec;
//					else{
//						trec=lstructlist;
//						while(trec->left!=NULL)trec=trec->left;
//						trec->left=newrec;
//					}
//					newrec->right=newrec->left=NULL;
					lrec=addlocalvar(itok.name,tk_structvar,paramsize);
					lrec->rec.newid=(char *)tteg;
					lrec->rec.flag=tteg->flag;
					lrec->rec.recrm=1;
					lrec->rec.recpost=LOCAL;
					goto endparam;
				}
				lrec=addlocalvar(itok.name,sparamtok,paramsize);
				lrec->rec.npointr=(unsigned short)numpointr;
endparam:
				lrec->rec.type=tp_paramvar;
				lrec->rec.recsize=lsize;
				paramsize+=lsize;
endparam1:
				nexttok();
			}while(tok==tk_camma);
		}
	}while(tok==tk_semicolon);
	param[i]=0;
//	puts(param);
}

void CharToBackBuf(char c)
{
	if(SizeBackBuf==0){
		MaxSizeBackBuf=STRLEN;
		BackTextBlock=(char *)MALLOC(STRLEN);
	}
	else if(SizeBackBuf==MaxSizeBackBuf){
		MaxSizeBackBuf+=STRLEN;
		BackTextBlock=(char *)REALLOC(BackTextBlock,MaxSizeBackBuf);
	}
	BackTextBlock[SizeBackBuf++]=c;
}

void  AddBackBuf(int oinptr,char ocha)
//создать листинг начальной инициализации лосальных переменных
{
int numblock=0;
unsigned char save;
char bcha;
	inptr=oinptr;
	cha=ocha;
	for(;;){
		save=TRUE;
		switch(cha){
			case '(':
			case '{':
			case '[':
				numblock++;
				break;
			case ')':
			case '}':
			case ']':
				numblock--;
				if(numblock<0)return;
				break;
			case ',':
				if(numblock>0)break;
				tok=tk_camma;
				goto endp;
			case ';':
			case 26:
				tok=tk_semicolon;
endp:
				CharToBackBuf(';');
				nextchar();
				inptr2=inptr;
				cha2=cha;
				return;
			case '/':	//отследить комментарии
				nextchar();
				if(cha=='*'){
					do{
						nextchar();
						if(cha=='*'){
							nextchar();
							if(cha=='/'){
								save=FALSE;
								break;
							}
						}
					}while(cha!=26);
				}
				else if(cha=='/'){
					do{
						nextchar();
					}while(cha!=13&&cha!=26);
				}
				else CharToBackBuf('/');
				break;
			case '"':
			case '\'':
				bcha=cha;
				do{
					CharToBackBuf(cha);
					nextchar();
					if(cha=='\\'){
						CharToBackBuf(cha);
						nextchar();
						CharToBackBuf(cha);
						nextchar();
					}
				}while(cha!=bcha);
				break;
		}
		if(save)CharToBackBuf(cha);
		nextchar();
	}
}

void RunBackText()
{
ITOK oitok,ostructadr;
SINFO ostr;
unsigned char *oldinput;
unsigned int oldinptr,oldendinptr;
unsigned char bcha;
int otok,otok2;
char *ostartline;
char *ostring,*obufrm;
COM_MOD *ocurmod;
	ostring=BackString((char *)string);
	oldinput=input;	//сохр некотор переменые
	oldinptr=inptr2;
	ostructadr=structadr;
	bcha=cha2;
	oldendinptr=endinptr;
	otok=tok;
	otok2=tok2;
	oitok=itok;
	ostr=strinf;
	strinf.bufstr=NULL;
	obufrm=bufrm;
	bufrm=NULL;

	ocurmod=cur_mod;
	cur_mod=NULL;

	input=(unsigned char *)BackTextBlock;
	inptr2=1;
	cha2=input[0];
	tok=tk_openbrace;
	SizeBackBuf=0;
	ostartline=startline;
	startline=(char*)input;
	endinptr=strlen((char *)input);
	endinput=startline+endinptr;

	doblock();

	endoffile=0;
	startline=ostartline;
	strinf=ostr;
	free(input);
	input=oldinput;
	inptr2=oldinptr;
	cha2=bcha;
	endinptr=oldendinptr;
	tok=otok;
	itok=oitok;
	if(bufrm)free(bufrm);
	bufrm=obufrm;
	tok2=otok2;
	strcpy((char *)string,ostring);
	if(strinf.bufstr)free(strinf.bufstr);
	structadr=ostructadr;
	free(ostring);

	cur_mod=ocurmod;
}

int PushLocInit(int ofs)
{
ITOK oitok,ostructadr,wtok;
SINFO ostr;
unsigned char *oldinput;
unsigned int oldinptr,oldendinptr;
unsigned char bcha;
int otok,otok2;
char *ostartline;
char *ostring,*obufrm;
int retcode=FALSE;
//	if(bufrm)puts(bufrm);
	ostring=BackString((char *)string);
	oldinput=input;	//сохр некотор переменые
	oldinptr=inptr2;
	ostructadr=structadr;
	bcha=cha2;
	oldendinptr=endinptr;
	otok=tok;
	otok2=tok2;
	oitok=itok;
	ostr=strinf;
	strinf.bufstr=NULL;
	obufrm=bufrm;
	bufrm=NULL;
	input=(unsigned char *)BackTextBlock;
	inptr2=ofs;
	ostartline=startline;
	cha2=input[inptr2++];
	endinptr=strlen((char *)input);
	startline=(char *)input+ofs;
	endinput=startline+endinptr;
	nexttok();
	wtok=itok;
	nexttok();
	nexttok();
	if((retcode=Push(&wtok))!=FALSE){
		retcode=TRUE;
		for(inptr2=ofs;;inptr2++){
			cha2=input[inptr2];
			input[inptr2]=' ';
			if(cha2==';'||cha2==0)break;
		}
	}
	endoffile=0;
	startline=ostartline;
	strinf=ostr;
	input=oldinput;
	inptr2=oldinptr;
	cha2=bcha;
	endinptr=oldendinptr;
	tok=otok;
	itok=oitok;
	if(bufrm)free(bufrm);
	bufrm=obufrm;
	tok2=otok2;
	strcpy((char *)string,ostring);
	if(strinf.bufstr)free(strinf.bufstr);
	structadr=ostructadr;
	free(ostring);
	return retcode;
}

int pushinit(int localline,unsigned int ofs)
{
int oline=linenumber;
COM_MOD *bcur_mod;
int retcode;
	linenum2=localline;
	bcur_mod=cur_mod;
	cur_mod=NULL;
	retcode=PushLocInit(ofs);
	cur_mod=bcur_mod;
	linenumber=linenum2=oline;
	return retcode;
}

int PossiblePush()
{
int retcode=FALSE;
	nexttok();
	switch(tok){
		case tk_dwordvar:
		case tk_longvar:
		case tk_reg32:
			if(am32==FALSE)break;
		case tk_postnumber:
		case tk_string:
		case tk_intvar:
		case tk_wordvar:
		case tk_reg:
			if(tok2==tk_semicolon||tok2==tk_camma)retcode=TRUE;
			break;
		case tk_undefofs:
			tok=tk_number;
			goto chnum;
		case tk_minus:
			if(tok2!=tk_number)break;
			nexttok();
		case tk_number:
chnum:
			doconstlongmath();
			if(tok==tk_semicolon||tok==tk_camma)retcode=TRUE;
			break;
	}

////   New 20.07.07 22:01
	if(bufrm){
		free(bufrm);
		bufrm=NULL;
	}
	if(strinf.bufstr){
		free(strinf.bufstr);
		strinf.bufstr=NULL;
	}
////////////////////////////

	return retcode;
}

#define MAXLOCVAR 128

void declarelocals(int mode,int finline) /* declare locals */
/*-----------------01.06.02 15:12-------------------
mode = 0 if local vars prior {
mode = 1 if local vars after {
--------------------------------------------------*/
{
unsigned int size,ssize,localtok;
int binptr;
int flag=0;
char bcha;
int slocaltok;
int numpointr,type;
unsigned int loop;
static int localline;
static int numinit;
static int maxlocvar;
static LILV *lilv=NULL;
static int headerinit;
	if(localsize==0)localline=0;
	if(lilv==NULL){
		numinit=0;
		lilv=(LILV *)MALLOC(sizeof(LILV)*MAXLOCVAR);
		maxlocvar=MAXLOCVAR;
		if(paramsize&&(!finline))headerinit=TRUE;
		else headerinit=FALSE;
	}
	dynamic_flag=0;
	do{
		while(tok==tk_semicolon)nexttok();
		if(tok==tk_static){
			flag|=f_static;
			nexttok();
		}
		size=0;
		switch(tok){
			case tk_union:
				loop=dounion(0,flag);
				goto locstruct;
			case tk_struct:
				loop=LocalStruct(flag,&localline);
locstruct:
				(lilv+numinit)->size=-loop;
				break;
			case tk_int: localtok=tk_intvar; size=2; break;
			case tk_word: localtok=tk_wordvar; size=2; break;
			case tk_char: localtok=tk_charvar; size=1; break;
			case tk_byte: localtok=tk_bytevar; size=1; break;
			case tk_long: localtok=tk_longvar; size=4; break;
			case tk_dword: localtok=tk_dwordvar; size=4; break;
			case tk_float: localtok=tk_floatvar; size=4; break;
			case tk_qword: localtok=tk_qwordvar; size=8; break;
			case tk_double: localtok=tk_doublevar; size=8; break;
			default:
				if((tok>=tk_bits&&tok<=tk_doublevar)||tok==tk_pointer||tok==tk_proc
						||tok==tk_declare||tok==tk_undefproc||tok==tk_structvar||
						(FindTeg(FALSE)==NULL&&FindTeg(TRUE)==NULL)){
					if(mode==0){
						(lilv+numinit)->size=0;
						datatype_expected();
						nexttok();
					}
					else{
						if(localsize){
							if(!finline){
								numpointr=0;
								if(SizeBackBuf){
									BackTextBlock[SizeBackBuf]=0;
									for(type=numinit-1,numpointr=0;type>=0;type--){//опр число иниц. переменных в конце
										if((lilv+type)->size<=0)break;
										numpointr+=(lilv+type)->size;
									}
									type++;	//число оставшихся переменных
									ssize=type;
								}
								size=localsize;
								if(lilv->size<=0&&optimizespeed==0&&chip>1){
									size-=numpointr;
									if(headerinit)outptr-=3;					/* remove PUSH BP and MOV BP,SP */
									op(0xC8); 						/* ENTER */
									outword(size); 	/* # of locals */
									op(0x00); 						/* level = 0 */
									flag=0;
									type=0;
									headerinit=TRUE;
									size=numpointr;
								}
								else{
									if(headerinit==FALSE){
										Enter();
										headerinit=TRUE;
									}
									flag=1;
								}
								if(SizeBackBuf){
									if(lilv->size>0){	//есть иниц переменные в начале
										for(loop=0;loop<numinit;loop++){
//										printf("size%d=%d %s\n",loop,(lilv+loop)->size,BackTextBlock);
											if((lilv+loop)->size<=0)break;
											if(pushinit(localline,(lilv+loop)->ofs)==FALSE)break;
											(lilv+loop)->rec->fuse|=INITVAR;
											size-=Align((lilv+loop)->size,(am32==FALSE?2:4));
											type--;
										}
									}
									if(size){
										binptr=size-numpointr;
										if(binptr){
											if(!optimizespeed&&type<3&&(type*(am32+1)*2)>=binptr)for(;type!=0;type--)op(0x50+ECX);
											else{
												if(binptr<128){
													outword(0xEC83);
													op(binptr);
												}
												else{
													outword(0xEC81);
													if(am32==FALSE)outword(binptr);
													else outdword((unsigned long)binptr);
												}
											}
										}
										if(numpointr){
											for(;ssize<numinit;ssize++){
//										printf("size%d=%d %s\n",loop,(lilv+loop)->size,BackTextBlock);
												if(pushinit(localline,(lilv+ssize)->ofs)==FALSE)break;
												(lilv+ssize)->rec->fuse|=INITVAR;
											}
										}
									}
								}
								else if(flag){
									if(localsize<128){
										outword(0xEC83);
										op(localsize);
									}
									else{
										outword(0xEC81);
										if(am32==FALSE)outword(localsize);
										else outdword((unsigned long)localsize);
									}
								}
							}
							else{	//finline
								if(SizeBackBuf!=0){
									free(BackTextBlock);
									SizeBackBuf=0;
								}
							}
						}
						if(psavereg->size){
							if(psavereg->all){
								op(0x60);
								addESP+=am32==FALSE?16:32;
							}
							else{
								for(int i=0;i<8;i++){
									if(psavereg->reg[i]){
										op66(psavereg->reg[i]);
										op(0x50+i);
										addESP+=am32==FALSE?2:4;

									}
								}
							}
							if(ESPloc&&am32&&(itok.type==tp_localvar||itok.type==tp_paramvar))itok.number+=addESP;
						}
//										printf("numinit=%d firstinit=%d\n%s\n",numinit,firstinit,BackTextBlock);
						if(SizeBackBuf!=0){
							CharToBackBuf('}');
							CharToBackBuf(0);
							int oline=linenumber;
							linenum2=localline;
							COM_MOD *bcur_mod;
							bcur_mod=cur_mod;
							cur_mod=NULL;
							RunBackText();
							cur_mod=bcur_mod;
							linenumber=linenum2=oline;
						}
						if(lilv){
							free(lilv);
							lilv=NULL;
						}
						return;
					}
				}
				else{
					loop=LocalStruct(flag,&localline);
					(lilv+numinit)->size=-loop;
				}
				break;
		}
		if(size!=0){
			do{
				binptr=inptr2;
				bcha=cha2;
				skipfind=TRUE;	//запретить искать в глобальном дереве
				nexttok();
				if(tok==tk_static){
					flag|=f_static;
					binptr=inptr2;
					bcha=cha2;
					nexttok();
				}
				slocaltok=localtok;
				ssize=size;
				numpointr=0;
				type=tp_localvar;
				while(tok==tk_mult){
					binptr=inptr2;
					bcha=cha2;
					nexttok();
					numpointr++;
				}
				if(tok!=tk_ID&&tok!=tk_id)idalreadydefined();
				else{
					long numcopyvar;
					numcopyvar=1;
					localrec *lrec=addlocalvar(itok.name,slocaltok,(flag&f_static)==0?localsize:postsize);
					loop=ssize;
					if(numpointr)loop=am32==TRUE?4:2;
					skipfind=FALSE;	//разрешить поиск в глобальном дереве
					if(tok2==tk_openblock){//[
						nexttok();
						nexttok();
						CheckMinusNum();
						if(tok!=tk_number){
							numexpected();
							nexttok();
						}
						else numcopyvar=doconstlongmath();
						loop=numcopyvar*ssize;
						if(tok!=tk_closeblock)expected(']');
					}
					lrec->rec.recsize=loop;
					if(!mode){
						nexttok();
					}
					if(flag&f_static){
						if(tok==tk_assign||(mode&&tok2==tk_assign)){
							if(mode)nexttok();
							if(numpointr){
								type=tk_char+slocaltok-tk_charvar;
								slocaltok=tk_pointer;
							}
							lrec->rec.rectok=slocaltok;
							lrec->rec.recnumber=0;
							lrec->rec.recpost=DYNAMIC_VAR;
							lrec->rec.line=linenumber;
							lrec->rec.file=currentfileinfo;
							lrec->rec.count=0;
//							lptr->rec.type=(unsigned short)type;
							lrec->rec.npointr=(unsigned short)numpointr;
							lrec->rec.sbuf=dynamic_var();
							lrec->rec.recsib=type;
							lrec->rec.type=tp_gvar;

//							if(strcmp(lrec->rec.recid,"nnil")==0)crec=&lrec->rec;

						}
						else{
							lrec->rec.type=tp_postvar;
							AddPostData(loop);
							if(mode)nexttok();
						}
					}
					else{
						lrec->rec.type=tp_localvar;
						lrec->rec.npointr=(unsigned short)numpointr;
						localsize+=loop;
						if(mode)nexttok();
						(lilv+numinit)->size=-loop;
						if(tok==tk_assign){
							if(localline==0)localline=linenumber;
							if(numcopyvar!=1){
								int i=binptr;
								while(input[i]!='[')i++;
								i++;
								input[i]='0';
								i++;
								for ( int j=1;j!=0;i++){
									switch(input[i]){
										case '[':
											j++;
											break;
										case ']':
											j--;
											break;
									}
									if(j!=0)input[i]= ' ';
								}
							}
							else{
								switch(slocaltok){
									case tk_floatvar:
									case tk_doublevar:
									case tk_qwordvar:
//										(lilv+numinit)->ofs=-1;
										break;
									default:
										if(PossiblePush()){
											(lilv+numinit)->size=loop;
											(lilv+numinit)->rec=lrec;
										}
										break;
								}
							}
							(lilv+numinit)->ofs=SizeBackBuf;
							AddBackBuf(binptr,bcha);
//							if(bufrm)puts(bufrm);
						}
						numinit++;
						if(numinit==maxlocvar){
							maxlocvar+=MAXLOCVAR;
							lilv=(LILV *)REALLOC(lilv,maxlocvar*sizeof(LILV));
						}
						lrec->rec.recnumber=-lrec->rec.recnumber-Align(loop,(am32==FALSE?2:4));
					}
				}
				if(localsize)localsize=Align(localsize,(am32==FALSE?2:4));
			}while(tok==tk_camma);
			seminext();
		}
		else{
			numinit++;
			if(numinit==maxlocvar){
				maxlocvar+=MAXLOCVAR;
				lilv=(LILV *)REALLOC(lilv,maxlocvar*sizeof(LILV));
			}
		}
		flag=0;
//		printf("tok=%d %s\n",tok,BackTextBlock);
	}while(tok!=tk_openbrace&&tok!=tk_eof);
}

int CheckDeclareProc()
{
unsigned int i=inptr2-1;
	while(input[i]!='('){
		i++;
		if(i>=endinptr){
			unexpectedeof();
			break;
		}
	}
lab1:
	i++;
	for(int j=1;j!=0;i++){
		char c=input[i];
		if(c=='(')j++;
		else if(c==')')j--;
		if(i>=endinptr){
			unexpectedeof();
			break;
		}
	}
	for(;;i++){
		if(input[i]=='(')goto lab1;
		if(input[i]>' ')break;
		if(i>=endinptr){
			unexpectedeof();
			break;
		}
	}

	for(;;i++){
		if(input[i]==';'||input[i]==',')return TRUE;	//объявление процедуры
		if(input[i]>' ')break;
		if(i>=endinptr){
			unexpectedeof();
			break;
		}
	}
	return FALSE;
}

void IsUses(idrec *rec)
{
int i;
	if(tok==tk_openbracket&&stricmp(itok2.name,"uses")==0){
		nexttok();
		i=0;
		nexttok();
		while(tok==tk_reg32||tok==tk_reg||tok==tk_beg){
			if(tok==tk_beg&&itok.number>3)itok.number-=4;
			i=i|(1<<itok.number);
			nexttok();
			if(tok==tk_camma)nexttok();
		}
		if(strcmp(itok.name,"allregs")==0){
			i=256;
			nexttok();
		}
		rec->recpost=0xFF^i;
		expecting(tk_closebracket);
	}
}

void declare_procedure(int oflag,int orm,int npointr)
{
int i,next=TRUE,j;
char pname[IDLENGTH];
idrec *rec;
	strcpy(pname,itok.name);
	param[0]=0;
	nexttok();
	if(npointr)expecting(tk_closebracket);
	expecting(tk_openbracket);
	if((oflag&f_typeproc)==tp_fastcall)declareparamreg();
	else declareparamstack();
	if(tok2==tk_assign)nexttok();
	itok.post=1;
	if(npointr){
		itok.segm=DS;
		itok.type=tk_proc;
		itok.sib=am32==FALSE?rm_d16:rm_d32;
		i=2;
		if(am32||(oflag&f_far))i=4;
		if(tok==tk_assign||(notpost==TRUE&&dynamic_flag==0)){	//= инициализированая переменная
			if((oflag&f_extern))preerror("extern variable do not initialize at declare");
			if(alignword&&(!dynamic_flag))alignersize+=AlignCD(DS,i);
			FindOff((unsigned char *)pname,DS);
			itok.number=outptrdata;
			if(tok!=tk_assign){
				if(dbg&2)AddDataLine(i);
				outword(0);
				if(i==4)outword(0);
			}
			else{
				ITOK oitok;
				oitok=itok;
				initglobalvar(tk_word,1,i,variable);
				itok=oitok;
				next=FALSE;
			}
			itok.post=0;
			datasize+=i;
		}
		else{
			if((oflag&f_extern)==0){
				itok.number=postsize;
				AddPostData(i);
			}
			else itok.number=externnum++;
		}
		j=tok;
		tok=tk_pointer;
	}
	else{
		tok=tk_declare;
		itok.type=tp_declare;
		itok.number=(oflag&f_extern)!=0?externnum++:secondcallnum++;
		itok.segm=NOT_DYNAMIC;
		itok.post=0;
	}
	itok.npointr=(unsigned short)npointr;
	itok.rm=orm;
	itok.flag=oflag;
	if(itok.rm==tokens)itok.rm=(am32==0?tk_word:tk_dword);
//	printf("rm=%d %s\n",itok.rm,itok.name);
	strcpy((char *)string,param);
	rec=addtotree(pname);
	if(next)nexttok();
	else tok=j;
	IsUses(rec);
}

void define_procedure()
{
	if(dynamic_flag)dynamic_proc();
	else{
		itok.segm=NOT_DYNAMIC;
		if(AlignProc!=FALSE)AlignCD(CS,alignproc);
		procedure_start=outptr;
		if(dbg)AddLine();
//		itok.flag&=~f_static; 26.08.05 00:09
		setproc((tok==tk_id||tok==tk_ID)?0:1);
		dopoststrings();
	}
}

void interruptproc()
{
char *bstring;
ITOK otok;
	inlineflag=0;
	procedure_start=outptr;
	returntype=tk_void;
	current_proc_type=f_interrupt;
	localsize=paramsize=0;
	nexttok();
//	FindOff((unsigned char *)itok.name,CS);
	itok.number=outptr;
	itok.segm=NOT_DYNAMIC;
	itok.post=0;
	itok.rm=tk_void;
	itok.flag=0;
	if(tok==tk_ID||tok==tk_id){
		tok=tk_interruptproc;
		itok.type=tp_ucnovn;
		addtotree((char *)string);
	}
	else if(tok==tk_undefproc){
		tok=tk_interruptproc;
		long hold=updatetree();
		updatecall((unsigned int)hold,(unsigned int)outptr,0);
	}
	else idalreadydefined();
	itok.rec->count=FindOff((unsigned char *)itok.name,CS);
	bstring=BackString((char *)string);
	otok=itok;
	nextexpecting2(tk_openbracket);
	expecting(tk_closebracket);
	if(tok==tk_inline){
		inlineflag=1;
		nexttok();
	}
	else if(tok!=tk_openbrace)declarelocals(0);
	expecting(tk_openbrace);
	declarelocals(1);
	startblock();
	doblock2();							/* do proc */
	endblock();
	if(retproc==FALSE){
		if(inlineflag==0)leaveproc();
		else if(localsize>0)Leave();
	}
	initBP=0;
	strcpy((char *)string,bstring);
	itok=otok;
	tok=tk_interruptproc;
	itok.size=outptr-procedure_start;
	updatetree();
	free(bstring);
	killlocals();
	nexttok();
	dopoststrings();
}

int skipstring(int pos,unsigned char term)
{
unsigned char c;
	do{
		pos++;
		c=input[pos];
		if(c==0x5C){
			pos++;
			continue;
//			c=input[pos+1];
		}
	}while(c!=term);
	return pos;
}

int skipcomment(int pos)
{
	if(input[pos+1]=='*'){
		pos+=2;
		for(;;pos++){
			if(input[pos]==13){
				linenumber++;
			}
			else if(input[pos]=='*'){
				pos++;
				if(input[pos]=='/')break;
			}
			if((unsigned int)pos>=endinptr){
				unexpectedeof();
				break;
			}
		}
	}
	else if(input[pos+1]=='/'){
		pos+=2;
		for(;;pos++){
			if(input[pos]==13){
				linenumber++;
				break;
			}
			if(input[pos]==10)break;
		}
	}
	return pos;
}

int swapparam()
{
int i,ns,j,lastofs;
int linep,linebak;
char c,ochar;
unsigned char *bufpar;
int numpar=0;
unsigned char *oldinput;
paraminfo *pi;
	if(tok!=tk_openbracket){
		expected('(');
		do{
			nexttok();
		}while(tok2!=tk_semicolon&&tok!=tk_eof);
		return 0;
	}
	pi=(paraminfo *)MALLOC(128*sizeof(paraminfo));
	do{
		inptr2--;
	}while(input[inptr2]!='(');
	inptr2++;
	pi->ofspar=inptr2;
	pi->type[0]=0;
	linep=linenumber;
	for(i=inptr2,ns=1;ns>0;i++){	//поиск конца параметров
		switch(input[i]){
			case '(': ns++; break;
			case ')': ns--; break;
			case ',':
				if(ns==1){
					if(numpar==127)preerror("To many parametrs in function");
					numpar++;
					(pi+numpar)->ofspar=i+1;
					(pi+numpar)->type[0]=0;
				}
				break;
			case '/':
				i=skipcomment(i);
				break;
			case '"':
			case 0x27:
				i=skipstring(i,input[i]);
				break;
			case 13:
				linenumber++;
				break;
		}
		if((unsigned int)i>=endinptr){
			unexpectedeof();
			break;
		}
	}
	for(j=0,ns=0;param[j]!=0;j++,ns++){//перевернуть задекларируемые параметры
		lastofs=0;
		ochar=c=param[j];
		(pi+ns)->type[0]=c;
		while(c=='*'){
			j++;
			ochar=c=param[j];
			(pi+ns)->type[++lastofs]=c;
		}
		if(c=='A'){
			if(ns){
				while(ns<=numpar){
					(pi+ns)->type[0]='U';
					(pi+ns)->type[1]=0;
					ns++;
				}
				break;
			}
		}
		if(ochar=='T'){
			do{
				j++;
				lastofs++;
				c=param[j];
				(pi+ns)->type[lastofs]=c;
			}while(isdigit(c));
			(pi+ns)->type[lastofs]=0;
			j--;
			continue;
		}
		c=param[j+1];
		if(c>='0'&&c<='7'){
			(pi+ns)->type[1]=c;
			j++;
			lastofs++;
			if(ochar=='Q'){
				(pi+ns)->type[2]=param[j+1];
				j++;
				lastofs++;
			}
		}
		(pi+ns)->type[++lastofs]=0;
	}
	ns--;
	for(j=0;ns>=0;ns--){
		lastofs=0;
		c=(pi+ns)->type[lastofs++];
		while(c!=0){
			param[j++]=c;
			c=(pi+ns)->type[lastofs++];
		}
	}
	param[j]=0;
//	puts(param);
//	if(crec)printf("start0 swapparams num=%08X\n",crec->recnumber);
	bufpar=(unsigned char *)MALLOC(i-inptr2+2);
//	printf("crec=%08X size=%d bufpar=%08X size=%d\n",crec,sizeof(idrec),bufpar,i-inptr2+2);
	inptr2=i;
	ochar=input[inptr2];
	inptr2++;
	i--;
	lastofs=0;
	for(;;){
		j=(pi+numpar)->ofspar;//[numpar];
		for(ns=0;(j+ns)!=i;ns++){
			bufpar[lastofs++]=input[j+ns];
		}
		i=j-1;
		numpar--;
		if(numpar<0)break;
		bufpar[lastofs++]=',';
	}
	bufpar[lastofs++]=')';
	*(short *)&bufpar[lastofs++]=';';
	free(pi);
	oldinput=input;	//сохр некотор переменые
//	puts((char *)(input+inptr));
//	printf("cur_mod=%08X input=%08X\n",cur_mod,input);
	ns=inptr2;
	j=endinptr;
	input=bufpar;
//	puts((char *)bufpar);
	inptr2=1;
	cha2=input[0];
	endinptr=lastofs;
	tok=tk_openbracket;
	if(bufrm!=NULL){
		free(bufrm);
		bufrm=NULL;
	}
	if(strinf.bufstr!=NULL){
		free(strinf.bufstr);
		strinf.bufstr=NULL;
	}
	linebak=linenumber;
	linenumber=linep;
	i=doparams();
	endoffile=0;
	input=oldinput;
	inptr2=ns;
	cha2=ochar;
	endinptr=j;
	linenumber=linebak;
//	printf("cur_mod=%08X input=%08X\n",cur_mod,input);
	free(bufpar);
//	puts((char *)input);
	return i;
}

int getrazr(int type)
{
	switch(type){
		case tk_char:
		case tk_byte:
			return r8;
		case tk_word:
		case tk_int:
			return r16;
		case tk_long:
		case tk_dword:
		case tk_float:
			return r32;
		case tk_qword:
		case tk_double:
			return r64;
	}
	if(am32)return r32;
	return r16;
}

int doparams() 		 /* do stack procedure parameter pushing */
{
char done=0,next;
int vartype;
int stackpar=0;
int i;
int jj=0;
char *bparam;	//буфер для декларируемых параметров
int ip=-1;	//номер параметра
char *ofsstr=NULL;
int useAX=FALSE;
int retreg=AX;
unsigned char oaddstack=addstack;
unsigned char oinline=useinline;
	useinline=0;
int structsize;
struct idrec *ptrs;
	addstack=FALSE;
	if(am32!=FALSE)jj=2;
	bparam=BackString(param);
	if(param[0]!=0)ip=0;
	ofsstr=GetLecsem(tk_camma,tk_closebracket);
	expectingoperand(tk_openbracket);
	ClearRegister();
	if(tok!=tk_closebracket){
		while(tok!=tk_eof&&done==0){
			useAX=FALSE;
			retreg=AX;
			i=0;
			next=1;
			if(ip!=-1){
				if(bparam[ip]=='*'){
					while(bparam[ip]=='*')ip++;
					ip++;
					vartype=(am32==FALSE?tk_word:tk_dword);
				}
				else{
					if((vartype=GetTypeParam(bparam[ip++]))==0)ip=-1;
					else{
						int c=bparam[ip];
						if(vartype==tk_struct){
							structsize=0;
							while(isdigit(c)){
								c-='0';
								structsize=structsize*10+c;
								ip++;
								c=param[ip];
							}
						}
						else if(c>='0'&&c<='7'){
							ip++;
							c-=0x30;
							if(vartype==tk_fpust)float2stack(c);
							else{
								if(vartype==tk_qword){
									c|=(bparam[ip]-0x30)*256;
									ip++;
								}
								CalcRegPar(c,vartype,&ofsstr);
							}
							goto endparam;
						}
					}
				}
				if(vartype==tk_multipoint){
					vartype=tokens;
					ip--;
				}
			}
			if(tok==tk_string){
				if(chip<2||(optimizespeed&&(chip==5||chip==6))){
					op(0xB8);
					if(am32!=FALSE)outdword(addpoststring());
					else outword(addpoststring());  // MOV AX,imm16
					op(0x50);
					useAX=TRUE;
					ClearReg(EAX);	//надо добавить оптимизацию регистров а пока так
				}			/* PUSH AX */
				else{
					op(0x68);  /* PUSH imm16 */
					if(am32!=FALSE)outdword(addpoststring());
					else outword(addpoststring());
					if(cpu<2)cpu=2;
				}
				stackpar+=2+jj;
				addESP+=2+jj;
				nexttok();
			}
			else{
				if(tok>=tk_char&&tok<=tk_double){
					vartype=tok;
					getoperand();
				}
				else if(tok==tk_openbracket){
					nexttok();
					if(tok>=tk_char&&tok<=tk_double)vartype=tok;
					nexttok();
					expectingoperand(tk_closebracket);
				}
				else{
					if(ip==-1||vartype==tokens){
						switch(tok){
							case tk_longvar: vartype=tk_long; break;
						case tk_floatvar: vartype=tk_float; break;
							case tk_dwordvar: vartype=tk_dword; break;
							case tk_doublevar: vartype=tk_double; break;
							case tk_qwordvar: vartype=tk_qword; break;
							case tk_number:
								vartype=(itok.rm==tk_float?tk_float:(am32==FALSE?tk_word:tk_dword));
								break;
							case tk_reg32: vartype=tk_dword; break;
							case tk_reg64: vartype=tk_qword; break;
							case tk_minus:
								if(tok2==tk_number){
									vartype=itok2.rm==tk_float?tk_float:(am32==FALSE?tk_word:tk_dword);
									break;
								}
							default: vartype=(am32==FALSE?tk_word:tk_dword); break;
						}
					}
				}
				if(tok==tk_minus&&tok2==tk_number&&vartype!=tk_float){	//проверка отрицательного числа
					nexttok();
					itok.lnumber=-itok.lnumber;
				}
				int razr;
				if(ofsstr){
					razr=getrazr(vartype);
					int retr;
					if((retr=CheckIDZReg(ofsstr,AX,razr))!=NOINREG){
//						printf("reg=%d\n",retr);
						GetEndLex(tk_camma,tk_closebracket);
						if(razr==r8)razr=am32==0?r16:r32;
						if(retr==SKIPREG)retreg=AX;
						else retreg=retr;
						op66(razr);
						op(0x50+retreg);
						useAX=TRUE;
						stackpar+=razr;
						addESP+=razr;
						goto endparam1;
					}
//						printf("reg=%d\n",retr);
				}
				razr=r32;
				if(itok2.type==tp_opperand||tok==tk_minus){	//составное
					switch(vartype){
						case tk_struct:
							i=structsize/((am32+1)*2);
							do_e_axmath(0,r32,&ofsstr);
							ClearReg(AX);
							if(am32){
								i--;
								itok.number=structsize;
								for(;i>0;i--){
									itok.number-=4;
									outword(0x70FF);
									op(itok.number);
								}
								outword(0x30FF);
							}
							else{
								ClearReg(BX);
								warningreg(regs[1][BX]);
								outword(0xC389);
								i--;
								itok.number=structsize;
								for(;i>0;i--){
									itok.number-=2;
									outword(0x77FF);
									op(itok.number);
								}
								outword(0x37FF);
							}
							stackpar+=structsize;
							addESP+=structsize;
							break;
						case tk_char: i=1;
						case tk_byte:
							stackpar+=jj;
							doalmath((char)i,&ofsstr);
							addESP+=2+jj;
							break;
						case tk_int: i=1;
						case tk_word:
							if(am32==FALSE)razr=r16;
							stackpar+=jj;
							goto blokl;
						case tk_long: i=1;
						case tk_dword:
							if(cpu<3)cpu=3;
							stackpar+=2;
blokl:
							if(chip>=2&&(!(optimizespeed&&(chip==5||chip==6)))&&(tok==tk_number
									||tok==tk_undefofs||tok==tk_postnumber)){
								int otok=tok;
								ITOK oitok=itok;
								tok=tk_number;
								if(OnlyNumber(i)){
									op66(razr);
									if(otok==tk_number&&(postnumflag&f_reloc)==0&&short_ok(itok.number,razr==r16?FALSE:TRUE)!=0){
										op(0x6A);	// PUSH 8 extend to 32 bit
										op((unsigned int)itok.number);
									}
									else{
										op(0x68);	// PUSH const
										if(otok==tk_undefofs)AddUndefOff(0,oitok.name);
										else if(otok==tk_postnumber)(oitok.flag&f_extern)==0?setwordpost(&oitok):setwordext(&oitok.number);
										else if((postnumflag&f_reloc)!=0)AddReloc();
										if(razr==r16)outword(itok.number);
										else outdword(itok.number);
									}
									addESP+=razr==r16?2:4;
									goto nopush;
								}
								tok=otok;
							}
							do_e_axmath(i,razr,&ofsstr);
							addESP+=razr==r16?2:4;
							op66(razr);
							break;
						case tk_double:
							if(doeaxfloatmath(tk_stackstart,0,4)!=tk_stackstart){
								op66(r32);
								op(0x50+EDX);	// PUSH EDX
								op66(r32);
								op(0x50);	//push eax
								useAX=TRUE;
							}
							if(cpu<3)cpu=3;
							stackpar+=6;
							addESP+=8;
							goto nopush;
						case tk_float:
							if(doeaxfloatmath(tk_stackstart)!=tk_stackstart){
								op66(r32);
								if(!am32){
									op(0x89);	//	mov ssdword[bp+2]=eax
									op(0x46);
									op(2);
									Leave();
								}
								else{
									op(0x50);	//push eax
									useAX=TRUE;
								}
							}
							if(cpu<3)cpu=3;
							stackpar+=2;
							addESP+=4;
							goto nopush;
						case tk_qwordvar:
							i=EAX|(EDX*256);
							getintoreg64(i);
							doregmath64(i);
							op66(r32);
							op(0x50+EDX);	// PUSH EDX
							op66(r32);
							next=0;
							if(cpu<3)cpu=3;
							stackpar+=6;
							addESP+=8;
							break;
						default: goto deflt;
					}
					op(0x50);/* PUSH AX or EAX */
					useAX=TRUE;
nopush:
					stackpar+=2;
				}
				else{	//одиночное
//					next=1;
//					printf("vartype=%d\n",vartype);
					if(vartype==tk_struct){
//						printf("tok=%d %s\n",tok,itok.name);
						i=structsize/((am32+1)*2);
						switch(tok){
							case tk_structvar:
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
									if(i>1&&am32){
										outseg(&itok,1);
										op(0xB8);
										outaddress(&itok);
										ClearReg(AX);
										i--;
										itok.number=structsize;
										for(;i>0;i--){
											itok.number-=4;
											outword(0x70FF);
											op(itok.number);
										}
										outword(0x30FF);
										break;
									}
								}
								itok.sib=(am32==FALSE?CODE16:CODE32);
								itok.flag=ptrs->flag;
								itok.number+=structsize;
								for(;i>0;i--){
									itok.number-=(am32+1)*2;
									outseg(&itok,2);
									op(0xFF);	// PUSH [dword]
									op(0x30+itok.rm);
									outaddress(&itok);
								}
								break;
							case tk_rmnumber:
								itok.number+=structsize;
								for(;i>0;i--){
									itok.number-=(am32+1)*2;
									outseg(&itok,2);
									op(0xFF);	// PUSH [dword]
									op(0x30+itok.rm);
									outaddress(&itok);
								}
								break;
							case tk_undefofs:
								itok.rm=(am32==FALSE?rm_d16:rm_d32);	//установки по умолчанию
							case tk_postnumber:
								if(i>1&&am32){
									if(tok!=tk_undefofs)outseg(&itok,1);
									op(0xB8);
									if(tok==tk_undefofs){
										AddUndefOff(0,(char *)string);
										outdword(itok.number);
									}
									else outaddress(&itok);
									ClearReg(AX);
									i--;
									itok.number=structsize;
									for(;i>0;i--){
										itok.number-=4;
										outword(0x70FF);
										op(itok.number);
									}
									outword(0x30FF);
									break;
								}
								itok.number+=structsize;
								for(;i>0;i--){
									itok.number-=(am32+1)*2;
									if(tok!=tk_undefofs)outseg(&itok,2);
									op(0xFF);	// PUSH [dword]
									op(0x30+itok.rm);
									if(tok==tk_undefofs){
										AddUndefOff(0,(char *)string);
										outdword(itok.number);
									}
									else outaddress(&itok);
								}
								break;
							case tk_reg32:
								i--;
								itok.rm=structsize;
								for(;i>0;i--){
									itok.rm-=4;
									op(0xFF);
									op(0x70+itok.number);
									op(itok.rm);
								}
								op(0xFF);
								op(0x30+itok.number);
								break;
							default:
//								preerror("for parametr function required structure");
								do_e_axmath(0,r32,&ofsstr);
								ClearReg(AX);
								if(am32){
									i--;
									itok.number=structsize;
									for(;i>0;i--){
										itok.number-=4;
										outword(0x70FF);
										op(itok.number);
									}
									outword(0x30FF);
								}
								else{
									ClearReg(BX);
									warningreg(regs[1][BX]);
									outword(0xC389);
									i--;
									itok.number=structsize;
									for(;i>0;i--){
										itok.number-=2;
										outword(0x77FF);
										op(itok.number);
									}
									outword(0x37FF);
								}
								break;
						}
						stackpar+=structsize;
						addESP+=structsize;
					}
					else if(vartype<tk_long){
#ifdef OPTVARCONST
						CheckConstVar3(&tok,&itok,razr);
#endif
						switch(tok){
							case tk_reg:
								op(0x50+(unsigned int)itok.number);
								break;
							case tk_beg:
								if(vartype==tk_int||vartype==tk_word)goto deflt;
								if(itok.number<AH){
									op(0x50+(unsigned int)itok.number);
									break;
								}
								op(0x88);
								op(0xC0+(unsigned int)itok.number*8);
								op(0x50);
								useAX=TRUE;
								break;
							case tk_seg:
								if(am32)goto deflt;
								if((unsigned int)itok.number<FS)op(0x06+(unsigned int)itok.number*8);
								else{
									op(0xF); op(0x80+(unsigned int)itok.number*8);
									if((unsigned int)itok.number<=GS)if(cpu<3)cpu=3;
								}
								break;
							case tk_longvar:
							case tk_dwordvar:
//								if(am32==FALSE)goto deflt;
								i=2;
							case tk_intvar:
							case tk_wordvar:
								i+=2;
								CheckAllMassiv(bufrm,i,&strinf);
								outseg(&itok,2);
								op(0xFF); 			/* PUSH [word] */
								op(0x30+itok.rm);
								outaddress(&itok);
								break;
							case tk_number:
								if(chip>=2/*&&(!(optimizespeed&&(chip==5||chip==6)))*/){	//28.03.07 19:04
									if((itok.flag&f_reloc)==0&&short_ok(itok.number)){
										op(0x6A);	/* PUSH const */
										op((unsigned int)itok.number);
									}
									else{
										op(0x68);	 /* PUSH const */
										if((itok.flag&f_reloc)!=0)AddReloc();
										if(am32==FALSE)outword((unsigned int)itok.number);
										else outdword(itok.number);
									}
									if(cpu<2)cpu=2;
								}
								else{
									MovRegNum(r16,itok.flag&f_reloc,itok.number,EAX);
									op(0x50);
								}
								break;
							case tk_apioffset:
								op(0x68);	// PUSH const
								AddApiToPost(itok.number);
								break;
							case tk_postnumber:
							case tk_undefofs:
								if(chip>=2&&(!(optimizespeed&&(chip==5||chip==6)))){
									op(0x68);	// PUSH const
									if(tok==tk_undefofs)AddUndefOff(0,(char *)string);
									else (itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
									if(am32==FALSE)outword((unsigned int)itok.number);
									else outdword(itok.number);
									break;
								}
							default:
deflt:
//if(tok==tk_new)puts("monovar default");
								switch(vartype){
									case tk_int: i=1;
									case tk_word: do_e_axmath(i,r16,&ofsstr); break;
									case tk_char: i=1;
									case tk_byte: doalmath((char)i,&ofsstr); break;
									default: beep(); break;
								}
								op(0x50); 	/* PUSH AX */
								useAX=TRUE;
								next=0;
								break;
						}
						stackpar+=2+jj;
						addESP+=2+jj;
					}
					else if(vartype<tk_qword){
#ifdef OPTVARCONST
						CheckConstVar3(&tok,&itok,razr);
#endif
//						printf("tok=%d %s\n",tok,itok.name);
						switch(tok){				// long or dword or float
							case tk_reg32:
								op66(r32);
								op(0x50+(unsigned int)itok.number);
								break;
							case tk_floatvar:
								if(vartype==tk_float)goto pushmem;
							case tk_qwordvar:
							case tk_dwordvar:
							case tk_longvar:
								if(optimizespeed&&(chip==5||chip==6))goto def;
pushmem:
								CheckAllMassiv(bufrm,4,&strinf);
								op66(r32);
								outseg(&itok,2);
								op(0xFF);	// PUSH [dword]
								op(0x30+itok.rm);
//								printf("push rm=%08X sib=%08X post=%u num=%08X rec=%08X flag=%08X %s\n",itok.rm,itok.sib,itok.post,itok.number,itok.rec,itok.flag,itok.name);
								outaddress(&itok);
								break;
							case tk_number:
								if(optimizespeed&&(chip==5||chip==6))goto def;
								if(vartype==tk_float){
									itok.number=doconstfloatmath();
									next=0;
								}
								else if(itok.rm==tk_double)itok.fnumber=itok.dnumber;
								op66(r32);
								if((itok.flag&f_reloc)==0&&short_ok(itok.number,TRUE)!=0){
									op(0x6A);	// PUSH 8 extend to 32 bit
									op((unsigned int)itok.number);
								}
								else{
									op(0x68);	// PUSH const
									if((itok.flag&f_reloc)!=0)AddReloc();
									outdword(itok.number);
								}
								break;
							case tk_apioffset:
								op66(r32);
								op(0x68);	// PUSH const
								AddApiToPost(itok.number);
								break;
							case tk_postnumber:
							case tk_undefofs:
								if(optimizespeed&&(chip==5||chip==6))goto def;
								op66(r32);
								op(0x68);	// PUSH const
								if(tok==tk_undefofs)AddUndefOff(0,(char *)string);
								else (itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
								outdword(itok.number);
								break;
							default:
def:
								if(vartype==tk_float)doeaxfloatmath(tk_stackstart);
								else{
									if(vartype==tk_long)i=1;
//									printf("tok=%d rm=%08X %s\n",tok,itok.rm,itok.name);
									if(tok==tk_rmnumber&&am32&&bufrm==NULL&&strinf.bufstr==NULL&&
											((itok.rm&7)!=4)&&((itok.rm&7)!=5)&&
											((itok.rm&rm_mod11)==rm_mod00)&&itok.number==0){
										op66(r32);
										op(0x50+(itok.rm&7));	// PUSH rm reg
										break;

									}
									do_e_axmath(i,r32,&ofsstr);
									op66(r32);
									op(0x50);	// PUSH EAX
									useAX=TRUE;
								}
								next=0;
								break;
						}
						stackpar+=4;
						addESP+=4;
						if(cpu<3)cpu=3;
					}
					else{	//qword or double
#ifdef OPTVARCONST
						CheckConstVar3(&tok,&itok,razr);
#endif
						switch(tok){
							case tk_reg64:
								op66(r32);
								op(0x50+(unsigned int)itok.number/256);
								op66(r32);
								op(0x50+(unsigned int)itok.number&255);
								break;
							case tk_doublevar:
							case tk_qwordvar:
								itok.number+=4;
								compressoffset(&itok);
								CheckAllMassiv(bufrm,8,&strinf);
								for(i=0;i<2;i++){
									op66(r32);
									outseg(&itok,2);
									op(0xFF);	// PUSH [dword]
									op(0x30+itok.rm);
									outaddress(&itok);
									if(i==1)break;
									itok.number-=4;
									compressoffset(&itok);
								}
								break;
							case tk_reg32:
								op66(r32);
								outword(0x6A);	// PUSH 8 extend to 32 bit
								op66(r32);
								op(0x50+(unsigned int)itok.number);
								break;
							case tk_number:
								long long lnumber;
								lnumber=itok.lnumber;
								itok.lnumber=lnumber>>32;
								for(i=0;i<2;i++){
									op66(r32);
									if((i==0||(itok.flag&f_reloc)==0)&&short_ok(itok.number,TRUE)!=0){
										op(0x6A);	// PUSH 8 extend to 32 bit
										op((unsigned int)itok.number);
									}
									else{
										op(0x68);	// PUSH const
										if(i==1&&(itok.flag&f_reloc)!=0)AddReloc();
										outdword(itok.number);
									}
									if(i==1)break;
									itok.number=lnumber;
								}
								break;
							case tk_postnumber:
							case tk_undefofs:
								op66(r32);
								outword(0x6A);	// PUSH 8 extend to 32 bit
								op66(r32);
								op(0x68);	// PUSH const
								if(tok==tk_undefofs)AddUndefOff(0,(char *)string);
								else (itok.flag&f_extern)==0?setwordpost(&itok):setwordext(&itok.number);
								outdword(itok.number);
								break;
							default:
								if(vartype==tk_double)doeaxfloatmath(tk_stackstart,0,4);
								else{
									op66(r32);
									outword(0x6A);
									do_e_axmath(0,r32,&ofsstr);
									op66(r32);
									op(0x50);	// PUSH EAX
									useAX=TRUE;
								}
								next=0;
								break;
						}
						stackpar+=8;
						addESP+=8;
						if(cpu<3)cpu=3;
					}
endparam1:
					if(next)nexttok();

				}
			}
endparam:
			if(ofsstr){
				if(useAX)IDZToReg(ofsstr,retreg,getrazr(vartype));
				free(ofsstr);
				ofsstr=NULL;
			}
			if(tok==tk_camma){
				ofsstr=GetLecsem(tk_camma,tk_closebracket);
				getoperand();
			}
			else if(tok==tk_closebracket)done=1;
			else{
				expected(')');
				done=1;
			}
		}
	}
	if(ofsstr){
		if(useAX)IDZToReg(ofsstr,retreg,getrazr(vartype));
		free(ofsstr);
	}
	if(ip!=-1&&bparam[ip]!=0&&bparam[ip]!='A'&&bparam[ip]!='V')missingpar();
	free(bparam);
	setzeroflag=FALSE;
	addstack=oaddstack;
	useinline=oinline;
	return stackpar;
}

void CheckDir()
{
	do{
		switch(itok.number){
			case d_ifdef:
			case d_ifndef:
			case d_endif:
			case d_else:
			case d_if:
			case d_elif:
				inptr2=inptr;
				cha2=cha;
				linenum2=linenumber;
				directive();
				inptr=inptr2;
				cha=cha2;
				break;
			default:
				FastTok(0);
				break;
		}
		if(tok==tk_eof)break;
	}while(tok==tk_question);
}

char *dynamic_var()
{
int start,size;
char *bstring;
int use_extract=FALSE;

#define TBUFSIZE 2048
#define MTBUFSIZE TBUFSIZE-IDLENGTH
int curbsize;
COM_MOD *ocur_mod;
	bstring=(char *)MALLOC(TBUFSIZE);
	curbsize=TBUFSIZE;
	size=0;
	if(tok2==tk_extract)use_extract=TRUE;
	do{
		start=inptr2-1;
		ocur_mod=cur_mod;
		if(size>(curbsize-IDLENGTH)){
			curbsize+=TBUFSIZE;
			bstring=(char *)REALLOC(bstring,curbsize);
		}
		nexttok();
		if(tok==tk_openbrace){
			bstring[size]='{';
			size++;
			do{
				start=inptr2-1;
				ocur_mod=cur_mod;
				if(size>(curbsize-IDLENGTH)){
					curbsize+=TBUFSIZE;
					bstring=(char *)REALLOC(bstring,curbsize);
				}
				nexttok();
				if(tok==tk_number){
					switch(itok.rm){
						case tk_double:
						case tk_float:
							sprintf(itok.name,"%e",itok.dnumber);
							break;
						case tk_qword:
							sprintf(itok.name,"0x%X%08X",itok.lnumber>>16,itok.number);
							break;
						default:
							sprintf(itok.name,"0x%X",itok.number);
							break;
					}
				}
				if(itok.name[0]!=0){
					strcpy(bstring+size,itok.name);
					size+=strlen(itok.name);
				}
				else{
					if(cur_mod!=ocur_mod){
						start=inptr2-2;
						if(start<0)start=0;
					}
					strncpy(bstring+size,(char *)(input+start),inptr2-1-start);
					size+=inptr2-1-start;
				}
			}while(tok!=tk_closebrace&&tok!=tk_eof);
			if(tok!=tk_eof)continue;
		}
		if(tok==tk_number){
			switch(itok.rm){
				case tk_double:
				case tk_float:
					sprintf(itok.name,"%e",itok.dnumber);
					break;
				case tk_qword:
					sprintf(itok.name,"0x%X%08X",itok.lnumber>>16,itok.number);
					break;
				default:
					sprintf(itok.name,"0x%X",itok.number);
					break;
			}
		}
		if(itok.name[0]!=0){
			strcpy(bstring+size,itok.name);
			size+=strlen(itok.name);
		}
		else{
			if(cur_mod!=ocur_mod){
				start=inptr2-2;
				if(start<0)start=0;
			}
			strncpy(bstring+size,(char *)(input+start),inptr2-1-start);
			size+=inptr2-1-start;
		}
		if(tok==tk_eof){
			unexpectedeof();
			return NULL;
		}
		if(tok==tk_camma&&use_extract==FALSE)break;
	}while(tok!=tk_semicolon);
//	size++;
	bstring[size]=0;
	return bstring;
}

int SkipBlock()
{
	for(int i=1;i!=0;){
		FastTok(0);
		if(tok==tk_question)CheckDir();
		switch(tok){
			case tk_eof: unexpectedeof(); return FALSE;
			case tk_openbrace: i++; break;
			case tk_closebrace: i--; break;
		}
	}
	return TRUE;
}

int SkipParam()
{
	for(int i=1;i!=0;){
		FastTok(0);
		if(tok==tk_question)CheckDir();
		switch(tok){
			case tk_openbracket: i++; break;
			case tk_closebracket: i--; break;
			case tk_eof: unexpectedeof(); return FALSE;
		}
	}
	return TRUE;
}

int SkipLocalVar()
{
	while(tok!=tk_openbrace&&tok!=tk_eof){
		if(tok==tk_question){
			CheckDir();
			continue;
		}
		if(tok==tk_id&&(strcmp(itok.name,"struct")==0||strcmp(itok.name,"union")==0)){
			do{
				FastTok(0);
				if(tok==tk_eof){
					unexpectedeof();
					return FALSE;
				}
			}while(tok!=tk_semicolon&&tok!=tk_openbrace);
			if(tok==tk_openbrace){
				FastTok(0);
				if(!SkipBlock())return FALSE;
			}
		}
		FastTok(1);
	}
	return TRUE;
}

SAVEPAR *SRparam(int save,SAVEPAR *par)	//save or restore global param compiler
{
	if(save){
		par=(SAVEPAR *)MALLOC(sizeof(SAVEPAR));
		par->ooptimizespeed=optimizespeed;
		par->owarning=      warning;
		par->odbg=          dbg;
		par->odosstring=    dosstring;
		par->ouseinline=    useinline;
		par->oam32= 		     am32;
		par->oalignword=    alignword;
		par->oAlignCycle=   AlignCycle;
		par->oidasm=        idasm;	//асс
		par->ooptnumber=    optnumber;
		par->odivexpand=    divexpand;
		par->ooptstr=	     optstr;	//оп
		par->ochip=         chip;
		par->oaligncycle=   aligncycle;
		par->ouselea=       uselea;
		par->oregoverstack= regoverstack;
		return par;
	}
	if(par){
		optimizespeed=par->ooptimizespeed;
		warning=      par->owarning;
		dbg=          par->odbg;
		dosstring=    par->odosstring;
		useinline=    par->ouseinline;
		am32= 		    par->oam32;
		alignword=    par->oalignword;
		AlignCycle=   par->oAlignCycle;
		idasm=        par->oidasm;	//асс
		optnumber=    par->ooptnumber;
		divexpand=    par->odivexpand;
		optstr=	     par->ooptstr;	//оп
		chip=         par->ochip;
		aligncycle=   par->oaligncycle;
		uselea    =   par->ouselea;
		regoverstack= par->oregoverstack;
		free(par);
	}
	return NULL;
}

void dynamic_proc()
{
int dtok,start,size,line;
ITOK otok;
char *bstring;
idrec *ptr;
_PROCINFO_ *pinfo;
	if(itok.npointr)itok.rm=(am32==TRUE?tk_dword:tk_word);
	dtok=tok;
	otok=itok;
	line=linenum2;
	switch(tok){
		case tk_id:
		case tk_ID:
			string[0]=0;
		case tk_undefproc:
		case tk_declare:
			break;
		default: idalreadydefined(); break;
	}
	if(itok.flag&f_export)preerror("'_export' not use in dynamic functions");
	bstring=BackString((char *)string);
	start=inptr2-1;
	pinfo=(_PROCINFO_ *)MALLOC(sizeof(_PROCINFO_));
	if(itok.flag&f_classproc)pinfo->classteg=searchteg;
	else pinfo->classteg=NULL;
	pinfo->warn=warning;
	pinfo->speed=optimizespeed;
	pinfo->lst=(dbg&2)>>1;
	pinfo->typestring=dosstring;
	pinfo->inlinest=useinline;
	pinfo->code32=am32;
	pinfo->align=alignword;
	pinfo->acycle=AlignCycle;
	pinfo->idasm=idasm;
	pinfo->opnum=optnumber;
	pinfo->de=divexpand;
	pinfo->ostring=optstr;
	pinfo->chip=chip;
	pinfo->sizeacycle=aligncycle;
	pinfo->uselea=uselea;
	pinfo->regoverstack=regoverstack;
	nexttok();
	if(dtok==tk_id||dtok==tk_ID){
		param[0]=0;
		if(tok2!=tk_closebracket&&(otok.flag&f_typeproc)==tp_fastcall){
			nexttok();	//параметры регистровой процедуры
			declareparamreg();
			free(bstring);
			bstring=BackString((char *)param);
			nexttok();
			inptr=inptr2;
			cha=cha2;
			linenumber=linenum2;
		}
	}
	else{
		inptr=inptr2;
		cha=cha2;
		if(!SkipParam()){
			free(bstring);
			return;
		}
		FastTok(1);
	}
	if(tok==tk_semicolon)preerror("error declare dynamic function");
	if((!SkipLocalVar())||(!SkipBlock())){
		free(bstring);
		return;
	}
	size=inptr-start+1;
	inptr2=inptr;
	cha2=cha;
	linenum2=linenumber;
	linenumber=line;
	itok=otok;
	strcpy((char *)string,bstring);
	free(bstring);
	bstring=(char *)MALLOC(size+1);
	strncpy(bstring,(char *)(input+start),size);
	bstring[size-1]=';';
	bstring[size]=0;

//	printf("tok=%d %s\n%s\n",dtok,itok.name,bstring);
	itok.size=0;
	tok=tk_proc;
	itok.segm=DYNAMIC;
	if(dtok==tk_id||dtok==tk_ID){
		int i;
		itok.number=secondcallnum++;
		itok.type=tp_ucnovn;
		if((i=FindUseName(itok.name))!=0)itok.segm=DYNAMIC_USED;
		ptr=addtotree(itok.name);
		if(i){
			ptr->count=i;
			ptr=itok.rec;
			AddDynamicList(ptr);
		}
	}
	else{
		if(dtok==tk_undefproc)itok.segm=DYNAMIC_USED;
		updatetree();
		ptr=itok.rec;
		//11.08.04 23:38
		strcpy(ptr->recid,itok.name);

		if(dtok==tk_undefproc&&(itok.flag&f_classproc))AddDynamicList(ptr);
	}
	ptr->line=linenumber;
	ptr->file=currentfileinfo;
	pinfo->buf=bstring;
	ptr->pinfo=pinfo;
//	ptr->sbuf=bstring;
//	linenumber=linenum2;
	if(searchteg)searchteg=NULL;
	nexttok();
}

/* ======= procedure handling ends here ======== */

int  macros(int expectedreturn)
{
int dynamicindex,actualreturn;
ITOK otok;
int orettype;
unsigned int oproctype;
int otok2;
unsigned int typep;
int snum=0;
	actualreturn=(am32==FALSE?tk_word:tk_dword);
	switch(tok){
		case tk_ID:
		case tk_id:
			dynamicindex=NOT_DYNAMIC;
			break;
		case tk_proc:
		case tk_undefproc:
		case tk_declare:
			dynamicindex=itok.segm;
			actualreturn=itok.rm;
			break;
		default:
			idalreadydefined();
			return expectedreturn;
	}
	typep=itok.flag;
	otok=itok;
	if(tok==tk_ID)param[0]=0;
	else strcpy(param,(char *)string);
	nexttok();
	orettype=returntype;
	returntype=actualreturn;	//01.08.04 14:45
	oproctype=current_proc_type;
	if(dynamicindex==NOT_DYNAMIC)doregparams();
	else{
		switch(typep&f_typeproc){
			case tp_cdecl:
			case tp_stdcall:
				snum=swapparam();
				break;
			case tp_pascal:
				snum=doparams();
				break;
			case tp_fastcall:
				doregparams();
				break;
		}
	}
	itok=otok;
	otok2=tok2;
	if(dynamicindex==NOT_DYNAMIC){
		if((actualreturn=includeit(0))==-1){
			char holdstr[IDLENGTH+16];
			sprintf(holdstr,"unknown macro '%s'",itok.name);
			preerror(holdstr);
		}
	}
	else insert_dynamic(TRUE);
	if(actualreturn!=tk_void&&expectedreturn!=tk_ID)convert_returnvalue(expectedreturn,actualreturn);
	returntype=orettype;
	current_proc_type=oproctype;
	if(snum!=0){
		if(typep&f_retproc)warningdestroyflags();
		CorrectStack(snum);
	}
	tok2=otok2;
	return actualreturn;
}

int updatecall(unsigned int which,unsigned int where,unsigned int top)
/* update output with newly defined location, but only for addresses after
	 and including top. return the number of addresses updated.
  which - адрес процедуры
  where - текущий адрес*/
{
unsigned int count=0;
long hold;
int updates=0;
	while(count<posts){
		if(((postbuf+count)->type>=CALL_SHORT&&(postbuf+count)->type<=CONTINUE_32)&&
				(postbuf+count)->num==which&&(postbuf+count)->loc>=top){
			hold=(long)where-(long)(postbuf+count)->loc;
			if((postbuf+count)->type>=CALL_NEAR&&(postbuf+count)->type<=CONTINUE_NEAR){	//NEAR
				hold-=2;
				*(unsigned short *)&output[(postbuf+count)->loc]=(unsigned short)hold;
			}
			else if((postbuf+count)->type>=CALL_32&&(postbuf+count)->type<=CONTINUE_32){//32-BIT
				hold-=4;
				*(unsigned long *)&output[(postbuf+count)->loc]=(unsigned long)hold;
			}
			else{	//SHORT
				hold--;	 // CALL_SHORT
				if(short_ok(hold))output[(postbuf+count)->loc]=(unsigned char)hold;
				else{
					if((postbuf+count)->type==BREAK_SHORT)preerror3("BREAK distance too large, use break",(postbuf+count)->line);
					else if((postbuf+count)->type==CONTINUE_SHORT)preerror3("CONTINUE distance too large, use continue",(postbuf+count)->line);
					else preerror3(shorterr,(postbuf+count)->line,(postbuf+count)->file);
				}
			}
			if(hold<127){
				if((postbuf+count)->type==JMP_NEAR||(postbuf+count)->type==JMP_32)warningjmp("GOTO",(postbuf+count)->line,(postbuf+count)->file);
				if((postbuf+count)->type==BREAK_NEAR||(postbuf+count)->type==BREAK_32)warningjmp("BREAK",(postbuf+count)->line);
				if((postbuf+count)->type==CONTINUE_NEAR||(postbuf+count)->type==CONTINUE_32)warningjmp("CONTINUE",(postbuf+count)->line);
			}
			killpost(count);
			updates++;
		}
		else count++;
	}
	if(updates==1&&hold==0)return -1;
	return(updates);
}

void define_locallabel()
{
	FindOff(string,CS);
	updatecall((unsigned int)updatelocalvar((char *)string,tk_number,outptr),outptr,procedure_start);
	nextexpecting2(tk_colon);
	RestoreStack();
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
}

void  addacall(unsigned int idnum,unsigned char callkind)
{
	CheckPosts();
	(postbuf+posts)->num=idnum;
	(postbuf+posts)->loc=outptr+1;
	(postbuf+posts)->type=callkind;
	(postbuf+posts)->line=(unsigned short)linenumber;
	(postbuf+posts)->file=(unsigned short)currentfileinfo;
	posts++;
}

unsigned int dofrom() // returns number of bytes read from FROM file
{
int filehandle;
long filesize;
	if(tok!=tk_string){
		stringexpected();
		return(0);
	}
#ifndef _WIN32_
	for(char* p=(char *)string3; *p; ++p) if(*p=='\\') *p='/';
#endif
	filehandle=open((char *)string3,O_BINARY|O_RDONLY);
	if(filehandle==-1){
		unableopenfile((char *)string3);
		return(0);
	}
	if((filesize=getfilelen(filehandle))==-1L){
		preerror("Unable to determine FROM file size");
		close(filehandle);
		return(0);
	}
	if(am32==FALSE&&filesize>=0xFFFFL){
		preerror("FROM file too large");
		close(filehandle);
		return(0);
	}
	LoadData(filesize,filehandle);
	return filesize;
}

unsigned int doextract() // returns number of bytes EXTRACTed
{
unsigned int sizetoread;
int filehandle;
long filesize,startpos;
	if(tok!=tk_string){
		stringexpected();
		return(0);
	}
	filehandle=open((char *)string3,O_BINARY|O_RDONLY);
	if(filehandle==-1){
		unableopenfile((char *)string3);
		return(0);
	}
	nexttok();
	expecting(tk_camma);
	if(tok!=tk_number){
		numexpected();
		return(0);
	}
	startpos=doconstlongmath();
	expecting(tk_camma);
	if(tok!=tk_number){
		numexpected();
		return(0);
	}
	sizetoread=doconstlongmath();
	if((filesize=getfilelen(filehandle))==-1L){
		preerror("Unable to determine EXTRACT file size");
		close(filehandle);
		return(0);
	}
	if(filesize<=startpos){
		preerror("EXTRACT offset exceeds the length of the file");
		close(filehandle);
		return(0);
	}
	if(sizetoread==0)sizetoread=filesize-startpos;
	if(am32==FALSE&&sizetoread>=0xFFFFL){
		preerror("Block to EXTRACT exceeds 64K");
		close(filehandle);
		return(0);
	}
	lseek(filehandle,startpos,0); 	// error checking required on this
	LoadData(sizetoread,filehandle);
	return sizetoread;
}

void LoadData(unsigned int size,int filehandle)
{
	if(splitdata){
		while(((unsigned long)size+(unsigned long)outptrdata)>=outdatasize){
			if(CheckDataSize()==0)break;
		}
		if((unsigned int)read(filehandle,outputdata+outptrdata,size)!=size)errorreadingfile((char *)string3);
		outptrdata+=size;
	}
	else{
		while(((unsigned long)size+(unsigned long)outptr)>=outptrsize){
			if(CheckCodeSize()==0)break;
		}
		if((unsigned int)read(filehandle,output+outptr,size)!=size)errorreadingfile((char *)string3);
		outptr+=size;
		outptrdata=outptr;
	}
	close(filehandle);
}

void  op66(int ctok)
{
	if((am32==FALSE&&ctok==r32)||(am32!=FALSE&&ctok==r16)){
		if(cpu<3)cpu=3;
		op(0x66);
	}
}

int  op67(int ctok)
{
	if((am32==FALSE&&ctok==r32)||(am32!=FALSE&&ctok==r16)){
		if(cpu<3)cpu=3;
		op(0x67);
		return TRUE;
	}
	return FALSE;
}

void  outseg(ITOK *outtok,unsigned int locadd)
{
int rmm1;
	rmm1=outtok->rm&7;
	if(outtok->sib!=CODE16){
		if(am32==FALSE)op(0x67);
		if(rmm1==4)locadd++;
	}
	else if(am32!=FALSE)op(0x67);
	switch(outtok->segm){
		case ES: op(0x26); break;
		case CS: op(0x2E); break;
		case FS: op(0x64);
			if(cpu<3)cpu=3;
			break;
		case GS: op(0x65);
			if(cpu<3)cpu=3;
			break;
		case SS:
			if(outtok->sib==CODE16){
				if(rmm1!=2&&rmm1!=3&&!(rmm1==6&&outtok->rm!=6))op(0x36);
			}
			else{
				if(rmm1==4){
					rmm1=outtok->sib&7;
					if(rmm1!=4){
						if(rmm1==5){
							if(outtok->rm==4)op(0x36);
							break;
						}
						op(0x36);
					}
				}
				else if(rmm1==5){
					if(outtok->rm==5)op(0x36);
					else break;
				}
				else op(0x36);
			}
			break;
		case DS:
			if(outtok->sib==CODE16){
				if(rmm1==2||rmm1==3||(rmm1==6&&outtok->rm!=6))op(0x3E);
			}
			else{
				if(rmm1==4){
					rmm1=outtok->sib&7;
					if(rmm1==4||(rmm1==5&&outtok->rm!=4))op(0x3e);
				}
				else if(rmm1==5&&outtok->rm!=5)op(0x3e);
			}
	}
	CheckPosts();
	if(outtok->post!=0&&outtok->post!=UNDEF_OFSET){
		if((outtok->flag&f_extern)){
			(postbuf+posts)->type=EXT_VAR;
			(postbuf+posts)->num=outtok->number&0xFFFF;
			outtok->number>>=16;
		}
		else if(outtok->post==USED_DIN_VAR){
			(postbuf+posts)->type=(unsigned short)(am32==0?DIN_VAR:DIN_VAR32);
//			printf("Add tok=%d %08X sib=%d %s\n",outtok->rec->rectok,outtok->rec->right,outtok->rec->recsib,outtok->rec->recid);
			if(outtok->rec->rectok==tk_structvar&&outtok->rec->recsib==tp_gvar){
				(postbuf+posts)->num=(int)outtok->rec;//02.09.05 17:10 ->right;
			}
			else (postbuf+posts)->num=(int)outtok->rec;
		}
//		else if((outtok->flag&f_dataseg))(postbuf+posts)->type=(unsigned short)(am32==0?DATABLOCK_VAR:DATABLOCK_VAR32);
		else (postbuf+posts)->type=(unsigned short)(am32==0?POST_VAR:POST_VAR32);
		(postbuf+posts)->loc=outptr+locadd;
		posts++;
	}
	else if(outtok->flag&f_reloc){
		(postbuf+posts)->type=(unsigned short)(am32==0?FIX_VAR:FIX_VAR32);
		(postbuf+posts)->loc=outptr+locadd;
		posts++;
	}
}

int addpoststring(int segm,int len,int term)		/* add a string to the post queue */
{
int i;
int returnvalue;
	if((returnvalue=FindDublString(segm,len,term))!=-1)return returnvalue;
	CheckPosts();
	(postbuf+posts)->type=(unsigned short)(am32==FALSE?POST_STRING:POST_STRING32);
	(postbuf+posts)->loc=(segm==CS?outptr:outptrdata);
	(postbuf+posts)->num=segm;
	posts++;
	returnvalue=poststrptr;
	if((int)(len+poststrptr+1)>=sbufstr){
		sbufstr+=SIZEBUF;
		bufstr=(char *)REALLOC(bufstr,sbufstr);
	}
	for(i=0;i<len;i++,poststrptr++)bufstr[poststrptr]=string[i];
	switch(term&3){
		case zero_term:
			if(term&s_unicod){
				poststrptr++;
				bufstr[poststrptr]=0;
			}
			bufstr[poststrptr]=0;
			poststrptr++;
			break;
		case dos_term:
			if(term&s_unicod){
				bufstr[poststrptr]=0;
				poststrptr++;
			}
			bufstr[poststrptr]='$';
			poststrptr++;
			break;
	}
	return(returnvalue);
}

int FindDublString(int segm,unsigned int len,int term)
{
STRING_LIST ins,outs;
void *nextstr=liststring,*prevstr=NULL;
	ins.len=len;
	ins.next=NULL;
	ins.type=term;
/*	if(splitdata){	//разделеные даные и код
		ins.plase=0;
		ins.ofs=outptrdata;
	}
	else{*/
		ins.plase=POST_STRING;
		ins.ofs=poststrptr;
//	}

	while(nextstr!=NULL){
		memcpy(&outs,nextstr,sizeof(STRING_LIST));
		if(term==outs.type&&len<=outs.len){
char *instr,*outstr;
			outstr=(char *)nextstr+sizeof(STRING_LIST)+outs.len-1;
			instr=(char *)string+len-1;
char c;
int i,j;
			for(i=len,j=outs.len;i!=0;j--,i--,instr--,outstr--){
				c=*instr;
				if(c!=*outstr)break;
			}
			if(i==0){	//найдена строка
				if(!optstr)return -1;
				warningstring();
				if(outs.plase==0){	//уже в файле
					AddReloc(DS);
					return outs.ofs+j;
				}
				CheckPosts();
				(postbuf+posts)->type=(unsigned short)(am32==FALSE?POST_STRING:POST_STRING32);
				(postbuf+posts)->loc=(segm==CS?outptr:outptrdata);
				(postbuf+posts)->num=segm;
				posts++;
				return outs.ofs+j;
			}
		}
		prevstr=nextstr;
		nextstr=outs.next;
	}
	outs.next=(void *)MALLOC(sizeof(STRING_LIST)+len);
	memcpy(outs.next,&ins,sizeof(STRING_LIST));
	if(len!=0)memcpy((char *)outs.next+sizeof(STRING_LIST),&string,len);
	if(prevstr!=NULL)memcpy(prevstr,&outs,sizeof(STRING_LIST));
	else liststring=outs.next;
	return -1;
}

void killpost(unsigned int poz)
{
	posts--;
	memcpy((postinfo *)(postbuf+poz),(postinfo *)(postbuf+posts),sizeof(postinfo));
}

void dopoststrings()
{
unsigned int addvalue,i;
	if(poststrptr==0)return;
	if(splitdata){
		addvalue=outptrdata;
		if((outptrdata+poststrptr)>=outdatasize)CheckDataSize();
	}
	else{
		addvalue=outptr;
		if((outptr+poststrptr)>=outptrsize)CheckCodeSize();
	}
	datasize+=poststrptr;
	if(dbg&2)AddDataNullLine(3);
	memcpy(&outputdata[outptrdata],bufstr,poststrptr);
	outptrdata+=poststrptr;
	if(!splitdata)outptr=outptrdata;
	for(i=0;i<posts;i++){
		int segm=(postbuf+i)->num;
		if((postbuf+i)->type==POST_STRING){
			if(segm==CS)*(unsigned short *)&output[(postbuf+i)->loc]+=(unsigned short)addvalue;
			else *(unsigned short *)&outputdata[(postbuf+i)->loc]+=(unsigned short)addvalue;
			if(splitdata&&modelmem==TINY)(postbuf+i)->type=(unsigned short)DATABLOCK_VAR;
			else if(FixUp==FALSE)killpost(i--);
			else (postbuf+i)->type=(unsigned short)(segm==DS?FIX_VAR:FIX_CODE);
		}
		else if((postbuf+i)->type==POST_STRING32){
			if(segm==CS)*(unsigned int *)&output[(postbuf+i)->loc]+=addvalue;
			else *(unsigned int *)&outputdata[(postbuf+i)->loc]+=addvalue;
			if(splitdata&&modelmem==TINY)(postbuf+i)->type=(unsigned short)DATABLOCK_VAR32;
			else if(FixUp==FALSE)killpost(i--);
			else (postbuf+i)->type=(unsigned short)(segm==DS?FIX_VAR32:FIX_CODE32);
		}
	}
	poststrptr=0; 	 /* reset the poststrptr */
STRING_LIST ins;
	void *nextstr=liststring;
	while(nextstr!=NULL){
		memcpy(&ins,nextstr,sizeof(STRING_LIST));
		if(ins.plase!=0){
			ins.plase=0;
			ins.ofs+=addvalue;
			memcpy(nextstr,&ins,sizeof(STRING_LIST));
		}
		nextstr=ins.next;
	}
	if(dbg&2)AddCodeNullLine();
}

void insertcode() 		// force code procedure at specified location
{
	nexttok();
	testInitVar(FALSE);
	if((itok.flag&f_extern)!=0){
		notexternfun();
		return;
	}
	int tproc=itok.flag&f_typeproc;
	setuprm();
	switch(tok){
		case tk_undefproc:
		case tk_declare:
			tok=tk_proc;
			itok.number=outptr;
			updatecall((unsigned int)updatetree(),(unsigned int)itok.number,0);
			if(tproc==tp_fastcall){
				if(includeit(1)==-1)thisundefined(itok.name);
			}
			else if(includeproc()==-1)thisundefined(itok.name);
			break;
		case tk_id:
		case tk_ID:
			tok=tk_proc;
			itok.number=outptr;
			string[0]=0;
			itok.type=tp_ucnovn;
			addtotree(itok.name);
			if(tproc==tp_fastcall){
				if(includeit(1)==-1)thisundefined(itok.name);
			}
			else if(includeproc()==-1)thisundefined(itok.name);
			break;
		case tk_proc:
			if(itok.segm<NOT_DYNAMIC)insert_dynamic();
			else preerror("Function already inserted in code");
			break;
		default: idalreadydefined(); break;
	}
	nextexpecting2(tk_openbracket);
	while(tok!=tk_eof&&tok!=tk_closebracket)nexttok();
	if(tok==tk_eof)unexpectedeof();
	else nextseminext();
}

/************ some of the dynamic procedure support functions *************/

void insert_dynamic(int insert)
{
unsigned char *oinput;
int oinptr;
unsigned char ocha;
int oline;
int ofile;
char *ostartline;
int oendinptr;
structteg *osearchteg;
int oinsert;
_PROCINFO_ *pinfo;
SAVEPAR *par;
	if(insert){
		osearchteg=searchteg;
		searchteg=NULL;
	}
//	printf("cur_mod=%08X\n",cur_mod);
	oinsert=insert;
	oinput=input;
	oinptr=inptr2;
	ocha=cha2;
	oline=linenum2;
	ofile=currentfileinfo;
	(startfileinfo+currentfileinfo)->stlist=staticlist;
	oendinptr=endinptr;
	endoffile=0;
	ostartline=startline;
	idrec *ptr=itok.rec;
	pinfo=ptr->pinfo;
	input=(unsigned char *)pinfo->buf;
	inptr2=1;
	startline=(char *)input;
	cha2=input[0];
	endinptr=strlen((char *)input);
	endinput=startline+endinptr;
	linenumber=linenum2=ptr->line;
	currentfileinfo=ptr->file;
	staticlist=(startfileinfo+currentfileinfo)->stlist;
	par=SRparam(TRUE,NULL);
	warning=pinfo->warn;
	optimizespeed=pinfo->speed;
	dosstring=pinfo->typestring;
	useinline=pinfo->inlinest;
	am32=pinfo->code32;
	alignword=pinfo->align;
	AlignCycle=pinfo->acycle;
	idasm=pinfo->idasm;
	optnumber=pinfo->opnum;
	divexpand=pinfo->de;
	optstr=pinfo->ostring;
	chip=pinfo->chip;
	aligncycle=pinfo->sizeacycle;
	uselea=pinfo->uselea;
	regoverstack=pinfo->regoverstack;
	if(pinfo->classteg!=NULL){
		/*if((itok.flag&f_static)==0)*/searchteg=(structteg *)pinfo->classteg;
		insert=0;
	}
	if(pinfo->lst)dbg|=2;
	else dbg&=0xFD;
//	puts(itok.name);
	if(!insert){
		procedure_start=outptr;
		if(dbg){
			if(dbg&2){
				char m1[130];
				//11.08.04 23:39
//				if(searchteg)sprintf(m1,"%s::%s()",searchteg->name,itok.name);
//				else sprintf(m1,"%s()",itok.name);
				sprintf(m1,"%s()",itok.name);

				AddCodeNullLine(m1);
			}
			else AddLine();
		}
		if(AlignProc)AlignCD(CS,alignproc);
//		puts((char *)input);
		if(pinfo->classteg==NULL)itok.flag&=~f_static;
		setproc(1);
		dopoststrings();
	}
	else insertproc();
	input=oinput;
	inptr2=oinptr;
	cha2=ocha;
	linenum2=oline;
//	printf("cur_mod=%08X\n",cur_mod);
	(startfileinfo+currentfileinfo)->stlist=staticlist;
	currentfileinfo=ofile;
	staticlist=(startfileinfo+currentfileinfo)->stlist;
	endinptr=oendinptr;
	endoffile=0;
	startline=ostartline;
	SRparam(FALSE,par);
	if(oinsert)searchteg=osearchteg;
	else searchteg=NULL;
//	if(insert)nexttok();
//	printf("tok=%d %08X\n",tok,cur_mod);
}

idrec *addtotree(char *keystring)//добавить строку в дерево
{
struct idrec *ptr,*newptr;
int cmpresult;
//выделить память под новую проц
	newptr=(struct idrec *)MALLOC(sizeof(struct idrec));
	ptr=(itok.flag&f_static)!=0?staticlist:treestart;	//начало дерева
	if(ptr==NULL)((itok.flag&f_static)!=0?staticlist:treestart)=newptr;//начало дерева
	else{	//поиск строки в дереве
		while(((cmpresult=strcmp(ptr->recid,keystring))<0&&ptr->left!=NULL)||
       (cmpresult>0&&ptr->right!=NULL))ptr=(cmpresult<0?ptr->left:ptr->right);
		(cmpresult<0?ptr->left:ptr->right)=newptr;
	}
	strcpy(newptr->recid,keystring);//скопир название
	newptr->newid=NULL;
	if(string[0]!=0)newptr->newid=BackString((char *)string);
	newptr->rectok=tok;
	newptr->recnumber=itok.number;
	newptr->recsegm=itok.segm;
	newptr->recrm=itok.rm;
	newptr->recpost=itok.post;
	newptr->flag=itok.flag;
	newptr->recsize=itok.size;
	newptr->left=newptr->right=NULL;
	newptr->sbuf=NULL;
	newptr->recsib=itok.sib;
	newptr->line=linenumber;
	newptr->file=currentfileinfo;
	newptr->count=0;
	newptr->type=itok.type;
	newptr->npointr=itok.npointr;
	itok.rec=newptr;
	return newptr;
}

long updatetree()			 // returns the old number value
{
struct idrec *ptr;
long hold;
	ptr=itok.rec;
	if(ptr==0)internalerror("address record not found when update tree");
	if(ptr->newid)free(ptr->newid);
	ptr->newid=NULL;
	if(string[0]!=0)ptr->newid=BackString((char *)string);
	ptr->rectok=tok;
	hold=ptr->recnumber;
	ptr->recnumber=itok.number;
	ptr->recsegm=itok.segm;
	ptr->recrm=itok.rm;
	ptr->flag=itok.flag;
	ptr->recsize=itok.size;
	ptr->recsib=itok.sib;
	return hold;
}

/* --------------- local variable handling starts here ----------------- */

unsigned int  updatelocalvar(char *str,int tok4,unsigned int num)
{
struct localrec *ptr;
unsigned int retvalue;
treelocalrec *ntlr=tlr;
	while(ntlr&&ntlr->level>1)ntlr=ntlr->next;
	for(ptr=ntlr->lrec;;ptr=ptr->rec.next){
		if(strcmp(ptr->rec.recid,str)==0){
			retvalue=ptr->rec.recnumber;
			ptr->rec.rectok=tok4;
			ptr->rec.recnumber=num;
			break;
		}
		if(ptr->rec.next==NULL)break;
	}
	return(retvalue);
}

localrec * addlocalvar(char *str,int tok4,unsigned int num,int addmain)
{
localrec *ptr,*newptr;
localrec *uptr;
treelocalrec *ntlr;
	if(addmain){
		ntlr=tlr;
		while(ntlr&&ntlr->level>1)ntlr=ntlr->next;
		uptr=ntlr->lrec;
	}
	else uptr=tlr->lrec;
	newptr=(struct localrec *)MALLOC(sizeof(struct localrec));

	if(uptr==NULL){
		if(addmain)ntlr->lrec=newptr;
		else tlr->lrec=newptr;
	}
	else{
		ptr=uptr;
		while(ptr->rec.next!=NULL)ptr=ptr->rec.next;
		ptr->rec.next=newptr;
	}
	strcpy(newptr->rec.recid,str);
	newptr->rec.rectok=tok4;
	newptr->rec.recnumber=num;
	newptr->rec.next=NULL;
	newptr->rec.right=NULL;
	newptr->rec.recsize=0;
	newptr->fuse=NOTINITVAR;
	newptr->rec.type=tp_ucnovn;
	newptr->rec.flag=0;
	newptr->rec.npointr=0;
	newptr->rec.recpost=LOCAL;
	newptr->li.count=0;
	newptr->li.start=linenumber;
	return newptr;
}

void KillTegList(structteg *tteg)
{
	if(tteg){
		KillTegList(tteg->left);
		KillTegList(tteg->right);
		if(tteg->baza)free(tteg->baza);
		free(tteg);
	}
}

void killlocals(/*int endp*/)
/* Clear and free the local linked list, check for any unresolved local
jump labels. */
{
/*	if(endp){
		dopoststrings();
		for(int i=0;i<posts;i++){
//				printf("%d type=%d num=%08X\n",i+1,(postbuf+i)->type,(postbuf+i)->num);
			if((postbuf+i)->type==DIN_VAR||(postbuf+i)->type==DIN_VAR32){
				idrec *ptr=(idrec *)(postbuf+i)->num;
//				printf("sib=%d num=%08X %s\n",ptr->recsib,ptr->recnumber,ptr->recid);
				if(ptr->recsib!=tp_gvar)continue;
				puts("recsib=tp_gvar");
				if(ptr->recpost==USED_DIN_VAR){
unsigned int otok,otok2;
ITOK oitok;
					oitok=itok;
					otok=tok;
					otok2=tok2;
					setdindata(ptr,i);
					itok=oitok;
					tok=otok;
					tok2=otok2;
				}
				else{
					if((postbuf+i)->type==DIN_VAR)*(unsigned short *)&output[(postbuf+i)->loc]+=(unsigned short)(ptr->recnumber);
					else *(unsigned long *)&output[(postbuf+i)->loc]+=ptr->recnumber;
				}
				if(FixUp)(postbuf+i)->type=(unsigned short)((postbuf+i)->type==DIN_VAR?FIX_VAR:FIX_VAR32);
				else killpost(i--);
			}
		}
		dopoststrings();
	}
	*/
treelocalrec *ftlr,*ftlr1;
struct localrec *ptr, *ptr1;
	for(ftlr=btlr;ftlr!=NULL;){
		ftlr1=ftlr;
		for(ptr=ftlr->lrec;ptr!=NULL;){
			ptr1=ptr;
			if(ptr->rec.rectok==tk_locallabel){  /* check for unresolved labels */
char holdstr[32+IDLENGTH];
				sprintf(holdstr,"local jump label '%s' unresolved",ptr1->rec.recid);
				preerror(holdstr);
			}
//			printf("type=%d post=%08X %s\n",ptr->rec.type,ptr->rec.recpost,ptr->rec.recid);
			if(ptr->rec.rectok==tk_structvar){
				if(ptr->rec.count==0){
					warningnotused(ptr->rec.recid,5);
					if(ptr->rec.type==tp_gvar)free(ptr->rec.sbuf);
				}
			}
			else if(ptr->fuse<USEDVAR){
				if(ptr->rec.type==tp_localvar||ptr->rec.type==tp_postvar||ptr->rec.type==tp_gvar)warningnotused(ptr->rec.recid,3);
				else if(ptr->rec.type==tp_paramvar)warningnotused(ptr->rec.recid,4);
				if(ptr->rec.type==tp_gvar)free(ptr->rec.sbuf);
			}
			ptr=ptr->rec.next;
			if(ptr1->rec.recpost!=USED_DIN_VAR)free(ptr1);
		}
		ftlr=ftlr->next;
		free(ftlr1);
	}
	btlr=NULL;
	paramsize=0;
	localsize=0;
	KillTegList(ltegtree);
	ltegtree=NULL;
}

/* ================ input procedures start ================= */
int loadinputfile(char *inpfile)	//считывание файла в память
{
unsigned long size;
int filehandle;
	if((filehandle=open(inpfile,O_BINARY|O_RDONLY))==-1)return -2;
	if((size=getfilelen(filehandle))==0){
		badinfile(inpfile);
		close(filehandle);
		return(-1);
	}
	if(totalmodule==0){
		startfileinfo=(struct FILEINFO *)MALLOC(sizeof(FILEINFO));
		totalmodule=1;
		currentfileinfo=0;
	}	
	else{	//поиск емени файла в списке обработанных
		for(currentfileinfo=0;currentfileinfo<totalmodule;currentfileinfo++){
			if(stricmp(inpfile,(startfileinfo+currentfileinfo)->filename)==0)break;
		}
		if(currentfileinfo!=totalmodule){
			if(crif!=FALSE)return 1;
			goto cont_load;
		}
		totalmodule++;
		startfileinfo=(struct FILEINFO *)REALLOC(startfileinfo,sizeof(FILEINFO)*(totalmodule));
	}

	(startfileinfo+currentfileinfo)->stlist=NULL;
	(startfileinfo+currentfileinfo)->filename=(char *)MALLOC(strlen(inpfile)+1);
	strcpy((startfileinfo+currentfileinfo)->filename,inpfile);
	(startfileinfo+currentfileinfo)->numdline=0;
	//GetFileTime(filehandle,&(startfileinfo+currentfileinfo)->time); // bug
//	getftime(filehandle,&(startfileinfo+currentfileinfo)->time);
cont_load:
	staticlist=(startfileinfo+currentfileinfo)->stlist;
	input=(unsigned char *)MALLOC(size+1);
	printf("%08lX %s %lu\n",input,inpfile,size);
	if((endinptr=read(filehandle,input,size))!=size){
printf("%d\n",endinptr);
		
		errorreadingfile(inpfile);
		close(filehandle);
		return(-1);
	}
	close(filehandle);
	return(0);
}

void notnegit(int notneg)
/* produce NOT .. or NEG .. */
{
int wordadd=0,i=0;
	getoperand();
	switch(tok){
		case tk_reg: wordadd=1; op66(r16);
			ClearReg(itok.number);
		case tk_beg:
			if(optimizespeed&&(chip==5||chip==6)){
				if(wordadd==0&&itok.number==AL)outword(0xFF34);
				else{
					if(wordadd)op(0x83);
					else op(0x80);
					op(0xF0+itok.number);
					op(0xFF);
				}
				if(notneg){
					if(wordadd){
						op66(r16);
						op(0x40+itok.number);
					}
					else{
						op(0xFE);
						op(0xC0+itok.number);
					}
				}
			}
			else{
				op(0xF6+wordadd);
				op(0xD0+notneg+itok.number);
			}
			if(wordadd==0)ClearReg(itok.number&3);
			break;
		case tk_wordvar:
		case tk_intvar: wordadd=1;
			i=1;
		case tk_bytevar:
		case tk_charvar:
			i++;
			CheckAllMassiv(bufrm,i,&strinf);
			if(wordadd)op66(r16);
			outseg(&itok,2);
			if((!notneg)&&optimizespeed&&(chip==5||chip==6)){
				op(wordadd!=0?0x83:0x80);
				op(0x30+itok.rm);
				outaddress(&itok);
				op(0xFF);
			}
			else{
				op(0xF6+wordadd);
				op(0x10+notneg+itok.rm);
				outaddress(&itok);
			}
			KillVar(itok.name);
			break;
		case tk_reg32:
			op66(r32);
			if(optimizespeed&&(chip==5||chip==6)){
				op(0x83);
				outword(0xFFF0+itok.number);
				if(notneg){
					op66(r32);
					op(0x40+itok.number);
				}
			}
			else{
				op(0xF7);
				op(0xD0+notneg+itok.number);
			}
			if(cpu<3)cpu=3;
			ClearReg(itok.number);
			break;
		case tk_reg64:
			int r1,r2;
			r1=itok.number&255;
			r2=itok.number/256;
			op66(r32);
			op(0xF7);
			op(0xD0+notneg+r2);  // NEG reg
			op66(r32);
			op(0xF7);
			op(0xD0+notneg+r1);  // NEG reg
			op66(r32);
			op(0x83);
			op(0xD8+r2);
			op(0);
			ClearReg(r1);
			ClearReg(r2);
			break;
		case tk_longvar:
		case tk_dwordvar:
			CheckAllMassiv(bufrm,4,&strinf);
			op66(r32);
			outseg(&itok,2);
			if((!notneg)&&optimizespeed&&(chip==5||chip==6)){
				op(0x83);
				op(0x30+itok.rm);
				outaddress(&itok);
				op(0xFF);
			}
			else{
				op(0xF7);
				op(0x10+notneg+itok.rm);
				outaddress(&itok);
			}
			if(cpu<3)cpu=3;
			KillVar(itok.name);
			break;
		case tk_qword:
			itok.number+=4;
			compressoffset(&itok);
			for(i=0;i<2;i++){
				CheckAllMassiv(bufrm,8,&strinf);
				op66(r32);
				outseg(&itok,2);
				op(0xF7);
				op(0x10+notneg+itok.rm);
				outaddress(&itok);
				if(i==1)break;
				itok.number-=4;
				compressoffset(&itok);
			}
			itok.number-=4;
			compressoffset(&itok);
			CheckAllMassiv(bufrm,8,&strinf);
			op66(r32);
			outseg(&itok,2);
			op(0x83); op(0x18+itok.rm);
			outaddress(&itok);
			op(0);
			KillVar(itok.name);
			if(cpu<3)cpu=3;
			break;
		case tk_doublevar:
			i=4;
		case tk_floatvar:
			if(notneg!=8)illegalfloat();
			CheckAllMassiv(bufrm,4+i,&strinf);
			outseg(&itok,2); //fld var
			op(0xd9+i);
			op(itok.rm);
			outaddress(&itok);
			outword(0xe0d9);    //fchs
			outseg(&itok,2);//fstp var
			op(0xd9+i);
			op(itok.rm+0x18);
			outaddress(&itok);
			fwait3();
			break;
		default: varexpected(0);
	}
}

void setreturn()
{
	if(numreturn){
unsigned int pos,dist;
//int j=0;
//restart:
		for(int i=0;i<numreturn;i++){
//			if(!(listreturn+i)->use)continue;
			pos=(listreturn+i)->loc;
			dist=outptr-pos;
			if((listreturn+i)->type==tk_RETURN){
				dist--;
//				if(dist){
					if(dist>127/*&&i>=j*/)jumperror((listreturn+i)->line,mesRETURN);
					output[pos]=(unsigned char)dist;
/*				}
				else{
					outptr-=2;
					j=i;
					(listreturn+i)->use=FALSE;
					goto restart;
				}*/
			}
			else{
				dist-=(am32==0?2:4);
//				if(dist){
					if(dist<128/*&&i>=j*/)warningjmp(mesRETURN,(listreturn+i)->line,currentfileinfo);
					if(am32)*(unsigned long *)&output[pos]=dist;
					else*(unsigned short *)&output[pos]=(unsigned short)dist;
/*				}
				else{
					outptr-=3;
					if(am32)outptr-=2;
					j=i;
					(listreturn+i)->use=FALSE;
					goto restart;
				}*/
			}
		}
		free(listreturn);
		listreturn=NULL;
		numreturn=0;
	}
}

void RestoreSaveReg()
{
	if(psavereg->all){
		op(0x61);
		addESP-=am32==FALSE?16:32;
	}
	else{
		for(int i=7;i>=0;i--){
			if(psavereg->reg[i]){
				op66(psavereg->reg[i]);
				op(0x58+i);
				addESP-=am32==FALSE?2:4;
			}
		}
	}
}

void AddRetList(int pos,int line,int type)
{
	if(numreturn==0)listreturn=(RETLIST *)MALLOC(sizeof(RETLIST));
	else listreturn=(RETLIST *)REALLOC(listreturn,sizeof(RETLIST)*(numreturn+1));
	(listreturn+numreturn)->loc=pos;
	(listreturn+numreturn)->line=line;
	(listreturn+numreturn)->type=type;
//	(listreturn+numreturn)->use=TRUE;
	if(type==tk_return)jumploc0();
	else outword(0x00EB); 	// JMP SHORT
	numreturn++;
}

void RetProc()
{
	if((current_proc_type&f_far)){
		if((current_proc_type&f_typeproc)==tp_cdecl)retf();
		else{
			if(paramsize==0)retf(); 							/* RETF */
			else{
				op(0xCA);
				outword(paramsize);
			}
		}
	}
	else{
		if((current_proc_type&f_typeproc)==tp_cdecl)ret();
		else if(current_proc_type==f_interrupt)op(0xCF);//interrupt procedure IRET
		else{
			if(paramsize==0)ret();							 /* RET */
			else{
				op(0xC2);
				outword(paramsize);
			}
		}
	}
}

void doreturn(int typer) 	 /* do return(...); */
{
char sign=0;
int line=linenumber;
char *ofsstr=NULL;
int i;
unsigned int oaddESP=addESP;
	if(tok2==tk_openbracket)nexttok();
	if((ofsstr=GetLecsem(tk_closebracket,tk_semicolon))){
		int retreg;
		int razr=getrazr(returntype);
		if((retreg=CheckIDZReg(ofsstr,AX,razr))!=NOINREG){
			GetEndLex(tk_closebracket,tk_semicolon);
			if(razr==r16)tok=tk_reg;
			else if(razr==r32)tok=tk_reg32;
			else tok=tk_beg;
			itok.number=retreg==SKIPREG?AX:retreg;
			goto nn1;
		}
	}
	getoperand();
	if(tok!=tk_closebracket&&tok!=tk_semicolon){
nn1:
		switch(returntype){
			case tk_int: sign=1;
			case tk_word: do_e_axmath(sign,r16,&ofsstr); break;
			case tk_char: sign=1;
			case tk_byte: doalmath(sign,&ofsstr); break;
			case tk_long: sign=1;
			case tk_dword: do_e_axmath(sign,r32,&ofsstr); break;
			case tk_qword:
				getintoreg64(EAX|(EDX*256));
				if(itok.type!=tp_stopper&&tok!=tk_eof&&itok.type!=tp_compare)doregmath64(EAX|(EDX*256));
				break;
			case tk_void: retvoid(); nexttok(); break;
			case tk_double:
/*				if(tok2==tk_closebracket)doregmath64(EAX|(EDX*256));
				else doeaxfloatmath(tk_reg64);
				break;*/
			case tk_fpust:
				doeaxfloatmath(tk_fpust);
				break;
			case tk_float:
				if(itok2.type==tp_stopper&&(tok==tk_floatvar||tok==tk_number)){
					if(tok==tk_floatvar){
						tok=tk_dwordvar;
						do_e_axmath(0,r32,&ofsstr);
					}
					else doeaxfloatmath(tk_reg32,0,0);
				}
				else{
					doeaxfloatmath(tk_stackstart,0,0);
					op66(r32);
					op(0x58);	//pop eax
				}
				break;
//			default:
//				printf("returntype=%d\n",returntype);
		}
	}
	if(tok==tk_closebracket)nexttok();
	seminext();
	if(ofsstr)free(ofsstr);
	clearregstat();
#ifdef OPTVARCONST
	ClearLVIC();
#endif
	if(typer!=tokens){
		i=(am32==0?2:4);
		if(typer==tk_RETURN)i=1;
		if(paramsize||localsize)i--;
		if(insertmode||((!optimizespeed)&&paramsize&&
				(current_proc_type&f_typeproc)!=tp_cdecl)||psavereg->size>i){
			if(numblocks>1){	//заменить return на goto
				AddRetList(outptr+1,line,typer);
				retproc=TRUE;
				return;
			}
			else{
				setreturn();
				if(insertmode)return;
			}
		}
	}
	if(numblocks==1)setreturn();	//06.09.04 22:20
	if(!inlineflag)leaveproc();
	else{
		AutoDestructor();
		RestoreStack();
		RestoreSaveReg();
		RetProc();
	}
	retproc=TRUE;
	if(numblocks>1||(numblocks==1&&tok!=tk_closebrace))addESP=oaddESP;
}

int IsSaveReg()
{
	if(psavereg->all)return TRUE;
	for(int i=7;i>=0;i--){
		if(psavereg->reg[i])return TRUE;
	}
	return FALSE;
}

void leaveproc()
{
	AutoDestructor();
	RestoreStack();
	RestoreSaveReg();
	if(ESPloc==FALSE||am32==FALSE){
		if(localsize)Leave();
		else if(paramsize)op(0x5D); /* POP BP */
		else if(initBP)Leave();
	}
	else if(localsize){
		if(short_ok(localsize,TRUE)){
			outword(0xC483);
			op(localsize);
		}
		else{
			outword(0xC481);
			outdword(localsize);
		}
	}
	RetProc();
}

DLLLIST *FindDLL()
{
DLLLIST *newdll;
	if(listdll!=NULL){	//список DLL не пуст
		for(newdll=listdll;stricmp(newdll->name,(char *)string)!=0;newdll=newdll->next){
			if(newdll->next==NULL){	//последняя в списке
				newdll->next=(DLLLIST *)MALLOC(sizeof(DLLLIST));//создать новую
				newdll=newdll->next;
				newdll->next=NULL;
				newdll->num=0;
				newdll->list=NULL;
				strcpy(newdll->name,(char *)string);
				break;
			}
		}
	}
	else{
		listdll=newdll=(DLLLIST *)MALLOC(sizeof(DLLLIST));
		newdll->next=NULL;
		newdll->num=0;
		newdll->list=NULL;
		strcpy(newdll->name,(char *)string);
	}
	return newdll;
}

void declareextern()
{
int next;
	nexttok();
	if(comfile==file_w32&&strcmp(itok.name,"WINAPI")==0){
		nexttok();
		if(tok!=tk_string)stringexpected();
		DLLLIST *newdll;
		newdll=FindDLL();
		nextexpecting2(tk_openbrace);	//открытие списка процедур
		APIPROC *listapi=newdll->list;
		returntype=tk_declare;
		do{
			if(tok==tk_enum)doenum();
			else if(tok==tk_struct)InitStruct();
			else{
				next=TRUE;
				if(testInitVar(FALSE)!=FALSE)preerror("Error declare WINAPI");
				if(itok.rm==tokens)itok.rm=tk_dword;
				if(itok.npointr)itok.rm=(am32==TRUE?tk_dword:tk_word);
				ITOK hitok=itok;
				int htok=tok;
				param[0]=0;
				hitok.sib=hitok.size=-1;
				if(tok2==tk_period){
					nexttok();
					if(tok2==tk_number){
						nexttok();
						hitok.sib=itok.number;
					}
				}
				if(tok2==tk_at){
					nexttok();
					if(tok2==tk_number){
						nexttok();
						hitok.size=itok.number;
					}
				}
				else{
					nextexpecting2(tk_openbracket);
					if((hitok.flag&f_typeproc)==tp_fastcall)declareparamreg();
					else declareparamstack();
				}
				if(htok==tk_id||htok==tk_ID){
					tok=tk_apiproc;
					itok=hitok;
					itok.number=secondcallnum++;
					itok.segm=NOT_DYNAMIC;
					itok.post=dEBX|dEDI|dESI;	//05.09.04 01:36
					strcpy((char *)string,param);
					itok.type=tp_ucnovn;
					if(newdll->num==0)listapi=(APIPROC *)MALLOC(sizeof(APIPROC));	//первая в списке
					else listapi=(APIPROC *)REALLOC(listapi,sizeof(APIPROC)*(newdll->num+1));
					(listapi+newdll->num)->recapi=addtotree(itok.name);
					if(tok2==tk_openbracket){
						next=0;
						nexttok();
						IsUses((listapi+newdll->num)->recapi);
					}
					newdll->num++;
				}
				else warningdefined(hitok.name);
				if(next)nexttok();
				seminext();
			}
		}while(tok!=tk_closebrace);
		returntype=tokens;
		newdll->list=listapi;
		nexttok();
	}
	else{
		itok.flag=f_extern;
		// { Added by Coldy 
		// extern is dynamical variable
		// This also suppress warning if variable is unused
		dynamic_flag = TRUE;
		// }
		switch(tok){
			case tk_far:
			case tk_cdecl:
			case tk_pascal:
			case tk_stdcall:
			case tk_fastcall:
			case tk_declare:
			case tk_undefproc:
			case tk_ID:
			case tk_id:
			case tk_float:
			case tk_long:
			case tk_dword:
			case tk_word:
			case tk_byte:
			case tk_char:
			case tk_int:
			case tk_void:
				int j;
				if((j=testInitVar())==FALSE)define_procedure();
				else if(j==TRUE)globalvar();
				break;
			case tk_struct: InitStruct(); break;
			default: preerror("Error declare extern");
		}
		if((!fobj)&&(!sobj))preerror("extern using only for compilation obj files");
	}
}

int testInitVar(int checkaldef)
{
unsigned int ffar=0;
unsigned int fexport=0;
unsigned int tproc=0;
unsigned int fretproc=0;
int rettype=tokens;
unsigned int flag=itok.flag;
unsigned int npointr=0;
	if(fstatic){
		flag|=f_static;
		fstatic=FALSE;
	}
	if(tok==tk_inline){
		flag=f_inline;
		nexttok();
	}
	for(;;){
		if(tok==tk_far)ffar=f_far;
		else if(tok==tk_export)fexport=f_export;
		else if(tok>=tk_pascal&&tok<=tk_fastcall)tproc=tok;
		else if((tok>=tk_void&&tok<=tk_double)||tok==tk_fpust){
			if(rettype!=tokens)unknowntype();
			rettype=tok;
		}
		else if(tok==tk_id){
			if(CheckDef())continue;
			if(tok2==tk_dblcolon)goto classdecl;
			if(tproc==0)tproc=(comfile==file_w32?tp_stdcall:tp_pascal);	//тип проц по умолчанию
			else tproc=(tproc-tk_pascal)*2;
			break;
		}
		else if(tok==tk_ID){
			if(CheckDef())continue;
			if(tok2==tk_dblcolon){
classdecl:
				itok.flag=(unsigned int)(flag|ffar|fexport|fretproc|f_classproc);
				itok.rm=rettype;
				itok.npointr=(unsigned short)npointr;
				doclassproc(tproc);
				return 2;
			}
			if(tproc==0)tproc=tp_fastcall;	//тип проц по умолчанию
			else tproc=(tproc-tk_pascal)*2;
			break;
		}
		else if(tok==tk_undefproc||tok==tk_declare/*||tok==tk_apiproc*/){

			flag|=itok.flag;	//new 18.04.07 12:19

			if(tproc==0){
				if(CidOrID()==tk_ID)tproc=tp_fastcall;
				else tproc=(comfile==file_w32?tp_stdcall:tp_pascal);
			}
			else tproc=(tproc-tk_pascal)*2; //   17.09.05 17:06
			if((flag&f_extern)!=0||tproc!=(itok.flag&f_typeproc)||
				ffar!=(itok.flag&f_far)||(unsigned short)npointr!=itok.npointr||
				((flag&f_static)&&(itok.flag&f_static)==0)){
				if(strcmp(itok.name,"main")){
					if(rettype==itok.rm||(rettype==tokens&&(itok.rm==(am32==0?tk_word:tk_dword))))break;
//					printf("rm=%d rmnew=%d\n",itok.rm,rettype);
					if(checkaldef)redeclare(itok.name);
				}
			}
			break;
		}
		else if(tok==tk_proc||tok==tk_apiproc){
			if(checkaldef)idalreadydefined();
			break;
		}
		else if((tok>=tk_bits&&tok<=tk_doublevar)||tok==tk_structvar||tok==tk_pointer){
			idalreadydefined();
			return 2;
		}
		else if(tok==tk_mult){
			do{
				npointr++;
				nexttok();
			}while(tok==tk_mult);
			if(rettype==tokens)rettype=am32==FALSE?tk_word:tk_dword;
			continue;
		}
		else if(tok==tk_openbracket){
			if(tok2!=tk_mult){
				unuseableinput();
				return 2;
			}
			if(tproc!=0)tproc=(tproc-tk_pascal)*2;
			itok.flag=(unsigned int)(flag|ffar|tproc|fexport|fretproc);
			itok.rm=rettype;
			itok.npointr=(unsigned short)npointr;
			return TRUE;
		}
		else if((tok>=tk_overflowflag&&tok<=tk_notzeroflag)||tok==tk_minusflag||
				tok==tk_plusflag)fretproc=(tok-tk_overflowflag+1)*256;
		else if(tok==tk_static)flag|=f_static;
		else if(tok==tk_fpust)rettype=tk_fpust;
		else{
			unuseableinput();
			return 2;
		}
		nexttok();
	}
	itok.flag=(unsigned int)(flag|ffar|tproc|fexport|fretproc);
	itok.rm=rettype;
	itok.npointr=(unsigned short)npointr;
	if(returntype==tk_declare&&
			(tok2==tk_openbracket||tok2==tk_at||tok2==tk_period))return FALSE;
	if(tok2==tk_openbracket)return CheckDeclareProc();
	if(rettype==tokens){
		thisundefined(itok.name);//02.09.04 20:55 was unuseableinput();
		return 2;
	}
	return TRUE;
/*-----------------23.12.01 02:11-------------------
 rerurn:
 FALSE - если определение, вставка процедуры
 TRUE  - переменная или объявление процедуры
 2 - ошибка или обработано - никаких действий не предпринимать.
	--------------------------------------------------*/
}

int CidOrID()
{
unsigned char cha;
unsigned char *string4=(unsigned char *)itok.name;
	for(;;){
		cha=*string4;
		if(cha>='a'&&cha<='z')return tk_id;
		if(cha==0)break;
		string4++;
	}
	return tk_ID;
}

void unpackteg(structteg *tteg)
{
int i;
elementteg *bazael;
structteg *newteg;
int ssize=0,count;
idrec *newrec,*ptr;
	if(alignword){	//выровнять на четный адрес
		if(am32==0){
			if(postsize%2==1)postsize++;
		}
		else if(ssize==4&&postsize%4!=0)postsize+=4-(postsize%4);
	}
	bazael=tteg->baza;
	string[0]=0;
	for(i=0;i<tteg->numoper;i++){
//		printf("tok=%d %s\n",(bazael+i)->tok,(bazael+i)->name);
		switch((bazael+i)->tok){
			case tk_floatvar:
			case tk_longvar:
			case tk_dwordvar:
			case tk_wordvar:
			case tk_bytevar:
			case tk_charvar:
			case tk_intvar:
			case tk_doublevar:
			case tk_qwordvar:
				ssize=GetVarSize((bazael+i)->tok);	//размер переменной
				itok.type=tp_ucnovn;
				tok=(bazael+i)->tok;
				count=FindOff((unsigned char *)(bazael+i)->name,VARPOST);
				itok.post=1;
				itok.segm=DS;
				itok.number=postsize;
				itok.flag=tteg->flag;
				itok.size=(bazael+i)->numel*ssize;
				itok.rm=(am32==FALSE?rm_d16:rm_d32);
				itok.npointr=0;
				newrec=addtotree((bazael+i)->name);
				newrec->count=count;
				break;
			case tk_struct:
			case tk_structvar:
				strcpy(itok.name,(bazael+i)->name);
				newteg=(structteg *)(bazael+i)->nteg;
				newrec=(struct idrec *)MALLOC(sizeof(struct idrec));
				ptr=((tteg->flag&f_static)==0?treestart:staticlist);	//начало дерева
				if(ptr==NULL)((tteg->flag&f_static)==0?treestart:staticlist)=newrec;//начало дерева
				else{	//поиск строки в дереве
					while(((ssize=strcmp(ptr->recid,itok.name))<0&&ptr->left!=NULL)||(ssize>0&&ptr->right!=NULL)){
						ptr=(ssize<0?ptr->left:ptr->right);
					}
					(ssize<0?ptr->left:ptr->right)=newrec;	//строка меньше
				}
				newrec->recsib=0;
				strcpy(newrec->recid,itok.name);//скопир название
				newrec->newid=(char *)newteg;
				newrec->left=NULL;
				newrec->right=NULL;
				newrec->rectok=tk_structvar;
				newrec->flag=tteg->flag|newteg->flag;
				newrec->line=linenumber;
				newrec->file=currentfileinfo;
				if(FixUp)newrec->flag|=f_reloc;
				newrec->recrm=(bazael+i)->numel;
				newrec->recsize=(bazael+i)->numel*newteg->size;
				newrec->recpost=1;
				count=FindOff((unsigned char *)newrec->recid,VARPOST);
				newrec->count=count;
				break;
			default:
				declareanonim();
				break;
		}
	}
	AddPostData(tteg->size);
}

void unpackteg2(structteg *tteg)
{
int i;
elementteg *bazael;
structteg *newteg;
//idrec *newrec,*trec;
localrec *lrec;
	bazael=tteg->baza;
	string[0]=0;
	for(i=0;i<tteg->numoper;i++){
		switch((bazael+i)->tok){
			case tk_floatvar:
			case tk_longvar:
			case tk_dwordvar:
			case tk_wordvar:
			case tk_bytevar:
			case tk_charvar:
			case tk_intvar:
			case tk_doublevar:
			case tk_qwordvar:
				lrec=addlocalvar((bazael+i)->name,(bazael+i)->tok,localsize);
				lrec->rec.recsize=(bazael+i)->numel*GetVarSize((bazael+i)->tok);
				lrec->rec.type=tp_localvar;
				lrec->rec.npointr=0;
				lrec->rec.recnumber=-lrec->rec.recnumber-Align(lrec->rec.recsize,(am32==FALSE?2:4));
				break;
			case tk_struct:
			case tk_structvar:
				newteg=(structteg *)(bazael+i)->nteg;
				lrec=addlocalvar((bazael+i)->name,tk_structvar,localsize);
				lrec->rec.newid=(char *)tteg;
				lrec->rec.flag=tteg->flag;
				lrec->rec.type=tp_localvar;
				lrec->rec.recrm=(bazael+i)->numel;
				lrec->rec.recsize=(bazael+i)->numel*newteg->size;
				lrec->rec.recpost=LOCAL;
				lrec->rec.recnumber=-lrec->rec.recnumber-Align(lrec->rec.recsize,(am32==FALSE?2:4));
				break;
			default:
				declareanonim();
				break;
		}
	}
	localsize+=tteg->size;
	localsize=Align(localsize,(am32==FALSE?2:4));
}

unsigned long dounion(int Global,int flag)
{
structteg *tteg;
int noname=FALSE;
	nexttok();
	if(tok==tk_openbrace)noname=TRUE;
	else if((tok!=tk_id&&tok!=tk_ID)||FindTeg(Global)||(Global==FALSE&&FindTeg(TRUE))){
		idalreadydefined();
		SkipBlock2();
		return 0;
	}
	if((tteg=CreatTeg(Global,TRUE,noname))!=NULL){
		if(tok==tk_semicolon){
			if(noname==TRUE){
				if(Global)unpackteg(tteg);
				else unpackteg2(tteg);
				if(tteg->baza)free(tteg->baza);
				free(tteg);
			}
			nexttok();
		}
		else{
			if(Global)InitStruct2(flag,tteg);
			else return LocalStruct2(flag,0,0,0,tteg);
		}
	}
	else declareunion();
	return 0;
}

char *BackString(char *str)
{
	char *retbuf=(char *)MALLOC(strlen(str)+1);
	strcpy(retbuf,str);
	return retbuf;
}


void GetFileTime(int fd,struct ftime *buf)
{
/*
struct stat sb;
struct tm *tblock;
	fstat(fd,&sb);
	tblock=localtime(&sb.st_atime);
	buf->ft_tsec=tblock->tm_sec;
	buf->ft_min=tblock->tm_min;
	buf->ft_hour=tblock->tm_hour;
	buf->ft_day=tblock->tm_mday;
	buf->ft_month=tblock->tm_mon;
	buf->ft_year=tblock->tm_year-80;*/
}

void CheckPosts()
{
	if(posts==maxposts){
		maxposts+=MAXPOSTS;
		postbuf=(postinfo *)REALLOC(postbuf,maxposts*sizeof(postinfo));
	}
}

void CheckRealizable()
{
	switch(tok){
		case tk_case:
		case tk_CASE:
		case tk_default:
		case tk_closebrace:
			return;
	}
	if(tok2==tk_colon)return;
	if(notunreach){
		notunreach=FALSE;
		return;
	}
	warningunreach();
//	preerror("Unreachable code");
}

void AddRegistr(int razr,int reg)
{
	if(razr==r8&&reg>3)reg-=4;
	if(razr==r64){
		stat_reg[reg&255]=1;
		stat_reg[reg/256]=1;
	}
	else stat_reg[reg]=1;
}

void ClearRegister()
{
	for(int i=0;i<8;i++)stat_reg[i]=0;
}

int GetRegister(int mode)
{
int reg=SI;
	if(am32!=FALSE||mode){
		if(stat_reg[AX]==0)reg=AX;
		else if(stat_reg[SI]==0)reg=SI;
		else if(stat_reg[DI]==0)reg=DI;
		else if(stat_reg[BX]==0)reg=BX;
		else if(stat_reg[CX]==0)reg=CX;
		else if(stat_reg[DX]==0)reg=DX;
	}
	else{
		if(stat_reg[SI]==0)reg=SI;
		else if(stat_reg[DI]==0)reg=DI;
		else if(stat_reg[BX]==0)reg=BX;
	}
	return reg;
}

void RegAddNum(int reg)
{
	if((!structadr.post)&&optnumadd(structadr.number,reg,am32==FALSE?r16:r32,0)!=FALSE)return;
	if(reg==AX)op(5);
	else{
		op(0x81);
		op(0xC0+reg);
	}
	if(structadr.post)setwordpost(&structadr);
	if(am32)outdword(structadr.number);
	else outword(structadr.number);
}

void RestoreStack()
{
	if(addstack&&sizestack){
		if(short_ok(sizestack,am32)){
			outword(0xC483);
			op(sizestack);
		}
		else{
			outword(0xC481);
			if(am32==FALSE)outword(sizestack);
			else outdword(sizestack);
		}
//		printf("%s(%d)> Restore %d bytes stacks.\n",startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename,linenumber,sizestack);
		addESP-=sizestack;
		sizestack=0;
	}
}

void startblock()
{
treelocalrec *nrec;
	numblocks++;
//	printf("start block %d\n",numblocks);
	nrec=(treelocalrec*)MALLOC(sizeof(treelocalrec));
	nrec->level=numblocks;
	nrec->lrec=NULL;
	nrec->addesp=addESP;
	nrec->next=tlr;
	tlr=nrec;
}

void endblock()
{
treelocalrec *orec;
//	printf("end block %d\n",numblocks);
	orec=tlr;
	tlr=tlr->next;
	if(tlr)numblocks=tlr->level;
	else numblocks=0;
	if(orec->lrec==NULL){
		free(orec);
		return;
	}
	orec->endline=linenumber;
	orec->next=btlr;
	btlr=orec;
//	if(addESP!=orec->addesp)???
}
/* end of TOKC.C */
