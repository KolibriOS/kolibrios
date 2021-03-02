KGCC = ../linuxtools/kgcc
KLD = ../linuxtools/kld
KPACK = kpack
KTCC=../../../programs/develop/ktcc/trunk/bin/kos32-tcc
KTCC_FLAGS = -nostdlib -I$(KLIBC)/source/include -L$(KLIBC)/lib $(KLIBC)/lib/crt0.o

static_kgcc:
	$(KGCC) basic_gui.c
	$(KLD) basic_gui.o -o basic_gui -lc 
	$(KPACK) basic_gui

static_tcc:
	$(KTCC) $(KTCC_FLAGS) basic_gui.c -o basic_gui -lc
	$(KPACK) basic_gui

dynamic_tcc:
	$(KTCC) $(KTCC_FLAGS) -D_DYNAMIC basic_gui.c -o basic_gui_dyn -lc.obj
	$(KPACK) basic_gui_dyn
