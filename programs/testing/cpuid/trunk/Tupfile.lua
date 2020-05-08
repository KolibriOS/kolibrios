if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > lang.inc", {"lang.inc"})
tup.rule({"CPUID.ASM", extra_inputs = {"lang.inc"}}, "fasm %f %o -m70000" .. tup.getconfig("KPACK_CMD"), "CPUID")
