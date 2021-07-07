if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"boot_fat12.asm", extra_inputs = {"lang.inc"}}, "fasm %f %o", "boot_fat12.bin")
