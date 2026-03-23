if tup.getconfig("NO_FASM") ~= "" then return end
tup.include("../../use_fasm.lua")
tup.rule("wrap_open.asm", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "wrap_open")