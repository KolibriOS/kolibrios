KGCC = ../linuxtools/kgcc
KLD = ../linuxtools/kld
KPACK = kpack
KTCC=../../../programs/develop/ktcc/trunk/bin/kos32-tcc 
KTCC_FLAGS = -nostdlib -stack=1048576 -I$(KLIBC)/source/include -L$(KLIBC)/lib $(KLIBC)/lib/crt0.o 

ifndef NAME
NAME = basic_gui
endif

static_kgcc:
	$(KGCC) $(NAME).c
	$(KLD) $(NAME).o -o $(NAME) -lc
	$(KPACK) $(NAME)

static_tcc:
	$(KTCC) $(KTCC_FLAGS) $(NAME).c -o $(NAME) -lc
	$(KPACK) $(NAME)

dynamic_kgcc:
	$(KGCC) -D_DYNAMIC $(NAME).c
	$(KLD) $(NAME).o -o $(NAME) -lc.obj
	$(KPACK) $(NAME)

dynamic_tcc:
	$(KTCC) $(KTCC_FLAGS) -D_DYNAMIC $(NAME).c -o $(NAME)_dyn  -lc.obj
	$(KPACK) $(NAME)_dyn
