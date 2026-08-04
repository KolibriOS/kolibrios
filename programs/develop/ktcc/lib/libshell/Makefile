CC = kos32-tcc
AR = ar
CFLAGS = -c -I$(LIBC_INCLUDE)

LIBC_INCLUDE = ../../libc.obj/include

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
	shell_putc.o \
	shell_write_string.o

$(LIB): $(OBJS)
	$(AR) -crs $@ $(OBJS)
	
%.o : %.c Makefile
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(OBJS) $(LIB)


shell_cls.o: shell_cls.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/sys/ksys.h
shell_get_pid.o: shell_get_pid.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/string.h
shell_getc.o: shell_getc.c $(LIBC_INCLUDE)/shell_api.h
shell_gets.o: shell_gets.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/string.h
shell_init.o: shell_init.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/sys/ksys.h $(LIBC_INCLUDE)/string.h $(LIBC_INCLUDE)/stdio.h $(LIBC_INCLUDE)/stdlib.h
shell_ping.o: shell_ping.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/sys/ksys.h
shell_printf.o: shell_printf.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/stdio.h
shell_putc.o: shell_putc.c $(LIBC_INCLUDE)/shell_api.h
shell_puts.o: shell_puts.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/string.h
shell_write_string.o: shell_write_string.c $(LIBC_INCLUDE)/shell_api.h $(LIBC_INCLUDE)/string.h
