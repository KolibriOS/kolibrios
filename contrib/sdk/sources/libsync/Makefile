LIBRARY= libsync

FASM = fasm
AR = kos32-ar
ARFLAGS = crs
INSTALLDIR:= /home/autobuild/tools/win32/lib

SRCS = 	mutex.asm	\
	m_destroy.asm	\
	m_lock.asm	\
	m_try.asm	\
	m_unlock.asm	\
	$(NULL)
	
OBJS =  $(patsubst %.asm, %.o, $(SRCS))
 
# targets 

all: libsync.a

libsync.a: $(OBJS) Makefile
	$(AR) $(ARFLAGS) libsync.a $(OBJS)
	mv -f libsync.a $(INSTALLDIR)

%.o : %.asm Makefile
	$(FASM) $< $@

clean: 
	-rm -f *.o 




