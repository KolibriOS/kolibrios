#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#ifndef __MINGW32__
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#ifdef HAVE_GLOB_H
#include <glob.h>
#endif
#else
#include <io.h>
#include <shlobj.h>
#endif
#ifdef WITH_LIBARCHIVE
#include <archive.h>
/* For backward compatibility. */
#if ARCHIVE_VERSION_NUMBER < 3001000
#define archive_read_free(...) \
	archive_read_finish(__VA_ARGS__)
#define archive_read_support_filter_all(...) \
	archive_read_support_compression_all(__VA_ARGS__)
#endif
#endif
#include "system.h"

#ifdef __MINGW32__
#define mkdir(a, b) mkdir(a)
#if MAX_PATH < PATH_MAX
#error MAX_PATH < PATH_MAX. You should use MAX_PATH.
#endif
#endif

static const char *fopen_mode(unsigned int mode)
{
	static const char *modes[4][2] = {
		{ "ab", "a" },
		{ "w+b", "w+" },
		{ "rb", "r" },
		{ NULL, NULL }
	};
	const char *(*cmode)[2] = &modes[0];

	if (!(mode & DGEN_APPEND)) {
		++cmode;
		if (!(mode & DGEN_WRITE)) {
			++cmode;
			if (!(mode & DGEN_READ))
				++cmode;
		}
	}
	return (*cmode)[(!!(mode & DGEN_TEXT))];
}

enum path_type {
	PATH_TYPE_UNSPECIFIED,
	PATH_TYPE_RELATIVE,
	PATH_TYPE_ABSOLUTE
};

#ifdef __MINGW32__

/**
 * Check whether a path is absolute or relative.
 *
 * Examples:
 * /foo/bar, \\foo\\bar, c:/foo/bar are absolute,
 * ./foo/bar, ., .., are relative.
 *
 * @param[in] path Path to parse.
 * @param len Length of path.
 * @return Path type (PATH_TYPE_ABSOLUTE, PATH_TYPE_RELATIVE or
 * PATH_TYPE_UNSPECIFIED).
 */
enum path_type path_type(const char *path, size_t len)
{
	if ((len == 0) || (path[0] == '\0'))
		return PATH_TYPE_UNSPECIFIED;
	if ((path[0] == '\\') || (path[0] == '/'))
		return PATH_TYPE_ABSOLUTE;
	if ((path[0] == '.') &&
	    (((len == 1) ||
	      (path[1] == '\0') || (path[1] == '\\') || (path[1] == '/')) ||
	     ((path[1] == '.') &&
	      ((len == 2) ||
	       (path[2] == '\0') || (path[2] == '\\') || (path[2] == '/')))))
		return PATH_TYPE_RELATIVE;
	do {
		if (*(++path) == ':')
			return PATH_TYPE_ABSOLUTE;
		--len;
	}
	while ((len) && (*path != '\0') && (*path != '\\') && (*path != '/'));
	return PATH_TYPE_UNSPECIFIED;
}

#else /* __MINGW32__ */

/**
 * Check whether a path is absolute or relative.
 *
 * Examples:
 * /foo/bar, \\foo\\bar are absolute,
 * ./foo/bar, ., .., are relative.
 *
 * @param[in] path Path to parse.
 * @param len Length of path.
 * @return Path type (PATH_TYPE_ABSOLUTE, PATH_TYPE_RELATIVE or
 * PATH_TYPE_UNSPECIFIED).
 */
enum path_type path_type(const char *path, size_t len)
{
	if ((len == 0) || (path[0] == '\0'))
		return PATH_TYPE_UNSPECIFIED;
	if (path[0] == '/')
		return PATH_TYPE_ABSOLUTE;
	if ((path[0] == '.') &&
	    (((len == 1) || (path[1] == '\0') || (path[1] == '/')) ||
	     ((path[1] == '.') &&
	      ((len == 2) || (path[2] == '\0') || (path[2] == '/')))))
		return PATH_TYPE_RELATIVE;
	return PATH_TYPE_UNSPECIFIED;
}

#endif /* __MINGW32__ */

/**
 * Return user's home directory.
 * The returned string doesn't have a trailing '/' and must be freed using
 * free() (unless "buf" is provided).
 *
 * @param[in,out] buf Used to store path in. If NULL, memory is allocated.
 * @param[in,out] size Size of "buf" when provided, then the returned path
 * size.
 * @return User's home directory (either as "buf" or a new buffer),
 * NULL in case of error.
 */
