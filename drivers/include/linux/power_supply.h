#ifndef _LINUX_POWER_SUPPLY_H
#define _LINUX_POWER_SUPPLY_H
/*
 * No battery subsystem on KolibriOS.  power_supply_is_system_supplied() is
 * already provided by <syscall.h>; only the enum labels are missing.
 */
enum {
	POWER_SUPPLY_TYPE_UNKNOWN = 0,
	POWER_SUPPLY_TYPE_BATTERY,
	POWER_SUPPLY_TYPE_MAINS,
};
#endif
