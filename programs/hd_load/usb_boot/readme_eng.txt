BOOT_F32.BIN - bootsector for FAT32;
MTLD_F32 - auxiliary loader file;
inst.exe - installer for WinNT+;
setmbr.exe - installs standard MBR (read below);
readme.txt - this file.

To install, flash with FAT32 file system, with free space available for
file kolibri.img and a couple of Kb for loader, is required.

Installation for WinNT+ users:
Run inst.exe, it will display a list of connected flash drives. Select
the drive, on which you want to install, and double-click on it.
The program will report success or fail (cannot read/write to drive or
drive is not FAT32-volume).
Copy to the flash the file kolibri.img with wanted distribution kit version.
(These two actions can be done in any order.)
Now you can boot from this flash drive.

I have encountered situation, when (recently released) flash does not boot
and displays message "Pen drive Without Operating System.Remove
Pen Drive And Reboot." If instead of booting you see the same or like message,
probably setmbr.exe can help. It must be runned with administrator rights.
After loading in the appeared list double-click on the drive corresponding to
your flash drive. Program will report success or fail.

Installation for users of other operating systems:
automatic - not supported yet. If you can work with disk editor, the following
information may help you: inst.exe does following:
- reads bootsector, checks that it specifies FAT32;
- copies to the flash the file MTLD_F32, at the same time sets attributes
"hidden","system","read-only" (they do not play any role for the loader itself,
they protect the file from unnecessary curiosity);
- reads the file BOOT_F32.BIN; in its data replaces volume parameters
from offset 3 to offset 0x5A (0x57 bytes) to parameters taken from current
bootsector;
- writes obtained data back to flash bootsector, and also in backup copy of
bootsector, if it is present (the 2-byte field on offset 0x32)
(backup copy indeed is not required to be modified, in real life it is not
used).

Under Linux a new bootsector can be installed to the drive /dev/sdb1 (replace
with a name of FAT32-volume of any device you want) with the sequence of
two following commands:
dd if=/dev/sdb1 of=BOOT_F32.BIN bs=1 skip=3 seek=3 count=87 conv=notrunc
dd if=BOOT_F32.BIN of=/dev/sdb1 bs=512 count=1 conv=notrunc
Files mtld_f32 and kolibri.img must be copied as usual.
