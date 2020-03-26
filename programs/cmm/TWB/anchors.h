
struct _anchors {
	collection anchor_name;
	collection anchor_position;
	void add();
	int get_pos_by_name();
	void clear();
} anchors;

void _anchors::add(dword _name, _pos)
{
	anchor_name.add(_name);
	anchor_position.add(itoa(_pos));
}

int _anchors::get_pos_by_name(dword _get_name)
{
	dword pos_name = anchor_name.get_pos_by_name(_get_name);
	if (ESBYTE[_get_name]==NULL) return 0;
	if (pos_name==-1) { 
		return -1;
	} else {
		return atoi(anchor_position.get(pos_name));
	}
}

void _anchors::clear()
{
	anchor_name.drop();
	anchor_position.drop();
}

