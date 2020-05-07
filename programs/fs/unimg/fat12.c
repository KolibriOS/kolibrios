/*  # KolibriOS Image Unpacker #

Extracts files from FAT12 KolibriOS image to specified folder.

Usage:  unimg path/to/img [output/folder] [-e]
        -e: Exit on success
        If output folder is skipped, the image will be unpacked at /TMP0/1/[file-name]

Author: Magomed Kostoev (Boppan, mkostoevr): FAT12 file system, driver.
Contributor: Kiril Lipatov (Leency) */

#ifdef __TINYC__
#   define TCC 1
#else
#   define GCC 1
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if TCC
#   include <conio.h>
#   define printf con_printf
#   define puts con_write_asciiz
#else
#   define con_init_console_dll() 0
#   define con_set_title(s)
#   define con_exit(close)
#endif

typedef struct {
    size_t length;
    size_t capacity;
    char *data;
} String;

typedef struct {
    char *image;
    int imageSize;
    const char *errorMessage;
    int bytesPerSector;
    int sectorsPerClaster;
    int reservedSectorCount;
    int numberOfFats;
    int maxRootEntries;
    int totalSectors;
    int sectorsPerFat;
    int firstFat;
    int rootDirectory;
    int dataRegion;
} Fat12;

typedef int (*ForEachCallback)(const char *, size_t, const uint8_t *, void *);

// system-dependent
static void mkdir(const char *name);
// misc
static void mkdir_p(const char *_name); // create folder creating its parents
static uint16_t get16(const void *_from, int index); // get uint16_t from array at offset
static uint32_t get32(const void *_from, int index); // get uint32_t from array at offset
// fat12
static int fat12__getItemNameSize(const void *_folderEntry);
static void fat12__getItemName(const void *_folderEntry, void *_name);
static int fat12__getNextClaster(const Fat12 *this, int currentClaster);
static int fat12__getFile(const Fat12 *this, void *_buffer, int size, int claster);
static int fat12__getOffsetByClaster(const Fat12 *this, int claster);
static int fat12__forEachFile_handleFolderEntry(const Fat12 *this, int folderEntryOffset, String *name,
                                                ForEachCallback callback, void *callbackParam);
static int fat12__forEachFile_handleFolder(const Fat12 *this, int claster, String *name,
                                           ForEachCallback callback, void *callbackParam);
static int fat12__forEachFile(const Fat12 *this, ForEachCallback callback, void *callbackParam);
static int fat12__open(Fat12 *this, const char *img);
static int fat12__error(Fat12 *this, const char *errorMessage);

static void mkdir(const char *name) {
    struct {
        int fn;
        int unused[4];
        char b;
        const char *path __attribute__((packed));
    } info;

    memset(&info, 0, sizeof(info));
    info.fn = 9;
    info.b = 0;
    info.path = name;
    asm volatile ("int $0x40"::"a"(70), "b"(&info));
}

static void mkdir_p(const char *_name) {
    char *name = calloc(strlen(_name) + 1, 1);

    strcpy(name, _name);
    char *ptr = name;
    while (ptr) {
        if (ptr != name) { *ptr = '/'; }
        ptr = strchr(ptr + 1, '/');
        if (ptr) { *ptr = 0; }
        mkdir(name);
    }
}

static uint32_t get32(const void *_from, int index) {
    const uint8_t *from = _from;
    return from[index] |
        (from[index + 1] << 8) |
        (from[index + 2] << 16) |
        (from[index + 3] << 24);
}

static uint16_t get16(const void *_from, int index) {
    const uint8_t *from = _from;

    return from[index] | (from[index + 1] << 8);
}

static int fat12__getNextClaster(const Fat12 *this, int currentClaster) {
    int nextClasterOffset = this->firstFat + currentClaster + (currentClaster >> 1);

    if (currentClaster % 2 == 0) {
        return get16(this->image, nextClasterOffset) & 0xfff;
    } else {
        return get16(this->image, nextClasterOffset) >> 4;
    }
}

static int fat12__getFile(const Fat12 *this, void *_buffer, int size, int claster) {
    int offset = 0;
    char *buffer = _buffer;

    while (claster < 0xff7) {
        int toCopy = this->bytesPerSector * this->sectorsPerClaster;
        void *clasterPtr = &this->image[fat12__getOffsetByClaster(this, claster)];

        claster = fat12__getNextClaster(this, claster);
        // if next claster is END OF FILE claster, copy only rest of file
        if (claster >= 0xff7) { toCopy = size % toCopy; }
        memcpy(&buffer[offset], clasterPtr, toCopy);
        offset += toCopy;
    }
    return 1;
}

