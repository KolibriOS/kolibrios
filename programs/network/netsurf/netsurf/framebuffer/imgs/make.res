OBJS = caret_image.o    history_image_g.o  menu_image.o  pointer_image.o   reload_g.o       scrolld.o  scrollu.o       throbber0.o  throbber3.o  throbber6.o \
hand_image.o     left_arrow.o       move_image.o  progress_image.o  right_arrow.o    scrolll.o  stop_image.o    throbber1.o  throbber4.o  throbber7.o \
history_image.o  left_arrow_g.o     osk_image.o   reload.o          right_arrow_g.o  scrollr.o  stop_image_g.o  throbber2.o  throbber5.o  throbber8.o
  


OUTFILE = TEST.o
CFLAGS += -I ../include/ -I ../ -I../../ -I./ -I/home/sourcerer/kos_src/newenginek/kolibri/include
include $(MENUETDEV)/makefiles/Makefile_for_o_lib

