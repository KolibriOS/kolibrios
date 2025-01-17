#include "kosSyst.h"
#include "func.h"
#include <stdarg.h>
#include "sprintf.h"

//////////////////////////////////////////////////////////////////////
//
// вывод строки на печать. barsuk добавил %f 



//
void sprintf( char *Str, char* Format, ... )
{
	int i, fmtlinesize, j, k, flag;
	Dword head, tail;
	char c;
	va_list arglist;
	//
	va_start(arglist, Format);

	//
	fmtlinesize = strlen( Format );
	//
	if( fmtlinesize == 0 ) return;
  
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
		// вывод строки
		case 'S':
			Byte* str;
			str = va_arg(arglist, Byte*);
			for( k = 0; ( c = str[k] ) != 0; k++ )
			{
				Str[j++] = c;
			}
			break;
		// вывод байта
		case 'B':
			k = va_arg(arglist, int) & 0xFF;
			Str[j++] = num2hex( ( k >> 4 ) & 0xF );
			Str[j++] = num2hex( k & 0xF );
			break;
		// вывод символа
		case 'C':
			Str[j++] = va_arg(arglist, int) & 0xFF;
			break;
		// вывод двойного слова в шестнадцатиричном виде
		case 'X':
			Dword val;
			val = va_arg(arglist, Dword);
			for( k = 7; k >= 0; k-- )
			{
				//
				c = num2hex ( ( val >> (k * 4) ) & 0xF );
				//
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
			}
			//
			if( flag == 0 ) Str[j++] = '0';
			break;
		// вывод двойного слова в десятичном виде
		case 'U':
			head = va_arg(arglist, Dword);
			tail = 0;
			for( k = 0; dectab[k] != 0; k++ )
			{
				tail = head % dectab[k];
				head /= dectab[k];
				c = head + '0';
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
				//
				head = tail;
			}
			//
			c = head + '0';
			Str[j++] = c;
			break;
		// вещественное число в формате 7.2
		case 'f':
		case 'F':
		case 'g':
		case 'G':
			{
			double val, w;
			int p;
			val = va_arg(arglist, double);
			if (val < 0.0)
			{
				Str[j++] = '-';
				val = -val;
			}
			for (k = 0; k < 30; k++)
				if (val < double_tab[k])
					break;

			if (val < 1.0)
			{
				Str[j++] = '0';
			}
			
			for (p = 1; p < k + 1; p++)
			{
				int d = (int)di(val / double_tab[k - p] - HALF) % 10;
				Str[j++] = '0' + d;
				val -= d * double_tab[k - p];
			}
			Str[j++] = '.';
			w = 0.1;
			for (p = 0; p < PREC - 1; p++)
			{
				val-=floor(val);
				Str[j++] = '0' + di(val / w - HALF) % 10;
				w /= 10.0;
			}
			}
			break;

		// вывод 64-битного слова в шестнадцатиричном виде
		case 'Q':
			unsigned int low_dword, high_dword;
			low_dword = va_arg(arglist, unsigned int);
			high_dword = va_arg(arglist, unsigned int);
			for( k = 7; k >= 0; k-- )
			{
				//
				c = num2hex ( ( ( high_dword + 1) >> (k * 4) ) & 0xF );
				//
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
			}
			//
			for( k=7; k >= 0; k-- )
			{
				//
				c = num2hex ( ( low_dword >> (k * 4) ) & 0xF );
				//
				if( c == '0' )
				{
					if( flag ) Str[j++] = c;
				}
				else
				{
					flag++;
					Str[j++] = c;
				}
			}
			//
			if( flag == 0 ) Str[j++] = '0';
			//
			break;
		//
		default:
			break;
		}
	}
	//
	Str[j] = 0;
}

char *ftoa(double d)
{
	char buffer[256], *p;
	sprintf(buffer, "%f", d);
	p = (char*)allocmem(strlen(buffer)+1);
	strcpy(p, buffer);
	return p;
}

