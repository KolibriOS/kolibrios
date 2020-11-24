KTCC_DIR=../../develop/ktcc/trunk

NAME=teatool

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=teatool.c
CFLAGS=-nostdinc -I $(KTCC_DIR)/libc/include
LFLAGS=-nostdlib -L $(KTCC_DIR)/bin/lib $(KTCC_DIR)/bin/lib/start.o
LIBS = -lck

all:
	$(KTCC) $(CFLAGS) $(LFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
