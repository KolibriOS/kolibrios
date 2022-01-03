#include <stdint.h>
#include <string.h>

inline void littleBigEndian (void *x, int sz) {
	unsigned char *toConvert = (unsigned char *)(x);
	unsigned char tmp;
	for (size_t i = 0; i < sz/2; ++i) {
		tmp = toConvert[i];
		toConvert[i] = toConvert[sz - i - 1];
		toConvert[sz - i - 1] = tmp;
	}
}

inline void BE16(uint16_t* w) {littleBigEndian(w, 2);}
inline void BE32(uint32_t* i) {littleBigEndian(i, 4);}