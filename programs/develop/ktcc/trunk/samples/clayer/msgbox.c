#include <clayer/msgbox.h>

int main()
{
    msgbox *msg1=NULL;
    msg1 = kolibri_new_msgbox("MsgBoxTest", "Hello world!", 0, "ok");
    kolibri_start_msgbox(msg1, NULL);
}
