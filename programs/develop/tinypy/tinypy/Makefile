export MENUETDEV=../../libraries/menuetlibc
OUTFILE = tpmain
OBJS = tpmain.o kolibri_init.o kolibri_fs.o kolibri_gui.o kolibri_dbg.o
CFLAGS = -I. -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2
include $(MENUETDEV)/makefiles/Makefile_for_program
kolibri_dbg.o: fasm_modules/kolibri_dbg.s
	fasm fasm_modules/kolibri_dbg.s
	cp fasm_modules/kolibri_dbg.o .
