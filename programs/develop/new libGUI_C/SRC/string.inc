/*
	some libC function working with memory
*/

static void *memmove(void *dst,const void *src,size_t length)
{
	void *value;

	if (length & 3)
	{//length not aligned in 4 bytes use reb movsb
		__asm__ __volatile__(
		"movl %%edi,%%eax\n\t"
		"cld\n\t"
		"rep\n\t"
		"movsb"
		:"=D"(value)
		:"c"(length),"S"(src),"D"(dst)
		:"eax");
	}
	else
	{//length aligned in 4 bytes use rep movsd
		length=length >> 2;//length=length/4
		__asm__ __volatile__(
		"movl %%edi,%%eax\n\t"
		"cld\n\t"
		"rep\n\t"
		"movsd"
		:"=D"(value)
		:"c"(length),"S"(src),"D"(dst)
		:"eax");

	}
	return(value);
}

static size_t strlen(const char *s)
{
	size_t	i;

	i=0;
	while(*s!='\0')
	{
		i++;
		s++;
	}
	return(i);
}

static char* strchr(const char *string, int c)
{
	while(*string!='\0')
	{
		if (*string==(char)c) return((char*)string);
		string++;
	}
	return(NULL);
}

static char* strrchr(const char *string, int c)
{
	char 	*s;
	int	i,j;

	s=(char*)string;
	while(*s!='\0')	{s++;}

	j=(int)(s-string);
	s--;

	for(i=0;i<j;i++)
	{
		if (*s==(char)c) return(s);
		s--;
	}

	return(NULL);
}

static char* strstr(const char *s1,const char *s2)
{
	char	*s;
	int	i,j,len1,len2;

	len2=strlen(s2);
	if (len2==0) return((char*)s1);

	len1=strlen(s1);
	for(i=0;i<len1-len2+1;i++)
	{
		if (s1[i]==s2[0])
		{
			for(j=0;j<len2;j++)
			{
				if (s1[i+j]!=s2[j]) break;
			}
			if (j==len2) return((char*)(s1+i));
		}
	}
	return(NULL);
}

static int strcmp(const char* string1, const char* string2)
{
	while (1)
	{
		if (*string1<*string2)
			return -1;
		if (*string1>*string2)
			return 1;
		if (*string1=='\0')
			return 0;
		string1++;
		string2++;
	}
}

static int strncmp(const char* string1, const char* string2, int count)
{
	while(count>0 && *string1==*string2)
	{
		if (*string1) return 0;
		++string1;
		++string2;
		--count;
	}
	if(count) return (*string1 - *string2);
	return 0;
}

static int sprintf(char *dest,const char *format,...)
{
  va_list arg;
  va_start (arg, format);
  return format_print(dest,4096, format, arg);
}

static int snprintf(char *dest, size_t size,const char *format,...)
{
  va_list arg;
  va_start (arg, format);
  return format_print(dest,size, format, arg);
}

static int vsnprintf(char *dest, size_t size,const char *format,va_list ap)
{
  return format_print(dest,size, format, ap);
}


