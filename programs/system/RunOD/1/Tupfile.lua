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

tup.rule("RUN.asm", "jwasm -zt0 -coff -Fi lang.inc %f -Fo %o", "RUN.obj")
tup.rule("RUN.obj", "ld -T LScript.x %f -o %o -L ../../../../contrib/sdk/lib -l KolibriOS", "RUN")
-- tup.rule("RUN", "objcopy %f -O binary -j .all", "RUN")