#include <kos/console.h>
#include <kos/http.h>
#include <kos/inputbox.h>

int main() {
    con_set_title("Dynamicaly linked app");
    con_write_asciiz("Wait, I'll ask you... when I'll done to fetch one site");
    http_msg *h = get("http://boppan.org", 0, HTTP_FLAG_BLOCK, "");
    if (!receive(h)) {
    	con_printf("%s\n", h->content_ptr);
    } else {
        con_write_asciiz("Danmit! Can't access to the page.");
    }
    char buffer[256];
    InputBox(buffer, "Hay!", "How do you do?", "Everything sucks", 0, 256, 0);
    con_write_asciiz(buffer);
    con_exit(0);
    return 0;
}
