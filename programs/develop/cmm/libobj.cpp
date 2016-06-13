#include "tok.h"
#include "coff.h"

char *meserr="Plese, send this obj and sources files to me (sheker@mail.ru)";
struct LISTINCLFILE{
	char *name;
	int typefind;
	int type;
};

enum{
	s_extern,
	s_code,
	s_data,
	s_bss
};

struct LISTNAMESYMBOL{
	int idx;	//индекс имени
	int seg;
	int adr;
	idrec *rec;
	int dbg;
	char name[IDLENGTH];
}*listnamesymbol;

char *curobjname;

int numobj=0,numname;
LISTINCLFILE *listobj;
unsigned int adr_end;

void LoadObj();
void AddPost(unsigned long loc,int type,unsigned int num);
int searchtree2(idrec *fptr,ITOK *itok4,int *tok4,unsigned char *string4);

int GetSegm(int seg)
{
OBJECT_ENTRY *obj;
	if(seg){
		seg--;
		obj=(OBJECT_ENTRY *)(input+(sizeof(COFF_HEADER)+sizeof(OBJECT_ENTRY)*seg));
		seg=obj->vsize;
	}
	return seg;
}

int GetOffSec(int seg)
{
OBJECT_ENTRY *obj;
	if(seg){
		seg--;
		obj=(OBJECT_ENTRY *)(input+(sizeof(COFF_HEADER)+sizeof(OBJECT_ENTRY)*seg));
		seg=obj->sectionRVA;
	}
	return seg;
}

void AddNameObj(char *name,int typefind,int type)
{
int len;
	len=strlen(name);
	if(!numobj)listobj=(LISTINCLFILE *)MALLOC(sizeof(LISTINCLFILE));
	else listobj=(LISTINCLFILE *)REALLOC(listobj,sizeof(LISTINCLFILE)*(numobj+1));
	(listobj+numobj)->name=BackString(name);
	(listobj+numobj)->typefind=typefind;
	(listobj+numobj)->type=type;
	numobj++;
}

void AddObj()
{
int hold;
	for(int i=0;i<numobj;i++){
		curobjname=(listobj+i)->name;
//		puts(curobjname);
		hold=loadfile(curobjname,(listobj+i)->typefind);
		if(hold==1||hold==-1){
			unableopenfile(curobjname);
			continue;
		}
		if((listobj+i)->type==0)LoadObj();
		free(input);
	}
}

int GetAlighSize(long flag)
{
int align=0;
	int i=(flag&IMAGE_SCN_ALIGN_MASK)>>20;
	if(i)align=1<<(i-1);
	if(align==1)align=0;
	return align;
}

void ConvertName(char *name)
{
int i1,i2;
char c;
	tok=tk_ID;
	i1=i2=0;
	c=name[i1];
	if(c=='?'){
		i1++;
		c=name[i1];
		if(c=='?'){
			i1=2;
			c=name[i1];
		}
	}
	else if(c=='_'){
		if(strncmp(name,"__imp__",7)==0){
			i1=7;
			c=name[i1];
		}
		else if(name[1]=='$'){
			i1=2;
			c=name[i1];
		}
	}
	while(c!=0){
		if(c>='a'&&c<='z')tok=tk_id;
		name[i2]=c;
		i1++;
		i2++;
		c=name[i1];
		if(c=='@'||c=='$')break;
	}
	name[i2]=0;
}

void AddPost(unsigned long loc,int type,unsigned int num)
{
	CheckPosts();
	(postbuf+posts)->num=num;
	(postbuf+posts)->type=type;
	(postbuf+posts)->loc=loc;
	posts++;
}

