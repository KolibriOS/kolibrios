
SRC_DIR:=$(SDK_DIR)/sources
MESA_SRC:= $(SRC_DIR)/Mesa/src

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -Os 
CFLAGS_OPT+= -fomit-frame-pointer -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

LDFLAGS = -nostdlib -shared -s --image-base 0 -T $(SRC_DIR)/newlib/dll.lds --out-implib libGL.dll.a -e _DllStartup

INC_MESA:= -I$(SRC_DIR)/newlib/libc/include -I$(SRC_DIR)/Mesa/include
INC_MESA+= -I./src -I$(MESA_SRC)/glsl -I$(MESA_SRC)/mesa -I$(MESA_SRC)/mapi


LIBPATH:= -L$(SDK_DIR)/lib -L/home/autobuild/tools/win32/mingw32/lib

LIBS:=  -ldll -ldrm.dll -lc.dll -lgcc

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.5\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.5\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -D_GLAPI_DLL_EXPORTS


LIBGL_SRC = \
	glapi/glapi.c			\
	glapi/glapi_dispatch.c		\
	glapi/glapi_entrypoint.c	\
	glapi/glapi_getproc.c		\
	glapi/glapi_nop.c		\
	glapi/glapi_x86.S		\
	u_current.c			\
	u_execmem.c			\
	$(NULL)
 
LIBGL_OBJS =  $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(LIBGL_SRC)))

# targets 

all: libGL.dll 

libGL.dll: $(LIBGL_OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ mesa.def $(LIBGL_OBJS) $(LIBS)
#	$(STRIP) $@
	mv -f $@ $(SDK_DIR)/bin
	mv -f libGL.dll.a $(SDK_DIR)/lib

%.o : %.c Makefile
	$(CC) $(CFLAGS) -std=c99 $(MESA_DEFS) $(INC_MESA) -o $@ $<

%.o : %.S Makefile
	$(CC) -c -U_WIN32 -I $(MESA_SRC)/mesa -o $@ $<

 