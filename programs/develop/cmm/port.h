#ifndef ___PORT_H___
#define ___PORT_H___

#ifdef __cplusplus
extern "C" {
#endif

#define _BORLAND_DOS_REGS

struct DWORDREGS {
  unsigned long edi;
  unsigned long esi;
  unsigned long ebp;
  unsigned long cflag;
  unsigned long ebx;
  unsigned long edx;
  unsigned long ecx;
  unsigned long eax;
  unsigned short eflags;
};
struct DWORDREGS_W {
  unsigned long di;
  unsigned long si;
  unsigned long bp;
  unsigned long cflag;
  unsigned long bx;
  unsigned long dx;  unsigned long cx;
  unsigned long ax;
  unsigned short flags;
};

struct WORDREGS {
  unsigned short di, _upper_di;
  unsigned short si, _upper_si;
  unsigned short bp, _upper_bp;
  unsigned short cflag, _upper_cflag;
  unsigned short bx, _upper_bx;
  unsigned short dx, _upper_dx;
  unsigned short cx, _upper_cx;
  unsigned short ax, _upper_ax;
  unsigned short flags;
};

struct BYTEREGS {
  unsigned short di, _upper_di;
  unsigned short si, _upper_si;
  unsigned short bp, _upper_bp;
  unsigned long cflag;
  unsigned char bl;
  unsigned char bh;
  unsigned short _upper_bx;
  unsigned char dl;
  unsigned char dh;
  unsigned short _upper_dx;
  unsigned char cl;
  unsigned char ch;
  unsigned short _upper_cx;
  unsigned char al;
  unsigned char ah;
  unsigned short _upper_ax;
  unsigned short flags;
};

union REGS {
  struct DWORDREGS d;
#ifdef _NAIVE_DOS_REGS
  struct WORDREGS x;
#else
#ifdef _BORLAND_DOS_REGS
  struct DWORDREGS x;
#else 
  struct DWORDREGS_W x;
#endif
#endif
  struct WORDREGS w;
  struct BYTEREGS h;
};


long getfilelen(int);

#ifndef _WIN32_
#define CP_ACP 0
#define MB_PRECOMPOSED 1
#define OemToChar OemToCharA
#define CharToOem CharToOemA


bool OemToCharA(char*, char*);
bool CharToOemA(char*, char*);
int MultiByteToWideChar(unsigned int,unsigned int,char*,int,wchar_t *,int);

#else

//int stricmp(const char*, const char*); //Leency
//int strnicmp(const char*, const char*, int); //Leency

#endif

char* strupr(char* s);
char* strlwr(char* s);

#ifdef _PORT_CPP_

long lseek (int, long, int);
char tolower(char c);
char toupper(char c);

char * getcwd (char *buffer, int size);
int 	stat (const char*, struct _stat*);

void exit(int);

void* malloc(int); 
#endif //_PORT_CPP_

#ifdef __cplusplus
}
#endif


#endif  // ___PORT_H___
