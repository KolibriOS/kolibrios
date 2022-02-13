#define _MAIN_

#ifdef _UNIX_
#define DIV_PATH ':'			//делитель путей в переменной окружения PATH
#define DIV_FOLD '/'			//этим символом разделяются папки в пути к файлу
#endif

#ifdef _WIN32_
#define DIV_PATH ';'
#define DIV_FOLD '\\'
#endif

#ifdef _KOS_
#define DIV_PATH ';'
#define DIV_FOLD '/'
#endif

#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "tok.h"

#ifdef _KOS_
#include <conio.h>
#endif

static char **_Argv; //!!! simplest way to make your own variable

unsigned char compilerstr[]="SPHINX C-- 0.239";
char *rawfilename;					/* file name */
char *rawext;
LISTCOM *listcom;
EWAR wartype={NULL,NULL},errfile={NULL,NULL};
int numfindpath=0;
char *findpath[16];
char modelmem=TINY;
char *stubfile=NULL;
char *winstub=NULL;
FILE *hout=NULL;
const char *namestartupfile="startup.h--";

char outext[4] = "com";
short extflag=TRUE;//расширение можно присвоить
//int scrsize;
unsigned char gwarning=FALSE;
unsigned char sobj=FALSE;
unsigned char usestub=TRUE;
unsigned char dpmistub=FALSE;
short dllflag=FALSE;
static int numstr;

char meinfo[]=
	"\nEdition of this version by\n"
	"    Mishel Sheker\n"
	"    Fido 2:5021/3.40\n"
	"    E-Mail sheker@mail.ru\n"
	"    Russia";

time_t systime;
struct tm timeptr;
char comsymbios=FALSE;
char fobj=FALSE;	//признак генерации obj
unsigned int	startptr = 0x100; 			// start address
unsigned char wconsole=FALSE;	//признак генерации консольного приложения windows
unsigned char optstr=FALSE;	//оптимизация строковых констант
unsigned char crif=TRUE;	//check reply include file
unsigned char idasm=FALSE;	//ассемблерные инструкции считать идентификаторами
unsigned char wbss=2;	//пост переменные в отдельную секцию
unsigned char use_env=FALSE;	//переменная окружения
int numrel=0;	//число элементов в таблице перемещений
unsigned char useordinal=FALSE;
unsigned char useDOS4GW=FALSE;
unsigned char clearpost=FALSE;
unsigned char uselea=TRUE;
unsigned char regoverstack=TRUE;
unsigned char shortimport=FALSE;
unsigned char useinline=2;
unsigned char ocoff=FALSE;
unsigned char ESPloc=FALSE;

int startupfile=-1;
int alignproc=8,aligncycle=8;

const char *usage[]={
"USAGE: C-- [options] [FILE_NAME.INI] [SOURCE_FILE_NAME]",
"",
"                       C-- COMPILER OPTIONS",
"",
"                           OPTIMIZATION",
"-OC  optimize for code size           -DE  enable temporary expansion variable",
"-OS  optimize for speed               -OST enable optimization string",
"-ON  enable optimization number       -AP[=n] align start function",
"-UST use startup code for variables   -AC[=n] align start cycles",
#ifdef OPTVARCONST
"-ORV replace variable on constant     -OIR skip repeated initializing register",
#else
"                                      -OIR skip repeated initializing register",
#endif
"",
"                          CODE GENERATION",
"-2   80286 code optimizations         -SA=#### start code address",
"-3   80386 code optimizations         -AL=## set value insert byte",
"-4   80486 code optimizations         -WFA fast call API functions",
"-5   pentium code optimizations       -IV  initial all variables",
"-A   enable address alignment         -SUV=#### start address variables",
"-AS[=n] def. alignment in structures  -LRS load in registers over stack",
"-UL  use 'lea' for adding registers   -JS  join stack calling functions",
"-BA  byte access to array",//             -ASP addressing local variable via ESP",
"",
"                           PREPROCESSOR",
"-IP=<path>  include file path         -IA   assembly instructions as identifier",
"-D=<idname> defined identifier        -CRI- not check include file on repeated",
"-MIF=<file> main input file           -IND=<name> import name from dll",
"-SF=<file>  other startup file",
"",
"                              LINKING",
"-AT    insert ATEXIT support block    -NS    disable stub",
"-ARGC  insert parse command line      -S=#####  set stack size",
"-P     insert parse command line      -WIB=##### set image base address",
"-C     insert CTRL<C> ignoring code   -WFU   add Fix Up table, for Windows",
"-R     insert resize memory block     -WMB   create Windows mono block",
"-ENV   insert variable with environ   -WS=<name> set name stub file for win32",
"-J0    disable initial jump to main() -WBSS  set post data in bss section",
"-J1    initial jump to main() short   -WO    call API functions on ordinals",
"-J2    initial jump to main() near    -CPA   clear post area",
"-STUB= <name> set name stub file      -WSI   short import table, for Windows",
"-DOS4GW file running with DOS4GW      -WAF=#### align Windows file (def 512)",
"-STM   startup code in main function",
"",
"                           OUTPUT FILES",
"-TEXE DOS EXE file (model TINY)       -D32  EXE file (32bit code for DOS)",
"-EXE  DOS EXE file (model SMALL)      -W32  EXE for Windows32 GUI",
"-OBJ  OBJ output file                 -W32C EXE for Windows32 console",
"-SOBJ slave OBJ output file           -DLL  DLL for Windows32",
"-COFF OBJ COFF output file            -DBG  create debug information",
"-SYM  COM file symbiosis              -LST  create assembly listing",
"-SYS  device (SYS) file               -B32  32bit binary files",
"-MEOS executable file for MeOS        -MAP  create function map file",
"-EXT= <ext> set file extension",
"",
"                           MISCELLANEOUS",
"-HELP -H -? help, this info           -WORDS list of C-- reserved words",
"-W         enable warning             -LAI   list of assembler instructions",
"-WF=<file> direct warnings to a file  -ME    display my name and my address",
"-MER=##    set maximum number errors  -X     disable SPHINX C-- header in output",
"-NW=##     disable selected warnings  -WE=## selected warning will be error",
//" -SCD        split code and date",
NULL};

const char *dir[]={
	"ME",    "WORDS",   "SYM",   "LAI",

	"OBJ",   "SOBJ",    "J0",		 "J1",     "J2",    "C",     "R",
	"P",     "X",       "EXE",   "S",      "SYS",   "ARGC",  "TEXE",
	"ROM",   "W32",     "D32",   "W32C",   "AT",    "WFA",   "SA",
	"STM",   "SUV",     "UST",   "MIF",    "DLL", 	"DOS4GW","ENV",
	"CPA",   "WBSS",    "MEOS",  "SF",     "B32",   "WIB",   "DBG",

	"OS",    "OC",      "A",     "0",      "1",     "2",     "3",
	"4",     "5",       "6",     "7",      "8",     "9",     "W",
	"WF",    "DE",
	"ON",    "IP",      "STUB",  "NS",     "AP",    "D",     "OST",
	"CRI",   "IA",      "SCD",   "AL",     "WFU",   "IV",
	"MER",   "WMB",     "HELP",  "H",      "?",     "AC",    "WS",
	"IND",   "WO",      "NW",    "LST",    "AS",    "UL",    "LRS",
	"WSI",   "WAF",     "OIR",   "COFF",   "JS",    "BA",    "ASP",
#ifdef OPTVARCONST
	"ORV",
#endif
   "MAP",  "WE",   "EXT",   NULL};

enum {
	c_me,    c_key,     c_sym,   c_lasm,   c_endinfo=c_lasm,

