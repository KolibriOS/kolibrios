THIS_SRCS = textcon.c

include $(MENUET_LIBC_TOPDIR)/Make.rules

mk_lib: gen_tmp all
	make -f Makefile-link OUTFILE="libcon2.a"
ifdef ON_MINGW
	copy libcon2.a $(MENUETDEV)\lib
	del libcon2.a
	copy textcon.h $(MENUETDEV)\include\menuet
else
	mv -f libcon2.a $(MENUETDEV)/lib
	cp textcon.h $(MENUETDEV)/include/menuet
endif

dll: _gen_tmp all
	make -f Makefile-link-dll OUTFILE="con-2.so"
ifdef ON_MINGW
	copy libcon2.a $(MENUETDEV)\lib
	del libcon2.a
else
	mv con-2.so $(MENUETDEV)/lib
endif

_gen_tmp:
	@$(D_ECHO) > ../tmp_make

gen_tmp:
ifdef ON_MINGW
	@echo foo = bar> ..\tmp_make
	@..\m_echo ..\tmp_make B_MENUET_LIBC_OBJS = 
else
	@echo "foo = bar" > ../tmp_make
	@../m_echo ../tmp_make B_MENUET_LIBC_OBJS = 
endif
