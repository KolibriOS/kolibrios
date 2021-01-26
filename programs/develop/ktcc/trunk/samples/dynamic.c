#include <conio.h>
#include <clayer/http.h>
#include <clayer/inputbox.h>

#define OK 200

int main() {
    if (con_init_console_dll()) return 1; // init fail
    con_write_asciiz("Wait, I'll ask you... when I'll done to fetch one site...\n");
    con_set_title("Dynamicaly linked app");
    
    http_msg *h = http_get("http://kolibri.org/", 0,  HTTP_FLAG_BLOCK, "");
    http_long_receive(h);
    
    if (h->status == OK) {
       con_write_string(h->content_ptr, h->content_length);
    } else {
        con_write_asciiz("Oops! Can't access to the page.\n");
    }
    char buffer[256];
    InputBox(buffer, "Hay!", "How do you do?", "Hmm?", 0, 256, 0);
    con_printf("Your answer is \"%s\"\n", buffer);
    con_write_string("It's surprising, isn't it?", 26);
    con_exit(0);
    return 0;
}
