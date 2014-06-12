include $(MENUETDEV)/osrules.mak
ifdef ON_MINGW
THIS_SRCS = new.cpp delete.cpp net\ip_addr.cpp net\udp_socket.cpp
else
THIS_SRCS = new.cpp delete.cpp net/ip_addr.cpp net/udp_socket.cpp
endif

include $(MENUET_LIBC_TOPDIR)/Make.rules

mk_lib: gen_tmp all
	make -f Makefile-link OUTFILE="libcpp.a"
ifdef ON_MINGW
	copy libcpp.a $(MENUETDEV)\lib
	del libcpp.a
else
	mv -f libcpp.a $(MENUETDEV)/lib
endif

dll: _gen_tmp all
	make -f Makefile-link-dll OUTFILE="cpp-glue.so"
ifdef ON_MINGW
	copy cpp-glue.so $(MENUETDEV)\lib
	del cpp-glue.so
else
	mv cpp-glue.so $(MENUETDEV)/lib
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
