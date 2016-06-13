//void PrintMem(void *mem);
//void CheckMem();

#include "port.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


//#define DEBUGMODE

#define OPTVARCONST // Замена переменных сонстантами

#define MAXNUMPATH 16
#define DATATYPES 9 	// number of data types
#define MAXDATA 65500 /* output run file buffer 65500 bytes */

#define DYNAMIC_USED 0
#define DYNAMIC      1
#define NOT_DYNAMIC 2 /* flag value specifing a non-dynamic proc */
#define DYNAMIC_SET 3

#define FILENAMESIZE 80
#define SHORTMAX	127 // largest 8 bit signed value
#define SHORTMIN -128 // smallest 8 bit signed value
#define IDLENGTH 65  /* length of significance of IDS + NULL, therefore 32 */
#define STRLEN 2048  /* length of string token holder */
#define MAXPOSTS 2048
#define SIZEBUF 16384
#define ID2S 6*DATATYPES
#define MAXSYSCOM 25
#define NUMNUM 32
#define NUMNUM64 64
#define OBJECTALIGN 4096	//выравнивание секций в памяти
#define WARNCOUNT 15	//число различных предупреждений

#define ver1 0
#define ver2 239

#define BETTA

#ifdef BETTA
#define betta " b26"
#else
#define betta ""
#endif

#include "const.h"
#include "struct.h"

#ifdef _UNIX_
	#ifndef stricmp
		#define stricmp strcasecmp
	#endif
#endif

#ifdef _UNIX_
	#ifndef O_BINARY
		#define O_BINARY 0
	#endif
#endif

extern unsigned char FixUpTable;	//запретить создание таблици Fix UP for Windows
extern unsigned char WinMonoBlock;
extern unsigned int currentfileinfo;

#if !defined (_MAIN_)
extern char fobj;	//признак генерации obj
extern char *rawfilename;
extern struct tm timeptr;
extern char outext[];
extern unsigned char compilerstr[];  /* compiler ID string */
extern LISTCOM *listcom;
extern unsigned char gwarning;
extern EWAR wartype,errfile;
extern int numfindpath;
extern char *findpath[];
extern char bufpath[];
extern unsigned int  startptr;
extern unsigned char wconsole;//признак генерации консольного приложения windows
extern unsigned long ImageBase;
extern int numexport;
extern struct listexport *lexport;
extern unsigned char optstr;	//оптимизация строковых констант
extern unsigned char crif;	//check reply include file
extern unsigned char idasm;	//ассемблерные инструкции считать идентификаторами
extern char modelmem;
extern char *stubfile;
extern char comsymbios;
extern unsigned char sobj;
extern short dllflag;
extern char *bufstr;	//буфер для строк из процедур
extern int sbufstr;	//начальный размер этого буфера
extern unsigned char wbss;	//пост переменные в отдельную секцию
extern int numrel;	//число элементов в таблице перемещений
extern unsigned char usestub;
extern char *winstub;
extern unsigned char dpmistub;
extern unsigned char useordinal;
extern int startupfile;
extern int alignproc,aligncycle;
extern FILE *hout;
extern unsigned char useDOS4GW;
extern unsigned char use_env;	//переменная окружения
extern unsigned char clearpost;
extern unsigned char uselea;
extern unsigned char regoverstack;
extern unsigned char shortimport;
extern char *namestartupfile;
extern unsigned char useinline;
extern unsigned char ocoff;
extern unsigned char ESPloc;

#endif

extern unsigned char string[STRLEN],string2[STRLEN+20];

