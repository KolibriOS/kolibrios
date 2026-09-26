#include "_stdio.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>
#include <sys/dirent.h>

/*
	Open new file
*/
FILE* fopen_r(const char* restrict _name, const char* restrict _mode, FILE* restrict out)
{
	if (!_name || !_mode || !out) {
		errno = EINVAL;
		return NULL;
	}

	bool file_not_exits = false;
	ksys_bdfe_t info;
	{
		int err = _ksys_file_get_info(_name, &info);

		switch (err) {
		case KSYS_FS_ERR_SUCCESS:

			if (info.attributes & IS_FOLDER) {
				errno = EISDIR;
				return NULL;
			}

			break;
		case KSYS_FS_ERR_5:
			file_not_exits = true;
			break;
		default:
			errno = err;
			return NULL;
			break;
		}
	}

	out->flags.val = 0;
	out->position = 0;
	out->name = strdup(_name);

	if (strchr(_mode, 'r')) {
		out->flags.read = true;

		if (file_not_exits) {
			errno = ENOENT;
			free(out->name);
			return NULL;
		}
	}
	if (strchr(_mode, 'w')) {
		out->flags.write = true;
	}
	if (strchr(_mode, 'a')) {
		out->flags.val |= _FILEMODE_W | _FILEMODE_A;
	}
	if (strchr(_mode, '+')) {
		out->flags.val |= _FILEMODE_R | _FILEMODE_W;
	}

	if (out->flags.write) {
		if (file_not_exits) {
			int err = _ksys_file_create(_name);
			if (err != KSYS_FS_ERR_SUCCESS) {
				errno = err;
				free(out->name);
				return NULL;
			}
		}
	}

	if (out->flags.append) {
		out->position = info.size;
	}

	return out;
}
