#include "tok.h"

void  obj_outrecord(int recordtype,unsigned int recordlength,unsigned char *data);
void outeachPUBDEF(struct idrec *ptr);
void  obj_outLEDATA(unsigned int segm,unsigned int offset,unsigned int recordlength,
			 unsigned char *data);

#define MAXNUMEXTNAME 1024;

int *numextname;
int maxnumextname=MAXNUMEXTNAME;
unsigned int lenextstr=0;	//длина строки с внешними именами
int numextern=0;

int postseg,stackseg;

unsigned int findextname(int extnum)
{
	for(unsigned int i=0;i<externnum;i++){
		if(numextname[i]==extnum)return i+1;
	}
	return 0;
}

int MakeObj()
{
unsigned int i;
unsigned int count,sizeblock;
	hout=CreateOutPut("obj","wb");
	i=strlen(startfileinfo->filename);
	string2[0]=(unsigned char)i;
	strcpy((char *)&string2[1],startfileinfo->filename);
	obj_outrecord(0x80,i+1,&string2[0]);// output the LNAMES
	sprintf((char *)&string2[3],"%s %s",compilerstr,__DATE__);
	i=strlen((char *)&string2[3]);
	*(short *)&string2[0]=0;
	string2[2]=(unsigned char)i;
	obj_outrecord(0x88,i+3,&string2[0]);// output the LNAMES
	for(count=0;count<totalmodule;count++){	//имена включаемых файлов
		*(struct ftime *)&string2[2]=(startfileinfo+count)->time;
		strcpy((char *)&string2[7],(startfileinfo+count)->filename);
		i=strlen((startfileinfo+count)->filename);
		*(short *)&string2[0]=0xE940;
		string2[6]=(unsigned char)i;
		obj_outrecord(0x88,i+7,&string2[0]);// output the LNAMES
	}
	count=outptr-startptr;	//размер кода
	unsigned char *data=output+startptr;	//начало данных
	*(short *)&string2[0]=0xE940;
	obj_outrecord(0x88,2,&string2[0]);//конец коментарий
	if(!am32){
		*(short *)&string2[0]=0xEA00;
		string2[2]=1;
		string2[3]=(unsigned char)(modelmem==SMALL?9:8);
		obj_outrecord(0x88,4,&string2[0]);
	}
	else{
		*(short *)&string2[0]=0xA140;
		obj_outrecord(0x88,2,&string2[0]);
	}
	obj_outrecord(0x96,39,(unsigned char *)"\000\005_TEXT\004CODE\004_BSS\003BSS\006DGROUP\005_DATA\004DATA");
// output the SEGDEF
	if(!am32){
		string2[0]=(unsigned char)0x28;
		*(short *)&string2[1]=(short)outptr;//count;// Set the length of the segment of DATA or CODE
		string2[3]=0x02;	//имя сегмента _TEXT
		*(short *)&string2[4]=0x0103;	//класс CODE Overlay NONE 1
		obj_outrecord(0x98,6,string2);
		i=2;

		if(comfile==file_exe&&modelmem==SMALL){
			string2[0]=(unsigned char)0x48;
			*(short *)&string2[1]=outptrdata;// Set the length of the segment DATA
			string2[3]=0x07;	//имя сегмента _DATA
			*(short *)&string2[4]=0x0108;	//класс DATA Overlay NONE
			obj_outrecord(0x98,6,string2);
			i++;
		}

		postseg=i;
		string2[0]=(unsigned char)0x48;
		*(short *)&string2[1]=(short)postsize;// Set the length of the segment BSS
		string2[3]=0x04;	//имя сегмента _BSS
		*(short *)&string2[4]=0x0105;	//класс BSS Overlay NONE
		obj_outrecord(0x98,6,string2);
		i++;

		if(comfile==file_exe&&modelmem==SMALL){
			obj_outrecord(0x96,6,(unsigned char *)"\005STACK");
			string2[0]=0x74;
			*(short *)&string2[1]=(short)stacksize;// Set the length of the segment STACK
			string2[3]=0x09;	//имя сегмента STACK
			*(short *)&string2[4]=0x0109;	//класс STACK Overlay NONE
			obj_outrecord(0x98,6,string2);
			stackseg=i;
		}
		string2[0]=6;	//имя DGROUP
		if(comfile==file_exe&&modelmem==SMALL){
			*(short *)&string2[1]=0x2FF;
			*(short *)&string2[3]=0x3FF;//postseg*256+255;//0x3FF;
			*(short *)&string2[5]=0x4ff;//stackseg*256+255;//0x4FF;
			i=7;
		}
		else{
			*(short *)&string2[1]=0x1FF;
//			*(short *)&string2[3]=0x2FF;
			*(short *)&string2[3]=0x2ff;//postseg*256+255;//0x3FF;
			i=5;
		}
		obj_outrecord(0x9A,i,string2);
	}
	else{
		string2[0]=(unsigned char)0xA9;
		*(long *)&string2[1]=(long)outptr;//count;// Set the length of the segment of DATA or CODE
		string2[5]=0x02;	//имя сегмента _TEXT
		*(short *)&string2[6]=0x0103;	//класс CODE Overlay NONE
		obj_outrecord(0x99,8,string2);
		i=2;
/*
		string2[0]=(unsigned char)0xA9;
		*(long *)&string2[1]=0;// Set the length of the segment DATA
		string2[5]=0x07;	//имя сегмента _DATA
		*(short *)&string2[6]=0x0108;	//класс DATA Overlay NONE
		obj_outrecord(0x99,8,string2);
		i++;*/

		postseg=i;
		string2[0]=(unsigned char)0xA9;
		*(long *)&string2[1]=(long)postsize;// Set the length of the segment BSS
		string2[5]=0x04;	//имя сегмента _BSS
		*(short *)&string2[6]=0x0105;	//класс BSS Overlay NONE
		obj_outrecord(0x99,8,string2);
		i++;

		obj_outrecord(0x96,11,(unsigned char *)"\005STACK\004FLAT");//9,10

		if(comfile!=file_w32){
			string2[0]=0x75;
			*(long *)&string2[1]=(long)stacksize;// Set the length of the segment STACK
			string2[5]=0x09;	//имя сегмента STACK
			*(short *)&string2[6]=0x0109;	//класс STACK Overlay NONE
			obj_outrecord(0x99,8,string2);
			stackseg=i;
		}
		string2[0]=10;

		obj_outrecord(0x9A,1,string2);	//GRPDEF Group: FLAT
		string2[0]=6;	//имя DGROUP
		i=1;
//		*(short *)&string2[i]=0x2FF;	//DATA
//		i+=2;
		*(short *)&string2[i]=postseg*256+255;//0x3FF;	//BSS
		i+=2;
		if(comfile!=file_w32){
			*(short *)&string2[i]=stackseg*256+255;//0x4FF
			i+=2;
		}
		obj_outrecord(0x9A,i,string2);
	}
// вывод EXTDEF
	while(externnum>maxnumextname)maxnumextname+=MAXNUMEXTNAME;
	numextname=(int *)MALLOC(maxnumextname*sizeof(int));
// output the PUBDEF records for each exteral procedures (all procedures)
	outeachPUBDEF(treestart);
	if(lenextstr!=0)obj_outrecord(0x8c,lenextstr,&string[0]);
// output the data (LEDATA) in 1K chunks as required!
	i=0;
	char *bufobj=(char *)MALLOC(512*5);
	while(i<count){
		unsigned int j;
		sizeblock=1024;
restart:
		for(j=0;j<posts;j++){
			if((postbuf+j)->type>=CALL_EXT&&(postbuf+j)->type<=FIX_CODE32&&
					(postbuf+j)->loc>=(i+startptr)&&(postbuf+j)->loc<(i+sizeblock+startptr)){
				if((postbuf+j)->loc>(i+startptr+sizeblock-(am32==FALSE?2:4))){
					sizeblock=(postbuf+j)->loc-i-startptr;//изменить размер блока
					goto restart;
				}
			}
		}
		if((i+sizeblock)>count)sizeblock=count-i;
		obj_outLEDATA(1,i+startptr,sizeblock,data+i);
		int ofsfix=0;
		for(j=0;j<posts;j++){
			if((postbuf+j)->loc>=(i+startptr)&&(postbuf+j)->loc<(i+sizeblock+startptr)){
				int hold=(postbuf+j)->loc-i-startptr;
				if((postbuf+j)->type>=CALL_32I/*POST_VAR*/&&(postbuf+j)->type<=FIX_CODE32){
					bufobj[ofsfix++]=(unsigned char)((am32==FALSE?0xC4:0xE4)|(hold/256));
					bufobj[ofsfix++]=(unsigned char)(hold%256);
					bufobj[ofsfix++]=0x14;
					bufobj[ofsfix++]=1;
					switch((postbuf+j)->type){
						case POST_VAR:
						case POST_VAR32:
							bufobj[ofsfix++]=postseg;
							break;
						case FIX_VAR:
						case FIX_VAR32:
							bufobj[ofsfix++]=(unsigned char)((comfile==file_exe&&modelmem==SMALL)?2:1);
							break;
						case FIX_CODE:
						case FIX_CODE32:
							bufobj[ofsfix++]=1;
							break;
					}
				}
				if((postbuf+j)->type>=POST_VAR32&&(postbuf+j)->type<=FIX_CODE32){

				}
				else if((postbuf+j)->type==CALL_EXT){
					int numext=findextname((postbuf+j)->num);
					if(numext!=0){
						bufobj[ofsfix++]=(unsigned char)((am32==FALSE?0x84:0xA4)|(hold/256));
						bufobj[ofsfix++]=(unsigned char)(hold%256);
						bufobj[ofsfix++]=0x56;
						bufobj[ofsfix++]=(unsigned char)numext;
					}
				}
				else if((postbuf+j)->type==EXT_VAR){
					int numext=findextname((postbuf+j)->num);
					if(numext!=0){
						bufobj[ofsfix++]=(unsigned char)((am32==FALSE?0xC4:0xE4)|(hold/256));
						bufobj[ofsfix++]=(unsigned char)(hold%256);
						bufobj[ofsfix++]=0x16;
						bufobj[ofsfix++]=1;
						bufobj[ofsfix++]=(unsigned char)numext;
					}
				}
			}
		}
		if(ofsfix!=0)obj_outrecord(0x9C+am32,ofsfix,(unsigned char *)bufobj);
		i+=sizeblock;
	}
	free(bufobj);
	if(comfile==file_exe&&modelmem==SMALL){
		i=0;
		while(i<outptrdata){
			if((i+1024)>outptrdata)obj_outLEDATA(2,i,outptrdata-i,outputdata+i);
			else obj_outLEDATA(2,i,1024,outputdata+i);
			i+=1024;
		}
	}
	if(sobj!=FALSE){
// output end of OBJ notifier
		string2[0]=0;
		i=1;
	}
	else{
		count=EntryPoint();
		*(short *)&string2[0]=0xC1;	//главный модуль имеет стартовый адрес котор нельзя менять.
		*(short *)&string2[2]=0x101;
		if(count<65536){
			*(short *)&string2[4]=(short)count;
			i=6;
		}
		else{
			*(long *)&string2[4]=(long)count;
			i=8;
		}
	}
	obj_outrecord(i==8?0x8B:0x8A,i,string2);
	free(numextname);
	runfilesize=ftell(hout);
	fclose(hout);
	hout=NULL;
	return 0;
}

