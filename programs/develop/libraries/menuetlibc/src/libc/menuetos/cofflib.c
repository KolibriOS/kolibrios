#include <menuet/os.h>
#include <stdio.h>

IMP_TABLE __kolibri__cofflib_load(const char* name){
	__asm__ __volatile__("int $0x40"::"a"(68L),"b"(19L),"c"((__u32)name));
}

__u32 __kolibri__cofflib_getproc(IMP_TABLE lib, const char* name){
	if(!name || !name[0]) return NULL;
	int i;
	for(i = 0; lib[i].name && strcmp(name, lib[i].name); i++);
	if(lib[i].name) return (__u32)lib[i].pointer;
	else return NULL;
}
