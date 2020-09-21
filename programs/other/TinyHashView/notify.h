#include "kos32sys.h"
void notify_show(char *text)
{
   start_app("/sys/@notify", text);
}
