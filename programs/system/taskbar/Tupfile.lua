-- @taskbar loads keymap.key with f68.27, which unpacks kpack'ed files.
tup.rule("KEYMAP.KEY", "cp %f %o" .. tup.getconfig("KPACK_CMD"), "keymap.key.kpack")

if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"TASKBAR.ASM", extra_inputs = {"lang.inc"}}, FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "TASKBAR")
