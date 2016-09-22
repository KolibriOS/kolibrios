LIBRARY= libstdc++

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/lib

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer -ffunction-sections -fdata-sections
CFLAGS+= -fno-rtti -fno-exceptions

ARFLAGS= crs

INCLUDES= -I./include/mingw32 -I./include -I../newlib/libc/include

LIBS:=  -ldll -lc.dll

DEFINES= -DHAVE_CONFIG_H -DIN_GCC -DUSE_EMUTLS=1 -D_GLIBCXX_HAVE_TLS

host_sources = \
        src/c++98/atomicity.cc 			\
        src/c++98/codecvt_members.cc 		\
        src/c++98/collate_members.cc 		\
        src/c++98/ctype_configure_char.cc 	\
        src/c++98/ctype_members.cc 		\
        src/c++98/messages_members.cc 		\
        src/c++98/monetary_members.cc 		\
        src/c++98/numeric_members.cc 		\
        src/c++98/time_members.cc

inst_sources = \
	src/c++98/allocator-inst.cc \
	src/c++98/concept-inst.cc \
	src/c++98/ext-inst.cc \
	src/c++98/ios-inst.cc \
	src/c++98/iostream-inst.cc \
	src/c++98/istream-inst.cc \
	src/c++98/locale-inst.cc \
	src/c++98/misc-inst.cc \
	src/c++98/ostream-inst.cc \
	src/c++98/sstream-inst.cc \
	src/c++98/streambuf-inst.cc \
	src/c++98/wlocale-inst.cc

parallel_sources = src/c++98/parallel_settings.cc

host_sources_extra = \
	src/c++98/basic_file.cc	\
	src/c++98/c++locale.cc	\
	${inst_sources} ${parallel_sources}

C++98SRCS = \
	src/c++98/bitmap_allocator.cc 		\
	src/c++98/pool_allocator.cc 		\
	src/c++98/mt_allocator.cc 		\
	src/c++98/codecvt.cc 			\
	src/c++98/complex_io.cc 		\
	src/c++98/ctype.cc 			\
	src/c++98/globals_io.cc 		\
	src/c++98/hash_tr1.cc 			\
	src/c++98/hashtable_tr1.cc 		\
	src/c++98/ios.cc 			\
	src/c++98/ios_failure.cc 		\
	src/c++98/ios_init.cc 			\
	src/c++98/ios_locale.cc 		\
	src/c++98/list.cc 			\
	src/c++98/list-aux.cc 			\
	src/c++98/list-aux-2.cc 		\
	src/c++98/list_associated.cc 		\
	src/c++98/list_associated-2.cc 		\
	src/c++98/locale.cc 			\
	src/c++98/locale_init.cc 		\
	src/c++98/locale_facets.cc 		\
	src/c++98/localename.cc 		\
	src/c++98/math_stubs_float.cc 		\
	src/c++98/math_stubs_long_double.cc 	\
	src/c++98/stdexcept.cc 			\
	src/c++98/strstream.cc 			\
	src/c++98/tree.cc 			\
	src/c++98/istream.cc 			\
	src/c++98/streambuf.cc 			\
	src/c++98/valarray.cc 			\
	${host_sources} 			\
	${host_sources_extra}

inst11_sources = \
	src/c++11/fstream-inst.cc \
	src/c++11/string-inst.cc \
	src/c++11/wstring-inst.cc

C++11SRCS = \
	src/c++11/chrono.cc 			\
	src/c++11/condition_variable.cc 	\
	src/c++11/debug.cc 			\
	src/c++11/functexcept.cc 		\
	src/c++11/functional.cc 		\
	src/c++11/future.cc 			\
	src/c++11/hash_c++0x.cc 		\
	src/c++11/hashtable_c++0x.cc 		\
	src/c++11/limits.cc 			\
	src/c++11/mutex.cc 			\
	src/c++11/placeholders.cc		\
	src/c++11/random.cc 			\
	src/c++11/regex.cc  			\
	src/c++11/shared_ptr.cc 		\
	src/c++11/system_error.cc 		\
	${inst11_sources}