	c_obj,   c_sobj,    c_j0,    c_j1,     c_j2,    c_ctrlc, c_r,
	c_p,     c_x,		    c_exe,   c_s,      c_sys,   c_arg,   c_texe,
	c_rom,   c_w32,     c_d32,   c_w32c,   c_at,    c_wfa,   c_sa,
	c_stm,   c_suv,     c_ust,   c_mif,    c_dll,   c_d4g,   c_env,
	c_cpa,   c_wbss,    c_meos,  c_sf,     c_b32,   c_wib,   c_dbg,
	c_endstart=c_dbg,

	c_os,    c_oc,      c_a,     c_0,      c_1,     c_2,     c_3,
	c_4,     c_5,       c_6,     c_7,      c_8,     c_9,     c_w,
	c_wf,    c_de,
	c_opnum, c_ip,      c_stub,  c_ns,     c_ap,    c_define,c_ost,
	c_cri,   c_ia,      c_scd,   c_al,     c_wfu,   c_iv,
	c_mer,   c_wmb,     c_help,  c_h,      c_hh,    c_ac,    c_ws,
	c_ind,   c_wo,      c_nw,    c_lst,    c_as,    c_ul,    c_lrs,
	c_wsi,   c_waf,     c_oir,   c_coff,   c_js,    c_ba,    c_asp,
#ifdef OPTVARCONST
	c_orv,
#endif
	c_map,   c_we,      c_ext,   c_end};

#define NUMEXT 6	//число разрешенных расширений компилируемого файла
char extcompile[NUMEXT][4]={"c--","cmm","c","h--","hmm","h"};

#ifdef _KOS_
char __pgmname[256];
char __cmdline[256];
#endif

char *bufstr=NULL;	//буфер для строк из процедур
int sbufstr=SIZEBUF;	//начальный размер этого буфера

void compile();
void PrintInfo(char **str);
void LoadIni(char *name);
//void CheckNumStr();
void ListId(int num,unsigned char *list,unsigned short *ofs);
void printmemsizes();
void print8item(char *str);
void doposts(void);
void GetMemExeDat();
void AddJmpApi();
void startsymbiosys(char *symfile);
int writeoutput();
void BadCommandLine(char *str);
void  CheckExtenshions();
void ImportName(char *name);
void WarnUnusedVar();//предупреждения о неиспользованных процедурах и переменных
void MakeExeHeader(EXE_DOS_HEADER *exeheader);
void CheckPageCode(unsigned int ofs);
int MakePE();
int MakeObj();
void CheckUndefClassProc();

// Added by Coldy
void ParseObjCommand(int cmd);

void ListId(int numfirstchar,unsigned char *list,unsigned short *ofs)
{
char buf[40];
	for(int i=0;i<numfirstchar;i++){
		if((short)ofs[i]!=-1){
			if(i<26)buf[0]=(char)('A'+i);
			else if(i==26)buf[0]='_';
			else buf[0]=(char)(i-27+'a');
			unsigned char *ii=(unsigned char *)(list+ofs[i]);
			while(*(short *)&*ii!=-1){
				ii+=2;
				strcpy(buf+1,(char *)ii);
				ii+=strlen((char *)ii)+1;
				puts(buf);
//				CheckNumStr();
			}
		}
	}
}

/*
void PrintTegList(structteg *tteg)
{
	if(tteg){
		PrintTegList(tteg->left);
		PrintTegList(tteg->right);
		puts(tteg->name);
	}
} */

//unsigned long maxusedmem=0;

void ErrOpenFile(char *str)
{
	printf("Unable to open file %s.\n",str);
}

int main(int argc,char *argv[])
{
int count;
unsigned char pari=FALSE;
	char *buffer;

	char compiler_ver[64];
	snprintf(compiler_ver, 64, "\nSPHINX C-- Compiler   Version %d.%d%s   %s\r\n",ver1,ver2,betta,__DATE__);
	puts(compiler_ver);
#ifdef _KOS_
	con_set_title(compiler_ver);
#endif
//	scrsize=24;
	if(argc>1){
		_Argv=argv;// This make portable code
		bufstr=(char *)MALLOC(SIZEBUF);
		output=(unsigned char *)MALLOC((size_t)MAXDATA);
		outputdata=output;
		postbuf=(postinfo *)MALLOC(MAXPOSTS*sizeof(postinfo));
		strcpy((char *)string,argv[0]);
		rawext=strrchr((char *)string,DIV_FOLD);
		
		if(rawext!=NULL){
			rawext[0]=0;
			IncludePath((char *)string);
		} 
		rawfilename=getenv("C--");
		if(rawfilename!=NULL)IncludePath(rawfilename);
		
		rawfilename=rawext=NULL;
		LoadIni((char *)"c--.ini");	 
		
		for(count=1;count<argc;count++){ //обработка командной строки
			if(argv[count][0]=='/'||argv[count][0]=='-'){
			//if(argv[count][0]=='-'){
				if(SelectComand(argv[count]+1,&count)==c_end) BadCommandLine(argv[count]);
			}
			else{
				if(pari==FALSE){
					rawfilename=argv[count];
					pari=TRUE;
					if((rawext=strrchr(rawfilename,'.'))!=NULL){
						if(stricmp(rawext,".ini")==0){	//указан ini файл
							rawfilename=NULL;
							rawext=NULL;
							LoadIni(argv[count]);
							if(rawfilename==NULL)pari=FALSE;
						}
						else{
							*rawext++=0;
							CheckExtenshions();
						}
					}
				}
			}
		}
	}
  //{	Added by Coldy
	//	If -coff (for fully mscoff)
	if (ocoff && !sobj) {
		am32 = TRUE;
		ParseObjCommand(c_sobj);

	}
	//}
	if(rawfilename==NULL){
		PrintInfo((char **)usage);
		exit( e_noinputspecified );
	}
	time(&systime); //текущее время
	memcpy(&timeptr,localtime(&systime),sizeof(tm));
	InitDefineConst();
	compile();
	if(error==0)exit(e_ok);
	exit(e_someerrors);
	return 0;
}

void CheckExtenshions()
{
int i;
	for(i=0;i<NUMEXT;i++){
		if(stricmp(rawext,extcompile[i])==0)break;
	}
	if(i==NUMEXT){
	 	printf("Bad input file extension '%s'.",rawext);
	  exit(e_badinputfilename);
	}
}

