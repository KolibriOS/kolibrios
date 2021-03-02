#include <stdio.h>
#include <ksys.h>

int remove(const char *name) {
	return _ksys_file_delete(name);
}
