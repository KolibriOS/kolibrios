#include <time.h>
#include <ksys.h>

time_t time (time_t* timer)
{
	time_t t = mktime(localtime(0));
	if (timer) *timer = t;
	return t;
}
