: %KolibriOS% - directory which contains KolibriOS.lib
@jwasm -zt0 -coff -Fi lang.inc RUN.asm
@ld -T LScript.x RUN.obj -o RUN.kex -L %KolibriOS% -l KolibriOS
@objcopy -O binary -j .all RUN.kex
@del RUN.obj
@del lang.inc
@pause