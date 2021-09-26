CC = kos32-tcc
AR = ar
CFLAGS = -c -I../../libc.obj/include

LIB = libshell.a

OBJS = \
	shell_exit.o \
	shell_puts.o \
	shell_get_pid.o \
	shell_ping.o \
	shell_getc.o \
	shell_cls.o \
	shell_init.o \
	shell_gets.o \
	shell_printf.o \
	shell_putc.o

$(LIB): $(OBJS)
	$(AR) -crs $@ $(OBJS)
	
%.o : %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(OBJS) $(LIB)