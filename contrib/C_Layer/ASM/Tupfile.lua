if tup.getconfig("NO_FASM") ~= "" then return end
tup.foreach_rule("*.asm", "fasm %f ../OBJ/%o ")
