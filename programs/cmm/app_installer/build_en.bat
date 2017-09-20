@del lang.h--
@echo #define LANG_ENG 1 >lang.h--

call generate_file_listing.bat

@C-- installer.c
@del installer.kex
@rename installer.com installer.kex
@kpack installer.kex
@del lang.h--
@del warning.txt
@pause