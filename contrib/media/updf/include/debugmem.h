

#ifndef __debugmem_h__
#define __debugmem_h__


#ifdef YACAS_DEBUG

#ifdef NO_GLOBALS
#error "Memory heap checking only possible with global variables!"
#endif

  void* YacasMallocPrivate(unsigned long size, const char* aFile, int aLine);
  void* YacasReAllocPrivate(void* orig, unsigned long size, const char* aFile, int aLine);
  void YacasFreePrivate(void* aOrig);
  void CheckPtr(void * anAllocatedPtr, const char* file, int line);
  void CheckAllPtrs(int final = 0);
#endif

#endif



