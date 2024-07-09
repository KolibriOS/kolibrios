if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
add_include(tup.getvariantdir())

-- environment variables
tup.import("KOLIBRIOS_BUILD_OFFSET") -- 1234
tup.import("KOLIBRIOS_BUILD_CMTID")  -- 0xaabbccdd
tup.import("KOLIBRIOS_BUILD_DBGTAG") -- 0x61 for 'a', etc

str_build = ""
if KOLIBRIOS_BUILD_OFFSET then
  str_build += " -dBUILD_OFFSET=" .. KOLIBRIOS_BUILD_OFFSET
end
if KOLIBRIOS_BUILD_CMTID then
  str_build += " -dBUILD_CMTID=" .. KOLIBRIOS_BUILD_CMTID
end
if KOLIBRIOS_BUILD_DBGTAG then
  str_build += " -dBUILD_DBGTAG=" .. KOLIBRIOS_BUILD_DBGTAG
end

tup.rule("echo lang fix " .. ((tup.getconfig("LANG") == "") and "en_US" or tup.getconfig("LANG")) .. " > %o", {"lang.inc"})
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, FASM .. " -m 262144 %f %o -s %o.fas" .. str_build .. tup.getconfig("KERPACK_CMD"), {"kernel.mnt", extra_outputs = {"kernel.mnt.fas"}})
--tup.rule({"kernel.mnt.fas", extra_inputs = {"kernel.mnt"}}, "symbols %f %o", "kernel.mnt.sym")
--tup.rule({"kernel.mnt.fas", extra_inputs = {"kernel.mnt"}}, "listing %f %o", "kernel.mnt.lst")
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, FASM .. " -m 262144 %f %o " .. str_build .. " -dextended_primary_loader=1 " .. tup.getconfig("KERPACK_CMD"), "kernel.mnt.ext_loader")
tup.rule({"kernel.asm", extra_inputs = {"lang.inc"}}, FASM .. " -m 262144 %f %o " .. str_build .. " -dpretest_build=1 -ddebug_com_base=0xe9", "kernel.mnt.pretest")
