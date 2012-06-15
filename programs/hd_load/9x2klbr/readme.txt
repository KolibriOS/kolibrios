Purpose: when it is started (from Win95/98/ME), (correctly) unloads Windows
and loads KolibriOS instead.

Installation is not required.

Start:
	9x2klbr [[drive:]\[path\][image_name]]
Image file must be situated on hard disk.
Default values: drive C:, root folder, image kolibri.img.
Path and image name must contain only characters from first half of
ASCII-table. In particular, there must be no russian letters.

FAT: Only short names of folders and file are accepted, i.e. progra~1 instead
of Program Files; for names such as kolibri and menuet.075 (no more than
8 characters in name, no more than 3 characters in extension, no special
characters) this is satisfied automatically, in general case short name can be
found out, for example, in Explorer dialog "Properties" (in column
"MS-DOS name").

If this requirements are not satisfied, loader will not format drive :-)
but simply says 'not found'.

Examples:
	9x2klbr d:\download\kolibri\kolibri1.img
	9x2klbr c:\progra~1\kolibri\
	9x2klbr \progra~1\kolibri\
		(will load from kolibri.img)
	9x2klbr e:\
		(equivalent to 9x2klbr e:\kolibri.img)
	9x2klbr
		(without parameters; equivalent to 9x2klbr c:\kolibri.img)

						diamond
						mailto: diamondz@land.ru
