#include <clayer/msgbox.h>

int main()
{
    msgbox* msg1 = NULL;
    msg1 = kolibri_new_msgbox("Title", "Text in window", 0, "Ok");
    kolibri_start_msgbox(msg1, NULL);
}
