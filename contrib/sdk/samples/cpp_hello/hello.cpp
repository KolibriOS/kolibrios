// for 'cout'
#include <iostream>

// for 'console.obj' functions
#include "console_obj.h"

using namespace std;

int main() 
{
	// load console.obj
	load_console();
	
	con_set_title("C++ Console.obj example");
	
	cout << "Press any key...\n";
	con_getch();
	cout << "Hello, KolibriOS!\n";
	con_set_title("Hello");
	return 0;
}
