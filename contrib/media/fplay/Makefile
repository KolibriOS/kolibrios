NAME= Fplay

FASM= fasm
CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld
CPP= kos32-g++
STRIP = kos32-strip

CFLAGS = -U_Win32 -U_WIN32 -U__MINGW32__ -c -O2 -fno-ident -std=c99 -fomit-frame-pointer  -mno-ms-bitfields
ARFLAG = crs

SDK_DIR:= $(abspath ../../sdk)
LIB_DIR:= $(SDK_DIR)/lib

INCLUDES= -I. -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/ffmpeg/ffmpeg-2.8
INCLUDES+=-I$(SDK_DIR)/sources/freetype/include -I$(SDK_DIR)/sources/vaapi/libva-1.6.2

DEFINES= -DDEBUG=1 -D_GNU_SOURCE -DHAVE_VAAPI -DPACKAGE_NAME=\"Fplay-vaapi\"
LIBS:= -lavdevice.dll -lavformat.dll -lavcodec.dll -lavutil.dll -lswscale.dll
LIBS+= -lswresample.dll -lsound -lpixlib3 -lfreetype.dll -lva.dll -lgcc -lc.dll

LDFLAGS = -static --subsystem native --stack 0x200000 -Map fplay.map -Tapp-dynamic.lds --image-base 0

SOURCES = 	opendial.asm			\
		audio.c				\
		decoder.c			\
		fplay.c				\
		vaapi.c				\
		video.c				\
		utils.c				\
		skin/skin.asm			\
		winlib/button.c			\
		winlib/caption.c		\
		winlib/fontlib.c		\
		winlib/frame.c			\
		winlib/panel.c			\
		winlib/window.c
 
OBJECTS =  $(patsubst %.asm, %.o, $(patsubst %.c, %.o, $(SOURCES)))

# targets 

all:$(NAME)

$(NAME): $(OBJECTS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)
	kos32-objcopy $@ -O binary

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.asm Makefile
	$(FASM) $< $@


clean: 
	-rm -f $(OBJECTS)