#if !defined (_TOKC_)
extern unsigned int outptrsize;	//размер выходного буфера
extern unsigned char string3[STRLEN];
extern char *BackTextBlock;	//буфер для перенесенного текста
extern int SizeBackBuf;
extern unsigned char cha;
extern unsigned int inptr;
extern unsigned char AlignProc;
extern unsigned int secondcallnum;	//# of different second calls and labels
extern unsigned char header;
extern unsigned char killctrlc;
extern unsigned char optimizespeed;
extern unsigned char alignword;
extern unsigned int outptr,outptrdata;
extern unsigned char comfile;
extern unsigned char chip;
extern unsigned char dos1,dos2;
extern unsigned int stacksize;
extern int error;
extern unsigned char *output,*outputdata;
extern unsigned char notdoneprestuff;
extern unsigned long postsize;
extern unsigned int posts;
extern postinfo *postbuf;
extern int maxposts;
extern unsigned char cpu;
extern long runfilesize;
extern unsigned char *input; 	 /* dynamic input buffer */
extern unsigned int endinptr;		 /* end index of input array */
extern unsigned int localsize,paramsize;
extern char endoffile;
extern unsigned int current_proc_type; 	/* current procedure type */
extern unsigned char aligner;
extern unsigned int alignersize;
extern unsigned int datasize;
extern unsigned char warning;
extern unsigned int startStartup;
extern unsigned char useStartup;
extern unsigned int endStartup;
extern unsigned char notpost;
extern unsigned char am32; 		      // режим 32 битной адресации
extern unsigned int externnum;
extern unsigned char FastCallApi;	//разрешить быстрый вызов API процедур
extern unsigned char FixUp;	//Делать ли таблицу перемещений
extern void *liststring;	//цепочка информационных блоков о строках
extern struct FILEINFO *startfileinfo;
extern unsigned int totalmodule;
extern int retproc;
extern unsigned char splitdata;	//отделить данные от кода
extern unsigned char AlignCycle;       //выравнивать начала циклов
extern char param[];	//буфер для параметров процедуры
extern unsigned char dynamic_flag;	//флаг обработки динамических элементов
extern unsigned char setzeroflag;	//операция меняет zero flag
extern unsigned char insertmode;
extern unsigned int numblocks;	//номер вложенного блока
extern unsigned char notunreach;
extern idrec *staticlist;
extern unsigned int procedure_start; /* address of start of procedure */
extern int lastcommand;	//последний оператор в блоке
extern unsigned int initBP;
extern unsigned char fstatic;
//extern int sizestack;	//размер не компенсированных параметров функций
extern unsigned char addstack;
extern unsigned long addESP;	//добавка стека
extern unsigned char blockproc;	//идетразборка блока функции
extern treelocalrec *tlr;	//цепочка локальных блоков
extern treelocalrec *btlr;	//цепочка использованых локальных блоков
extern int returntype; 				 /* return type, (void, byte, word, ...) */
#endif
extern int tok,tok2;

#if !defined (_TOKR_)
extern char useasm;
extern short ofsmnem[];
extern unsigned char asmMnem[];
extern char asmparam;
#endif

#if !defined (_TOKA_)
extern unsigned char id[];
extern short idofs[];
extern char id2[ID2S][9];
extern char regs[2][8][4];
extern char begs[8][3];
extern char segs[8][3];
extern struct idrec *treestart;
extern unsigned int linenum2;
extern unsigned int inptr2;
extern unsigned char cha2;
extern char displaytokerrors;
extern char *bufrm;
extern UNDEFOFF *undefoffstart;
extern DLLLIST *listdll;
extern char skipfind;
extern struct structteg *tegtree;			//глобальный срисок тегов
extern struct structteg *ltegtree;		//локальный срисок тегов
//extern struct idrec *lstructlist; //список локальных структур
extern struct idrec *definestart;
extern SINFO strinf;
extern char *startline, *endinput;
extern ITOK structadr;
extern int scanlexmode;
extern COM_MOD *cur_mod;
extern unsigned char bytesize;
#endif