void  obj_outLEDATA(unsigned int segm,unsigned int offset,unsigned int recordlength,unsigned char *data)
{
int checksum=0;
int i;
unsigned char buf[8];
	buf[3]=(unsigned char)segm;
	if(offset>(65536-1024)){
		*(short *)&buf[1]=(short)(recordlength+6);
		buf[0]=0xA1;
		*(long *)&buf[4]=(long)offset;
		i=8;
	}
	else{
		*(short *)&buf[1]=(short)(recordlength+4);
		buf[0]=0xA0;
		*(short *)&buf[4]=(short)offset;
		i=6;
	}
	fwrite(buf,i,1,hout);
	for(i--;i>=0;i--)checksum+=buf[i];
	for(i=0;(unsigned int)i<recordlength;i++)checksum+=data[i];
	fwrite(data,recordlength,1,hout);
	checksum=-checksum;
	fwrite(&checksum,1,1,hout);
}

void  obj_outrecord(int recordtype,unsigned int recordlength,unsigned char *data)
// Outputs an OBJ record.
{
int checksum;
unsigned int i;
	recordlength++;
	checksum=recordtype;
	fwrite(&recordtype,1,1,hout);
	checksum+=(recordlength/256);
	checksum+=(recordlength&255);
	fwrite(&recordlength,2,1,hout);
	recordlength--;
	for(i=0;i<recordlength;i++)checksum+=data[i];
	fwrite(data,recordlength,1,hout);
	checksum=-checksum;
	fwrite(&checksum,1,1,hout);
}

