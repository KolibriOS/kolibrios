#include <kos/console.h>
#include <kos/http.h>
#include <kos/inputbox.h>

int main() {
    con_write_asciiz("Wait, I'll ask you... when I'll done to fetch one site...\n");
    con_set_title("Dynamicaly linked app");
    http_msg *h = get("http://example.com", 0, HTTP_FLAG_BLOCK, "");
    if (!receive(h)) {
        con_printf("%s\n", h->content_ptr);
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