char *dgen_userdir(char *buf, size_t *size)
{
	char *path;
	size_t sz_dir;
	size_t sz;
#if !defined __MINGW32__ && !defined _KOLIBRI
	struct passwd *pwd = getpwuid(geteuid());

	if ((pwd == NULL) || (pwd->pw_dir == NULL))
		return NULL;
	sz_dir = strlen(pwd->pw_dir);
#elif defined _KOLIBRI
	char *kos_home = getenv("HOME");
	if (!kos_home)
		return NULL;
	sz_dir = strlen(kos_home);
#endif
	if (buf != NULL) {
		sz = *size;
#if	defined __MINGW32__ || defined _KOLIBRI
		if (sz < PATH_MAX)
			return NULL;
#else
		if (sz < (sz_dir + 1))
			return NULL;
#endif
		path = buf;
	}
	else {
#if defined __MINGW32__ || defined _KOLIBRI
		sz = PATH_MAX;
#else
		sz = (sz_dir + 1);
#endif
		if ((path = malloc(sz)) == NULL)
			return NULL;
	}
#ifndef __MINGW32__
	#ifdef _KOLIBRI
	strncpy(path, kos_home, sz_dir);
	#else
	strncpy(path, pwd->pw_dir, sz_dir);
	#endif
#else
	if (SHGetFolderPath(NULL, (CSIDL_PROFILE | CSIDL_FLAG_CREATE),
			    0, 0, path) != S_OK) {
		if (buf == NULL)
			free(path);
		return NULL;
	}
	sz_dir = strlen(path);
	if (sz < (sz_dir + 1)) {
		if (buf == NULL)
			free(path);
		return NULL;
	}
#endif
	path[sz_dir] = '\0';
	if (size != NULL)
		*size = sz_dir;
	return path;
}

/**
 * Return DGen's home directory with an optional subdirectory (or file).
 * The returned string doesn't have a trailing '/' and must be freed using
 * free() (unless "buf" is provided).
 *
 * @param[in,out] buf Buffer to store result in. If NULL, memory is allocated.
 * @param[in,out] size Size of "buf" when provided, then the returned path
 * size.
 * @param[in] sub NUL-terminated string to append to the path.
 * @return DGen's home directory (either as "buf" or a new buffer),
 * NULL in case of error.
 */
char *dgen_dir(char *buf, size_t *size, const char *sub)
{
	char *path;
	size_t sz_dir;
	size_t sz_sub;
	const size_t sz_bd = strlen(DGEN_BASEDIR);
	size_t sz;
#ifndef __MINGW32__
	#ifndef _KOLIBRI
	struct passwd *pwd = getpwuid(geteuid());
	if ((pwd == NULL) || (pwd->pw_dir == NULL))
		return NULL;
	sz_dir = strlen(pwd->pw_dir);
	#else
	char *kos_home = getenv("HOME");
	if (!kos_home)
		return NULL;
	sz_dir = strlen(kos_home);
	#endif
#endif

	if (sub != NULL)
		sz_sub = strlen(sub);
	else
		sz_sub = 0;
	if (buf != NULL) {
		sz = *size;
#if defined(__MINGW32__) || defined(_KOLIBRI) 
		if (sz < PATH_MAX)
			return NULL;
#else
		if (sz < (sz_dir + 1 + sz_bd + !!sz_sub + sz_sub + 1))
			return NULL;
#endif
		path = buf;
	}
	else {
#if defined(__MINGW32__) || defined(_KOLIBRI)
		sz = PATH_MAX;
#else
		sz = (sz_dir + 1 + sz_bd + !!sz_sub + sz_sub + 1);
#endif
		if ((path = malloc(sz)) == NULL)
			return NULL;
	}
#ifndef __MINGW32__
	#ifndef _KOLIBRI
	strncpy(path, pwd->pw_dir, sz_dir);
	#else
	strncpy(path, kos_home, sz_dir);
	#endif
#else
	if (SHGetFolderPath(NULL, (CSIDL_APPDATA | CSIDL_FLAG_CREATE),
			    0, 0, path) != S_OK) {
		if (buf == NULL)
			free(path);
		return NULL;
	}
	sz_dir = strlen(path);
	if (sz < (sz_dir + 1 + sz_bd + !!sz_sub + sz_sub + 1)) {
		if (buf == NULL)
			free(path);
		return NULL;
	}
#endif
	path[(sz_dir++)] = DGEN_DIRSEP[0];
	memcpy(&path[sz_dir], DGEN_BASEDIR, sz_bd);
	sz_dir += sz_bd;
	if (sz_sub) {
		path[(sz_dir++)] = DGEN_DIRSEP[0];
		memcpy(&path[sz_dir], sub, sz_sub);
		sz_dir += sz_sub;
	}
	path[sz_dir] = '\0';
	if (size != NULL)
		*size = sz_dir;
	return path;
}

/**
 * Open a file relative to DGen's home directory (when "relative" is NULL or
 * path_type(relative) returns PATH_TYPE_UNSPECIFIED) and create the directory
 * hierarchy if necessary, unless the file name is already relative to
 * something or found in the current directory if mode contains DGEN_CURRENT.
 *
 * @param[in] relative Subdirectory to look in.
 * @param[in] file File name to open.
 * @param mode Mode flags to use (DGEN_READ, DGEN_WRITE and others).
 * @return File pointer, or NULL in case of error.
 * @see dgen_freopen()
 * @see system.h
 */