void compile()
{
long segments_required;
union{
	long longhold;
	void *nextstr;
};
//создать имя файла с предупреждениями и если он есть удалить
	errfile.name=(char *)MALLOC(strlen(rawfilename)+5);
	sprintf(errfile.name,"%s.err",rawfilename);
	if(stat(errfile.name,(struct stat *)string2)==0)remove(errfile.name);
//если есть имя файла для предупреждений проверить его существование и удалить.
	if(wartype.name!=NULL){
		if(stat(wartype.name,(struct stat *)string2)==0)remove(wartype.name);
	}
	puts("Compiling Commenced . . .");
	if(rawext!=NULL)sprintf((char *)string,"%s.%s",rawfilename,rawext);
	else{
		for(unsigned int i=0;i<NUMEXT;i++){
			sprintf((char *)string,"%s.%s",rawfilename,extcompile[i]);
			if(stat((char *)string,(struct stat *)string2)==0)break;
		}
	}
	linenumber=0;
	initregstat();
#ifdef OPTVARCONST
	CreateMainLVIC();
#endif
#ifdef __NEWLEX__
	inittokn();
#endif
	compilefile((char *)string,2); //собственно разборка и компиляция
	puts("Link . . .");
	if(comfile==file_w32&&wbss==2){
		wbss=FALSE;
		if(wconsole==FALSE)wbss=TRUE;
	}
	if(notdoneprestuff==TRUE)doprestuff();	//startup code
	if(endifcount>=0)preerror("?endif expected before end of file");
	AddObj();
	docalls();	//добавить внешние процедуры
	addinitvar();
	CheckUndefClassProc();
	if(undefoffstart!=NULL){	//выдать список неизвестных ссылок
		UNDEFOFF *curptr=undefoffstart;
		for(;;){
			char holdstr[80];
			UNDEFOFF *ocurptr;
			linenumber=curptr->pos->line;
			sprintf(holdstr,"\'%s\' offset undefined",curptr->name);
			currentfileinfo=curptr->pos->file;
			preerror(holdstr);
			free(curptr->pos);
			if(curptr->next==NULL)break;
			ocurptr=curptr->next;
			free(curptr);
			curptr=ocurptr;
		}
		free(curptr);
	}
	while(liststring!=NULL){
		STRING_LIST *ins;
		ins=(STRING_LIST *)liststring;
		nextstr=ins->next;
		free(liststring);
		liststring=nextstr;
	}
	free(bufstr);
	if(warning==TRUE&&wact[7].usewarn)WarnUnusedVar();//предупреждения о неиспользованных процедурах и переменных
	if(numstrtbl)CreatStrTabRes();	//завершить создание ресурсов
	if(fobj==FALSE){
		if(comfile==file_w32&&error==0){
			AddJmpApi();	//косвенные вызовы API
			CreatWinStub();
		}
		longhold=outptr;
		if(comfile==file_rom){
			ooutptr=outptr;
			if(modelmem==SMALL){
				*(short *)&output[stackstartaddress]=(short)(((outptrdata+postsize+stacksize)/4+1)*4);
				*(short *)&output[dataromstart]=(short)(outptr+4);
				*(short *)&output[dataromsize]=(short)(outptrdata/2);
//				printf("outptr=%d outptrdate=%d outptrsize=%d\n",outptr,outptrdata,outptrsize);
				for(unsigned int i=0;i<outptrdata;i++)op(outputdata[i]);
			}
			if(romsize==0){
				unsigned int i=outptr/1024;
				if((outptr%1024)!=0)i++;
				if(i>32)i=64;
				else if(i>16)i=32;
				else if(i>8)i=16;
				else if(i>4)i=8;
				else if(i>2)i=4;
		 		romsize=i*1024;
				output[2]=(unsigned char)(romsize/512);
			}
			if(outptr>=romsize)preerror("The size of a code is more than the size of the ROM");
			for(;outptr<romsize;)op(aligner);
			unsigned char summa=0;
			for(unsigned int i=0;i<romsize;i++)summa+=output[i];
			output[romsize-1]-=summa;
			outptr=ooutptr;
		}
		else if(modelmem==SMALL&&comfile==file_exe){ // if an EXE file
			longhold+=AlignCD(CS,16);
//			if((outptr%16)!=0)outptr+=16-outptr%16;// paragraph align the end of the code seg
			if(((long)outptrdata+(long)postsize+(long)stacksize)>65535L)
					preerror("Data and stack total exceeds 64k");
		}
		else if(comfile==file_sys){
			for(int i=0;i<sysnumcom;i++){
				searchvar((listcom+i)->name);
				*(short *)&output[syscom+i*2]=(unsigned short)itok.number;
			}
			free(listcom);
		}
		else longhold+=(long)postsize+(long)(stacksize);
		if(am32==0&&longhold>65535L&&!(modelmem==TINY&&(!resizemem)))preerror("Code, data and stack total exceeds 64k");
		if(posts>0)doposts();  //Установить адреса вызовов процедур и переходов
		if(resizemem&&comfile==file_com){
			segments_required=(outptr+postsize+stacksize+15)/16;
			*(short *)&output[resizesizeaddress]=(short)segments_required;
			*(short *)&output[stackstartaddress]=(short)(segments_required*16);
		}
	}
	deinitregstat();
#ifdef OPTVARCONST
	KillMainLVIC();
#endif
//	puts("List Teg name:");
//	PrintTegList(tegtree);
	printf("COMPILING FINISHED.  Errors: %d\n",error);
	if(error==0){
		if(cpu>=1){
			char m1[12];
			switch(cpu){
				case 5:
					strcpy(m1,"Pentium");
					break;
				case 6:
					strcpy(m1,"MMX");
					break;
				case 7:
					strcpy(m1,"Pentium II");
					break;
				case 8:
					strcpy(m1,"Pentium III");
					break;
				case 9:
					strcpy(m1,"Pentium IV");
					break;
				default: sprintf(m1,"80%d86",cpu);
			}
			printf("CPU required: %s or greater.\n",m1);
		}
		runfilesize=outptr-startptr;
		if(comfile==file_rom)runfilesize=romsize;
		else if(modelmem==SMALL&&comfile==file_exe){
			runfilesize+=outptrdata-startptrdata+0x20;
			postsize+=postsize%2;
			stacksize=(stacksize+15)/16*16;
		}
		else if((comfile==file_exe||comfile==file_d32)&&modelmem==TINY)
				runfilesize+=0x20;
		printmemsizes();
		endinptr=outptr;
		if(writeoutput()==0)printf("Run File Saved (%ld bytes).\n",runfilesize);
		if(comfile==file_w32&&fobj==FALSE)printf("Created file of a format PE for Windows.\nFor alignment section code, added %u zero bytes.\n",filingzerope);
//		else if(FILEALIGN&&fobj==FALSE)printf("For alignment file, added %u zero bytes.\n",filingzerope);
	}
		if(pdbg)DoTDS();
}

void printmemsizes()
{
long stacklong;
unsigned int stackword;
unsigned int postword,codeword;
	postword=postsize;
	codeword=outptr-startptr;
	stackword=stacksize;
	if(comfile==file_com||(comfile==file_exe&&modelmem==TINY)){
		if(resizemem==0){
			stacklong=0xFFFE - outptr - postsize;
			stackword=stacklong;
		}
		codeword=codeword-datasize-alignersize;
	}
	else if(comfile==file_sys)stackword=sysstack;
	else if(comfile==file_exe||comfile==file_rom)datasize=outptrdata;
	else if(comfile==file_d32)codeword-=datasize;
	printf("Code: %u bytes, Data: %u bytes, Post: %u bytes, Stack: %u bytes\n"
			,codeword,datasize,postword,stackword);
	for(int i=0;i<posts;i++){
		switch((postbuf+i)->type){
			case CODE_SIZE:
				*(short *)&output[(postbuf+i)->loc]+=codeword;
				break;
			case CODE_SIZE32:
				*(long *)&output[(postbuf+i)->loc]+=codeword;
				break;
			case DATA_SIZE:
				*(short *)&output[(postbuf+i)->loc]+=datasize;
				break;
			case DATA_SIZE32:
				*(long *)&output[(postbuf+i)->loc]+=datasize;
				break;
			case POST_SIZE:
				*(short *)&output[(postbuf+i)->loc]+=postword;
				break;
			case POST_SIZE32:
				*(long *)&output[(postbuf+i)->loc]+=postword;
				break;
			case STACK_SIZE:
				*(short *)&output[(postbuf+i)->loc]+=stackword;
				break;
			case STACK_SIZE32:
				*(long *)&output[(postbuf+i)->loc]+=stackword;
				break;
		}
	}
}

void PrintInfo(char **str)
{
	numstr=1;
	for(int i=0;str[i]!=NULL;i++){
		puts(str[i]);
//		CheckNumStr();
	}
}

/*void CheckNumStr()
{
#ifndef _UNIX_
	if(((numstr+1)%(scrsize-1))==0&&outfile!=0){
		puts("Press any key...");
		getch();
	}
	numstr++;
#endif
} */

