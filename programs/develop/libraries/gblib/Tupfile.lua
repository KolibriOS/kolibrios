if tup.getconfig("NO_GCC") ~= "" then return end
tup.rule("gblib.c", "kos32-gcc -fno-ident -Os -c -o %o %f " .. tup.getconfig("KPACK_CMD"), "gblib.obj")
