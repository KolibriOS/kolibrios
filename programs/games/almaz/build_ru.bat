@erase lang.inc
@echo lang fix ru >lang.inc
@fasm ALMAZ.asm ALMAZ
@erase lang.inc
@kpack ALMAZ
@pause