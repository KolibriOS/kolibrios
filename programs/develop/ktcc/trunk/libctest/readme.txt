most test adapted from "musl-libctest-master" project
some taken from newlib

Status or libc tests

---NOT TESTED---
no library fns realized
qsort
strtol
time

---HANG---
sscanf
>TEST_F(0x1234p56)


---STACK IS SMALL---
strtod_long 
tstring	


--other--
fscanf 
-?scanf ignores width specs, '*' and [chars], cant read %a float
-%n counts as parameter

snprintf
-some format misturbances

ungetc
-ungetc fails if filepos == 0 - no tricks

all file ops limited to 2Gb

