
SRC_DIR:=$(SDK_DIR)/sources
MESA_SRC:= $(SRC_DIR)/Mesa/mesa-9.2.5/src

CFLAGS_OPT = -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER -Os 
CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS   = -c $(CFLAGS_OPT)

LDFLAGS = -shared -s --image-base 0 -T dll.lds --out-implib libegl.dll.a -e _DllStartup

INC_EGL:= -I$(SRC_DIR)/newlib/libc/include -I$(SDK_DIR)/includes -I$(SRC_DIR)/Mesa/mesa-9.2.5/include
INC_EGL+= -I$(SRC_DIR)/libdrm -I$(SRC_DIR)/libdrm/include/drm -I./main -I./wayland
INC_EGL+= -I$(MESA_SRC)/gbm/backends/dri -I$(MESA_SRC)/gbm/main

LIBS:=  -ldll -ldrm.dll -lgcc -lc.dll

MESA_DEFS= -DMESA_DLL -DBUILD_GL32 -DMAPI_MODE_UTIL -DMAPI_TABLE_NUM_STATIC=87 -DUSE_X86_ASM -DUSE_MMX_ASM -DUSE_SSE_ASM -DMAPI_TABLE_NUM_DYNAMIC=256
MESA_DEFS+= -DPACKAGE_NAME=\"Mesa\" -DPACKAGE_VERSION=\"9.2.5\" -DPACKAGE_BUGREPORT=\"https://bugs.freedesktop.org/enter_bug.cgi\?product=Mesa\"
MESA_DEFS+= -DPACKAGE_URL=\"\" -DPACKAGE=\"mesa\" -DVERSION=\"9.2.5\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1
MESA_DEFS+= -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1

EGL_DEFS = -DHAVE_DRM_PLATFORM -D__unix__ -DMESA_EGL_NO_X11_HEADERS -D_EGL_BUILT_IN_DRIVER_DRI2

EGL_SRC = 					\
	drivers/dri2/egl_dri2.c		\
	drivers/dri2/platform_drm.c	\
	main/eglapi.c			\
	main/eglarray.c			\
	main/eglconfig.c		\
	main/eglcontext.c		\
	main/eglcurrent.c		\
	main/egldisplay.c		\
	main/egldriver.c		\
	main/eglfallbacks.c		\
	main/eglglobals.c		\
	main/eglimage.c			\
	main/egllog.c			\
	main/eglmisc.c			\
	main/eglmode.c			\
	main/eglscreen.c		\
	main/eglstring.c		\
	main/eglsurface.c		\
	main/eglsync.c			\
	$(MESA_SRC)/gbm/backends/dri/driver_name.c	\
	$(MESA_SRC)/gbm/backends/dri/gbm_dri.c		\
	$(MESA_SRC)/gbm/main/backend.c			\
	$(MESA_SRC)/gbm/main/gbm.c			\
	$(NULL)

 
EGL_OBJS   =  $(patsubst %.c, %.o, $(EGL_SRC))

# targets 

all: libegl.dll

libegl.dll: $(EGL_OBJS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(MESA_SRC)/../egl.def $(EGL_OBJS) $(LIBS)
#	$(STRIP) $@
#	sed -e "s/ @[^ ]*//" egl1.def > egl.def
	mv -f $@ $(SDK_DIR)/bin
	mv -f libegl.dll.a $(INSTALLDIR)

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(MESA_DEFS) $(EGL_DEFS) $(INC_EGL) -o $@ $<


 