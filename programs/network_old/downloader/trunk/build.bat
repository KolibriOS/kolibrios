@erase lang.inc
@echo lang fix ru >lang.inc
@fasm downloader.asm downloader
@kpack downloader
@erase lang.inc
@pause