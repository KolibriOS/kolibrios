Set NAME=main
Set BCC=bcc32
Set MSVC=link
Set LD=ld
Set SDK_DIR=../../../contrib/sdk

%BCC% -c %NAME%.c
%MSVC% -edit %NAME%.obj
%LD% -static -S -nostdlib -T %SDK_DIR%/sources/newlib/app.lds --image-base 0 -L%SDK_DIR%/lib --subsystem console -o %NAME%.kex %NAME%.obj -lgcc -lc.dll
 
if exist %NAME%.kex kpack %NAME%.kex
pause
