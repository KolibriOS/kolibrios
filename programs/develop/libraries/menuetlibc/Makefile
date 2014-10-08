include osrules.mak
all:
	gcc mkversion.c -o mkversion$(EXESUFFIX)
	./mkversion$(EXESUFFIX) < version > include/libc/version.h
	make -C src depend
	make -C src
	make -C stub

clean:
	make -C src clean
	make -C stub clean
	$(RM) mkversion$(EXESUFFIX)
