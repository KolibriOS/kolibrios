
CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wno-format -Werror
CFLAGS   = -c $(CFLAGS_OPT)

INCLUDES=  -I. -I../bfd -I../include -I$(SDK_DIR)/sources/newlib/libc/include

DEFINES=  -DHAVE_CONFIG_H

SRCS = \
	dis-buf.c disassemble.c	\
	dis-init.o i386-dis.o	\
	i386-opc.o 

OBJS  =  $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SRCS)))

# targets

all: libopcodes.a

libopcodes.a: $(OBJS) MAkefile
	$(AR) crs $@ $(OBJS)
	mv -f $@ $(SDK_DIR)/lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

 