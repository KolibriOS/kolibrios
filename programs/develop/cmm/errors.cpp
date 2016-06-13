#define _ERRORS_
#include "tok.h"

void warningprint(char *str,int line,int file);
WARNACT wact[WARNCOUNT]={
	warningprint,1,warningprint,1,warningprint,1,warningprint,1,warningprint,1,
	warningprint,1,warningprint,1,warningprint,1,warningprint,1,warningprint,1,
	warningprint,1,warningprint,1,warningprint,1,warningprint,1,warningprint,1};

int	maxerrors = 16; 				// number of errors to stop at

void warningprint(char *str,int line,int file);
unsigned char mapfile=FALSE;
FILE *hmap=NULL;

char shorterr[]="SHORT jump distance too large";
char buferr[128];

/* ================== error messages start =========================== */

void FindStopTok()
{
//	while(tok!=tk_eof&&itok2.type!=tp_stopper/*tok2!=tk_semicolon&&tok2!=tk_camma*/){
	do{
		nexttok();
	}while(tok!=tk_eof&&itok2.type!=tp_stopper&&itok.type!=tp_stopper/*tok2!=tk_semicolon&&tok2!=tk_camma*/);
//	}
}

void SkipBlock2()
{
	cha=cha2;
	inptr=inptr2;
	SkipBlock();
	cha2=cha;
	inptr=inptr2;
	linenum2=linenumber;
	FindStopTok();
}

void FindEndLex()
{
	while(tok2!=tk_semicolon&&tok!=tk_eof)nexttok();
}

void  preerror(char *str) /* error on currentline with line number and file name */
{
	preerror3(str,linenumber);
}

void  preerror3(char *str,unsigned int line,unsigned int file)//error message at a different than current line
{
	if(error<maxerrors){
		error++;
		sprintf((char *)string3,"%s(%d)#%d> %s.\n",startfileinfo==NULL?"":(startfileinfo+file)->filename,line,error,str);
		printf((char *)string3);
		if(errfile.file==NULL)errfile.file=fopen(errfile.name,"w+t");
		if(errfile.file!=NULL)fprintf(errfile.file,(char *)string3);
	}
	else exit(e_toomanyerrors);
}

void internalerror (char *str)// serious internal compiler error message
{
char buf[200];
	sprintf(buf,"*** SERIOUS COMPILER INTERNAL ERROR ***\n>%s",str);
	preerror(buf);
	printf("STRING:%s\nIDNAME:%s\n",string,itok.name);
	printf("TOK:%d SEGM:%d POST:%d RM:%d number:%ld\n",tok,itok.segm,itok.post,itok.rm,itok.number);
	printf("STRING2:%s\nIDNAME2:%s\n",string2,itok2.name);
	printf("TOK2:%d SEGM2:%d POST2:%d RM2:%d number2:%ld\n",tok2,itok2.segm,itok2.post,itok2.rm,itok2.number);
	printf("Out position:%04X\n",outptr);
	exit(e_internalerror);
}

char *getnumoperand(int type,char *name)
{
	switch(type){
		case 0:
			return "";
		case 1:
			strcpy(buferr,"1-st ");
			break;
		case 2:
			strcpy(buferr,"2-nd ");
			break;
		case 3:
			strcpy(buferr,"3-rd ");
			break;
		default:
			sprintf(buferr,"%d-th ",type%256);
			break;
	}
	strcat(buferr,name);
	return buferr;
}

void  expected (char ch)
{
char holdstr[80];
	sprintf(holdstr,"'%c' expected",ch);
	preerror(holdstr);
}

