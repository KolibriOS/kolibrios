#include<stdio.h>
#include<stdlib.h>
#include<string.h>

extern char __curdir_buf[1024];

char* __get_curdir(void) {return __curdir_buf;}

static void ___chdir(char* dest, const char* src)
{
	// handle absolute paths
	if (src[0]=='/')
	{
		strcpy(dest,src);
		return;
	}
	// handle relative paths
	char* ptr = dest + strlen(dest);
	while (*src)
	{
		if (src[0] == '.' && src[1] == 0)
			break;
		if (src[0] == '.' && src[1] == '/')
		{++src;++src;continue;}
		if (src[0] == '.' && src[1] == '.' &&
			(src[2] == 0 || src[2] == '/'))
		{
			while (ptr > dest && ptr[-1] != '/')
				--ptr;
			++src;++src;
			if (*src == 0) break;
			++src;
			continue;
		}
		*ptr++ = '/';
		if (*src == '/') ++src;
		while (*src && *src!='/') *ptr++ = *src++;
	}
	*ptr = 0;
}

void __chdir(const char* path)
{
	___chdir(__curdir_buf,path);
}

static char __libc_combine_buffer[1024];
char* __libc_combine_path(const char* c)
{
	strcpy(__libc_combine_buffer,__curdir_buf);
	___chdir(__libc_combine_buffer,c);
	return __libc_combine_buffer;
}


#ifdef __TEST_IN_DOS__

int main(void)
{
 init_dir_stack();
 printf("|%s|\n",__get_curdir());
 __chdir("jp/1/2/3");
 printf("|%s|\n",__get_curdir());
 __chdir("/jp/1/2/3");
 printf("|%s|\n",__get_curdir());
 __chdir("../4");
 printf("|%s|\n",__get_curdir());
 __chdir("./../..");
 printf("|%s|\n",__get_curdir());
 printf("Combined=|%s|\n",combine_path("./abc/def/../../../rd/2"));
 return 0;
}
#endif
