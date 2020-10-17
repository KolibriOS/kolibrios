Adaption of C_Layer wrappers for TCC

_____________
TODO
_____________

- Add wrappers in boxlib.h for:
 - statictext
 - filebrowse
 - editor
 - treelist

- Port other wrappers from C_Layer


_____________
How to use?
_____________

1. Include header to your program:

#include <clayer/boxlib.h>

2. Write some program
3. Compile with link to program .o file:

kos32-tcc sample.c loadboxlib.o -lck -o sample.kex

(.o files in ktcc/trunk/bin/lib/clayer)