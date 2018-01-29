#include <stdlib.h>
#include <stdio.h>
#ifndef KOS32
#include <time.h>
#else
#include <kos32sys1.h>
#endif
#ifndef WINDOWS
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

void randomize(void) {
#ifndef KOS32
	srand((int)time(NULL));
#else
	srand(get_tick_count());
#endif
}

///#ifndef WINDOWS
int max(int a, int b) {
	if (a > b) return a;
	return b;
}

int min(int a, int b) {
	if (a < b) return a;
	return b;
}
#ifndef WINDOWS
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