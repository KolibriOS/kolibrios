
export CC = kos32-gcc
export AR = kos32-ar
export LD = kos32-ld
export STRIP = kos32-strip

export SDK_DIR:= $(abspath ../../sdk)


CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -W -Wall -Wwrite-strings -Wc++-compat -Wstrict-prototypes -pedantic
CFLAGS   = -c $(CFLAGS_OPT)

LDFLAGS = -nostdlib -shared -s --image-base 0 -T ../newlib/dll.lds -e _DllStartup
LDFLAGS+= --out-implib

SUBDIRS = libiberty bfd opcodes binutils gas ld

# targets 

all:
	@echo $(SDK_DIR);	\
	for i in $(SUBDIRS); do	\
	$(MAKE) -C $$i;		\
	done
 