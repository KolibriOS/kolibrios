#include <iostream>
#include <sys/kos_LoadConsole.h> // to load console.obj

using namespace std;

int main() {
	load_console();
	
	con_set_title("C++ Console.obj example");
	cout << "Press any key...\n";
	con_getch();
	cout << "Hello, KolibriOS!\n";
	con_set_title("Hello");
	
	return 0;
}
