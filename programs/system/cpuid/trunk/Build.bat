@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 cpuid.asm cpuid
@erase lang.inc
@kpack cpuid
@pause
