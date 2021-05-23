KTCC_DIR=../ktcc/trunk
KLIBC_DIR = ../libraries/kolibri-libc

NAME=bin/tinybas.kex

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK=kpack

SRC=    src/common.c \
	src/errors.c \
	src/expression.c \
	src/formatter.c \
	src/generatec.c \
	src/interpret.c \
	src/options.c \
	src/parser.c \
	src/statement.c \
	src/tinybasic.c \
	src/token.c \
	src/tokeniser.c

CFLAGS= -D_KOLIBRI -I$(KLIBC_DIR)/include -I inc
LFLAGS= -nobss -nostdlib -L $(KLIBC_DIR)/lib $(KLIBC_DIR)/lib/crt0.o
LIBS = -ltcc -lc.obj

all:
	$(KTCC) $(CFLAGS) $(SRC) $(LFLAGS) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	rm $(NAME)