FILE *dgen_fopen(const char *relative, const char *file, unsigned int mode)
{
	return dgen_freopen(relative, file, mode, NULL);
}

/**
 * @see dgen_fopen()
 */
FILE *dgen_freopen(const char *relative, const char *file, unsigned int mode,
		   FILE *f)
{
	size_t size;
	size_t file_size;
	char *tmp;
	int e = errno;
	const char *fmode = fopen_mode(mode);
	char *path = NULL;

	if ((file == NULL) || (file[0] == '\0') || (fmode == NULL))
		goto error;
	/*
	  Try to open the file in the current directory if DGEN_CURRENT
	  is specified.
	*/
	if (mode & DGEN_CURRENT) {
		FILE *fd;

		if (f == NULL)
			fd = fopen(file, fmode);
		else
			fd = freopen(file, fmode, f);
		if (fd != NULL)
			return fd;
	}
	if (path_type(file, ~0u) != PATH_TYPE_UNSPECIFIED)
		size = 0;
	else if ((relative == NULL) ||
		 (path_type(relative, ~0u) == PATH_TYPE_UNSPECIFIED)) {
		if ((path = dgen_dir(NULL, &size, relative)) == NULL)
			goto error;
	}
	else {
		if ((path = strdup(relative)) == NULL)
			goto error;
		size = strlen(path);
	}

	if ((mode & (DGEN_WRITE | DGEN_APPEND)) && (path != NULL))
		mkdir(path, 0777); /* XXX make that recursive */

	file_size = strlen(file);
	if ((tmp = realloc(path, (size + !!size + file_size + 1))) == NULL)
		goto error;
	path = tmp;
	if (size)
		path[(size++)] = DGEN_DIRSEP[0];
	memcpy(&path[size], file, file_size);
	size += file_size;
	path[size] = '\0';
	errno = e;
	if (f == NULL)
		f = fopen(path, fmode);
	else
		f = freopen(path, fmode, f);
	e = errno;
	free(path);
	errno = e;
	return f;
error:
	free(path);
	errno = EACCES;
	return NULL;
}

/**
 * Return the base name in path, like basename() but without allocating
 * anything nor modifying the "path" argument.
 *
 * @param[in] path Path to extract the last component from.
 * @return Last component from "path".
 */
const char *dgen_basename(const char *path)
{
	char *tmp;

	while ((tmp = strpbrk(path, DGEN_DIRSEP)) != NULL)
		path = (tmp + 1);
	return path;
}

#define CHUNK_SIZE BUFSIZ

struct chunk {
	size_t size;
	struct chunk *next;
	struct chunk *prev;
	uint8_t data[];
};

/**
 * Unload pointer returned by load().
 *
 * @param[in] data Pointer to unload.
 */
void unload(uint8_t *data)
{
	struct chunk *chunk = ((struct chunk *)data - 1);

	assert(chunk->next == chunk);
	assert(chunk->prev == chunk);
	free(chunk);
}

#ifdef HAVE_FTELLO
#define FTELL(f) ftello(f)
#define FSEEK(f, o, w) fseeko((f), (o), (w))
#define FOFFT off_t
#else
#define FTELL(f) ftell(f)
#define FSEEK(f, o, w) fseek((f), (o), (w))
#define FOFFT long
#endif

/**
 * Call this when you're done with your file.
 *
 * @param[in,out] context Context returned by load().
 */
void load_finish(void **context)
{
#ifdef WITH_LIBARCHIVE
	struct archive *archive = *context;

	if (archive != NULL)
		archive_read_free(archive);
#endif
	*context = NULL;
}

/**
 * Return the remaining file size from the current file offset.
 *
 * @param[in] file File pointer.
 */
static size_t load_size(FILE *file)
{
	FOFFT old = FTELL(file);
	FOFFT pos;
	size_t ret = 0;

	if ((old == (FOFFT)-1) ||
	    (FSEEK(file, 0, SEEK_END) == -1))
		return 0;
	if (((pos = FTELL(file)) != (FOFFT)-1) && (pos >= old))
		ret = (size_t)(pos - old);
	FSEEK(file, old, SEEK_SET);
	return ret;
}

/**
 * Allocate a buffer and stuff the file inside using transparent decompression
 * if libarchive is available. If file_size is non-NULL, store the final size
 * there. If max_size is nonzero, refuse to load anything larger.
 * In case the returned value is NULL, errno should contain the error.
 *
 * If an error is returned but errno is 0, EOF has been reached.
 *
 * @param[in,out] context On first call of load() this should point to NULL.
 * @param[out] file_size Final size.
 * @param[in] file File pointer to load data from.
 * @param max_size If nonzero, refuse to load anything larger.
 * @return Buffer containing loaded data.
 */
