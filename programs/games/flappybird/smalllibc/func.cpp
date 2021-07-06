

#include "func.h"

int convert_error = 0;
int SysColor = 0;
char debuf[50] = "";


// почему-то не было в стандартной библиотеке
void kos_DrawLine( Word x1, Word y1, Word x2, Word y2, Dword colour, Dword invert )
{
	Dword arg1, arg2, arg3;

	//
	arg1 = ( x1 << 16 ) | x2;
	arg2 = ( y1 << 16 ) | y2;
	arg3 = (invert)?0x01000000:colour;
	//
	__asm{
		mov eax, 38
		mov ebx, arg1
		mov ecx, arg2
		mov edx, arg3
		int 0x40
	}
}

// похищено из библиотеки к C--
void DrawRegion(Dword x,Dword y,Dword width,Dword height,Dword color1)
{
	kos_DrawBar(x,y,width,1,color1); //полоса гор сверху
	kos_DrawBar(x,y+height,width,1,color1); //полоса гор снизу
	kos_DrawBar(x,y,1,height,color1); //полоса верт слева
	kos_DrawBar(x+width,y,1,height+1,color1); //полоса верт справа
}


// да, это баян
int atoi(const char* string)
{
	int res=0;
	int sign=0;
	const char* ptr;
	for (ptr=string; *ptr && *ptr<=' ';ptr++);
	if (*ptr=='-') {sign=1;++ptr;}
	while (*ptr >= '0' && *ptr <= '9')
	{
		res = res*10 + *ptr++ - '0';
	}
	if (sign) res = -res;
	return res;
}

/*int abs(int n)
{
	return (n<0)?-n:n;
}*/





double fabs(double x)
{
	__asm	fld	x
	__asm	fabs
}
#define M_PI       3.14159265358979323846
double cos(double x)
{
	__asm	fld	x
	__asm	fcos
}
double sin(double x)
{
	__asm	fld	x
	__asm	fsin
}

bool isalpha(char c)
{
	return (c==' ' || c=='\n' || c=='\t' || c=='\r');
}

// эта функция - велосипед. но проще было написать чем найти.
double convert(char *s, int *len)
{

	int i;


	double sign,res, tail, div;

	convert_error = 0;

	res = 0.0;

	i=0;
	while (s[i] && isalpha(s[i])) i++;
	if (len) *len=i;
	if (s[i] == '\0')
	{
		convert_error = ERROR_END;
		return 0.0;
	}

	sign=1.0;
	if (s[i] == '-')
	{
		sign=-1.0;
		i++;
	}
	while (s[i] && s[i] >= '0' && s[i] <= '9')
	{
		res *= 10.0;
		res += id(s[i] - '0');
		i++;
	}
	if (len) *len=i;
	if (!s[i] || isalpha(s[i]))
		return sign*res;
	if (s[i] != '.' && s[i] != ',')
	{
		convert_error = ERROR;
		return 0;
	}
	i++;
	if (len) *len=i;
	if (!s[i])
		return sign*res;
	
	div = 1.0;
	tail = 0.0;
	while (s[i] && s[i] >= '0' && s[i] <= '9')
	{
		tail *= 10.0;
		tail += id(s[i] - '0');
		div *= 10.0;
		i++;		
	}
	res += tail/div;
	if (len) *len=i;
	return sign*res;
}

/*
#define PREC 2

double double_tab[]={1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15};

// это sprintf, умеющий форматировать _только_ вещественные числа (double) %f
void format( char *Str, int len, char* Format, ... )
{
	int i, fmtlinesize, j, k, flag;
	char c;
	va_list arglist;
	//
	va_start(arglist, Format);

	//
	fmtlinesize = strlen( Format );
	//
	if( fmtlinesize == 0 ) return;

	for (i = 0; i < len; i++)
		Str[i] = 0;
  
	//
	for( i = 0, j = 0; i < fmtlinesize; i++ )
	{
		//
		c = Format[i];
		//
		if( c != '%' )
		{
			Str[j++] = c;
			continue;
		}
		//
		i++;
		//
		if( i >= fmtlinesize ) break;

		//
		flag = 0;
		//
		c = Format[i];
		//
		switch( c )
		{
		//
		case '%':
			Str[j++] = c;
			break;
		// auaia aauanoaaiiiai ?enea
		case 'f':
			// ii?aaaeeou ?enei oeo? ai oi?ee
			double val, w;
			int p;
			val = va_arg(arglist, double);
			if (val < 0.0)
			{
				Str[j++] = '-';
				val = -val;
			}
			for (k = 0; k < 15; k++)
				if (val < double_tab[k])
					break;

			if (val < 1.0)
			{
				Str[j++] = '0';
			}
			
			for (p = 1; p < k + 1; p++)
			{
				Str[j++] = '0' + di(val / double_tab[k - p] - 0.499) % 10;
			}
			Str[j++] = '.';
			w = 0.1;
			for (p = 0; p < 2; p++)
			{
				val-=floor(val);
				Str[j++] = '0' + di(val / w - 0.499) % 10;
				w /= 10.0;
			}

		//
		default:
			break;
		}
	}
	//
	Str[j] = 0;
}

void *memcpy(void *dst, const void *src, unsigned size)
{
	while (size--)
		*((char*)dst+size) = *((char*)src+size);
	return dst;
}
*/
int strcmp(const char *s1, const char *s2)
{
	int i;

	if (s1 == NULL)
		if (s2 == NULL)
			return 0;
		else
			return 1;
	else
		if (s2 == NULL)
			return 1;

	for (i = 0;;i++)
	{
		if (s1[i] == '\0')
			if (s2[i] == '\0')
				return 0;
			else
				return 1;
		else
			if (s2[i] == '\0')
				return 1;
			else
			{
				if (s1[i] != s2[i])
					return 1;
			}
	}
	return 0;
}

