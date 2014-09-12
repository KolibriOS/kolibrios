if tup.getconfig("NO_GCC") ~= "" then return end
tup.rule("qs.c", "kos32-gcc -fno-ident -Os -c -o %o %f " .. tup.getconfig("KPACK_CMD"), "qs.obj")