void strbtrim(char *st)
{
int i;
char *p,*q;
	p=q=st;
	while(isspace(*p))p++;	//пока незначащие символы
	while(*p)*q++=*p++;     //переместить строку
	*q='\0';
	for(i=strlen(st)-1;isspace(st[i])&&i>=0;i--);
	st[i+1]='\0';
}

unsigned long getnumber(unsigned char *buf)
{
int temp2;
unsigned long retnum;
unsigned char *oinput;
unsigned int oinptr,oendinptr;
	if(!isdigit(buf[0]))return 0;
	oinptr=inptr;
	oinput=input;
	oendinptr=endinptr;
	input=buf;
	inptr=0;
	endinptr=256;
	retnum=scannumber(&temp2);
	inptr=oinptr;
	input=oinput;
	endinptr=oendinptr;
	return retnum;
}

void ParseObjCommand(int cmd){
	switch (cmd) {
	case c_sobj:
		sobj = TRUE;
		FixUp = TRUE;
		jumptomain = CALL_NONE;
	case c_obj:
		fobj = TRUE;
		//					if(comfile==file_d32)FixUp=TRUE;
		FastCallApi = FALSE;
	}
}

int SelectComand(char *pptr,int *count)
{
int i;
unsigned char neg=FALSE;
char *ptr;
int len;
	if((ptr=strchr(pptr,';'))!=NULL)*ptr=0;// ищем комментарий отсекаем все после него
        if((ptr=strchr(pptr,'='))!=NULL){ // ищем знак равенства
		*ptr=0; // делим
		ptr++;
	  strbtrim(ptr);	//убрать лишние пробелы
	}
  strbtrim(pptr);	//убрать лишние пробелы
	if(*pptr==0)return c_end+1;	//пустая строка
	if((i=strlen(pptr))>1&&pptr[i-1]=='-'){
		neg=TRUE;
		pptr[i-1]=0;
	}
	strupr(pptr);
	for(i=0;dir[i]!=NULL;i++){
		if(strcmp(dir[i],pptr)==0){
			if((i<=c_endinfo)&&count==0){
				char buf[80];
				sprintf(buf,"Option '%s' is used only in command line",dir[i]);
				preerror(buf);
				return i;
			}
			if(i<=c_endstart&&notdoneprestuff!=TRUE){
errlate:
				char buf[80];
				sprintf(buf,"Too late used '#pragma option %s'",dir[i]);
				preerror(buf);
				return i;
			}
			switch(i){
				case c_j0: jumptomain=(unsigned char)(neg!=FALSE?CALL_NEAR:CALL_NONE); header=(unsigned char)0^neg; break;
				case c_j1: jumptomain=(unsigned char)CALL_SHORT; header=(unsigned char)1; break;
				case c_j2: jumptomain=(unsigned char)(neg==FALSE?CALL_NEAR:CALL_NONE); header=(unsigned char)1^neg; break;
				case c_ctrlc: killctrlc=(unsigned char)1^neg; break;
				case c_os: optimizespeed=(unsigned char)1^neg; break;
				case c_oc: optimizespeed=(unsigned char)0^neg; break;
				case c_r: resizemem=(unsigned char)1^neg; break;
				case c_p: parsecommandline=(unsigned char)1^neg; break;
				case c_a: alignword=(unsigned char)1^neg; break;
				case c_sym:
					startptr=0x100;
					comsymbios=TRUE;
					*count=*count+1;
          startsymbiosys(_Argv[*count]);
					break;
				case c_0: chip=0; break;
				case c_1: chip=1; break;
				case c_2: chip=2; break;
				case c_3: chip=3; break;
				case c_4: chip=4; break;
				case c_5: chip=5; break;
				case c_6: chip=6; break;	//MMX
				case c_7: chip=7; break;  //Pro
				case c_8: chip=8; break;  //PII
				case c_9: chip=9; break;  //PIII
				case c_x: header=(unsigned char)0^neg; break;
				case c_exe:
					comfile=file_exe;
					modelmem=SMALL;
					splitdata=TRUE;
					GetMemExeDat();
					if(extflag) strcpy(outext,"exe");
					startptr=0; 			// start address
					startptrdata=0; 	// data start address
					dos1=2;
					dos2=0;
					break;
				case c_sys:
					comfile=file_sys;
					if(extflag) strcpy(outext,"sys");
					startptr=0; 		// start address
					startptrdata=0; 	// data start address
					jumptomain=CALL_NONE;
					header=0;
					break;
				case c_sobj:
/*					sobj=TRUE;
					FixUp=TRUE;
					jumptomain=CALL_NONE;
*/			case c_obj:
/*					fobj=TRUE;
//					if(comfile==file_d32)FixUp=TRUE;
					FastCallApi=FALSE;
*/
          ParseObjCommand(i);
					break;
				case c_me:
					puts(meinfo);
					exit( e_ok );
				case c_key:
					int j,jj;
					puts("LIST OF RESERVED IDENTIFIERS:");
					numstr=1;
					ListId(53,(unsigned char *)id,idofs);
					for(j=0;j<ID2S;j++){
						puts(id2[j]);
//						CheckNumStr();
					}
					for(jj=0;jj<2;jj++){
						for(j=0;j<8;j++)printf("%s ",regs[jj][j]);
						puts("");
//						CheckNumStr();
					}
					for(j=0;j<8;j++)printf("%s ",begs[j]);
					puts("");
//					CheckNumStr();
					for(j=0;j<6;j++)printf("%s ",segs[j]);
					print8item("ST(%d) ");
					puts("ST");
					print8item("st(%d) ");
					puts("st");
					exit(e_ok);
				case c_lasm:
					puts("LIST OF SUPPORTED ASSEMBLER INSTRUCTIONS:");
					numstr=1;
					ListId(26,asmMnem,ofsmnem);
					exit(e_ok);
				case c_s:
					if(ptr==NULL)return c_end;
					if((stacksize=getnumber((unsigned char *)ptr))==0){
						puts("Bad stack size.");
						exit(e_unknowncommandline);
					}
					stacksize=Align(stacksize,4);
					break;
				case c_w:
					gwarning=(unsigned char)TRUE^neg;
					break;
				case c_wf:
					if(wartype.name)free(wartype.name);
					wartype.name=BackString(ptr);
					break;
				case c_de:
					divexpand=(unsigned char)TRUE^neg;
					break;
				case c_opnum:
					optnumber=(unsigned char)TRUE^neg;
					break;
				case c_ip:
					IncludePath(ptr);
					break;
				case c_arg:
					parsecommandline=(unsigned char)TRUE^neg;
					fargc=(unsigned char)TRUE^neg;
					break;
				case c_texe:
					if(extflag) strcpy(outext,"exe");
					comfile=file_exe;
					break;
				case c_rom:
					if(extflag) strcpy(outext,"rom");
					comfile=file_rom;
					startptr=0;
					startptrdata=0; 	// data start address
					GetMemExeDat();
					break;
				case c_dll:
					wconsole=TRUE;
					dllflag=TRUE;
					if(extflag) strcpy(outext,"dll");
					comfile=file_w32;
					FixUpTable=TRUE;
					goto nexpardll;

/*					FixUp=TRUE;
					startptrdata=startptr=0;
					am32=TRUE;
					if(chip<3)chip=3;
					if(FILEALIGN==0)FILEALIGN=512;
					break;*/
				case c_w32c:
					wconsole=TRUE;
					goto init_w32;
				case c_w32:
					wconsole=FALSE;
					dllflag=FALSE;
init_w32:
					comfile=file_w32;
					goto nexpar;
				case c_d32:
					comfile=file_d32;
nexpar:
					if(extflag) strcpy(outext,"exe");
nexpardll:
					FixUp=TRUE;
					startptrdata=startptr=0;
					am32=TRUE;
					if(chip<3)chip=3;
					if(FILEALIGN==0)FILEALIGN=512;
					break;
				case c_meos:
					comfile=file_meos;
					am32=TRUE;
					startptrdata=startptr=0;
					if(extflag) strcpy(outext,"");
					if(chip<3)chip=3;
					break;
				case c_b32:
					comfile=file_bin;
					am32=TRUE;
					startptrdata=startptr=0;
					if(extflag) strcpy(outext,"bin");
					FixUp=TRUE;
					ImageBase=0;
					if(chip<3)chip=3;
					break;
				case c_stub:
					if(stubfile)free(stubfile);
					stubfile=BackString(ptr);
					dpmistub=FALSE;
					if(stricmp(stubfile,"dpmi")==0){
						if(notdoneprestuff!=TRUE)goto errlate;
						dpmistub=TRUE;
						usestub=FALSE;
					}
					break;
				case c_ns:
					usestub=(unsigned char)0^neg;
					break;
				case c_ap:
					AlignProc=(unsigned char)1^neg;
					if(ptr!=NULL){
						alignproc=getnumber((unsigned char *)ptr);
						if(alignproc<1||alignproc>4096)alignproc=8;
					}
					break;
				case c_define:
					addconsttotree(ptr,TRUE);
					break;
				case c_ost:
					optstr=(unsigned char)TRUE^neg;
					break;
				case c_cri:
					crif=(unsigned char)1^neg;
					break;
				case c_ia:
					idasm=(unsigned char)1^neg;
					break;
				case c_dbg:
					dbg&=0xFE;
					char c;
					c=(unsigned char)1^neg;
					dbg|=c;
					if(!neg)InitDbg();
					break;
				case c_scd:
/*-----------------13.08.00 23:01-------------------
 будет введена после доработки динамических процедур
	--------------------------------------------------*/
					splitdata=(unsigned char)1^neg;
					if(modelmem==SMALL)splitdata=TRUE;
					break;
				case c_al:
					if(ptr==NULL)return c_end;
					aligner=(unsigned char)getnumber((unsigned char *)ptr);
					break;
				case c_at:
					atex=(unsigned char)1^neg;
					break;
				case c_wfa:
					FastCallApi=(unsigned char)1^neg;
					break;
				case c_wfu:
					FixUpTable=(unsigned char)1^neg;
					break;
				case c_wib:
					if(ptr==NULL)return c_end;
					ImageBase=getnumber((unsigned char *)ptr);
					break;
				case c_iv:
					notpost=(unsigned char)1^neg;
					break;
				case c_mer:
					if(ptr==NULL)return c_end;
					if((maxerrors=getnumber((unsigned char *)ptr))==0)maxerrors=16;
					break;
				case c_sa:
					if(ptr==NULL)return c_end;
					startptrdata=startptr=getnumber((unsigned char *)ptr);
					break;
				case c_stm:
					startuptomain=(unsigned char)1^neg;
					break;
				case c_suv:
					if(ptr==NULL)return c_end;
					startStartup=getnumber((unsigned char *)ptr);
					break;
				case c_wmb:
					WinMonoBlock=(unsigned char)1^neg;
					break;
				case c_ust:
					useStartup=(unsigned char)1^neg;
					break;
				case c_help:
				case c_h:
				case c_hh:
					PrintInfo((char **)usage);
					exit(e_ok);
				case c_mif:
					if(ptr==NULL)return c_end;
					if(rawfilename!=NULL){
						free(rawfilename);
						rawfilename=NULL;
					}
					if(rawext!=NULL){
						free(rawext);
						rawext=NULL;
					}
					char *temp;
        	if((temp=strrchr(ptr,'.'))!=NULL){
						*temp++=0;
						rawext=BackString(temp);
						CheckExtenshions();
 	  	    }
					rawfilename=BackString(ptr);
					break;
				case c_ac:
					AlignCycle=(unsigned char)1^neg;
					if(ptr!=NULL){
						aligncycle=getnumber((unsigned char *)ptr);
						if(aligncycle<1&&aligncycle>4096)aligncycle=8;
					}
					break;
				case c_ws:	//dos-stub for windows programs
					if(winstub)free(winstub);
					winstub=BackString(ptr);
					break;
				case c_ind:
					ImportName(ptr);
					break;
				case c_wbss:
					wbss=(unsigned char)1^neg;
					break;
				case c_wo:
					useordinal=(unsigned char)1^neg;
					break;
				case c_nw:
					if(ptr==NULL)return c_end;
					len=getnumber((unsigned char *)ptr);
					if(len>0&&len<=WARNCOUNT)wact[len-1].usewarn=(unsigned char)0^neg;
					break;
				case c_we:
					if(ptr==NULL)return c_end;
					len=getnumber((unsigned char *)ptr);
					if(len>0&&len<=WARNCOUNT){
						if(neg)wact[len-1].fwarn=warningprint;
						else wact[len-1].fwarn=preerror3;
					}
					break;
				case c_lst:
					SetLST(neg);
					break;
				case c_d4g:
					useDOS4GW=(unsigned char)1^neg;
					break;
				case c_env:
					use_env=(unsigned char)1^neg;
					break;
				case c_cpa:
					clearpost=(unsigned char)1^neg;
					break;
				case c_ul:
					uselea=(unsigned char)1^neg;
					break;
				case c_as:
					if(ptr){
						len=getnumber((unsigned char *)ptr);
						if(caselong(len)!=NUMNUM)strpackdef=len;
					}
					else strpackdef=(neg==FALSE?8:1);
					strpackcur=strpackdef;
					break;
				case c_lrs:
					regoverstack=(unsigned char)1^neg;
					break;
				case c_wsi:
					shortimport=(unsigned char)1^neg;
					break;
				case c_waf:
					if(ptr!=NULL){
						FILEALIGN=Align(getnumber((unsigned char *)ptr),16);
					}
					break;
				case c_sf:
					if(ptr==NULL)return c_end;
					namestartupfile=BackString(ptr);
					break;
				case c_oir:
					optinitreg=(unsigned char)1^neg;
					break;
				case c_coff:
					ocoff=(unsigned char)1^neg;
					break;
				case c_js:
					addstack=(unsigned char)1^neg;
					if(addstack==0)RestoreStack();
					break;
				case c_ba:
					bytesize=(unsigned char)1^neg;
					break;
				case c_asp:
					if(blockproc)goto errlate;
					else ESPloc=(unsigned char)1^neg;
					break;
#ifdef OPTVARCONST
				case c_orv:
					replasevar=(unsigned char)1^neg;
					if(replasevar&&listvic)ClearLVIC();
					break;
				case c_map:
					mapfile=(unsigned char)1^neg;
					break;
#endif
				case c_ext:     //***lev***
					strcpy(outext,BackString(ptr)); //***lev***
					extflag=FALSE; //чтобы расширение не перезабивалось другими ключами, если они идут позже
					break;  //***lev***
			}
			break;
		}
	}
	return i;
}