static int fat12__getOffsetByClaster(const Fat12 *this, int claster) {
    return this->dataRegion + (claster - 2) 
        * this->bytesPerSector * this->sectorsPerClaster;
}

static int fat12__getItemNameSize(const void *_folderEntry) {
    const uint8_t *folderEntry = _folderEntry;

    // Long File Name entry, not a file itself
    if ((folderEntry[11] & 0x0f) == 0x0f) { return 0; }
    if ((folderEntry[11 - 32] & 0x0f) != 0x0f) {
        // regular file "NAME8888" '.' "EXT" '\0'
        int length = 13;

        for (int i = 10; folderEntry[i] == ' ' && i != 7; i--) { length--; }
        for (int i = 7; folderEntry[i] == ' ' && i != 0 - 1; i--) { length--; }
        if (folderEntry[8] == ' ') { length--; } // no ext - no'.'
        return length;
    } else {
        // file with long name
        // format of Long File Name etries is described in fat12__getItemName
        int length = 1;

        for (int i = 1; i < 255 / 13; i++) {
            //! TODO: Add UTF-16 support
            length += 13;
            if (folderEntry[i * -32] & 0x40) {
                // if first char from back is 0xffff, this is stub after name
                // otherwice is last character, so we can return calculated length
                if (get16(folderEntry, i * -32 + 30) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 28) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 24) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 22) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 20) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 18) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 16) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 14) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 9) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 7) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 5) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 3) == 0xffff) { length--; } else { return length; }
                if (get16(folderEntry, i * -32 + 1) == 0xffff) { length--; } else { return length; }
                return length;
            }
        }
    }
    return 0; // WAT?
}

static void fat12__getItemName(const void *_folderEntry, void *_name) {
    const uint8_t *folderEntry = _folderEntry;
    uint8_t *name = _name;

    if ((folderEntry[11 - 32] & 0x0f) != 0x0f) {
        int length = 8;

        memset(name, 0, 13);
        memcpy(name, folderEntry, 8);
        while (name[length - 1] == ' ') { length--; }
        if (folderEntry[9] != ' ') {
            name[length++] = '.';
            memcpy(&name[length], &folderEntry[8], 3);
            length += 3;
        }
        while (name[length - 1] == ' ') { length--; }
        name[length] = '\0';
    } else {
        // previous folder entries hold long name in format:
        // 0 sequence nmber (in turn back to first Long File Name entry, from 1)
        // 1 - 10 file name next characters in utf-16
        // 11 file attributes (0x0f - LFN entry)
        // 12 reserved
        // 13 checksum
        // 14 - 25 file name next characters
        // 26 - 27 reserved
        // 28 - 31 file name next characters
        // in these entries name placed in sequential order
        // but first characters are located in first previous entry
        // next characters - in next previous etc.
        // if current entry is orificated by 0x40 - the entry is last (cinains last characters)
        // unneed places for characters in the last entry are filled by 0xff
        int length = 0;

        for (int i = 1; i < 255 / 13; i++) {
            //! TODO: Add unicode support
            name[length++] = folderEntry[i * -32 + 1];
            name[length++] = folderEntry[i * -32 + 3];
            name[length++] = folderEntry[i * -32 + 5];
            name[length++] = folderEntry[i * -32 + 7];
            name[length++] = folderEntry[i * -32 + 9];
            name[length++] = folderEntry[i * -32 + 14];
            name[length++] = folderEntry[i * -32 + 16];
            name[length++] = folderEntry[i * -32 + 18];
            name[length++] = folderEntry[i * -32 + 20];
            name[length++] = folderEntry[i * -32 + 22];
            name[length++] = folderEntry[i * -32 + 24];
            name[length++] = folderEntry[i * -32 + 28];
            name[length++] = folderEntry[i * -32 + 30];
            if (folderEntry[i * -32] & 0x40) {
                while (name[length - 1] == 0xff) { name[--length] = 0; }
                name[length++] = 0;
                return;
            }
        }
    }
}


