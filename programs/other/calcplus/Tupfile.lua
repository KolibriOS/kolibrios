if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("calcplus.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "calcplus")
