Set KolibriOS="../../../../../../contrib/sdk/lib"
Set Name=test(InputBox)UASM
@uasm32 -zt0 -coff %Name%.asm
@ld -T LScript.x %Name%.obj -o %Name%.kex -L %KolibriOS% -l KolibriOS
@objcopy -O binary -j .all %Name%.kex
@del %Name%.obj
@pause