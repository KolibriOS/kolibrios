#define _CLASS_

#include "tok.h"

structteg *searchteg=NULL;
int destructor=FALSE;

void notclassname(char *name)
{
char buf[90];
	sprintf(buf,"'%s' not class name",name);
	preerror(buf);
	nexttok();	//пропуск ::
	nexttok();
}

void notclassproc(char *classname, char* procname)
{
char buf[160];
	sprintf(buf,"'%s' not member class '%s'",procname,classname);
	preerror(buf);
}

void AddThis()
{
	int lsize=(am32==TRUE?4:2);
	localrec *lrec=addlocalvar("this",(am32==TRUE?tk_dwordvar:tk_wordvar),paramsize);
	lrec->rec.recsize=lsize;
	lrec->rec.type=tp_paramvar;
	lrec->fuse=USEDVAR;
	paramsize+=lsize;
}

void doclassproc(unsigned int tproc)
{
int type=itok.rm;	//тип возврата
unsigned int flag=itok.flag;
unsigned int npointr=itok.npointr;
char classname[IDLENGTH];
	if((searchteg=FindTeg(TRUE,itok.name))!=NULL){
		strcpy(classname,itok.name);
		nexttok();	//пропуск ::
		if(tok2==tk_tilda){
			nexttok();
			destructor=TRUE;
			flag|=fs_destructor;
			type=tk_void;
		}
		nexttok();
		char *tn;
		char name[IDLENGTH];
		strcpy(name,itok.name);
		if((tn=strchr(name,'@'))!=NULL)*tn=0;

		if(strcmp(classname,name)==0&&(flag&fs_destructor)==0)flag|=fs_constructor;
		if((tok!=tk_declare&&tok!=tk_undefproc)||(itok.flag&f_classproc)==0){
			notclassproc(classname,name);
			searchteg=NULL;
			define_procedure();
			return;
		}
		if(tproc==0){
			if(CidOrID()==tk_ID)tproc=tk_fastcall;
			else tproc=(comfile==file_w32?tk_stdcall:tk_pascal);
		}
		flag|=(tproc-tk_pascal)*2;
		if(type==tokens)type=am32==FALSE?tk_word:tk_dword;
		if(flag!=itok.flag||type!=itok.rm||(unsigned short)npointr!=itok.npointr){
//		printf("flag %08X - %08X\n",flag,itok.flag);
//		printf("type %u - %u\n",type,itok.rm);
			redeclare(name);
		}
		if(dynamic_flag){
			dynamic_proc();
			searchteg=NULL;
			return;
		}
//		if(itok.flag&f_static)searchteg=NULL;
		if(AlignProc!=FALSE)AlignCD(CS,alignproc);
		if(dbg)AddLine();
		setproc(1);
	}
	else{
		notclassname(itok.name);
		setproc(0);
	}
	dopoststrings();
}
