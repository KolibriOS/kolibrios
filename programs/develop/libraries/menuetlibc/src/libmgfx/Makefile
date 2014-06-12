THIS_SRCS = format.c load_image.c readjpeg.c libinit.c
include $(MENUET_LIBC_TOPDIR)/Make.rules

ifdef ON_MINGW
mk_lib: gen_tmp all
	make -f Makefile-link OUTFILE="libmgfx.a"
	copy libmgfx.a $(MENUETDEV)\lib
	del libmgfx.a

dll: _gen_tmp all
	make -f Makefile-link-dll OUTFILE="mgfx.so"
	copy mgfx.so $(MENUETDEV)\lib
	del mgfx.so

_gen_tmp:
	@$(D_ECHO) > ..\tmp_make

gen_tmp:
	@echo foo = bar> ..\tmp_make
	@..\m_echo ..\tmp_make B_MENUET_LIBC_OBJS = 

else

mk_lib: gen_tmp all
	make -f Makefile-link OUTFILE="libmgfx.a"
	mv -f libmgfx.a $(MENUETDEV)/lib

dll: _gen_tmp all
	make -f Makefile-link-dll OUTFILE="mgfx.so"
	mv -f mgfx.so $(MENUETDEV)/lib

_gen_tmp:
	@$(D_ECHO) > ../tmp_make

gen_tmp:
	@echo "foo = bar" > ../tmp_make
	@../m_echo ../tmp_make B_MENUET_LIBC_OBJS = 
endif
