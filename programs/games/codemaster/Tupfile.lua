if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. " > lang.inc", {"lang.inc"})
tup.rule({"binary_master.asm", extra_inputs = {"lang.inc"}}, "fasm -m 65536 %f %o " .. tup.getconfig("KPACK_CMD"), "binary_master")
tup.rule({"hang_programmer.asm", extra_inputs = {"lang.inc"}}, "fasm -m 65536 %f %o " .. tup.getconfig("KPACK_CMD"), "hang_programmer")
tup.rule({"kolibri_puzzle.asm", extra_inputs = {"lang.inc"}}, "fasm -m 65536 %f %o " .. tup.getconfig("KPACK_CMD"), "kolibri_puzzle")