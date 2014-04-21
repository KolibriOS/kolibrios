LIBRARY= libsupc++

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

ARFLAGS= crs

INCLUDES= -I. -I../newlib/include

LIBS:=  -ldll -lc.dll

DEFINES= -DIN_GCC -DUSE_EMUTLS=1 -D_GLIBCXX_HAVE_TLS


SOURCES = 	gthr_mutex.c			\
		bad_alloc.cc			\
		class_type_info.cc		\
		del_op.cc			\
		del_opv.cc			\
		eh_alloc.cc			\
		eh_aux_runtime.cc		\
		eh_call.cc			\
		eh_catch.cc			\
		eh_exception.cc			\
		eh_globals.cc			\
		eh_personality.cc		\
		eh_term_handler.cc		\
		eh_terminate.cc			\
		eh_throw.cc			\
		eh_type.c			\
		eh_unex_handler.cc		\
		guard.cc			\
		guard_error.cc			\
		new_handler.cc			\
		new_op.cc			\
		new_opnt.cc			\
		new_opv.cc			\
		pbase_type_info.cc		\
		pure.cc				\
		si_class_type_info.cc		\
		tinfo.cc			\
		vmi_class_type_info.cc		\
		vterminate.cc
 

OBJECTS =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(SOURCES)))

# targets 

all:$(LIBRARY).a

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJECTS)		
	mv -f $(LIBRARY).a ../../lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.cc Makefile
	$(CPP) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<


clean: 
	-rm -f *.o 

