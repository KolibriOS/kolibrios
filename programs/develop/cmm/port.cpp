#define SEEK_SET	(0)
#define	SEEK_CUR	(1)
#define SEEK_END	(2)


#define _PORT_CPP_
#include "port.h"


long getfilelen(int fd)
{ 
	long reslt,curr;
	curr = lseek(fd,0,SEEK_CUR);
	reslt = lseek(fd,0,SEEK_END);
	lseek(fd,curr,SEEK_SET);
	return reslt;
}



#ifndef _WIN32_ 

char* strupr(char* s)
{
  char* p = s; 
  while (*p = toupper(*p)) p++;
  return s;
}

char* strlwr(char* s)
{
  char* p = s;
  while (*p = tolower(*p)) p++;
  return s;
}


int strnicmp(const char* s1, const char* s2, int len)
{
  unsigned char c1,c2;
  
  if(!len)
	return 0;
  
  do{
    c1 = *s1++; 
    c2 = *s2++;
    if (!c1||!c2)
      break;
    if (c1 == c2)
      continue;
    c1 = tolower(c1);
    c2 = tolower(c2);
    if (c1!=c2)
      break;
  } while (--len);
  return (int)c1 - (int)c2;
}

int stricmp(const char* s1, const char* s2)
{
  unsigned char c1,c2; 
  
  do{
    c1 = *s1++;
    c2 = *s2++;
    if (!c1||!c2)
      break;
    if (c1 == c2)
      continue;
    c1 = tolower(c1);
    c2 = tolower(c2);
    if (c1!=c2)
      break;
  } while (c1);
  
  return (int)c1 - (int)c2;
}

bool OemToCharA(char* a, char* b)
{ 
  *b = *a;
  return true;
}

bool CharToOemA(char* a, char* b)
{ 
  *b = *a;
  return true;
}

//строка-результат не используется
int MultiByteToWideChar( 
    unsigned int CodePage,	// code page 
    unsigned int dwFlags,	// character-type options 
    char* lpMultiByteStr,	// address of string to map 
    int cchMultiByte,	// number of characters in string 
    wchar_t* lpWideCharStr,	// address of wide-character buffer 
    int cchWideChar 	// size of buffer 
   )
{
	int i;
	while ((lpMultiByteStr[i*2]!=0) && (lpMultiByteStr[i*2+1]!=0)) i++;
	return i/2;
}


#ifdef _KOS_

typedef struct _fs
{
	int cmd;
	int numL;
	int numH;
	int size;
	void* pointer;
	unsigned char nul;
	char* path;
} fs;

int stat (const char* path, struct _stat* buf)
{
	//fs fstruc;
	char buff[40];
	int __ret;
	char pathin[256];
	int i;
	char fstruc[25];
	
	for(i=0; i<256; i++){
		pathin[i]=path[i];
		if (pathin[i]==0) break; 
	}
	pathin[255]=0;
	
	*((int*)(fstruc+0)) = 5;
	*((int*)(fstruc+4)) = 0;
	*((int*)(fstruc+8)) = 0;
	*((int*)(fstruc+12)) = 0;
	*((char**)(fstruc+16)) = buff;
	*((char*)(fstruc+20)) = 0;
	*((char**)(fstruc+21)) = pathin;
	__asm__ __volatile__("int $0x40":"=a"(__ret):"a"(70),"b"(&fstruc));
	
	return __ret;
}
 
char * getcwd (char *buffer, int size)
{
	int len=0;
	if (size==0){
		if (buffer!=0)
			return 0;
		size = 256;
	}
		
	if (buffer==0)
		buffer = (char*)malloc(size);
	
	__asm__ __volatile__("int $0x40":"=a"(len):"a"(30),"b"(2),"c"(buffer),"d"(size));
	if (len!=0)
		return buffer;
	else
		return 0;
}

void exit(int a){
	__asm__ __volatile__("int $0x40"::"a"(-1));
}

#endif //_KOS_

#endif //not _WIN32_

