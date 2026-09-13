#ifndef _ACPI_ACPI_H
#define _ACPI_ACPI_H
/*
 * Minimal ACPI-CA type surface for KolibriOS.
 *
 * CONFIG_ACPI is never defined here, so <linux/acpi.h> compiles only its
 * "no ACPI" branch - but that branch still mentions a handful of ACPI-CA
 * types in the signatures of its inline stubs.  Those types are what this
 * header provides; there is no ACPI interpreter behind them.
 */
#include <linux/types.h>

typedef u32  acpi_status;
typedef u32  acpi_object_type;
typedef u64  acpi_size;
typedef u64  acpi_io_address;
typedef u64  acpi_physical_address;
typedef void *acpi_handle;
typedef char *acpi_string;

#define AE_OK			((acpi_status) 0x0000)
#define AE_ERROR		((acpi_status) 0x0001)
#define AE_NOT_FOUND		((acpi_status) 0x0005)
#define AE_SUPPORT		((acpi_status) 0x001D)

#define ACPI_SUCCESS(a)		((a) == AE_OK)
#define ACPI_FAILURE(a)		((a) != AE_OK)

#define ACPI_TYPE_INTEGER	0x01
#define ACPI_TYPE_STRING	0x02
#define ACPI_TYPE_BUFFER	0x03
#define ACPI_TYPE_PACKAGE	0x04
#define ACPI_TYPE_ANY		0x00

union acpi_object {
	acpi_object_type type;
	struct {
		acpi_object_type type;
		u64 value;
	} integer;
	struct {
		acpi_object_type type;
		u32 length;
		char *pointer;
	} string;
	struct {
		acpi_object_type type;
		u32 length;
		u8 *pointer;
	} buffer;
	struct {
		acpi_object_type type;
		u32 count;
		union acpi_object *elements;
	} package;
};

struct acpi_buffer {
	acpi_size length;
	void	 *pointer;
};

struct acpi_object_list {
	u32 count;
	union acpi_object *pointer;
};

#endif /* _ACPI_ACPI_H */
