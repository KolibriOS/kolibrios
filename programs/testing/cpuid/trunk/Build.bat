@echo lang fix en_US >lang.inc
@fasm cpuid.asm cpuid
@pause
@erase lang.inc
