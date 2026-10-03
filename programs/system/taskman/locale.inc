; Strings in UTF-8; the caption starts with its encoding byte.

if lang eq ru_RU
sz_title        db      3, "Диспетчер задач", 0
sz_tab_procs    db      "Процессы", 0
sz_tab_perf     db      "Нагрузка", 0
sz_tab_disks    db      "Диски", 0
sz_col_free     db      "Свободно", 0
sz_run          db      "Запустить...", 0
sz_kill         db      "Завершить", 0
sz_shutdown     db      "Завершение работы...", 0
sz_info         db      "Детали", 0
sz_system       db      "Системные", 0
sz_col_name     db      "Имя", 0
sz_col_cpu      db      "ЦП", 0
sz_col_mem      db      "Память", 0
sz_freq         db      "Частота", 0
sz_mhz          db      " МГц", 0
sz_uptime       db      "Время работы", 0
sz_threads      db      "Потоки", 0
sz_used         db      "Занято", 0
sz_total        db      "Всего", 0
sz_temp         db      "Температура", 0
sz_net          db      "Сеть", 0
sz_received     db      "Принято", 0
sz_sent         db      "Отправлено", 0
sz_crit         db      "Критическая", 0
sz_units        db      " КБ", 0, " МБ", 0, " ГБ", 0, " ТБ", 0
sz_bits         db      " бит/с", 0, " Кбит/с", 0, " Мбит/с", 0, " Гбит/с", 0

else if lang eq es_ES
sz_title        db      3, "Gestor de tareas", 0
sz_tab_procs    db      "Procesos", 0
sz_tab_perf     db      "Recursos", 0
sz_tab_disks    db      "Discos", 0
sz_col_free     db      "Libre", 0
sz_run          db      "Ejecutar...", 0
sz_kill         db      "Finalizar", 0
sz_shutdown     db      "Apagar el sistema...", 0
sz_info         db      "Detalles", 0
sz_system       db      "Sistema", 0
sz_col_name     db      "Nombre", 0
sz_col_cpu      db      "CPU", 0
sz_col_mem      db      "Memoria", 0
sz_freq         db      "Frecuencia", 0
sz_uptime       db      "Activo", 0
sz_threads      db      "Hilos", 0
sz_used         db      "En uso", 0
sz_total        db      "Total", 0
sz_temp         db      "Temperatura", 0
sz_net          db      "Red", 0
sz_received     db      "Recibido", 0
sz_sent         db      "Enviado", 0
sz_crit         db      "Crítica", 0

else
sz_title        db      3, "Task Manager", 0
sz_tab_procs    db      "Processes", 0
sz_tab_perf     db      "Performance", 0
sz_tab_disks    db      "Disks", 0
sz_col_free     db      "Free", 0
sz_run          db      "Run...", 0
sz_kill         db      "End", 0
sz_shutdown     db      "Shut down...", 0
sz_info         db      "Details", 0
sz_system       db      "System", 0
sz_col_name     db      "Name", 0
sz_col_cpu      db      "CPU", 0
sz_col_mem      db      "Memory", 0
sz_freq         db      "Frequency", 0
sz_uptime       db      "Uptime", 0
sz_threads      db      "Threads", 0
sz_used         db      "Used", 0
sz_total        db      "Total", 0
sz_temp         db      "Temperature", 0
sz_net          db      "Network", 0
sz_received     db      "Received", 0
sz_sent         db      "Sent", 0
sz_crit         db      "Critical", 0
end if

if ~ lang eq ru_RU
sz_mhz          db      " MHz", 0
sz_units        db      " KB", 0, " MB", 0, " GB", 0, " TB", 0
sz_bits         db      " bit/s", 0, " kbit/s", 0, " Mbit/s", 0, " Gbit/s", 0
end if
sz_col_id       db      "ID", 0
if lang eq ru_RU | lang eq es_ES
sz_col_num      db      "Nº", 0
else
sz_col_num      db      " #", 0
end if
sz_deg          db      " °C", 0
sz_pct          db      "%", 0
