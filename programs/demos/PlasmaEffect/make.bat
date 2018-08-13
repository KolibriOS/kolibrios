: %KolibriOS% - directory which contains KolibriOS.lib
uasm32 -zt0 -coff PlasmaEffect.asm
ld -T LScript.x PlasmaEffect.obj -o PlasmaEffect.kex -L %KolibriOS% -l KolibriOS
objcopy -O binary -j .all PlasmaEffect.kex
Del PlasmaEffect.obj
Pause