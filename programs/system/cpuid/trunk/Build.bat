@echo lang fix en >lang.inc
@fasm cpuid.asm cpuid
@pause
@erase lang.inc