LIBRARY= libsupc++

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/lib

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer
CPPFLAGS = -fno-rtti

ARFLAGS= crs

INCLUDES= -I. -I../include/mingw32 -I../include -I../../newlib/libc/include -I/home/autobuild/tools/win32/include

LIBS:=  -ldll -lc.dll

DEFINES= -DHAVE_CONFIG_H -DIN_GCC -DIN_GLIBCPP_V3 -DUSE_EMUTLS=1 -D_GLIBCXX_HAVE_TLS


SOURCES = 					\
		cp-demangle.c			\
		array_type_info.cc		\
		atexit_thread.cc		\
		bad_alloc.cc			\
		bad_cast.cc			\
		bad_typeid.cc			\
		class_type_info.cc		\
		del_op.cc			\
		del_opnt.cc			\
		del_opv.cc			\
		del_opvnt.cc			\
		dyncast.cc			\
		eh_alloc.cc			\
		eh_aux_runtime.cc		\
		eh_call.cc			\
		eh_catch.cc			\
		eh_exception.cc			\
		eh_globals.cc			\
		eh_personality.cc		\
		eh_ptr.cc			\
		eh_term_handler.cc		\
		eh_terminate.cc			\
		eh_throw.cc			\
		eh_tm.cc			\
		eh_type.c			\
		eh_unex_handler.cc		\
		enum_type_info.cc		\
		function_type_info.cc		\
		fundamental_type_info.cc	\
		guard.cc			\
		guard_error.cc			\
		hash_bytes.cc			\
		nested_exception.cc		\
		new_handler.cc			\
		new_op.cc			\
		new_opnt.cc			\
		new_opv.cc			\
		new_opvnt.cc			\
		pbase_type_info.cc		\
		pmem_type_info.cc		\
		pointer_type_info.cc		\
		pure.cc				\
		si_class_type_info.cc		\
		tinfo.cc			\
		tinfo2.cc			\
		vec.cc				\
		vmi_class_type_info.cc		\
		vterminate.cc
 

OBJECTS =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(SOURCES)))

# targets 

all:$(LIBRARY).a

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJECTS)		
	mv -f $(LIBRARY).a $(INSTALLDIR)

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.cc Makefile
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

eh_ptr.o: eh_ptr.cc
	$(CPP) $(CFLAGS) -std=gnu++0x $(CPPFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

eh_throw.o: eh_throw.cc
	$(CPP) $(CFLAGS) -std=gnu++0x $(CPPFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

guard.o: guard.cc
	$(CPP) $(CFLAGS) -std=gnu++0x $(CPPFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

atexit_thread.o: atexit_thread.cc
	$(CPP) $(CFLAGS) -std=gnu++0x $(CPPFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

nested_exception.o: nested_exception.cc
	$(CPP) $(CFLAGS) -std=gnu++0x $(CPPFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean: 
	-rm -f *.o 

