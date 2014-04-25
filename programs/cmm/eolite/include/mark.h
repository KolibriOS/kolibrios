#define MAX_ELEMENT 10

byte mark_active = 0;

struct path_strng {
	char Item[4096];
};

struct Elements_Path {
	dword	size;
	dword	type;
	int     count;
	path_strng element_list[MAX_ELEMENT];
};	

Elements_Path elements_path;

void mark_default()
{
	mark_active = 0;
	elements_path.count = 0;
	elements_path.type = 3;
	for (i = 0; i < MAX_ELEMENT; i++) strcpy(#elements_path.element_list[i].Item[0], 0);
}

void add_to_mark(dword pcth)
{
	if (mark_active) mark_active = 1;
	strlcpy(#elements_path.element_list[elements_path.count].Item, pcth);
	elements_path.count++;
	
}