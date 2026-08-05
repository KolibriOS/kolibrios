#include "misc.h"

int getRandomNumber(int _min, int _max) {
	return rand() % (_max - _min + 1) + _min;
}

// Zero-padded decimal, keeps only the last ndigits (num is never negative here)
void intToStr(int num, int ndigits, char* result) {
	result[ndigits] = '\0';
	for (int i = ndigits - 1; i >= 0; i--) {
		result[i] = '0' + num % 10;
		num /= 10;
	}
}

int getTimeStamp() { // in ms
	uint64_t x = 0;
	x = _ksys_get_ns_count();
	return (x/1000000);
}
