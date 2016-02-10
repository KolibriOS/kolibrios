if not exist bin mkdir bin
@copy *.png bin\*.png
if not exist bin\font8x9.bmp @copy ..\..\..\..\..\fs\kfar\trunk\font8x9.bmp bin\font8x9.bmp
if not exist bin\tinygl.obj @fasm.exe -m 16384 ..\tinygl.asm bin\tinygl.obj

@fasm.exe -m 16384 test0.asm bin\test0.kex
@fasm.exe -m 16384 test1.asm bin\test1.kex
@fasm.exe -m 16384 test2.asm bin\test2.kex
@fasm.exe -m 16384 test3.asm bin\test3.kex

@fasm.exe -m 16384 test_array0.asm bin\test_array0.kex
@fasm.exe -m 16384 test_array1.asm bin\test_array1.kex

@fasm.exe -m 16384 test_glu0.asm bin\test_glu0.kex
@fasm.exe -m 16384 test_glu1.asm bin\test_glu1.kex
@fasm.exe -m 16384 test_glu2.asm bin\test_glu2.kex

@fasm.exe -m 16384 gears.asm bin\gears.kex
@fasm.exe -m 16384 textures0.asm bin\textures0.kex
@fasm.exe -m 16384 textures1.asm bin\textures1.kex
@fasm.exe -m 16384 textures2.asm bin\textures2.kex


@kpack bin\test0.kex
@kpack bin\test1.kex
@kpack bin\test2.kex
@kpack bin\test3.kex

@kpack bin\test_array0.kex
@kpack bin\test_array1.kex

@kpack bin\test_glu0.kex
@kpack bin\test_glu1.kex
@kpack bin\test_glu2.kex

@kpack bin\gears.kex
@kpack bin\textures0.kex
@kpack bin\textures1.kex
@kpack bin\textures2.kex

pause