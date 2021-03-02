AR = ar -rcs
CLINK = ../linuxtools/clink
KPACK = kpack

INCLUDE = include
LIB_DIR = ../lib

LIBC.A = $(LIB_DIR)/libc.a
LIBC.OBJ = $(LIB_DIR)/libc.obj

CFLAGS = -I$(INCLUDE) -m32 -nostdinc -nostdlib -DGNUC -Os -fno-common -fno-builtin -fno-leading-underscore -fno-pie
DIRS :=  string stdlib stdio sys math ctype exports setjmp

cfiles := $(foreach dir,$(DIRS),$(patsubst %.c, %.o, $(wildcard $(dir)/*.c)))
asmfiles := $(foreach dir,$(DIRS),$(patsubst %.s, %.o, $(wildcard $(dir)/*.s)))
.PHONY: clean all

ifdef windir
cfiles := $(subst /,\,$(cfiles))
asmfiles := $(subst /,\,$(asmfiles))
LIB_DIR := $(subst /,\,$(LIB_DIR))
LIBNAME := $(subst /,\,$(LIBNAME))
RM = del /F /Q
MKDIR_P = md
else
RM = rm -rf
MKDIR_P = mkdir -p
endif

all: $(cfiles) $(asmfiles) $(LIB_DIR) $(LIBC.A)
	fasm crt/crt0.asm $(LIB_DIR)/crt0.o

$(LIBC.A): $(cfiles) $(asmfiles)
	$(AR) $(LIBC.A) $^

$(LIB_DIR):
	$(MKDIR_P) $(LIB_DIR)

$(asmfiles):
	$(CC) -E -x c -nostdinc -Imath $*.s > $*.sx
	$(AS) --32 $*.sx -o $*.o
	$(RM) $*.sx

clean:
	$(RM) $(cfiles) $(asmfiles)
	$(RM) $(LIBNAME)

shared: $(cfiles) $(asmfiles)
	$(CLINK) $^
	$(KPACK) a.out.obj
	mv a.out.obj $(LIBC.OBJ)

test:
	make -C ../test -f Makefile.tcc
	kex ../test/test