void SetLST(unsigned char neg)
{
	if(((dbg&2)>>1)==neg){
		dbg&=0xFD;
		unsigned char c=(unsigned char)((1^neg)<<1);
		dbg|=c;
		if(neg){
			if((dbg&0xFE)==0)dbgact=TRUE;
			AddEndLine();
		}
		else{
			InitDbg();
			if(notdoneprestuff!=TRUE)dbgact=FALSE;	//startup cod
		}
	}
}

void print8item(char *str)
{
//	CheckNumStr();
	for(int j=0;j<8;j++)printf(str,j);
	puts("");
}




void _loadIni(FILE *inih)
{
	char m1[256];
	for(;;){
		if(fgets(m1,255,inih)==NULL)break;
		if(SelectComand(m1,0)==c_end)BadCommandLine(m1);
	}
	fclose(inih);
}

void LoadIni(char *name)
{
FILE *inih;
char m1[256];

							// load name
	if (inih = fopen(name,"rb"))
	{_loadIni(inih);return;}
	
	if(strcmp(name,"c--.ini")!=0)
			return;

							//load findpath[0]\c--.ini
	if (findpath[0]!=0)
	{
		sprintf(m1,"%s%s",findpath[0],name);
		if (inih = fopen(m1,"rb"))
		{_loadIni(inih);return;}
	}
							//load PATH[i=0..end]/c--.ini
	char* pth = getenv("PATH");	
	if (pth != 0)
	{
		char* start;
		int size;
		start = pth;	
		while(*start) 
		{
			size = 0;
			char* endp = strchr(start, DIV_PATH);
			size = (endp == 0)? strlen(start): endp-start;
			strncpy(m1, start, size);
			start += size + 1;
			if (m1[size - 1] != DIV_FOLD)
				m1[size++] = '/';
			
			strcpy(m1 + size,"c--.ini");
			if (inih = fopen(m1,"rb"))
			{_loadIni(inih);return;}
		}
	}
#ifdef _KOS_
								//for KolibriOS: load program_dir/c--.ini 
	int p;
	strcpy(m1,__pgmname);
	p = strlen(m1);
	while ((*(m1+p)!='/') && (p != 0))
		p--;
	if (p){
		p++;
		strcpy(m1+p,"c--.ini");
		if (inih = fopen(m1,"rb"))
		{_loadIni(inih);return;}
	}
								//for KolibriOS: load /sys/settings/c--.ini
	inih = fopen("/sys/settings/c--.ini","rb");
	for(;;){
		if(fgets(m1,255,inih)==NULL)break;
		if(SelectComand(m1,0)==c_end)BadCommandLine(m1);
	}
	fclose(inih);
	return;
#endif //_KOS_		
}
			