uint8_t *load(void **context,
	      size_t *file_size, FILE *file, size_t max_size)
{
	size_t pos;
	size_t size = 0;
	struct chunk *chunk;
	struct chunk head = { 0, &head, &head };
	size_t chunk_size = load_size(file);
	int error = 0;
#ifdef WITH_LIBARCHIVE
	struct archive *archive = *context;
	struct archive_entry *archive_entry;

	if (archive != NULL)
		goto init_ok;
	archive = archive_read_new();
	*context = archive;
	if (archive == NULL) {
		error = ENOMEM;
		goto error;
	}
	archive_read_support_filter_all(archive);
	archive_read_support_format_all(archive);
	archive_read_support_format_raw(archive);
	if (archive_read_open_FILE(archive, file) != ARCHIVE_OK) {
		error = EIO;
		goto error;
	}
init_ok:
	switch (archive_read_next_header(archive, &archive_entry)) {
	case ARCHIVE_OK:
		break;
	case ARCHIVE_EOF:
		error = 0;
		goto error;
	default:
		error = EIO;
		goto error;
	}
#else
	*context = (void *)0xffff;
#endif
	if (chunk_size == 0)
		chunk_size = CHUNK_SIZE;
	else if ((max_size != 0) && (chunk_size > max_size))
		chunk_size = max_size;
	while (1) {
		pos = 0;
		chunk = malloc(sizeof(*chunk) + chunk_size);
		if (chunk == NULL) {
			error = errno;
			goto error;
		}
		chunk->size = chunk_size;
		chunk->next = &head;
		chunk->prev = head.prev;
		chunk->prev->next = chunk;
		head.prev = chunk;
		do {
			size_t i;
#ifdef WITH_LIBARCHIVE
			ssize_t j;

			j = archive_read_data(archive, &chunk->data[pos],
					      (chunk->size - pos));
			/*
			  Don't bother with ARCHIVE_WARN and ARCHIVE_RETRY,
			  consider any negative value an error.
			*/
			if (j < 0) {
				error = EIO;
				goto error;
			}
			i = (size_t)j;
#else
			i = fread(&chunk->data[pos], 1, (chunk->size - pos),
				  file);
#endif
			if (i == 0) {
				chunk->size = pos;
#ifndef WITH_LIBARCHIVE
				if (ferror(file)) {
					error = EIO;
					goto error;
				}
				assert(feof(file));
#endif
				goto process;
			}
			pos += i;
			size += i;
			if ((max_size != 0) && (size > max_size)) {
				error = EFBIG;
				goto error;
			}
		}
		while (pos != chunk->size);
		chunk_size = CHUNK_SIZE;
	}
process:
	chunk = realloc(head.next, (sizeof(*chunk) + size));
	if (chunk == NULL) {
		error = errno;
		goto error;
	}
	chunk->next->prev = chunk;
	head.next = chunk;
	pos = chunk->size;
	chunk->size = size;
	chunk = chunk->next;
	while (chunk != &head) {
		struct chunk *next = chunk->next;

		memcpy(&head.next->data[pos], chunk->data, chunk->size);
		pos += chunk->size;
		chunk->next->prev = chunk->prev;
		chunk->prev->next = chunk->next;
		free(chunk);
		chunk = next;
	}
	chunk = head.next;
	chunk->prev = chunk;
	chunk->next = chunk;
	if (file_size != NULL)
		*file_size = chunk->size;
	return chunk->data;
error:
#ifdef WITH_LIBARCHIVE
	load_finish(context);
#endif
	chunk = head.next;
	while (chunk != &head) {
		struct chunk *next = chunk->next;

		free(chunk);
		chunk = next;
	}
	errno = error;
	return NULL;
}

/**
 * Free NULL-terminated list of strings and set source pointer to NULL.
 * This function can skip a given number of indices (starting from 0)
 * which won't be freed.
 *
 * @param[in,out] pppc Pointer to an array of strings.
 * @param skip Number of indices to skip in *pppc[].
 */
static void free_pppc(char ***pppc, size_t skip)
{
	char **p = *pppc;
	size_t i;

	if (p == NULL)
		return;
	*pppc = NULL;
	for (i = 0; (p[i] != NULL); ++i) {
		if (skip == 0)
			free(p[i]);
		else
			--skip;
	}
	free(p);
}

/**
 * Return a list of path names that match "len" characters of "path" on the
 * file system, or NULL if none was found or if an error occured.
 *
 * @param[in] path Path name to match.
 * @param len Number of characters in "path" to match.
 * @return List of matching path names or NULL.
 */
