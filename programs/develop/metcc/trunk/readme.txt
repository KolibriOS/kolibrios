The main file of metcc is "tcc.c". It certainly can be compiled by MinGW Studio. 
In order to compile MenuetOS program you must have start.o, metcc.exe in the same 
directory. The command line should be of type "metcc.exe program.c melibc.a -oprogram".
In order to compile "melibc.a" you should configure paths is compile.js and run it. 
------------------------------------------------------------------------------------
ƒл€ компил€ции melibc необходимо запустить скрипт libc/make.cmd
по умолчанию считаетс€ что в переменной окружени€ PATH у вас указан путь к пакету mingw32
и к ассемблеру fasm.
------------------------------------------------------------------------------------
ƒл€ более подробных инструкций обращатесь на форум в тему 
http://meos.sysbin.com/viewtopic.php?t=565&highlight=metcc
For more help go to link above