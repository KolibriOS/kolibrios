/*
 * Easy and fast memory allocator from
 * https://wiki.osdev.org/Memory_Allocation
 * Coded by Siemargl, 2018
 * 
 * No Garbage Collector
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define UINT_MAX  (4294967295U)

#ifndef NDEBUG
#include <stdio.h>
#	ifdef __TINYC__
#		include <kolibrisys.h>
#		define TRACE1(s, a) { char buf[400]; sprintf(buf, s, a); debug_out_str(buf); }
#		define TRACE2(s, a, b) { char buf[400]; sprintf(buf, s, a, b); debug_out_str(buf); }
#	else
#		define TRACE1(s, a) printf(s, a)
#		define TRACE2(s, a, b) printf(s, a, b)
#	endif
#else
#	define TRACE1(s, a) (void)0
#	define TRACE2(s, a, b) (void)0
#endif




// get address, fromwhere function was called
#define CALLEDFROM(param1) (*(int*)((char*)&param1-4)-5)
   
const uint32_t c_used = 0x44455355; //'USED'
const uint32_t c_free = 0x45455246; //'FREE'

struct hdrfree {
	uint32_t 	mark; // 'FREE'
	size_t		size; // including header
	struct hdrfree	*prev;
	struct hdrfree	*next;
};

struct hdrused {
	uint32_t	mark; // 'USED'
	size_t		size;
};


static char *__freebase = NULL; 	// begin of free area
static char *__freetop = NULL;		// after last byte of free area
static struct hdrfree *__firstfree = NULL;	// ptr to first node in dual-link list

static struct {
	uint32_t	malloc_calls;
	uint32_t	malloc_max;
	uint32_t	malloc_sum;
	
	uint32_t	sysalloc_calls;
	uint32_t	sysalloc_max;
	uint32_t	sysalloc_sum;
	
	uint32_t	crtfreeblocks; // number of free blocks, checking corruptions
	uint32_t	freeblocks_sum;
} wtalloc_stat;


void *wtmalloc(size_t sz)
{
	struct hdrfree *fndnode, *newnode;
	sz = (sizeof(struct hdrused) + sz + 15) & ~15; // align 16bytes
//TRACE1("_call alloc(%d)\n", sz);

	//statistics
	wtalloc_stat.malloc_calls++;
	if (sz > wtalloc_stat.malloc_max) wtalloc_stat.malloc_max = sz;
	wtalloc_stat.malloc_sum += sz;
	
	// try to find free block enough size
	fndnode = __firstfree;
	while(fndnode)
	{
#ifndef NDEBUG
		if (fndnode->mark != c_free)
		{
			TRACE2("heap free block list corrupt %x  EIP@%x\n", fndnode, CALLEDFROM(sz));
			assert(0);
		}
#endif
		if (fndnode->size >= sz) break;
		fndnode = fndnode->next;
	}
	
	if (fndnode) // found free block
	{
		if (fndnode->size - sz > 15) // split smaller size, move free node
		{
//TRACE2("alloc(%d) split (%x)\n", sz, fndnode);
			wtalloc_stat.freeblocks_sum -= sz;
			newnode = (struct hdrfree*)((char*)fndnode + sz);
			newnode->mark = c_free;
			newnode->size = fndnode->size - sz;
			newnode->next = fndnode->next;
			newnode->prev = fndnode->prev;
			
			if (fndnode->next)
				fndnode->next->prev = newnode;
			
			//перед может быть не нода, а 1й указатель
			if (fndnode->prev)
				newnode->prev->next = newnode;
			else
				__firstfree = newnode;
		} else // nothing to split, just exclude
		{
//TRACE1("alloc(%d) remove freenode\n", sz);

			wtalloc_stat.crtfreeblocks--;
			wtalloc_stat.freeblocks_sum -= fndnode->size;
			if (fndnode->next)
				fndnode->next->prev = fndnode->prev;
			//перед может быть не нода, а 1й указатель
			if (fndnode->prev)
				fndnode->prev->next = fndnode->next;
			else
				__firstfree = fndnode->next;
		}
		
		fndnode->mark = c_used;
		fndnode->size = sz;
		return (char*)fndnode + sizeof(struct hdrused);
	}
	
	char *ptr;
	// free block not found, try to add @end
	if (__freetop - __freebase < sz)  // not enough memory - call system 
	{
		if (sz > UINT_MAX - 16) return NULL;  // check 32-bit heap overflow
//		size_t new_heap_size = (__freetop - __freebase + sz + 4095) & ~4095;   
		size_t new_heap_size = (sz + sz / 5 + 4095) & ~4095;  // 20% reserved 
		
		//statistics
		wtalloc_stat.sysalloc_calls++;
		if (new_heap_size > wtalloc_stat.malloc_max) wtalloc_stat.sysalloc_max = new_heap_size;
		wtalloc_stat.sysalloc_sum += new_heap_size;


		//хвост сунуть в свободные, а фритоп и базу перености на новый кусок
		ptr = sysmalloc(new_heap_size);   // rounded 4k
//TRACE2("call systemalloc(%d) returned %x\n", new_heap_size, ptr);
		if (!ptr)
		{
			TRACE2("sysmalloc failed trying to allocate %u bytes EIP@%x\n", sz, CALLEDFROM(sz));
			return NULL;
		}
		// add new free block in front of list
		if (__freetop - __freebase > 15)
		{
			newnode = (struct hdrfree*)__freebase;
			newnode->mark = c_free;
			newnode->size = __freetop - __freebase;
			newnode->next = __firstfree;
			newnode->prev = NULL;
			if (__firstfree)
				__firstfree->prev = newnode;
			__firstfree = newnode;
			wtalloc_stat.crtfreeblocks++;
			wtalloc_stat.freeblocks_sum += newnode->size;
//TRACE2("alloc(%d) add tail %d to freenode", sz, newnode->size);
//TRACE1(".tail [%x]\n", newnode);
		}
		// we don't save allocated block from system, so cant free them ltr
		
		__freebase = ptr;
		__freetop = __freebase + new_heap_size;
	}
	
	ptr = __freebase + sizeof(struct hdrused);
	((struct hdrused*)__freebase)->mark = c_used;
	((struct hdrused*)__freebase)->size = sz;
	__freebase += sz;
//TRACE1("__freebase [%x]\n", __freebase);


// check list availability
/*
int maxfree = 0;
for (fndnode = __firstfree; fndnode; fndnode = fndnode->next)
{
	if (fndnode->size > maxfree) maxfree = fndnode->size;
}

TRACE2("alloc(%d) from freebase, maxfree = %d,", sz, maxfree);
TRACE1(" freelist len = %u \n", wtalloc_stat.crtfreeblocks);
*/	
	return ptr;
}

