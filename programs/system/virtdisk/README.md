# VIRT_DISK
Driver for mounting RAW disk images in KolibriOS.

To demonstrate the operation of the driver, the virtdisk program was written. Program allows you to add, delete and view virtual disks.
![foto](https://github.com/Doczom/VIRT_DISK/blob/main/utils/scr_1.png)

## List of virtdisk arguments:
 - Delete command:
 
   <CODE> virtdisk -d <DISK_NUMBER> </CODE>

 - Information from disk:

   <CODE> virtdisk -i <DISK_NUMBER> </CODE>

 - Add disk image in file system:

   <CODE> virtdisk -a <IMAGE_PATH> -s <SECTOR_SIZE> -t <IMAGE_TYPE> -f <ACCESS_FLAGS> </CODE>

 - Input list all virtual disks:

   <CODE> virtdisk -l </CODE>

## List flags:
 - <CODE>ro</CODE> - read only access
 - <CODE>rw</CODE> - read-write access

## List disk image types:
 - <CODE>RAW</CODE> - it is used to mount disk images in "raw", "img" and "iso" formats

## Exemples command:
   <CODE> virtdisk -a /sd0/4/kolibri.img -f ro </CODE>
   
   <CODE> virtdisk -d 3 </CODE>