void ScanSection(int numobj)
{
int poutptr,poutptrdata,ppostsize,align;
OBJECT_ENTRY *obj;
long flag;
	poutptr=outptr;
	poutptrdata=outptrdata;
	ppostsize=postsize;
	for(int i=0;i<numobj;i++){
		obj=(OBJECT_ENTRY *)(input+(sizeof(COFF_HEADER)+sizeof(OBJECT_ENTRY)*i));
		flag=obj->flags;
		align=GetAlighSize(flag);
		obj->vsize=-1;
		if((flag&IMAGE_SCN_CNT_CODE)&&(flag&IMAGE_SCN_MEM_EXECUTE)){//секция кода
			obj->vsize=s_code;
			obj->sectionRVA=poutptr=Align(poutptr,align);
			poutptr+=obj->psize;
			if(splitdata==FALSE)poutptrdata=poutptr;
		}
		else{
			if((flag&IMAGE_SCN_MEM_READ)&&(flag&IMAGE_SCN_MEM_WRITE)){
				if(flag&IMAGE_SCN_CNT_INITIALIZED_DATA){	//секция инициализированных данных
					obj->vsize=s_data;
					obj->sectionRVA=poutptrdata=Align(poutptrdata,align);
					poutptrdata+=obj->psize;
					if(splitdata==FALSE)poutptr=poutptrdata;
				}
				else if(flag&IMAGE_SCN_CNT_UNINITIALIZED_DATA){	//секция bss
					obj->vsize=s_bss;
					obj->sectionRVA=ppostsize=Align(postsize,align);
					postsize+=obj->psize;
				}
			}
		}
	}
}

void AddLstName(int numname)
{
unsigned int minofs;
int i,j,minname;
int oseg;
unsigned int ooutptr,ooutptrdata;
	ooutptr=outptr;
	ooutptrdata=outptrdata;
	for(i=0;i<numname;i++){
		if((listnamesymbol+i)->dbg==0){
			oseg=(listnamesymbol+i)->seg;
			minofs=(listnamesymbol+i)->adr;
			minname=i;
			for(j=i+1;j<numname;j++){
				if(splitdata==FALSE)goto checkadr;
				else if((listnamesymbol+j)->seg==oseg){
checkadr:
					if(minofs>=(listnamesymbol+j)->adr&&(listnamesymbol+j)->dbg==0){
						if(minofs==(listnamesymbol+j)->adr){
							(listnamesymbol+minname)->dbg=1;
						}
						minofs=(listnamesymbol+j)->adr;
						minname=j;
						oseg=(listnamesymbol+j)->seg;
					}
				}
			}
			if(minname!=i)i--;
//			printf("seg=%d adr=%d %s\n",oseg,(listnamesymbol+minname)->adr,(listnamesymbol+minname)->name);
			if(oseg==s_code){
				if((listnamesymbol+minname)->rec){
					idrec *ptr;
					ptr=(listnamesymbol+minname)->rec;
					if(ptr->rectok==tk_undefproc||ptr->rectok==tk_proc||ptr->rectok==tk_interruptproc)goto dproc;
					outptrdata=(listnamesymbol+minname)->adr;
					if(splitdata==FALSE)outptr=outptrdata;
					AddDataNullLine(3,(listnamesymbol+minname)->name);
				}
				else{
dproc:
					outptr=(listnamesymbol+minname)->adr;//12.08.04 00:05 +ooutptr;
					AddCodeNullLine((listnamesymbol+minname)->name);
				}
			}
			else if(oseg==s_data){
				outptrdata=(listnamesymbol+minname)->adr;//12.08.04 01:19 +ooutptrdata;
				if(splitdata==FALSE)outptr=outptrdata;
				AddDataNullLine(3,(listnamesymbol+minname)->name);
			}
			(listnamesymbol+minname)->dbg=1;
		}
	}
	outptr=ooutptr;
	outptrdata=ooutptrdata;
}

