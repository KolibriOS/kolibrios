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
	// No 64-bit C arithmetic here: any long long op would reference
	// libtcc1.o, and that links the whole 3.5K library into the binary.
	unsigned lo, hi;
	// sysfn 26.10: nanosecond counter in edx:eax
	asm_inline("int $0x40" : "=a"(lo), "=d"(hi) : "a"(26), "b"(10));
	// exact low 32 bits of ns/1000000: the quotient of (hi%d):lo by d
	// always fits in 32 bits
	hi %= 1000000;
	asm_inline("divl %2" : "+a"(lo), "+d"(hi) : "r"(1000000u));
	return lo;
}
