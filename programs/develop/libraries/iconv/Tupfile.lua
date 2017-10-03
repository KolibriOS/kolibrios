if tup.getconfig("NO_GCC") ~= "" then return end
tup.rule("iconv.c", "kos32-gcc -c -I../../../../contrib/sdk/sources/newlib/libc/include/ -fno-ident -Os -c -o %o %f " .. tup.getconfig("KPACK_CMD"), "iconv.obj")
