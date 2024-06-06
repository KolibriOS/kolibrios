@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm img_transform.asm img_transform.kex
@kpack img_transform.kex
@erase lang.inc
@pause
