LIBRARY= libstdc++

CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/mingw32/lib

export SDK_DIR:= $(abspath ../../../../../sdk)

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -fomit-frame-pointer
CFLAGS+= -nostdinc++ -fno-implicit-templates -Wall -Wextra -Wwrite-strings -Wcast-qual -Wabi
CFLAGS+= -fdiagnostics-show-location=once -ffunction-sections -fdata-sections

ARFLAGS= crs

GLIBCXX_INCLUDE_DIR= ../include
INCLUDES= -I../include/mingw32 -I../include -I../libsupc++ -I$(SDK_DIR)/sources/newlib/libc/include

LIBS:=  -ldll -lc.dll

GLIBCXX_ABI_FLAGS = -D_GLIBCXX_USE_CXX11_ABI=0
PARALLEL_FLAGS = -D_GLIBCXX_PARALLEL

DEFINES= -D_GLIBCXX_HAVE_TLS

CXXCOMPILE= $(CPP) $(CFLAGS) $(INCLUDES) $(DEFINES)


cxx98_sources = \
	c++98/compatibility.cc \
	c++98/compatibility-debug_list.cc	\
	c++98/compatibility-debug_list-2.cc

cxx11_sources = \
	c++11/compatibility-c++0x.cc \
	c++11/compatibility-atomic-c++0x.cc \
	c++11/compatibility-thread-c++0x.cc \
	c++11/compatibility-chrono.cc \
	c++11/compatibility-condvar.cc

compat_sources = $(cxx98_sources) $(cxx11_sources)

cow_string_host_sources = \
	c++98/collate_members_cow.cc \
	c++98/messages_members_cow.cc \
	c++98/monetary_members_cow.cc \
	c++98/numeric_members_cow.cc


# Source files linked in via configuration/make substitution for a
# particular host.
host_sources = \
	$(cow_string_host_sources)		\
	c++98/atomicity.cc 			\
	c++98/codecvt_members.cc 		\
	c++98/collate_members.cc 		\
	c++98/messages_members.cc 		\
	c++98/monetary_members.cc 		\
	c++98/numeric_members.cc 		\
	c++98/time_members.cc

# Source files linked in via configuration/make substitution for a
# particular host, but with ad hoc naming rules.
host_sources_extra = \
	c++98/basic_file.cc c++98/c++locale.cc \
	${inst_sources} ${parallel_sources}

cxx11_abi_sources = \
	c++98/cow-istream-string.cc


inst_sources = \
	c++98/allocator-inst.cc \
	c++98/concept-inst.cc \
	c++98/ext-inst.cc \
	c++98/misc-inst.cc \

parallel_sources = c++98/parallel_settings.cc

sources = \
    c++98/bitmap_allocator.cc       \
    c++98/pool_allocator.cc         \
    c++98/mt_allocator.cc       \
    c++98/codecvt.cc            \
    c++98/complex_io.cc         \
    c++98/globals_io.cc         \
    c++98/hash_tr1.cc           \
    c++98/hashtable_tr1.cc      \
    c++98/ios_failure.cc        \
    c++98/ios_init.cc           \
    c++98/ios_locale.cc         \
    c++98/list.cc           \
    c++98/list-aux.cc           \
    c++98/list-aux-2.cc         \
    c++98/list_associated.cc        \
    c++98/list_associated-2.cc      \
    c++98/locale.cc             \
    c++98/locale_init.cc        \
    c++98/locale_facets.cc      \
    c++98/localename.cc         \
    c++98/math_stubs_float.cc       \
    c++98/math_stubs_long_double.cc     \
    c++98/stdexcept.cc          \
    c++98/strstream.cc          \
    c++98/tree.cc           \
    c++98/istream.cc            \
    c++98/istream-string.cc     \
    c++98/streambuf.cc          \
    c++98/valarray.cc           \
	${cxx11_abi_sources} 			\
	${host_sources} 			\
	${host_sources_extra}


host_sources_11 = \
    c++11/ctype_configure_char.cc \
    c++11/ctype_members.cc