void outeachPUBDEF(struct idrec *ptr)
{
unsigned int i;
	if(ptr!=NULL){
		outeachPUBDEF(ptr->right);
		if(ptr->rectok==tk_apiproc){
			i=0;
			for(unsigned int j=0;j<posts;j++){	//поиск использования процедуры
				if((postbuf+j)->num==(unsigned long)ptr->recnumber&&((postbuf+j)->type==CALL_32I||(postbuf+j)->type==CALL_32)){
					i++;
					(postbuf+j)->type=CALL_EXT;
					externnum++;
					if(externnum>=maxnumextname){
						maxnumextname+=MAXNUMEXTNAME;
						numextname=(int *)REALLOC(numextname,maxnumextname*sizeof(int));
					}
				}
			}
			if(i)goto nameext;
			goto endp;
		}
		if(externnum!=0&&ptr->rectok==tk_undefproc&&(ptr->flag&f_extern)!=0){
nameext:
			numextname[numextern++]=ptr->recnumber;
			i=strlen(ptr->recid);
			if((lenextstr+i+2)>=STRLEN){
				obj_outrecord(0x8c,lenextstr,&string[0]);
				lenextstr=0;
			}
			string[lenextstr++]=(unsigned char)i;
			strcpy((char *)&string[lenextstr],ptr->recid);
			lenextstr+=i+1;
		}
		else{
			if((ptr->rectok==tk_proc||ptr->rectok==tk_interruptproc)&&ptr->recsegm>=NOT_DYNAMIC){
				string2[0]=(unsigned char)(am32==0?0:1);
				string2[1]=0x01;
			}
			else if((ptr->rectok>=tk_bits&&ptr->rectok<=tk_doublevar)||ptr->rectok==tk_structvar){
				if((ptr->flag&f_extern))goto nameext;
				if(am32){
					string2[0]=1;
					string2[1]=(unsigned char)(ptr->recpost==0?1:postseg);
				}
				else if(comfile==file_exe&&modelmem==SMALL){
					string2[0]=1;
					string2[1]=(unsigned char)(ptr->recpost==0?2:postseg);
				}
				else{
					string2[0]=(unsigned char)(ptr->recpost==0?0:1);
					string2[1]=(unsigned char)(ptr->recpost==0?1:postseg);
				}
			}
			else goto endp;
			for(i=0;ptr->recid[i]!=0;i++)string2[i+3]=ptr->recid[i];
			string2[2]=(unsigned char)i;
			if(ptr->recnumber<65536){
				*(short *)&string2[i+3]=(short)ptr->recnumber;
				string2[i+5]=0x00;
				obj_outrecord(0x90,i+6,&string2[0]);
			}
			else{
				*(long *)&string2[i+3]=(long)ptr->recnumber;
				string2[i+7]=0x00;
				obj_outrecord(0x91,i+8,&string2[0]);
			}
		}
endp:
		outeachPUBDEF(ptr->left);
	}
}

