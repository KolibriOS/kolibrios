@echo lang fix ru >lang.inc
@fasm.exe -m 16384 HA.ASM HA
@fasm.exe -m 16384 HACONFIG.ASM HACONFIG
@erase lang.inc
@kpack HA
@kpack HACONFIG
@pause