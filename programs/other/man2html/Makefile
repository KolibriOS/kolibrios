CC = gcc
NAME = man2html

CFLAGS = -DNOCGI

SRC = man2html.c cgibase.c abbrev.c strdefs.c

$(NAME): $(SRC)
	$(CC) $(SRC) -o $(NAME)

clean:
	rm -f $(NAME)

