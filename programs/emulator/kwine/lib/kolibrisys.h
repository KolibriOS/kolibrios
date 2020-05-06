#ifndef __KOS_32_SYS_H__
#define __KOS_32_SYS_H__

#include "stddef.h"

static inline uint32_t _ksys_get_date(void)
{
    uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(29));
    return val;
}

static inline uint32_t _ksys_get_system_clock(void)
{
	uint32_t val;
    asm volatile ("int $0x40":"=a"(val):"a"(3));
    return val;
}

#endif