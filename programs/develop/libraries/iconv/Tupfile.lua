if tup.getconfig("NO_GCC") ~= "" then return end
tup.rule("iconv.c", "kos32-gcc -Os -fno-ident -fno-leading-underscore -fno-pie -c -o %o %f && kos32-strip %o --strip-unneeded " ..  tup.getconfig("KPACK_CMD"), "iconv.obj")
