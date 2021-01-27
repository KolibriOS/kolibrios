KTCC_DIR = ../../develop/ktcc/trunk

NAME = weather

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK = kpack

SRC= weather.c
CFLAGS= -I $(KTCC_DIR)/libc/include
LIBS =  -ljson -lck -limg -lhttp

all:
	$(KTCC) $(CFLAGS) -L json $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)
clean:
	rm $(NAME)
