checkers : checkers.o
		g++ -L/usr/X11R6/lib -lX11 -o checkers checkers.o
checkers.o : checkers.cpp board.h position.h player.h tmplayer.h buttons.h history.h hash.h gr-draw.h gnu-draw.h sysproc.h
		g++ -c checkers.cpp
