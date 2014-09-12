if tup.getconfig("NO_FASM") ~= "" then return end

-- Select mixing rate: 22050, 44100 or 48000 (22.05 KHz, 44.1 KHz or 48 KHz)
UF_FREQ="48000"

-- Set volume ramping mode (interpolation): NONE, WEAK or STRONG
UF_RAMP="STRONG"

-- Set build mode: NORMAL, UNSAFE or AC97SND
UF_MODE="NORMAL"

if tup.getconfig("TUP_PLATFORM") == "win32"
then tup.rule(
  "echo UF_FREQ  equ $(UF_FREQ)  > %o && " ..
  "echo UF_RAMP  equ $(UF_RAMP) >> %o && " ..
  "echo UF_MODE  equ $(UF_MODE) >> %o && " ..
  "echo DEBUG    equ 0          >> %o && " ..
  "echo NOLINKER equ 0          >> %o && " ..
  "echo include 'eff.inc'       >> %o && " ..
  "echo include 'fasm.asm'      >> %o",
  {"tmp.asm"})
else tup.rule(
  "echo UF_FREQ  equ $(UF_FREQ)  > %o && " ..
  "echo UF_RAMP  equ $(UF_RAMP) >> %o && " ..
  "echo UF_MODE  equ $(UF_MODE) >> %o && " ..
  "echo DEBUG    equ 0          >> %o && " ..
  "echo NOLINKER equ 0          >> %o && " ..
  "echo include \"'eff.inc'\"   >> %o && " ..
  "echo include \"'fasm.asm'\"  >> %o",
  {"tmp.asm"})
end
tup.rule("tmp.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ufmod.obj")
