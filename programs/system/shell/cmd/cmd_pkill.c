
int cmd_pkill(char param[])
{
    int i = 1, n = 0; int process_count = 0;
    char *buf1k = NULL;
    unsigned PID=0;
    
    if(!strlen(param)){
        printf(CMD_PKILL_HELP);
        return TRUE;
	}
    
    buf1k = malloc(1024);
    if (buf1k == NULL){
        return FALSE;        
    }

	while (i != n) {
	    n = kol_process_info(i, buf1k);
        if(!strnicmp(buf1k+10, param, 10)){
            memcpy(&PID, buf1k+30 ,sizeof(unsigned));
            if(kol_process_kill_pid(PID)){
                printf(CMD_PKILL_NOT_KILL, PID);
            }else{
                printf(CMD_PKILL_KILL, PID);
            }
            process_count++;
        }
        i++;
    }

    if(!process_count){
        printf(CMD_PKILL_NOT_FOUND);
    }

    free(buf1k);
    return TRUE;
}