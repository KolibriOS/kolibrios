include $(MENUETDEV)/osrules.mak

all: process_subdirs
	make -f Makefile-link OUTFILE="libc.a"
ifdef ON_MINGW
	copy libc.a $(MENUETDEV)\lib
	del libc.a
else
	mv libc.a $(MENUETDEV)/lib
endif

process_subdirs:
	@$(D_ECHO) > ../tmp_make
	make -C ansi
	make -C ansif
	make -C compat
	make -C crt0
	make -C dos
	make -C fsext
	make -C menuetos
	make -C net
	make -C pc_hw
	make -C posix
	make -C termios
	make -C emu_layer

clean:
	make -C ansi clean
	make -C ansif clean
	make -C compat clean
	make -C crt0 clean
	make -C dos clean
	make -C fsext clean
	make -C menuetos clean
	make -C net clean
	make -C pc_hw clean
	make -C posix clean
	make -C termios clean
	make -C emu_layer clean

depend:
	make -C ansi depend
	make -C ansif depend
	make -C compat depend
	make -C crt0 depend
	make -C dos depend
	make -C fsext depend
	make -C menuetos depend
	make -C net depend
	make -C pc_hw depend
	make -C posix depend
	make -C termios depend
	make -C emu_layer depend