int ScanName(COFF_HEADER *head)
{
int size,i,j,numname,seg,count;
IMAGE_SYMBOL *tsym;
unsigned int ooutptr,ooutptrdata,opostsize;
idrec *rec;
	ooutptr=outptr;
	ooutptrdata=outptrdata;
	opostsize=postsize;
	numname=head->COFFsize;
	size=numname*sizeof(IMAGE_SYMBOL)+head->pCOFF;
	listnamesymbol=(LISTNAMESYMBOL *)MALLOC(sizeof(LISTNAMESYMBOL)*numname);
	for(i=0,j=0;i<numname;i++){
		tsym=(IMAGE_SYMBOL *)(input+head->pCOFF+i*sizeof(IMAGE_SYMBOL));
		(listnamesymbol+j)->rec=0;
		if(tsym->SectionNumber>=0){
			(listnamesymbol+j)->idx=i;
			if(tsym->N.Name.Short==0){
				strcpy((char *)string,(char *)(input+size+tsym->N.Name.Long));
			}
			else{
				strncpy((char *)string,(char *)tsym->N.sname,8);
				string[8]=0;
			}
			//преобразовать имя
			ConvertName((char *)string);
			seg=(listnamesymbol+j)->seg=GetSegm(tsym->SectionNumber);
//			printf("find name \"%s\" seg=%d type=%d class=%d\n",(char *)string,seg,tsym->Type,tsym->StorageClass);
			(listnamesymbol+j)->adr=tsym->Value+GetOffSec(tsym->SectionNumber);
			(listnamesymbol+j)->dbg=0;
			strcpy((listnamesymbol+j)->name,(char *)string);
			if(seg==s_data){
				outptrdata=(listnamesymbol+j)->adr;//12.08.04 01:20 +ooutptrdata;
				count=FindOff(string,DS);
			}
			else if(seg==s_bss){
				postsize=(listnamesymbol+j)->adr;//12.08.04 01:20 +opostsize;
				count=FindOff(string,VARPOST);
			}
			else if(seg==s_code){
				outptr=(listnamesymbol+j)->adr;//11.08.04 23:59 +ooutptr;
				count=FindOff(string,CS);
//				printf("adr=%08X count=%d\n",outptr,count);
			}
			displaytokerrors=FALSE;
			if(searchtree2(treestart,&itok,&tok,string)){
				rec=(listnamesymbol+j)->rec=itok.rec;
				rec->count+=count;
				if(tok==tk_undefproc||tok==tk_declare){
					tok=tk_proc;
					itok.number=outptr;
//					printf("%08X %08X %s\n",(listnamesymbol+j)->adr,ooutptr,itok.name);
					updatecall(updatetree(),(unsigned int)itok.number,0);
				}
				else if(tok!=tk_apiproc&&tsym->SectionNumber!=0)idalreadydefined();
//				if(tok==tk_proc||(tok>=tk_charvar&&tok<=tk_floatvar))
//				printf("tok=%d %s\n",tok,itok.name);
			}
			else{
				if(seg==s_extern/*&&tsym->Type==32*/){	//внешний объект любого типа
					strcpy(itok.name,(char *)string);
//					printf("undef proc \"%s\"\n",itok.name);
					string[0]=0;
					itok.flag=(unsigned char)(tok==tk_ID?tp_fastcall:(comfile==file_w32?tp_stdcall:tp_pascal));
					tok=tk_undefproc;
					itok.number=secondcallnum;
					itok.segm=NOT_DYNAMIC;
					itok.rm=tk_void;
					itok.post=0;
					rec=(listnamesymbol+j)->rec=addtotree(itok.name);
					rec->count=count;
					secondcallnum++;
				}
				else{
//					printf("type=%d seg=%d\n",tsym->Type,seg);
					if(/*tsym->Type==32*/tsym->StorageClass!=3&&seg==s_code){
						tok=tk_proc;
						itok.number=outptr;
						itok.segm=NOT_DYNAMIC;
						itok.rm=tk_void;
						itok.post=0;
						strcpy(itok.name,(char *)string);
						string[0]=0;
						itok.flag=(unsigned char)(tok==tk_ID?tp_fastcall:(comfile==file_w32?tp_stdcall:tp_pascal));
						rec=(listnamesymbol+j)->rec=addtotree(itok.name);
						rec->count=count;
					}
					else{
						(listnamesymbol+j)->rec=NULL;
//						printf("undef \"%s\"\n",string);
						if(seg==s_extern){
//							puts("323");
							preerror(meserr);
//			printf("Type=%d Class=%d seg=%d %s\n",tsym->Type,tsym->StorageClass,seg,string);
//							printf("??? %s\n",string);
						}
					}
				}
			}
//			printf("Type=%d Class=%d %s\n",tsym->Type,tsym->StorageClass,string);
			j++;
		}
		i+=tsym->NumberOfAuxSymbols;
	}
	if(j!=numname){
		listnamesymbol=(LISTNAMESYMBOL *)REALLOC(listnamesymbol,sizeof(LISTNAMESYMBOL)*j);
	}
	outptr=ooutptr;
	outptrdata=ooutptrdata;
	postsize=opostsize;
	if(dbg&2)AddLstName(j);
	return j;
}