void wtfree(void *ptr)
{
	if (!ptr) return;

//TRACE1("free() to freenode, sized %d\n", ((struct hdrused*)((char*)ptr - 8))->size);

#ifndef NDEBUG
	if (((struct hdrused*)((char*)ptr - 8))->mark != c_used)
	{
		TRACE2("try free unallocated block ptr = %x bytes EIP@%x\n", ptr, CALLEDFROM(ptr));
		assert(0);
	}
#endif
	struct hdrfree *newnode = (struct hdrfree*)((char*)ptr - 8);
	newnode->mark = c_free;
	//size stays
	newnode->next = NULL;
	newnode->prev = NULL;
	

	// experimental - try to merge, if adjanced from bottom is also freeblock
	int reorganized = 0;
	struct hdrfree *higher;
	{
		struct hdrfree *p1;
		higher = NULL;
		for (p1 = __firstfree; p1; p1 = p1->next)
		{
			higher = (struct hdrfree *)((char*)p1 + p1->size);
			if (higher == newnode) break;
		}
		if (p1) // yes, it is
		{
			wtalloc_stat.freeblocks_sum += newnode->size;
			p1->size += newnode->size;
			// p1->prev, p1->next  already OK
			newnode->mark = 0; // for safety
			newnode = p1;  // continue optimization
//TRACE2("free block merged w/bottom sized %u bytes, list len %u\n", p1->size, wtalloc_stat.crtfreeblocks);
			reorganized = 1;
		}
	}


/* removed, as very seldom succeeds */
	// experimental - try to merge, if adjanced from top is also freeblock
	higher = (struct hdrfree *)((char*)newnode + newnode->size);
// dont work - we try to read after our memory
//	if ((char*)higher < (char*)__freetop &&   // saves from reading out of our memory
//		higher->mark == c_free) // only suspisious, must be in list
	{
		struct hdrfree *p1;
		for (p1 = __firstfree; p1 && p1 != higher; p1 = p1->next);
		if (p1) // yes, it is
		{
			if (newnode->next || newnode->prev)  // optimized 1st stage, must remove from list and readd later
			{
				wtalloc_stat.crtfreeblocks--;
				wtalloc_stat.freeblocks_sum -= newnode->size;
				if (newnode->next)
					newnode->next->prev = newnode->prev;
				if (newnode->prev)
					newnode->prev->next = newnode->next;
				else
					__firstfree = newnode->next;
			}
			wtalloc_stat.freeblocks_sum += newnode->size;
			newnode->size += higher->size;
			newnode->prev = higher->prev;
			newnode->next = higher->next;
			higher->mark = 0; // for safety
			if (higher->next)
				higher->next->prev = newnode;
			if (higher->prev)
				higher->prev->next = newnode;
			else
				__firstfree = newnode;
//TRACE1("free block merged w/top\n", 0);
			reorganized = 1;
		}
	}

	if (reorganized) return;  // experimental reorganized do all work
	
//TRACE1("free block added\n", 0);
	wtalloc_stat.crtfreeblocks++;
	wtalloc_stat.freeblocks_sum += newnode->size;

	newnode->next = __firstfree;
	newnode->prev = NULL;
	if (__firstfree)
		__firstfree->prev = newnode;
	__firstfree = newnode;
}	