static char **complete_path_simple(const char *path, size_t len)
{
	size_t rlen;
	const char *cpl;
	char *root;
	struct dirent *dent;
	DIR *dir;
	char **ret = NULL;
	size_t ret_size = 256;
	size_t ret_used = 0;
	struct stat st;

	if ((rlen = strlen(path)) < len)
		len = rlen;
	cpl = path;
	while (((root = strpbrk(cpl, DGEN_DIRSEP)) != NULL) &&
	       (root < (path + len)))
		cpl = (root + 1);
	rlen = (cpl - path);
	len -= rlen;
	if (rlen == 0) {
		path = "." DGEN_DIRSEP;
		rlen = 2;
	}
	if ((root = malloc(rlen + 1)) == NULL)
		return NULL;
	memcpy(root, path, rlen);
	root[rlen] = '\0';
	if (((dir = opendir(root)) == NULL) ||
	    ((ret = malloc(sizeof(*ret) * ret_size)) == NULL))
		goto error;
	ret[(ret_used++)] = NULL;
	while ((dent = readdir(dir)) != NULL) {
		size_t i;
		char *t;

		if ((cpl[0] != '\0') && (strncmp(cpl, dent->d_name, len)))
			continue;
		/* Remove "." and ".." entries. */
		if ((dent->d_name[0] == '.') &&
		    ((dent->d_name[1] == '\0') ||
		     ((dent->d_name[1] == '.') && (dent->d_name[2] == '\0'))))
			continue;
		if (ret_used == ret_size) {
			char **rt;

			ret_size *= 2;
			if ((rt = realloc(ret,
					  (sizeof(*rt) * ret_size))) == NULL)
				break;
			ret = rt;
		}
		i = strlen(dent->d_name);
		/* Allocate one extra char in case it's a directory. */
		if ((t = malloc(rlen + i + 1 + 1)) == NULL)
			break;
		memcpy(t, root, rlen);
		memcpy(&t[rlen], dent->d_name, i);
		t[(rlen + i)] = '\0';
		if ((stat(t, &st) != -1) && (S_ISDIR(st.st_mode))) {
			t[(rlen + (i++))] = DGEN_DIRSEP[0];
			t[(rlen + i)] = '\0';
		}
		for (i = 0; (ret[i] != NULL); ++i)
			if (strcmp(dent->d_name, &ret[i][rlen]) < 0)
				break;
		memmove(&ret[(i + 1)], &ret[i],
			(sizeof(*ret) * (ret_used - i)));
		ret[i] = t;
		++ret_used;
	}
	closedir(dir);
	free(root);
	if (ret[0] != NULL)
		return ret;
	free(ret);
	return NULL;
error:
	if (dir != NULL)
		closedir(dir);
	free(root);
	if (ret != NULL) {
		while (*ret != NULL)
			free(*(ret++));
		free(ret);
	}

	return NULL;
}

#if defined(HAVE_GLOB_H) && !defined(__MINGW32__)

#define COMPLETE_USERDIR_TILDE 0x01
#define COMPLETE_USERDIR_EXACT 0x02
#define COMPLETE_USERDIR_ALL 0x04

/**
 * Return the list of home directories that match "len" characters of a
 * user's name ("prefix").
 * COMPLETE_USERDIR_TILDE - Instead of directories, the returned strings are
 * tilde-prefixed user names.
 * COMPLETE_USERDIR_EXACT - Prefix must exactly match a user name.
 * COMPLETE_USERDIR_ALL - When prefix length is 0, return all user names
 * instead of the current user only.
 *
 * @param[in] prefix Path name to match.
 * @param len Number of characters to match in "path".
 * @return List of home directories that match "len" characters of "prefix".
 */
static char **complete_userdir(const char *prefix, size_t len, int flags)
{
	char **ret = NULL;
	char *s;
	struct passwd *pwd;
	size_t n;
	size_t i;
	int tilde = !!(flags & COMPLETE_USERDIR_TILDE);
	int exact = !!(flags & COMPLETE_USERDIR_EXACT);
	int all = !!(flags & COMPLETE_USERDIR_ALL);

	setpwent();
	if ((!all) && (len == 0)) {
		if (((pwd = getpwuid(geteuid())) == NULL) ||
		    ((ret = calloc(2, sizeof(ret[0]))) == NULL))
			goto err;
		if (tilde)
			s = pwd->pw_name;
		else
			s = pwd->pw_dir;
		i = strlen(s);
		if ((ret[0] = calloc((tilde + i + 1),
				     sizeof(*ret[0]))) == NULL)
			goto err;
		if (tilde)
			ret[0][0] = '~';
		memcpy(&ret[0][tilde], s, i);
		ret[0][(tilde + i)] = '\0';
		goto end;
	}
	n = 64;
	if ((ret = calloc(n, sizeof(ret[0]))) == NULL)
		goto err;
	i = 0;
	while ((pwd = getpwent()) != NULL) {
		size_t j;

		if (exact) {
			if (strncmp(pwd->pw_name, prefix,
				    strlen(pwd->pw_name)))
				continue;
		}
		else if (strncmp(pwd->pw_name, prefix, len))
			continue;
		if (i == (n - 1)) {
			char **tmp;

			n += 64;
			if ((tmp = realloc(ret, (sizeof(ret[0]) * n))) == NULL)
				goto end;
			ret = tmp;
		}
		if (tilde)
			s = pwd->pw_name;
		else
			s = pwd->pw_dir;
		j = strlen(s);
		if ((ret[i] = calloc((tilde + j + 1),
				     sizeof(*ret[0]))) == NULL)
			break;
		if (tilde)
			ret[i][0] = '~';
		memcpy(&ret[i][tilde], s, j);
		ret[i][(tilde + j)] = '\0';
		++i;
	}
	if (i == 0) {
		free(ret);
		ret = NULL;
	}
end:
	endpwent();
	return ret;
err:
	endpwent();
	free_pppc(&ret, 0);
	return NULL;
}