kol_struct_import* kol_cofflib_load(char *name)
{
//asm ("int $0x40"::"a"(68), "b"(19), "c"(name));
	__asm
	{
		mov eax, 68
		mov ebx, 19
		mov ecx, name
		int 0x40
	}
}


void* kol_cofflib_procload (kol_struct_import *imp, char *name)
{
	
int i;
for (i=0;;i++)
	if ( NULL == ((imp+i) -> name))
		break;
	else
		if ( 0 == strcmp(name, (imp+i)->name) )
			return (imp+i)->data;
return NULL;

}


unsigned kol_cofflib_procnum (kol_struct_import *imp)
{
	
unsigned i, n;

for (i=n=0;;i++)
	if ( NULL == ((imp+i) -> name))
		break;
	else
		n++;

return n;

}


void kol_cofflib_procname (kol_struct_import *imp, char *name, unsigned n)
{
	
unsigned i;
*name = 0;

for (i=0;;i++)
	if ( NULL == ((imp+i) -> name))
		break;
	else
		if ( i == n )
			{
			strcpy(name, ((imp+i)->name));
			break;
			}

}



/*
end of system part
*/


// поскольку я портировал с древнего доса...
void line( int x1, int y1, int x2, int y2)
{
	kos_DrawLine(x1,y1,x2,y2,SysColor,0);
}

void outtextxy( int x, int y, char *s, int len)
{
	kos_WriteTextToWindow(x,y,0,SysColor,s,len);
}

double textwidth( char *s, int len)
{
	int i;
	for (i = 0; i < len; i++)
		if (s[i] == 0)
			break;
	return id(i * 6);
}

double textheight( char *s, int len)
{
	return 8.0;
}

void setcolor( DWORD color)
{
	SysColor = color;
}

void rectangle( int x1, int y1, int x2, int y2)
{
	kos_DrawBar(x1,y1,x2-x1,y2-y1,SysColor);
}



Dword kos_GetSkinHeight()
{
	__asm{
		mov		eax, 48
		mov		ebx, 4
		int		0x40
	}
}

Dword kos_GetSpecialKeyState()
{
	__asm{
		mov		eax, 66
		mov		ebx, 3
		int		0x40
	}
}



Dword kos_GetSlotByPID(Dword PID)
{
	__asm
	{
		push ebx
		push ecx
		mov eax, 18
		mov ebx, 21
		mov ecx, PID
		int	0x40
		pop ecx
		pop ebx
	}
}


Dword kos_GetActiveSlot()
{
	__asm
	{
		push ebx
		mov eax, 18
		mov ebx, 7
		int	0x40
		pop ebx
	}
}



void kos_GetScrollInfo(int &vert, int &hor)
{
	short v, h;
	__asm
	{
		mov eax, 37
		mov ebx, 7
		int	0x40
		mov ebx, eax
		and eax, 0xffff
		mov v, ax
		shr ebx, 16
		mov h, bx
	}
	vert = v;
	hor = h;
}


// получение информации о состоянии "мыши" функция 37/1
void kos_GetMouseStateWnd( Dword & buttons, int & cursorX, int & cursorY )
{
	Dword mB;
	Word curX;
	Word curY;
	sProcessInfo sPI;

	//
	__asm{
		mov		eax, 37
		mov		ebx, 1
		int		0x40
		mov		curY, ax
		shr		eax, 16
		mov		curX, ax
		mov		eax, 37
		mov		ebx, 2
		int		0x40
		mov		mB, eax
	}
	//
	kos_ProcessInfo( &sPI );
	//
	buttons = mB;
	cursorX = curX - sPI.processInfo.x_start;
	cursorY = curY - sPI.processInfo.y_start;
}

double atof(char *s)
{
	return convert(s, NULL);
}


int di(double x)
{
	int a;
	__asm fld x
	__asm fistp a
	return a;
}

double id(int x)
{
	double a;
	__asm fild x
	__asm fstp a
	return a;
}
