#define MEMSIZE 4096*20
#define ENTRY_POINT #main

#include "../lib/fs.h"

void main()
{
	RunProgram("/kolibrios/drivers/acpi/acpi", NULL);

	pause(300);
	if (file_exists("/rd/1/drivers/devices.dat")) {
		if (GetSystemLanguage()==4) {
			notify("'ACPI/APIC\n/rd/1/drivers/devices.dat был успешно сгенерирован.\nУстановка ещё не закончена. Следуйте указаниям в Readme!' -tdO");
		} else {
			notify("'ACPI/APIC\n/rd/1/drivers/devices.dat was succesfully generated.\nInstallation is not completed.\nFor the next steps please check Readme!' -tdO");
		}
		RunProgram("/sys/@open", "/kolibrios/drivers/acpi/readme.txt");
	} else {
		notify("'Error generating /rd/1/drivers/devices.dat' -E");
	}

	ExitProcess();
}
