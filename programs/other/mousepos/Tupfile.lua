if tup.getconfig("NO_FASM") ~= "" then return end
LANG = (tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")
tup.rule("mousepos.asm", "fasm -dlang=" .. LANG .. " %f %o " .. tup.getconfig("KPACK_CMD"), "mousepos")
