/*
	2015
	Author: Pavel Yakovlev.
*/

#define LIB_NAME "array"

#include "coff.h"

#include <string.c>
#include <stdlib.c>

typedef struct
{
	dword key;
	dword value;
} array;

char *ARRAY_KEY_STRING = (char *)0;
char *ARRAY_VALUE_STRING = (char *)0;
array *ARRAY_ADRESS = (dword)0;

dword hash_binary(char *data,dword size)
{
	byte h1,h2,h3,h4;
	if(!size) return 0;
	h1 = *data;
	if(size==1) return h1<<24;
	h2 = *++data;
	if(size==2) return (h1<<24)|(h2<<16);
	h3 = *++data;
	if(size==3) return (h1<<24)|(h2<<16)|(h3<<8);
	h4 = *++data;
	if(size==4) return (h1<<24)|(h2<<16)|(h3<<8)|(h4);
	
	size-=4;
	
	while(size--)
	{
		h1^=*data;h2^=*data;h3^=*data;h4^=*data;
		++data;
	}
	
	return (h1<<24)|(h2<<16)|(h3<<8)|(h4);
}
dword hash_string(char *data)
{
	byte h1,h2,h3,h4;
	if(!*data) return 0;
	h1 = *data;
	if(!*++data) return h1<<24;
	h2 = *data;
	if(!*++data) return (h1<<24)|(h2<<16);
	h3 = *data;
	if(!*++data) return (h1<<24)|(h2<<16)|(h3<<8);
	h4 = *data;
	if(!*++data) return (h1<<24)|(h2<<16)|(h3<<8)|(h4);
	
	while(*data)
	{
		h1^=*data;h2^=*data;h3^=*data;h4^=*data;
		++data;
	}
	
	return (h1<<24)|(h2<<16)|(h3<<8)|(h4);
}

dword hash_integer(dword dec)
{
	dword tmp = dec;
	tmp <<= 16;
	tmp ^= 0xFFFFFFFF;
	dec &= tmp;
	dec ^= dec>>5;
	dec += dec<<3;
	dec ^= dec>>13;
	tmp = dec<<9;
	tmp ^= 0xFFFFFFFF;
	dec += tmp;
	dec ^= dec>>17;
	return dec;
}

dword buffer = 0xFF;
dword ARRAY_COUNT = 0;
dword tmp,tmp1;

array *key_string_set(dword *ary,char *key,void *data)
{
	//dword *ptr = (dword *)ary;
	array *m;
	if(!*ary)
	{
		*ary = (dword)malloc(sizeof(array)*(buffer+1));
		m = (array *)*ary;
		m[0].value = buffer;
	}
	
	else
	{
		m = (array *)*ary;
		if(m[0].value<=m[0].key)
		{
			m[0].value += buffer;
			*ary = (dword)realloc((void*)*ary,sizeof(array)*(m[0].value+1));
		}
	}
	
	++m[0].key;
	
	dword tmp = hash_string(key)%buffer;
	tmp+=(m[0].value/buffer)*buffer;
	++tmp;
	
	while(m[tmp].key)
	{
		if(!strcmp(key,(char*)m[tmp].key))break;
		++tmp;
	}
	m[tmp].key = (dword)key;
	m[tmp].value = (dword)data;
	return m;
}

array *key_binary_set(dword *ary,void *key,void *data,dword size)
{
	
	array *m;
	if(!*ary)
	{
		*ary = (dword)malloc(sizeof(array)*(buffer+1));
		m = (array *)*ary;
		m[0].value = buffer+1;
	}
	else
	{
		m = (array *)*ary;
	}
	
	++m[0].key;
	
	dword tmp = hash_string(key)%buffer;
	++tmp;
	while(m[tmp].key)
	{
		if(!strncmp((char*)key,(char*)m[tmp].key,size))break;
		++tmp;
	}
	m[tmp].key = (dword)key;
	m[tmp].value = (dword)data;
	return m;
}

array *key_integer_set(dword *ary,dword key,void *data)
{
	
	
	if(!*ary)
	{
		//(dword)m[0].value = (dword)buffer;
		*ary = (dword)malloc(sizeof(array)*(buffer+1));
	}
	
	array *m = (array *)*ary;
	
	dword tmp = hash_integer(key)%buffer;
	
	while(m[tmp].key)
	{
		if(key==m[tmp].key)break;
		++tmp;
	}
	m[tmp].key = (dword)key;
	m[tmp].value = (dword)data;
	return m;
}


void *key_string_get(dword *ary,char *key)
{
	if(!*ary)return 0;
	
	array *m = (array *)*ary;
	
	dword tmp = hash_string(key)%buffer;
	tmp+=(m[0].value/buffer)*buffer;
	++tmp;
	while(m[tmp].key)
	{
		if(!strcmp(key,(char*)m[tmp].key))break;
		++tmp;
	}
	return (void *)m[tmp].value;
}

void *key_binary_get(dword *ary,char *key,dword size)
{
	if(!*ary)return 0;
	
	array *m = (array *)*ary;
	
	dword tmp = hash_string(key)%buffer;
	
	while(m[tmp].key)
	{
		if(!strncmp(key,(char*)m[tmp].key,size))break;
		++tmp;
	}
	return (void *)m[tmp].value;
}

void *key_integer_get(dword *ary,dword key)
{
	if(!*ary)return 0;
	
	array *m = (array *)*ary;
	
	dword tmp = hash_integer(key)%buffer;
	
	while(m[tmp].key)
	{
		if(key==m[tmp].key)break;
		++tmp;
	}
	return (void *)m[tmp].value;
}


EXPORT_
	export(buffer)
	
	export(hash_string)
	export(hash_binary)
	export(hash_integer)
	
	export(key_string_set)
	export(key_binary_set)
	export(key_integer_set)
	
	export(key_string_get)
	export(key_binary_get)
	export(key_integer_get)
	
_EXPORT