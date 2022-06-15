ren Cyclone.obj Cyclone.ob_

del Cyclone.asm
del Cyclone.s
del Cyclone.exe
del *.obj
del vc60.idb
del vc60.pdb
del Cyclone.plg
del Cyclone.ncb
del Cyclone.pch
del Cyclone.ilk
del Cyclone.pdb

del /q Release\*.*
rd  Release
del /q Debug\*.*
rd  Debug

ren Cyclone.ob_ Cyclone.obj
