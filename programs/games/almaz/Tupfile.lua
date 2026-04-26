if tup.getconfig("NO_FASM") ~= "" then return end

lang = (tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")
tup.rule("ALMAZ.ASM", "fasm -dlang=" .. lang .. " %f %o " .. tup.getconfig("KPACK_CMD"), "almaz")
