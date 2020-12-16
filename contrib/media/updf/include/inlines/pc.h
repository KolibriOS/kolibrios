/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_Inline_pc_h_
#define __dj_include_Inline_pc_h_

#ifdef __GNUC__

#ifdef __cplusplus
extern "C" {
#endif

extern __inline__ unsigned char inportb (unsigned short _port)
{
 unsigned char __ret;
 __asm__ __volatile__("inb %%dx,%%al":"=a"(__ret):"d"(_port));
 return __ret;
}

extern __inline__ unsigned short inportw (unsigned short _port)
{
 unsigned short __ret;
 __asm__ __volatile__("inw %%dx,%%ax":"=a"(__ret):"d"(_port));
 return __ret;
}

extern __inline__ unsigned long inportl (unsigned short _port)
{
 unsigned long __ret;
 __asm__ __volatile__("inl %%dx,%%eax":"=a"(__ret):"d"(_port));
 return __ret;
}

extern __inline__ void outportb (unsigned short _port, unsigned char _data)
{
 __asm__ __volatile__("outb %%al,%%dx"::"a"(_data),"d"(_port));
}

extern __inline__ void outportw (unsigned short _port, unsigned short _data)
{
 __asm__ __volatile__("outw %%ax,%%dx"::"a"(_data),"d"(_port));
}

extern __inline__ void outportl (unsigned short _port, unsigned long _data)
{
 __asm__ __volatile__("outl %%eax,%%dx"::"a"(_data),"d"(_port));
}

#ifdef __cplusplus
}
#endif

#endif

#endif
