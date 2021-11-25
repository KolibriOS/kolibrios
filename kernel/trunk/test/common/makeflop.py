#!/usr/bin/env python3
#
# makeflop.py
# Version 3
# Brad Smith, 2019
# http://rainwarrior.ca
#
# Simple file operations for a FAT12 floppy disk image.
# Public domain.

import sys
assert sys.version_info[0] == 3, "Python 3 required."

import struct
import datetime
import os

class Floppy:
    """
    Simple file operations for a FAT12 floppy disk image.

    Floppy() - creates a blank DOS formatted 1.44MB disk.
    Floppy(data) - parses a disk from a given array of bytes.
    .open(filename) - loads a file and returns a Floppy from its data.
    .save(filename) - saves the disk image to a file.
    .flush() - Updates the .data member with any pending changes.
    .data - A bytearray of the disk image.

    .files() - Returns a list of strings, each is a file or directory. Directories end with a backslash.
    .delete_path(path) - Deletes a file or directory (recursive).
    .add_dir_path(path) - Creates a new empty directory, if it does not already exist (recursive). Returns cluster of directory, or -1 if failed.
    .add_file_path(path,data) - Creates a new file (creating directory if needed) with the given data. Returns False if failed.
    .extract_file_path(path) - Returns a bytearray of the file at the given path, None if not found.
    .set_volume_id(id=None) - Sets the 32-bit volume ID. Use with no arguments to generate ID from current time.
    .set_volume_label(label) - Sets the 11-character volume label.

    .boot_info() - Returns a string displaying boot sector information.
    .fat_info() - Returns a very long string of all 12-bit FAT entries.
    .files_info() - Returns a string displaying the files() list.

    .add_all(path,prefix) - Adds all files from local path to disk (uppercased). Use prefix to specify a target directory. Returns False if any failed.
    .extract_all(path) - Dumps entire contents of disk to local path.

    .find_path(path) - Returns a Floppy.FileEntry.
    FileEntry.info() - Returns and information string about a FileEntry.

    This class provides some basic interface to a FAT12 floppy disk image.
    Some things are fragile, in particular filenames that are too long,
    or references to clusters outside the disk may cause exceptions.
    The FAT can be accessed directly with some internal functions (see implementation)
    and changes will be applied to the stored .data image with .flush().

    Example:
        f = Floppy.open("disk.img")
        print(f.boot_info()) # list boot information about disk
        print(f.file_info()) # list files and directories
        f.extract_all("disk_dump\\")
        f.delete_path("DIR\\FILE.TXT") # delete a file
        f.delete_path("DIR") # delete an entire directory
        f.add_file_path("DIR\\NEW.TXT",open("new.txt","rb").read()) # add a file, directory automatically created
        f.add_dir_path("NEWDIR") # creates a new directory
        f.add_all("add\\","ADDED\\") # adds all files from a local directory to to a specified floppy directory.
        f.set_volume_id() # generates a new volume ID
        f.set_volume_label("MY DISK") # changes the volume label
        f.save("new.img")
    """

    EMPTY = 0xE5 # incipit value for an empty directory entry

    def _filestring(s, length):
        """Creates an ASCII string, padded to length with spaces ($20)."""
        b = bytearray(s.encode("ASCII"))
        b = b + bytearray([0x20] * (length - len(b)))
        if len(b) != length:
            raise self.Error("File string '%s' too long? (%d != %d)" % (s,len(b),length))
        return b

    class Error(Exception):
        """Floppy has had an error."""
        pass
    
    class FileEntry:
        """A directory entry for a file."""

        def __init__(self, data=None, dir_cluster=-1, dir_index=-1):
            """Unpacks a 32 byte directory entry into a FileEntry structure."""
            if data == None:
                data = bytearray([Floppy.EMPTY]+([0]*31))
            self.data = bytearray(data)
            self.path = ""
            self.incipit = data[0]
            if self.incipit != 0x00 and self.incipit != Floppy.EMPTY:
                filename = data[0:8].decode("cp866")
                filename = filename.rstrip(" ")
                extension = data[8:11].decode("cp866")
                extension = extension.rstrip(" ")
                self.path = filename
                if len(extension) > 0:
                    self.path = self.path + "." + extension
            block = struct.unpack("<BHHHHHHHHL",data[11:32])
            self.attributes = block[0]
            self.write_time = block[6]
            self.write_date = block[7]
            self.cluster = block[8]
            self.size = block[9]
            self.dir_cluster = dir_cluster
            self.dir_index = dir_index

        def compile(self):
            """Commits any changed data to the entry, rebuilds and returns the 32 byte structure."""
            filename = ""
            extension = ""
            period = self.path.find(".")
            if (period >= 0):
                filename = self.path[0:period]
                extension = self.path[period+1:]
            else:
                filename = self.path
            if self.incipit != 0x00 and self.incipit != 0xEF:
                self.data[0:8] = Floppy._filestring(filename,8)
                self.data[8:11] = Floppy._filestring(extension,3)
            else:
                self.data[0] = self.incipit
            self.data[11] = self.attributes
            self.data[22:32] = bytearray(struct.pack("<HHHL",
                self.write_time,
                self.write_date,
                self.cluster,
                self.size))
            return bytearray(self.data)

        def info(self):
            """String of information about a FileEntry."""
            s = ""
            s += "Path: [%s]\n" % self.path
            s += "Incipit: %02X\n" % self.incipit
            s += "Attributes: %02X\n" % self.attributes
            s += "Write: %04X %04X\n" % (self.write_date, self.write_time)
            s += "Cluster: %03X\n" % self.cluster
            s += "Size: %d bytes\n" % self.size
            s += "Directory: %03X, %d\n" % (self.dir_cluster, self.dir_index)
            return s

        def fat_time(year, month, day, hour, minute, second):
            """Builds a FAT12 date/time entry."""
            date = ((year - 1980) << 9) | ((month) << 5) | day
            time = (hour << 11) | (minute << 5) | (second >> 1)
            return (date, time)

        def fat_time_now():
            """Builds the current time as a FAT12 date/time entry."""
            now = datetime.datetime.now()
            return Floppy.FileEntry.fat_time(now.year, now.month, now.day, now.hour, now.minute, now.second)

        def set_name(self,s):
            """Sets the filename and updates incipit."""
            self.path = s
            self.incipit = Floppy._filestring(s,12)[0]

        def set_now(self):
            """Updates modified date/time to now."""
            (date,time) = Floppy.FileEntry.fat_time_now()
            self.write_date = date
            self.write_time = time
            
        def new_file(name):
            """Generate a new file entry."""
            e = Floppy.FileEntry()
            e.set_name(name)
            e.set_now()
            e.attributes = 0x00
            return e

        def new_dir(name):
            """Generate a new subdirectory entry."""
            e = Floppy.FileEntry()
            e.set_name(name)
            e.set_now()
            e.attributes = 0x10
            return e

        def new_volume(name):
            """Generate a new volume entry."""
            e = Floppy.FileEntry()
            if len(name) > 8:
                name = name[0:8] + "." + name[8:]
            e.set_name(name)
            e.set_now()
            e.attributes = 0x08
            return e

        def new_terminal():
            """Generate a new directory terminating entry."""
            e = Floppy.FileEntry()
            e.incipit = 0x00
            return e
            
    # blank formatted 1.44MB MS-DOS 5.0 non-system floppy
    blank_floppy = [ # boot sector, 2xFAT, "BLANK" volume label, F6 fill
        0xEB,0x3C,0x90,0x4D,0x53,0x44,0x4F,0x53,0x35,0x2E,0x30,0x00,0x02,0x01,0x01,0x00,
        0x02,0xE0,0x00,0x40,0x0B,0xF0,0x09,0x00,0x12,0x00,0x02,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x29,0xE3,0x16,0x53,0x1B,0x42,0x4C,0x41,0x4E,0x4B,
        0x20,0x20,0x20,0x20,0x20,0x20,0x46,0x41,0x54,0x31,0x32,0x20,0x20,0x20,0xFA,0x33,
        0xC0,0x8E,0xD0,0xBC,0x00,0x7C,0x16,0x07,0xBB,0x78,0x00,0x36,0xC5,0x37,0x1E,0x56,
        0x16,0x53,0xBF,0x3E,0x7C,0xB9,0x0B,0x00,0xFC,0xF3,0xA4,0x06,0x1F,0xC6,0x45,0xFE,
        0x0F,0x8B,0x0E,0x18,0x7C,0x88,0x4D,0xF9,0x89,0x47,0x02,0xC7,0x07,0x3E,0x7C,0xFB,
        0xCD,0x13,0x72,0x79,0x33,0xC0,0x39,0x06,0x13,0x7C,0x74,0x08,0x8B,0x0E,0x13,0x7C,
        0x89,0x0E,0x20,0x7C,0xA0,0x10,0x7C,0xF7,0x26,0x16,0x7C,0x03,0x06,0x1C,0x7C,0x13,
        0x16,0x1E,0x7C,0x03,0x06,0x0E,0x7C,0x83,0xD2,0x00,0xA3,0x50,0x7C,0x89,0x16,0x52,
        0x7C,0xA3,0x49,0x7C,0x89,0x16,0x4B,0x7C,0xB8,0x20,0x00,0xF7,0x26,0x11,0x7C,0x8B,
        0x1E,0x0B,0x7C,0x03,0xC3,0x48,0xF7,0xF3,0x01,0x06,0x49,0x7C,0x83,0x16,0x4B,0x7C,
        0x00,0xBB,0x00,0x05,0x8B,0x16,0x52,0x7C,0xA1,0x50,0x7C,0xE8,0x92,0x00,0x72,0x1D,
        0xB0,0x01,0xE8,0xAC,0x00,0x72,0x16,0x8B,0xFB,0xB9,0x0B,0x00,0xBE,0xE6,0x7D,0xF3,
        0xA6,0x75,0x0A,0x8D,0x7F,0x20,0xB9,0x0B,0x00,0xF3,0xA6,0x74,0x18,0xBE,0x9E,0x7D,
        0xE8,0x5F,0x00,0x33,0xC0,0xCD,0x16,0x5E,0x1F,0x8F,0x04,0x8F,0x44,0x02,0xCD,0x19,
        0x58,0x58,0x58,0xEB,0xE8,0x8B,0x47,0x1A,0x48,0x48,0x8A,0x1E,0x0D,0x7C,0x32,0xFF,
        0xF7,0xE3,0x03,0x06,0x49,0x7C,0x13,0x16,0x4B,0x7C,0xBB,0x00,0x07,0xB9,0x03,0x00,
        0x50,0x52,0x51,0xE8,0x3A,0x00,0x72,0xD8,0xB0,0x01,0xE8,0x54,0x00,0x59,0x5A,0x58,
        0x72,0xBB,0x05,0x01,0x00,0x83,0xD2,0x00,0x03,0x1E,0x0B,0x7C,0xE2,0xE2,0x8A,0x2E,
        0x15,0x7C,0x8A,0x16,0x24,0x7C,0x8B,0x1E,0x49,0x7C,0xA1,0x4B,0x7C,0xEA,0x00,0x00,
        0x70,0x00,0xAC,0x0A,0xC0,0x74,0x29,0xB4,0x0E,0xBB,0x07,0x00,0xCD,0x10,0xEB,0xF2,
        0x3B,0x16,0x18,0x7C,0x73,0x19,0xF7,0x36,0x18,0x7C,0xFE,0xC2,0x88,0x16,0x4F,0x7C,
        0x33,0xD2,0xF7,0x36,0x1A,0x7C,0x88,0x16,0x25,0x7C,0xA3,0x4D,0x7C,0xF8,0xC3,0xF9,
        0xC3,0xB4,0x02,0x8B,0x16,0x4D,0x7C,0xB1,0x06,0xD2,0xE6,0x0A,0x36,0x4F,0x7C,0x8B,
        0xCA,0x86,0xE9,0x8A,0x16,0x24,0x7C,0x8A,0x36,0x25,0x7C,0xCD,0x13,0xC3,0x0D,0x0A,
        0x4E,0x6F,0x6E,0x2D,0x53,0x79,0x73,0x74,0x65,0x6D,0x20,0x64,0x69,0x73,0x6B,0x20,
        0x6F,0x72,0x20,0x64,0x69,0x73,0x6B,0x20,0x65,0x72,0x72,0x6F,0x72,0x0D,0x0A,0x52,
        0x65,0x70,0x6C,0x61,0x63,0x65,0x20,0x61,0x6E,0x64,0x20,0x70,0x72,0x65,0x73,0x73,
        0x20,0x61,0x6E,0x79,0x20,0x6B,0x65,0x79,0x20,0x77,0x68,0x65,0x6E,0x20,0x72,0x65,
        0x61,0x64,0x79,0x0D,0x0A,0x00,0x49,0x4F,0x20,0x20,0x20,0x20,0x20,0x20,0x53,0x59,
        0x53,0x4D,0x53,0x44,0x4F,0x53,0x20,0x20,0x20,0x53,0x59,0x53,0x00,0x00,0x55,0xAA,
        ] + \
        [0xF0,0xFF,0xFF] + ([0]*(0x1200-3)) + \
        [0xF0,0xFF,0xFF] + ([0]*(0x1200-3)) + [
        0x42,0x4C,0x41,0x4E,0x4B,0x20,0x20,0x20,0x20,0x20,0x20,0x28,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0x7C,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        ] + ([0] * (0x4200-0x2620)) + \
        ([0xF6] * (0x168000-0x4200))

    def __init__(self,data=blank_floppy):
        """Create Floppy() instance from image bytes, or pre-formatted blank DOS floppy by default."""
        self.data = bytearray(data)
        self._boot_open()
        self._fat_open()

    def _boot_open(self):
        """Parses the boot sector."""
        if (len(self.data) < 38):
            raise self.Error("Not enough data in image for boot sector. (%d bytes)" % len(data))
        boot = struct.unpack("<HBHBHHBHHH",self.data[11:28])
        self.sector_size = boot[0]
        self.cluster_sects = boot[1]
        self.reserved_sects = boot[2]
        self.fat_count = boot[3]
        self.root_max = boot[4]
        self.sectors = boot[5]
        self.fat_sects = boot[7]
        self.track_sects = boot[8]
        self.heads = boot[9]
        self.volume_id = 0
        self.volume_label = ""
        if self.data[38] == 0x29 and len(self.data) >= 54:
            self.volume_id = struct.unpack("<L",self.data[39:43])[0]
            self.volume_label = self.data[43:54].decode("ASCII").rstrip(" ")
        if (self.sectors * self.sector_size) > len(self.data):
            raise self.Error("Not enough data to contain %d x %d byte sectors? (%d bytes)" %
                             (self.sectors, self.sector_size, len(self.data)))
        self.root = self.sector_size * (self.reserved_sects + (self.fat_count * self.fat_sects))
        root_sectors = ((self.root_max * 32) + (self.sector_size-1)) // self.sector_size # round up to fill sector
        self.cluster2 = self.root + (self.sector_size * root_sectors)

    def _boot_flush(self):
        """Commits changes to the boot sector."""
        boot = struct.pack("<HBHBHHBHHH",
            self.sector_size,
            self.cluster_sects,
            self.reserved_sects,
            self.fat_count,
            self.root_max,
            self.sectors,
            self.data[21],
            self.fat_sects,
            self.track_sects,
            self.heads)
        self.data[11:28] = bytearray(boot)
        if self.data[38] == 0x29 and len(self.data) >= 54:
            self.data[39:43] = bytearray(struct.pack("<L",self.volume_id))
            self.data[43:54] = Floppy._filestring(self.volume_label,11)

    def boot_info(self):
        """String of information about the boot sector."""
        s = ""
        s += "Volume Label: [%s]\n" % self.volume_label
        s += "Volume ID: %04X\n" % self.volume_id
        s += "Sector size: %d bytes\n" % self.sector_size
        s += "Cluster size: %d sectors\n" % self.cluster_sects
        s += "Reserved sectors: %d\n" % self.reserved_sects
        s += "Number of FATs: %d\n" % self.fat_count
        s += "Maximum root entries: %d\n" % self.root_max
        s += "Total sectors: %d\n" % self.sectors
        s += "FAT size: %d sectors\n" % self.fat_sects
        s += "Track size: %d sectors\n" % self.track_sects
        s += "Heads: %d\n" % self.heads
        s += "Root directory: %08X\n" % self.root
        s += "Cluster 2: %08X\n" % self.cluster2
        return s

    def _fat_open(self):
        """Parses the FAT table."""
        fat_start = self.reserved_sects * self.sector_size
        fat_sects = self.fat_sects * self.sector_size
        fat_end = fat_start + fat_sects
        # make sure they're in the image
        if (self.sectors < (self.reserved_sects + (self.fat_count * self.fat_sects))):
            raise self.Error("Not enough sectors to contain %d + %d x %d FAT tables? (%d sectors)" %
                            (self.reserved_sects, self.fat_count, self.fat_sects, self.sectors))
        if (self.fat_count < 1) or (self.fat_sects < 1):
            raise self.Error("No FAT tables? (%d x %d FAT sectors)" %
                             (self.fat_count, self.fat_sects))        
        # verify FAT tables match
        for i in range(1,self.fat_count):
            fat2_start = fat_start + (fat_sects * i)
            fat2_end = fat2_start + fat_sects
            if self.data[fat_start:fat_end] != self.data[fat2_start:fat2_end]:
                raise self.Error("FAT mismatch in table %d." % i)
        # read FAT 0
        self.fat = []
        e = fat_start
        while (e+2) <= fat_end:
            entry = 0
            if (len(self.fat) & 1) == 0:
                entry = self.data[e+0] | ((self.data[e+1] & 0x0F) << 8)
                e += 1
            else:
                entry = ((self.data[e+0] & 0xF0) >> 4) | (self.data[e+1] << 4)
                e += 2
            self.fat.append(entry)

    def _fat_flush(self):
        """Commits changes to the FAT table."""
        fat_start = self.reserved_sects * self.sector_size
        fat_sects = self.fat_sects * self.sector_size
        fat_end = fat_start + fat_sects
        # build FAT 0
        e = self.reserved_sects * self.sector_size
        for i in range(len(self.fat)):
            entry = self.fat[i]
            if (i & 1) == 0:
                self.data[e+0] = entry & 0xFF
                self.data[e+1] = (self.data[e+1] & 0xF0) | ((entry >> 8) & 0x0F)
                e += 1
            else:
                self.data[e+0] = (self.data[e+0] & 0x0F) | ((entry << 4) & 0xF0)
                self.data[e+1] = (entry >> 4) & 0xFF
                e += 2
        # copy to all tables
        for i in range(1,self.fat_count):
            fat2_start = fat_start + (fat_sects * i)
            fat2_end = fat2_start + fat_sects
            self.data[fat2_start:fat2_end] = self.data[fat_start:fat_end]

    def fat_info(self):
        """String of information about the FAT."""
        per_line = 32
        s = ""
        for i in range(len(self.fat)):
            if (i % per_line) == 0:
                s += "%03X: " % i
            if (i % per_line) == (per_line // 2):
                s += "  " # extra space to mark 16
            s += " %03X" % self.fat[i]
            if (i % per_line) == (per_line - 1):
                s += "\n"
        return s

    def _cluster_offset(self, cluster):
        """Image offset of a FAT indexed logical cluster."""
        if cluster < 2:
            return self.root
        return self.cluster2 + (self.sector_size * self.cluster_sects * (cluster-2))

    def _read_chain(self, cluster, size):
        data = bytearray()
        """Returns up to size bytes from a chain of clusters starting at cluster."""
        while cluster < 0xFF0:
            offset = self._cluster_offset(cluster)
            #print("read_chain(%03X,%d) at %08X" % (cluster,size,offset))
            if cluster < 2:
                return data + bytearray(self.data[self.root:self.root+size]) # root directory
            read = min(size,(self.sector_size * self.cluster_sects))
            data = data + bytearray(self.data[offset:offset+read])
            size -= read
            if (size < 1):
                return data
            cluster = self.fat[cluster]
        return data

    def _read_dir_chain(self, cluster):
        """Reads an entire directory chain starting at the given cluster (0 for root)."""
        if cluster < 2:
            # root directory is contiguous
            return self.data[self.root:self.root+(self.root_max*32)]
        # directories just occupy as many clusters as in their FAT chain, using a dummy max size
        return self._read_chain(cluster, self.sector_size*self.sectors//self.cluster_sects)

    def _delete_chain(self, cluster):
        """Deletes a FAT chain."""
        while cluster < 0xFF0 and cluster >= 2:
            link = self.fat[cluster]
            self.fat[cluster] = 0
            cluster = link

    def _add_chain(self,data):
        """Adds a block of data to the disk and creates its FAT chain. Returns start cluster, or -1 for failure."""
        cluster_size = self.sector_size * self.cluster_sects
        clusters = (len(data) + cluster_size-1) // cluster_size
        if clusters < 1:
            clusters = 1
        # find a chain of free clusters
        chain = []
        for i in range(2,len(self.fat)):
            if self.fat[i] == 0:
                chain.append(i)
            if len(chain) >= clusters:
                break
        if len(chain) < clusters:
            return -1 # out of space
        # store the FAT chain
        start_cluster = chain[0]
        for i in range(0,len(chain)-1):
            self.fat[chain[i]] = chain[i+1]
        self.fat[chain[len(chain)-1]] = 0xFFF
        # store the data in the given clusters
        data = bytearray(data)
        c = 0
        while len(data) > 0:
            write = min(len(data),cluster_size)
            offset = self._cluster_offset(chain[c])
            self.data[offset:offset+write] = data[0:write]
            c += 1
            data = data[write:]
        # done
        return start_cluster
        
    def _files_dir(self, cluster, path):
        """Returns a list of files in the directory starting at the given cluster. Recursive."""
        #print("files_dir(%03X,'%s')" % (cluster, path))
        entries = []
        directory = self._read_dir_chain(cluster)
        for i in range(len(directory) // 32):
            e = self.FileEntry(directory[(i*32):(i*32)+32],cluster,i)
            #print(("entry %d (%08X)\n"%(i,self._dir_entry_offset(cluster,i)))+e.info())
            if e.incipit == 0x00: # end of directory
                return entries
            if e.incipit == Floppy.EMPTY: # empty
                continue
            if (e.attributes & 0x08) != 0: # volume label
                continue
            if (e.attributes & 0x10) != 0: # subdirectory
                if e.path == "." or e.path == "..":
                    continue
                subdir = path + e.path + "\\"
                entries.append(subdir)
                entries = entries + self._files_dir(e.cluster,subdir)
            else:
                entries.append(path + e.path)
        return entries

    def files(self):
        """Returns a list of files in the image."""
        root = self.sector_size * (self.reserved_sects + (self.fat_count * self.fat_sects))
        return self._files_dir(0,"")

    def files_info(self):
        """String of the file list."""
        s = ""
        for path in self.files():
            s += path + "\n"
        return s

    def _dir_entry_offset(self,cluster,dir_index):
        """Find the offset in self.data to a particular directory entry."""
        if (cluster < 2):
            return self.root + (32 * dir_index)
        if (cluster >= 0xFF0):
            raise self.Error("Directory entry %d not in its cluster chain?" % dir_index)
        per_cluster = (self.sector_size*self.cluster_sects)//32
        if (dir_index < per_cluster): # within this cluster
            return self._cluster_offset(cluster) + (32 * dir_index)
        # continue to next cluster
        return self._dir_entry_offset(self.fat[cluster],dir_index-per_cluster)

    def delete_file(self, entry):
        """Deletes a FileEntry."""
        self._delete_chain(entry.cluster) # delete its FAT chain
        offset = self._dir_entry_offset(entry.dir_cluster,entry.dir_index)
        self.data[offset+0] = Floppy.EMPTY # empty this entry

    def _find_path_dir(self, cluster, path):
        """Recursive find path, breaking out subdirectories progressively."""
        #print("_find_path_dir(%03X,'%s')" % (cluster,path))
        separator = path.find("\\")
        path_seek = path
        path_next = ""
        if separator >= 0:
            path_seek = path[0:separator]
            path_next = path[separator+1:]
        directory = self._read_dir_chain(cluster)
        for i in range(len(directory) // 32):
            e = self.FileEntry(directory[(i*32):(i*32)+32],cluster,i)
            if e.incipit == 0x00: # end of directory
                return None
            if e.incipit == Floppy.EMPTY: # empty
                continue
            if (e.attributes & 0x08) != 0: # volume label
                continue
            if (e.attributes & 0x10) != 0: # subdirectory
                if e.path == "." or e.path == "..":
                    continue
                if e.path == path_seek:
                    if (len(path_next) > 0):
                        return self._find_path_dir(e.cluster, path_next)
                    else:
                        return e
            elif e.path == path_seek and path_next == "":
                return e
        return None

    def find_path(self, path):
        """Finds a FileEntry for a given path."""
        return self._find_path_dir(0,path)

    def _delete_tree(self,de):
        """Recursively deletes directory entries."""
        #print("_delete_tree\n" + de.info())
        directory = self._read_dir_chain(de.cluster)
        for i in range(len(directory) // 32):
            e = self.FileEntry(directory[(i*32):(i*32)+32],de.cluster,i)
            #print(e.info())
            if e.incipit == 0x00: # end of directory
                return
            if e.incipit == Floppy.EMPTY: # empty
                continue
            if (e.attributes & 0x08) != 0: # volume label
                continue
            if (e.attributes & 0x10) != 0: # subdirectory
                if e.path == "." or e.path == "..":
                    continue
                self._delete_tree(e) # recurse
                self.delete_file(e)
            else:
                self.delete_file(e)

    def delete_path(self, path):
        """Finds a file or directory and deletes it (recursive), if it exists. Returns True if successful."""
        e = self.find_path(path)
        if (e == None):
            return False
        if (e.attributes & 0x10) != 0:
            self._delete_tree(e)
        self.delete_file(e)
        return True

    def _add_entry(self, dir_cluster, entry):
        """
        Adds an entry to a directory starting at the given cluster, appending a new cluster if needed.
        Sets entry.dir_cluster and entry.dir_index to match their new directory.
        Returns False if out of space.
        """
        #print(("_add_entry(%d):\n"%dir_cluster) + entry.info())
        directory = self._read_dir_chain(dir_cluster)
        dir_len = len(directory)//32
        i = 0
        terminal = False
        while i < dir_len:
            e = self.FileEntry(directory[(i*32):(i*32)+32],dir_cluster,i)
            #print(e.info())
            if e.incipit == 0x00:
                terminal = True # make sure to add another terminal entry after this one
                break
            if e.incipit == Floppy.EMPTY:
                break
            i += 1
        # extend directory if out of room
        if i >= dir_len:
            if dir_cluster < 2:
                return False # no room in root
            # add a zero-filled page to the end of this directory's FAT chain
            chain = self._add_chain(bytearray([0]*(self.sector_size*self.cluster_sects)))
            if (chain < 0):
                return False # no free clusters
            tail = dir_cluster
            while self.fat[tail] < 0xFF0:
                tail = self.fat[tail]
            self.fat[tail] = chain
            self.fat[chain] = 0xFFF
        # insert entry
        entry.dir_cluster = dir_cluster
        entry.dir_index = i
        offset = self._dir_entry_offset(dir_cluster,i)
        self.data[offset:offset+32] = entry.compile()
        # add a new terminal if needed
        if terminal:
            i += 1
            if i < dir_len: # if it was the last entry, no new terminal is needed
                offset = self._dir_entry_offset(dir_cluster,i)
                self.data[offset:offset+32] = Floppy.FileEntry.new_terminal().compile()
                
        # success!
        return True

    def _add_dir_recursive(self, cluster, path):
        """Recursively creates directory, returns cluster of created dir, -1 if failed."""
        #print("_add_dir_recursive(%03X,'%s')"%(cluster,path))
        separator = path.find("\\")
        path_seek = path
        path_next = ""
        if separator >= 0:
            path_seek = path[0:separator]
            path_next = path[separator+1:]
        directory = self._read_dir_chain(cluster)
        for i in range(len(directory) // 32):
            e = self.FileEntry(directory[(i*32):(i*32)+32],cluster,i)
            #print(e.info())
            if e.incipit == 0x00: # end of directory
                break
            if e.incipit == Floppy.EMPTY: # empty
                continue
            if (e.attributes & 0x10) != 0: # subdirectory
                if e.path == path_seek: # already exists
                    if len(path_next) < 1:
                        return e.cluster # return existing directory
                    else:
                        return self._add_dir_recursive(e.cluster,path_next) # keep descending
        # not found: create the directory
        dp0 = Floppy.FileEntry.new_dir("_") # .
        dp1 = Floppy.FileEntry.new_dir("__") # ..
        dp1.cluster = cluster
        dir_block = dp0.compile() + dp1.compile() + bytearray([0] * ((self.sector_size*self.cluster_sects)-64))
        new_cluster = self._add_chain(dir_block)
        if new_cluster < 0:
            return -1 # out of space
        # fix up "." to point to itself
        dp0.cluster = new_cluster
        offset = self._dir_entry_offset(new_cluster,0)
        self.data[offset:offset+32] = dp0.compile()
        # fix special directory names (FileEntry.compile() is incapable of . or ..)
        self.data[offset+ 0:offset+11] = bytearray(".          ".encode("ASCII"))
        self.data[offset+32:offset+43] = bytearray("..         ".encode("ASCII"))
        # create entry to point to new directory cluster
        new_dir = Floppy.FileEntry.new_dir(path_seek)
        new_dir.cluster = new_cluster
        if not self._add_entry(cluster, new_dir):
            self._delete_chain(new_cluster)
            return -1 # out of space
        # return the entry if tail is reached, or keep descending
        if len(path_next) < 1:
            return new_cluster
        else:
            return self._add_dir_recursive(new_cluster,path_next)

    def add_dir_path(self, path):
        """
        Recursively ensures that the given directory path exists, creating it if necessary.
        Path should not end in backslash.
        Returns cluster of directory at path, or -1 if failed.
        """
        if (len(path) < 1):
            return 0 # root
        return self._add_dir_recursive(0,path)

    def add_file_path(self, path, data):
        """
        Adds the given data as a file at the given path.
        Will automatically create directories to complete the path.
        Returns False if failed.
        """
        self.delete_path(path) # remove file if it already exists
        dir_path = ""
        file_path = path
        separator = path.rfind("\\")
        if (separator >= 0):
            dir_path = path[0:separator]
            file_path = path[separator+1:]
        dir_cluster = self.add_dir_path(dir_path)
        if dir_cluster < 0:
            return False # couldn't find or create directory
        cluster = self._add_chain(data)
        if (cluster < 0):
            return False # out of space
        entry = Floppy.FileEntry.new_file(file_path)
        entry.cluster = cluster
        entry.size = len(data)
        if not self._add_entry(dir_cluster, entry):
            self._delete_chain(cluster)
            return False # out of space for directory entry
        offset = self._dir_entry_offset(entry.dir_cluster, entry.dir_index)
        self.data[offset:offset+32] = entry.compile()
        return True

    def extract_file_path(self, path):
        """Finds a file and returns all of its data. None on failure."""
        e = self.find_path(path)
        if e == None:
            return None
        return self._read_chain(e.cluster, e.size)
        
    def set_volume_id(self, value=None):
        """Sets the volume ID. None for time-based."""
        if (value == None):
            (date,time) = Floppy.FileEntry.fat_time_now()
            value = time | (date << 16)
        self.volume_id = value & 0xFFFFFFFF

    def set_volume_label(self, label):
        """Sets the volume label. Creates one if necessary."""
        self.data[38] = 0x29
        self.volume_label = label
        self.data[54:62] = Floppy._filestring("FAT12",8)
        # adjust existing volume entry in root
        directory = self._read_dir_chain(0)
        for i in range(len(directory) // 32):
            e = self.FileEntry(directory[(i*32):(i*32)+32],0,i)
            if e.incipit == 0x00: # end of directory
                break
            if e.incipit == Floppy.EMPTY: # empty
                continue
            if (e.attributes & 0x08) != 0: # existing volume label
                offset = self._dir_entry_offset(0,i)
                self.data[offset:offset+11] = Floppy._filestring(label,11)
                return
        # volume entry does not exist in root, add one
        self._add_entry(0,Floppy.FileEntry.new_volume(label))
        return

    def open(filename):
        """Opens a floppy image file and creates a Floppy() instance from it."""
        return Floppy(open(filename,"rb").read())

    def flush(self):
        """Commits all unfinished changes to self.data image."""
        self._fat_flush()
        self._boot_flush()

    def save(self, filename):
        """Saves image (self.data) to file. Implies flush()."""
        self.flush()
        open(filename,"wb").write(self.data)

    def extract_all(self, out_directory):
        """Extracts all files from image to specified directory."""
        for in_path in self.files():
            out_path = os.path.join(out_directory,in_path)
            out_dir = os.path.dirname(out_path)
            if not os.path.exists(out_dir):
                try:
                    os.makedirs(out_dir)
                except OSError as e:
                    if e.errono != errno.EEXIST:
                        raise
            if not in_path.endswith("\\"):
                open(out_path,"wb").write(self.extract_file_path(in_path))
                print(out_path)
            else:
                print(out_path)

    def add_all(self, in_directory, prefix=""):
        """
        Adds all files from specified directory to image.
        Files will be uppercased. Long filenames are not checked and will cause an exception.
        prefix can be used to prefix a directory path (ending with \) to the added files.
        """
        result = True
        def dospath(s):
            s = s.upper()
            s = s.replace("/","\\")
            return s
        if len(in_directory) < 1:
            in_directory = "."
        in_directory = os.path.normpath(in_directory) + os.sep
        for (root, dirs, files) in os.walk(in_directory):
            base = root[len(in_directory):]
            for d in dirs:
                dir_path = prefix + dospath(os.path.join(base,d))
                result = result and (self.add_dir_path(dir_path) >= 0)
                print(dir_path + "\\")
            for f in files:
                file_path = prefix + dospath(os.path.join(base,f))
                data = open(os.path.join(root,f),"rb").read()
                result = result and self.add_file_path(file_path,data)
                print(file_path + " (%d bytes)" % len(data))
        return result
