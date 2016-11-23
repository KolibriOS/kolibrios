//visual text comparer 
//by den po - jdp@bk.ru

#define MEMSIZE 4096 * 60
#include "../lib/io.h"
#include "../lib/strings.h"
#include "../lib/obj/console.h"
IO io1, io2;

#define MAX_PATH 260

#include "32user32.h"
#include "objects.h"
#include "diff_gui.h"

#define mincmpstrings 2
#define maxcmpstrings 10
#define maxcmpoffset  100

char window_title[] = "Kolibri Diff tool (Visual Text Comparer)";

char* srcfilename;
char* dstfilename;
dword srcfile;
dword dstfile;
dword srcfilesize;
dword dstfilesize;
TCollection srcfilelines; //file lines
TCollection dstfilelines;
TCollection srcfilenums; //lines numbers
TCollection dstfilenums;

struct TSimpleCollection:TSortedCollection
{
  TSimpleCollection();
  int  Compare( int Key1, Key2 );
};
int TSimpleCollection::Compare( int Key1, Key2 ){
  return Key1-Key2;
}
TSimpleCollection::TSimpleCollection():TSortedCollection(1000,1000);
{
  comparemethod=#Compare;
}

TSimpleCollection diffs;   //list of differing strings
dword srcfilelinks;     //pointer to the previos line with the same first symbol
dword dstfilelinks;     //

getstrings(dword srcfile,srcfilesize,srcfilelines){
  ECX=srcfilesize;
  ESI=srcfile;
  WHILE(ECX){
	$push ECX,ESI
	EAX=srcfilelines; 
	EAX.TCollection.Insert(ESI);
	$pop ESI,ECX
	WHILE(ECX){
	  $cld $lodsb ECX--;
	  if(AL==0x0D)||(AL==0x0A)DSBYTE[ESI-1]=0;
	  if(AL==0x0D)&&(DSBYTE[ESI]==0x0A){$lodsb;ECX--;}
	  if(AL==0x0D)||(AL==0x0A)BREAK;
	}
  }
}

#include "if.h"

bool getparam() 
{
	int i, param_len = strlen(#param);
	if (param[0]=='"') {
		for (i=1; i<param_len; i++) if (param[i]=='"') param[i]=NULL;
		srcfilename = #param + 1;
		dstfilename = #param + strlen(#param) + 3;
		return true;
	}
	notify("'Wrong params specified. Use next format:\nAPPPATH \"PARAM1\" \"PARAM2\"' -E");
	return false;
}

main(){
	if (param[0]) getparam();
	if (!srcfilename) || (!dstfilename) gui(); else console();
}

console() {
	int i;
	int p;
	int bs,bd,bsc,bdc,bsp,bdp;
	int cache[256];
	char s1;
	int s2;

	srcfile = io1.read(srcfilename);
	dstfile = io2.read(dstfilename);

	srcfilesize = io1.FILES_SIZE;
	dstfilesize = io2.FILES_SIZE;

	if (!srcfile) die("'First file not found' -E"); 
	if (!dstfile) die("'Second file not found' -E");
	
	srcfilelines.TCollection(srcfilesize/40,8192);
	dstfilelines.TCollection(dstfilesize/40,8192);
	//fill line pointers
	getstrings(srcfile,srcfilesize,#srcfilelines);
	getstrings(dstfile,dstfilesize,#dstfilelines);
	srcfilenums.TCollection(srcfilelines.Count,1000);
	dstfilenums.TCollection(dstfilelines.Count,1000);

	srcfilelinks=malloc(srcfilelines.Count*4);
	dstfilelinks=malloc(dstfilelines.Count*4);

	//fill links on the next strings with the same first symbols
	FillMemory(#cache,sizeof(cache),-1); i=srcfilelines.Count;
	WHILE(i){
	  i--;
	  EBX=srcfilelines.At(i); EBX=DSBYTE[EBX];
	  DSDWORD[i<<2+srcfilelinks]=cache[EBX*4];
	  cache[EBX*4]=i;
	}
	FillMemory(#cache,sizeof(cache),-1); i=dstfilelines.Count;
	WHILE(i){
	  i--;
	  EBX=dstfilelines.At(i); EBX=DSBYTE[EBX];
	  DSDWORD[i<<2+dstfilelinks]=cache[EBX*4];
	  cache[EBX*4]=i;
	}

	diffs.TSimpleCollection();

	while( bsp < srcfilelines.Count ) || ( bdp < dstfilelines.Count )
	{
	  ////////////////////////////////////////////////////////
	  bsc=0;
	  p=dstfilelines.At(bdp);//current dst position
	  s1=DSBYTE[p];
	  bs=bsp+1;//found src line, bsc - number of matched
	  while( bs != -1 )//no next line starting with the same symbols
		 &&(bs-bsp<=maxcmpoffset)//check for 100 lines depth
		 &&(bs<srcfilelines.Count)
	  {
		s2=srcfilelines.At(bs);
		if(!strcmp(p,s2))
		{//line found
		  bsc=1;
		  WHILE(bsc<maxcmpstrings)//counting number of matching lines
			 &&(bdp+bsc<dstfilelines.Count)
			 &&(bs+bsc<srcfilelines.Count)
			 &&(!strcmp(dstfilelines.At(bdp+bsc),srcfilelines.At(bs+bsc)))bsc++;
		  BREAK;
		}
		if(DSBYTE[s2]==s1)bs=DSDWORD[bs<<2+srcfilelinks];else bs++;
	  }
	  bdc=0;
	  p=srcfilelines.At(bsp);//current src position
	  s1=DSBYTE[p];
	  bd=bdp+1;//found dst line, bsc - number of matched
	  while( bd != -1 )//no next line starting with the same symbols
		 &&(bd-bdp<=maxcmpoffset)//check for 100 lines depth
		 &&(bd<dstfilelines.Count)
	  {
		s2=dstfilelines.At(bd);
		if(!strcmp(p,s2))
		{
		  bdc=1;
		  WHILE(bdc<maxcmpstrings)//counting number of matching lines
			 &&(bsp+bdc<srcfilelines.Count)
			 &&(bd+bdc<dstfilelines.Count)
			 &&(!strcmp(srcfilelines.At(bsp+bdc),dstfilelines.At(bd+bdc)))bdc++;
		  BREAK;
		}
		if(DSBYTE[s2]==s1)bd=DSDWORD[bd<<2+dstfilelinks];else bd++;
	  }

	  if(bsc<bdc)
	   if(bd-bdp<bdc)
	   ||(bdc>=mincmpstrings)
	  {
		WHILE(bdp<bd){
		  diffs.Insert(srcfilenums.Count);
		  srcfilenums.Insert(-1); dstfilenums.Insert(bdp); bdp++;
		}
		continue;
	  }

	  if(strcmp(srcfilelines.At(bsp),dstfilelines.At(bdp)))diffs.Insert(srcfilenums.Count);
	  //lines are equal
	  srcfilenums.Insert(bsp); bsp++;
	  dstfilenums.Insert(bdp); bdp++;
	  CONTINUE;
	}

	free(srcfilelinks);
	free(dstfilelinks);

	if(!diffs.Count)
		notify("'Nothing to compare' -E"); 
	else ifinit();

	diffs.DeleteAll();        delete diffs;
	dstfilenums.DeleteAll();  delete dstfilenums;
	srcfilenums.DeleteAll();  delete srcfilenums;
	dstfilelines.DeleteAll(); delete dstfilelines;
	srcfilelines.DeleteAll(); delete srcfilelines;
	free(dstfile);
	free(srcfile);
}
