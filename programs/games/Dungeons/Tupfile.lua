if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule({"Dungeons.asm"}, "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "Dungeons")
