==================== Русский ====================
Версия: 0.1.1
Kobra (Kolibri Bus for Reaching Applications) - демон (сервер), позволяющий приложениям проще общаться друг с другом.
Для работы с Kobra необходимо зарегистрироваться на сервере (через IPC). После регистрации приложению доступны следующие функции:
-Вступить в группу потоков (если нет - создаётся)
-Выйти из группы
-Послать сообщение всем потокам какой-либо группы
-Получить имя именованной области со списком групп и потоков, входящих в них (и смещение списка групп) (пока не реализовано)
Позже скорее всего появятся ещё несколько функций.

Примеров работы пока нет, однако в ближайшее время использование Kobra будет реализовано в Launch и в новом хранителе экрана.

==================== English ====================
Version: 0.1.1
Kobra (Kolibri Bus for Reaching Applications) is a daemon (server), which allows applications to easier communicate with each other.
For working with Kobra application should register on server (via IPC). After registration following functions are available for application:
-Join thread group (if there is no such group, it will be created)
-Leave group
-Send message for all threads in some group
-Get name of named memory area with list of groups and threads in them (and offset of group list) (not realised yet)
Later there may be few more functions.

There is no examples of work but using of Kobra will be realised in Launch and new screen saver.