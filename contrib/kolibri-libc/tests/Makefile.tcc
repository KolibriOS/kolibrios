KTCC= ../bin/kos32-tcc
KPACK= kpack

SRC=$(NAME).c
CFLAGS = -I ../source/include -stack=1048576  # set stack size 1Mb

.PHONY: all

all: $(NAME)

$(NAME): $(SRC)
	$(KTCC) $(CFLAGS) $(SRC) -o $(NAME) -lc
	$(KPACK) $(NAME)

clean:
	rm $(NAME).o $(NAME)
