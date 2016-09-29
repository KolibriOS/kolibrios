LIBRARY= libsupc++

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/mingw32/lib

export SDK_DIR:= $(abspath ../../../../../sdk)

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer
CFLAGSCPP = -nostdinc++ -fno-implicit-templates -Wall -Wextra -Wwrite-strings -Wcast-qual -Wabi
CFLAGSCPP+= -fdiagnostics-show-location=once -ffunction-sections -fdata-sections

ARFLAGS= crs

GLIBCXX_INCLUDE_DIR= ../include
INCLUDES= -I../include/mingw32 -I../include -I./ -I$(SDK_DIR)/sources/newlib/libc/include

DEFINES= -D_GLIBCXX_HAVE_TLS

C_COMPILE= $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES)
CXXCOMPILE= $(CPP) $(CFLAGS) $(CFLAGSCPP) $(INCLUDES) $(DEFINES)


SOURCES = 					\
	cp-demangle.c \
	array_type_info.cc \
	atexit_arm.cc \
	atexit_thread.cc \
	bad_alloc.cc \
	bad_array_length.cc \
	bad_array_new.cc \
	bad_cast.cc \
	bad_typeid.cc \
	class_type_info.cc \
	del_op.cc \
	del_ops.cc \
	del_opnt.cc \
	del_opv.cc \
	del_opvs.cc \
	del_opvnt.cc \
	dyncast.cc \
	eh_alloc.cc \
	eh_arm.cc \
	eh_aux_runtime.cc \
	eh_call.cc \
	eh_catch.cc \
	eh_exception.cc \
	eh_globals.cc \
	eh_personality.cc \
	eh_ptr.cc \
	eh_term_handler.cc \
	eh_terminate.cc \
	eh_tm.cc \
	eh_throw.cc \
	eh_type.cc \
	eh_unex_handler.cc \
	enum_type_info.cc \
	function_type_info.cc \
	fundamental_type_info.cc \
	guard.cc \
	guard_error.cc \
	hash_bytes.cc \
	nested_exception.cc \
	new_handler.cc \
	new_op.cc \
	new_opnt.cc \
	new_opv.cc \
	new_opvnt.cc \
	pbase_type_info.cc \
	pmem_type_info.cc \
	pointer_type_info.cc \
	pure.cc \
	si_class_type_info.cc \
	tinfo.cc \
	tinfo2.cc \
	vec.cc \
	vmi_class_type_info.cc \
	vterminate.cc
  

OBJECTS =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(SOURCES)))

# targets 

all:$(LIBRARY).a

$(LIBRARY).a: $(OBJECTS) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJECTS)
	mv -f $(LIBRARY).a $(INSTALLDIR)



%.o : %.cc Makefile
	$(CXXCOMPILE) -o $@ $<


cp-demangle.o: cp-demangle.c
	$(C_COMPILE) -DHAVE_CONFIG_H -D_USE_32BIT_TIME_T -DIN_GLIBCPP_V3 -Wno-error -o $@ $<

bad_array_length.o: bad_array_length.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

bad_array_new.o: bad_array_new.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

eh_aux_runtime.o: eh_aux_runtime.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

eh_ptr.o: eh_ptr.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

eh_terminate.o: eh_terminate.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

eh_throw.o: eh_throw.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

guard.o: guard.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

atexit_thread.o: atexit_thread.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

nested_exception.o: nested_exception.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

new_handler.o: new_handler.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

new_op.o: new_op.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

new_opnt.o: new_opnt.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

del_ops.o: del_ops.cc 
	$(CXXCOMPILE) -std=gnu++14 -o $@ $<

del_opvs.o: del_opvs.cc
	$(CXXCOMPILE) -std=gnu++14 -o $@ $<


clean: 
	-rm -f *.o 