static int fat12__forEachFile_handleFolderEntry(const Fat12 *this, int folderEntryOffset, String *name,
                                                ForEachCallback callback, void *callbackParam) {
    int nameSize = 0;

    if (this->image[folderEntryOffset] == 0) { return 1; } // zero-entry, not file nor folder
    nameSize = fat12__getItemNameSize(&this->image[folderEntryOffset]); // includes sizeof '\0'
    if (nameSize != 0) {
        while (name->capacity < name->length + nameSize + 1) {
            name->capacity += name->capacity / 2;
            name->data = realloc(name->data, name->capacity);
        }
        name->data[name->length++] = '/';
        fat12__getItemName(&this->image[folderEntryOffset], &name->data[name->length]);
        name->length += nameSize - 1;
        if ((this->image[folderEntryOffset + 11] & 0x10)) {
            // the item is folder
            // handle folder only if it isn't current folder or parent one
            if (memcmp(&this->image[folderEntryOffset], ".          ", 11) &&
                memcmp(&this->image[folderEntryOffset], "..         ", 11)) {
                if (!fat12__forEachFile_handleFolder(this, get16(this->image, folderEntryOffset + 26), name, callback, callbackParam)) {
                    return 0;
                }
            }
        } else {
            // the item is a regular file
            void *buffer = NULL;
            int size = get32(this->image, folderEntryOffset + 28);
            int cluster = get16(this->image, folderEntryOffset + 26);

            buffer = malloc(size);
            if (!fat12__getFile(this, buffer, size, cluster)) {
                free(buffer);
                return 0;
            }
            callback(name->data, size, buffer, callbackParam);
            free(buffer);
        }
        name->length -= nameSize - 1; // substract length of current item name
        name->length--; // substract length of '/'
        name->data[name->length] = '\0';
    }
    return 1;
}

static int fat12__forEachFile_handleFolder(const Fat12 *this, int claster, String *name,
                                           ForEachCallback callback, void *callbackParam) {
    for (; claster < 0xff7; claster = fat12__getNextClaster(this, claster)) {
        int offset = fat12__getOffsetByClaster(this, claster);

        for (int i = 0; i < (this->bytesPerSector * this->sectorsPerClaster / 32); i++) {
            if (!fat12__forEachFile_handleFolderEntry(this, offset + 32 * i, name, callback, callbackParam)) {
                return 0;
            }
        }
    }
    return 1;
}

static int fat12__forEachFile(const Fat12 *this, ForEachCallback callback, void *callbackParam) {
    String name = { 0 };

    name.capacity = 4096;
    name.data = malloc(name.capacity);
    name.length = 0;
    name.data[0] = '\0';

    for (int i = 0; i < this->maxRootEntries; i++) {
        if (!fat12__forEachFile_handleFolderEntry(this, this->rootDirectory + 32 * i, &name, callback, callbackParam)) {
            free(name.data);
            return 0;
        }
    }
    free(name.data);
    return 1;
}

static int fat12__open(Fat12 *this, const char *img) {
    FILE *fp = NULL;

    if (!(fp = fopen(img, "rb"))) { 
        return fat12__error(this, "Can't open imput file"); 
    }
    fseek(fp, 0, SEEK_END);
    this->imageSize = ftell(fp);
    rewind(fp);
    if (!(this->image = malloc(this->imageSize))) { 
        return fat12__error(this, "Can't allocate memory for image"); 
    }
    fread(this->image, 1, this->imageSize, fp);
    fclose(fp);
    this->bytesPerSector = *(uint16_t *)((uintptr_t)this->image + 11);
    this->sectorsPerClaster = *(uint8_t *)((uintptr_t)this->image + 0x0d);
    this->reservedSectorCount = *(uint16_t *)((uintptr_t)this->image + 0x0e);
    this->numberOfFats = *(uint8_t *)((uintptr_t)this->image + 0x10);
    this->maxRootEntries = *(uint16_t *)((uintptr_t)this->image + 0x11);
    this->totalSectors = *(uint16_t *)((uintptr_t)this->image + 0x13);
    if (!this->totalSectors) { 
        this->totalSectors = *(uint32_t *)((uintptr_t)this->image + 0x20); 
    }
    this->sectorsPerFat = *(uint16_t *)((uintptr_t)this->image + 0x16);
    this->firstFat = (0 + this->reservedSectorCount) * this->bytesPerSector;
    this->rootDirectory = this->firstFat + this->numberOfFats 
        * this->sectorsPerFat * this->bytesPerSector;
    this->dataRegion = this->rootDirectory + this->maxRootEntries * 32;
    printf("\nBytes per sector: %d\n",    this->bytesPerSector);
    printf("Sectors per claster: %d\n",   this->sectorsPerClaster);
    printf("Reserver sector count: %d\n", this->reservedSectorCount);
    printf("Number of FATs: %d\n",        this->numberOfFats);
    printf("Max root entries: %d\n",      this->maxRootEntries);
    printf("Total sectors: %d\n",         this->totalSectors);
    printf("Sectors per FAT: %d\n",       this->sectorsPerFat);
    printf("First FAT: %d\n",             this->firstFat);
    printf("Root directory: %d\n",        this->rootDirectory);
    printf("Data region: %d\n\n",         this->dataRegion);
    return 1;
}

