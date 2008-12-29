
#pragma once

#include "kosSyst.h"
#include "kosFile.h"
#include "MCSMEMM.H"

#include <stdarg.h>


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


#define ERROR -1
#define ERROR_END -2

extern int convert_error;

#define PREC 2

typedef int HDC;
typedef int DWORD;

extern int SysColor;
extern char debuf[50];

typedef double (*function_t)(double);

typedef struct
{
  double x, y;
} TCoord;

struct kosBDVK 
{
	Dword attrib;
	Dword name_type;
	Dword create_time;
	Dword create_date;
	Dword access_time;
	Dword access_date;
	Dword modify_time;
	Dword modify_date;
	Dword size_low;
	Dword size_high;	
};

Dword kos_GetSlotByPID(Dword PID);
Dword kos_GetActiveSlot();
Dword kos_GetSkinHeight();
Dword kos_GetSpecialKeyState();
void kos_GetMouseStateWnd( Dword & buttons, int & cursorX, int & cursorY );
void kos_DrawLine( Word x1, Word y1, Word x2, Word y2, Dword colour, Dword invert);
void DrawRegion(Dword x,Dword y,Dword width,Dword height,Dword color1);
int atoi(const char* string);
void kos_GetScrollInfo(int &vert, int &hor);


Dword kos_GetSlotByPID(Dword PID);
Dword kos_GetActiveSlot();
Dword kos_GetSkinHeight();
Dword kos_GetSpecialKeyState();


double fabs(double x);
double cos(double x);
double sin(double x);
bool isalpha(char c);
double convert(char *s, int *len=NULL);
void format( char *Str, int len, char* Format, ... );

void line( int x1, int y1, int x2, int y2);

void outtextxy( int x, int y, char *s, int len);
void settextstyle( int a1, int a2, int a3);


double textwidth( char *s, int len);
double textheight( char *s, int len);
void setcolor( DWORD color);
void unsetcolor(HDC hdc);
void rectangle( int x1, int y1, int x2, int y2);

typedef struct 
{
unsigned	p00 ;
unsigned	p04 ;
unsigned	p08 ;
unsigned	p12 ;
unsigned	p16 ;
char		p20 ;
char		*p21 ;
} kol_struct70 ;


typedef struct
{
unsigned	p00 ;
char		p04 ;
char		p05[3] ;
unsigned	p08 ;
unsigned	p12 ;
unsigned	p16 ;
unsigned	p20 ;
unsigned	p24 ;
unsigned	p28 ;
unsigned	p32[2] ;
unsigned	p40 ;
} kol_struct_BDVK ;

typedef struct
{
char	*name ;
void	*data ;
} kol_struct_import ;



kol_struct_import* kol_cofflib_load(char *name);
void* kol_cofflib_procload (kol_struct_import *imp, char *name);
unsigned kol_cofflib_procnum (kol_struct_import *imp);
void kol_cofflib_procname (kol_struct_import *imp, char *name, unsigned n);
int strcmp(const char* string1, const char* string2);

char *ftoa(double d);
double atof(char *s);


int di(double x); 
double id(int x);
