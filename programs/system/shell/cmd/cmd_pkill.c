int cmd_pkill(char param[])
{
    int i=1, n=0; int process_count=0;
    char *buf1k=NULL;
    unsigned PID=0;
    
    #ifdef LANG_RUS
        #define PKILL_HELP      "  pkill <имя_процесса>\n\r"
        #define PKILL_KILL      "  PID: %u - убит\n"
        #define PKILL_NOT_KILL  "  PID: %u - не убит\n"
        #define PKILL_NOT_FOUND "  Процессов с таким именем не найдено!\n"
    #else
        #define PKILL_HELP      "  pkill <process_name>\n\r"
        #define PKILL_KILL      "  PID: %u - killed\n"
        #define PKILL_NOT_KILL  "  PID: %u - not killed\n"
        #define PKILL_NOT_FOUND "  No processes with this name were found!\n"
    #endif


    if(!strlen(param)){
        printf(PKILL_HELP);
        return TRUE;
	}
    
    buf1k = malloc(1024);
    if(buf1k==NULL){
        return FALSE;        
    }

	while(i!=n){
	    n = kol_process_info(i, buf1k);
        if(!strnicmp(buf1k+10, param, 10)){
            memcpy(&PID, buf1k+30 ,sizeof(unsigned));
            if(kol_process_kill_pid(PID)){
                printf(PKILL_NOT_KILL, PID);
            }else{
                printf(PKILL_KILL, PID);
            }
            process_count++;
        }
        i++;
    }

    if(!process_count){
        printf(PKILL_NOT_FOUND);
    }

    free(buf1k);
    return TRUE;
}