/*****************************************************************************
* .NAME   : MALLOC
* .TITLE  : Выделяет память с обработкой ошибок.
*****************************************************************************/
void OutMemory()
{
	preerror("Compiler out of memory");
	exit(e_outofmemory);
}

void * MALLOC (unsigned long size)
{
void *mem;
	mem=malloc(size);
	if(mem==NULL)OutMemory();
#ifdef _UNIX_
	else memset(mem,0,size);
#endif

//	if(((unsigned long)mem+size)>maxusedmem)maxusedmem=(unsigned long)mem+size;

	return mem;
}

void *REALLOC(void *block,unsigned long size)
{
void *mem;
	mem=realloc(block,size);
	if(mem==NULL)OutMemory();

//	if(((unsigned long)mem+size)>maxusedmem)maxusedmem=(unsigned long)mem+size;

	return mem;
}

void IncludePath(char *buf)
{
	char divfold[3];
	sprintf(divfold,"%c",DIV_FOLD);
	if(numfindpath<MAXNUMPATH-1){
		int len=strlen(buf);
		if(buf[len-1]==DIV_FOLD)buf[len-1]=0;
		else len++;
		char *a=(char *)MALLOC(len+1);
		strcpy(a,buf);
		strcat(a,divfold);
		findpath[numfindpath]=a;
		numfindpath++;
		findpath[numfindpath]="";
	}
	else puts("Too many include paths");
}

void doposts()
{
unsigned long addvalue,i,addval,addvalw32=0,addvalbss=0;
	if(splitdata&&modelmem==TINY&&outptrdata){
		memcpy((char *)&output[outptr],(char *)&outputdata[0],outptrdata);
		addval=outptr;
		outptr+=outptrdata;
		outptrdata=outptr;
	}
	addvalue=outptrdata;
	if(comfile==file_bin)addvalbss=addvalw32=ImageBase;
	else if(comfile==file_w32){
		addvalbss=addvalw32=ImageBase+vsizeheader;
		if(postsize&&wbss){
			addvalw32+=Align(postsize,OBJECTALIGN);
			addvalue=0;
		}
	}
	else{
		if((outptrdata%2)==1){	/* alignment of entire post data block manditory */
			addvalue++;	//начало неиниц.данных
			postsize++;
		}
/*		if(am32&&(outptrdata%4)==2){
			addvalue+=2;	//начало неиниц.данных
			postsize+=2;
		}*/
	}
	if(am32==0&&(MAXDATA-outptrdata)<postsize)preerror("post variable size exceeds size left in data segment");
	for(i=0;i<posts;i++){
		switch((postbuf+i)->type){
			case POST_VAR:
				*(short *)&output[(postbuf+i)->loc]+=(short)addvalue;
				break;
			case POST_VAR32:
				*(long *)&output[(postbuf+i)->loc]+=addvalue+addvalbss;
				numrel++;
				break;
			case FIX_VAR32:
			case FIX_CODE32:
				*(long *)&output[(postbuf+i)->loc]+=addvalw32;
				numrel++;
				break;
			case FIX_CODE_ADD:
				if(am32){
					*(long *)&output[(postbuf+i)->loc]+=addvalw32+(postbuf+i)->num;
					(postbuf+i)->type=(unsigned short)FIX_VAR32;
				}
				else{
					*(short *)&output[(postbuf+i)->loc]+=(short)(addval+(postbuf+i)->num);
					(postbuf+i)->type=(unsigned short)FIX_VAR;
				}
				numrel++;
				break;
			case DATABLOCK_VAR:
				*(short *)&output[(postbuf+i)->loc]+=(short)addval;
				if(FixUp)(postbuf+i)->type=(unsigned short)FIX_VAR;
				break;
			case DATABLOCK_VAR32:
				*(long *)&output[(postbuf+i)->loc]+=addval+addvalw32;
				if(FixUp)(postbuf+i)->type=(unsigned short)FIX_VAR32;
				numrel++;
				break;
		}
	}
	ooutptr=addvalue;	//сохранить начало post для debug;
}

void GetMemExeDat()
{
	if(outputdata==output&&outputdata!=0)outputdata=(unsigned char *)MALLOC((size_t)MAXDATA);
}



int writeoutput()
{
EXE_DOS_HEADER exeheader;  // header for EXE format
	if(fobj){
		if(comfile==file_w32&&ocoff
       // Edited by Coldy
			||ocoff&&sobj)return MakeCoff();
		return MakeObj();
	}
	if(comfile==file_w32)return MakePE();
	if(comfile==file_meos)return MakeMEOS();
	if(comfile==file_bin)return MakeBin32();
	memset(&exeheader,0,sizeof(EXE_DOS_HEADER));
	if(comfile==file_d32){
		if(usestub){
			MakeLE();
			runfilesize+=ftell(hout)-32;
			goto savecode;
		}
		else goto exefile;
	}
	if(comfile==file_com||comfile==file_sys||comfile==file_rom){
		hout=CreateOutPut(outext,"wb");
		if(fwrite(output+startptr,comfile==file_rom?romsize:outptr-startptr,1,hout)!=1){
			ErrWrite();
			return(-1);
		}
	}
	else if(comfile==file_exe){
exefile:
		hout=CreateOutPut(outext,"wb");
		MakeExeHeader(&exeheader);
		if(fwrite(&exeheader,sizeof(EXE_DOS_HEADER),1,hout)!=1){
errwr:
			fclose(hout);
			hout=NULL;
			ErrWrite();
			return(-1);
		}
		outputcodestart=ftell(hout);
savecode:
		if(fwrite(output+startptr,outptr-startptr,1,hout)!=1)goto errwr;
		if(modelmem==SMALL&&outptrdata!=0){
			if(fwrite(outputdata,outptrdata,1,hout)!=1)goto errwr;
		}
	}
	fclose(hout);
	hout=NULL;
	return(0);
}

