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
-no symbols (mapfile) for debug, see howtodebugtcc


-how to use packed attribute see test82
-alias attribute wont work
-unnamed structs in union may lead to compiler internal error
-tcc: error: undefined symbol '__tcc_cvt_ftol' 
--in config.h - used workaround (#define COMMIT_4ad186c5ef61_IS_FIXED
--but this is precision bugfix - see \tests\tests2\000_cvttoftol.c 
-not working: default search path are ./include ./lib from executable 
--under KOS need to use -Bpath_to_ktcc
--start.o not found using -B (kos) - put near your.c file
-if static var sized more than 14096+ -> crash compiled .exe (kos) 
---^ stack size set in menuet header at compile time tccmeos.c:177 about 4k

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
-bench timing error (0s or 1s)
-minimal memory allocator
-memmove cannot overlap


libc not full
no files:
assert.h
errno.h
limits.h
locale.h
setjmp.h
signall.h
time.h
check functions:

stdio.h:
Operations on files: none http://www.cplusplus.com/reference/cstdio/
reopen
setbuf, setvbuf
scanf, sscanf, vfscanf(C11), vscanf(C11), vsscanf(C11)
vfprintf, vsfprintf

+fgets, gets
fputs, puts
getchar
putc
+putchar
Error-handling: only feof
Macros: only EOF, NULL, FILE

-all files in libc/kolibrisys catalog are stdcall in header, but in asm cdecl
