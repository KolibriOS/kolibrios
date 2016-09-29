LIBRARY= libstdc++

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/mingw32/lib

export SDK_DIR:= $(abspath ../../../../sdk)

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer
CFLAGS+= -nostdinc++ -fno-implicit-templates -Wall -Wextra -Wwrite-strings -Wcast-qual -Wabi
CFLAGS+= -fdiagnostics-show-location=once -ffunction-sections -fdata-sections

ARFLAGS= crs

GLIBCXX_INCLUDE_DIR= ./include
INCLUDES= -I./include/mingw32 -I./include -I./libsupc++ -I$(SDK_DIR)/sources/newlib/libc/include

LIBS:=  -ldll -lc.dll

GLIBCXX_ABI_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0
PARALLEL_FLAGS = -D_GLIBCXX_PARALLEL

DEFINES= -D_GLIBCXX_HAVE_TLS

CXXCOMPILE= $(CPP) $(CFLAGS) $(INCLUDES) $(DEFINES) 


cxx98_sources = \
	src/c++98/compatibility.cc \
	src/c++98/compatibility-debug_list.cc	\
	src/c++98/compatibility-debug_list-2.cc

cxx11_sources = \
	src/c++11/compatibility-c++0x.cc \
	src/c++11/compatibility-atomic-c++0x.cc \
	src/c++11/compatibility-thread-c++0x.cc \
	src/c++11/compatibility-chrono.cc \
	src/c++11/compatibility-condvar.cc
 
compat_sources = $(cxx98_sources) $(cxx11_sources)

cow_string_host_sources = \
	src/c++98/collate_members_cow.cc \
	src/c++98/messages_members_cow.cc \
	src/c++98/monetary_members_cow.cc \
	src/c++98/numeric_members_cow.cc


# Source files linked in via configuration/make substitution for a
# particular host.
host_sources = \
	$(cow_string_host_sources)		\
	src/c++98/atomicity.cc 			\
	src/c++98/codecvt_members.cc 		\
	src/c++98/collate_members.cc 		\
	src/c++98/messages_members.cc 		\
	src/c++98/monetary_members.cc 		\
	src/c++98/numeric_members.cc 		\
	src/c++98/time_members.cc

# Source files linked in via configuration/make substitution for a
# particular host, but with ad hoc naming rules.
host_sources_extra = \
	src/c++98/basic_file.cc src/c++98/c++locale.cc \
	${inst_sources} ${parallel_sources}

cxx11_abi_sources = \
	src/c++98/cow-istream-string.cc


inst_sources = \
	src/c++98/allocator-inst.cc \
	src/c++98/concept-inst.cc \
	src/c++98/ext-inst.cc \
	src/c++98/misc-inst.cc \

parallel_sources = src/c++98/parallel_settings.cc

sources = \
	src/c++98/bitmap_allocator.cc 		\
	src/c++98/pool_allocator.cc 		\
	src/c++98/mt_allocator.cc 		\
	src/c++98/codecvt.cc 			\
	src/c++98/complex_io.cc 		\
	src/c++98/globals_io.cc 		\
	src/c++98/hash_tr1.cc 			\
	src/c++98/hashtable_tr1.cc 		\
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
	src/c++98/istream-string.cc		\
	src/c++98/streambuf.cc 			\
	src/c++98/valarray.cc 			\
	${cxx11_abi_sources} 			\
	${host_sources} 			\
	${host_sources_extra}


host_sources_11 = \
	src/c++11/ctype_configure_char.cc \
	src/c++11/ctype_members.cc

cxx11_abi_sources_11 = \
	src/c++11/cow-locale_init.cc \
	src/c++11/cow-shim_facets.cc \
	src/c++11/cxx11-hash_tr1.cc \
	src/c++11/cxx11-ios_failure.cc \
	src/c++11/cxx11-shim_facets.cc \
	src/c++11/cxx11-stdexcept.cc