long CopyFile(FILE *in,FILE *out)
{
char tbuf[1024];
long loads=0;
unsigned int len;
	do{
		len=fread(tbuf,1,1024,in);
		if(fwrite(tbuf,1,len,out)!=len){
			ErrWrite();
			return -1;
		}
		loads+=len;
	}while(len==1024);
	return loads;
}

unsigned int EntryPoint()
{
ITOK btok;
int bb=tk_id;
unsigned char bufmain[]="main";
unsigned char buf2[]="__startupproc";
unsigned char *buf;
	if(comfile==file_com)return startptr;
	btok.number=0;
//	if(jumptomain!=CALL_NONE||(comfile==file_w32&&dllflag))buf=buf2;
//	else buf=bufmain;

	if(jumptomain==CALL_NONE){
		if(useDOS4GW)buf=buf2;
		else buf=bufmain;
	}
	else buf=buf2;

	searchtree(&btok,&bb,buf);
	if(bb==tk_id){
		if(comfile==file_w32&&dllflag)return 0xffffffff;
		printf("Error! Entry point '%s' is not found.\n",buf);
		exit(e_entrynotfound);
	}
	return btok.number;
}

void MakeExeHeader(EXE_DOS_HEADER *exeheader)
{
long paragraphsrequired;
unsigned short count,i;
int pointstart;
	exeheader->sign=0x5a4D;				 // MZ
//	if(modelmem==TINY&&comfile==file_exe)pointstart=0x100;
/*	else*/ pointstart=EntryPoint();
	count=(unsigned short)(runfilesize%512);
	i=(unsigned short)(runfilesize/512);
	exeheader->numlastbyte=count;
	exeheader->numpage=(unsigned short)(count==0?i:i+1);
	exeheader->headsize=2; 		// size of header in paragraphs (2 paragraphs)
	exeheader->initIP=(unsigned short)pointstart;	 // IP at entry (0x0000)
	paragraphsrequired=(outptr+outptrdata+postsize+stacksize+15)/16;
	if(modelmem==TINY&&comfile==file_exe){
		exeheader->initSS=0xFFF0; 		 // displacement of SS
		exeheader->initSP=0xFFFE; // intial value of SP
		exeheader->initCS=0xfff0;						 // displacement of CS (0x0000)
		if(!resizemem){
			exeheader->minmem=0xfff;	// min-paragraphs
			exeheader->maxmem=0xffff;	// max-paragraphs
		}
		else{
			paragraphsrequired-=0x10;
			exeheader->initSP=(unsigned short)(paragraphsrequired*16); // intial value of SP
			exeheader->minmem=(unsigned short)paragraphsrequired;	// min-paragraphs
			exeheader->maxmem=(unsigned short)paragraphsrequired;	// max-paragraphs
		}
	}
	else if(comfile==file_d32){
		exeheader->initSP=(unsigned short)stacksize; // intial value of SP
		exeheader->initSS=(unsigned short)((outptr+postsize+15)/16); // displacement of SS
		exeheader->initCS=(unsigned short)((pointstart/65536)*4096);
		if(resizemem){
			exeheader->minmem=(unsigned short)paragraphsrequired;	// min-paragraphs
			exeheader->maxmem=(unsigned short)paragraphsrequired;	// max-paragraphs
		}
		else exeheader->maxmem=(unsigned short)0xFFFF;	// max-paragraphs
	}
	else{
		exeheader->initSS=(unsigned short)(outptr/16); 		 // displacement of SS
		exeheader->initSP=(unsigned short)(outptrdata+postsize+stacksize); // intial value of SP
		exeheader->minmem=(unsigned short)paragraphsrequired;	// min-paragraphs
		exeheader->maxmem=(unsigned short)paragraphsrequired;	// max-paragraphs
	}
	exeheader->ofsreloc=0x1c;						 // offset of first relocation item (0x0000)
}

void startsymbiosys(char *symfile)
{
unsigned int size;
int filehandle;
long filesize;
	outptr=startptr;
	if((filehandle=open(symfile,O_BINARY|O_RDONLY))==-1){;
		ErrOpenFile(symfile);
		exit(e_symbioerror);
	}
	if((filesize=getfilelen(filehandle))!=-1L){
		if(filesize+outptr<MAXDATA){
			size=filesize;
			if((unsigned int)read(filehandle,output+outptr,size)!=size){
				close(filehandle);
				puts("Error reading symbio COM file.");
				exit(e_symbioerror);
			}
			outptr+=size;
		}
		else{
			puts("Symbio COM file is too large.");
			exit(e_symbioerror);
		}
	}
	else{
		puts("Unable to determine symbio COM file size.");
		exit(e_symbioerror);
	}
	close(filehandle);
	outptrdata=outptr;
	outputcodestart=outptr-startptr;
	addconsttotree("__comsymbios",TRUE);
}

void BadCommandLine(char *str)
{
	printf("Unknown or bad command line option '%s'.\n",str);
//					PrintInfo(usage);
	exit(e_unknowncommandline);
}

void warnunused(struct idrec *ptr)
{
//static count=0;
	if(ptr!=NULL){
		if(ptr->count==0&&startupfile!=ptr->file){
			linenumber=ptr->line;
			currentfileinfo=ptr->file;
			int i=0;
			switch(ptr->rectok){
				case tk_proc:
					if(ptr->recsegm!=NOT_DYNAMIC||strcmp(ptr->recid,mesmain)==0)break;
					i++;
				case tk_structvar:
					i++;
				case tk_charvar:
				case tk_bytevar:
				case tk_intvar:
				case tk_wordvar:
				case tk_longvar:
				case tk_dwordvar:
				case tk_floatvar:
				case tk_pointer:
				case tk_qword:
				case tk_double:
					if(i<=1&&(ptr->recpost==DYNAMIC_VAR||ptr->recpost==DYNAMIC_POST))break;
					warningnotused(ptr->recid,i);
					break;
			}
		}
		warnunused(ptr ->left);
		warnunused(ptr ->right);
	}
}

void WarnUnusedVar()//предупреждения о неиспользованных процедурах и переменных
{
	warnunused(treestart);
	for(unsigned int i=0;i<totalmodule;i++)warnunused((startfileinfo+i)->stlist);
}

