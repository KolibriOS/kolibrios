typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long long qword;

#pragma pack(push,1)
typedef struct
{
unsigned        p00;
unsigned        p04;
unsigned        p08;
unsigned        p12;
unsigned        p16;
char            p20;
char            *p21;
} FS_struct70;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
   byte d;
   byte m;
   word y;
}date;


#pragma pack(push,1)
typedef struct
{
   byte s;
   byte m;
   byte h;
   byte zero;
}time;

#pragma pack(push,1)
typedef struct
{
dword       attrib;
byte        enc_name;
byte        res[3];
time        c_time;
date        c_date;
time        a_time;
date        a_date;
time        m_time;
date        m_date;
dword         size;
byte     name[520];
} FS_struct_BDVK;
#pragma pack(pop)

static inline int FS_file_70(FS_struct70 *k)
{
        int err=0;
        asm volatile ("int $0x40":"=a"(err):"a"(70), "b"(k));
        return err;
}

FS_struct_BDVK ret_struct;

static inline FS_struct_BDVK* get_bdvk(char *path)
{
        FS_struct70 file_read_struct;
        file_read_struct.p00 = 5;
        file_read_struct.p04 = 0;
        file_read_struct.p08 = 0;
        file_read_struct.p12 = 0;
        file_read_struct.p16 = (unsigned)&ret_struct;
        file_read_struct.p20 = 0;
        file_read_struct.p21 = path;
        if(!FS_file_70(&file_read_struct))
        {
            return (FS_struct_BDVK*)&ret_struct;
        }
        else
        {
            return NULL;
        }
}
