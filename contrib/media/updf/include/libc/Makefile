all:
	make -C linuxtools
	make -C linuxtools install
	make -C src
	make -C stub
	make -C programs

clean:
	make -C src clean
	make -C linuxtools clean
	make -C stub clean
	make -C programs clean
