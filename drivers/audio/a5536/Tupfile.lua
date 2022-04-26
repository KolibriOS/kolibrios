if tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("NO_GCC") ~= "" then return end
tup.include("../../../programs/use_gcc.lua")

DRV_DIR = "../.."
INCLUDES = string.format(" -I. -I. -I%s/include -I%s/include/linux -I%s/include/uapi ", DRV_DIR, DRV_DIR, DRV_DIR) 

DEFINES = [[ -D__KERNEL__ -DGEODE_LOG="\"/tmp0/1/geode.log"\" -DCONFIG_X86_32 -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE -DCONFIG_PRINTK -DCONFIG_PCI ]]
CFLAGS =  " -Os -fomit-frame-pointer -fno-builtin-printf " .. DEFINES
LDFLAGS = " -nostdlib -shared -s --image-base 0 --file-alignment 512 --section-alignment 4096 -L../../ddk "

NAME = "geode.sys"

compile_gcc{ "geode.c" }
OBJS.extra_inputs = {"../../ddk/<libddk>", "../../ddk/<libcore>"}

LIBS = " -lddk -lcore "
tup.rule(OBJS, "kos32-ld" .. LDFLAGS .. "%f -o %o " .. LIBS .. tup.getconfig("KPACK_CMD"), NAME);
tup.rule("geode.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "geode")
