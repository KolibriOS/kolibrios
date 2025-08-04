@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm jpegview.asm jpegview
@erase lang.inc
@kpack jpegview
@pause
