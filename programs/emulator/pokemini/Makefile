OUTFILE = pokemini
OBJS = $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(OBJS7) $(OBJS8) $(OBJS9)
OBJS1 = PokeMini_SDL.o ../../sourcex/KeybMapSDL.o ../../sourcex/ExportBMP.o ../../sourcex/ExportWAV.o ../../source/PMCommon.o ../../source/PokeMini.o
OBJS2 = ../../source/Multicart.o ../../source/Hardware.o ../../source/Video.o ../../source/Video_x1.o ../../source/Video_x2.o ../../source/Video_x3.o
OBJS3 = ../../source/Video_x4.o ../../source/Video_x5.o ../../source/Video_x6.o ../../source/CommandLine.o ../../source/MinxCPU.o
OBJS4 = ../../source/MinxCPU_XX.o ../../source/MinxCPU_CE.o ../../source/MinxCPU_CF.o ../../source/MinxCPU_SP.o ../../source/MinxTimers.o
OBJS5 = ../../source/MinxIO.o ../../source/MinxIRQ.o ../../source/MinxPRC.o ../../source/MinxColorPRC.o ../../source/MinxLCD.o
OBJS6 = ../../source/MinxAudio.o ../../source/UI.o ../../source/Joystick.o ../../source/Keyboard.o 
OBJS7 = ../../resource/PokeMini_ColorPal.o ../../resource/PokeMini_Font12.o ../../resource/PokeMini_Icons12.o ../../resource/PokeMini_BG2.o
OBJS8 = ../../resource/PokeMini_BG3.o ../../resource/PokeMini_BG4.o ../../resource/PokeMini_BG5.o ../../resource/PokeMini_BG6.o
OBJS9 = ./freebios/freebios.o

CFLAGS = -Os -fomit-frame-pointer -DPERFORMANCE -DNO_ZIP -DNO_SCANDIRS -I$(MENUETDEV)/include/SDL -I./ -I./../../source -I./../../sourcex -I./../../resource -I../../dependencies/minizip -I../../freebios
LIBS = -lSDL -lm 
include $(MENUETDEV)/makefiles/Makefile_for_program
