/* Copyright (C) 2019-2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>

#include "lang_en.c"
#include "tea.c"

#define ENCRYPT 1       
#define DECRYPT 2

typedef unsigned char flag;
uint32_t key[4]; 

long size_orig_file(FILE* file)
{
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);  
    return size;
}

long size_xcrypt_file(FILE* file)
{
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if(size%8==0) {
        return size;
    }else{
        return (size/8+1)*8;
    }
}

void xcrypt_file_speed(char *in_file, char* out_file, char arg)
{
    FILE *input, *output; 
    
    if((input = fopen(in_file,"rb"))==NULL){
        printf(FILE_NOT_FOUND, in_file);
        exit(1);
    }
    
    output = fopen(out_file,"wb");

    long size_f=size_xcrypt_file(input);                               
    uint8_t size_diff;
    size_diff=(uint8_t)(size_f-size_orig_file(input)); 
    uint32_t *buff; 
    buff=malloc(size_f);
    if(!buff) {
        puts(MEMORY_ERROR);
        exit(-1);
    }
        
    if(arg==ENCRYPT){                        
        printf(LOAD_IN_RAM,in_file);
        fread(buff, 1,size_f, input); 
        printf(FILE_ENCRYPTION); 
            
        for(long i=0; i<(size_f/4); i=i+2)
        {
            TEA_encrypt(buff+i,key); 
        }
            
        printf(RECORD_ENCRYPT_DATA); 
        fwrite(&size_diff,1, 1, output);      
        fwrite(buff,1,size_f, output);        
        fclose(input);                         
        fclose(output);                        
        printf(DATA_ENCRYPT,in_file,out_file);
        exit(0);
    }
    
    else if(arg==DECRYPT){                     
        long size_f=size_orig_file(input);
        printf(LOAD_IN_RAM,in_file);
        fread(&size_diff,1,1,input);          
        fread(buff,1,size_f-1, input);        
        printf(FILE_DECRYPTION);  
              
        for(long i=0; i<size_f/4; i=i+2)
        {
            TEA_decrypt(buff+i,key);        
        } 
     
        printf(RECORD_DECRYPT_DATA);
        fwrite(buff,1,size_f-size_diff-1, output); 
        fclose(input);
        fclose(output);
        printf(DATA_DECRYPT,in_file,out_file);
        exit(0);
    }
}

void xcrypt_file(char *in_file, char* out_file, char arg) 
{
    uint32_t temp_block[2];  
    FILE *input, *output;
   
    if((input = fopen(in_file,"rb"))==NULL){
        printf(FILE_NOT_FOUND, in_file);
        exit(1);
    }

    output = fopen(out_file,"wb");
    
    register long size_f=size_xcrypt_file(input);                 
    uint8_t size_diff=(uint8_t)(size_f-size_orig_file(input)); 
    
    if(arg==ENCRYPT){                                   
        fwrite(&size_diff,1,1,output);
        printf(FILE_ENCRYPTION);          
        
        while(!feof(input))
        {
            memset(temp_block, 0x00, 2);
            fread(temp_block, sizeof(uint32_t), 2, input) ;
            TEA_encrypt(temp_block,key);
            fwrite(temp_block, sizeof(uint32_t),2, output);
        }
        
        fclose(input); 
        fclose(output);
        printf(DATA_ENCRYPT,in_file,out_file);
        exit(0);

    }
    else if(arg==DECRYPT){ 
        size_f = size_orig_file(input);
        fread(&size_diff,1,1,input);
        size_f=size_f-size_diff-1; 
        printf(FILE_DECRYPTION); 
           
        while(!feof(input))      
        { 
            fread(temp_block, sizeof(uint32_t), 2, input);
            TEA_decrypt(temp_block,key);
            
            if(size_f>=8){
                fwrite(temp_block,sizeof(uint32_t),2,output);
            }else{    
                fwrite(temp_block,1,size_f,output);
            }
            
            size_f=size_f-8;
            
            if(size_f<0){
                size_f=0;
            }    
        }

        fclose(input);
        fclose(output);
        printf(DATA_DECRYPT,in_file,out_file);
        exit(0);
    }
}



void str_to_strkey(char *str, char str_key[4][9])
{
    int count=0;
    for(int i=0; i<4; i++)
    {
        int j=0;
        while (j<8)
        {
            str_key[i][j]=str[count];
            count++;
            j++;
        }
    }
}

int valid_key(char *str)                        
{
    int count=0;
    char hex[]={"abcdefABCDEF0123456789"};
    for(int i=0; i<32; i++)
    {
        if(strchr(hex,str[i])!=NULL){
            count++;
        }
     }
     if(count==32){return 1;}
     else{ return 0;}
}


void key_con_read(char *str)      
{
    char str_key[4][9];
    if(valid_key(str)&&(strlen(str)==32))
    {
        for(int i=0; i<4; i++){
            str_to_strkey(str, str_key);        
            key[i]=(uint32_t)strtol(str_key[i],NULL,16);
        }
    }else{
        printf(INVALID_KEY_FORMAT); 
        exit(-1);
    }
}

void key_file_read(char *key_file)                  
{
    FILE *keyfile;
    if((keyfile = fopen(key_file,"rb"))==NULL){
        printf(FILE_NOT_FOUND, key_file);
        exit(-1);
    }

    if(size_orig_file(keyfile)==16)    {
        fread(key,sizeof(uint32_t),4, keyfile);     
    }else{
        printf(INVALID_KEY_FORMAT);
        exit(-1);
    }
    fclose(keyfile);
}


void findopt(int argc, char *argv[],char *in_file, char *out_file)
{
    char found=0;
    for(int j=3; j<argc; j++)
    {
        if(!strcmp(argv[j],"-k")){
            found=1;
            key_con_read(argv[j+1]);
            break;
        }
        else if(!strcmp(argv[j],"-K")){
            found=1;
            key_file_read(argv[j+1]);
            break;
        }
    }

    if(!found){
        printf(NO_KEY_OR_KEYFILE);
        exit(-1);
    }
    
    for(int i=3;i<argc; i++){
        if(!strcmp(argv[i],"-e"))
        {
           if(!strcmp(argv[i+1],"normal")){xcrypt_file(in_file, out_file, ENCRYPT);}
           if(!strcmp(argv[i+1],"speed")){xcrypt_file_speed(in_file, out_file, ENCRYPT);}
        }
        if(!strcmp(argv[i],"-d"))
        {
           if(!strcmp(argv[i+1],"normal")){xcrypt_file(in_file, out_file, DECRYPT);}
           if(!strcmp(argv[i+1],"speed")){xcrypt_file_speed(in_file, out_file, DECRYPT);}
        }
    }
    printf(INVALID_ARG);
    exit(0);
}

void key_write_in_file(char *keyfilename)
{
    FILE *keyfile;
    if((keyfile = fopen(strcat(keyfilename, ".key"), "wb"))==NULL){
        printf(INCORRECT_FILE, keyfilename);
        exit(-1);
    }
    fwrite(key,sizeof(uint8_t), 16, keyfile);
    printf(KEY_RECORD_IN_FILE, keyfilename);
    fclose(keyfile);
    exit(0);
}

int main(int argc, char **argv)
{
   con_init_console_dll();
   con_set_title("TEAtool\0"); 
   if(argc==7){
         findopt(argc,argv, argv[1],argv[2]);
   }
   else if(argc==2 && !strcmp(argv[1],"-a")){
       show_about();
       exit(0);
   }
   else if(argc==2 && !strcmp(argv[1],"-h")){
       show_help();
       exit(0);
   }
   else if(argc==4 && !strcmp(argv[1],"-r")){
       key_con_read(argv[2]);
       key_write_in_file(argv[3]);
   }
   else{
       printf(INVALID_ARG);
       exit(0);
   }
    return 0;
}