void addinitvar()
{
unsigned int i;
	if(numfloatconst){	//вставить константы float и привязать их
		if(alignword||optimizespeed)AlignCD(DS,chip>5?16:4);
		for(i=0;i<posts;i++){
			if((postbuf+i)->type==POST_FLOATNUM){
				if(am32)*(unsigned long *)&output[(postbuf+i)->loc]=outptrdata+(postbuf+i)->num;
				else *(unsigned short *)&output[(postbuf+i)->loc]=(unsigned short)(outptrdata+(postbuf+i)->num);
				if(FixUp)(postbuf+i)->type=FIX_VAR32;
				else killpost(i--);
			}
		}
		for(i=0;i<numfloatconst;i++){
			if(dbg&2){
				if((floatnum+i)->type==tk_float)sprintf((char *)string,"const float %f",(floatnum+i)->fnum);
				else sprintf((char *)string,"const double %f",(floatnum+i)->dnum);
				AddDataNullLine(4,(char *)string);
			}
			outdwordd((floatnum+i)->num[0]);
			if((floatnum+i)->type!=tk_float){
				outdwordd((floatnum+i)->num[1]);
				datasize+=4;
			}
			datasize+=4;
		}
		free(floatnum);
		numfloatconst=0;
		floatnum=NULL;
	}
	for(i=0;i<(unsigned int)numswtable;i++){	//создать и вставить таблицы switch
		int j;
		FSWI *swt=swtables+i;
		if(alignword)AlignCD(DS,swt->type);
		if(dbg&2)AddDataNullLine((char)swt->type,"switch table address");
		if(am32==FALSE){	//вставить в код адрес таблицы
			*(unsigned short *)&output[swt->ptb]=(unsigned short)outptrdata;
		}
		else *(unsigned long *)&output[swt->ptb]=outptrdata;
		unsigned char oam32=am32;
		am32=(unsigned char)(swt->type/2-1);

		unsigned long val=swt->defal;
		int oline=outptrdata;
		for(j=0;j<swt->sizetab;j++){	//заполнить таблицу значениями по умолчанию
//			if((swt->info+jj)->type==singlcase)
			AddReloc(DS);
			if(am32)outdwordd(val);
			else outwordd(val);
		}
		if(swt->mode==2){
			if(dbg&2)AddDataNullLine((char)swt->razr,"switch table value");
			if(oam32==FALSE){	//вставить в код адрес таблицы
				*(unsigned short *)&output[swt->ptv]=(unsigned short)outptrdata;
			}
			else *(unsigned long *)&output[swt->ptv]=outptrdata;
		}
		int ii=0;	//счетчик case
		for(int jj=0;jj<swt->numcase;jj++){
			j=(swt->info+jj)->value;	//значение
			val=(swt->info+jj)->postcase;
			if((swt->info+jj)->type==singlcase){
				if(swt->mode==1){
					if(am32==FALSE)*(unsigned short *)&outputdata[oline+j*2]=(unsigned short)val;
					else *(unsigned long *)&outputdata[oline+j*4]=val;
				}
				else{
					if(am32==FALSE)*(unsigned short *)&outputdata[oline+ii*2]=(unsigned short)val;
					else *(unsigned long *)&outputdata[oline+ii*4]=val;
					switch(swt->razr){
						case r8: opd(j); break;
						case r16: outwordd(j); break;
						case r32: outdwordd(j); break;
					}
					ii++;
				}
			}
			else{
				jj++;
				for(;(unsigned int)j<=(swt->info+jj)->value;j++){
					if(swt->mode==1){
						if(am32==FALSE)*(unsigned short *)&outputdata[oline+j*2]=(unsigned short)val;
						else *(unsigned long *)&outputdata[oline+j*4]=val;
					}
					else{
						if(am32==FALSE)*(unsigned short *)&outputdata[oline+ii*2]=(unsigned short)val;
						else *(unsigned long *)&outputdata[oline+ii*4]=val;
						switch(swt->razr){
							case r8: opd(j); break;
							case r16: outwordd(j); break;
							case r32: outdwordd(j); break;
						}
						ii++;
					}
				}
			}
		}
		am32=oam32;
		free(swt->info);
	}
	if(numswtable){
		free(swtables);
		numswtable=0;
	}

	for(i=0;i<posts;i++){
		if((postbuf+i)->type==DIN_VAR||(postbuf+i)->type==DIN_VAR32){
			idrec *ptr=(idrec *)(postbuf+i)->num;
//			printf("post=%u num=%08X %s\n",ptr->recpost,ptr->recnumber,ptr->recid);
			if(ptr->recpost==USED_DIN_VAR)setdindata(ptr,i);
			else{
				if((postbuf+i)->type==DIN_VAR){
					*(unsigned short *)&output[(postbuf+i)->loc]+=(unsigned short)(ptr->recnumber);
				}
				else{
//					printf("loc=%08X num=%08X\n",*(unsigned long *)&output[(postbuf+i)->loc],ptr->recnumber);
					*(unsigned long *)&output[(postbuf+i)->loc]+=ptr->recnumber;
				}
			}
			if(FixUp)(postbuf+i)->type=(unsigned short)((postbuf+i)->type==DIN_VAR?FIX_VAR:FIX_VAR32);
			else killpost(i--);
		}
	}
	dopoststrings();
}

void setdindata(idrec *ptr,int i)
{
unsigned char *oldinput;
unsigned int oldinptr,oldendinptr;
unsigned char bcha;
unsigned int oline,ofile;
char *ostartline;
	if(alignword){
		if(ptr->rectok==tk_structvar)alignersize+=AlignCD(DS,2);	//выровнять
		else alignersize+=AlignCD(DS,GetVarSize(ptr->rectok));
	}
//	printf("loc=%08X out=%08X num=%08X\n",*(unsigned long *)&output[(postbuf+i)->loc],outptrdata,ptr->recnumber);
	if((postbuf+i)->type==DIN_VAR)*(unsigned short *)&output[(postbuf+i)->loc]+=(unsigned short)(outptrdata);
	else *(unsigned long *)&output[(postbuf+i)->loc]+=outptrdata;
	ptr->recpost=0;
	ptr->recnumber+=outptrdata;
	oline=linenum2;
	ofile=currentfileinfo;
	oldinput=input;
	oldinptr=inptr2;
	bcha=cha2;
	oldendinptr=endinptr;
	input=(unsigned char *)ptr->sbuf;
	inptr2=1;
	ostartline=startline;
	startline=(char*)input;
	cha2=input[0];
	linenum2=ptr->line;
	currentfileinfo=ptr->file;
	endinptr=strlen((char *)input);
	endinput=startline+endinptr;
	endoffile=0;
	tok=ptr->rectok;
	if(tok==tk_structvar)datasize+=initstructvar((structteg *)ptr->newid,ptr->recrm);
	else{
long type = 0,ssize = 0;
unsigned char typev = 0;
		if(tok>=tk_charvar&&tok<=tk_doublevar){
			type=tok-(tk_charvar-tk_char);
			typev=variable;
			ssize=typesize(type);
		}
		else if(tok==tk_pointer){
			typev=pointer;
			type=itok.type;
			if(am32==FALSE&&((itok.flag&f_far)==0))ssize=2;
			else ssize=4;
		}
		datasize+=initglobalvar(type,ptr->recsize/ssize,ssize,typev);
	}
	free(input);
	linenum2=oline;
	currentfileinfo=ofile;
	input=oldinput;
	inptr2=oldinptr;
	cha2=bcha;
	endinptr=oldendinptr;
	endoffile=0;
	startline=ostartline;
}

FILE *CreateOutPut(char *ext,char *mode)
{
char buf[256];
FILE *diskout;
    if(ext && strlen(ext)) {
        sprintf(buf,"%s.%s",rawfilename,ext);
    } else {
        strcpy(buf, rawfilename);
    }
    
	if((diskout=fopen(buf,mode))==NULL){
		ErrOpenFile(buf);
		exit(e_notcreateoutput);
	}
	return diskout;
}

int numundefclassproc=0;
idrec **undefclassproc;

void AddUndefClassProc()
{
	if(numundefclassproc==0)undefclassproc=(idrec **)MALLOC(sizeof(idrec **));
	else undefclassproc=(idrec **)REALLOC(undefclassproc,sizeof(idrec **)*(numundefclassproc+1));
	undefclassproc[numundefclassproc]=itok.rec;
	numundefclassproc++;
}

void CheckUndefClassProc()
{
	for(int i=0;i<numundefclassproc;i++){
		idrec *ptr=undefclassproc[i];
		if(ptr->rectok==tk_undefproc){
			currentfileinfo=ptr->file;
			linenumber=ptr->line;
			thisundefined(ptr->recid);
		}
	}
}
