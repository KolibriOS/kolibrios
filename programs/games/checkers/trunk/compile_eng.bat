del *.o

echo #define LANG_ENG 1 > lang.h
     
g++ -c checkers.cpp
g++ -L/usr/X11R6/lib -lX11 -o checkers checkers.o
pause