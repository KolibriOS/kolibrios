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
	uint64_t ns = _ksys_get_ns_count();
	unsigned lo = (unsigned)ns;
	unsigned rem = (unsigned)(ns >> 32) % 1000000;
	// exact low 32 bits of ns/1000000, without libtcc1's 64-bit divide:
	// quotient of (hi%d):lo by d always fits in 32 bits
	asm_inline("divl %2" : "+a"(lo), "+d"(rem) : "r"(1000000u));
	return lo;
}
