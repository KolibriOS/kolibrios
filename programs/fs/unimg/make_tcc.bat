@echo #define COMPILLER TCC > compiller.h
kos32-tcc fat12.c -lck -o unimg
@del compiller.h
@pause