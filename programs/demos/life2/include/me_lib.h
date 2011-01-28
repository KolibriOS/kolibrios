#ifndef __MENUET_LIB_H_INCLUDED_
#define __MENUET_LIB_H_INCLUDED_

// Menuet interface.

namespace Menuet   // All menuet functions, types and data are nested in the (Menuet) namespace.
{
	unsigned int StrLen(const char *str);
	char *StrCopy(char *dest, const char *src);
	void *MemCopy(void *dest, const void *src, unsigned int n);
	void *MemSet(void *s, char c, unsigned int n);

	double Floor(double x);
}

#endif  // __MENUET_LIB_H_INCLUDED_
