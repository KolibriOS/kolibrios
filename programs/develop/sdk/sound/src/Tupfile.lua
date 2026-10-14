if tup.getconfig("NO_FASM") ~= "" or (tup.getconfig("NO_MSVC") ~= "" and tup.getconfig("NO_GCC") ~= "") then return end
tup.include(HELPERDIR .. "/use_fasm.lua")

OBJS = tup.foreach_rule("*.asm", FASM .. " %f %o", "%B.obj")
if tup.getconfig("NO_GCC") == ""
then tup.rule(OBJS, "kos32-ar rcs %o %f", "sound.lib")
else tup.rule(OBJS, "link.exe /lib /out:%o %f", "sound.lib")
end
