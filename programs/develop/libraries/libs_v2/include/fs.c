#ifndef FS_C_INCLUDE
#define FS_C_INCLUDE

#include "stdlib.c"

#pragma pack(push,1)
typedef struct 
{
unsigned	p00;
unsigned	p04;
unsigned	p08;
unsigned	p12;
unsigned	p16;
char		p20;
char		*p21;
} FS_struct70;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
unsigned	p00;
char		p04;
char		p05[3];
unsigned	p08;
unsigned	p12;
unsigned	p16;
unsigned	p20;
unsigned	p24;
unsigned	p28;
//unsigned	p32[2];
long long p32;
unsigned	p40;
} FS_struct_BDVK;
#pragma pack(pop)

#define FS_COPY_BUFFER_SET 0x100000
static inline char BUF_COPY[FS_COPY_BUFFER_SET] = {0};

static inline char *string_error_code(char code)
{
	if(!code)return "Successfully!";
	if(code==1)return "Not defined basis and / or hard disk partition (sub-functions 7, 8 function 21)!";
	if(code==2)return "Function is not supported for this file system!";
	if(code==3)return "Unknown file system!";
	if(code==4)return "Reserved, never returned to the current implementation!";
	if(code==5)return "File not found!";
	if(code==6)return "The file is ended!";
	if(code==7)return "Pointer out the application memory!";
	if(code==8)return "Disk is full!";
	if(code==9)return "FAT table is destroyed!";
	if(code==10)return "Access is denied!";
	if(code==11)return "Device error!";
	return "An unexpected error!";
}

static inline dword sprintf(char *mstr,const char *fmt,...)
{
	
	dword *arg = &fmt;
	char *tmp = 0;
	char *pos = mstr;
	--pos;
	--fmt;
	while(*++fmt)
	{
		char s = *fmt;
		if(s=='%')
		{
			s = *++fmt;
			if(s=='s')
			{
				tmp = *++arg;
				while(*tmp)*++pos = *tmp++;
			}
			else
			{
				*++pos='%';
				--fmt;
			}
		}
		else *++pos=s;
	}
	*++pos = 0;
	return pos-mstr;
}

static inline int FS_file_70(FS_struct70 *k)
{
	asm volatile ("int $0x40"::"a"(70), "b"(k));
}

FS_struct_BDVK ret_struct;
static inline FS_struct_BDVK* get_bdvk(char *path)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 5;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = 0;
	file_read_struct.p16 = (unsigned)&ret_struct;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	FS_file_70(&file_read_struct);
	return (FS_struct_BDVK*)&ret_struct;
}

static inline long long file_size(char *path)
{
	FS_struct_BDVK *data =  get_bdvk(path);
	return data->p32;
}

static inline byte file_isdir(char *path)
{
	FS_struct_BDVK *data =  get_bdvk(path);
	if(data->p00&0b10000)return 1;
	return 0;
}

static inline byte file_ismetka(char *path)
{
	FS_struct_BDVK *data =  get_bdvk(path);
	if(data->p00&0b1000)return 1;
	return 0;
}

static inline byte file_isfile()
{
	//FS_struct_BDVK *data =  get_bdvk(path);
	if(ret_struct.p00&0b11000)return 0;
	return 1;
}

static inline int file_run(char *path,char *arg)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 7;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = arg;
	file_read_struct.p12 = 0;
	file_read_struct.p16 = 0;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	return FS_file_70(&file_read_struct);
	
}

static inline int file_read_binary(char *path,unsigned pos,unsigned size,void *adr)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 0;
	file_read_struct.p04 = pos;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = size;
	file_read_struct.p16 = (unsigned)adr;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	char c = FS_file_70(&file_read_struct);
	if(c)
	{
		sprintf(&BUF_COPY,"'Error read file (%s) file: %s.'E",string_error_code(c),path);
		file_run("/sys/@notify",&BUF_COPY);
	}
	return c;
}

static inline int file_delete(char *path)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 8;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = 0;
	file_read_struct.p16 = 0;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	char c = FS_file_70(&file_read_struct);
	if(c)
	{
		sprintf(&BUF_COPY,"'Error delete file: %s. Info: %s'E",string_error_code(c),path);
		file_run("/sys/@notify",&BUF_COPY);
	}
	return c;
}

static inline int file_mkdir(char *path)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 9;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = 0;
	file_read_struct.p16 = 0;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	char c = FS_file_70(&file_read_struct);
	if(c)
	{
		sprintf(&BUF_COPY,"'Error make dir: %s. Info: %s.'E",string_error_code(c),path);
		file_run("/sys/@notify",&BUF_COPY);
	}
	return c;
}

static inline int file_write(char *path,void *ukaz,dword size)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 2;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = size;
	file_read_struct.p16 = ukaz;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	char c = FS_file_70(&file_read_struct);
	if(c)
	{
		sprintf(&BUF_COPY,"'Error write file: %s. Info: %s.'E",string_error_code(c),path);
		file_run("/sys/@notify",&BUF_COPY);
	}
	return c;
}

static inline int file_rewrite(char *path,dword pos1,dword pos2,void *ukaz,dword size)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 3;
	file_read_struct.p04 = pos1;
	file_read_struct.p08 = pos2;
	file_read_struct.p12 = size;
	file_read_struct.p16 = ukaz;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	char c = FS_file_70(&file_read_struct);
	if(c)
	{
		sprintf(&BUF_COPY,"'Error rewrite file (%s) file: %s.'E",string_error_code(c),path);
		file_run("/sys/@notify",&BUF_COPY);
	}
	return c;
}


