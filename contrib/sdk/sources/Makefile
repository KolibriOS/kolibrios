
DLL_DIR:= $(abspath ../bin)

LIB_DIR:= $(abspath ../lib)

# targets 

all: 	newlib zlib libpng freetype pixman cairo	\
	libdrm pixlib ffmpeg libsupc++ gcc_eh expat 	\
	Mesa eglut		

newlib: $(DLL_DIR)/libc.dll $(LIB_DIR)/libc.dll.a $(LIB_DIR)/libapp.a $(LIB_DIR)/libdll.a

$(DLL_DIR)/libc.dll $(LIB_DIR)/libc.dll.a $(LIB_DIR)/libapp.a $(LIB_DIR)/libdll.a:
	$(MAKE) -C newlib/libc shared 
	$(MAKE) -C newlib/libc install

#####################################

zlib: $(DLL_DIR)/libz.dll $(LIB_DIR)/libz.a $(LIB_DIR)/libz.dll.a
   
$(DLL_DIR)/libz.dll $(LIB_DIR)/libz.a $(LIB_DIR)/libz.dll.a: newlib
	$(MAKE) -C zlib 

#####################################

libpng: $(DLL_DIR)/libpng16.dll $(LIB_DIR)/libpng16.dll.a $(LIB_DIR)/libpng16.a

$(DLL_DIR)/libpng16.dll $(LIB_DIR)/libpng16.dll.a $(LIB_DIR)/libpng16.a: newlib zlib
	$(MAKE) -C libpng

#####################################

freetype: $(DLL_DIR)/freetype.dll $(LIB_DIR)/libfreetype.dll.a $(LIB_DIR)/libfreetype.a

$(DLL_DIR)/freetype.dll $(LIB_DIR)/libfreetype.dll.a $(LIB_DIR)/libfreetype.a: newlib
	$(MAKE) -C freetype

#####################################

pixman: $(DLL_DIR)/pixman-1.dll $(LIB_DIR)/libpixman-1.dll.a $(LIB_DIR)/libpixman-1.a

$(DLL_DIR)/pixman-1.dll $(LIB_DIR)/libpixman-1.dll.a $(LIB_DIR)/libpixman-1.a: newlib
	$(MAKE) -C pixman

#####################################

cairo: $(DLL_DIR)/cairo2.dll $(LIB_DIR)/libcairo2.dll.a $(LIB_DIR)/libcairo2.a

$(DLL_DIR)/cairo2.dll $(LIB_DIR)/libcairo2.dll.a $(LIB_DIR)/libcairo2.a:\
	newlib freetype zlib pixman libpng
	$(MAKE) -C cairo

#####################################
	 
libdrm: $(DLL_DIR)/libdrm.dll $(LIB_DIR)/libdrm.dll.a $(LIB_DIR)/libdrm.a

$(DLL_DIR)/libdrm.dll $(LIB_DIR)/libdrm.dll.a $(LIB_DIR)/libdrm.a: newlib
	$(MAKE) -C libdrm 

#####################################

pixlib: $(DLL_DIR)/pixlib.dll

$(DLL_DIR)/pixlib.dll: newlib libdrm
	$(MAKE) -C Intel-2D

#####################################

ffmpeg: $(DLL_DIR)/avcodec-55.dll $(DLL_DIR)/avdevice-55.dll \
	$(DLL_DIR)/avformat-55.dll $(DLL_DIR)/avutil-52.dll \
	$(DLL_DIR)/swresample-0.dll $(DLL_DIR)/swscale-2.dll \
	$(LIB_DIR)/libavcodec.dll.a $(LIB_DIR)/libavdevice.dll.a \
	$(LIB_DIR)/libavformat.dll.a $(LIB_DIR)/libavutil.dll.a \
	$(LIB_DIR)/libswresample.dll.a $(LIB_DIR)/libswscale.dll.a

$(DLL_DIR)/avcodec-55.dll $(DLL_DIR)/avdevice-55.dll \
	$(DLL_DIR)/avformat-55.dll $(DLL_DIR)/avutil-52.dll \
	$(DLL_DIR)/swresample-0.dll $(DLL_DIR)/swscale-2.dll \
	$(LIB_DIR)/libavcodec.dll.a $(LIB_DIR)/libavdevice.dll.a \
	$(LIB_DIR)/libavformat.dll.a $(LIB_DIR)/libavutil.dll.a \
	$(LIB_DIR)/libswresample.dll.a $(LIB_DIR)/libswscale.dll.a : newlib
	$(MAKE) -C ffmpeg

#####################################

libsupc++: $(LIB_DIR)/libsupc++.a

$(LIB_DIR)/libsupc++.a: newlib
	$(MAKE) -C libsupc++

#####################################

expat: $(LIB_DIR)/libexpat.a

$(LIB_DIR)/libexpat.a: newlib
	$(MAKE) -C expat

#####################################

gcc_eh: $(LIB_DIR)/libgcc_eh.a

$(LIB_DIR)/libgcc_eh.a: newlib
	$(MAKE) -C gcc_eh

#####################################

Mesa: $(DLL_DIR)/libGL.dll $(DLL_DIR)/libegl.dll $(DLL_DIR)/i965_dri.drv

$(DLL_DIR)/libGL.dll $(DLL_DIR)/libegl.dll $(DLL_DIR)/i965_dri.drv \
	$(LIB_DIR)/libGL.dll.a $(LIB_DIR)/libegl.dll.a \
	$(LIB_DIR)/libglsl.a: newlib libdrm libsupc++ gcc_eh expat
	$(MAKE) -C Mesa

#####################################

eglut: $(DLL_DIR)/libeglut.dll $(LIB_DIR)/libeglut.dll.a $(LIB_DIR)/libeglut.a

$(DLL_DIR)/libeglut.dll $(LIB_DIR)/libeglut.dll.a $(LIB_DIR)/libeglut.a : newlib Mesa
	$(MAKE) -C eglut
 