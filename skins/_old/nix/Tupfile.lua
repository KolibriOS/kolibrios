if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("nix_small.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "nix.skn")
