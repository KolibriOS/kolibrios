if tup.getconfig('NO_JWASM') ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
 
tup.rule({"PlasmaEffect.asm"}, "jwasm -zt0 -coff %f ", "PlasmaEffect.o")
tup.rule("PlasmaEffect.o", "kos32-ld -T LScript.x -o %o %f -L ../../../contrib/sdk/lib -l KolibriOS && kos32-objcopy %o -O binary -j .all" .. tup.getconfig("KPACK_CMD"),"PlasmaEffect")