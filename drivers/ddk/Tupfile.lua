if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")

CFLAGS =[[ -c -Os -march=i686 -fomit-frame-pointer -fno-builtin-printf -mno-stack-arg-probe -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -fno-ident -UWIN32 -U_WIN32 -U__WIN32__ -D_KOLIBRI -DKOLIBRI -D__KERNEL__ -DCONFIG_X86_32 -DCONFIG_DMI -DCONFIG_TINY_RCU -DCONFIG_X86_L1_CACHE_SHIFT=6 -DCONFIG_ARCH_HAS_CACHE_LINE_SIZE -DCONFIG_PRINTK -I../include -I../include/asm -I../include/uapi -I../include/drm ]]

LDFLAGS = " -nostdlib -shared -s --major-os-version 0 --minor-os-version 7 --major-subsystem-version 0 --minor-subsystem-version 5 --subsystem native  -T../drv.lds --image-base 0 --file-alignment 512 --section-alignment 4096 -L../../../contrib/sdk/lib -L../../ddk "


DDK_SRC = {
    "debug/dbglog.c",
    "dma/dma_alloc.c",
    "dma/fence.c",
    "io/create.c",
    "io/finfo.c",
    "io/ssize.c",
    "io/write.c",
    "linux/bitmap.c",
    "linux/ctype.c",
    "linux/div64.c",
    "linux/dmapool.c",
    "linux/dmi.c",
    "linux/fbsysfs.c",
    "linux/find_next_bit.c",
    "linux/firmware.c",
    "linux/gcd.c",
    "linux/hdmi.c",
    "linux/hexdump.c",
    "linux/idr.c",
    "linux/interval_tree.c",
    "linux/kasprintf.c",
    "linux/kmap.c",
    "linux/list_sort.c",
    "linux/mutex.c",
    "linux/rbtree.c",
    "linux/scatterlist.c",
    "linux/string.c",
    "linux/time.c",
    "linux/workqueue.c",
    "linux/msr.c",
    "malloc/malloc.c",
    "stdio/vsprintf.c",
    "string/strstr.c",
    "string/_memmove.S",
    "string/_strncat.S",
    "string/_strncmp.S",
    "string/_strncpy.S",
    "string/_strnlen.S",
    "string/bcmp.S",
    "string/bcopy.S",
    "string/bzero.S",
    "string/index.S",
    "string/memchr.S",
    "string/memcmp.S",
    "string/memcpy.S",
    "string/memmove.S",
    "string/memset.S",
    "string/rindex.S",
    "string/strcat.S",
    "string/strchr.S",
    "string/strcmp.S",
    "string/strcpy.S",
    "string/strlen.S",
    "string/strncat.S",
    "string/strncmp.S",
    "string/strncpy.S",
    "string/strnlen.S",
    "string/strrchr.S",
    "debug/chkstk.S"
}

compile_gcc(DDK_SRC)
tup.rule(OBJS, "kos32-ar -crs %o %f", {"libddk.a", extra_outputs={"<libddk>"}});
tup.rule("core.S", "kos32-as %f -o %o", "core.o");
tup.rule("core.o", "kos32-ld -shared -s --out-implib %o --output-def core.def -o core.dll %f", {"libcore.a", extra_outputs={"core.def", "core.dll", "<libcore>"}});
