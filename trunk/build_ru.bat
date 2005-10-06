@if not exist lang.inc (
@echo lang fix ru >lang.inc
)
@fasm kernel.asm kernel.mnt