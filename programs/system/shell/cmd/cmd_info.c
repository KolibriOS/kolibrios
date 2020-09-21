/* turbocat2001 */
#if LANG_ENG
        #define HELP "info <object_name>\n"
        #define FILE_NOT_FOUND "Object '%s' not found!\n"
        #define OBJECT_INFO "Object '%s' information:\n\n"
        #define TYPE "Type: "
        #define DIR "'Folder'"
        #define PART "'Part'"
        #define FILE "'File'"
        #define CREATED "Created:    %02d.%02d.%02d  %02d:%02d:%02d\n" 
        #define MODIFID "Modified:   %02d.%02d.%02d  %02d:%02d:%02d\n"
        #define DATE_TIME "           Date        Time\n"
        #define ATTRIB "Attributes: "
        #define RO "'Read only' "
        #define HIDDEN "'Hidden' "
        #define SYS "'System' "
        #define NOT_ARCHIV "'Not archived' "
        #define FILE_SIZE "\nFile size: %u KB (%u B)\n"

#elif LANG_RUS
		#define HELP "info <имя_объекта>\n"
        #define FILE_NOT_FOUND "Объект '%s' не найден!\n"
        #define OBJECT_INFO "Информация об объекте '%s':\n\n"
        #define TYPE "Тип: "
        #define DIR "'Папка'"
        #define PART "'Том'"
        #define FILE "'Файл'"
        #define CREATED "Создан:    %02d.%02d.%02d  %02d:%02d:%02d\n" 
        #define MODIFID "Изменён:   %02d.%02d.%02d  %02d:%02d:%02d\n"
        #define DATE_TIME "           Дата        Время\n"
        #define ATTRIB "Атрибуты: "
        #define RO "'Только для чтения' "
        #define HIDDEN "'Скрытый' "
        #define SYS "'Системный' "
        #define NOT_ARCHIV "'Не архивный' "
        #define FILE_SIZE "\nРазмер файла: %u KБ (%u Б)\n"
#endif


int cmd_info(char param[])
{
    byte is_dir=0, is_part=0; // Folder or part?
    char* argv[100]; 
    if(1 != parameters_prepare(param, argv)) //get number of parameters
    {
        printf(HELP); 
        return TRUE;
    }
    
    FS_struct_BDVK *info=NULL; // BDVK struct define
    info=get_bdvk(argv[0]); // Get file info (BDVK)
    if(info==NULL)
    {
        printf(FILE_NOT_FOUND, argv[0]);
        return TRUE;
    }
    printf(OBJECT_INFO, argv[0]);
    
    printf(TYPE);
    if (info->attrib & (1 << 4))
    {
        printf(DIR);
        is_dir=1;
    }
    else if (info->attrib & (1 << 3))
    {
        printf(PART);
        is_part=1;
    }
    else
    {
        printf(FILE);
    }
    printf("\n\n");
    printf(DATE_TIME); // Show date and time
    printf(CREATED, info->c_date.d, info->c_date.m, info->c_date.y, info->c_time.h, info->c_time.m, info->c_time.s);
    printf(MODIFID, info->m_date.d, info->m_date.m, info->m_date.y, info->m_time.h, info->m_time.m, info->m_time.s);
    printf("\n");
    printf(ATTRIB); // Show Attributes
    
    if (info->attrib & (1 << 0))
    {
        printf(RO);
    }
    if (info->attrib & (1 << 1))
    {
        printf(HIDDEN);
    }
    if (info->attrib & (1 << 2))
    {
        printf(SYS);
    }
    
    if (info->attrib & (1 << 5))
    {
        printf(NOT_ARCHIV);
    }
    
    printf("\n");
    
    if (!is_dir && (info->size)>=0 && !is_part) // If dir or part then not show file size 
    {
        printf(FILE_SIZE, info->size/1024, info->size);
    }
    return TRUE;
}
