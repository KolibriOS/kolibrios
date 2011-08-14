export MENUETDEV=../../libraries/menuetlibc
OUTFILE = tpmain
OBJS = tpmain.o kolibri_init.o kolibri_fs.o kolibri_gui.o kolibri_dbg.o
CFLAGS = -I.
include $(MENUETDEV)/makefiles/Makefile_for_program
kolibri_dbg.o: fasm_modules/kolibri_dbg.s
	fasm fasm_modules/kolibri_dbg.s
	cp fasm_modules/kolibri_dbg.o .
