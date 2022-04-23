/* WARNING: SAVE ONLY IN CP866 ENCODING! */

const command_t COMMANDS[]=
{
        {"about",   "  Выводит информацию о программе Shell\n\r", &cmd_about},
        {"alias",   "  Показывает и позволяет изменить список синонимов команд\n\r", &cmd_alias},
        {"cd",      "  Изменяет текущую дерикторию. Использование:\n\r    cd <директория>\n\r", &cmd_cd},
        {"clear",   "  Очищает экран\n\r", &cmd_clear},
        {"cp",      "  Копирует файл\n\r", &cmd_cp},
        {"mv",      "  Перемещает файл\n\r", &cmd_mv},
        {"ren",     "  Переименовывает файл\n\r", &cmd_ren},
        {"date",    "  Показывает текущую дату и время\n\r", &cmd_date},
        {"echo",    "  Выводит данные на экран. Использование:\n\r    echo <данные>\n\r", &cmd_echo},
        {"exit",    "  Завершение работы Shell\n\r", &cmd_exit},
        {"free",    "  Показывает объём оперативной памяти: всей, свободной и используемой\n\r", &cmd_memory},
        {"help",    "  Справка по командам. Использование:\n\r    help ;список всех команд\n\r    help <команда> ;справка по команде\n\r", &cmd_help},
        {"history", "  Список использованных команд\n\r", &cmd_history},   
		{"kfetch",  "  Печатает лого и информацию о системе.\n\r", &cmd_kfetch},    
        {"kill",    "  Убивает процесс. Использование:\n\r    kill <PID процесса>\n\r    kill all\n\r", &cmd_kill},
        {"pkill",   "  Убивает все процессы по имени. Использование:\n\r    pkill <имя_процесса>\n\r", &cmd_pkill},
        {"ls",      "  Выводит список файлов. Использование:\n\r    ls ;список файлов в текущем каталоге\n\r    ls <директория> ;список файлов из заданной директории\n\r", &cmd_ls},
        {"lsmod",	"  list working driver \n\r", &cmd_lsmod},
		{"mkdir",   "  Создает каталог и родительские каталоги при необходимости. Использование:\n\r    mkdir <имя/папки>", &cmd_mkdir},
        {"more",    "  Выводит содержимое файла на экран. Использование:\n\r    more <имя файла>\n\r", &cmd_more},
        {"ps",      "  Выводит список процессов\n\r  Если указано <имяпроцесса>, показывает больше данных и сохраняет LASTPID\n\r", &cmd_ps},
        {"pwd",     "  Показывает имя текущей директории\n\r", &cmd_pwd},
        {"reboot",  "  Перезагружает компьютер или ядро KolibriOS. Использование:\n\r    reboot ;перезагрузить ПК\n\r    reboot kernel ;перезапустить ядро Kolibri\n\r", &cmd_reboot},
        {"rm",      "  Удаляет файл. Использование:\n\r    rm <имя файла>\n\r", &cmd_rm},
        {"rmdir",   "  Удаляет папку. Использование:\n\r    rmdir <директория>\n\r", &cmd_rmdir},
        {"shutdown","  Выключает компьютер\n\r", &cmd_shutdown},
        {"sleep",   "  Останавливает работу Shell'а на заданное время. Использование:\n\r    sleep <интервал в сотых доля секунды>\n\r  Пример:\n\r    sleep 500 ;пауза на 5 сек.\n\r", &cmd_sleep},
        {"touch",   "  Создаёт пустой файл или изменяет дату/время создания файла. Использование:\n\r    touch <имя файла>\n\r", &cmd_touch},
        {"uptime",  "  Показывает uptime\n\r", &cmd_uptime},
        {"ver",     "  Показывает версию. Использование:\n\r    ver ;версия Shell\n\r    ver kernel ;версия и номер ревизии ядра KolibriOS\n\r    ver cpu ;информация о процессоре\n\r", &cmd_ver},
        {"waitfor", "  Приостанавливает выполнение команд. Использование:\n\r    waitfor ;ожидаем предыдущий запущенный процесс LASTPID\n\r    waitfor <PID>;ждем завершения процесса с указанным PID\n\r", &cmd_waitfor},
};

#define CMD_ABOUT_MSG "Shell %s\n\r"
#define CMD_CD_USAGE "  cd <директория>\n\r"
#define CMD_CP_USAGE "  cp <источник> <результат>\n\r"
#define CMD_DATE_DATE_FMT "  Дата [дд.мм.гг]: %x%x.%x%x.%x%x"
#define CMD_DATE_TIME_FMT "\n\r  Время [чч:мм:сс]: %x%x:%x%x:%x%x\n\r"
#define CMD_FREE_FMT "  Всего        [КБ / МБ / %%]:  %-7d / %-5d / 100\n\r  Свободно     [КБ / МБ / %%]:  %-7d / %-5d / %d\n\r  Используется [КБ / МБ / %%]:  %-7d / %-5d / %d\n\r"
#define CMD_HELP_AVAIL "  Количество доступных команд: %d\n\r"
#define CMD_HELP_CMD_NOT_FOUND "  Команда \'%s\' не найдена.\n\r"

#define CMD_KILL_USAGE "  kill <PID>\n\r"
#define CMD_MKDIR_USAGE "  mkdir <директория>\n\r"
#define CMD_MORE_USAGE "  more <имя файла>\n\r"
#define CMD_MV_USAGE "  mv <источник> <результат>\n\r"
#define CMD_PKILL_HELP      "  pkill <имя процесса>\n\r"
#define CMD_PKILL_KILL      "  PID: %u - убит\n"
#define CMD_PKILL_NOT_KILL  "  PID: %u - не убит\n"
#define CMD_PKILL_NOT_FOUND "  Процессов с таким именем не найдено!\n"

#define CMD_REN_USAGE "  ren <файл> <новое имя>\n\r"
#define CMD_RM_USAGE "  rm <имя файла>\n\r"
#define CMD_RMDIR_USAGE "  rmdir <директория>\n\r"
#define CMD_SLEEP_USAGE "  sleep <интервал в сотых доляx секунды>\n\r"
#define CMD_TOUCH_USAGE "  touch <имя файла>\n\r"
#define CMD_UPTIME_FMT "  Uptime: %d дней, %d:%d:%d.%d\n\r"
#define CMD_VER_FMT1 "  KolibriOS v%d.%d.%d.%d. SVN-рев. ядра: %d\n\r"
#define CMD_WAITFOR_FMT "  Ожидаем завершения PID %d\n\r"
#define EXEC_STARTED_FMT "  '%s' запущен. PID = %d\n\r"
#define EXEC_SCRIPT_ERROR_FMT "Ошибка в '%s' : скрипт должен начинаться со строчки #SHS\n\r"
#define UNKNOWN_CMD_ERROR "  Ошибка!\n\r"
#define CON_APP_ERROR "  Ошибка в консольном приложении.\n\r"
#define FILE_NOT_FOUND_ERROR "  Файл '%s' не найден.\n\r"