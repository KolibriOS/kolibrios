include ../osrules.mak

MENUET_LIBC_OBJS :=
MENUET_LIBC_TOPDIR := $(CURDIR)

export MENUET_LIBC_OBJS
export MENUET_LIBC_TOPDIR

all: m_echo$(EXESUFFIX) mks$(EXESUFFIX)
	@echo $(CC) $(LD) $(CFLAGS) $(LDFLAGS)
	cc -v
	ld -v
	make -C libc
	make -C libcon2 mk_lib
	make -C libcpp mk_lib
	make -C libm mk_lib
	make -C libmgfx mk_lib
	make -C libmcoff
	$(RM) m_echo$(EXESUFFIX)
	$(RM) tmp_make
	$(RM) mks$(EXESUFFIX)
	@echo ---------------------------------------------------------
	@echo Libraries for MenuetOS/KolibriOS were successfully built
	@echo ---------------------------------------------------------

dll: m_echo
	make -C libcon2 dll
	make -C libcpp dll
	make -C libm dll
	make -C libmgfx dll

m_echo$(EXESUFFIX): m_echo.c
	gcc m_echo.c -o m_echo$(EXESUFFIX) -s -O2 -fomit-frame-pointer
mks$(EXESUFFIX): mks.c
	gcc mks.c -o mks$(EXESUFFIX) -s -O2 -fomit-frame-pointer

clean:
	make -C libc clean
	make -C libcon2 clean
	make -C libcpp clean
	make -C libm clean
	make -C libmgfx clean
	make -C libmcoff clean
	$(RM) tmp_make m_echo$(EXESUFFIX) mks$(EXESUFFIX)

depend:
	make -C libc depend
	make -C libcon2 depend
	make -C libcpp depend
	make -C libm depend
	make -C libmgfx depend
