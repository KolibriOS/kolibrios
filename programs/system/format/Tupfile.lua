if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "ru" or tup.getconfig("LANG")) .. " > lang.inc", {"lang.inc"})
tup.rule({"format.asm", extra_inputs = {"lang.inc"}}, "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "FORMAT")
