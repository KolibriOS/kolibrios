#include "tok.h"
#include "le.h"

#define _OUTLE_

unsigned short swapshort(unsigned int var)
{
unsigned short h,l;
	l=var>>8;
	h=var<<8;
	return l|h;
}

int MakeBin32()
{
	hout=CreateOutPut(outext,"wb");
	if(fwrite(output,outptr,1,hout)!=1){
		ErrWrite();
		fclose(hout);
		hout=NULL;
		return(-1);
	}
	return 0;
}

unsigned int EntryParamStr()
{
ITOK btok;
int bb=tk_id;
unsigned char *buf=(unsigned char *)"__bufcomstr";
	btok.number=0;
	searchtree(&btok,&bb,buf);
	if(bb==tk_id)btok.number=0;
	else{
		btok.number+=outptrdata;
		if((outptrdata%2)==1)btok.number++;	/* alignment of entire post data block manditory */
	}
	return btok.number;
}

int MakeMEOS()
{
MEOSheader hdr;
	hout=CreateOutPut(outext,"wb");
	strcpy((char *)hdr.sign,"MENUET01");
	hdr.vers=1;
	hdr.start=EntryPoint();
	hdr.size=outptr;
	hdr.alloc_mem=Align(outptr+postsize+stacksize,16);
	hdr.esp=hdr.alloc_mem-4;
	hdr.I_Param=EntryParamStr();
	hdr.I_Icon=0;
	memcpy(output+startptr,&hdr,sizeof(MEOSheader));
	if(fwrite(output,outptr,1,hout)!=1){
		ErrWrite();
		fclose(hout);
		hout=NULL;
		return(-1);
	}
	return 0;
}

