/*
	2015
	Author: Pavel Yakovlev.
*/

#define LIB_NAME "fs"

#include "coff.h"
#include <kolibri.c>
#include <stdlib.c>


//char _PATH_[4096];
char *_PATH_;
dword *_ADR_ = 32;
char *FS_SELF_PATH;
char *FS_SELF_DIR=0;

char TMP1[4096] = {0};
char TMP2[4096] = {0};

static void*(* _stdcall pointer_callback_copy)(void);
static void*(* _stdcall pointer_callback_move)(void);
static void*(* _stdcall pointer_callback_remove)(void);

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
static inline char get_bdvk(char *path)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 5;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = 0;
	file_read_struct.p16 = (unsigned)&ret_struct;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	return FS_file_70(&file_read_struct);
	//return (FS_struct_BDVK*)&ret_struct;
}

static inline char set_bdvk(char *path,FS_struct_BDVK *bdvk)
{
	FS_struct70 file_read_struct;
	file_read_struct.p00 = 6;
	file_read_struct.p04 = 0;
	file_read_struct.p08 = 0;
	file_read_struct.p12 = 0;
	file_read_struct.p16 = bdvk;
	file_read_struct.p20 = 0;
	file_read_struct.p21 = path;
	return FS_file_70(&file_read_struct);
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


static inline byte strncmp(char *s1,const char *s2, dword n)
{
	while(n)
	{
		if(*s1!=*s2)return *s1-*s2;
		++s1;
		++s2;
		--n;
	}
	return 0;
}

char *strdup(char *str)
{
	char *r = malloc(strlen(str));
	strcpy(r,str);
	return r;
}

char *self_get_dir(void)
{
	if(!FS_SELF_DIR)
	{
		FS_SELF_DIR = malloc(4096);
		//FS_SELF_PATH = malloc(4096);
		//strcpy(FS_SELF_PATH,*_ADR_);
		FS_SELF_PATH = *_ADR_;
		int pos=0;
		int tmp=0;

		while(FS_SELF_PATH[pos])
		{
			FS_SELF_DIR[pos] = FS_SELF_PATH[pos];
			if(FS_SELF_PATH[pos]=='/')tmp = pos;
			++pos;
		}
		if(tmp)pos=tmp;
		FS_SELF_DIR[pos]=0;
	}
	return FS_SELF_DIR;
}


char *get_full_path(char *path)
{
	self_get_dir();
	if(!path) return FS_SELF_DIR;
	char *pos = path;
	if(!_PATH_) _PATH_ = malloc(4096);
	if(*pos=='/')
	{
		++pos;
		if(!strncmp(pos,"sys/",4)) return path;
		if(!strncmp(pos,"kolibrios/",10)) return path;
		if(!strncmp(pos,"rd/",3)) return path;
		if(!strncmp(pos,"fd/",3)) return path;
		if(!strncmp(pos,"cd",2)) return path;
		if(!strncmp(pos,"hd",2)) return path;
		if(!strncmp(pos,"bd",2)) return path;
		if(!strncmp(pos,"tmp",3)) return path;
		if(!strncmp(pos,"usbhd",5)) return path;
		if(!*pos)return path;
		strcpy(_PATH_,"/sys");
		strcpy(_PATH_+4,path);
		return _PATH_;
	}
	if(!strncmp(path,"./",2)) return path;
	sprintf(_PATH_,"%s/%s",FS_SELF_DIR,path);
	return _PATH_;
}

long long FS_LENGHT=0;

dword *__BUF__COUNT__ = 0;
int COUNT_FILE = 0;
int _get_entries_count(char *path)
{
	//char BUF[32];
	if(!__BUF__COUNT__)__BUF__COUNT__=malloc(32);
	if(!file_read_dir(0,0, __BUF__COUNT__, path))
	{
		if(strcmp(path,"/"))COUNT_FILE = *(__BUF__COUNT__+2)-2;
		else COUNT_FILE = *(__BUF__COUNT__+2);
		return COUNT_FILE;
	}
	COUNT_FILE = -1;
	return -1;
}

int get_entries_count(char *path)
{
	return _get_entries_count(get_full_path(path));
}

dword _get_dir_info(char *path)
{
	_get_entries_count(path);
	if(!strcmp(path,"/"))
	{
		if(COUNT_FILE==-1)return 0;
		dword buffer = malloc(304*COUNT_FILE+72);
		file_read_dir(0,COUNT_FILE,buffer,path);
		return buffer;
	}
	if(COUNT_FILE==-1)return 0;
	dword buffer = malloc(304*COUNT_FILE+72);
	file_read_dir(2,COUNT_FILE+2,buffer,path);
	return buffer;
}

dword get_dir_info(char *path)
{
	return _get_dir_info(get_full_path(path));
}

dword get_dir_position(dword a,dword i)
{
	return i*344+32+a;
}

char *get_file_name(void *args)
{
	return args+40;
}

char BUF_HEADER[5]={0};
dword read(char *path)
{
	dword *adr = &BUF_HEADER;
	path = get_full_path(path);
	file_read_binary(path,0,4,adr);
	if(*adr=='KCPK') return file_readKPACK(path);
	dword ret = file_read(path);
	//FS_LENGHT = size_long_file;
	asm volatile(""::"c"((long)size_long_file));
	return ret;
}


dword quantity_dir = 0,quantity_file = 0;
long long fs_size_global = 0;
long long LOOP_SIZE(char *path)
{
	dword tmp_len = file_size(path);
	if(ret_struct.p00&0b10000)
	{
		++quantity_dir;
		dword tmp_buf = get_dir_info(path);
		if(COUNT_FILE<1) return 0;
		
		char *new_path_file = malloc(4096);
		int tmp_count = COUNT_FILE;
		int i = 0;
		fs_size_global = 0;
		while(i<tmp_count)
		{
			sprintf(new_path_file,"%s/%s",path,304*i+tmp_buf+72);
			fs_size_global += LOOP_SIZE(new_path_file);
			++i;
		}
		free(tmp_buf);
		free(new_path_file);
		return fs_size_global;
	}
	++quantity_file;
	return tmp_len;
}

dword get_size(char *path)
{
	quantity_dir = 0;
	quantity_file = 0;
	long long size = LOOP_SIZE(get_full_path(path));
	if(quantity_dir>0)--quantity_dir;
	asm volatile(""::"c"(quantity_dir),"d"(quantity_file));
	return (dword)size;
}

/*
char set_attributes_loop(char *path,dword strucs)
{
	if(get_bdvk(path))return 0;
	dword tmp = (ret_struct.p00^strucs)&0b111;
	ret_struct.p00=(tmp^ret_struct.p00)^0xFFFFFFFF;
	if(ret_struct.p00&0b10000)
	{
		dword tmp_buf = _get_dir_info(path);
		if(!COUNT_FILE) goto END;
		
		char *new_path_file = malloc(4096);
		int tmp_count = COUNT_FILE;
		int i = 0;
		char *position = tmp_buf+72;
		while(i<tmp_count)
		{	
			sprintf(new_path_file,"%s/%s",path,position);
			position+=304;
			if(!set_attributes_loop(new_path_file,strucs))
			{
				free(tmp_buf);
				free(new_path_file);
				return 0;
			}
			++i;
		}
		free(new_path_file);
		END:
		free(tmp_buf);
	}
	if(set_bdvk(path,&ret_struct))return 0;
	return 1;
}

char set_attributes(char *path,dword strucs,char cmd)
{
	if(cmd)return set_attributes_loop(get_full_path(path),strucs);
	
}
*/

char remove_loop(char *path)
{
	if(get_bdvk(path))return 0;
	if(ret_struct.p00&0b10000)
	{
		dword tmp_buf = _get_dir_info(path);
		if(!COUNT_FILE) goto END;
		
		char *new_path_file = malloc(4096);
		int tmp_count = COUNT_FILE;
		int i = 0;
		char *position = tmp_buf+72;
		while(i<tmp_count)
		{	
			sprintf(new_path_file,"%s/%s",path,position);
			position+=304;
			if(!remove_loop(new_path_file))
			{
				free(tmp_buf);
				free(new_path_file);
				return 0;
			}
			++i;
		}
		free(new_path_file);
		END:
		free(tmp_buf);
	}
	if(file_delete(path))return 0;
	return 1;
}

char remove(char *path)
{
	return remove_loop(get_full_path(path));
}

int make_dir(char *path)
{
	return file_mkdir(get_full_path(path));
}

char copy_loop(char *path1,char *path2)
{
	if(get_bdvk(path1))return 0;
	if(ret_struct.p00&0b10000)
	{
		file_mkdir(path2);
		set_bdvk(path2,&ret_struct);
		dword tmp_buf = _get_dir_info(path1);
		if(!COUNT_FILE) goto END;
		
		char *new_path_file1 = malloc(4096);
		char *new_path_file2 = malloc(4096);
		int tmp_count = COUNT_FILE;
		int i = 0;
		dword position = tmp_buf+72;
		while(i<tmp_count)
		{
			sprintf(new_path_file1,"%s/%s",path1,position);
			sprintf(new_path_file2,"%s/%s",path2,position);
			if(pointer_callback_copy)
			{
				asm volatile(""::"c"(new_path_file1),"d"(new_path_file2));
				pointer_callback_copy();
			}
			position+=304;
			if(!copy_loop(new_path_file1,new_path_file2))
			{
				free(tmp_buf);
				free(new_path_file1);
				free(new_path_file2);
				return 0;
			}
			++i;
		}
		free(new_path_file1);
		free(new_path_file2);
		END:
		free(tmp_buf);
		return 1;
	}
	char r = file_copy(path1,path2);
	set_bdvk(path2,&ret_struct);
	return r;
}

char copy(char *path1,char *path2)
{
	char *tmp = get_full_path(path1);
	path1 = malloc(4096);
	strcpy(path1,tmp);
	
	char *r=copy_loop(path1,get_full_path(path2));
	
	free(path1);
	return r;
}

char move_loop(char *path1,char *path2)
{
	if(get_bdvk(path1))return 0;
	if(ret_struct.p00&0b10000)
	{
		file_mkdir(path2);
		set_bdvk(path2,&ret_struct);
		dword tmp_buf = _get_dir_info(path1);
		if(!COUNT_FILE) goto END;
		
		char *new_path_file1 = malloc(4096);
		char *new_path_file2 = malloc(4096);
		int tmp_count = COUNT_FILE;
		int i = 0;
		dword position = tmp_buf+72;
		while(i<tmp_count)
		{
			sprintf(new_path_file1,"%s/%s",path1,position);
			sprintf(new_path_file2,"%s/%s",path2,position);
			if(pointer_callback_move)
			{
				asm volatile(""::"c"(new_path_file1),"d"(new_path_file2));
				pointer_callback_copy();
			}
			position+=304;
			if(!move_loop(new_path_file1,new_path_file2))
			{
				free(tmp_buf);
				free(new_path_file1);
				free(new_path_file2);
				return 0;
			}
			++i;
		}
		free(new_path_file1);
		free(new_path_file2);
		END:
		free(tmp_buf);
		if(file_delete(path1)) return 0;
		return 1;
	}
	char r = 0;
	if(file_copy(path1,path2)) if(!file_delete(path1))r=1;
	if(r)set_bdvk(path2,&ret_struct);
	return r;
}


char move(char *path1,char *path2)
{
	char *tmp = get_full_path(path1);
	path1 = malloc(4096);
	strcpy(path1,tmp);
	
	char *r=move_loop(path1,get_full_path(path2));
	
	free(path1);
	return r;
}

char write(char *path,void *data,dword size)
{
	if(file_write(get_full_path(path),data,size))return 0;
	return 1;
}

dword execute(char *path,char *arg)
{
	return file_run(get_full_path(path),arg);
}

dword open(char *path)
{
	return file_run("/sys/@open",get_full_path(path));
}

char rename(char *path,char *new_name)
{
	if(!strcmp(path,new_name))return 0;
	char *pos = path;
	char *pos1 = &TMP1;
	char *tmp = 0;
	while(*pos)
	{
		*pos1=*pos;
		if(*pos=='/')
		{
			if(*++pos)tmp = pos1;
		}
		else ++pos;
		++pos1;
	}
	if(tmp)
	{
		while(*new_name)*++tmp = *new_name++;
		*++tmp = 0;
		return move(path,&TMP1);
	}
	return move(path,new_name);
}

char callback_copy(char *path1,char *path2,dword func)
{
	pointer_callback_copy = func;
	return copy(path1,path2);
}

char callback_move(char *path1,char *path2,dword func)
{
	pointer_callback_move = func;
	return move(path1,path2);
}

char callback_remove(char *path,dword func)
{
	pointer_callback_remove = func;
	return remove(path);
}

char *version = "Ver. 1.3, Author:Pavel Yakovlev, http://vk.com/pavelyakov39";

EXPORT_
	export(execute)
	export(open)
	export(read)
	export(write)
	export(copy)
	export(move)
	export(remove)
	export(rename)
	export(get_size)
	//export(set_attributes)
	export(get_entries_count)
	export(get_dir_info)
	export(get_dir_position)
	export(get_file_name)
	
	export(get_full_path)
	
	export(make_dir)
	
	export(callback_copy)
	export(callback_move)
	export(callback_remove)
	
	export(pointer_callback_copy)
	export(pointer_callback_move)
	export(pointer_callback_remove)
	
	export(version)
_EXPORT