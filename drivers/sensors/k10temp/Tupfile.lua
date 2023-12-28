if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")

CFLAGS =[[ -std=gnu99 -Os -march=i686 -fno-ident -msse2 -fomit-frame-pointer -fno-builtin-printf -mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -mno-ms-bitfields -UWIN32 -U_WIN32 -U__WIN32__ -D_KOLIBRI -DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_DMI -DCONFIG_TINY_RCU  -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE -DCONFIG_PRINTK -DCONFIG_PCI -DCONFIG_PCI  -DCONFIG_AMD_NB -DKBUILD_MODNAME="\"k10temp"\" -I../../include -I../../include/asm -I../../include/uapi -I../../include/drm ]]

LDFLAGS = " -nostdlib -shared -s --major-os-version 0 --minor-os-version 7 --major-subsystem-version 0 --minor-subsystem-version 5 --subsystem native  -T../drv.lds --image-base 0 --file-alignment 512 --section-alignment 4096 -L../../../contrib/sdk/lib -L../../ddk "

LIBS = " -lddk -lcore -lgcc "

compile_gcc{ 
    "k10temp.c",  "../pci.c",  "../amd_nb.c", "../cpu_detect.c"
}

OBJS.extra_inputs = {"../../ddk/<libcore>", "../../ddk/<libddk>"}

tup.rule(OBJS, "kos32-ld" .. LDFLAGS .. "%f -o %o " .. LIBS .. tup.getconfig("KPACK_CMD"), "k10temp.sys");