/**
 * Return a list of pathnames that match "len" characters of "prefix" on the
 * file system, or NULL if none was found or if an error occured. This is done
 * using glob() in order to handle wildcard characters in "prefix".
 *
 * When "prefix" isn't explicitly relative nor absolute, if "relative" is
 * non-NULL, then the path will be completed as if "prefix" was a subdirectory
 * of "relative". If "relative" is NULL, DGen's home directory will be used.
 *
 * If "relative" isn't explicitly relative nor absolute, it will be considered
 * a subdirectory of DGen's home directory.
 *
 * @param[in] prefix Path name to match.
 * @param len Number of characters to match in "path".
 * @param[in] relative If non-NULL, consider path relative to this.
 * @return List of path names that match "len" characters of "prefix".
 */
char **complete_path(const char *prefix, size_t len, const char *relative)
{
	char *s;
	char **ret;
	size_t i;
	glob_t g;
	size_t strip;

	(void)complete_path_simple; /* unused */
	if ((i = strlen(prefix)) < len)
		len = i;
	else
		i = len;
	if (((s = strchr(prefix, '/')) != NULL) && ((i = (s - prefix)) > len))
		i = len;
	if ((len == 0) ||
	    ((prefix[0] != '~') &&
	     (strncmp(prefix, ".", i)) &&
	     (strncmp(prefix, "..", i)))) {
		size_t n;

		if ((relative == NULL) ||
		    (path_type(relative, ~0u) == PATH_TYPE_UNSPECIFIED)) {
			char *x = dgen_dir(NULL, &n, relative);

			if ((x == NULL) ||
			    ((s = realloc(x, (n + 1 + len + 2))) == NULL)) {
				free(x);
				return NULL;
			}
		}
		else {
			n = strlen(relative);
			if ((s = malloc(n + 1 + len + 2)) == NULL)
				return NULL;
			memcpy(s, relative, n);
		}
		s[(n++)] = '/';
		strip = n;
		memcpy(&s[n], prefix, len);
		len += n;
		s[(len++)] = '*';
		s[len] = '\0';
	}
	else if (prefix[0] == '~') {
		char **ud;
		size_t n;

		if (s == NULL)
			return complete_userdir(&prefix[1], (i - 1),
						(COMPLETE_USERDIR_TILDE |
						 COMPLETE_USERDIR_ALL));
		ud = complete_userdir(&prefix[1], (i - 1),
				      COMPLETE_USERDIR_EXACT);
		if (ud == NULL)
			goto no_userdir;
		n = strlen(ud[0]);
		if ((s = realloc(ud[0], (n + (len - i) + 2))) == NULL) {
			free_pppc(&ud, 0);
			goto no_userdir;
		}
		free_pppc(&ud, 1);
		len -= i;
		strip = 0;
		memcpy(&s[n], &prefix[i], len);
		len += n;
		s[(len++)] = '*';
		s[len] = '\0';
	}
	else {
	no_userdir:
		if ((s = malloc(len + 2)) == NULL)
			return NULL;
		memcpy(s, prefix, len);
		s[(len++)] = '*';
		s[len] = '\0';
		strip = 0;
	}
	switch (glob(s, (GLOB_MARK | GLOB_NOESCAPE), NULL, &g)) {
	case 0:
		break;
	case GLOB_NOSPACE:
	case GLOB_ABORTED:
	case GLOB_NOMATCH:
	default:
		free(s);
		return NULL;
	}
	free(s);
	if ((ret = calloc((g.gl_pathc + 1), sizeof(ret[0]))) == NULL)
		goto err;
	for (i = 0; (g.gl_pathv[i] != NULL); ++i) {
		size_t j;

		len = strlen(g.gl_pathv[i]);
		if (strip > len)
			break;
		j = (len - strip);
		if ((ret[i] = calloc((j + 1), sizeof(ret[i][0]))) == NULL)
			break;
		memcpy(ret[i], &(g.gl_pathv[i][strip]), j);
		ret[i][j] = '\0';
	}
	if (i == 0)
		goto err;
	globfree(&g);
	return ret;
err:
	globfree(&g);
	free_pppc(&ret, 0);
	return NULL;
}

