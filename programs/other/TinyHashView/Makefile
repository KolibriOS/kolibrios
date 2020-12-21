KTCC_DIR=../../develop/ktcc/trunk

NAME=thashview

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=thashview.c
CFLAGS=-I $(KTCC_DIR)/libc/include
LIBS = -lck -lcryptal -ldialog -lbox

all:
	$(KTCC) $(CFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
