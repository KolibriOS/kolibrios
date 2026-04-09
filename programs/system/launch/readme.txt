==================== Русский ====================
Версия: 0.1.80.1 (0.2 beta)
Launch - программа для запуска приложения из директорий поиска.
При запуске читает файл launch.cfg в /sys/etc, затем в директории запуска.
После этого смотрит параметры командной строки. Приоритет параметров - по
порядку считывания.
Из параметров командной строки пока реализовано только имя программы и аргументы, передаваемые ей.
При включённом использовании Kobra все заинтересованные (входящие в группу launch_reactive) приложения оповещаются
в случае успешного запуска (посылается сообщение dword 1 dword tid, tid - идентификатор запущенного процесса).
Настройки:
main.path - путь к директориям поиска
debug.debug - опции отладки (no - нет отладки или console - вывод через консоль)
debug.level - уровень отладки (0 - только сообщение удачно/неудачно, 1 - выводить сообщение для каждой директории)
kobra.use - использование Kobra

ПРЕДУПРЕЖДЕНИЕ:
для работы нужна libconfig.

==================== English ====================
Version: 0.1.80.1 (0.2 beta)
Launch is a programme that launches applications from search dirictories.
On the start it reads file launch.cfg in /sys/etc and in current dirictory.
Than it reads command line arguments. Priority of arguments is as reading.
Now there are only few command line arguments: the name of application and its arguments.
If using Kobra is enabled all intrested (members of launch_reactive group) applications are notified if
application is launched (sending message dword 1 dword tid, tid - identifier of launched process).
Configuration:
main.path - path to search dirictories
debug.debug - debug options (no or console)
debug.level - debug level (0 - show only ok/error messages, 1 - show for each directory)
kobra.use - using of Kobra
ATTENTION:
you need libconfig to use launch.