#else /* defined(HAVE_GLOB_H) && !defined(__MINGW32__) */

/**
 * Return a list of pathnames that match "len" characters of "prefix" on the
 * file system, or NULL if none was found or if an error occured.
 *
 * When "prefix" isn't explicitly relative nor absolute, if "relative" is
 * non-NULL, then the path will be completed as if "prefix" was a subdirectory
 * of "relative". If "relative" is NULL, DGen's home directory will be used.
 *
 * If "relative" isn't explicitly relative nor absolute, it will be considered
 * a subdirectory of DGen's home directory.
 *
 * @param[in] prefix Path name to match.
 * @param len Number of characters to match in "path".
 * @param[in] relative If non-NULL, consider path relative to this.
 * @return List of path names that match "len" characters of "prefix".
 */
char **complete_path(const char *prefix, size_t len, const char *relative)
{
	char *s;
	char **ret;
	size_t i;
	size_t n;
	size_t strip;
	enum path_type pt;

	if ((i = strlen(prefix)) < len)
		len = i;
	if (((pt = path_type(prefix, len)) == PATH_TYPE_ABSOLUTE) ||
	    (pt == PATH_TYPE_RELATIVE))
		return complete_path_simple(prefix, len);
	if ((len != 0) && (prefix[0] == '~') &&
	    ((len == 1) ||
	     (prefix[1] == '\0') ||
	     (strpbrk(prefix, DGEN_DIRSEP) == &prefix[1]))) {
		char *x = dgen_userdir(NULL, &n);

		if ((x == NULL) ||
		    ((s = realloc(x, (n + 1 + 2 + len + 1))) == NULL)) {
			free(x);
			return NULL;
		}
		++prefix;
		--len;
		strip = 0;
	}
	else if ((relative == NULL) ||
		 (path_type(relative, ~0u) == PATH_TYPE_UNSPECIFIED)) {
		char *x = dgen_dir(NULL, &n, relative);

		if ((x == NULL) ||
		    ((s = realloc(x, (n + 1 + len + 1))) == NULL)) {
			free(x);
			return NULL;
		}
		strip = (n + 1);
	}
	else {
		n = strlen(relative);
		if ((s = malloc(n + 1 + len + 1)) == NULL)
			return NULL;
		memcpy(s, relative, n);
		strip = (n + 1);
	}
	s[(n++)] = DGEN_DIRSEP[0];
	memcpy(&s[n], prefix, len);
	len += n;
	s[len] = '\0';
	ret = complete_path_simple(s, len);
	free(s);
	if (ret == NULL)
		return NULL;
	if (strip == 0)
		return ret;
	for (i = 0; (ret[i] != NULL); ++i)
		memmove(ret[i], &ret[i][strip],
			((strlen(ret[i]) - strip) + 1));
	return ret;
}

#endif /* defined(HAVE_GLOB_H) && !defined(__MINGW32__) */

/**
 * Free return value of complete*() functions.
 *
 * @param[in, out] cp Buffer to pass to free_pppc().
 */
void complete_path_free(char **cp)
{
	free_pppc(&cp, 0);
}

/**
 * Create an escaped version of a string.
 * When not NULL, "pos" refers to an offset in string "src". It is updated
 * with its new offset value in the escaped string.
 *
 * @param[in] src String to escape.
 * @param size Number of characters from "src" to process.
 * @param flags BACKSLASHIFY_* flags.
 * @param[in, out] pos Offset in string "src" to update.
 * @return Escaped version of "src", NULL on error.
 */
