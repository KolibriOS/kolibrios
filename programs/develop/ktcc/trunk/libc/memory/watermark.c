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
#	define TRACE1(s, a) printf(s, a)
#	define TRACE2(s, a, b) printf(s, a, b)
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
static unsigned __crtfreeblocks = 0; // number of free blocks, checking corruptions


void *wtmalloc(size_t sz)
{
	struct hdrfree *fndnode, *newnode;
	sz = (sizeof(struct hdrused) + sz + 15) & ~15; // align 16bytes
//TRACE1("_call alloc(%d)\n", sz);
	
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
			newnode = (struct hdrfree*)((char*)fndnode + sz);
			newnode->mark = c_free;
			newnode->size = fndnode->size - sz;
			newnode->next = fndnode->next;
			newnode->prev = fndnode->prev;
			
			if (fndnode->next)
				fndnode->next->prev = newnode;
			
			//перед может быть не нода, а 1й указатель
			if (!fndnode->prev)
				__firstfree = newnode;
			else
				newnode->prev->next = newnode;
		} else // nothing to split, just exclude
		{
//TRACE1("alloc(%d) remove freenode\n", sz);

			__crtfreeblocks--;
			if (fndnode->next)
				fndnode->next->prev = fndnode->prev;
			//перед может быть не нода, а 1й указатель
			if (!fndnode->prev)
				__firstfree = fndnode->next;
			else
				fndnode->prev->next = fndnode->next;
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
		size_t new_heap_size = (__freetop - __freebase + sz + 4095) & ~4095;   
		
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
			__crtfreeblocks++;
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
	
	return ptr;
}

void wtfree(void *ptr)
{
	if (!ptr) return;

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
	newnode->next = __firstfree;
	newnode->prev = NULL;
	if (__firstfree)
		__firstfree->prev = newnode;
	__firstfree = newnode;
	__crtfreeblocks++;
//TRACE1("free to freenode\n", 0);
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
	
	void *newptr = wtmalloc(sz);
	if (newptr)
	{
		memcpy(newptr, (char*)oldptr +8, oldptr->size -8); // why -8 dont fail test?!?
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
	if (cnt != __crtfreeblocks)
	{
		TRACE1("allocated memory freelist check fail, length must be = %u\n", __crtfreeblocks);
		return 0;
	}
	return 1;	
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
