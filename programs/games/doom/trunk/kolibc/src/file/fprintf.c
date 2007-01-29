#include "kolibc.h"
const char xdigs_lower[16]="0123456789abcdef";
const char xdigs_upper[16]="0123456789ABCDEF";
int fprintf(FILE* file, const char* format, ...)
{
	char* arg;
	int ispoint;
	int beforepoint;
	int afterpoint;
	int longflag;
	int contflag;
	int i;
	long long number;
	char buffer[512];
	char* str;
	arg= (void*)&format;
	arg+=sizeof(const char*);
	while (*format!='\0')
	{
		if (*format!='%')
		{
			fputc(*format,file);
			format++;
			continue;
		}
		ispoint=0;
		beforepoint=0;
		afterpoint=0;
		longflag=0;
		contflag=1;
		format++;
		while (*format && contflag)
		{
			switch (*format)
			{
			case '.':
				ispoint=1;
				format++;
				break;	
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (ispoint)
					afterpoint=afterpoint*10+(*format)-'0';
				else
					beforepoint=beforepoint*10+(*format)-'0';
				format++;
				break;
			case 'l':
				if (longflag==0)
					longflag=1;
				else
					longflag=2;
				format++;
				break;
			case 'L':
				longflag=2;
				format++;
				break;
			case 'f':
			case 'd':
			case 'x':
			case 'X':
			case 'c':
			case 's':
			case '%':
				contflag=0;
				break;
			default:
				contflag=0;
			}
		}
		if (contflag)
			break;
		switch (*format)
		{
		case '%':
			fputc('%',file);
			break;
		case 'd':
			if (longflag==2)
			{
				number=*((long long *)arg);
				arg+=sizeof(long long);
			}else
			{
				number=*((int*)arg);
				arg+=sizeof(int);
			}
			if (number<0)
			{
				beforepoint--;
				fputc('-',file);
				number=-number;
			}
			i=0;
			while (number>0)
			{
				buffer[i]='0'+number%10;
				number=number/10;
				i++;
			}
			while (i<beforepoint)
			{
				fputc(' ',file);
				beforepoint--;
			}
			while (i>0)
			{
				i--;
				fputc(buffer[i],file);
			}
			break;
		case 'c':
			fputc(*(char*)arg,file);
			arg+=sizeof(char);
			break;
		case 's':
			str=*(char**)arg;
			arg+=sizeof(char*);
			if (beforepoint==0)
				beforepoint--;
			while (*str && beforepoint)
			{
				fputc(*str,file);
				beforepoint--;
				str++;
			}
			break;
		case 'x':
			if (longflag==2)
			{
				number=*((long long *)arg);
				arg+=sizeof(long long);
			}else
			{
				number=*((int*)arg);
				arg+=sizeof(int);
			}
			if (number<0)
			{
				beforepoint--;
				fputc('-',file);
				number=-number;
			}
			i=0;
			while (number>0)
			{
				buffer[i]=xdigs_lower[number & 15];
				number=number>>4;
				i++;
			}
			while (i<beforepoint)
			{
				fputc(' ',file);
				beforepoint--;
			}
			while (i>0)
			{
				i--;
				fputc(buffer[i],file);
			}
			break;
		case 'X':
			if (longflag==2)
			{
				number=*((long long *)arg);
				arg+=sizeof(long long);
			}else
			{
				number=*((int*)arg);
				arg+=sizeof(int);
			}
			if (number<0)
			{
				beforepoint--;
				fputc('-',file);
				number=-number;
			}
			i=0;
			while (number>0)
			{
				buffer[i]=xdigs_upper[number & 15];
				number=number>>4;
				i++;
			}
			while (i<beforepoint)
			{
				fputc(' ',file);
				beforepoint--;
			}
			while (i>0)
			{
				i--;
				fputc(buffer[i],file);
			}
			break;
		case 'f':
		        if (longflag==2)
		        	arg+=10;
		        else if (longflag==1)
		        	arg+=8;
		        else
		        	arg+=4;
			break;					
		}
		format++;
	}
}
