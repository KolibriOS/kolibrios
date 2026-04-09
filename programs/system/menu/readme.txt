English text is below
НОВОЕ ГЛАВНОЕ МЕНЮ.
Внимание: для корректной работы рекомендуется MENUET не ниже 0.76
и цветной монитор (на монохромном подсветка не видна)

Отличия от Виллиного меню:
1.Конфигурируемость. Пункты как основного, так и дополнительных
менюшек задаются файлом MENU.DAT. 
Это позволяет:
-добавлять/убирать любые пункты в меню. Программа сама анализирует
изменения и рисует окно с кнопками по количеству пунктов.В дальнейшем
имхо это позволит конфигурировать меню не только вручную, но и прог-
раммно. Никакого вмешательства в код, что позволяет конфигурить меню
и простым пользователям.
-переводить меню на любые языки, не лазя в код.
-поскольку запуск приложений через 58-ю функцию, приложения могут находить-
ся не только на рамдиске.
2.Вещица, совсем не интересная для пользователей, но возможно
представляющая интерес для программистов. Приложение многопоточное, но все
потоки запускаются на одном и том же коде. Это позволило заменить испол-
няемые файлы MENU, SELECT1, SELECT2 и т.д. одним-единственным MENU
и сильно сэкономить место на диске.
3.Самоуничтожаемость меню при клике за его пределами и при запуске приложения
4.Кнопки, подсвечиваемые при наведении на них мышью (на монохромном мониторе
подсветка не видна).
5.Поддержка клавиатуры. Кнопки Вверх, Вниз, Enter и Esc.
В общем, постарался приблизиться к виндовской менюшке.

Замечания по синтаксису файла MENU.DAT:
Размер файла MENU.DAT-не более 2К
Меню #0-всегда главное.
Количество меню-не более 10 - от #0 до #9
В каждой строке либо путь на исполняемый файл, либо ссылка на дочернее
меню, например /@5
Маркер конца ## обязателен (внимание! TINYPAD бывает его обрезает)
Под текст на менюшных кнопках отводятся первые 20 позиций каждой строки
Каждая строка отделяется ENTERом, т.е. должны присутствовать знаки пере-
вода строки 0x0d,0x0a

Прога ОЧЕНЬ сырая, поэтому просьба не удивляться, если что-нить не будет
работать. С файлом MENU.DAT просьба обращаться очень осторожно. TINYPAD
иногда его калечит. Особенно маркер конца файла!
Исполняемый файл очень рекомендуется назвать MENU. (при компиляции)
Тогда он будет вызываться из панели как и положено.
Все замечания и предложения с удовольствием принимаются на lisovin@26.ru
Приаттаченные файлы следует высылать на mutny@rambler.ru
С уважением,
Михаил Лисовин

11.07.06 - Mario79, приложение использует функцию 70.

NEW MAIN MENU
Requirements: MENUET 0.76, color monitor
WHAT'S NEW?
1.Self-configuring menu. All the configurational data is in MENU.DAT
You may add/remove menu positions, translate menu to any language,
run menu applications from HDD without source code change.
2.Multi-thread application. There're two files only: MENU and MENU.DAT
instead of MENU, SELECT1, SELECT2, SELECT3 etc.
3.Self-closing when running application or clicking out of menu.
4.Button highlight
5.Keyboard support (keys Up, Dn, Enter, Esc.)
So, it's just like Windows menu ;)
NOTES ON MENU.DAT:
Size of MENU.DAT should be not more than 2K
Number of menus-not more than 10 (from #0 to #9). #0 is always main menu
## is an end file marker - always required.
First 20 positions of any string reserved for button text
Any string contains file path or link to submenu, for example /@4.
You may edit MENU.DAT by any text editor, but be careful when using
TINYPAD (sometimes it cuts end marker).
It is recommended to compile MMENU.ASM as MENU. So, you can run it from
standard panel.
All the comments and bugreports send to lisovin@26.ru
Michail Lisovin.  

11.07.06 - Mario79, application used function 70.
