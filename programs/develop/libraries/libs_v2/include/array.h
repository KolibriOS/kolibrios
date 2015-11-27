typedef struct
{
	dword key;
	dword value;
} array_data;

typedef struct 
{
	array_data *(* key_string_set)(char *);
	void *(* key_string_get)(char *,array_data *);
} ARRAY;


ARRAY array;

char _init_array_ = 0;
void init_array(void)
{
	if(_init_array_) return;
	library.load("/sys/lib/array.obj");
	array.key_string_set = library.get("array.key_string_set");
	array.key_string_get = library.get("array.key_string_get");
	_init_array_ = 1;
}