int MakeLE()
{
LE_Header hdr;
Object_Table ot[2];
Page_Map_Table *pmt;
unsigned int i;
int headerofs;
int headsize=sizeof(LE_Header);
unsigned long *fpto;
int sizeb;
	CreatStub(stubfile);
	memset(&hdr,0,sizeof(LE_Header));
#ifdef _WC_
	hdr.Signature='EL';
#else
	hdr.Signature='LE';
#endif
	hdr.CPU_Type=i80386;
	hdr.Target_OS=1;
	hdr.Type_Flags=0x200;
	hdr.Number_Of_Memory_Pages=outptr/0x1000+1;
	hdr.Initial_CS=1;
	hdr.Initial_EI=EntryPoint();
	hdr.Initial_SS=2;
	hdr.Initial_ESP=stacksize;
	hdr.Memory_Page_Size=0x1000;
	hdr.Bytes_On_Last_Page=outptr%0x1000;
	hdr.Object_Table_Offset=sizeof(LE_Header);
	hdr.Object_Table_Entries=2;
	headerofs=ftell(hout);
	if(fwrite(&hdr,sizeof(LE_Header),1,hout)!=1){
errwrite:
		ErrWrite();
		fclose(hout);
		hout=NULL;
		return(-1);
	}

	ot[0].Virtual_Segment_Size=outptr+postsize;//hdr.Initial_ESP;
	ot[0].Relocation_Base_Address=0x10000;
	ot[0].ObjTableFlags=0x2045;
	ot[0].Page_MAP_Index=1;
	ot[0].Page_MAP_Entries=hdr.Number_Of_Memory_Pages;
	ot[1].Virtual_Segment_Size=hdr.Initial_ESP;
	ot[1].Relocation_Base_Address=Align(outptr+postsize,0x10000)+0x10000;
	ot[1].ObjTableFlags=0x2043;
	ot[1].Page_MAP_Index=2;
	ot[0].Reserved=ot[1].Reserved=ot[1].Page_MAP_Entries=0;
	if(fwrite(&ot,sizeof(Object_Table)*2,1,hout)!=1)goto errwrite;
	hdr.Object_Page_Map_Table_Offset=headsize+=sizeof(Object_Table)*2;

	sizeb=sizeof(Page_Map_Table)*hdr.Number_Of_Memory_Pages;
	pmt=(Page_Map_Table *)MALLOC(sizeb);
	for(i=0;i<hdr.Number_Of_Memory_Pages;i++){
		(pmt+i)->High_Page_Number=swapshort(((i+1)/256));
		(pmt+i)->Low_Page_Number=(unsigned char)((i+1)%256);
		(pmt+i)->FLAGS=0;
	}
	if(fwrite(pmt,sizeb,1,hout)!=1)goto errwrite;
	free(pmt);
	hdr.Resource_Table_Offset=hdr.Resident_Names_Table_Offset=headsize+=sizeb;

	i=strlen(rawfilename);
	string2[0]=(unsigned char)i;
	strcpy((char *)&string2[1],rawfilename);
	*(long *)&string2[i+2]=0;
	if(fwrite(string2,i+5,1,hout)!=1)goto errwrite;

	hdr.Entry_Table_Offset=headsize+=i+4;
	headsize++;
	sizeb=sizeof(unsigned long)*(hdr.Number_Of_Memory_Pages+1);
	fpto=(unsigned long *)MALLOC(sizeb);
	hdr.Fixup_Page_Table_Offset=headsize;
	memset(fpto,0,sizeb);
//	for(i=0;i<(hdr.Number_Of_Memory_Pages+1);i++){
//		fpto[i]=0;
//	}
	if(fwrite(fpto,sizeb,1,hout)!=1)goto errwrite;
	headsize+=sizeb;

//Fixup_Record_Table
int sizefixpage=0;
	hdr.Fixup_Record_Table_Offset=headsize;
	for(i=0;i<hdr.Number_Of_Memory_Pages;i++){
		unsigned int startblc=i*4096;	//адрес первого блока
		for(unsigned int j=0;j<posts;j++){	//обходим всю таблицу post
			if(((postbuf+j)->type==CALL_32I||
	    ((postbuf+j)->type>=POST_VAR32&&(postbuf+j)->type<=FIX_CODE32))&&
			(postbuf+j)->loc>=startblc&&(postbuf+j)->loc<(startblc+4096)){
				int sizerec;
				string2[0]=7;
				*(unsigned short *)&string2[2]=(unsigned short)((postbuf+j)->loc-startblc);
				string2[4]=1;
				unsigned int val=*(unsigned long *)&output[(postbuf+j)->loc];
				if(val<65536){
					string2[1]=0;
					*(unsigned short *)&string2[5]=(unsigned short)val;
					sizerec=7;
				}
				else{
					string2[1]=0x10;
					*(unsigned long *)&string2[5]=val;
					sizerec=9;
				}
				sizefixpage+=sizerec;
				if(fwrite(string2,sizerec,1,hout)!=1)goto errwrite;
			}
			fpto[i+1]=sizefixpage;
		}
	}
	headsize+=sizefixpage;

//Imported_Procedure_Name_Table
	hdr.Imported_Module_Names_Table_Offset=
			hdr.Imported_Procedure_Name_Table_Offset=headsize;
	string2[0]=0;
	if(fwrite(string2,1,1,hout)!=1)goto errwrite;
	headsize++;


	hdr.Fixup_Section_Size=headsize-hdr.Fixup_Page_Table_Offset;//9+sizefixpage;
	hdr.Loader_Section_Size=headsize-sizeof(LE_Header);
	hdr.Automatic_Data_Object=2;
	hdr.Data_Pages_Offset=Align(headsize+headerofs,16/*1024*/);
	ChSize(hdr.Data_Pages_Offset);

	fseek(hout,headerofs,SEEK_SET);
	if(fwrite(&hdr,sizeof(LE_Header),1,hout)!=1)goto errwrite;
	fseek(hout,headerofs+hdr.Fixup_Page_Table_Offset,SEEK_SET);
	if(fwrite(fpto,sizeof(unsigned long)*(hdr.Number_Of_Memory_Pages+1),1,hout)!=1)goto errwrite;
	free(fpto);

	fseek(hout,0,SEEK_END);
	return 0;
}
