if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"bootbios.asm", extra_inputs = {"lang.inc"}}, FASM .. " %f %o ", "bootbios.bin")
tup.rule({"bootbios.asm", extra_inputs = {"lang.inc"}}, FASM .. " %f %o -dextended_primary_loader=1", "bootbios.bin.ext_loader")
tup.rule({"kernel.asm", extra_inputs = {"bootbios.bin", "lang.inc"}}, FASM .. " -m 65536 %f %o " .. tup.getconfig("KERPACK_CMD"), "kernel.mnt")
tup.rule({"kernel.asm", extra_inputs = {"bootbios.bin.ext_loader", "lang.inc"}}, FASM .. " -m 65536 %f %o -dextended_primary_loader=1" .. tup.getconfig("KERPACK_CMD"), "kernel.mnt.ext_loader")
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, FASM .. " -m 65536 %f %o -dUEFI=1 -dextended_primary_loader=1", "kolibri.krn")