cxx11_abi_sources_11 = \
    c++11/cow-locale_init.cc \
    c++11/cow-shim_facets.cc \
    c++11/cxx11-hash_tr1.cc \
    c++11/cxx11-ios_failure.cc \
    c++11/cxx11-shim_facets.cc \
    c++11/cxx11-stdexcept.cc

sources_11 = \
    c++11/chrono.cc \
    c++11/codecvt.cc \
    c++11/condition_variable.cc \
    c++11/cow-stdexcept.cc \
    c++11/ctype.cc \
    c++11/debug.cc \
    c++11/functexcept.cc \
    c++11/functional.cc \
    c++11/futex.cc \
    c++11/future.cc \
    c++11/hash_c++0x.cc \
    c++11/hashtable_c++0x.cc \
    c++11/ios.cc \
    c++11/limits.cc \
    c++11/mutex.cc \
    c++11/placeholders.cc \
    c++11/random.cc \
    c++11/regex.cc  \
    c++11/shared_ptr.cc \
    c++11/snprintf_lite.cc \
    c++11/system_error.cc \
    c++11/thread.cc \
	${cxx11_abi_sources_11} \
	${host_sources_11}


extra_string_inst_sources = \
    c++11/cow-fstream-inst.cc \
    c++11/cow-sstream-inst.cc \
    c++11/cow-string-inst.cc \
    c++11/cow-wstring-inst.cc \
    c++11/cxx11-locale-inst.cc \
    c++11/cxx11-wlocale-inst.cc

inst_sources_11 = \
	$(extra_string_inst_sources) \
    c++11/ext11-inst.cc \
    c++11/fstream-inst.cc \
    c++11/ios-inst.cc \
    c++11/iostream-inst.cc \
    c++11/istream-inst.cc \
    c++11/locale-inst.cc \
    c++11/ostream-inst.cc \
    c++11/sstream-inst.cc \
    c++11/streambuf-inst.cc \
    c++11/string-inst.cc \
    c++11/wlocale-inst.cc \
    c++11/wstring-inst.cc

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



c++98/%.o : c++98/%.cc Makefile
	$(CXXCOMPILE) -o $@ $<

c++11/%.o : c++11/%.cc Makefile
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

c++98/collate_members_cow.o: c++98/collate_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

c++98/messages_members_cow.o: c++98/messages_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

c++98/monetary_members_cow.o: c++98/monetary_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

c++98/numeric_members_cow.o: c++98/numeric_members_cow.cc
	$(CXXCOMPILE) $(GLIBCXX_ABI_FLAGS) -fimplicit-templates -o $@ $<

c++98/locale_init.o: c++98/locale_init.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

c++98/localename.o: c++98/localename.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

c++98/strstream.o: c++98/strstream.cc Makefile
	$(CXXCOMPILE) -I$(GLIBCXX_INCLUDE_DIR)/backward -Wno-deprecated -o $@ $<

# Use special rules for the concept-checking instantiations so that all

# the generated template functions are also instantiated.  Force the checks

# to be on so that the instantiations are actually seen.

c++98/concept-inst.o: c++98/concept-inst.cc
	$(CXXCOMPILE) -D_GLIBCXX_CONCEPT_CHECKS -fimplicit-templates -o $@ $<

c++98/parallel_settings.o: c++98/parallel_settings.cc
	$(CXXCOMPILE) $(PARALLEL_FLAGS) -o $@ $<


c++11/hashtable_c++0x.o: c++11/hashtable_c++0x.cc
	$(CXXCOMPILE) -std=gnu++11 -fimplicit-templates -o $@ $<

c++11/compatibility-c++0x.o: c++11/compatibility-c++0x.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

c++11/compatibility-thread-c++0x.o: c++11/compatibility-thread-c++0x.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

c++11/compatibility-chrono.o: c++11/compatibility-chrono.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

c++11/compatibility-condvar.o: c++11/compatibility-condvar.cc
	$(CXXCOMPILE) -std=gnu++11 -o $@ $<

clean:
	-rm -f *.o

