SUBDIRS = assert ctype errno locale math setjmp stdio stdlib string time

all: process_subdirs

process_subdirs:
	make -C assert
	make -C ctype
	make -C errno
	make -C locale
	make -C math
	make -C setjmp
	make -C stdio
	make -C stdlib
	make -C string
	make -C time


clean:
	make -C assert clean
	make -C ctype clean
	make -C errno clean
	make -C locale clean
	make -C math clean
	make -C setjmp clean
	make -C stdio clean
	make -C stdlib clean
	make -C string clean
	make -C time clean

depend:
	make -C assert depend
	make -C ctype depend
	make -C errno depend
	make -C locale depend
	make -C math depend
	make -C setjmp depend
	make -C stdio depend
	make -C stdlib depend
	make -C string depend
	make -C time depend
