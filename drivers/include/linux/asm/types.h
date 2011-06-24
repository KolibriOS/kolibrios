#ifndef _ASM_X86_TYPES_H
#define _ASM_X86_TYPES_H

#define dma_addr_t	dma_addr_t

#include <asm-generic/types.h>

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

typedef u64 dma64_addr_t;

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL__ */

#endif /* _ASM_X86_TYPES_H */
