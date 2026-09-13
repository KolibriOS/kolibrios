#ifndef _LINUX_EFI_H
#define _LINUX_EFI_H
/* KolibriOS boots via BIOS only; amdgpu only tests efi_enabled(). */
#define EFI_BOOT		0
#define EFI_RUNTIME_SERVICES	1
static inline bool efi_enabled(int feature) { return false; }
#endif