static inline char file_copy(char *path1,char *path2)
{
	long long size = file_size(path1);
	if(!size)file_write(path2,&BUF_COPY,0);
	long long cel = size;
	cel /= FS_COPY_BUFFER_SET;
	dword ost = size-cel*FS_COPY_BUFFER_SET;
	long long i = 0;
	char err=0;
	if(cel)
	{
		if(file_read_binary(path1,0,FS_COPY_BUFFER_SET,&BUF_COPY))goto ERROR;
		if(file_write(path2,&BUF_COPY,FS_COPY_BUFFER_SET))goto ERROR;
		++i;
	}
	else
	{
		if(file_read_binary(path1,0,ost,&BUF_COPY))goto ERROR;
		if(file_write(path2,&BUF_COPY,ost))goto ERROR;
		return 1;
	}
	while(i<cel)
	{
		if(file_read_binary(path1,FS_COPY_BUFFER_SET*i,FS_COPY_BUFFER_SET,&BUF_COPY))goto ERROR;
		if(file_rewrite(path2,FS_COPY_BUFFER_SET*i,0,&BUF_COPY,FS_COPY_BUFFER_SET))goto ERROR;
		++i;
	}
	if(file_read_binary(path1,FS_COPY_BUFFER_SET*i,ost,&BUF_COPY))goto ERROR;
	if(file_rewrite(path2,FS_COPY_BUFFER_SET*i,0,&BUF_COPY,ost))goto ERROR;
	return 1;
	ERROR:
	
	//asm("":"=a"(err));
	//sprintf(&BUF_COPY,"Error %s file: %s.",string_error_code(err),path1);
	//file_run("/sys/@notify",&BUF_COPY);
	return 0;
}

/*
dword FS_COPY_BUFFER_SET_POINT = 0x100000;
dword BUF_COPY_POINT = 0;
static inline char file_copy(char *path1,char *path2)
{
	long long size = file_size(path1);
	if(!size)file_write(path2,BUF_COPY_POINT,0);
	long long cel = size;
	cel /= FS_COPY_BUFFER_SET_POINT;
	dword ost = size-cel*FS_COPY_BUFFER_SET_POINT;
	long long i = 0;
	char err=0;
	if(cel)
	{
		if(file_read_binary(path1,0,FS_COPY_BUFFER_SET_POINT,BUF_COPY_POINT))goto ERROR;
		if(file_write(path2,BUF_COPY_POINT,FS_COPY_BUFFER_SET_POINT))goto ERROR;
		++i;
	}
	else
	{
		if(file_read_binary(path1,0,ost,BUF_COPY_POINT))goto ERROR;
		if(file_write(path2,BUF_COPY_POINT,ost))goto ERROR;
		return 1;
	}
	while(i<cel)
	{
		if(file_read_binary(path1,FS_COPY_BUFFER_SET_POINT*i,FS_COPY_BUFFER_SET_POINT,BUF_COPY_POINT))goto ERROR;
		if(file_rewrite(path2,FS_COPY_BUFFER_SET_POINT*i,0,BUF_COPY_POINT,FS_COPY_BUFFER_SET_POINT))goto ERROR;
		++i;
	}
	if(file_read_binary(path1,FS_COPY_BUFFER_SET_POINT*i,ost,BUF_COPY_POINT))goto ERROR;
	if(file_rewrite(path2,FS_COPY_BUFFER_SET_POINT*i,0,BUF_COPY_POINT,ost))goto ERROR;
	return 1;
	ERROR:
	
	//asm("":"=a"(err));
	//sprintf(BUF_COPY_POINT,"Error %s file: %s.",string_error_code(err),path1);
	//file_run("/sys/@notify",BUF_COPY_POINT);
	return 0;
}
*/
/*
#define FS_COPY_BUFFER_SET 0x100000
char BUF_COPY[FS_COPY_BUFFER_SET] = {0};
static inline char file_copy(char *path1,char *path2)
{
	long long size = file_size(path1);
	if(!size)file_write(path2,&BUF_COPY,0);
	long long cel = size;
	cel /= FS_COPY_BUFFER_SET;
	dword ost = size-cel*FS_COPY_BUFFER_SET;
	long long i = 0;
	
	if(cel)
	{
		file_read_binary(path1,0,FS_COPY_BUFFER_SET,&BUF_COPY);
		file_write(path2,&BUF_COPY,FS_COPY_BUFFER_SET);
		++i;
	}
	else
	{
		file_read_binary(path1,0,ost,&BUF_COPY);
		file_write(path2,&BUF_COPY,ost);
		return 1;
	}
	while(i<cel)
	{
		file_read_binary(path1,FS_COPY_BUFFER_SET*i,FS_COPY_BUFFER_SET,&BUF_COPY);
		file_rewrite(path2,FS_COPY_BUFFER_SET*i,0,&BUF_COPY,FS_COPY_BUFFER_SET);
		++i;
	}
	file_read_binary(path1,FS_COPY_BUFFER_SET*i,ost,&BUF_COPY);
	file_rewrite(path2,FS_COPY_BUFFER_SET*i,0,&BUF_COPY,ost);
	return 1;
}
*/
long long size_long_file=0;

static inline void *file_read(char *path)
{
	size_long_file = file_size(path);
	void *data = malloc(size_long_file+1);
	
	if(file_read_binary(path,0,size_long_file,data))return 0;
	
	return data;
}

static inline void *file_readKPACK(char *path)
{
	void *data = 0;
	dword size=0;
	asm ("int $0x40":"=a"(data),"=d"(size):"a"(68), "b"(27), "c"(path));
	size_long_file = size;
	asm volatile(""::"c"(size));
	return data;
}

static inline int file_read_dir(dword begin,dword file_count,dword read_buffer,char *dir_path)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 1;
	file_read_struct.p04 = begin;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = file_count;
	file_read_struct.p16 = read_buffer;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = dir_path;
	return FS_file_70(&file_read_struct);
}

#endif