static int fat12__error(Fat12 *this, const char *errorMessage) {
    this->errorMessage = errorMessage;
    return 0;
}

static int handleError(const Fat12 *fat12) {
    printf("Error in Fat12: %s\n", fat12->errorMessage);
    con_exit(0);
    return -1;
}

void writeFile(const char *fileName, int size, const uint8_t *data) {
#if TCC
    struct Info {
        int number;
        int reserved0;
        int reserved1;
        int dataSize;
        const void *data;
        char zero;
        const char *name __attribute__((packed));
    } info;

    memset(&info, 0, sizeof(struct Info));
    info.number = 2; // create/overwrite file
    info.dataSize = size;
    info.data = data;
    info.zero = 0;
    info.name = fileName;
    asm volatile ("int $0x40" :: "a"(70), "b"(&info));
#else
    FILE *fp = NULL;
    if (!(fp = fopen(fileName, "wb"))) { perror(NULL); }
    fwrite(data, 1, size, fp);
    fclose(fp);
#endif
}

static int callback(const char *name, size_t size, const uint8_t *data, void *param) {
    String *outputPath = param;

    while (outputPath->capacity < outputPath->length + strlen(name) + 1 + 1) {
        outputPath->capacity += outputPath->capacity / 2;
        outputPath->data = realloc(outputPath->data, outputPath->capacity);
    }
    strcat(outputPath->data, name);
    { // don't let mkdir_p create folder where file should be located
        char *fileNameDelim = NULL;
    
        // no slash = no folders to create, outputPath->data contains only file name
        // yes, I know, outputPath->data always contains '/', but who knows...
        if ((fileNameDelim = strrchr(outputPath->data, '/'))) {
            *fileNameDelim = '\0';
            mkdir_p(outputPath->data);
            *fileNameDelim = '/';
        }
    }
    printf("Extracting %s\n", outputPath->data);
    writeFile(outputPath->data, size, data);
    outputPath->data[outputPath->length] = '\0';
    return 0;
}



int main(int argc, char* argv[]) {
    Fat12 fat12 = { 0 };
    char *imageFile = NULL;
    String outputFolder = { 0 };
    int closeOnExit = 0;

    if (con_init_console_dll()) { return -1; }
    con_set_title("UnImg - kolibri.img file unpacker");

    if (argc < 2) { 
        puts(" Usage:");
        puts(" unimg \"/path/to/kolibri.img\" \"/optional/extract/path\"");
        puts("       where optional key [-e] is exit on success");
        con_exit(0);
        return -1;
    } else {
        imageFile = argv[1];
        printf("File: %s\n", imageFile);
    }
    
    outputFolder.capacity = 4096;
    outputFolder.data = malloc(outputFolder.capacity);

    //! ACHTUNG: possible buffer overflow, is 4096 enough in KolibriOS?
    if (argc >= 3 && argv[2][0] != '-') { strcpy(outputFolder.data, argv[2]); }
    else {
        strcpy(outputFolder.data, "/tmp0/1");
        strcat(outputFolder.data, strrchr(imageFile, '/'));
    }

    outputFolder.length = strlen(outputFolder.data);

    // handle -e parameter - exit on success
    if (argc >= 3 && !strcmp(argv[argc - 1], "-e")) { closeOnExit = 1; }

    if (!fat12__open(&fat12, imageFile)) {
        return handleError(&fat12);
    }
    if (!fat12__forEachFile(&fat12, callback, &outputFolder)) {
        return handleError(&fat12);
    }

    puts("\nDONE!");
    con_exit(closeOnExit);
    return 0;
}