#if !defined (_TOKE_)
extern int maxerrors;
extern char mesmain[];
extern int endifcount;
extern int sysstack;
extern int sysnumcom;
extern int syscom;
extern unsigned char fargc;
extern unsigned char jumptomain;
extern unsigned int  startptrdata;
extern unsigned char parsecommandline;
extern unsigned int romsize;
extern unsigned char resizemem;
extern unsigned int resizesizeaddress;
extern unsigned int stackstartaddress;
extern int dataromstart,dataromsize;
extern unsigned char startuptomain;
extern unsigned char dosstring;
extern unsigned int numdomain;	//число процедур запускаемых до main
extern char *domain;	//буфер имен процедур запускаемых до main
extern unsigned char usedirectiv;	//идет обработка директивы
extern unsigned char atex;
extern unsigned int postnumflag;	//флаг последнего идентификатора в вычислении номера
extern unsigned char sdp_mode;	//режим принудительной выгрузки динамических процедур
extern int calcnumber;
extern int strpackdef;
extern int strpackcur;
#ifdef DEBUGMODE
extern int debug;
#endif
extern int dirmode;

#endif

extern ITOK itok,itok2,ptok;

#if !defined (_TOKB_)
extern int divexpand;
extern int optnumber;
extern char *badadr;
extern LISTFLOAT *floatnum;	//список float констант
extern unsigned int numfloatconst;
extern unsigned int ofsfloatlist;
#endif
extern unsigned int linenumber;

#if !defined (_DEBUG_)
extern unsigned int ooutptr;
extern unsigned int outputcodestart;
extern unsigned char dbg,dbgact;
extern unsigned int pdbg;      // number of post entrys
#endif

#if !defined (_OUTPE_)
extern unsigned long ImageBase;
extern unsigned long vsizeheader; //виртуальный размер заголовка.
extern int filingzerope;
extern unsigned long FILEALIGN;	// выравнивание секций в файле
#endif

#if !defined (_ERRORS_)
extern WARNACT wact[];
extern unsigned char mapfile;
#endif

#if !defined (_DISASM_)
extern unsigned char seg_size;   /* default size is 16 */
extern unsigned long instruction_offset;
#endif

#if !defined (_SWITCH_)
extern FSWI *swtables;
extern int numswtable;
#endif

#if !defined (_CLASS_)
extern structteg *searchteg;
extern int destructor;
#endif

#if !defined (_RES_)
extern unsigned char *resbuf;
extern unsigned int curposbuf;
extern int numres;	//текущее число ресурсов
extern int numstrtbl;
#endif

extern unsigned char idxregs[5];

// start of procedure pre-definitions
/*-----------------19.09.98 17:18-------------------
	 Функции определенные в main.cpp
--------------------------------------------------*/
void *MALLOC(unsigned long size);
void *REALLOC(void *block,unsigned long size);
void IncludePath(char *buf);
int SelectComand(char *pptr,int *count);
void strbtrim(char *st);
unsigned long  Align(unsigned long size,unsigned long val);
int AlignCD(char segm,int val);	//выравнять данные или код
void ErrOpenFile(char *str);
unsigned int EntryPoint();
long CopyFile(FILE *in,FILE *out);
unsigned long getnumber(unsigned char *buf);
void addinitvar();
FILE *CreateOutPut(char *ext,char *mode);
void SetLST(unsigned char neg);
void AddUndefClassProc();
int MakeCoff();
void setdindata(idrec *ptr,int i);

/*-----------------08.03.98 20:10-------------------
 Функции определеные в toka.c
--------------------------------------------------*/
void CheckAllMassiv(char *&buf,int sizeel,SINFO *strc,ITOK *ctok=&itok,int reg1=idxregs[0],int reg2=idxregs[1]);
void docalls(); //attempt to declare undefs from library and dynamic proc's
int FindOff(unsigned char *name,int base);
void nextchar();	//опр в toke
void nexttok();
void whitespace(); //пропуск нзначащих символов
int searchtree(ITOK *itk4,int *tk4,unsigned char *strin4);
void AddUndefOff(int segm,char *ostring);
void InitStruct();	//инициализировать структуру
unsigned long LocalStruct(int flag,int *localline);	//инициализировать локальную структуру
struct structteg * FindTeg(int Global,char *name=itok.name);	//найти тег
void dostruct();
int FastSearch(unsigned char *list,short *ofs,int type,char *str);
void FindDirectiv();
unsigned long long scannumber(int *rm);
void FastTok(int mode,int *tok4=&tok,ITOK *itok4=&itok);
unsigned int initstructvar(structteg *tteg,int numel);
unsigned int ScanTok3();
int GetVarSize(int var);
void compressoffset(ITOK *thetok);
void AddDynamicList(idrec *ptr);
int CheckUseAsUndef(unsigned char *name);
int CheckMassiv(char *&buf,int sizeel,int treg,int *idx=0,int *base=0,long *num=0);
void AutoDestructor();
void dodelete();
void donew();
void RunNew(int size);
int CallDestructor(structteg *searcht);
int FindUseName(char *name);	//поиск ссылок на текущее имя
void DateToStr(char *buf);
int CalcRm16(int base,int idx);
int CheckDef();
void SetNewStr(char *name);
struct structteg *CreatTeg(int Global,int useunion=FALSE,int noname=FALSE);
void InitStruct2(unsigned int flag,structteg *tteg);
unsigned long LocalStruct2(int flag,int *localline,int binptr,char bcha,structteg *tteg);
void retoldscanmode(int mode);
void ExpandRm(int rm,int sib,int *zoom,int *base,int *idx);
void BackMod();

