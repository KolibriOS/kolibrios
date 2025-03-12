if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule({"3DCUBE2.ASM"}, "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "3DCUBE2")
