/**************************************************************************
 *
 * Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


/**
 * \file exemem.c
 * Functions for allocating executable memory.
 *
 * \author Keith Whitwell
 */


#include "pipe/p_compiler.h"
#include "util/u_debug.h"
#include "os/os_thread.h"
#include "util/u_memory.h"

#include "rtasm_execmem.h"

#include "util/u_mm.h"

#define EXEC_HEAP_SIZE (4*1024*1024)

pipe_static_mutex(exec_mutex);

static struct mem_block *exec_heap = NULL;
static unsigned char *exec_mem = NULL;


static void
init_heap(void)
{
   if (!exec_heap)
      exec_heap = u_mmInit( 0, EXEC_HEAP_SIZE );
   
   if (!exec_mem)
      exec_mem = (unsigned char *) user_alloc(EXEC_HEAP_SIZE);
}


void *
rtasm_exec_malloc(size_t size)
{
   struct mem_block *block = NULL;
   void *addr = NULL;

   pipe_mutex_lock(exec_mutex);

   init_heap();

   if (exec_heap) {
      size = (size + 31) & ~31;  /* next multiple of 32 bytes */
      block = u_mmAllocMem( exec_heap, size, 5, 0 ); /* 5 -> 32-byte alignment */
   }

   if (block)
      addr = exec_mem + block->ofs;
   else 
      debug_printf("rtasm_exec_malloc failed\n");
   
   pipe_mutex_unlock(exec_mutex);
   
   return addr;
}

 
void 
rtasm_exec_free(void *addr)
{
   pipe_mutex_lock(exec_mutex);

   if (exec_heap) {
      struct mem_block *block = u_mmFindBlock(exec_heap, (unsigned char *)addr - exec_mem);
   
      if (block)
	 u_mmFreeMem(block);
   }

   pipe_mutex_unlock(exec_mutex);
}

