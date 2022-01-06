================================ ENG ================================

Current driver installation is semi-manual. 
To turn on APIC you have to:

1) Run Installer (install.kex)
2) Wait 3 seconds and get sure that there is a message about succesfull
   file generation /sys/drivers/devices.dat
   Note: log can be found in /tmp0/1/acpi.log
3) Make kernel restart (MENU -> END -> HOME key)
4) Check that kernel and drivers are working well.
5) Save kolibri.img. Now each time you boot APIC would be turned on automatically.

================================ RUS ================================

Установка драйвера делается в частично ручном режиме, о чём дальше.
Чтобы включить APIC надо:

1) Запустить установщик (install.kex)
2) Подождать 3 секунды и убедиться, что показалось сообщение
   об успешной генерации /sys/drivers/devices.dat
   Лог драйвера находится в /tmp0/1/acpi.log
3) Сделать рестарт ядра (Меню -> Завершение работы -> Ядро)
4) Проверить работу ядра и драйверов
5) Сохранить образ. Теперь APIC будет включаться при каждой загрузке ядра.

Детали реализации.

Ядро в процессе инициализации, ещё до переключения в режим страничной адресации, читает таблицы ACPI и определяет базовые адреса IOAPIC и Local APIC. На втором этапе функция APIC_init проверяет базовые адреса и загружает файл devices.dat. Если файл загружен успешно, ядро настраивает IOAPIC и Local APIC, переключает обработку прерываний в режим APIC и патчит номера линий IRQ в конфигурационном пространстве PCI значениями из devices.dat.

Подробнее https://board.kolibrios.org/viewtopic.php?f=1&t=1195&hilit=devices.dat&start=105#p37822