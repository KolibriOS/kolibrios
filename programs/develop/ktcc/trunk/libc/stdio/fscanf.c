#include <stdio.h>
void skipspaces(FILE* file)
{
	int c;
	while(1)
	{
		c=getc(file);
		if (c!=' ' && c!='\r' && c!='\n')
		{
			ungetc(c,file);
			return;
		}
	}
}
int fscanf(FILE* file,const char* format, ...)
{
	int res;
	void* arg;
	int i;
	int c;
	int contflag;
	int longflag;
	int sign;
	long long number;
	long double rnumber;
	char* str;
	res=0;
	arg=&format;
	arg+=sizeof(const char*);
	while (*format!='\0')
	{
		if (*format!='%')
		{
			c=fgetc(file);
			if (c!=*format)
			{
				fungetc(c,file);
				return -1;
			}
			format++;
			continue;
		}
		contflag=1;
		longflag=0;
		while (*format && contflag)
		{
			switch(*format)
			{
				case '.':
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
					format++;
					continue;
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
		switch(*format)
		{
		case '%':
			c=fgetc(file);
			if (c!='%')
			{
				ungetc(c,file);
				return -1;
			}
			res--;
			break;
		case 'd':
			number=0;
			sign=1;
			skipspaces(file);
			c=fgetc(file);
			if (c=='-')
			{
				sign=-1;
			}else if (c!='+')
				ungetc(c,file);
			contflag=0;
			while(1)
			{
				c=fgetc(file);
				if (c>='0' && c<='9')
				{
					contflag++;
					number=number*10+(c-'0');
				}else
					break;
			}
			ungetc(c,file);
			if (!contflag)
				return res;
			if (longflag<=1)
			{
				*((int*)arg)=number;
				arg+=sizeof(int);
			}else
			{
				*((long long*)arg)=number;
				arg+=sizeof(long long);
			}
			break;
		case 'c':
			c=fgetc(file);
			if (c==EOF)
				return res;
			*((char*)arg)=c;
			arg+=sizeof(char);
			break;
		case 's':
			skipspaces(file);
			contflag=0;
			str=*((char**)arg);
			arg+=sizeof(char*);
			while(1)
			{
				c=fgetc(file);
				if (c==EOF || c==' ' || c=='\n' || c=='\r')
				{
					ungetc(c,file);
					break;
				}
				*str=c;
				str++;
				contflag++;
			}
			if (!contflag)
				return res;
			break;
		case 'f':
			skipspaces(file);
			// TODO: read real numbers
			rnumber=0;
			switch (longflag)
			{
			case 0:
				*((float*)arg)=rnumber;
				arg+=sizeof(float);
				break;
			case 1:
				*((double*)arg)=rnumber;
				arg+=sizeof(double);
				break;
			case 2:
				*((long double*)arg)=rnumber;
				arg+=sizeof(long double);
				break;
			default:
				return res;
			}
			break;
		default:
			break;
		}
		format++;
		res++;
	}
	return res;	
}
