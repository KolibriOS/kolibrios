#include <stdio.h>
#define INVALID_ARG "Invalid arguments! Use the help: -h!\n"
#define KEY_RECORD_IN_FILE "The key is recorded in file: %s\n"
#define INCORRECT_FILE "Error! Incorrect file:'%s'\n"
#define NO_KEY_OR_KEYFILE "No key or key file!\n"
#define INVALID_KEY_FORMAT "Invalid key format!\n"
#define FILE_NOT_FOUND "File '%s' not found!\n"
#define DATA_DECRYPT "Data from file: %s successfully DECRYPTED to file: %s\n"
#define DATA_ENCRYPT "Data from file: %s successfully ENCRYPTED to file: %s\n"
#define FILE_DECRYPTION "File decryption...\n"
#define FILE_ENCRYPTION "File encryption...\n"
#define RECORD_DECRYPT_DATA "Record decryped data...\n"
#define LOAD_IN_RAM "Loading a '%s' file in RAM...\n"
#define RECORD_ENCRYPT_DATA "Record encryped data...\n"
#define MEMORY_ERROR "To big file, not enough memory! Use normal mode."

void show_help()
{
    puts("Usage: \nTEAtool [infile] [outfile] [arguments]\n");
    puts("Arguments:"); 
    puts("-e [mode] Encrypt file in 'speed' or 'normal' mode.");  
    puts("-d [mode] Decrypt file in 'speed' or 'normal' mode.");   
    puts("   In 'speed' mode, file is entirely loaded into RAM.");
    puts("   In 'normal' mode, file is loaded with parts in RAM.");  
    puts("-k [key]  128bit-key in hex format."); 
    puts("-K [keyfile]  Use key from file."); 
    puts("-r [key] [keyfile].key  Key entry to key file.");
    puts("-h This reference"); 
    puts("-a About the program \n");
}

void show_about()
{
    puts("\n");
    puts("-----------TEAtool-ENG-----------\n");
    printf("      )  (        Version:     \n");
    printf("     (   ) )     1.8-stable    \n");
    printf("      ) ( (                    \n");
    printf("    _______)_     Author:      \n");
    printf(" .-'---------|  turbocat2001   \n");
    printf("( C|/////////|                 \n");
    printf(" '-./////////| Tester: rgimad  \n");
    printf("   '_________'                 \n");
    printf("   '-------''  License: GPLv3  \n\n");
    printf("          Powered by:          \n");
    printf("  Tiny Encryption Algorithm. \n\n");
}


