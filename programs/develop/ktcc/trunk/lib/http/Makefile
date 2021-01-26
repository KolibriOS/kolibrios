FASM = fasm

OBJS = get.o head.o __lib__.o post.o receive.o send.o

%.o : %.asm
	$(FASM) $<

all: $(OBJS)
	ar -rsc libhttp.a *.o
	mv -f libhttp.a ../../bin/lib
	rm -f *.o
