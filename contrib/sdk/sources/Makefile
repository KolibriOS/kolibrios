
DLL_DIR:= $(abspath ../bin)

LIB_DIR:= $(abspath ../lib)

# targets 

all: 	newlib zlib libpng freetype pixman cairo \
	libdrm pixlib		

newlib: $(DLL_DIR)/libc.dll $(LIB_DIR)/libc.dll.a $(LIB_DIR)/libapp.a $(LIB_DIR)/libdll.a

$(DLL_DIR)/libc.dll $(LIB_DIR)/libapp.a $(LIB_DIR)/libdll.a:
	$(MAKE) -C newlib shared 
	$(MAKE) -C newlib install

#####################################

zlib: $(DLL_DIR)/libz.dll $(LIB_DIR)/libz.a $(LIB_DIR)/libz.dll.a
   
$(DLL_DIR)/libz.dll $(LIB_DIR)/libz.a $(LIB_DIR)/libz.dll.a: $(LIB_DIR)/libdll.a $(LIB_DIR)/libc.dll.a
	$(MAKE) -C zlib 

#####################################

libpng: $(DLL_DIR)/libpng16.dll $(LIB_DIR)/libpng16.dll.a $(LIB_DIR)/libpng16.a

$(DLL_DIR)/libpng16.dll $(LIB_DIR)/libpng16.dll.a $(LIB_DIR)/libpng16.a:\
	$(LIB_DIR)/libdll.a $(LIB_DIR)/libc.dll.a $(LIB_DIR)/libz.dll.a
	$(MAKE) -C libpng

#####################################

freetype: $(DLL_DIR)/freetype.dll $(LIB_DIR)/libfreetype.dll.a $(LIB_DIR)/libfreetype.a

$(DLL_DIR)/freetype.dll $(LIB_DIR)/libfreetype.dll.a $(LIB_DIR)/libfreetype.a:\
	$(LIB_DIR)/libdll.a $(LIB_DIR)/libc.dll.a
	$(MAKE) -C freetype

#####################################

pixman: $(DLL_DIR)/pixman-1.dll $(LIB_DIR)/libpixman-1.dll.a $(LIB_DIR)/libpixman-1.a

$(DLL_DIR)/pixman-1.dll $(LIB_DIR)/libpixman-1.dll.a $(LIB_DIR)/libpixman-1.a:\
	$(LIB_DIR)/libdll.a $(LIB_DIR)/libc.dll.a
	$(MAKE) -C pixman

#####################################

cairo: $(DLL_DIR)/cairo2.dll $(LIB_DIR)/libcairo2.dll.a $(LIB_DIR)/libcairo2.a

$(DLL_DIR)/cairo2.dll $(LIB_DIR)/libcairo2.dll.a $(LIB_DIR)/libcairo2.a:\
	newlib freetype zlib pixman libpng
	$(MAKE) -C cairo

#####################################
	 
libdrm: $(DLL_DIR)/libdrm.dll $(LIB_DIR)/libdrm.dll.a $(LIB_DIR)/libdrm.a

$(DLL_DIR)/libdrm.dll $(LIB_DIR)/libdrm.dll.a $(LIB_DIR)/libdrm.a: $(LIB_DIR)/libdll.a $(LIB_DIR)/libc.dll.a
	$(MAKE) -C libdrm 

#####################################

pixlib: $(DLL_DIR)/pixlib.dll

$(DLL_DIR)/pixlib.dll: $(LIB_DIR)/libdll.a $(LIB_DIR)/libc.dll.a $(LIB_DIR)/libdrm.dll.a
	$(MAKE) -C Intel-2D

 