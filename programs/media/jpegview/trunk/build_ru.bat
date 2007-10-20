@erase lang.inc
@echo lang fix ru >lang.inc
@fasm jpegview.asm jpegview
@erase lang.inc
@kpack jpegview
@pause