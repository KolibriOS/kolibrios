KTCC_DIR = ../../develop/ktcc/trunk
KLIBC = ../../develop/ktcc/trunk/libc.obj

NAME = weather

KTCC=$(KTCC_DIR)/bin/kos32-tcc
KPACK = kpack

SRC= weather.c json/json.c
FLAGS= -B$(KTCC_DIR)/bin -I $(KLIBC)/include 
LIBS = -limg -lhttp 

all:
	$(KTCC) $(FLAGS) $(SRC) $(LIBS) -o $(NAME)
	$(KPACK) $(NAME)
clean:
	rm $(NAME)
