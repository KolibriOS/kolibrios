if tup.getconfig("NO_GCC") ~= "" then return end
tup.rule("base64.c", "kos32-gcc -fno-ident -Os -c -o %o %f " .. tup.getconfig("KPACK_CMD"), "base64.obj")
