
struct _anchors {
	collection anchor_name;
	collection anchor_position;
	void add();
	int get_anchor_pos();
} anchors;

void _anchors::add(dword _name, _pos)
{
	anchor_name.add(_name);
	anchor_position.add(itoa(_pos));
}

int _anchors::get_anchor_pos(dword _get_name)
{
	dword pos_name = anchor_name.get_pos_by_name(_get_name);
	if (pos_name==-1) return -1;
	return atoi(anchor_position.get(pos_name));
}

