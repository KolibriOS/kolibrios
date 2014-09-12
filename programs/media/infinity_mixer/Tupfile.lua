if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("infinity_mixer.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "infinity_mixer")
