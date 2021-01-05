NAME = kpm

FASM = fasm
CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP = kos32-g++
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -Os -fomit-frame-pointer -fno-asynchronous-unwind-tables -fno-ident -fomit-frame-pointer  -mno-ms-bitfields
ARFLAG = crs

SDK_DIR = $(abspath ../../sdk)
LIB_DIR = $(SDK_DIR)/lib

INCLUDES = -I. -I$(SDK_DIR)/sources/newlib/libc/include
INCLUDES += -I$(SDK_DIR)/sources/freetype/include

#DEFINES= -DDEBUG=1 -D_7ZIP_PPMD_SUPPPORT

DEFINES = -DNDEBUG

LIBS = -liberty -lsupc++ -lgcc -lc.dll 

LIBPATH = -L$(LIB_DIR) -L/home/autobuild/tools/win32/mingw32/lib

LDFLAGS = -static -nostdlib --stack 0x10000 -Map kpm.map -T$(SDK_DIR)/sources/newlib/app.lds --image-base 0 --subsystem native

SOURCES =	http.asm			\
			kpm.c				\
			utils.c				\
			collection.cpp			\
			tinyxml/tinyxml.cpp		\
			tinyxml/tinystr.cpp		\
			tinyxml/tinyxmlparser.cpp	\
			tinyxml/tinyxmlerror.cpp	\
			7z/7z.c				\
			7z/7zFile.c			\
			7z/7zStream.c			\
			7z/7zCrc.c			\
			7z/7zCrcOpt.c			\
			7z/CpuArch.c			\
			7z/7zArcIn.c			\
			7z/7zBuf.c			\
			7z/7zDec.c			\
			7z/LzmaDec.c			\
			7z/Lzma2Dec.c			\
			7z/Bcj2.c			\
			7z/Delta.c			\
			7z/Bra.c			\
			7z/Bra86.c			\
			7z/BraIA64.c			\
			7z/7zAlloc.c			\
			$(NULL)
 
OBJECTS = $(patsubst %.asm, %.o, $(patsubst %.cpp, %.o, $(patsubst %.c, %.o, $(SOURCES))))

# targets  

all: $(NAME)

$(NAME): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(OBJECTS) $(LIBS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.cpp Makefile
	$(CPP) $(CFLAGS) -fno-rtti -fno-exceptions $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.asm Makefile
	$(FASM) $< $@


clean: 
	-rm -f *.o */*.o




