
LIBRARY= libsupc++

CC=gcc
CPP=g++
CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fomit-frame-pointer

LD = ld

AR= ar

STRIP = $(PREFIX)strip

INCLUDES= -I. -I../newlib/include


LIBS:=  -ldll -lc.dll


DEFINES= -DIN_GCC -DUSE_EMUTLS=1


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
	ar cvrs $(LIBRARY).a $(OBJECTS)		
	mv -f $(LIBRARY).a ../../lib


%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.cc Makefile
	$(CPP) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<


clean: 
	-rm -f *.o 




