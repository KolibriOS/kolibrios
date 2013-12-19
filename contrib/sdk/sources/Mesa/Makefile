
EGL = egl

CC = gcc

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -O2 -march=i686 -msse2
CFLAGS_OPT+= -fomit-frame-pointer -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

LD = ld
LDFLAGS = -nostdlib -shared -s --image-base 0 -T ../newlib/dll.lds --out-implib lib$(EGL).dll.a -e _DllStartup

STRIP = $(PREFIX)strip

INCLUDES= -I./include -I../libdrm -I../libdrm/include/drm -I./src/egl/main -I./src/gbm/backends/dri -I./src/gbm/main -I../newlib/include

LIBPATH:= -L../../lib

LIBS:=  -ldll -ldrm.dll -lc.dll -lgcc

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.2\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.2\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1

EGL_DEFS = -DHAVE_DRM_PLATFORM -D__unix__ -DMESA_EGL_NO_X11_HEADERS -D_EGL_BUILT_IN_DRIVER_DRI2

EGL_SRC = 					\
	src/egl/drivers/dri2/egl_dri2.c		\
	src/egl/drivers/dri2/platform_drm.c	\
	src/egl/main/eglapi.c			\
	src/egl/main/eglarray.c			\
	src/egl/main/eglconfig.c		\
	src/egl/main/eglcontext.c		\
	src/egl/main/eglcurrent.c		\
	src/egl/main/egldisplay.c		\
	src/egl/main/egldriver.c		\
	src/egl/main/eglfallbacks.c		\
	src/egl/main/eglglobals.c		\
	src/egl/main/eglimage.c			\
	src/egl/main/egllog.c			\
	src/egl/main/eglmisc.c			\
	src/egl/main/eglmode.c			\
	src/egl/main/eglscreen.c		\
	src/egl/main/eglstring.c		\
	src/egl/main/eglsurface.c		\
	src/egl/main/eglsync.c			\
	src/gbm/backends/dri/gbm_dri.c		\
	src/gbm/main/backend.c			\
	src/gbm/main/gbm.c			\
	$(NULL)
 
EGL_OBJS     = $(patsubst %.c, %.o, $(EGL_SRC))

# targets 

all:$(EGL).dll

$(EGL).dll: $(EGL_OBJS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ egl.def $(EGL_OBJS) $(LIBS)
	$(STRIP) $@
#	sed -e "s/ @[^ ]*//" egl1.def > egl.def
	mv -f $@ ../../bin
	mv -f lib$(EGL).dll.a ../../lib   
	
%.o : %.c Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(EGL_DEFS) $(INCLUDES) -o $@ $<

 