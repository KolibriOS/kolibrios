if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("OpusOs_Blue.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "OpusOs_Blue.skn")
