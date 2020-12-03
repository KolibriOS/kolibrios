if tup.getconfig("NO_FASM") ~= "" then return end
tup.foreach_rule("*.asm", "fasm %f %o", {"../OBJ/%B.o", "../OBJ/<C_Layer>"})
