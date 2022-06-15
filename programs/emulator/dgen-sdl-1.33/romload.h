#ifndef ROMLOAD_H_
#define ROMLOAD_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define ROMLOAD_DECL_BEGIN__ extern "C" {
#define ROMLOAD_DECL_END__ }
#else
#define ROMLOAD_DECL_BEGIN__
#define ROMLOAD_DECL_END__
#endif

ROMLOAD_DECL_BEGIN__

extern uint8_t *load_rom(size_t *rom_size, const char *name);
extern void unload_rom(uint8_t *rom);
extern void set_rom_path(const char *path);

ROMLOAD_DECL_END__

#endif /* ROMLOAD_H_ */
