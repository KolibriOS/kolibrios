if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("nix_big.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "nix_big.skn")