/*-----------------08.03.98 21:45-------------------
 Функции определеные в tokb.h
--------------------------------------------------*/
void AddReloc(int segm=itok.segm);
int doalmath(int sign,char **ofsstr);
int do_e_axmath(int sign,int razr,char **ofsstr);
void getintoal(int gtok,ITOK *gstok,char *&gbuf,SINFO *gstr); /* AH may also be changed */
void getinto_e_ax(int sign,int gtok,ITOK *gstok,char *&gbuf,SINFO *gstr,int razr,int useAX=FALSE);
int  doeaxfloatmath(int itreturn=tk_reg32,int reg=AX,int addop=0);
int dobytevar(int sign,int terminater=tk_semicolon); 	// byte, char
int do_d_wordvar(int sign,int razr,int terminater=tk_semicolon); 		 /* signed or unsigned 16 bit memory variable */
int doreg_32(int reg,int razr,int terminater=tk_semicolon);
void doseg(int seg);
int caselong(unsigned long val);
int dobeg(int beg,int terminater=tk_semicolon);
void dobegmath(int beg);	/* math done is on all begs except AL */
void doregmath_32(int reg,int razr,int sign,char **ofsstr,int i=0);	/* math done is on all regs except AX */
int getintobeg(int beg,char **ofsstr);
int getintoreg_32(int reg,int razr,int sign,char **ofsstr,int useloop=TRUE); /* get into word reg (except AX) with enum */
void outaddress(ITOK *outtok);
void FloatToNumer(int addop=0);
int dofloatvar(int addop=0,int retrez=tk_floatvar,int terminater=tk_semicolon);
void fwait3();
void AddFloatConst(long long fnumber,int type=tk_float);
void setwordpost(ITOK *); 					 /* for post word num setting */
void setwordext(long *id);
void RegMulNum(int reg,unsigned long num,int razr,int sign,int *expand,int flag);
int OnlyNumber(int sign);
void PopSeg(int seg);
void PushSeg(int seg);
void MovRegNum(int razr,int relocf,unsigned long number,int reg);
int CheckMinusNum();
int getintoreg(int reg,int razr,int sign,char **ofsstr);
void dobits();
void bits2reg(int reg,int razr);
void getoperand(int reg=BX);
int optnumadd(unsigned long num,int reg,int razr,int vop);
int MultiAssign(int razr,int usereg,int npointr=0);
void CheckInitBP();
void RestoreBP();
void fistp_stack(int addop=0);
void fld_stack(int size);
void fildq_stack();
void cpointr(int reg,int numpointr);
int doqwordvar(int terminater=tk_semicolon);	//64 bit memory variable
void doreg64(int reg,int terminater=tk_semicolon);
void doregmath64(int reg);
void getintoreg64(int reg);
void float2stack(int num);
void dofloatstack(int num);

