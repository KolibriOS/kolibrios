Компилятор XD Pascal, [оригинал проекта](https://github.com/vtereshkov/xdpw), автор оригинала Василий Терешков.
В [архиве](./release) компилятор и исходники для `xdpw_2026`(версия для Windows) и `xdpk`(версия для KolibriOS).
Компилятор также компилирует сам себя из-под KolibriOS.
Размер сжатого компилятора менее 30 килобайтов.

В папке `xdpk/projects` находятся примеры, также там находится сам компилятор под KolibriOS(`xdpk/projects/Compiler`).
Чтобы собрать какой-нибудь пример, нужно просто зайти из-под KolibriOS в папку с примером и запустить файл `MAKE.SH`.
Скомпилированное приложение должно появиться в той же папке с примером(если нет, то нажмите "обновить"(F5) в файловом менеджере).
А для сборки из-под Windows нужно запустить файл `make.bat`. 

Компиляция:
  * "xdpw_2026/xdpw.exe" — компилирует из-под Windows приложения для Windows
  * "xdpk/xdpk.exe" — компилирует из-под Windows приложения для KolibriOS
  * "xdpk/xdpk.kex" — компилирует из-под KolibriOS приложения для KolibriOS

Компиляторы вырезают недостижимый код (smart linking): бинарники меньше в 1.5–5 раз.
Ключ `-nosmart` отключает оптимизацию. В `xdpw_2020` лежит прежний компилятор без неё.

Для KolibriOS реализованы модули:
  * CRT
  * LibImg
  * LibINI
  * Network
  * OpenDlg
  * ColorDlg
  * TinyGL

[Сайт проекта XD Pascal для KolibriOS: xdpk](https://gitflic.ru/project/kolibrios-programming/xd-pascal)
[Архивная копия](https://web.archive.org/web/http://forum.cantor.systems/viewtopic.php?id=131) обсуждения на форуме.
