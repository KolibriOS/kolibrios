include osrules.mak
all: genconfig
	@echo Building library for $(VERSION_OS)
	gcc mkversion.c -o mkversion$(EXESUFFIX)
ifdef ON_MINGW
	mkversion$(EXESUFFIX) < version > include/libc/version.h
else
	./mkversion$(EXESUFFIX) < version > include/libc/version.h
endif
	make -C linuxtools
	make -C src depend
	make -C src
	make -C stub
	make -C programs

genconfig:
ifdef ON_MINGW
	@echo #$(NEED_UNDERSCORES) NEEDS_UNDERSCORES> config.h
else
	@echo "#$(NEED_UNDERSCORES) NEEDS_UNDERSCORES" > config.h
endif

clean:
	make -C src clean
	make -C linuxtools clean
	make -C stub clean
	make -C programs clean
	$(RM) mkversion$(EXESUFFIX)
