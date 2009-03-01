==================== Русский ====================
Версия: 0.1.3
Launch - программа для запуска приложения из директорий поиска.
При запуске читает файл launch.cfg в /sys/etc, затем в директории запуска.
После этого смотрит параметры командной строки. Приоритет параметров - по
порядку считывания.
Из параметров командной строки пока реализовано только имя программы и аргументы, передаваемые ей.
Настройки:
main.path - путь к директориям поиска
debug.debug - опции отладки (no - нет отладки или console - вывод через консоль)
debug.level - уровень отладки (0 - только сообщение удачно/неудачно, 1 - выводить сообщение для каждой директории)
ПРЕДУПРЕЖДЕНИЕ:
для вывода в консоль нужна изменённая библиотека console.obj!

==================== English ====================
Version: 0.1.3
Launch is a programme that launches applications from search dirictories.
On the start it reads file launch.cfg in /sys/etc and in current dirictory.
Than it reads command line arguments. Priority of arguments is as reading.
Now there are few command line arguments: the name of application and its arguments.
Configuration:
main.path - path to search dirictories
debug.debug - debug options (no or console)
debug.level - debug level (0 - show only ok/error messages, 1 - show for each directory)
ATTENTION:
to use console output you need new console.obj library!