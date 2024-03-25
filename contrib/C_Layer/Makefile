staticName = libclayer
ar = kos32-ar

OBJECTS = OBJ/loadboxlib.obj OBJ/loadbuf2d.obj OBJ/loadhttp.obj OBJ/loadkmenu.obj OBJ/loadlibimg.obj OBJ/loadlibini.obj OBJ/loadmsgbox.obj OBJ/loadnetwork.obj OBJ/loadproclib.obj OBJ/loadrasterworks.obj

all: MAKEIT $(staticName).a INSTALL

MAKEIT:
	$(MAKE) -C ASM
$(staticName).a: $(OBJECTS)
	$(ar) rc $(staticName).a $(OBJECTS)
INSTALL:
	mv $(staticName).a ../sdk/lib