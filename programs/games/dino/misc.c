#include "misc.h"

int getRandomNumber(int _min, int _max) {
	return rand() % (_max - _min + 1) + _min;
}

void intToStr(int num, int ndigits, char* result) {
	char num_str[16]; // 16 more than enough for int
	sprintf(num_str, "%d", num); // Convert num to a string
	if (strlen(num_str) > ndigits) {
		// Copy only the last ndigits to result
		strcpy(result, num_str + strlen(num_str) - ndigits);
	}
	else {
		// Pad the string with leading zeros until it reaches a length of ndigits
		size_t z = ndigits - strlen(num_str);
		for (size_t i = 0; i < z; i++) {
			result[i] = '0';
		}
		strcpy(result + z, num_str);
	}
}

int getTimeStamp() { // in ms
	uint64_t x = 0;
	x = _ksys_get_ns_count();
	return (x/1000000);
}
