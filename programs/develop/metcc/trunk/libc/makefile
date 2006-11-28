INCLUDE = include
LIBNAME = melibc.a
CC = gcc
CFLAGS = -I$(INCLUDE) -nostdinc -DGNUC
DIRS := file mesys string mem 

##############################################################
#files := $(foreach dir,$(DIRS),$(dir)/$(wildcard $(dir)/*))
asmfiles := $(foreach dir,$(DIRS),$(patsubst %.asm, %.o, $(wildcard $(dir)/*.asm)))
cfiles := $(foreach dir,$(DIRS),$(patsubst %.c, %.o, $(wildcard $(dir)/*.c)))

.PHONY: clean all

all: $(cfiles) $(asmfiles)
	ar -ru $(LIBNAME) $^

$(cfiles): $(INCLUDE)/*.h

$(asmfiles):
	fasm $*.asm $*.o

clean:
	del /F /Q $(subst /,\,$(cfiles)) $(subst /,\,$(asmfiles))
