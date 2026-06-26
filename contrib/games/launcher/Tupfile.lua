if tup.getconfig("NO_FASM") ~= "" then return end
lang = (tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")
tup.rule("launcher.asm", "fasm -dlang=" .. lang .. " %f %o " .. tup.getconfig("KPACK_CMD"), "launcher")