/*-----------------08.03.98 20:59-------------------
 Функции определеные в tokc.c
--------------------------------------------------*/
localrec * addlocalvar(char *str,int tok,unsigned int num,int addmain=FALSE);
int addpoststring(int segm=CS,int len=itok.number, int term=itok.flag); 	 /* add a string to the post queue */
void define_locallabel();
unsigned int dofrom();
unsigned int doextract();
int doparams();			/* do stack procedure parameter pushing */
int swapparam();
long updatetree();
void addacall(unsigned int idnum,unsigned char callkind);
idrec * addtotree(char *keystring);
void compilefile(char *filename,int firstflag);
void convert_returnvalue(int expectedreturn,int actualreturn);
int doid (char uppercase,int expectedreturn);
void insert_dynamic(int insert=FALSE);
int macros(int expectedreturn);
void op(int byte);
void opd(int byte);	//вывод байта в сегмент данных
void op66(int ctoc);
int op67(int ctok);
void outqwordd (unsigned long long);
void outqword (unsigned long long);
void outdwordd (unsigned long);
void outdword (unsigned long);
void outwordd (unsigned int);
void outword (unsigned int);
void outseg(ITOK *outtok,unsigned int locadd);
int procdo(int expectedreturn);
int updatecall(unsigned int which,unsigned int where,unsigned int top);
void AddBackBuf(int,char);
void CharToBackBuf(char c);
void missingpar(char *name="");
int CheckCodeSize();
void CheckPosts();
int doanyundefproc(int jumpsend=FALSE);
int doanyproc(int jumpsend=FALSE);
void killpost(unsigned int poz);
char *BackString(char *str);
DLLLIST *FindDLL();
long initglobalvar(int type,long elements,long ssize,char typev);
int typesize(int vartype);
void dopoststrings();
char *dynamic_var();
void uptdbr(/*int usesw=FALSE*/);
void docommand(); 		/* do a single command */
//int CheckStopBlock();
void MakeBreak(unsigned char typeb);
void SetBreakLabel();
void SetContinueLabel();
void CheckDir();
int SkipParam();
long GetBitMask(int ofs,int size);
void AddPostData(unsigned int loop);
//void NotPostUnion();
unsigned int initparamproc();
void CorrectStack(unsigned int num);
int testInitVar(int checkaldef=TRUE);
void declareparamreg();
void declareparamstack(); /* declare procedure parameters */
void declareparams(); 		 /* declare procedure parameters */
void declarelocals(int mode,int finline=FALSE); 		 /* declare locals */
void doblock2();
void doblock();
void setreturn();
int CidOrID();
void dynamic_proc();
void setproc(int defflag);
void define_procedure();
void doregparams();
int CheckDeclareProc();
int loadfile(char *filename,int firstflag);
void RestoreStack();
void IsUses(idrec *rec);
int SkipBlock();
void declareanonim();
void declareunion();
void startblock();
void endblock();
void LLabel();
void AddApiToPost(unsigned int num);

/*-----------------08.03.98 20:06-------------------
 функции определены в toke.c
--------------------------------------------------*/
void jumperror(unsigned int line,char *type);
void beep();			 /* beep for any internal errors */
void codeexpected();
void datatype_expected(int type=0);
unsigned long doconstdwordmath();
signed long doconstlongmath();
long doconstfloatmath();
void dwordvalexpected();
void idalreadydefined();
void illegalfloat();
void internalerror (char *str);// serious internal compiler error message
void maxoutputerror();
void maxwordpostserror();
void nextseminext();
void numexpected(int type=0);
void operatorexpected();
void seminext();
void shortjumptoolarge();
void stringexpected();
void swaperror();
void SwTok(int want);
void unabletoopen(char *str);
void unexpectedeof();
void unknowncompop();
void valueexpected();
void varexpected(int type);
void wordvalexpected();
int includeit(int type);
int includeproc();
int CheckMacros();
void tobigpost();
void expected (char ch);
int expecting(int want);
void outprocedure(unsigned char *array,unsigned int length);
void preerror(char *str);//error on currentline with line number and file name
void thisundefined(char *str,int next=TRUE);
void addconsttotree(char *keystring,long long constvalue,int type=tk_dword);
void directive();
void doenum();
void doprestuff(); //do initial code things, like resize mem, jump to main...
void searchvar(char *name,int err=1);
void expectingoperand(int want);
void InitDefineConst();
unsigned long long doconstqwordmath();
long long doconstdoublemath();
int calclongnumber(long *retnum,long number,int operand);
int calcqwordnumber(unsigned long long *retnum,unsigned long long number,int operand);
int calcdwordnumber(unsigned long *retnum,unsigned long number,int operand);
int calcfloatnumber(float *retnum,float number,int operand);
int calcdoublenumber(double *retnum,double number,int operand);

