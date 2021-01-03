KTCC_DIR=../../develop/ktcc/trunk

NAME=gamehack

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=gh_shell.c
CFLAGS=-I $(KTCC_DIR)/libc/include
LIBS = -lck

all:
	$(KTCC) $(CFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)
clean:
	rm $(NAME)
