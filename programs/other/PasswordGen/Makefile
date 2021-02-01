KTCC_DIR=D:/ktcc

NAME=passwordgen

KTCC=$(KTCC_DIR)/kos32-tcc.exe
KPACK=D:/ktcc/kpack.exe

SRC=passwordgen.c
CFLAGS=#-I $(KTCC_DIR)/libc/include -L $(KTCC_DIR)/bin/lib -D_KOS32
LIBS= -lck -lbox

all:
	$(KTCC) $(CFLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)

clean:
	del $(NAME)