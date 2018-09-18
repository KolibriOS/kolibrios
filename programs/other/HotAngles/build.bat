@echo lang fix ru >lang.inc
@fasm.exe -m 16384 HOTANGLES.ASM HOTANGLES
@fasm.exe -m 16384 HACONFIG.ASM HACONFIG
@erase lang.inc
@kpack HOTANGLES
@kpack HACONFIG
@pause