void *wtrealloc(void *ptr, size_t sz)
{
	if (!ptr) return wtmalloc(sz);
	
	struct hdrused* oldptr = (struct hdrused*)((char*)ptr - 8);

#ifndef NDEBUG
	if (oldptr->mark != c_used)
	{
		TRACE2("try realloc unallocated block ptr = %x  EIP@%x\n", ptr, CALLEDFROM(ptr));
		assert(0);
	}
#endif

	if (oldptr->size - 8 >= sz) return ptr; // enough room in this block, ex from freelist
	
	/* experimental	growth last block */
	int growth = (oldptr->size + sz + 15) & ~15;
	if ((char*)oldptr + oldptr->size == __freebase && 
		__freetop - __freebase + oldptr->size >= growth )  // we at top, can grow up
	{
		wtalloc_stat.malloc_sum += growth - oldptr->size;
		__freebase += growth - oldptr->size;
		oldptr->size = growth;
		return ptr;
	}
	
	void *newptr = wtmalloc(sz);
	if (newptr)
	{
		memcpy(newptr, (char*)oldptr +8, oldptr->size -8); // why forgeting -8 dont fail test?!?
		wtfree((char*)oldptr +8);
		return newptr;
	}
	
	return NULL;
}	

void* wtcalloc( size_t num, size_t size )
{
	void *newptr = wtmalloc(num * size);
	if (newptr)
		memset(newptr, 0, num * size);
	return newptr;
}




int wtmalloc_freelist_check()
//контроль целостности списка фри OK == 1
{
	int cnt = 0;
	struct hdrfree *ptr = __firstfree;
	
	if(ptr && ptr->prev)
	{
		TRACE1("allocated memory freelist 1st block fail, ptr = %x\n", ptr);
		return 0;
	}
	
	for(;ptr; ptr = ptr->next)
	{
//TRACE1("(%x)", ptr);

		cnt++;
		if (ptr->mark != c_free)
		{
			TRACE1("allocated memory freelist check fail, ptr = %x\n", ptr);
			return 0;
		}
	}
	if (cnt != wtalloc_stat.crtfreeblocks)
	{
		TRACE2("allocated memory freelist check fail, length must be = %u but is %u\n", wtalloc_stat.crtfreeblocks, cnt);
		return 0;
	}
	return 1;	
}

void wtmalloc_freelist_print()
{
	struct hdrfree *ptr = __firstfree;
	for(;ptr; ptr = ptr->next)
	{
		TRACE2("(%x[%u])", ptr, ptr->size);
	}
	TRACE1("\n", 0);
	
}

int wtmalloc_poiner_check(void *ptr)
//контроль указателя - mark  OK == 1
{
	if (((struct hdrused*)((char*)ptr - 8))->mark != c_used)
	{
		TRACE2("pointer watermark check fail ptr = %x bytes EIP@%x\n", ptr, CALLEDFROM(ptr));
		return 0;
	}
	return 1;
}

void wtdump_alloc_stats()
{
		TRACE1("----Watermark allocator stats:----\n", 0);
		TRACE2("allocated %u calls, max of %u bytes\n", wtalloc_stat.malloc_calls, wtalloc_stat.malloc_max);
		TRACE2("total %u bytes, average call %u bytes\n", wtalloc_stat.malloc_sum, wtalloc_stat.malloc_sum / wtalloc_stat.malloc_calls);
		TRACE1("SYSTEM:\n", 0);
		TRACE2("allocated %u calls, max of %u bytes\n", wtalloc_stat.sysalloc_calls, wtalloc_stat.sysalloc_max);
		TRACE2("total %u bytes, average call %u bytes\n", wtalloc_stat.sysalloc_sum, wtalloc_stat.sysalloc_sum / wtalloc_stat.sysalloc_calls);
		TRACE2("free list %u bytes, length %u chunks\n", wtalloc_stat.freeblocks_sum, wtalloc_stat.crtfreeblocks);
}
