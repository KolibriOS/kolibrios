include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc


AM_CFLAGS = \
	$(GALLIUM_DRIVER_CFLAGS) \
	$(RADEON_CFLAGS) \
	-Wstrict-overflow=0
# ^^ disable warnings about overflows (os_time_timeout)

noinst_LTLIBRARIES = libradeon.la

libradeon_la_SOURCES = \
	$(C_SOURCES)

if NEED_RADEON_LLVM

AM_CFLAGS += \
	$(LLVM_CFLAGS)

libradeon_la_SOURCES += \
	$(LLVM_C_FILES)

libradeon_la_LIBADD = \
	$(CLOCK_LIB) \
	$(LLVM_LIBS) \
	$(ELF_LIB)

libradeon_la_LDFLAGS = \
	$(LLVM_LDFLAGS)

endif

EXTRA_DIST = \
	LLVM_REVISION.txt
