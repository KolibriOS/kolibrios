if tup.getconfig('NO_JWASM') ~= "" or tup.getconfig("NO_GCC") ~= "" then return end

if     tup.getconfig("LANG") == "it" then
  tup.definerule{command = "echo LANG_IT = 1 > lang.inc", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "sp" then
  tup.definerule{command = "echo LANG_SP = 1 > lang.inc", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "ru" then
  tup.definerule{command = "echo LANG_RU = 1 > lang.inc", outputs = {"lang.inc"}}
elseif tup.getconfig("LANG") == "en" then
  tup.definerule{command = "echo LANG_EN = 1 > lang.inc", outputs = {"lang.inc"}}
else
  tup.definerule{command = "echo LANG_EN = 1 > lang.inc", outputs = {"lang.inc"}}
end
 
tup.rule({"RUN.asm", extra_inputs = {"lang.inc"}}, "jwasm -zt0 -coff -Fi lang.inc %f ", "RUN.o")
tup.rule("RUN.o", "kos32-ld -T LScript.x -o %o %f -L ../../../../contrib/sdk/lib -l KolibriOS && kos32-objcopy %o -O binary -j .all" .. tup.getconfig("KPACK_CMD"),"RUN")