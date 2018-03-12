#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef WINDOWS
#ifndef KOS32
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif
#endif

void randomize(void) {
	srand((int)time(NULL));
}

#if !defined(KOS32) && !defined(__TINYC__)
int max(int a, int b) {
	if (a > b) return a;
	return b;
}

int min(int a, int b) {
	if (a < b) return a;
	return b;
}
#endif

#ifndef WINDOWS
#ifndef KOS32
int // <editor-fold defaultstate="collapsed" desc="comment">
getch// </editor-fold>
(void) {
	char chbuf[1];
    struct termios oldstate, newstate;
    fflush(stdout);
	tcgetattr(0, &oldstate);
	newstate = oldstate;
	newstate.c_lflag &= ~ICANON;
	newstate.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW,  &newstate);
	read(0, &chbuf, 1);
	tcsetattr(0, TCSANOW, &oldstate);
        return chbuf[0];
}
#endif
#endif