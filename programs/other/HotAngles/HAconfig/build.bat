@echo lang fix ru >lang.inc
@fasm.exe -m 16384 HACONFIG.asm HACONFIG
@erase lang.inc
@kpack HACONFIG
@pause