enum {
	PAGE=1, IMG, FILE
};

struct _cache
{
	dword current_buf;
	dword current_size;
	dword current_type;
	dword current_charset;
	collection url;
	collection_int data;
	collection_int size;
	collection_int type;
	collection_int charset;
	void add();
	bool has();
	void clear();
} cache;

void _cache::add(dword _url, _data, _size, _type, _charset)
{
	dword data_pointer;
	data_pointer = malloc(_size);
	memmov(data_pointer, _data, _size);
	data.add(data_pointer);

	url.add(_url);
	size.add(_size);
	type.add(_type);
	charset.add(_charset);

	current_buf = data_pointer;
	current_size = _size;
}

bool _cache::has(dword _link)
{
	int pos;
	if (!url.count) return false;
	pos = url.get_pos_by_name(_link);
	if (pos != -1) {
		current_buf = data.get(pos);
		current_size = size.get(pos);
		current_type = type.get(pos);
		current_charset = charset.get(pos);
		return true;
	}
	return false;
}

void _cache::clear()
{
	int i;
	for (i=0; i<data.count; i++) free(data.get(i));
	url.drop();
	data.drop();
	size.drop();
	type.drop();
	current_buf = NULL;
	current_size = NULL;
	current_type = NULL;
	current_charset = NULL;
}


struct _cache2
{
	collection url;
	collection path;
	dword cur;

	void add();
	bool is_contain();
	void clear();

	void save();
	void load();
} cache2;

void _cache2::add(dword _url, _path)
{
	if (!is_contain(_url)) {
		url.add(_url);
		path.add(_path);
	}
}

bool _cache2::is_contain(dword _url)
{
	cur = url.get_pos_by_name(_url);
	if (cur == -1) return false;
	return true;
}

void _cache2::clear()
{
	url.drop();
	path.drop();
}

void _cache2::save()
{
	dword buf, buf_size, i;
	buf_size = url.data_size + path.data_size + url.count + url.count + 7;
	buf = malloc(buf_size);
	strcpy(buf, "[img]\n");
	for ( i = 0 ; i < path.count ; i++ ) {
		if (!url.get(i)) continue;
		strcat(buf, url.get(i));
		strcat(buf, "=");
		strcat(buf, path.get(i));
		strcat(buf, "\n");
	}
	strcat(buf, "\0");
	CreateFile(math.min(buf_size, strlen(buf)), buf, "/tmp0/1/Cache/_list_db.ini");
	free(buf);
}

void _cache2::load()
{
	ini_enum_keys stdcall ("/tmp0/1/Cache/_list_db.ini", "img", #_cache2_load_sections);
}

byte _cache2_load_sections(dword key_value, key_name, sec_name, f_name)
{
	cache2.add(key_name, key_value);
}