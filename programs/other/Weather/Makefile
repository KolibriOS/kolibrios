KTCC_DIR = ../../develop/ktcc/trunk
KLIBC = ../../develop/libraries/kolibri-libc

NAME = weather

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK = kpack

SRC= weather.c json/json.c
FLAGS= -nostdlib $(KLIBC)/lib/crt0.o -I $(KLIBC)/include -I $(KTCC_DIR)/libc/include -L$(KLIBC)/lib
LIBS =-ltcc -limg -lhttp -lc.obj

all:
	$(KTCC) $(FLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)
clean:
	rm $(NAME)
