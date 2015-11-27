
static struct FS
{
	void *(*read)(char *path);
};


void *(* _stdcall _ptr_fs_read_)(char *path);

char _init_fs_ = 0;
void init_fs(void)
{
	if(_init_fs_) return;
	library.load("/sys/lib/fs.obj");
	_ptr_fs_read_ = library.get("fs.read");
	_init_fs_ = 1;
}

static inline dword _FS__READ_(char *name)
{
	return _ptr_fs_read_(name);
}
struct FS fs = {&_FS__READ_};