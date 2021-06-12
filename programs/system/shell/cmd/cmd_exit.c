
int cmd_exit(char param[])
{
    free(ALIASES);
    con_exit(1);
    kol_exit();
    return TRUE;
}