/*-----------------08.03.98 22:24-------------------
 Функции определеные в tokr.c
--------------------------------------------------*/
void doasm(int next=FALSE);
int short_ok(long thenumber,int reg32=FALSE);
void callloc(long loc); 	/* produce CALL # */
void callloc0();
void cbw(void);
int GOTO();
void expecting2(int want);
unsigned char gotol(int faradd);
void jumploc(long loc); 		/* produce JUMP # */
void jumploc0();
void movsb(void);
void movsd(void);
void movsw(void);
void popes(); 	/* produce POP ES */
void pushds();	/* produce PUSH DS */
void pushss();
void ret(); 	 /* produce RET */
void retf();	 /* produce RETF */
void stosb(void);
void stosd(void);
void stosw(void);
void fwait();
void xorAHAH(void); 	/* produce XOR AH,AH */
void xorAXAX(void); 	/* produce XOR AX,AX */
void xorEAXEAX(void); 	/* produce XOR EAX,EAX */
void nextexpecting2(int want);
void tobedefined(int callkind,int expectedreturn);
void cwdq(int razr);
void Leave();
void CheckIP();
int iTest(int mode=0);
void ZeroReg(int reg, int razr);
int Push(ITOK *wtok=NULL);

/*-----------------08.08.00 22:50-------------------
 errors.cpp
	--------------------------------------------------*/
void warningstring();
void warningexpand();
void warningjmp(char *str2,int line=linenumber,int file=currentfileinfo);
void warningreg(char *str2);
void preerror3(char *str,unsigned int line,unsigned int file=currentfileinfo);// error message at a different than current line
void unableopenfile(char *name);
void tegnotfound();
void errstruct();
void  warningdefined(char *);
void extraparam(char *name="");
void warningretsign();
void ErrWrite();
void ErrReadStub();
void warningnotused(char *name,int type);
void regBXDISIBPexpected();
void reg32expected(int type=0);
void InvOperComp();
void warningusenotintvar(char *name);
void warningdestroyflags();
void warningunreach();
void unuseableinput();
void redeclare(char *name);
void undefclass(char *name);
void badinfile(char *name);
void errorreadingfile(char *name);
void expectederror(char *str);
void unknowntype();
void unknownstruct (char *name,char *sname);
void unknowntagstruct (char *name);
void warninline();
void ZeroMassiv();
void bytevalexpected(int type);
void FindStopTok();
//void not_union_static();
extern void edpip(int num=0);
void waralreadinit(char *reg);
void waralreadinitreg(char *reg,char *reg2);
void OnlyComFile();
void warnsize();
void destrdestreg();
void qwordvalexpected();
void fpustdestroed();
void unknownobj(char *name);
void FindEndLex();
void fpu0expected();
void unknownpragma(char *name);
void warpragmapackpop();
void SkipBlock2();
void warreplasevar(char *name);
void warcompneqconst();
void warcompeqconst();
void warpointerstruct();
void warESP();
void waralreadinitvar(char *name,unsigned int num);
void warningprint(char *str,unsigned int line,unsigned int file);
void notexternfun();

void AddDataLine(char ssize/*,char typev*/);
void mapfun(int);

/*-----------------24.01.01 01:42-------------------
 disasm.cpp
	--------------------------------------------------*/
void undata(unsigned ofs,unsigned long len,unsigned int type);
void unassemble(unsigned long ofs);

/*-----------------25.01.01 23:02-------------------
 debug.cpp
	--------------------------------------------------*/