void LoadObj()
{
COFF_HEADER *head;
int numobj,align,size,i,j,numrel;
OBJECT_ENTRY *obj;
unsigned char *buf;
IMAGE_RELOCATION *trel;
	head=(COFF_HEADER *)input;
	if(head->cpu!=0x14C){
		sprintf((char *)string,"file %s is not supported format",curobjname);
		preerror((char *)string);
		return;
	}
	if(comfile!=file_w32){
		preerror("include obj file posible only for windows programs");
		return;
	}
	linenumber=0;
	numobj=head->numobj;
	ScanSection(numobj);
	numname=ScanName(head);
	for(i=0;i<numobj;i++){
		obj=(OBJECT_ENTRY *)(input+(sizeof(COFF_HEADER)+sizeof(OBJECT_ENTRY)*i));
		align=GetAlighSize(obj->flags);
		size=obj->psize;
		adr_end=outptr+size;
		numrel=obj->NumberOfRelocations;
		trel=(IMAGE_RELOCATION *)(input+obj->PointerToRelocations);
		buf=(unsigned char *)(input+obj->pOffset);
//		printf("type=%d section=%d\n",(trel+j)->Type,obj->vsize);
		switch(obj->vsize){
			case s_code:
				if(align)AlignCD(CS,align);
				adr_end=Align(outptr+size,4);
				for(j=0;j<numrel;j++){
					int adr;
					LISTNAMESYMBOL *lns;
					int ii;
					for(ii=0;ii<numname;ii++){
						if((listnamesymbol+ii)->idx==(trel+j)->SymbolTableIndex)break;
					}
					lns=(LISTNAMESYMBOL *)(listnamesymbol+ii);
					idrec *rec=lns->rec;
//					printf("type=%d seg=%d tok=%d num=%d %s\n",(trel+j)->Type,lns->seg,rec->rectok,rec->recnumber,lns->name);
					switch((trel+j)->Type){
						case IMAGE_REL_I386_DIR32:
							*(long *)&buf[(trel+j)->VirtualAddress]+=lns->adr;
							switch(lns->seg){
								case s_code:
									adr=(trel+j)->VirtualAddress+outptr;
									AddPost(adr,FIX_CODE32,0);
									break;
								case s_data:
									adr=(trel+j)->VirtualAddress+outptrdata;
									AddPost(adr,FIX_VAR32,0);
									break;
								case s_bss:
									adr=(trel+j)->VirtualAddress+outptr;
									AddPost(adr,POST_VAR32,0);
									*(long *)&buf[(trel+j)->VirtualAddress]+=postsize;
									break;
								default:
									if(rec!=NULL){
										adr=(trel+j)->VirtualAddress+outptr;
										if(rec->rectok==tk_apiproc){
											AddPost(adr,CALL_32I,rec->recnumber);
										}
										else if(rec->rectok==tk_undefproc){
											AddPost(adr,CALL_32,rec->recnumber);
										}
										else if(rec->rectok==tk_proc){
											AddPost(adr,FIX_CODE_ADD,adr_end);
											AddPost(adr_end,FIX_CODE32,0);
											if(dbg&2){
												int ooutptr=outptr;
												outptr=adr_end;
												AddDataNullLine(4,lns->name);
												outptr=ooutptr;
											}
											*(long *)&output[adr_end]=rec->recnumber;
											adr_end+=4;
										}
										rec->count++;
									}
									break;
							}
							break;
						case IMAGE_REL_I386_REL32:
							adr=(trel+j)->VirtualAddress+outptr;
							if(rec!=NULL){
								if(rec->rectok!=tk_proc){
									AddPost(adr,CALL_32,rec->recnumber);
								}
								else{
									*(long *)&buf[(trel+j)->VirtualAddress]=rec->recnumber-adr-4;
								}
							}
							else{
								*(long *)&buf[(trel+j)->VirtualAddress]=lns->adr-adr-4;
							}
							if(rec)rec->count++;
							break;
						default:
//							puts("432");
							preerror(meserr);
							break;
					}
				}
/*				if(dbg&2){
					sprintf((char *)string,"code from %s",curobjname);
					AddCodeNullLine((char *)string);
				}*/
				for(j=0;j<size;j++)op(buf[j]);
				outptr=adr_end;
				break;
			case s_data:
				if(align)AlignCD(DS,align);
				for(j=0;j<numrel;j++){
					int adr;
					LISTNAMESYMBOL *lns;
					int ii;
					for(ii=0;ii<numname;ii++){
						if((listnamesymbol+ii)->idx==(trel+j)->SymbolTableIndex)break;
					}
					lns=(LISTNAMESYMBOL *)(listnamesymbol+ii);
					idrec *rec=lns->rec;
					switch((trel+j)->Type){
						case IMAGE_REL_I386_DIR32:
							*(long *)&buf[(trel+j)->VirtualAddress]+=lns->adr;
							switch(lns->seg){
								case s_code:
									adr=(trel+j)->VirtualAddress+outptr;
									AddPost(adr,FIX_CODE32,0);
									break;
								case s_data:
									adr=(trel+j)->VirtualAddress+outptrdata;
									AddPost(adr,FIX_VAR32,0);
									break;
								case s_bss:
									adr=(trel+j)->VirtualAddress+outptr;
									AddPost(adr,POST_VAR32,0);
									*(long *)&buf[(trel+j)->VirtualAddress]+=postsize;
									break;
								default:
//									puts("472");
									preerror(meserr);
/*									if(rec!=NULL){
										adr=(trel+j)->VirtualAddress+outptr;
										if(rec->rectok==tk_apiproc){
											AddPost(adr,CALL_32I,rec->recnumber);
										}
										else if(rec->rectok==tk_undefproc){
											AddPost(adr,CALL_32,rec->recnumber);
										}
										rec->count++;
									}*/
									break;
							}
							break;
						case IMAGE_REL_I386_REL32:
//							puts("488");
							preerror(meserr);
/*							adr=(trel+j)->VirtualAddress+outptr;
							if(rec!=NULL){
								if(rec->rectok!=tk_proc){
									AddPost(adr,CALL_32,rec->recnumber);
								}
								else{
									*(long *)&buf[(trel+j)->VirtualAddress]=rec->recnumber-adr-4;
								}
							}
							else{
								*(long *)&buf[(trel+j)->VirtualAddress]=lns->adr-adr-4;
							}
							if(rec)rec->count++;*/
							break;
					}
				}
				for(j=0;j<size;j++)opd(buf[j]);
				break;
			case s_bss:
				if(align)postsize=Align(postsize,align);
				postsize+=size;
				break;
		}
	}
}
