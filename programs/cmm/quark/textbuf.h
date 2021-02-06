struct TEXTBUF
{
	dword p;
	unsigned len;
	unsigned max;
	void drop();
	void extend();
	void set();
	void insert_ch();
	void insert_str();
	void del();
} textbuf;

void TEXTBUF::drop()
{
	p = free(p);
	len = max = 0;
}

void TEXTBUF::extend(unsigned _size)
{
	max = _size;
	p = realloc(p, max);
}

void TEXTBUF::set(dword _p, unsigned _size)
{
	len = _size;
	extend(len + 4096);
	strncpy(p, _p, len);
}

void TEXTBUF::insert_ch(unsigned _pos, char _ch)
{
	char str[1];
	str[0] = _ch;
	insert_str(_pos, #str, 1);
}

void TEXTBUF::insert_str(unsigned _pos, dword _string, unsigned _sl)
{
	dword i;
	if (len + _sl >= max) {
		extend(len + _sl + 4096);
	}
	len += _sl;
	for (i=p+len; i>=_pos+_sl; i--) {
		ESBYTE[i] = ESBYTE[i-_sl];
	}
	memmov(_pos, _string, _sl);
}

void TEXTBUF::del(unsigned _start, _end)
{
	if (_start > _end) _end >< _start;
	strcpy(_start, _end);
	len -= _end - _start;
}