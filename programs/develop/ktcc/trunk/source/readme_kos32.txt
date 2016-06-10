Siemargl port comments 

Used github branch https://github.com/TinyCC/tinycc
It have a vesion 0.9.26 with heads up to 0.9.27 - see ChangeLog

Kolibri version errata/changelog:

-added TCC_TARGET_MEOS as needed
-leading_underscore by default is 0 (can use -f[no-]leading-underscore), 
otherwise (error) underscoring all symbols, not only cdecl
-added message in tccmeos.c about missed symbols when linking KOS executable
-start.o added automatically, when -nostdlib not used
-to use standard ktcc lib must add -lck at commandline
-default search paths are ./include ./lib from executable (under KOS need to 
 use -Bpath_to_ktcc and put start.o in current dir)
-when config.h is ready, compiler can be easy builded as [kos32-]gcc tcc.c libtcc.c
 see also makefile.kos32
-silent (kos) -> writes to debugboard
-impossible using with mingw-gcc compiled lib, incompatible library format:
 .o is PE-format from gcc but ELF from tcc, may be linux-gcc does it ok
-__fastcall incompatible with other compilers. now stack freed by caller. 
 must fix i386-gen.c@490,572 (fixed in other branch https://github.com/mirror/tinycc)


-using __attribute__((packed)) see test82. need naming struct twice as in kos32sys1.h
-using __attribute__ ((alias xxx)) restricted only for non "static inline" functions 
-erroneous or "non TCC" member using in nested structs or unions can lead to compiler internal error
-not working: default search path are ./include ./lib from executable 
--under KOS need to use -Bpath_to_ktcc
--start.o not found using -B (kos) - put near your.c file
-bench timing coarse (0s or 1s), no usec in newlib gettimeofday. OK

Tests status:
asmtest +
abitest not tested (embedding compiler)
libtcctest not tested (embedding compiler)
boundtest ----- alloca removed from tcc  libtcc.c:945 (really not worked)
tcctest most test ok, some problems with long double
vla_test.c +

pp/* +  (minor comment error in 13.s)

tests2/* : see below

// errata
skippin' tests
test76 fail dollars in identifiers
test34 fail (array assignment not supported)
test73 fail compile (no stdint.h), printfloat, ARM specific
test46 no stdin - removed funtionality read from console, but file ops works


libc:
-no "finished" in title of console program after exit console - use con_exit()
-used system memory allocator (4096 bytes minimum)


libc not complete. overall status:
no files:
errno.h  - in stdio
limits.h
locale.h
setjmp.h
signall.h
time.h  - can use get_tick_count()/100 from kos32sys1.h
wchar.h
wctype.h



functions absent list:

math.h
frexp
ldexp
modf
fmod

HUGE_VAL

stdio.h:
remove
rename
tmpfile
tmpnam
freopen
setbuf
setvbuf


stdlib.h:
atexit
getenv
system
bsearch
qsort
mblen
mbtowc
wctomb
mbstowcs
wcstombs

string.h
strxfrm


            Status or libc tests

---FAILED---
strtoul incorrect work with big unsigned > MAX_LONG


---NOT TESTED---
no library fns realized
qsort
time

---HANG---
sscanf
>TEST_F(0x1234p56) - no %a formats


---STACK IS SMALL---
use new -stack=1280000 option to pass test
tstring	
strtodlong


--other--
fscanf 
-?scanf ignores width specs, '*' and [chars], cant read %a float
-%n counts as parameter

snprintf
-some format misturbances
-may incorrect prints unsigned > 2147483647L

ungetc
-ungetc fails if filepos == 0 - no tricks

all file ops limited to 2Gb
