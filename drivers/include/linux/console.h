#ifndef _LINUX_CONSOLE_H
#define _LINUX_CONSOLE_H
/* KolibriOS has no VT console layer; the fbdev handover hooks are no-ops. */
#define CON_PRINTBUFFER	(1)
#define CON_CONSDEV	(2)
#define CON_ENABLED	(4)
#define CON_BOOT	(8)
#define CON_ANYTIME	(16)
#define CON_BRL		(32)

static inline void console_lock(void)   { }
static inline void console_unlock(void) { }
static inline int  console_trylock(void) { return 1; }
#endif
