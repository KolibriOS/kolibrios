#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

static void * ___brk_addr = 0;
extern char end[];
static unsigned long min_brk,max_brk,cur_brk;
extern unsigned long __menuet__memsize;
static void* cur_dynamic_area;
static unsigned cur_total_size;

static inline void heap_init(void)
{
	__asm__("int $0x40" :: "a"(68),"b"(11));
}

static inline void* heap_alloc(unsigned size)
{
	void* res;
	__asm__("int $0x40" : "=a"(res) : "a"(68),"b"(12),"c"(size));
	return res;
}

static inline void heap_free(void* ptr)
{
	__asm__("int $0x40" :: "a"(68),"b"(13),"c"(ptr));
}

void init_brk(void)
{
 cur_brk=min_brk=(((unsigned long)&end)+0xfff)&~0xfff;
 max_brk=(__menuet__memsize-32768)&~0xfff;
 ___brk_addr=(void *)min_brk;
 cur_dynamic_area = NULL;
 cur_total_size = max_brk;
 heap_init();
}

/*static int sys_brk(unsigned long end_data_seg)
{
 if(!end_data_seg) return cur_brk;
 if (end_data_seg >= min_brk &&
     end_data_seg < max_brk)
  cur_brk = end_data_seg;
 return cur_brk;
}*/

/*int brk(void *_heaptop)
{
 return sys_brk((unsigned long)_heaptop); 
}

static int __init_brk (void)
{
 if (___brk_addr == 0)
 {
  ___brk_addr=(void *)sys_brk(0);
  if (___brk_addr == 0)
  {
   errno = ENOMEM;
   return -1;
  }
 }
 return 0;
}*/

void * sbrk(int increment)
{
/* if (__init_brk () == 0)
 {
  void * tmp = ___brk_addr+increment;
  ___brk_addr=(void *)sys_brk((unsigned long)tmp);
  if (___brk_addr == tmp) return tmp-increment;
  errno = ENOMEM;
  return ((void *) -1);
 }
 return ((void *) -1);*/
	void* res;
	unsigned long tmp;
	if (increment <= 0)
	{
		/* release memory */
		while (cur_dynamic_area && increment)
		{
			tmp = cur_brk - (unsigned long)cur_dynamic_area -
				3*sizeof(void*);
			if (tmp > increment)
				tmp = increment;
			cur_brk -= tmp;
			increment -= tmp;
			if (cur_brk == (unsigned long)cur_dynamic_area + 3*sizeof(void*))
			{
				cur_brk = (unsigned long)((void**)cur_dynamic_area)[1];
				max_brk = (unsigned long)((void**)cur_dynamic_area)[2];
				res = cur_dynamic_area;
				cur_dynamic_area = ((void**)cur_dynamic_area)[0];
				heap_free(res);
			}
		}
		if (!cur_dynamic_area)
		{
			cur_brk += increment;
			if (cur_brk < min_brk)
				cur_brk = min_brk;
		}
		return (void*)cur_brk;
	}
	/* allocate memory */
	if (cur_brk + increment <= max_brk)
	{
		/* We have memory in current block, so use it */
		res = (void*)cur_brk;
		cur_brk += increment;
		return res;
	}
	tmp = 65536;
	/* We do not have memory in current block, so allocate new one */
	if (increment > 65536 - 3*sizeof(void*))
		tmp = (increment + 3*sizeof(void*) + 0xFFF) & ~0xFFF;
	res = heap_alloc(tmp);
	if (!res)
	{
		errno = ENOMEM;
		return (void*)-1;
	}
	((void**)res)[0] = cur_dynamic_area;
	((void**)res)[1] = (void*)cur_brk;
	((void**)res)[2] = (void*)max_brk;
	cur_dynamic_area = res;
	cur_brk = (unsigned long)res + 3*sizeof(void*);
	max_brk = (unsigned long)res + tmp;
	res = (void*)cur_brk;
	cur_brk += increment;
	return (void*)res;
}