void numexpected(int type)
{
char buf[40];
	sprintf(buf,"%snumber expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void varexpected(int type)
{
char buf[45];
	sprintf(buf,"%svariable expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void stringexpected()
{
	preerror("string expected");
}

void valueexpected()
{
	preerror("value expected");
}

void wordvalexpected()
{
	preerror("word value expected");
}

void dwordvalexpected()
{
	preerror("dword value expected");
}

void qwordvalexpected()
{
	preerror("qword value expected");
}

void codeexpected()
{
	preerror("assembly opcode expected");
}

void operatorexpected()
{
	preerror("operator identifier expected");
}

void unexpectedeof()
{
	preerror("unexpected END OF FILE");
}

void swaperror()
{
	preerror("invalid or incompatable swap item");
}

void notexternfun()
{
	preerror("Do not insert extern function");
}

void idalreadydefined()
{
char holdstr[80];
	sprintf(holdstr,"identifier '%s' already defined",itok.name);
	preerror(holdstr);
	FindStopTok();
//	nexttok();
}

void  jumperror(unsigned int line,char *type)
{
char smalltype[IDLENGTH];
char buf[80];
	strcpy(smalltype,type);
	strlwr(smalltype);
	sprintf(buf,"'%s' jump distance too large, use '%s'",type,smalltype);
	preerror3(buf,line);
}

void unknowncompop()
{
	preerror("unknown comparison operator");
}

void maxoutputerror()
{
	preerror("maximum output code size exceeded");
	exit( e_outputtoobig );
}

void unableopenfile(char *name)
{
char holdstr[256];
	sprintf(holdstr,"unable to open file '%s'",name);
	preerror(holdstr);
}

void shortjumptoolarge()
{
	preerror(shorterr);
}

void thisundefined(char *str,int next)
{
char holdstr[80];
	sprintf(holdstr,"'%s' undefined",str);
	preerror(holdstr);

	if(next)FindStopTok();
}

void datatype_expected(int type)
{
char buf[45];
	sprintf(buf,"%smemory variable expected",getnumoperand(type,"operand "));
	preerror(buf);
	FindStopTok();
}

void illegalfloat()
{
	preerror("illegal use of float point");
}

void tobigpost()
{
	preerror("maximum size of post variables exceeded");
	postsize=0xFFFF;
}

void unuseableinput()
{
	preerror("unuseable input");
	FindStopTok();
}

void ManyLogicCompare()
{
	preerror("to many use logic compare");
}

void ZeroMassiv()
{
	preerror("size massiv unknown or zero");
}

void maxdataerror()
{
	preerror("maximum output data size exceeded");
	exit( e_outputtoobig );
}

void errorreadingfile(char *name)
{
char buf[256];
	sprintf(buf,"error reading from file '%s'",name);
	preerror(buf);
}

void badinfile(char *name)
{
char buf[256];
	sprintf(buf,"bad input file '%s'",name);
	preerror(buf);
}

void edpip(int num)
{
char buf[64];
//	preerror("error declare parameters in function");
	sprintf(buf,"error declare %sparameters in function",getnumoperand(num,""));
	preerror(buf);
}

void CompareOr()
{
	preerror("compare logic OR or AND to big distance");
}

void dynamiclabelerror()
{
	preerror("global labels illegal within dynamic functions");
}

void OnlyComFile()
{
	preerror("this option only for COM output files");
}

void redeclare(char *name)
{
char buf[120];
	sprintf(buf,"error redeclare function \"%s\"",name);
	preerror(buf);
}

void retvoid()
{
	preerror("function has return type of void");
}

void extraparam(char *name)
{
char buf[120];
	sprintf(buf,"extra parameter in function %s",name);
	preerror(buf);
}

void blockerror()
{
	preerror("illegal syntax within [ ]");
}

void block16_32error()
{
	preerror("only one of 16 or 32 bit allowed within [ ]");
}

void notstructname()
{
	preerror("unique struct name expected");
}

void badtoken()
{
char buf[80];
	if(displaytokerrors){
		sprintf(buf,"tokenizer: bad character value - '%c'",cha);
		preerror(buf);
	}
}

void expectederror(char *str)
{
char holdstr[80];
	if(displaytokerrors){
		sprintf(holdstr,"%s expected",str);
		preerror(holdstr);
	}
}

void declareanonim()
{
	preerror("Error declare anonimus union");
}

void declareunion()
{
	preerror("Error declare union");
}
/*
void not_union_static()
{
	preerror("'static' not use in 'union'");
} */

void segoperror()
{
	preerror("only '=' or '><' operands valid with segment register");
}

void segbyteerror()
{
	preerror("segment registers can not be used in byte or char math");
}

void regmathoperror()
{
	preerror("invalid operation for non-AX register math");
}

void begmathoperror()
{
	preerror("invalid operation for non-AL register math");
}

void negregerror()
{
	preerror("negative non-constant invalid for non-AX register math");
}

void regbyteerror()
{
	preerror("byte or char operands invalid for non-AX register math");
}

void begworderror()
{
	preerror("specified 16 bit operand invalid for non-AL register math");
}

void regshifterror()
{
	preerror("only CL or 1 valid for non AX or AL register bit shifting");
}

void regmatherror()
{
	preerror("invalid operand for non-AX register math");
}

void DevideZero()
{
	preerror("impossible to divide into ZERO");
}

void wordnotoper()
{
	preerror("word or int operands invalid for non-EAX register math");
}

void regexpected(int type)
{
char buf[50];
	sprintf(buf,"%sword register expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void bytevalexpected(int type)
{
char buf[50];
	sprintf(buf,"%sbyte value expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void shortjumperror()
{
	preerror("invalid operand for SHORT jump");
}

void invalidfarjumpitem()
{
	preerror("invalid operand for FAR jump");
}

void invalidfarcallitem()
{
	preerror("invalid operand for FAR call");
}

void begexpected(int type)
{
char buf[50];
	sprintf(buf,"%sbyte register expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void reg32expected(int type)
{
char buf[50];
	sprintf(buf,"%s32 bit register expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void reg32regexpected(int type)
{
char buf[50];
	sprintf(buf,"%s16 or 32 bit register expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void regBXDISIBPexpected()
{
	preerror("use only one of BX, DI, SI or BP register");
}

void bytedxexpected()
{
	preerror("byte constant or DX expected");
}

void axalexpected()
{
	preerror("EAX, AX or AL expected");
}

void invalidoperand(int type)
{
char buf[25];
	sprintf(buf,"%sinvalid",getnumoperand(type,"operand "));
	preerror(buf);
}

void mmxregexpected(int type)
{
char buf[50];
	sprintf(buf,"%sMMX register expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void xmmregexpected(int type)
{
char buf[50];
	sprintf(buf,"%sXMM register expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void xmmregorvarexpected(int type)
{
char buf[60];
	sprintf(buf,"%sXMM register or memory varible expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void mmxregordwordexpected(int type)
{
char buf[60];
	sprintf(buf,"%sMMX register or memory varible expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void clornumberexpected()
{
	preerror("CL or constant expected");
}

void fpuvarexpected(int type)
{
char buf[70];
	sprintf(buf,"%sexpected FPU register|long|dword|float var",getnumoperand(type,"operand "));
	preerror(buf);
}

void fpustakexpected(int type)
{
char buf[40];
	sprintf(buf,"%sexpected FPU register",getnumoperand(type,"operand "));
	preerror(buf);
}

void fpu0expected()
{
	preerror("2-nd expected only st(0) fpu register");
}

void fpustdestroed()
{
	preerror("FPU register more 6 destroed");
}

void errstruct()
{
	preerror("illegal use of struct");
}

void tegnotfound()
{
	preerror("tag struct not found");
}

void ErrWrite()
{
	//fprintf(stderr,"unable to write output file.\n");	printf("unable to write output file.\n");
}

void ErrReadStub()
{
	//fprintf(stderr,"unable to read stubfile\n");	printf("unable to read stubfile\n");
}

void InvOperComp()
{
	preerror("invalid operation for compare");
}

void mmxormem(int type)
{
char buf[60];
	sprintf(buf,"%sexpected mmx register or memory variable",getnumoperand(type,"operand "));
	preerror(buf);
}

void reg32orword(int type)
{
char buf[60];
	sprintf(buf,"%s32 bit register or word variable expected",getnumoperand(type,"operand "));
	preerror(buf);
}

void undefclass(char *name)
{
char buf[30+IDLENGTH];
	sprintf(buf,"Base class '%s' not defined",name);
	preerror(buf);
}

void unknowntype()
{
	preerror("unknown type");
}

void destrdestreg()
{
	preerror("destroyed destination register");
}

void unknownstruct (char *name,char *sname)
{
char buf[IDLENGTH*2+30];
	sprintf(buf,"unknown member '%s' in struct '%s'",name,sname);
	preerror(buf);
}

void unknowntagstruct(char *name)
{
char buf[IDLENGTH+16];
	sprintf(buf,"unknown tag '%s'",name);
	preerror(buf);
}

void unknownobj(char *name)
{
char buf[IDLENGTH+32];
	sprintf(buf,"unknown object for member '%s'",name);
	preerror(buf);
}
void q();
void unknownpragma(char *name)
{
char buf[IDLENGTH+32];
	sprintf(buf,"unknown parametr for #pragma %s",name);
	preerror(buf);
}

/*-----------------08.08.00 22:49-------------------
 Предупреждения
	--------------------------------------------------*/

void warningoptnum()
{
	if(wact[0].usewarn)wact[0].fwarn("Optimize numerical expressions",linenumber,currentfileinfo);
}

void  warningreg(char *str2)
{
char buf[50];
	if(wact[1].usewarn){
		sprintf(buf,"%s has been used by compiler",str2);		wact[1].fwarn(buf,linenumber,currentfileinfo);
	}
}

void  warningjmp(char *str2,int line,int file)
{
char buf[50];
	if(wact[2].usewarn){
		sprintf(buf,"Short operator '%s' may be used",str2);
		wact[2].fwarn(buf,line,file);
	}
}

void warningstring()
{
	if(wact[3].usewarn){
		sprintf((char *)string2,"String \"%s\" repeated",string3);
		wact[3].fwarn((char *)string2,linenumber,currentfileinfo);
	}
}

void warningexpand()
{
	if(wact[4].usewarn)wact[4].fwarn("Expansion variable",linenumber,currentfileinfo);
}

void warningretsign()
{
	if(wact[5].usewarn)wact[5].fwarn("Signed value returned",linenumber,currentfileinfo);
}

void warningprint(char *str,unsigned int line,unsigned int file)
{
	if(warning==TRUE){
		//if(wartype.name!=NULL&&wartype.file==stdout){
		//	if((wartype.file=fopen(wartype.name,"w+t"))==NULL)wartype.file=stdout;
		//}
		//fprintf(wartype.file,"%s(%d)> Warning! %s.\n",startfileinfo==NULL?"":(startfileinfo+file)->filename,line,str);		printf("%s(%d)> Warning! %s.\n",startfileinfo==NULL?"":(startfileinfo+file)->filename,line,str);
	}
}

void  warningdefined(char *name)
{
char buf[IDLENGTH+30];
	if(wact[6].usewarn){
		sprintf(buf,"'%s' defined above, therefore skipped",name);
		wact[6].fwarn(buf,linenumber,currentfileinfo);
	}
}

void warningnotused(char *name,int type)
{
char buf[IDLENGTH+40];
char *typenames[]={"Variable","Structure","Function","Local variable",
"Parameter variable","Local structure"};
	if(wact[7].usewarn){
		sprintf(buf,"%s '%s' possible not used",(char *)typenames[type],name);
		wact[7].fwarn(buf,linenumber,currentfileinfo);
	}
}

void warningusenotintvar(char *name)
{
char buf[IDLENGTH+50];
	if(wact[8].usewarn){
		sprintf(buf,"Non-initialized variable '%s' may have been used",name);
		wact[8].fwarn(buf,linenumber,currentfileinfo);
	}
}

void warningdestroyflags()
{
	if(wact[9].usewarn)wact[9].fwarn("Return flag was destroyed",linenumber,currentfileinfo);
}

void warningunreach()
{
	if(wact[10].usewarn)wact[10].fwarn("Code may not be executable",linenumber,currentfileinfo);
}

void warninline()
{
	if(wact[11].usewarn)wact[11].fwarn("Don't use local/parametric values in inline functions",linenumber,currentfileinfo);
}

void warnsize()
{
	if(wact[12].usewarn)wact[12].fwarn("Sources size exceed destination size",linenumber,currentfileinfo);
}

void waralreadinit(char *reg)
{
#ifdef BETTA
char buf[IDLENGTH+50];
	sprintf(buf,"Register %s already initialized",reg);
	warningprint(buf,linenumber,currentfileinfo);
#endif
}

void waralreadinitreg(char *reg,char *reg2)
{
#ifdef BETTA
char buf[IDLENGTH+50];
	sprintf(buf,"Register %s same as %s",reg,reg2);
	warningprint(buf,linenumber,currentfileinfo);
#endif
}

void warpragmapackpop()
{
	if(wact[13].usewarn)wact[13].fwarn("Pragma pack pop with no matching pack push",linenumber,currentfileinfo);
}

void missingpar(char *name)
{
char buf[120];
	if(wact[14].usewarn){
		sprintf(buf,"Missing parameter in function %s",name);
		wact[14].fwarn(buf,linenumber,currentfileinfo);
	}
//	preerror(buf);
}

void warreplasevar(char *name)
{
char buf[120];
//	if(usewarn[14]){
	if(displaytokerrors){
		sprintf(buf,"Variable %s is replased on constant",name);
		warningprint(buf,linenumber,currentfileinfo);
	}
//	}
//	preerror(buf);
}

void waralreadinitvar(char *name,unsigned int num)
{
char buf[120];
//	if(usewarn[14]){
	if(displaytokerrors){
		sprintf(buf,"Variable %s already initialized by constant %d",name,num);
		warningprint(buf,linenumber,currentfileinfo);
	}
//	}
//	preerror(buf);
}

void warcompneqconst()
{
	warningprint("Comparison not equal constant. Skipped code",linenumber,currentfileinfo);
}

void warcompeqconst()
{
	warningprint("Comparison equal constant. Skipped compare",linenumber,currentfileinfo);
}

void warpointerstruct()
{
	warningprint("Compiler not support pointers on structure",linenumber,currentfileinfo);
}

void warESP()
{
	warningprint("ESP has ambiguous value",linenumber,currentfileinfo);
}

/* *****************   map file *************** */

void OpenMapFile()
{
char buf[256];
	sprintf(buf,"%s.map",rawfilename);
	hmap=fopen(buf,"w+t");
}

char *GetRetType(int type,int flag)
{
char *retcode;
	if(flag&f_interrupt)return "";
	switch(type){
		case tk_bytevar:
		case tk_byte:
			retcode="byte ";
			break;
		case tk_charvar:
		case tk_char:
			retcode="char ";
			break;
		case tk_wordvar:
		case tk_word:
			retcode="word ";
			break;
		case tk_longvar:
		case tk_long:
			retcode="long ";
			break;
		case tk_dwordvar:
		case tk_dword:
			retcode="dword ";
			break;
		case tk_doublevar:
		case tk_double:
			retcode="double ";
			break;
		case tk_fpust:
			retcode="fpust ";
			break;
		case tk_floatvar:
		case tk_float:
			retcode="float ";
			break;
		case tk_qwordvar:
		case tk_qword:
			retcode="qword ";
			break;
		case tk_void:
			retcode="void ";
			break;
/*		case tk_intvar:
		case tk_int:
			retcode="int ";
			break;*/
		case tk_structvar:
			retcode="struct";
			break;
		default:
			retcode="int ";
			break;
	}
	return retcode;;
}

char *GetTypeProc(int flag)
{
char *retcode;
char *t;
	string2[0]=0;
	if(flag&f_interrupt)return "interrupt";
	if(flag&f_far)strcat((char *)string2,"far ");
	if(flag&f_extern)strcat((char *)string2,"extern ");
	if(flag&f_export)strcat((char *)string2,"_export ");
	if(flag&f_inline)strcat((char *)string2,"inline ");
	if(flag&f_static)strcat((char *)string2,"static ");
	if(flag&f_retproc){
		t="";
		switch(((flag&f_retproc)>>8)+tk_overflowflag-1){
			case tk_overflowflag:
				t="OVERFLOW ";
				break;
			case tk_notoverflowflag:
				t="NOTOVERFLOW ";
				break;
			case tk_carryflag:
				t="CARRYFLAG ";
				break;
			case tk_notcarryflag:
				t="NOTCARRYFLAG ";
				break;
			case tk_zeroflag:
				t="ZEROFLAG ";
				break;
			case tk_notzeroflag:
				t="NOTZEROFLAG ";
				break;
			case tk_minusflag:
				t="MINUSFLAG ";
				break;
			case tk_plusflag:
				t="PLUSFLAG ";
				break;
		}
		strcat((char *)string2,t);
	}
	switch(flag&f_typeproc){
		case tp_pascal:
			t="pascal ";
			break;
		case tp_cdecl:
			t="cdecl ";
			break;
		case tp_stdcall:
			t="stdcall ";
			break;
		case tp_fastcall:
			t="fastcall ";
			break;
	}
	strcat((char *)string2,t);
	return (char *)string2;
}

char *GetFunParam(unsigned char c,unsigned char c2,unsigned char c3)
{
	switch(c){
		case 'B':
			if(c2>=0x30&&c2<=0x37)return begs[c2-0x30];
			return "byte";
		case 'W':
			if(c2>=0x30&&c2<=0x37)return regs[0][c2-0x30];
			return "word";
		case 'D':
			if(c2>=0x30&&c2<=0x37)return regs[1][c2-0x30];
			return "dword";
		case 'C': return "char";
		case 'I': return "int";
		case 'L': return "long";
		case 'F': return "float";
		case 'A': return "...";
		case 'Q':
			if(c2>=0x30&&c2<=0x37){
				sprintf((char *)string2,"%s:%s",regs[1][c2-0x30],regs[1][c3-0x30]);
				return (char *)string2;
			}
			return "qword";
		case 'E': return "double";
		case 'S':
			if(c2>=0x30&&c2<=0x37){
				sprintf((char *)string2,"st(%c)",c2);
				return (char *)string2;
			}
			return "fpust";
		case 'T': return "struct";
		case 'U': return "void";
	}
	return "";;
}

char *GetName(char *name,int flag)
{
char *tn;
char iname[IDLENGTH];
	strcpy(iname,name);
	if((tn=strchr(iname,'@'))!=NULL)*tn=0;
	if(flag&fs_constructor)sprintf((char *)string3,"%s::%s",iname,iname);
	else if(flag&fs_destructor)sprintf((char *)string3,"%s::~%s",iname,iname);
	else if(flag&f_classproc)sprintf((char *)string3,"%s::%s",searchteg->name,iname);
	else strcpy((char *)string3,iname);
	return (char *)string3;
}

char *GetSizeVar(int type,int adr)
{
char *reg;
char *sign;
	if(type==tp_postvar||type==tp_gvar)return "DS:???";
	if(am32){
		if(ESPloc)reg="ESP";
		else reg="EBP";
	}
	else reg="BP";
	if(adr<0)sign="";
	else sign="+";
	sprintf((char *)string2,"%s%s%d",reg,sign,adr);
	return (char *)string2;
}

void GetRangeUsed(char *buf,localinfo *ptr)
{
	if(ptr->count==0)buf[0]=0;
	else if(ptr->count==1)sprintf(buf,"%d",ptr->usedfirst);
	else sprintf(buf,"%d-%d",ptr->usedfirst,ptr->usedlast);
}

void mapfun(int line)
{
treelocalrec *ftlr;
struct localrec *ptr;
int i,fheader;
char buf[32];
	if(hmap==NULL)OpenMapFile();
	if(hmap){
		fprintf(hmap,"\n%s%s%s(",GetTypeProc(itok.flag),GetRetType(itok.rm,itok.flag),GetName(itok.name,itok.flag));
		for(i=0;;i++){
			unsigned char c=string[i];
			if(c==0)break;
			if(c>=0x30&&c<=0x37)continue;
			if(i)fputc(',',hmap);
			unsigned char c2=string[i+1];
			unsigned char c3=string[i+2];
			fputs(GetFunParam(c,c2,c3),hmap);
		}
		fputc(')',hmap);
		fprintf(hmap,"\nlocation: line %d-%d file %s",line,linenumber,startfileinfo==NULL?"":(startfileinfo+currentfileinfo)->filename);
		fprintf(hmap,"\noffset=0x%X(%d)\tsize=0x%X(%d)",itok.number,itok.number,itok.size,itok.size);
	}
	fheader=0;
	for(ftlr=btlr;ftlr!=NULL;ftlr=ftlr->next){
		for(ptr=ftlr->lrec;ptr!=NULL;ptr=ptr->rec.next){
			i=ptr->rec.type;
			if(i==tp_postvar||i==tp_gvar||i==tp_localvar||i==tp_paramvar){
				if(!fheader){
					fputs("\nType    Address   Used     Count  Size    Name",hmap);
					fputs("\n----------------------------------------------",hmap);
					fheader=TRUE;
				}
				GetRangeUsed(buf,&ptr->li);
				fprintf(hmap,"\n%-8s%-10s%-12s%-4d%-8d",GetRetType(ptr->rec.rectok,0),GetSizeVar(ptr->rec.type,ptr->rec.recnumber),buf,ptr->li.count,ptr->rec.recsize);
				if(ptr->rec.npointr)for(i=ptr->rec.npointr;i>0;i--)fputc('*',hmap);
				fputs(ptr->rec.recid,hmap);
			}
		}
	}
	fputs("\n",hmap);
}

