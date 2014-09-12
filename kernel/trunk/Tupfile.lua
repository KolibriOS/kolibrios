if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, "fasm -m 65536 %f %o " .. tup.getconfig("KERPACK_CMD"), "kernel.mnt")