char *backslashify(const uint8_t *src, size_t size, unsigned int flags,
		   size_t *pos)
{
	char *dst = NULL;
	char *tmp;
	size_t i;
	size_t j;
	char buf[5];

again:
	for (i = 0, j = 0; (i < size); ++i) {
		switch (src[i]) {
		case '\a':
			tmp = "\\a";
			break;
		case '\b':
			tmp = "\\b";
			break;
		case '\f':
			tmp = "\\f";
			break;
		case '\n':
			tmp = "\\n";
			break;
		case '\r':
			tmp = "\\r";
			break;
		case '\t':
			tmp = "\\t";
			break;
		case '\v':
			tmp = "\\v";
			break;
		case '\'':
			if (flags & BACKSLASHIFY_NOQUOTES)
				goto noquotes;
			tmp = "\\'";
			break;
		case '"':
			if (flags & BACKSLASHIFY_NOQUOTES)
				goto noquotes;
			tmp = "\\\"";
			break;
		case ' ':
			if (flags & BACKSLASHIFY_NOQUOTES)
				tmp = " ";
			else
				tmp = "\\ ";
			break;
		case '\0':
			tmp = "\\0";
			break;
		case '\\':
			if (flags & BACKSLASHIFY_NOQUOTES)
				goto noquotes;
			tmp = "\\\\";
			break;
		default:
		noquotes:
			tmp = buf;
			if (isgraph(src[i])) {
				tmp[0] = src[i];
				tmp[1] = '\0';
				break;
			}
			tmp[0] = '\\';
			tmp[1] = 'x';
			snprintf(&tmp[2], 3, "%02x", src[i]);
			break;
		}
		if (dst != NULL)
			strncpy(&dst[j], tmp, strlen(tmp));
		if ((pos != NULL) && (i == *pos)) {
			*pos = j;
			pos = NULL;
		}
		j += strlen(tmp);
	}
	if ((pos != NULL) && (i == *pos)) {
		*pos = j;
		pos = NULL;
	}
	if (dst == NULL) {
		dst = malloc(j + 1);
		if (dst == NULL)
			return NULL;
		dst[j] = '\0';
		goto again;
	}
	return dst;
}

/**
 * Convert a UTF-8 character to its 32 bit representation.
 * Return the number of valid bytes for this character.
 * On error, u32 is set to (uint32_t)-1.
 *
 * @param[out] u32 Converted character, (uint32_t)-1 on error.
 * @param[in] u8 Multibyte character to convert.
 * @return Number of bytes read.
 */
size_t utf8u32(uint32_t *u32, const uint8_t *u8)
{
	static const uint8_t fb[] = {
		/* first byte: mask, expected value, size */
		0x80, 0x00, 1,
		0xe0, 0xc0, 2,
		0xf0, 0xe0, 3,
		0xf8, 0xf0, 4,
		0xfc, 0xf8, 5,
		0xfe, 0xfc, 6,
		0xff, 0x00, 0
	};
	const uint8_t *s = fb;
	size_t i = 0;
	size_t rem;
	uint32_t ret;

	while ((*u8 & s[0]) != s[1])
		s += 3;
	rem = s[2];
	if (!rem)
		goto error;
	ret = (*u8 & ~s[0]);
	while (++i != rem) {
		++u8;
		if ((*u8 & 0xc0) != 0x80)
			goto error;
		ret <<= 6;
		ret |= (*u8 & ~0xc0);
	}
	if (((ret & ~0x07ff) == 0xd800) ||
	    ((ret & ~0x0001) == 0xfffe))
		goto error;
	*u32 = ret;
	return i;
error:
	*u32 = (uint32_t)-1;
	return i;
}

/**
 * The opposite of utf8u32().
 *
 * @param[out] u8 Converted character.
 * @param u32 Character to convert.
 * @return Number of characters written to "u8", 0 on error.
 */
size_t utf32u8(uint8_t *u8, uint32_t u32)
{
	size_t l;
	size_t i;
	uint8_t fb;
	uint32_t u;

	if ((u32 & 0x80000000) ||
	    ((u32 & ~0x07ff) == 0xd800) ||
	    ((u32 & ~0x0001) == 0xfffe))
		return 0;
	if (u32 < 0x80) {
		if (u8 != NULL)
			*u8 = u32;
		return 1;
	}
	for (l = 0, u = u32; (u & ~0x3c); ++l)
		u >>= 6;
	if (u8 == NULL)
		return l;
	for (i = l, fb = 0; (--i); u32 >>= 6, fb >>= 1, fb |= 0xc0)
		u8[i] = (0x80 | (u32 & 0x3f));
	u8[0] = (fb | u32);
	return l;
}

/**
 * Look for the longest common prefix between a string and an array
 * of strings while ignoring case.
 *
 * @param[in] str String to compare argv entries to.
 * @param[in] argv NULL-terminated array of prefixes to match.
 * @return Index in argv or -1 if nothing matches.
 */
int prefix_casematch(const char *str, const char *argv[])
{
	unsigned int i;
	size_t ret_len = 0;
	int ret = -1;

	for (i = 0; (argv[i] != NULL); ++i) {
		size_t len = strlen(argv[i]);

		if ((len < ret_len) ||
		    (strncasecmp(str, argv[i], len)))
			continue;
		ret_len = len;
		ret = i;
	}
	return ret;
}

/**
 * Read number from initial portion of a string and convert it.
 *
 * @param[in] str String to read from.
 * @param[out] num If not NULL, stores the converted number.
 * @return Length of the number in str, 0 on error.
 */
size_t prefix_getuint(const char *str, unsigned int *num)
{
	size_t len = 0;
	unsigned int ret = 0;

	while (isdigit(str[len])) {
		ret *= 10;
		ret += (str[len] - '0');
		++len;
	}
	if (len == 0)
		return 0;
	if (num != NULL)
		*num = ret;
	return len;
}
