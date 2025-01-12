KTCC_DIR = ../kolibrios-nextgen/programs/develop/ktcc/trunk
KLIBC = $(KTCC_DIR)/libc.obj

NAME = dino

KTCC = $(KTCC_DIR)/bin/kos32-tcc
KPACK = kpack

SRC = $(wildcard *.c)
FLAGS= -B$(KTCC_DIR)/bin -I $(KLIBC)/include -Wall# -stack=20480
LIBS = -limg

all: $(SRC)
	$(KTCC) $(FLAGS) $(SRC) $(LIBS) -o $(NAME)
# $(KPACK) $(NAME)

clean:
	rm $(NAME)
