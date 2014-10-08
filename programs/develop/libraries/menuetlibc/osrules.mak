EXESUFFIX =
RM = rm -f
MV = mv
D_ECHO = echo
LIBDIR = $(MENUETDEV)/lib

MGCC=kos32-gcc -c -Os -nostdinc -fno-builtin -I$(MENUETDEV)/include -fno-common -D__DEV_CONFIG_H="\"../../config.h\"" -D__MENUETOS__
MGPP=kos32-g++ -c -Os -nostdinc -fno-builtin -I$(MENUETDEV)/include -fno-common -I$(MENUETDEV)/include/STL -D__MENUETOS__ -fno-rtti -fno-exceptions -fomit-frame-pointer
MLD=kos32-ld -T$(MENUETDEV)/include/scripts/menuetos_app_v01.ld -nostdlib -L$(MENUETDEV)/lib $(MENUETDEV)/stub/crt0.o
MMKDEP=kos32-gcc -nostdinc -I$(MENUETDEV)/include -D__DEV_CONFIG_H="\"../../config.h\"" -M
