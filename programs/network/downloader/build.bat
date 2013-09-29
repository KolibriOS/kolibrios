@erase lang.inc
@echo lang fix en >lang.inc
@fasm -m 16384 downloader.asm downloader
@kpack downloader
@erase lang.inc
@pause