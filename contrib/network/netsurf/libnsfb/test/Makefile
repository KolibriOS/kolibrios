# Sources
OBJS := bitmap.o nsglobe.o


OUTFILE = bitmap

CFLAGS += -I ../../include/ -I ../../ -I ../ -I ./ -I /home/sourcerer/kos_src/newenginek/kolibri/include -I /home/sourcerer/kos_src/me/include/SDL/
LIBS += -L /home/sourcerer/kos_src/newenginek/kolibri/lib  -lm    --whole-archive -lnsfb   --no-whole-archive -lSDL
include $(MENUETDEV)/makefiles/Makefile_for_program
