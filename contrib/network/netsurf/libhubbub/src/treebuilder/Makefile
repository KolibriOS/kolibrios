# Sources
OBJS := treebuilder.o \
		initial.o before_html.o before_head.o in_head.o \
		in_head_noscript.o after_head.o in_body.o in_table.o \
		in_caption.o in_column_group.o in_table_body.o in_row.o \
		in_cell.o in_select.o in_select_in_table.o \
		in_foreign_content.o after_body.o in_frameset.o \
		after_frameset.o after_after_body.o after_after_frameset.o \
		generic_rcdata.o
OUTFILE = libo.o

CFLAGS += -I ../../include/ -I ../../ -I ../ -I ./ -I /home/sourcerer/kos_src/newenginek/kolibri/include
include $(MENUETDEV)/makefiles/Makefile_for_o_lib

