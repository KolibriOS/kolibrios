if tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("NO_GCC") ~= "" then return end
tup.include("../../programs/use_gcc.lua")

DRV_DIR = ".."
INCLUDES = string.format(" -I./include  -I./acpica/include -I. -I%s/include -I%s/include/linux -I%s/include/uapi ", DRV_DIR, DRV_DIR, DRV_DIR) 

DEFINES = " -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE -DHAVE_ACPICA -DCONFIG_ACPI -DLINUX_MOD_DEVICETABLE_H -DCONFIG_PCI "

CFLAGS = " -w -Os -march=i686 -fno-ident -msse2 -fomit-frame-pointer -fno-builtin-printf -mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -mno-ms-bitfields " .. DEFINES 

LDFLAGS = " -nostdlib -T acpi.lds -shared -s --image-base 0 --file-alignment 512 --section-alignment 4096 -L../../contrib/sdk/lib -L../ddk -Lacpica "

NAME = "acpi.sys"

compile_gcc{ "acpi.c", "scan.c", "pci_root.c", "pci_bind.c", "pci_irq.c", "pci/probe.c", "pci/pci.c", "pci/access.c" }
OBJS.extra_inputs = {"../ddk/<libcore>", "../ddk/<libddk>", "./acpica/libacpica.a"}

LIBS = " -lacpica -lgcc -lddk -lcore "
tup.rule(OBJS, "kos32-ld" .. LDFLAGS .. "%f -o %o " .. LIBS .. tup.getconfig("KPACK_CMD"), NAME);
tup.rule("acpi.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "acpi")