sources_11 = \
	src/c++11/chrono.cc \
	src/c++11/codecvt.cc \
	src/c++11/condition_variable.cc \
	src/c++11/cow-stdexcept.cc \
	src/c++11/ctype.cc \
	src/c++11/debug.cc \
	src/c++11/functexcept.cc \
	src/c++11/functional.cc \
	src/c++11/futex.cc \
	src/c++11/future.cc \
	src/c++11/hash_c++0x.cc \
	src/c++11/hashtable_c++0x.cc \
	src/c++11/ios.cc \
	src/c++11/limits.cc \
	src/c++11/mutex.cc \
	src/c++11/placeholders.cc \
	src/c++11/random.cc \
	src/c++11/regex.cc  \
	src/c++11/shared_ptr.cc \
	src/c++11/snprintf_lite.cc \
	src/c++11/system_error.cc \
	src/c++11/thread.cc \
	${cxx11_abi_sources_11} \
	${host_sources_11}


extra_string_inst_sources = \
	src/c++11/cow-fstream-inst.cc \
	src/c++11/cow-sstream-inst.cc \
	src/c++11/cow-string-inst.cc \
	src/c++11/cow-wstring-inst.cc \
	src/c++11/cxx11-locale-inst.cc \
	src/c++11/cxx11-wlocale-inst.cc

inst_sources_11 = \
	$(extra_string_inst_sources) \
	src/c++11/ext11-inst.cc \
	src/c++11/fstream-inst.cc \
	src/c++11/ios-inst.cc \
	src/c++11/iostream-inst.cc \
	src/c++11/istream-inst.cc \
	src/c++11/locale-inst.cc \
	src/c++11/ostream-inst.cc \
	src/c++11/sstream-inst.cc \
	src/c++11/streambuf-inst.cc \
	src/c++11/string-inst.cc \
	src/c++11/wlocale-inst.cc \
	src/c++11/wstring-inst.cc

sources_11+= $(inst_sources_11)

OBJ98 =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(sources)))

OBJ11 =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(sources_11)))

com_obj =  $(patsubst %.cc, %.o, $(patsubst %.c, %.o, $(compat_sources)))

# targets

all:$(LIBRARY).a

#$(LIBRARY).a: $(OBJ98) $(OBJ11) $(com_obj) Makefile

$(LIBRARY).a: $(OBJ98) $(OBJ11) $(com_obj) Makefile
	$(AR) $(ARFLAGS) $(LIBRARY).a $(OBJ98) $(OBJ11) $(com_obj)
	mv -f $(LIBRARY).a $(INSTALLDIR)

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

#src/%.o : src/%.cc Makefile
#	$(CXXCOMPILE) -o $@ $<



src/c++98/%.o : src/c++98/%.cc Makefile
	$(CXXCOMPILE) -o $@ $<

src/c++11/%.o : src/c++11/%.cc Makefile
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

src/c++98/collate_members_cow.o: src/c++98/collate_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

src/c++98/messages_members_cow.o: src/c++98/messages_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

src/c++98/monetary_members_cow.o: src/c++98/monetary_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

src/c++98/numeric_members_cow.o: src/c++98/numeric_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

src/c++98/locale_init.o: src/c++98/locale_init.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

src/c++98/localename.o: src/c++98/localename.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

src/c++98/strstream.o: src/c++98/strstream.cc Makefile
	$(CXXCOMPILE) -I$(GLIBCXX_INCLUDE_DIR)/backward -Wno-deprecated -o $@ $<

# Use special rules for the concept-checking instantiations so that all

# the generated template functions are also instantiated.  Force the checks

# to be on so that the instantiations are actually seen.

src/c++98/concept-inst.o: src/c++98/concept-inst.cc 
	$(CXXCOMPILE) -D_GLIBCXX_CONCEPT_CHECKS -fimplicit-templates -o $@ $<

src/c++98/parallel_settings.o: src/c++98/parallel_settings.cc
	$(CXXCOMPILE) $(PARALLEL_FLAGS) -o $@ $<


src/c++11/hashtable_c++0x.o: src/c++11/hashtable_c++0x.cc
	$(CXXCOMPILE) -std=gnu++11 -fimplicit-templates -o $@ $<

src/c++11/compatibility-c++0x.o: src/c++11/compatibility-c++0x.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

src/c++11/compatibility-thread-c++0x.o: src/c++11/compatibility-thread-c++0x.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

src/c++11/compatibility-chrono.o: src/c++11/compatibility-chrono.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

src/c++11/compatibility-condvar.o: src/c++11/compatibility-condvar.cc 
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

clean:
	-rm -f *.o

