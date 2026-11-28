#include <shell_api.h>

#define MAX_SHELL_IDLE_YIELDS   2000

static int prog_alive(int pid) {
    return (_ksys_get_thread_slot(pid) != 0);
}

int program_console(int pid) {
    char name[32];
    struct shell_shm_buffer *buffer;
    char *buf1k;
    int result;
    int i;
    char command;
    int size;
    int is_end;
    unsigned int numIdle;

    itoa(pid, name);
    strcat(name, "-SHELL");

    buffer = NULL;
    buf1k = NULL;

    for (i = 0; i < 30;  i++) {
        result = kol_buffer_open(name, SHM_OPEN | SHM_WRITE, 0, &buffer);
        if (buffer != NULL)
            break;

        kol_sleep(2);
    }

    if (buffer == NULL)
        return 0;
    else
        size = result;

    is_end = 0;
    numIdle = 0;
    for (;;) {
        command = buffer->cmd;

        if(command != SHELL_OK) {
            numIdle = 0;
        }

        switch (command) {
            case SHELL_EXIT:
                buffer->cmd = SHELL_OK;
                is_end = 1;
                break;

            case SHELL_OK:
                if (!prog_alive(pid)) {
                    printf("\n\r");
                    return 1;
                }
                numIdle++;
                if (numIdle < MAX_SHELL_IDLE_YIELDS)
                    _ksys_thread_yield();
                else
                    kol_sleep(1);

                break;

            case SHELL_CLS:
                con_cls();
                buffer->cmd = SHELL_OK;
                break;

            case SHELL_PUTC:
                printf("%c", buffer->data[0]);
                buffer->cmd = SHELL_OK;
                break;

            case SHELL_PUTS:
                buffer->return_value.written_bytes = printf("%s", &buffer->data );
                buffer->cmd = SHELL_OK;
                break;

            case SHELL_GETC:
                *((int *)buffer->data) = getch();
                buffer->cmd = SHELL_OK;
                break;

            case SHELL_GETS:
                buffer->return_value.str = gets(buffer->data, sizeof(buffer->data));
                buffer->return_value.written_bytes = strlen(buffer->data);
                buffer->cmd = SHELL_OK;
                break;

            case SHELL_PID:
                buf1k=malloc(1024);
                kol_process_info(-1, buf1k);
                memcpy(buffer->data, buf1k+30, sizeof(unsigned));
                buffer->cmd = SHELL_OK;
                free(buf1k);
                break;

            case SHELL_PING:
                buffer->cmd = SHELL_OK;
                break;

            default:
                printf (CON_APP_ERROR);
                return 0;
        };
        if (is_end) {
            printf("\n\r");
            return 1;
        }
    } // for end
    return 9;
}

