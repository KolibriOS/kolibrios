#ifndef __KOLIBRI_LIB_H_INCLUDED_
#define __KOLIBRI_LIB_H_INCLUDED_

// Kolibri interface.

namespace Kolibri   // All kolibri functions, types and data are nested in the (Kolibri) namespace.
{
	unsigned int StrLen(const char *str);
	char *StrCopy(char *dest, const char *src);
	void *MemCopy(void *dest, const void *src, unsigned int n);
	void *MemSet(void *s, char c, unsigned int n);

	double Floor(double x);
}

#endif  // __KOLIBRI_LIB_H_INCLUDED_
