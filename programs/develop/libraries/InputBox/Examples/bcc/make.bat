Set KolibriOS="../../../../../../contrib/sdk/lib"
Set NAME=test(InputBox)BCC
@BCC32 -c %NAME%.c
@LINK -edit %NAME%.obj
@LD -T LScript.x %NAME%.obj -o %NAME%.kex -L %KolibriOS% -l KolibriOS
@OBJCOPY -O binary -j .all %NAME%.kex
@pause