compat_sources = \
	src/c++98/compatibility.cc 			\
	src/c++98/compatibility-debug_list.cc		\
	src/c++98/compatibility-debug_list-2.cc		\
	src/c++11/compatibility-c++0x.cc		\
	src/c++11/compatibility-atomic-c++0x.cc		\
	src/c++11/compatibility-thread-c++0x.cc		\
	src/c++11/compatibility-chrono.cc

OBJ98 =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(C++98SRCS)))

OBJ11 =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(C++11SRCS)))

com_obj =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(compat_sources)))

# targets

all:$(LIBRARY).a

$(LIBRARY).a: $(OBJ98) $(OBJ11) $(com_obj) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJ98) $(OBJ11) $(com_obj)
	mv -f $(LIBRARY).a $(INSTALLDIR)

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.cc Makefile
	$(CPP) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/strstream.o: src/c++98/strstream.cc Makefile
	$(CPP) $(CFLAGS) -Wno-deprecated $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/allocator-inst.o: src/c++98/allocator-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/concept-inst.o: src/c++98/concept-inst.cc Makefile
	$(CPP) $(CFLAGS) -D_GLIBCXX_CONCEPT_CHECKS -fimplicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/ext-inst.o: src/c++98/ext-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/ios-inst.o: src/c++98/ios-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/iostream-inst.o: src/c++98/iostream-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/istream-inst.o: src/c++98/istream-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/locale-inst.o: src/c++98/locale-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/misc-inst.o: src/c++98/misc-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/ostream-inst.o: src/c++98/ostream-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/sstream-inst.o: src/c++98/sstream-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/streambuf-inst.o: src/c++98/streambuf-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/wlocale-inst.o: src/c++98/wlocale-inst.cc Makefile
	$(CPP) $(CFLAGS) -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++98/parallel_settings.o: src/c++98/parallel_settings.cc Makefile 
	$(CPP) $(CFLAGS) -D_GLIBCXX_PARALLEL $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/chrono.o: src/c++11/chrono.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/condition_variable.o: src/c++11/condition_variable.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/debug.o: src/c++11/debug.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/functexcept.o: src/c++11/functexcept.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/functional.o: src/c++11/functional.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/future.o: src/c++11/future.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/hash_c++0x.o: src/c++11/hash_c++0x.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/hashtable_c++0x.o: src/c++11/hashtable_c++0x.cc
	$(CPP) $(CFLAGS) -std=gnu++11 -fimplicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/limits.o: src/c++11/limits.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/mutex.o: src/c++11/mutex.cc 		
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/placeholders.o: src/c++11/placeholders.cc	
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/random.o: src/c++11/random.cc 		
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/regex.o: src/c++11/regex.cc 		
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/shared_ptr.o: src/c++11/shared_ptr.cc	
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/system_error.o: src/c++11/system_error.cc	
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/thread.o: src/c++11/thread.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/fstream-inst.o: src/c++11/fstream-inst.cc
	$(CPP) $(CFLAGS) -std=gnu++11 -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/string-inst.o: src/c++11/string-inst.cc
	$(CPP) $(CFLAGS) -std=gnu++11 -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/wstring-inst.o: src/c++11/wstring-inst.cc
	$(CPP) $(CFLAGS) -std=gnu++11 -fno-implicit-templates $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/compatibility-c++0x.o: src/c++11/compatibility-c++0x.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/compatibility-atomic-c++0x.o: src/c++11/compatibility-atomic-c++0x.cc 
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/compatibility-thread-c++0x.o: src/c++11/compatibility-thread-c++0x.cc 
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

src/c++11/compatibility-chrono.o: src/c++11/compatibility-chrono.cc
	$(CPP) $(CFLAGS) -std=gnu++11 $(DEFINES) $(INCLUDES) -o $@ $<

clean:
	-rm -f *.o

