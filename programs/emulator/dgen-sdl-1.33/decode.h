/* Header file for decode functions */
#ifndef __GENIE_DECODE_H__
#define __GENIE_DECODE_H__

struct patch { unsigned int addr, data; };

#ifdef __cplusplus
extern "C" {
#endif

void genie_decode(const char *code, struct patch *result);
void hex_decode(const char *code, struct patch *result);
void decode(const char *code, struct patch *result);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __GENIE_DECODE_H__
