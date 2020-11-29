INCLUDE = include
LIBSFORBUILD = math
LIBNAME = libck.a
CC = ../bin/kos32-tcc
CFLAGS = -I$(INCLUDE) -m32 -nostdinc -nostdlib -DGNUC
DIRS := stdio memory kolibrisys string stdlib math dlfcn libgen fs

##############################################################
#files := $(foreach dir,$(DIRS),$(dir)/$(wildcard $(dir)/*))
asmfiles := $(foreach dir,$(DIRS),$(patsubst %.asm, %.o, $(wildcard $(dir)/*.asm)))
cfiles := $(foreach dir,$(DIRS),$(patsubst %.c, %.o, $(wildcard $(dir)/*.c)))

.PHONY: clean all

ifdef windir
doClean = del /F /Q $(subst /,\,$(cfiles)) $(subst /,\,$(asmfiles))
else
doClean = rm $(cfiles) $(asmfiles)
endif

all: $(cfiles) $(asmfiles)
	ar -ru $(LIBNAME) $^

$(cfiles): $(INCLUDE)/*.h

$(asmfiles):
	fasm $*.asm $*.o

clean:
	$(doClean)

install:
	cp $(LIBNAME) ../bin/lib
