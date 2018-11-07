see 
source/readme.*
source/changelog
source/tcc-doc.info or  .texi

building Kolibri version 
>make -f Makefile.kos32 

========= for compiler developers =========
read .\source\readme_kos32.txt

------ TODO -------
-minimal memory allocator
-more libc stardard functions. see report below 
-more Kolibly SysFn wrappers. see \libc\KOSfuncs_inc_status.txt
-add stdin, stderr, stdout emulation не хватает stdin, stdout - можно сделать как stderr!, но надо возиться заодно с ferror & feof
-getchar, gets if returs errorcode  (0, null) - you must exit program, because of closed console window  
-при нормальном выходе закрывать консоль


------ errors ------
-not working: default search path are ./include ./lib from executable (under KOS need to use -Bpath_to_ktcc)
--start.o not found using -B (kos) - put near your.c file
-если проект многофайловый - .dbg генерит дублирующиеся метки данных, типа L.78 может указывать на другой сегмент (
-.dbg sometimes generated improperly for source code labels

----- fixed errors ------
-if static var sized more than 14096+ -> crash compiled .exe (kos) 
(^ default stack size set at compile time tccmeos:177 is below 4k)
FIX - use -stack=1280000 option
-con_set_title is NULL. fixed 180128 



========= libc ===========
-no "finished" in title of console program after exit console - use con_exit()
-used system memory allocator (4096 bytes minimum)


libc not complete. overall status:
no files:
limits.h
locale.h
setjmp.h
signal.h
wchar.h
wctype.h



functions absent list:

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

string.h:
strxfrm

time.h:  - needs include kos32sys1.h
asctime
ctime
gmtime
localtime - non standard
strftime






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
-ungetc fails if filepos == 0 - by design

all file ops limited to 2Gb


