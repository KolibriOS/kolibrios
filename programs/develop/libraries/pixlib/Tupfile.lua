if tup.getconfig("NO_GCC") ~= "" then return end
tup.rule("pixlib.c", "kos32-gcc -march=pentium-mmx -fno-ident -Os -c -o %o %f " .. tup.getconfig("KPACK_CMD"), "pixlib.obj")
