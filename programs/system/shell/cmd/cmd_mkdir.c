extern int (*mkdir)(char*);

int cmd_mkdir(char dir[]) {
	unsigned dir_len = strlen(dir);
	if(!dir_len){
		printf(CMD_MKDIR_USAGE);
		return TRUE;
	}

	char *temp = malloc(dir_len+3);
	char *dir_path = strdup(dir);
	ksys_bdfe_t *bdfe = malloc(sizeof(ksys_bdfe_t));
	if(!dir_path || !temp || !bdfe){
		return FALSE;
	}
	
	if(dir[0]=='/' || dir[0]=='\\'){
		temp[0]='\0';
	}else{
		strcpy(temp,".");
	}

	char *pch = strtok(dir_path,"\\/");
	
	while (pch != NULL){
		strcat(temp, "/");
		strcat(temp, pch);
		if(_ksys_file_get_info(temp, bdfe)){
			if(mkdir(temp)){
				printf("\033[0;31;40m  FAIL  %s\n", temp);
			}else {
				printf("\033[0;32;40m  OK    %s\n", temp);
			}
		}else{
			printf("\033[0;33;40m  EXIST %s\n", temp);
		}
		pch = strtok(NULL, "\\/");
	}
	printf("\033[0m");
	free(bdfe);
	free(dir_path);
	free(temp);
	return TRUE;
}