void AddLine(int SkipLine=FALSE);
void DoTDS();
void InitDbg();
void KillLastLine();
void AddDataNullLine(char ssize/* new!!! */,char *name=NULL);
void AddCodeNullLine(char *name=NULL);
void AddEndLine();
#ifdef DEBUGMODE
void printdebuginfo();
#endif
//void AddMainLine();

/*-----------------12.04.01 22:46-------------------
 outpe
	--------------------------------------------------*/
void CreatStub(char *name);
void CreatWinStub();
void ChSize(long size);

/*-----------------12.04.01 22:57-------------------
 outle
	--------------------------------------------------*/
int MakeLE();
int MakeMEOS();
int MakeBin32();

/*-----------------08.12.01 23:43-------------------
 pointer
	--------------------------------------------------*/
void dopointer();
void dovalpointer();
void getpointeradr(ITOK *gstok,char *&gbuf,SINFO *gstr,int numpointer,int razr,int reg=BX);
void dopointerproc();

/*-----------------09.12.01 00:28-------------------
 new_type
	--------------------------------------------------*/
void convert_type(int *sign,int *rettype,int *pointr,int reg=BX);

/*-----------------23.12.01 02:39-------------------
 class
	--------------------------------------------------*/
void doclassproc(unsigned int);
void AddThis();

/*-----------------27.01.02 23:39-------------------
 res
	--------------------------------------------------*/
void input_res();
int MakeRes(unsigned long ofsres,LISTRELOC **listrel);
void CreatStrTabRes();

/*-----------------14.04.03 21:31-------------------
 optreg
--------------------------------------------------*/
void initregstat();
void deinitregstat();
void IDXToReg(char *name,int size,int reg);
int CheckIDXReg(char *name,int size,int reg);
void IDZToReg(char *name,int reg,int razr);
int CheckIDZReg(char *name,int reg,int razr);
void clearregstat(int regs=0);
void ConstToReg(unsigned long num,int reg,int razr);
void ClearReg(int reg);
int RegSwapReg(int reg1,int reg2,int razr);
char *GetLecsem(int stop1,int stop2=tk_eof,int type=-1);
REGISTERSTAT *BakRegStat();
void CopyRegStat(REGISTERSTAT *bak);
void KillVar(char *name);
void CompareRegStat(REGISTERSTAT *bak);
int RegToReg(int regd,int regs,int razr);
void GenRegToReg(int regd,int reds,int razr);
void GetEndLex(int stop1,int stop2=tk_eof,int type=-1);
int GetRegNumber(int reg,unsigned long *num,int razr);
int GetNumberR(int reg,unsigned long *num,int razr,unsigned long number);
void FreeStat(REGISTERSTAT *bak);
void AddRegVar(int reg, int razr,ITOK *itok4);
int GetRegVar(ITOK *itok4);
#ifdef OPTVARCONST
void CreateMainLVIC();
void KillMainLVIC();
void ClearLVIC();
int Const2Var(ITOK *itok4,long long num,int typenum);
void ClearVarByNum(ITOK *itok4);
int UpdVarConst(ITOK *itok4,long long num,int typenum,int operand);
void FreeGlobalConst();
int CheckRegToConst(int reg,ITOK *itok4,int razr);
int CheckUpdRegToConst(int reg,ITOK *itok4,int operand,int razr);
int SwapVarConst(ITOK *itok2,ITOK *itok4);
int SwapVarRegConst(int reg,ITOK *itok4,int razr);
int CheckConstVar(ITOK *itok4);
//int CheckConstVar2(ITOK *itok4,long long *num,int *typenum);
void CheckConstVar3(int *tok4,ITOK *itok4,int razr);
void Const2VarRec(LVIC *varconst);
#endif

#ifndef _REGOPT_
extern unsigned char optinitreg;
#ifdef OPTVARCONST
extern unsigned char replasevar;
extern LVIC *listvic;
#endif
#endif

/*-----------------------
libobj
 -------------------------*/
void AddNameObj(char *name,int typefind,int type);
void AddObj();

#ifdef __NEWLEX__
void inittokn();
void doblockstart(char *);

#endif
/* end of TOK.H */
